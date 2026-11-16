#include "panel_common.h"
#include <fcntl.h>
#include <sys/file.h>

ZoneWindow g_zone_left = {0};
ZoneWindow g_zone_center = {0};
ZoneWindow g_zone_right = {0};

static int s_server_fd = -1;
static int s_auto_collapse = 1;

#define PANEL_TOP_MARGIN -2   /* bar reserves 40px; -2 tucks the sheet under it */
#define PANEL_HIT_TOP 34
#define PANEL_SIDE_MARGIN 12
#define PANEL_GROW_MS 190.0
#define PANEL_SHRINK_MS 150.0

static void set_input_region(ZoneWindow *zw, int h) {
    cairo_region_t *region;
    if (h <= 0) {
        region = cairo_region_create();
    } else {
        cairo_rectangle_int_t rect = { 0, 0, zw->width, h };
        region = cairo_region_create_rectangle(&rect);
    }
    gtk_widget_input_shape_combine_region(zw->window, region);
    cairo_region_destroy(region);
}

/* Map every drawer once at startup as an invisible 1px sliver. Keeping the
   layer surfaces mapped removes the ~100ms map/first-buffer delay, so the
   grow animation starts on the very first frame after a click. */
static void warm_up_zone(ZoneWindow *zw) {
    if (!zw || !zw->window) return;
    zw->current_height = 1;
    zw->is_visible = 0;
    gtk_widget_set_size_request(zw->container, zw->width, 1);
    gtk_widget_set_size_request(zw->clip, zw->width, 1);
    gtk_window_resize(GTK_WINDOW(zw->window), zw->width, 1);
    gtk_widget_set_opacity(zw->window, 0.0);
    set_input_region(zw, 0);
    gtk_widget_show_all(zw->window);
}

/* ------------------------------------------------------------------ */
/*  Panel shell                                                        */
/*  window -> GtkFixed -> background sheet + full-height content       */
/*  The window height is animated while the content stays anchored to  */
/*  the top, so the panel appears to grow straight out of the bar.     */
/* ------------------------------------------------------------------ */
GtkWidget *create_panel_shell(ZoneWindow *zw, const char *name, int width, int height, int anchor) {
    zw->width = width;
    zw->height = height;
    zw->anchor = anchor;
    zw->current_height = 1;

    zw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_name(zw->window, name);
    gtk_style_context_add_class(gtk_widget_get_style_context(zw->window), "panel-window");

    /* The content is taller than the window while it grows; allow the
       toplevel to shrink so the window (not just the sheet) clips it. */
    GdkGeometry hints = {0};
    hints.min_width = 1;
    hints.min_height = 1;
    gtk_window_set_geometry_hints(GTK_WINDOW(zw->window), NULL, &hints,
                                  GDK_HINT_MIN_SIZE);

    gtk_layer_init_for_window(GTK_WINDOW(zw->window));
    gtk_layer_set_namespace(GTK_WINDOW(zw->window), "zone-panel");
    gtk_layer_set_layer(GTK_WINDOW(zw->window), GTK_LAYER_SHELL_LAYER_TOP);
    gtk_layer_set_anchor(GTK_WINDOW(zw->window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_margin(GTK_WINDOW(zw->window), GTK_LAYER_SHELL_EDGE_TOP, PANEL_TOP_MARGIN);
    gtk_layer_set_keyboard_mode(GTK_WINDOW(zw->window), GTK_LAYER_SHELL_KEYBOARD_MODE_NONE);

    if (anchor < 0) {
        gtk_layer_set_anchor(GTK_WINDOW(zw->window), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
        gtk_layer_set_margin(GTK_WINDOW(zw->window), GTK_LAYER_SHELL_EDGE_LEFT, PANEL_SIDE_MARGIN);
    } else if (anchor > 0) {
        gtk_layer_set_anchor(GTK_WINDOW(zw->window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
        gtk_layer_set_margin(GTK_WINDOW(zw->window), GTK_LAYER_SHELL_EDGE_RIGHT, PANEL_SIDE_MARGIN);
    }

    zw->fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(zw->window), zw->fixed);

    /* The sheet itself: no children, so its height can follow the window. */
    zw->container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(zw->container), "panel-bg");
    gtk_fixed_put(GTK_FIXED(zw->fixed), zw->container, 0, 0);
    gtk_widget_set_size_request(zw->container, width, 1);

    /* Fixed content: lives in a viewport so the window can be shorter than the
       content while growing, and clips the rest instead of letting widgets
       float over the desktop. */
    zw->clip = gtk_scrolled_window_new(NULL, NULL);
    gtk_style_context_add_class(gtk_widget_get_style_context(zw->clip), "panel-clip");
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(zw->clip),
                                   GTK_POLICY_EXTERNAL, GTK_POLICY_EXTERNAL);
    gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(zw->clip), TRUE);
    gtk_fixed_put(GTK_FIXED(zw->fixed), zw->clip, 0, 0);
    gtk_widget_set_size_request(zw->clip, width, 1);

    zw->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(zw->box), "panel-content");
    gtk_container_add(GTK_CONTAINER(zw->clip), zw->box);
    gtk_widget_set_size_request(zw->box, width, height);

    gtk_window_resize(GTK_WINDOW(zw->window), width, 1);
    return zw->box;
}

