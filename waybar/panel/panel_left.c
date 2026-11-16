#include "panel_common.h"

// Forward declarations
static void on_workspace_clicked(GtkButton *btn, gpointer user_data);
static void on_media_play_pause(GtkButton *btn, gpointer user_data);
static void on_media_next(GtkButton *btn, gpointer user_data);
static void on_media_prev(GtkButton *btn, gpointer user_data);
static gboolean on_visualizer_toggled(GtkSwitch *sw, gboolean state, gpointer user_data);
static void on_visualizer_sens_changed(GtkRange *range, gpointer user_data);
static gboolean on_visualizer_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);

static GtkWidget *s_workspaces_container = NULL;
static GtkWidget *s_media_title = NULL;
static GtkWidget *s_media_artist = NULL;
static GtkWidget *s_media_play_btn = NULL;
static GtkWidget *s_vis_switch = NULL;
static GtkWidget *s_vis_draw = NULL;
static GtkWidget *s_vis_sens_scale = NULL;
static gboolean s_vis_syncing = FALSE;

static const char *tile_icon_for_class(const char *cls) {
    if (!cls) return "󰖯";
    if (strcasestr(cls, "kitty") || strcasestr(cls, "alacritty") ||
        strcasestr(cls, "foot") || strcasestr(cls, "wezterm") ||
        strcasestr(cls, "konsole"))
        return "󰆍";
    if (strcasestr(cls, "brave") || strcasestr(cls, "firefox") ||
        strcasestr(cls, "chromium") || strcasestr(cls, "chrome"))
        return "󰖟";
    if (strcasestr(cls, "code") || strcasestr(cls, "cursor"))
        return "󰨞";
    if (strcasestr(cls, "spotify") || strcasestr(cls, "music"))
        return "󰓇";
    if (strcasestr(cls, "discord") || strcasestr(cls, "telegram") ||
        strcasestr(cls, "slack"))
        return "󰙯";
    if (strcasestr(cls, "mpv") || strcasestr(cls, "vlc"))
        return "󰕼";
    if (strcasestr(cls, "nautilus") || strcasestr(cls, "thunar") ||
        strcasestr(cls, "dolphin") || strcasestr(cls, "file"))
        return "󰉋";
    if (strcasestr(cls, "gimp") || strcasestr(cls, "inkscape"))
        return "󰏘";
    return "󰖯";
}

