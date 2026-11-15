#include "panel_common.h"
#include <fcntl.h>

ZoneWindow g_zone_left = {0};
ZoneWindow g_zone_center = {0};
ZoneWindow g_zone_right = {0};

static int s_server_fd = -1;

GtkWidget *create_tab_button(const char *label, GtkWidget *stack, const char *child_name, int tab_idx, int *active_tab_ptr) {
    GtkWidget *btn = gtk_button_new_with_label(label);
    gtk_style_context_add_class(gtk_widget_get_style_context(btn), "tab-btn");
    if (*active_tab_ptr == tab_idx) {
        gtk_style_context_add_class(gtk_widget_get_style_context(btn), "active");
    }

    g_object_set_data_full(G_OBJECT(btn), "child-name", strdup(child_name), free);
    g_object_set_data(G_OBJECT(btn), "stack", stack);
    g_object_set_data(G_OBJECT(btn), "tab-idx", GINT_TO_POINTER(tab_idx));
    g_object_set_data(G_OBJECT(btn), "active-ptr", active_tab_ptr);

    g_signal_connect(btn, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer u) {
        GtkWidget *stk = GTK_WIDGET(g_object_get_data(G_OBJECT(b), "stack"));
        const char *name = (const char *)g_object_get_data(G_OBJECT(b), "child-name");
        int idx = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(b), "tab-idx"));
        int *ptr = (int *)g_object_get_data(G_OBJECT(b), "active-ptr");

        gtk_stack_set_visible_child_name(GTK_STACK(stk), name);
        if (ptr) *ptr = idx;

        // Update button visual active classes
        GtkWidget *parent = gtk_widget_get_parent(GTK_WIDGET(b));
        GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
        for (GList *l = children; l != NULL; l = l->next) {
            gtk_style_context_remove_class(gtk_widget_get_style_context(GTK_WIDGET(l->data)), "active");
        }
        g_list_free(children);
        gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(b)), "active");
    }), NULL);

    return btn;
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

    if (!zw->is_visible) {
        gtk_widget_show_all(zw->window);
        zw->is_visible = 1;
    }

    if (zw == &g_zone_left) refresh_left_zone();
    else if (zw == &g_zone_center) refresh_center_zone();
    else if (zw == &g_zone_right) refresh_right_zone();
}

