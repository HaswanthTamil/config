#include "panel_common.h"

// Forward declarations
static void on_workspace_clicked(GtkButton *btn, gpointer user_data);
static void on_window_focus_clicked(GtkButton *btn, gpointer user_data);
static void on_media_play_pause(GtkButton *btn, gpointer user_data);
static void on_media_next(GtkButton *btn, gpointer user_data);
static void on_media_prev(GtkButton *btn, gpointer user_data);
static void on_media_seek_back(GtkButton *btn, gpointer user_data);
static void on_media_seek_fwd(GtkButton *btn, gpointer user_data);
static void on_visualizer_toggled(GtkSwitch *sw, gboolean state, gpointer user_data);
static void on_visualizer_bars_changed(GtkComboBoxText *combo, gpointer user_data);
static void on_visualizer_sens_changed(GtkRange *range, gpointer user_data);
static gboolean on_visualizer_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);

static GtkWidget *s_workspaces_container = NULL;
static GtkWidget *s_media_title = NULL;
static GtkWidget *s_media_artist = NULL;
static GtkWidget *s_media_status = NULL;
static GtkWidget *s_media_progress = NULL;
static GtkWidget *s_media_play_btn = NULL;
static GtkWidget *s_vis_switch = NULL;
static GtkWidget *s_vis_draw = NULL;
static GtkWidget *s_vis_sens_scale = NULL;