void build_left_zone(void) {
    GtkWidget *content = create_panel_shell(&g_zone_left, "panel-left", 420, 440, -1);

    // Tab bar matching the bar's inner capsules
    g_zone_left.tab_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_style_context_add_class(gtk_widget_get_style_context(g_zone_left.tab_box), "tab-bar");
    gtk_box_pack_start(GTK_BOX(content), g_zone_left.tab_box, FALSE, FALSE, 0);

    // Stack for face cards
    g_zone_left.stack = gtk_stack_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(g_zone_left.stack), "panel-stack");
    gtk_stack_set_transition_type(GTK_STACK(g_zone_left.stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_duration(GTK_STACK(g_zone_left.stack), 180);
    gtk_box_pack_start(GTK_BOX(content), g_zone_left.stack, TRUE, TRUE, 0);
    g_signal_connect(g_zone_left.stack, "notify::visible-child", G_CALLBACK(on_zone_stack_child_changed), &g_zone_left);

    g_zone_left.active_tab = 0;

    GtkWidget *b0 = create_icon_tab_button("󰕰", "Workspaces", g_zone_left.stack, "workspaces", 0, &g_zone_left.active_tab);
    GtkWidget *b1 = create_icon_tab_button("", "Now Playing", g_zone_left.stack, "media", 1, &g_zone_left.active_tab);

    gtk_box_pack_start(GTK_BOX(g_zone_left.tab_box), b0, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(g_zone_left.tab_box), b1, FALSE, FALSE, 0);

    /* -------------------------------------------------------------
       Tab 0: Workspaces — 5 x 2 grid
       ------------------------------------------------------------- */
    GtkWidget *card0 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(card0), "face-card");

    GtkWidget *w0_hdr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(card0), w0_hdr, FALSE, FALSE, 0);
    GtkWidget *w0_title = gtk_label_new("Workspaces");
    gtk_style_context_add_class(gtk_widget_get_style_context(w0_title), "card-header-title");
    gtk_box_pack_start(GTK_BOX(w0_hdr), w0_title, FALSE, FALSE, 0);

    GtkWidget *w0_hint = gtk_label_new("Click to switch");
    gtk_style_context_add_class(gtk_widget_get_style_context(w0_hint), "item-subtitle");
    gtk_box_pack_end(GTK_BOX(w0_hdr), w0_hint, FALSE, FALSE, 0);

    s_workspaces_container = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(s_workspaces_container), 8);
    gtk_grid_set_column_spacing(GTK_GRID(s_workspaces_container), 8);
    gtk_grid_set_row_homogeneous(GTK_GRID(s_workspaces_container), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(s_workspaces_container), TRUE);
    gtk_box_pack_start(GTK_BOX(card0), s_workspaces_container, FALSE, FALSE, 4);

    gtk_stack_add_named(GTK_STACK(g_zone_left.stack), card0, "workspaces");

    /* -------------------------------------------------------------
       Tab 1: Now Playing + beat visualizer
       ------------------------------------------------------------- */
    GtkWidget *card1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(card1), "face-card");

    GtkWidget *m_hdr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(card1), m_hdr, FALSE, FALSE, 0);
    GtkWidget *m_title = gtk_label_new("Now Playing");
    gtk_style_context_add_class(gtk_widget_get_style_context(m_title), "card-header-title");
    gtk_box_pack_start(GTK_BOX(m_hdr), m_title, FALSE, FALSE, 0);

    GtkWidget *m_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_style_context_add_class(gtk_widget_get_style_context(m_card), "item-card");
    gtk_box_pack_start(GTK_BOX(card1), m_card, FALSE, FALSE, 0);

    s_media_title = gtk_label_new("No media playing");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_media_title), "item-title");
    gtk_widget_set_halign(s_media_title, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(m_card), s_media_title, FALSE, FALSE, 0);

    s_media_artist = gtk_label_new("Start playback in Spotify, YouTube or local player");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_media_artist), "item-subtitle");
    gtk_widget_set_halign(s_media_artist, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(m_card), s_media_artist, FALSE, FALSE, 0);

    GtkWidget *ctrl_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(ctrl_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(m_card), ctrl_box, FALSE, FALSE, 6);

    GtkWidget *btn_prev = gtk_button_new_with_label("⏮");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_prev), "action-btn");
    g_signal_connect(btn_prev, "clicked", G_CALLBACK(on_media_prev), NULL);
    gtk_box_pack_start(GTK_BOX(ctrl_box), btn_prev, FALSE, FALSE, 0);

    s_media_play_btn = gtk_button_new_with_label("󰐊 Play");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_media_play_btn), "action-btn");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_media_play_btn), "primary");
    g_signal_connect(s_media_play_btn, "clicked", G_CALLBACK(on_media_play_pause), NULL);
    gtk_box_pack_start(GTK_BOX(ctrl_box), s_media_play_btn, FALSE, FALSE, 0);

    GtkWidget *btn_next = gtk_button_new_with_label("⏭");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_next), "action-btn");
    g_signal_connect(btn_next, "clicked", G_CALLBACK(on_media_next), NULL);
    gtk_box_pack_start(GTK_BOX(ctrl_box), btn_next, FALSE, FALSE, 0);

    /* Beat visualizer toggle + preview */
    GtkWidget *vis_hdr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(card1), vis_hdr, FALSE, FALSE, 4);

    GtkWidget *vis_title = gtk_label_new("Beat Visualizer");
    gtk_style_context_add_class(gtk_widget_get_style_context(vis_title), "card-header-title");
    gtk_box_pack_start(GTK_BOX(vis_hdr), vis_title, FALSE, FALSE, 0);

    s_vis_switch = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(s_vis_switch), TRUE);
    g_signal_connect(s_vis_switch, "state-set", G_CALLBACK(on_visualizer_toggled), NULL);
    gtk_box_pack_end(GTK_BOX(vis_hdr), s_vis_switch, FALSE, FALSE, 0);

    s_vis_draw = gtk_drawing_area_new();
    gtk_widget_set_size_request(s_vis_draw, 300, 120);
    gtk_style_context_add_class(gtk_widget_get_style_context(s_vis_draw), "item-card");
    g_signal_connect(s_vis_draw, "draw", G_CALLBACK(on_visualizer_draw), NULL);
    gtk_box_pack_start(GTK_BOX(card1), s_vis_draw, TRUE, TRUE, 0);

    GtkWidget *vis_opts = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(card1), vis_opts, FALSE, FALSE, 4);

    GtkWidget *lbl_sens = gtk_label_new("Sensitivity:");
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl_sens), "item-subtitle");
    gtk_box_pack_start(GTK_BOX(vis_opts), lbl_sens, FALSE, FALSE, 0);

    s_vis_sens_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.5, 3.0, 0.1);
    gtk_range_set_value(GTK_RANGE(s_vis_sens_scale), 1.0);
    g_signal_connect(s_vis_sens_scale, "value-changed", G_CALLBACK(on_visualizer_sens_changed), NULL);
    gtk_box_pack_start(GTK_BOX(vis_opts), s_vis_sens_scale, TRUE, TRUE, 0);

    gtk_stack_add_named(GTK_STACK(g_zone_left.stack), card1, "media");

    /* Animate the visualizer preview while the drawer is open. */
    g_timeout_add(80, +[](gpointer u) -> gboolean {
        if (s_vis_draw && g_zone_left.is_visible)
            gtk_widget_queue_draw(s_vis_draw);
        return G_SOURCE_CONTINUE;
    }, NULL);

    refresh_left_zone();
}