static double ease_out_cubic(double t) {
    double u = 1.0 - t;
    return 1.0 - u * u * u;
}

static double ease_in_cubic(double t) {
    return t * t * t;
}

static gboolean anim_tick(gpointer data) {
    ZoneWindow *zw = (ZoneWindow *)data;

    int growing = zw->anim_to > zw->anim_from;
    double duration = growing ? PANEL_GROW_MS : PANEL_SHRINK_MS;
    double t = (double)(g_get_monotonic_time() - zw->anim_start) / 1000.0 / duration;
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;

    double e = growing ? ease_out_cubic(t) : ease_in_cubic(t);
    int h = zw->anim_from + (int)((zw->anim_to - zw->anim_from) * e + 0.5);
    if (h < 1) h = 1;

    zw->current_height = h;
    gtk_widget_set_size_request(zw->container, zw->width, h);
    gtk_widget_set_size_request(zw->clip, zw->width, h);
    gtk_window_resize(GTK_WINDOW(zw->window), zw->width, h);
    set_input_region(zw, h);

    if (!growing) {
        /* Fade only on the way out, so the final collapse is invisible. */
        gtk_widget_set_opacity(zw->window, 1.0 - e);
    }

    if (t >= 1.0) {
        zw->anim_id = 0;
        zw->animating = 0;

        if (zw->anim_to <= 0) {
            zw->current_height = 0;
            gtk_widget_set_size_request(zw->container, zw->width, 1);
            gtk_widget_set_size_request(zw->clip, zw->width, 1);
            gtk_window_resize(GTK_WINDOW(zw->window), zw->width, 1);
            gtk_widget_set_opacity(zw->window, 0.0);
            set_input_region(zw, 0);
        } else {
            zw->current_height = zw->height;
        }
        return G_SOURCE_REMOVE;
    }

    return G_SOURCE_CONTINUE;
}

void animate_zone(ZoneWindow *zw, int target_height) {
    if (!zw || !zw->window) return;

    if (zw->anim_id > 0) {
        g_source_remove(zw->anim_id);
        zw->anim_id = 0;
    }

    zw->anim_from = zw->current_height;
    zw->anim_to = target_height;
    zw->anim_start = g_get_monotonic_time();
    zw->animating = 1;

    if (zw->anim_from == zw->anim_to) {
        zw->animating = 0;
        if (zw->anim_to <= 0) {
            zw->current_height = 0;
            gtk_widget_set_size_request(zw->container, zw->width, 1);
            gtk_widget_set_size_request(zw->clip, zw->width, 1);
            gtk_window_resize(GTK_WINDOW(zw->window), zw->width, 1);
            gtk_widget_set_opacity(zw->window, 0.0);
            set_input_region(zw, 0);
        } else {
            set_input_region(zw, zw->anim_to);
        }
        return;
    }

    gtk_widget_set_opacity(zw->window, 1.0);
    zw->anim_id = g_timeout_add(16, anim_tick, zw);
}