void build_left_zone(void) {
    g_zone_left.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_name(g_zone_left.window, "panel-left");
    gtk_style_context_add_class(gtk_widget_get_style_context(g_zone_left.window), "panel-window");

    // Configure Layer Shell
    gtk_layer_init_for_window(GTK_WINDOW(g_zone_left.window));
    gtk_layer_set_layer(GTK_WINDOW(g_zone_left.window), GTK_LAYER_SHELL_LAYER_TOP);
    gtk_layer_set_anchor(GTK_WINDOW(g_zone_left.window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(g_zone_left.window), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
    gtk_layer_set_margin(GTK_WINDOW(g_zone_left.window), GTK_LAYER_SHELL_EDGE_TOP, 37);
    gtk_layer_set_margin(GTK_WINDOW(g_zone_left.window), GTK_LAYER_SHELL_EDGE_LEFT, 12);
    gtk_layer_set_keyboard_mode(GTK_WINDOW(g_zone_left.window), GTK_LAYER_SHELL_KEYBOARD_MODE_NONE);

    // Outer container
    GtkWidget *container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(container), "panel-container");
    gtk_widget_set_size_request(container, 440, 480);
    gtk_container_add(GTK_CONTAINER(g_zone_left.window), container);

    // Tab bar
    g_zone_left.tab_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_style_context_add_class(gtk_widget_get_style_context(g_zone_left.tab_box), "tab-bar");
    gtk_box_pack_start(GTK_BOX(container), g_zone_left.tab_box, FALSE, FALSE, 0);

    // Stack for face cards
    g_zone_left.stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(g_zone_left.stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_duration(GTK_STACK(g_zone_left.stack), 150);
    gtk_box_pack_start(GTK_BOX(container), g_zone_left.stack, TRUE, TRUE, 0);

    // Tab buttons
    GtkWidget *btn1 = create_tab_button("  Workspaces", g_zone_left.stack, "workspaces", 0, &g_zone_left.active_tab);
    GtkWidget *btn2 = create_tab_button("  Now Playing", g_zone_left.stack, "media", 1, &g_zone_left.active_tab);
    GtkWidget *btn3 = create_tab_button("󰓃  Visualizer", g_zone_left.stack, "visualizer", 2, &g_zone_left.active_tab);
    gtk_box_pack_start(GTK_BOX(g_zone_left.tab_box), btn1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(g_zone_left.tab_box), btn2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(g_zone_left.tab_box), btn3, TRUE, TRUE, 0);

    /* -------------------------------------------------------------
       Tab 1: Workspaces Face Card
       ------------------------------------------------------------- */
    GtkWidget *card1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(card1), "face-card");
    
    GtkWidget *w1_title = gtk_label_new("Workspace Overview & Windows");
    gtk_widget_set_halign(w1_title, GTK_ALIGN_START);
    gtk_style_context_add_class(gtk_widget_get_style_context(w1_title), "card-title");
    gtk_box_pack_start(GTK_BOX(card1), w1_title, FALSE, FALSE, 0);

    GtkWidget *scroll1 = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll1), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(card1), scroll1, TRUE, TRUE, 0);

    s_workspaces_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_container_add(GTK_CONTAINER(scroll1), s_workspaces_container);

    gtk_stack_add_named(GTK_STACK(g_zone_left.stack), card1, "workspaces");

    /* -------------------------------------------------------------
       Tab 2: Now Playing Face Card
       ------------------------------------------------------------- */
    GtkWidget *card2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_style_context_add_class(gtk_widget_get_style_context(card2), "face-card");

    GtkWidget *m_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(m_card), "card-section");
    gtk_box_pack_start(GTK_BOX(card2), m_card, FALSE, FALSE, 0);

    s_media_title = gtk_label_new("No media playing");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_media_title), "card-title");
    gtk_label_set_line_wrap(GTK_LABEL(s_media_title), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(s_media_title), 36);
    gtk_box_pack_start(GTK_BOX(m_card), s_media_title, FALSE, FALSE, 0);

    s_media_artist = gtk_label_new("Start playback in Spotify, YouTube or local player");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_media_artist), "card-subtitle");
    gtk_box_pack_start(GTK_BOX(m_card), s_media_artist, FALSE, FALSE, 0);

    s_media_status = gtk_label_new("Status: Stopped");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_media_status), "card-subtitle");
    gtk_box_pack_start(GTK_BOX(m_card), s_media_status, FALSE, FALSE, 0);

    s_media_progress = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(s_media_progress), 0.0);
    gtk_box_pack_start(GTK_BOX(m_card), s_media_progress, FALSE, FALSE, 4);

    // Playback controls
    GtkWidget *ctrl_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(ctrl_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(m_card), ctrl_box, FALSE, FALSE, 6);

    GtkWidget *btn_seek_back = gtk_button_new_with_label("« -5s");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_seek_back), "action-btn");
    g_signal_connect(btn_seek_back, "clicked", G_CALLBACK(on_media_seek_back), NULL);
    gtk_box_pack_start(GTK_BOX(ctrl_box), btn_seek_back, FALSE, FALSE, 0);

    GtkWidget *btn_prev = gtk_button_new_with_label("⏮ Prev");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_prev), "action-btn");
    g_signal_connect(btn_prev, "clicked", G_CALLBACK(on_media_prev), NULL);
    gtk_box_pack_start(GTK_BOX(ctrl_box), btn_prev, FALSE, FALSE, 0);

    s_media_play_btn = gtk_button_new_with_label("⏵ Play");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_media_play_btn), "action-btn");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_media_play_btn), "primary");
    g_signal_connect(s_media_play_btn, "clicked", G_CALLBACK(on_media_play_pause), NULL);
    gtk_box_pack_start(GTK_BOX(ctrl_box), s_media_play_btn, FALSE, FALSE, 0);

    GtkWidget *btn_next = gtk_button_new_with_label("Next ⏭");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_next), "action-btn");
    g_signal_connect(btn_next, "clicked", G_CALLBACK(on_media_next), NULL);
    gtk_box_pack_start(GTK_BOX(ctrl_box), btn_next, FALSE, FALSE, 0);

    GtkWidget *btn_seek_fwd = gtk_button_new_with_label("+5s »");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_seek_fwd), "action-btn");
    g_signal_connect(btn_seek_fwd, "clicked", G_CALLBACK(on_media_seek_fwd), NULL);
    gtk_box_pack_start(GTK_BOX(ctrl_box), btn_seek_fwd, FALSE, FALSE, 0);

    gtk_stack_add_named(GTK_STACK(g_zone_left.stack), card2, "media");

    /* -------------------------------------------------------------
       Tab 3: Beat Visualizer Face Card
       ------------------------------------------------------------- */
    GtkWidget *card3 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(card3), "face-card");

    GtkWidget *vis_header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(card3), vis_header, FALSE, FALSE, 0);

    GtkWidget *vis_title = gtk_label_new("PipeWire Audio-Reactive Visualizer");
    gtk_style_context_add_class(gtk_widget_get_style_context(vis_title), "card-title");
    gtk_box_pack_start(GTK_BOX(vis_header), vis_title, FALSE, FALSE, 0);

    s_vis_switch = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(s_vis_switch), TRUE);
    g_signal_connect(s_vis_switch, "state-set", G_CALLBACK(on_visualizer_toggled), NULL);
    gtk_box_pack_end(GTK_BOX(vis_header), s_vis_switch, FALSE, FALSE, 0);

    // Real-time animated canvas
    s_vis_draw = gtk_drawing_area_new();
    gtk_widget_set_size_request(s_vis_draw, 400, 160);
    gtk_style_context_add_class(gtk_widget_get_style_context(s_vis_draw), "card-section");
    g_signal_connect(s_vis_draw, "draw", G_CALLBACK(on_visualizer_draw), NULL);
    gtk_box_pack_start(GTK_BOX(card3), s_vis_draw, TRUE, TRUE, 0);

    // Controls
    GtkWidget *vis_opts = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_box_pack_start(GTK_BOX(card3), vis_opts, FALSE, FALSE, 4);

    GtkWidget *lbl_bars = gtk_label_new("Bars:");
    gtk_box_pack_start(GTK_BOX(vis_opts), lbl_bars, FALSE, FALSE, 0);
    GtkWidget *combo_bars = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_bars), "8 Bars");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_bars), "16 Bars");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_bars), "24 Bars");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_bars), 0);
    g_signal_connect(combo_bars, "changed", G_CALLBACK(on_visualizer_bars_changed), NULL);
    gtk_box_pack_start(GTK_BOX(vis_opts), combo_bars, FALSE, FALSE, 0);

    GtkWidget *lbl_sens = gtk_label_new("Sensitivity:");
    gtk_box_pack_start(GTK_BOX(vis_opts), lbl_sens, FALSE, FALSE, 0);
    s_vis_sens_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.5, 3.0, 0.1);
    gtk_range_set_value(GTK_RANGE(s_vis_sens_scale), 1.0);
    g_signal_connect(s_vis_sens_scale, "value-changed", G_CALLBACK(on_visualizer_sens_changed), NULL);
    gtk_box_pack_start(GTK_BOX(vis_opts), s_vis_sens_scale, TRUE, TRUE, 0);

    gtk_stack_add_named(GTK_STACK(g_zone_left.stack), card3, "visualizer");

    // Connect mouse enter/leave
    gtk_widget_add_events(g_zone_left.window, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
    g_signal_connect(g_zone_left.window, "enter-notify-event", G_CALLBACK(+[](GtkWidget *w, GdkEventCrossing *e, gpointer u) -> gboolean {
        if (g_zone_left.hide_timer_id > 0) {
            g_source_remove(g_zone_left.hide_timer_id);
            g_zone_left.hide_timer_id = 0;
        }
        return FALSE;
    }), NULL);
    g_signal_connect(g_zone_left.window, "leave-notify-event", G_CALLBACK(+[](GtkWidget *w, GdkEventCrossing *e, gpointer u) -> gboolean {
        if (e->detail != GDK_NOTIFY_INFERIOR) {
            if (g_zone_left.hide_timer_id > 0) g_source_remove(g_zone_left.hide_timer_id);
            g_zone_left.hide_timer_id = g_timeout_add(350, +[](gpointer data) -> gboolean {
                hide_zone("left");
                g_zone_left.hide_timer_id = 0;
                return FALSE;
            }, NULL);
        }
        return FALSE;
    }), NULL);

    refresh_left_zone();
}