void refresh_left_zone(void) {
    if (!s_workspaces_container) return;

    GList *children = gtk_container_get_children(GTK_CONTAINER(s_workspaces_container));
    for (GList *l = children; l != NULL; l = l->next) {
        gtk_widget_destroy(GTK_WIDGET(l->data));
    }
    g_list_free(children);

    char *active_ws = run_shell_output("hyprctl activeworkspace -j 2>/dev/null | jq -r .id 2>/dev/null");
    int active_id = (active_ws && strlen(active_ws) > 0) ? atoi(active_ws) : 1;
    free(active_ws);

    /* Collect the first window on each workspace so every tile can act as a
       tiny preview of what is actually inside that workspace. */
    char win_titles[11][160];
    char win_classes[11][64];
    int win_counts[11] = {0};
    for (int i = 0; i <= 10; i++) {
        win_titles[i][0] = '\0';
        win_classes[i][0] = '\0';
    }

    char *clients = run_shell_output(
        "hyprctl clients -j 2>/dev/null | jq -r '.[] | [.workspace.id, .class, .title] | @tsv' 2>/dev/null");
    if (clients) {
        char *line_save = NULL;
        char *line = strtok_r(clients, "\n", &line_save);
        while (line) {
            char *field_save = NULL;
            char *id_s = strtok_r(line, "\t", &field_save);
            char *class_s = strtok_r(NULL, "\t", &field_save);
            char *title_s = strtok_r(NULL, "\t", &field_save);
            if (id_s) {
                int wid = atoi(id_s);
                if (wid >= 1 && wid <= 10) {
                    if (win_counts[wid] == 0) {
                        snprintf(win_classes[wid], sizeof(win_classes[wid]), "%s", class_s ? class_s : "");
                        snprintf(win_titles[wid], sizeof(win_titles[wid]), "%s",
                                 (title_s && strlen(title_s) > 0) ? title_s :
                                 (class_s && strlen(class_s) > 0) ? class_s : "Window");
                    }
                    win_counts[wid]++;
                }
            }
            line = strtok_r(NULL, "\n", &line_save);
        }
        free(clients);
    }

    for (int ws = 1; ws <= 10; ws++) {
        GtkWidget *btn = gtk_button_new();
        gtk_style_context_add_class(gtk_widget_get_style_context(btn), "ws-tile");
        if (ws == active_id)
            gtk_style_context_add_class(gtk_widget_get_style_context(btn), "active");
        g_object_set_data(G_OBJECT(btn), "ws-id", GINT_TO_POINTER(ws));
        g_signal_connect(btn, "clicked", G_CALLBACK(on_workspace_clicked), NULL);
        gtk_widget_set_hexpand(btn, TRUE);
        gtk_widget_set_vexpand(btn, FALSE);

        GtkWidget *v = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
        gtk_widget_set_valign(v, GTK_ALIGN_CENTER);
        gtk_container_add(GTK_CONTAINER(btn), v);

        char num[8];
        snprintf(num, sizeof(num), "%d", ws);
        GtkWidget *lbl_num = gtk_label_new(num);
        gtk_style_context_add_class(gtk_widget_get_style_context(lbl_num), "ws-num");
        gtk_widget_set_halign(lbl_num, GTK_ALIGN_CENTER);
        gtk_box_pack_start(GTK_BOX(v), lbl_num, FALSE, FALSE, 0);
        if (win_counts[ws] > 0) {
            GtkWidget *lbl_icon = gtk_label_new(tile_icon_for_class(win_classes[ws]));
            gtk_style_context_add_class(gtk_widget_get_style_context(lbl_icon), "ws-icon");
            gtk_widget_set_halign(lbl_icon, GTK_ALIGN_CENTER);
            gtk_box_pack_start(GTK_BOX(v), lbl_icon, FALSE, FALSE, 0);
        }

        char mini[192];
        if (win_counts[ws] > 0) {
            if (win_counts[ws] > 1)
                snprintf(mini, sizeof(mini), "%s  +%d", win_titles[ws], win_counts[ws] - 1);
            else
                snprintf(mini, sizeof(mini), "%s", win_titles[ws]);
        } else {
            snprintf(mini, sizeof(mini), "Empty");
        }
        GtkWidget *lbl_win = gtk_label_new(mini);
        gtk_style_context_add_class(gtk_widget_get_style_context(lbl_win), "ws-title");
        gtk_label_set_ellipsize(GTK_LABEL(lbl_win), PANGO_ELLIPSIZE_END);
        gtk_label_set_max_width_chars(GTK_LABEL(lbl_win), 12);
        gtk_widget_set_halign(lbl_win, GTK_ALIGN_CENTER);
        gtk_box_pack_start(GTK_BOX(v), lbl_win, FALSE, FALSE, 0);
        if (win_counts[ws] > 0)
            gtk_widget_set_tooltip_text(btn, win_titles[ws]);

        gtk_grid_attach(GTK_GRID(s_workspaces_container), btn,
                        (ws - 1) % 5, (ws - 1) / 5, 1, 1);
    }
    gtk_widget_show_all(s_workspaces_container);

    // Media info
    char *title = run_shell_output("playerctl metadata title 2>/dev/null");
    char *artist = run_shell_output("playerctl metadata artist 2>/dev/null");
    char *status = run_shell_output("playerctl status 2>/dev/null");

    if (title && strlen(title) > 0) {
        gtk_label_set_text(GTK_LABEL(s_media_title), title);
        gtk_label_set_text(GTK_LABEL(s_media_artist), (artist && strlen(artist) > 0) ? artist : "Unknown Artist");
        if (status && strstr(status, "Playing")) {
            gtk_button_set_label(GTK_BUTTON(s_media_play_btn), "󰏤 Pause");
        } else {
            gtk_button_set_label(GTK_BUTTON(s_media_play_btn), "󰐊 Play");
        }
    } else {
        gtk_label_set_text(GTK_LABEL(s_media_title), "No media playing");
        gtk_label_set_text(GTK_LABEL(s_media_artist), "Launch Spotify, YouTube or an MPRIS player");
        gtk_button_set_label(GTK_BUTTON(s_media_play_btn), "󰐊 Play");
    }
    free(title);
    free(artist);
    free(status);

    /* Keep the drawer switch in sync with the daemon state. */
    char *vis_state = run_shell_output(
        "python3 -c \"import json,os; print('1' if json.load(open(os.path.expanduser('~/.config/waybar/state/visualizer.json'))).get('enabled', True) else '0')\" 2>/dev/null");
    if (s_vis_switch && vis_state) {
        s_vis_syncing = TRUE;
        gtk_switch_set_active(GTK_SWITCH(s_vis_switch), vis_state[0] == '1');
        s_vis_syncing = FALSE;
    }
    free(vis_state);

    if (s_vis_draw) gtk_widget_queue_draw(s_vis_draw);
    fit_zone_to_content(&g_zone_left);
}