static void activate_icon_tab(GtkButton *b, gpointer user_data) {
    GtkWidget *stk = GTK_WIDGET(g_object_get_data(G_OBJECT(b), "stack"));
    const char *name = (const char *)g_object_get_data(G_OBJECT(b), "child-name");
    int idx = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(b), "tab-idx"));
    int *ptr = (int *)g_object_get_data(G_OBJECT(b), "active-ptr");
    if (!stk || !name) return;

    /* Already showing this tab: nothing to do (also prevents hover loops). */
    if (ptr && *ptr == idx &&
        g_strcmp0(gtk_stack_get_visible_child_name(GTK_STACK(stk)), name) == 0)
        return;

    gtk_stack_set_visible_child_name(GTK_STACK(stk), name);
    if (ptr) *ptr = idx;

    // Update all buttons in tab bar
    GtkWidget *parent = gtk_widget_get_parent(GTK_WIDGET(b));
    GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
    for (GList *l = children; l != NULL; l = l->next) {
        GtkWidget *tb = GTK_WIDGET(l->data);
        const char *ic = (const char *)g_object_get_data(G_OBJECT(tb), "tab-icon");
        const char *lb = (const char *)g_object_get_data(G_OBJECT(tb), "tab-label");
        if (!ic || !lb) continue;
        if (tb == GTK_WIDGET(b)) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%s  %s", ic, lb);
            gtk_button_set_label(GTK_BUTTON(tb), buf);
            gtk_style_context_add_class(gtk_widget_get_style_context(tb), "active-pill");
        } else {
            gtk_button_set_label(GTK_BUTTON(tb), ic);
            gtk_style_context_remove_class(gtk_widget_get_style_context(tb), "active-pill");
        }
    }
    g_list_free(children);
}

GtkWidget *create_icon_tab_button(const char *icon, const char *label, GtkWidget *stack, const char *child_name, int tab_idx, int *active_tab_ptr) {
    GtkWidget *btn = gtk_button_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(btn), "icon-tab-btn");

    g_object_set_data_full(G_OBJECT(btn), "tab-icon", strdup(icon), free);
    g_object_set_data_full(G_OBJECT(btn), "tab-label", strdup(label), free);
    g_object_set_data_full(G_OBJECT(btn), "child-name", strdup(child_name), free);
    g_object_set_data(G_OBJECT(btn), "stack", stack);
    g_object_set_data(G_OBJECT(btn), "tab-idx", GINT_TO_POINTER(tab_idx));
    g_object_set_data(G_OBJECT(btn), "active-ptr", active_tab_ptr);

    if (*active_tab_ptr == tab_idx) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s  %s", icon, label);
        gtk_button_set_label(GTK_BUTTON(btn), buf);
        gtk_style_context_add_class(gtk_widget_get_style_context(btn), "active-pill");
    } else {
        gtk_button_set_label(GTK_BUTTON(btn), icon);
    }

    g_signal_connect(btn, "clicked", G_CALLBACK(activate_icon_tab), NULL);
    g_signal_connect(btn, "enter-notify-event", G_CALLBACK(+[](GtkWidget *w, GdkEventCrossing *ev, gpointer u) -> gboolean {
        activate_icon_tab(GTK_BUTTON(w), u);
        return FALSE;
    }), NULL);
    /* Reliable hover trigger for GTK3 buttons (which are windowless). */
    g_signal_connect(btn, "state-flags-changed", G_CALLBACK(+[](GtkWidget *w, GtkStateFlags flags, gpointer u) {
        if (gtk_widget_get_state_flags(w) & GTK_STATE_FLAG_PRELIGHT)
            activate_icon_tab(GTK_BUTTON(w), u);
    }), NULL);

    return btn;
}