void refresh_left_zone(void) {
    if (!s_workspaces_container) return;

    // 1. Refresh Workspaces & Windows from Hyprland
    GList *children = gtk_container_get_children(GTK_CONTAINER(s_workspaces_container));
    for (GList *l = children; l != NULL; l = l->next) {
        gtk_widget_destroy(GTK_WIDGET(l->data));
    }
    g_list_free(children);

    char *active_ws = run_shell_output("hyprctl activeworkspace -j 2>/dev/null | grep '\"id\":' | head -n 1 | awk '{print $2}' | tr -d ','");
    int active_id = active_ws ? atoi(active_ws) : 1;
    free(active_ws);

    char *clients_json = run_shell_output("hyprctl clients -j 2>/dev/null");

    for (int ws = 1; ws <= 5; ws++) {
        GtkWidget *row = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
        gtk_style_context_add_class(gtk_widget_get_style_context(row), "card-section");

        GtkWidget *h = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        gtk_box_pack_start(GTK_BOX(row), h, FALSE, FALSE, 0);

        char ws_label[32];
        snprintf(ws_label, sizeof(ws_label), "Workspace %d", ws);
        GtkWidget *lbl = gtk_label_new(ws_label);
        gtk_style_context_add_class(gtk_widget_get_style_context(lbl), "card-title");
        gtk_box_pack_start(GTK_BOX(h), lbl, FALSE, FALSE, 0);

        if (ws == active_id) {
            GtkWidget *badge = gtk_label_new("ACTIVE");
            gtk_style_context_add_class(gtk_widget_get_style_context(badge), "card-subtitle");
            gtk_box_pack_start(GTK_BOX(h), badge, FALSE, FALSE, 4);
        }

        GtkWidget *btn_switch = gtk_button_new_with_label("Switch");
        gtk_style_context_add_class(gtk_widget_get_style_context(btn_switch), "action-btn");
        g_object_set_data(G_OBJECT(btn_switch), "ws-id", GINT_TO_POINTER(ws));
        g_signal_connect(btn_switch, "clicked", G_CALLBACK(on_workspace_clicked), NULL);
        gtk_box_pack_end(GTK_BOX(h), btn_switch, FALSE, FALSE, 0);

        // Windows list in workspace
        GtkWidget *win_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_box_pack_start(GTK_BOX(row), win_box, FALSE, FALSE, 0);

        char grep_cmd[128];
        snprintf(grep_cmd, sizeof(grep_cmd),
                 "hyprctl clients -j 2>/dev/null | grep -B2 -A10 '\"workspace\": {\"id\": %d' | grep '\"title\":' | cut -d'\"' -f4", ws);
        char *win_titles = run_shell_output(grep_cmd);
        if (win_titles && strlen(win_titles) > 1) {
            char *line = strtok(win_titles, "\n");
            while (line) {
                GtkWidget *wbtn = gtk_button_new_with_label(line);
                gtk_style_context_add_class(gtk_widget_get_style_context(wbtn), "action-btn");
                gtk_widget_set_halign(wbtn, GTK_ALIGN_START);
                g_signal_connect(wbtn, "clicked", G_CALLBACK(on_window_focus_clicked), NULL);
                gtk_box_pack_start(GTK_BOX(win_box), wbtn, FALSE, FALSE, 1);
                line = strtok(NULL, "\n");
            }
        } else {
            GtkWidget *empty_lbl = gtk_label_new("Empty workspace");
            gtk_style_context_add_class(gtk_widget_get_style_context(empty_lbl), "card-subtitle");
            gtk_widget_set_halign(empty_lbl, GTK_ALIGN_START);
            gtk_box_pack_start(GTK_BOX(win_box), empty_lbl, FALSE, FALSE, 0);
        }
        free(win_titles);

        gtk_box_pack_start(GTK_BOX(s_workspaces_container), row, FALSE, FALSE, 0);
    }
    free(clients_json);
    gtk_widget_show_all(s_workspaces_container);

    // 2. Refresh Media info
    char *title = run_shell_output("playerctl metadata title 2>/dev/null");
    char *artist = run_shell_output("playerctl metadata artist 2>/dev/null");
    char *status = run_shell_output("playerctl status 2>/dev/null");

    if (title && strlen(title) > 0) {
        gtk_label_set_text(GTK_LABEL(s_media_title), title);
        gtk_label_set_text(GTK_LABEL(s_media_artist), (artist && strlen(artist) > 0) ? artist : "Unknown Artist");
        char st_buf[64];
        snprintf(st_buf, sizeof(st_buf), "Status: %s", (status && strlen(status) > 0) ? status : "Playing");
        gtk_label_set_text(GTK_LABEL(s_media_status), st_buf);
        if (status && strstr(status, "Playing")) {
            gtk_button_set_label(GTK_BUTTON(s_media_play_btn), "⏸ Pause");
        } else {
            gtk_button_set_label(GTK_BUTTON(s_media_play_btn), "⏵ Play");
        }
    } else {
        gtk_label_set_text(GTK_LABEL(s_media_title), "No media player active");
        gtk_label_set_text(GTK_LABEL(s_media_artist), "Launch Spotify, YouTube or an MPRIS player");
        gtk_label_set_text(GTK_LABEL(s_media_status), "Status: Stopped");
        gtk_button_set_label(GTK_BUTTON(s_media_play_btn), "⏵ Play");
    }
    free(title);
    free(artist);
    free(status);

    if (s_vis_draw) {
        gtk_widget_queue_draw(s_vis_draw);
    }
}