static void on_workspace_clicked(GtkButton *btn, gpointer user_data) {
    int ws = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "ws-id"));
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "hyprctl dispatch workspace %d", ws);
    run_shell_command(cmd);
    refresh_left_zone();
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

static gboolean on_visualizer_toggled(GtkSwitch *sw, gboolean state, gpointer user_data) {
    if (s_vis_syncing) return FALSE;
    run_shell_command("~/.config/waybar/scripts/audio/visualizer_toggle.sh");
    refresh_left_zone();
    return FALSE;
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

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.2);
    cairo_paint(cr);

    FILE *fp = fopen("/tmp/waybar_visualizer.txt", "r");
    char bars[64] = "········";
    if (fp) {
        if (fgets(bars, sizeof(bars), fp) == NULL) {
            strcpy(bars, "········");
        }
        fclose(fp);
    }

    int count = g_utf8_strlen(bars, -1);
    if (count <= 0) return FALSE;

    double bar_w = ((double)alloc.width - 20) / (double)count;
    cairo_set_source_rgba(cr, 0.72, 0.48, 0.23, 0.9); // Warm amber accent

    const char *p = bars;
    for (int i = 0; i < count; i++) {
        gunichar ch = g_utf8_get_char(p);
        p = g_utf8_next_char(p);

        double frac = 0.12;
        switch (ch) {
            case 0x2581: frac = 0.18; break;
            case 0x2582: frac = 0.28; break;
            case 0x2583: frac = 0.38; break;
            case 0x2584: frac = 0.48; break;
            case 0x2585: frac = 0.58; break;
            case 0x2586: frac = 0.68; break;
            case 0x2587: frac = 0.78; break;
            case 0x2588: frac = 0.90; break;
            default: break;
        }

        double h = frac * (alloc.height - 30);
        double x = 10 + i * bar_w;
        double y = alloc.height - 15 - h;

        cairo_rectangle(cr, x + 2, y, bar_w - 4, h);
        cairo_fill(cr);
    }

    return FALSE;
}