/* Fit the drawer to the active tab instead of using a fixed height. */
void fit_zone_to_content(ZoneWindow *zw) {
    if (!zw || !zw->window || !zw->stack || !zw->box || !zw->tab_box) return;

    int tab_h = 0;
    int child_h = 0;
    gtk_widget_get_preferred_height(zw->tab_box, NULL, &tab_h);

    GtkWidget *child = gtk_stack_get_visible_child(GTK_STACK(zw->stack));
    if (child) gtk_widget_get_preferred_height(child, NULL, &child_h);

    GtkBorder tab_margin = {0}, stack_margin = {0};
    gtk_style_context_get_margin(gtk_widget_get_style_context(zw->tab_box),
                                 GTK_STATE_FLAG_NORMAL, &tab_margin);
    gtk_style_context_get_margin(gtk_widget_get_style_context(zw->stack),
                                 GTK_STATE_FLAG_NORMAL, &stack_margin);

    int target = tab_margin.top + tab_h + tab_margin.bottom +
                 stack_margin.top + child_h + stack_margin.bottom + 2;

    int max_h = 820;
    GdkScreen *screen = gdk_screen_get_default();
    if (screen) {
        int sh = gdk_screen_get_height(screen);
        if (sh > 200) max_h = sh - 90;
    }
    if (target > max_h) target = max_h;
    /* Keep every expanded pill tall enough to look intentional even when a
       tab has little content; taller content still grows the drawer. */
    if (target < 360) target = 360;

    zw->height = target;
    gtk_widget_set_size_request(zw->box, zw->width, target);

    if (zw->is_visible && !zw->animating)
        animate_zone(zw, target);
    else if (zw->is_visible && zw->animating && zw->anim_to != target) {
        /* Re-target the running animation when hovering between tabs. */
        animate_zone(zw, target);
    }
}

void on_zone_stack_child_changed(GtkStack *stack, GParamSpec *pspec, gpointer user_data) {
    (void)pspec;
    ZoneWindow *zw = (ZoneWindow *)user_data;
    if (zw && zw->stack == GTK_WIDGET(stack))
        fit_zone_to_content(zw);
}

void show_zone(const char *name, int tab_idx) {
    ZoneWindow *zw = NULL;
    if (strcmp(name, "left") == 0) zw = &g_zone_left;
    else if (strcmp(name, "center") == 0) zw = &g_zone_center;
    else if (strcmp(name, "right") == 0) zw = &g_zone_right;
    if (!zw || !zw->window) return;

    if (zw->hide_timer_id > 0) {
        g_source_remove(zw->hide_timer_id);
        zw->hide_timer_id = 0;
    }

    if (tab_idx >= 0 && zw->tab_box && zw->stack) {
        GList *children = gtk_container_get_children(GTK_CONTAINER(zw->tab_box));
        int i = 0;
        for (GList *l = children; l != NULL; l = l->next, i++) {
            if (i == tab_idx) {
                g_signal_emit_by_name(l->data, "clicked");
                break;
            }
        }
        g_list_free(children);
    }

    /* Only one drawer is open at a time. */
    if (zw != &g_zone_left && (g_zone_left.is_visible || g_zone_left.animating)) hide_zone("left");
    if (zw != &g_zone_center && (g_zone_center.is_visible || g_zone_center.animating)) hide_zone("center");
    if (zw != &g_zone_right && (g_zone_right.is_visible || g_zone_right.animating)) hide_zone("right");

    /* Refresh before mapping: the data gathering is synchronous, so doing it
       here keeps the grow animation smooth instead of stalling mid-flight. */
    if (zw == &g_zone_left) refresh_left_zone();
    else if (zw == &g_zone_center) refresh_center_zone();
    else if (zw == &g_zone_right) refresh_right_zone();

    fit_zone_to_content(zw);

    if (!zw->is_visible) {
        zw->is_visible = 1;
        gtk_widget_set_opacity(zw->window, 1.0);

        if (!gtk_widget_get_visible(zw->window)) {
            /* Fallback for the unlikely case the warm-up map didn't happen. */
            zw->current_height = 1;
            gtk_widget_set_size_request(zw->container, zw->width, 1);
            gtk_widget_set_size_request(zw->clip, zw->width, 1);
            gtk_window_resize(GTK_WINDOW(zw->window), zw->width, 1);
            gtk_widget_show_all(zw->window);
        }
        animate_zone(zw, zw->height);
    }
}

void hide_zone(const char *name) {
    ZoneWindow *zw = NULL;
    if (strcmp(name, "left") == 0) zw = &g_zone_left;
    else if (strcmp(name, "center") == 0) zw = &g_zone_center;
    else if (strcmp(name, "right") == 0) zw = &g_zone_right;
    if (!zw || !zw->window) return;

    if (!zw->is_visible && !zw->animating) return;

    zw->is_visible = 0;
    if (gtk_widget_get_visible(zw->window)) animate_zone(zw, 0);
}