static void on_workspace_clicked(GtkButton *btn, gpointer user_data) {
    int ws = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "ws-id"));
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "hyprctl dispatch workspace %d", ws);
    run_shell_command(cmd);
    refresh_left_zone();
}

static void on_window_focus_clicked(GtkButton *btn, gpointer user_data) {
    const char *title = gtk_button_get_label(btn);
    if (!title) return;
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "hyprctl dispatch focuswindow title:\"%s\"", title);
    run_shell_command(cmd);
}

static void on_media_play_pause(GtkButton *btn, gpointer user_data) {
    run_shell_command("playerctl play-pause 2>/dev/null");
    refresh_left_zone();
}

static void on_media_next(GtkButton *btn, gpointer user_data) {
    run_shell_command("playerctl next 2>/dev/null");
    refresh_left_zone();
}

static void on_media_prev(GtkButton *btn, gpointer user_data) {
    run_shell_command("playerctl previous 2>/dev/null");
    refresh_left_zone();
}

static void on_media_seek_back(GtkButton *btn, gpointer user_data) {
    run_shell_command("playerctl position 5- 2>/dev/null");
}

static void on_media_seek_fwd(GtkButton *btn, gpointer user_data) {
    run_shell_command("playerctl position 5+ 2>/dev/null");
}