void hide_zone(const char *name) {
    ZoneWindow *zw = NULL;
    if (strcmp(name, "left") == 0) zw = &g_zone_left;
    else if (strcmp(name, "center") == 0) zw = &g_zone_center;
    else if (strcmp(name, "right") == 0) zw = &g_zone_right;
    if (!zw || !zw->window) return;

    if (zw->is_visible) {
        gtk_widget_hide(zw->window);
        zw->is_visible = 0;
    }
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

// Cursor monitor thread for hover expansion & automatic collapse
static void *cursor_monitor_thread(void *arg) {
    int screen_w = 1920;
    GdkScreen *screen = gdk_screen_get_default();
    if (screen) {
        int w = gdk_screen_get_width(screen);
        if (w > 640) screen_w = w;
    }

    int left_hover_ticks = 0;
    int center_hover_ticks = 0;
    int right_hover_ticks = 0;

    int left_out_ticks = 0;
    int center_out_ticks = 0;
    int right_out_ticks = 0;

    while (1) {
        char *pos = run_shell_output("hyprctl cursorpos 2>/dev/null");
        if (pos && strlen(pos) > 2) {
            int cx = 0, cy = 0;
            if (sscanf(pos, "%d, %d", &cx, &cy) == 2) {
                // Pill bounds: only trigger expansion when "very inside"
                int in_left_pill = (cy >= 10 && cy <= 34 && cx >= 24 && cx <= 280);
                int in_center_pill = (cy >= 10 && cy <= 34 && cx >= (screen_w / 2 - 75) && cx <= (screen_w / 2 + 75));
                int in_right_pill = (cy >= 10 && cy <= 34 && cx >= (screen_w - 350) && cx <= (screen_w - 25));

                // Panel bounds (when open)
                int in_left_panel = (g_zone_left.is_visible && cy >= 36 && cy <= 525 && cx >= 10 && cx <= 460);
                int in_center_panel = (g_zone_center.is_visible && cy >= 36 && cy <= 545 && cx >= (screen_w / 2 - 230) && cx <= (screen_w / 2 + 230));
                int in_right_panel = (g_zone_right.is_visible && cy >= 36 && cy <= 565 && cx >= (screen_w - 480) && cx <= (screen_w - 10));

                // --- EXPANSION: Trigger only when stable inside pill for 2 ticks (~100ms) ---
                if (in_left_pill) {
                    left_hover_ticks++;
                    if (left_hover_ticks == 2 && !g_zone_left.is_visible) {
                        g_idle_add(+[](gpointer u) -> gboolean {
                            show_zone("left", -1);
                            return FALSE;
                        }, NULL);
                    }
                } else {
                    left_hover_ticks = 0;
                }

                if (in_center_pill) {
                    center_hover_ticks++;
                    if (center_hover_ticks == 2 && !g_zone_center.is_visible) {
                        g_idle_add(+[](gpointer u) -> gboolean {
                            show_zone("center", -1);
                            return FALSE;
                        }, NULL);
                    }
                } else {
                    center_hover_ticks = 0;
                }

                if (in_right_pill) {
                    right_hover_ticks++;
                    if (right_hover_ticks == 2 && !g_zone_right.is_visible) {
                        g_idle_add(+[](gpointer u) -> gboolean {
                            show_zone("right", -1);
                            return FALSE;
                        }, NULL);
                    }
                } else {
                    right_hover_ticks = 0;
                }

                // --- AUTOMATIC COLLAPSE: When cursor is out of both pill and panel ---
                if (g_zone_left.is_visible) {
                    int inside_left = (cy >= 6 && cy <= 38 && cx >= 12 && cx <= 320) || in_left_panel;
                    if (!inside_left) {
                        left_out_ticks++;
                        if (left_out_ticks >= 3) {
                            g_idle_add(+[](gpointer u) -> gboolean {
                                hide_zone("left");
                                return FALSE;
                            }, NULL);
                            left_out_ticks = 0;
                        }
                    } else {
                        left_out_ticks = 0;
                    }
                }

                if (g_zone_center.is_visible) {
                    int inside_center = (cy >= 6 && cy <= 38 && cx >= (screen_w / 2 - 100) && cx <= (screen_w / 2 + 100)) || in_center_panel;
                    if (!inside_center) {
                        center_out_ticks++;
                        if (center_out_ticks >= 3) {
                            g_idle_add(+[](gpointer u) -> gboolean {
                                hide_zone("center");
                                return FALSE;
                            }, NULL);
                            center_out_ticks = 0;
                        }
                    } else {
                        center_out_ticks = 0;
                    }
                }

                if (g_zone_right.is_visible) {
                    int inside_right = (cy >= 6 && cy <= 38 && cx >= (screen_w - 380) && cx <= (screen_w - 10)) || in_right_panel;
                    if (!inside_right) {
                        right_out_ticks++;
                        if (right_out_ticks >= 3) {
                            g_idle_add(+[](gpointer u) -> gboolean {
                                hide_zone("right");
                                return FALSE;
                            }, NULL);
                            right_out_ticks = 0;
                        }
                    } else {
                        right_out_ticks = 0;
                    }
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
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return 1;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return 1; // Server not running
    }

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
    if (argc > 1) {
        // If client command succeeded, exit
        if (send_client_command(argc, argv) == 0) {
            return 0;
        }
        // If server wasn't running, daemonize and process arg
        if (fork() != 0) {
            // Parent waits briefly then sends command
            usleep(200000);
            send_client_command(argc, argv);
            return 0;
        }
    }

    gtk_init(&argc, &argv);
    apply_panel_styles();

    build_left_zone();
    build_center_zone();
    build_right_zone();

    // Start IPC thread
    pthread_t ipc_tid;
    pthread_create(&ipc_tid, NULL, ipc_server_thread, NULL);
    pthread_detach(ipc_tid);

    // Start cursor monitor thread
    pthread_t cur_tid;
    pthread_create(&cur_tid, NULL, cursor_monitor_thread, NULL);
    pthread_detach(cur_tid);

    // Periodic refresh timer (1.5 seconds)
    g_timeout_add(1500, +[](gpointer u) -> gboolean {
        if (g_zone_left.is_visible) refresh_left_zone();
        if (g_zone_center.is_visible) refresh_center_zone();
        if (g_zone_right.is_visible) refresh_right_zone();
        return TRUE;
    }, NULL);

    gtk_main();

    unlink(SOCKET_PATH);
    return 0;
}