void toggle_zone(const char *name) {
    ZoneWindow *zw = NULL;
    if (strcmp(name, "left") == 0) zw = &g_zone_left;
    else if (strcmp(name, "center") == 0) zw = &g_zone_center;
    else if (strcmp(name, "right") == 0) zw = &g_zone_right;
    if (!zw || !zw->window) return;

    if (zw->is_visible) hide_zone(name);
    else show_zone(name, -1);
}

// Background IPC message dispatcher
static gboolean on_ipc_message(gpointer data) {
    char *cmd = (char *)data;
    if (!cmd) return FALSE;

    char action[32] = "", target[32] = "";
    int tab = -1;
    sscanf(cmd, "%31s %31s %d", action, target, &tab);

    if (strcmp(action, "show") == 0) {
        show_zone(target, tab);
    } else if (strcmp(action, "hide") == 0) {
        hide_zone(target);
    } else if (strcmp(action, "toggle") == 0) {
        toggle_zone(target);
    } else if (strcmp(action, "quit") == 0) {
        gtk_main_quit();
    }

    free(cmd);
    return FALSE;
}

// IPC server thread
static void *ipc_server_thread(void *arg) {
    unlink(SOCKET_PATH);
    s_server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (s_server_fd < 0) return NULL;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(s_server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(s_server_fd);
        return NULL;
    }
    listen(s_server_fd, 5);

    while (1) {
        int client_fd = accept(s_server_fd, NULL, NULL);
        if (client_fd < 0) break;

        char buf[256];
        ssize_t n = read(client_fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            g_idle_add(on_ipc_message, strdup(buf));
            write(client_fd, "OK\n", 3);
        }
        close(client_fd);
    }
    return NULL;
}

// Cursor monitor: automatic collapse only. Opening is done with the
// tiny drawer buttons in the bar.
static int panel_contains(ZoneWindow *zw, int cx, int cy, int screen_w) {
    if (!zw->is_visible && !zw->animating) return 0;

    int x;
    if (zw->anchor < 0) x = PANEL_SIDE_MARGIN;
    else if (zw->anchor > 0) x = screen_w - PANEL_SIDE_MARGIN - zw->width;
    else x = (screen_w - zw->width) / 2;

    int h = zw->current_height > 0 ? zw->current_height : zw->height;
    return (cx >= x - 8 && cx <= x + zw->width + 8 &&
            cy >= PANEL_HIT_TOP && cy <= PANEL_TOP_MARGIN + 40 + h + 8);
}

static void *cursor_monitor_thread(void *arg) {
    int screen_w = 1920;
    GdkScreen *screen = gdk_screen_get_default();
    if (screen) {
        int w = gdk_screen_get_width(screen);
        if (w > 640) screen_w = w;
    }

    int left_out_ticks = 0;
    int center_out_ticks = 0;
    int right_out_ticks = 0;

    while (1) {
        char *pos = run_shell_output("hyprctl cursorpos 2>/dev/null");
        if (pos && strlen(pos) > 2) {
            int cx = 0, cy = 0;
            if (sscanf(pos, "%d, %d", &cx, &cy) == 2) {
                // Bar zones (includes the drawer buttons at the end of each zone)
                int in_left_bar = (cy >= 4 && cy <= 42 && cx >= 10 && cx <= 360);
                int in_center_bar = (cy >= 4 && cy <= 42 && cx >= (screen_w / 2 - 150) && cx <= (screen_w / 2 + 150));
                int in_right_bar = (cy >= 4 && cy <= 42 && cx >= (screen_w - 370) && cx <= (screen_w - 8));

                int in_left_panel = panel_contains(&g_zone_left, cx, cy, screen_w);
                int in_center_panel = panel_contains(&g_zone_center, cx, cy, screen_w);
                int in_right_panel = panel_contains(&g_zone_right, cx, cy, screen_w);

                // --- AUTOMATIC COLLAPSE: cursor left both bar zone and panel ---
                if (s_auto_collapse) {
                    if (g_zone_left.is_visible) {
                        if (in_left_bar || in_left_panel) {
                            left_out_ticks = 0;
                        } else if (++left_out_ticks >= 5) {
                            left_out_ticks = 0;
                            g_idle_add(+[](gpointer u) -> gboolean {
                                hide_zone("left");
                                return FALSE;
                            }, NULL);
                        }
                    } else {
                        left_out_ticks = 0;
                    }

                    if (g_zone_center.is_visible) {
                        if (in_center_bar || in_center_panel) {
                            center_out_ticks = 0;
                        } else if (++center_out_ticks >= 5) {
                            center_out_ticks = 0;
                            g_idle_add(+[](gpointer u) -> gboolean {
                                hide_zone("center");
                                return FALSE;
                            }, NULL);
                        }
                    } else {
                        center_out_ticks = 0;
                    }

                    if (g_zone_right.is_visible) {
                        if (in_right_bar || in_right_panel) {
                            right_out_ticks = 0;
                        } else if (++right_out_ticks >= 5) {
                            right_out_ticks = 0;
                            g_idle_add(+[](gpointer u) -> gboolean {
                                hide_zone("right");
                                return FALSE;
                            }, NULL);
                        }
                    } else {
                        right_out_ticks = 0;
                    }
                } else {
                    left_out_ticks = center_out_ticks = right_out_ticks = 0;
                }
            }
            free(pos);
        }
        usleep(50000); // 50ms polling interval
    }
    return NULL;
}