static void on_visualizer_toggled(GtkSwitch *sw, gboolean state, gpointer user_data) {
    run_shell_command("~/.config/waybar/scripts/audio/visualizer_toggle.sh");
}

static void on_visualizer_bars_changed(GtkComboBoxText *combo, gpointer user_data) {
    const char *text = gtk_combo_box_text_get_active_text(combo);
    if (!text) return;
    int b = 8;
    if (strstr(text, "16")) b = 16;
    else if (strstr(text, "24")) b = 24;
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "python3 -c \"import json; f='$HOME/.config/waybar/state/visualizer.json'; d=json.load(open(f)); d['bars']=%d; json.dump(d,open(f,'w'),indent=2)\"", b);
    run_shell_command(cmd);
}

static void on_visualizer_sens_changed(GtkRange *range, gpointer user_data) {
    double v = gtk_range_get_value(range);
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "python3 -c \"import json; f='$HOME/.config/waybar/state/visualizer.json'; d=json.load(open(f)); d['sensitivity']=%.2f; json.dump(d,open(f,'w'),indent=2)\"", v);
    run_shell_command(cmd);
}

static gboolean on_visualizer_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    GtkAllocation alloc;
    gtk_widget_get_allocation(widget, &alloc);

    // Draw background
    cairo_set_source_rgba(cr, 0.08, 0.09, 0.11, 0.95);
    cairo_paint(cr);

    // Read real visualizer data
    FILE *fp = fopen("/tmp/waybar_visualizer.txt", "r");
    char bars[64] = "········";
    if (fp) {
        if (fgets(bars, sizeof(bars), fp) == NULL) {
            strcpy(bars, "········");
        }
        fclose(fp);
    }

    size_t count = strlen(bars);
    if (count == 0) return FALSE;

    double bar_w = ((double)alloc.width - 20) / (double)count;
    cairo_set_source_rgba(cr, 0.48, 0.55, 0.62, 0.9); // Cool blue-grey

    for (size_t i = 0; i < count; i++) {
        double frac = 0.1;
        unsigned char c = (unsigned char)bars[i];
        if (c >= 0xE2) { // UTF-8 block char
            frac = 0.5;
        } else if (c == ' ') {
            frac = 0.05;
        } else {
            frac = 0.25;
        }
        double h = frac * (alloc.height - 30);
        double x = 10 + i * bar_w;
        double y = alloc.height - 15 - h;

        cairo_rectangle(cr, x + 2, y, bar_w - 4, h);
        cairo_fill(cr);
    }

    return FALSE;
}