// Client helper
static int send_client_command(int argc, char **argv) {
    int fd = -1;
    struct sockaddr_un addr;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    /* Retry briefly so a command issued just as the daemon starts does not
       mistake a still-booting server for a missing one and spawn a duplicate. */
    for (int attempt = 0; attempt < 40; attempt++) {
        fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd < 0) return 1;
        if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == 0)
            break;
        close(fd);
        fd = -1;
        usleep(50000);
    }
    if (fd < 0) return 1; // Server not running

    char msg[256] = "";
    for (int i = 1; i < argc; i++) {
        strcat(msg, argv[i]);
        if (i < argc - 1) strcat(msg, " ");
    }
    write(fd, msg, strlen(msg));
    close(fd);
    return 0;
}

int main(int argc, char **argv) {
    if (getenv("ZONE_PANEL_NO_AUTOCOLLAPSE")) s_auto_collapse = 0;

    /* Single-instance gate: the process that owns this lock is the only one
       allowed to create layer surfaces and the IPC server. */
    int lock_fd = open("/tmp/waybar_zone_panel.lock", O_CREAT | O_RDWR, 0600);
    int is_primary = (lock_fd >= 0 && flock(lock_fd, LOCK_EX | LOCK_NB) == 0);

    if (argc > 1) {
        // If client command succeeded, exit
        if (send_client_command(argc, argv) == 0) {
            return 0;
        }
        // A live instance holds the lock but did not answer; don't duplicate it.
        if (!is_primary) {
            return 0;
        }
        // If server wasn't running, daemonize and process arg
        if (fork() != 0) {
            // Parent waits briefly then sends command
            usleep(400000);
            send_client_command(argc, argv);
            return 0;
        }
    } else if (!is_primary) {
        return 0; // another zone-panel is already running
    }
    /* lock_fd intentionally stays open for the lifetime of the process */

    gtk_init(&argc, &argv);
    apply_panel_styles();

    build_left_zone();
    build_center_zone();
    build_right_zone();

    /* Keep the layer surfaces mapped so opening has no map latency. */
    warm_up_zone(&g_zone_left);
    warm_up_zone(&g_zone_center);
    warm_up_zone(&g_zone_right);

    // Start IPC thread
    pthread_t ipc_tid;
    pthread_create(&ipc_tid, NULL, ipc_server_thread, NULL);
    pthread_detach(ipc_tid);

    // Start cursor monitor thread
    pthread_t cur_tid;
    pthread_create(&cur_tid, NULL, cursor_monitor_thread, NULL);
    pthread_detach(cur_tid);

    // Periodic refresh timer (4 seconds, paused while a drawer animates)
    g_timeout_add(4000, +[](gpointer u) -> gboolean {
        if (!g_zone_left.animating && g_zone_left.is_visible) refresh_left_zone();
        if (!g_zone_center.animating && g_zone_center.is_visible) refresh_center_zone();
        if (!g_zone_right.animating && g_zone_right.is_visible) refresh_right_zone();
        return TRUE;
    }, NULL);

    gtk_main();

    unlink(SOCKET_PATH);
    return 0;
}
