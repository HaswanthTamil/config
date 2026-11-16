#include "panel_common.h"
#include <time.h>

static GtkWidget *s_calendar = NULL;
static GtkWidget *s_events_box = NULL;
static GtkWidget *s_ev_title_entry = NULL;
static GtkWidget *s_ev_time_entry = NULL;

static GtkWidget *s_notifs_box = NULL;
static GtkWidget *s_reminders_box = NULL;
static GtkWidget *s_rem_title_entry = NULL;
static GtkWidget *s_rem_time_entry = NULL;

static void on_today_clicked(GtkButton *btn, gpointer user_data);
static void on_calendar_day_selected(GtkCalendar *cal, gpointer user_data);
static void on_add_event_clicked(GtkButton *btn, gpointer user_data);
static void on_clear_all_notifs(GtkButton *btn, gpointer user_data);
static void on_add_reminder_clicked(GtkButton *btn, gpointer user_data);
static void on_reminder_complete_toggled(GtkToggleButton *btn, gpointer user_data);
static void on_reminder_delete_clicked(GtkButton *btn, gpointer user_data);

void build_center_zone(void) {
    GtkWidget *content = create_panel_shell(&g_zone_center, "panel-center", 420, 470, 0);

    // Tab bar matching the bar's inner capsules
    g_zone_center.tab_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_style_context_add_class(gtk_widget_get_style_context(g_zone_center.tab_box), "tab-bar");
    gtk_box_pack_start(GTK_BOX(content), g_zone_center.tab_box, FALSE, FALSE, 0);

    // Stack for face cards
    g_zone_center.stack = gtk_stack_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(g_zone_center.stack), "panel-stack");
    gtk_stack_set_transition_type(GTK_STACK(g_zone_center.stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_duration(GTK_STACK(g_zone_center.stack), 180);
    gtk_box_pack_start(GTK_BOX(content), g_zone_center.stack, TRUE, TRUE, 0);
    g_signal_connect(g_zone_center.stack, "notify::visible-child", G_CALLBACK(on_zone_stack_child_changed), &g_zone_center);

    g_zone_center.active_tab = 0;

    GtkWidget *b0 = create_icon_tab_button("󰃭", "Calendar", g_zone_center.stack, "calendar", 0, &g_zone_center.active_tab);
    GtkWidget *b1 = create_icon_tab_button("󰂚", "Reminders", g_zone_center.stack, "reminders", 1, &g_zone_center.active_tab);
    GtkWidget *b2 = create_icon_tab_button("󰂜", "Notifications", g_zone_center.stack, "notifications", 2, &g_zone_center.active_tab);

    gtk_box_pack_start(GTK_BOX(g_zone_center.tab_box), b0, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(g_zone_center.tab_box), b1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(g_zone_center.tab_box), b2, FALSE, FALSE, 0);

    /* -------------------------------------------------------------
       Tab 0: Calendar Face Card
       ------------------------------------------------------------- */
    GtkWidget *card0 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(card0), "face-card");

    GtkWidget *cal_hdr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(card0), cal_hdr, FALSE, FALSE, 0);

    GtkWidget *cal_title = gtk_label_new("Calendar");
    gtk_style_context_add_class(gtk_widget_get_style_context(cal_title), "card-header-title");
    gtk_box_pack_start(GTK_BOX(cal_hdr), cal_title, FALSE, FALSE, 0);

    GtkWidget *btn_today = gtk_button_new_with_label("Today");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_today), "action-btn");
    g_signal_connect(btn_today, "clicked", G_CALLBACK(on_today_clicked), NULL);
    gtk_box_pack_end(GTK_BOX(cal_hdr), btn_today, FALSE, FALSE, 0);

    s_calendar = gtk_calendar_new();
    gtk_calendar_set_display_options(GTK_CALENDAR(s_calendar),
        (GtkCalendarDisplayOptions)(GTK_CALENDAR_SHOW_HEADING | GTK_CALENDAR_SHOW_DAY_NAMES));
    g_signal_connect(s_calendar, "day-selected", G_CALLBACK(on_calendar_day_selected), NULL);
    gtk_box_pack_start(GTK_BOX(card0), s_calendar, FALSE, FALSE, 0);

    GtkWidget *ev_create_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(card0), ev_create_box, FALSE, FALSE, 0);

    s_ev_title_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(s_ev_title_entry), "Event title...");
    gtk_box_pack_start(GTK_BOX(ev_create_box), s_ev_title_entry, TRUE, TRUE, 0);

    s_ev_time_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(s_ev_time_entry), "14:00");
    gtk_widget_set_size_request(s_ev_time_entry, 60, -1);
    gtk_box_pack_start(GTK_BOX(ev_create_box), s_ev_time_entry, FALSE, FALSE, 0);

    GtkWidget *btn_add_ev = gtk_button_new_with_label("+ Add");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_add_ev), "action-btn");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_add_ev), "primary");
    g_signal_connect(btn_add_ev, "clicked", G_CALLBACK(on_add_event_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(ev_create_box), btn_add_ev, FALSE, FALSE, 0);

    GtkWidget *ev_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(ev_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(card0), ev_scroll, TRUE, TRUE, 0);

    s_events_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_add(GTK_CONTAINER(ev_scroll), s_events_box);

    gtk_stack_add_named(GTK_STACK(g_zone_center.stack), card0, "calendar");

    /* -------------------------------------------------------------
       Tab 1: Notifications Face Card
       ------------------------------------------------------------- */
    GtkWidget *card1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(card1), "face-card");

    GtkWidget *n_hdr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(card1), n_hdr, FALSE, FALSE, 0);

    GtkWidget *n_title = gtk_label_new("Notifications");
    gtk_style_context_add_class(gtk_widget_get_style_context(n_title), "card-header-title");
    gtk_box_pack_start(GTK_BOX(n_hdr), n_title, FALSE, FALSE, 0);

    GtkWidget *btn_clear_notifs = gtk_button_new_with_label("Clear All");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_clear_notifs), "action-btn");
    g_signal_connect(btn_clear_notifs, "clicked", G_CALLBACK(on_clear_all_notifs), NULL);
    gtk_box_pack_end(GTK_BOX(n_hdr), btn_clear_notifs, FALSE, FALSE, 0);

    GtkWidget *n_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(n_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(card1), n_scroll, TRUE, TRUE, 0);

    s_notifs_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_add(GTK_CONTAINER(n_scroll), s_notifs_box);

    gtk_stack_add_named(GTK_STACK(g_zone_center.stack), card1, "notifications");

    /* -------------------------------------------------------------
       Tab 2: Reminders Face Card
       ------------------------------------------------------------- */
    GtkWidget *card2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(card2), "face-card");

    GtkWidget *rem_title = gtk_label_new("Reminders");
    gtk_style_context_add_class(gtk_widget_get_style_context(rem_title), "card-header-title");
    gtk_box_pack_start(GTK_BOX(card2), rem_title, FALSE, FALSE, 0);

    GtkWidget *rem_input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(card2), rem_input_box, FALSE, FALSE, 0);

    s_rem_title_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(s_rem_title_entry), "New reminder...");
    gtk_box_pack_start(GTK_BOX(rem_input_box), s_rem_title_entry, TRUE, TRUE, 0);

    s_rem_time_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(s_rem_time_entry), "YYYY-MM-DD HH:MM");
    gtk_widget_set_size_request(s_rem_time_entry, 130, -1);
    gtk_box_pack_start(GTK_BOX(rem_input_box), s_rem_time_entry, FALSE, FALSE, 0);

    GtkWidget *btn_add_rem = gtk_button_new_with_label("+ Add");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_add_rem), "action-btn");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_add_rem), "primary");
    g_signal_connect(btn_add_rem, "clicked", G_CALLBACK(on_add_reminder_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(rem_input_box), btn_add_rem, FALSE, FALSE, 0);

    GtkWidget *rem_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(rem_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(card2), rem_scroll, TRUE, TRUE, 0);

    s_reminders_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_add(GTK_CONTAINER(rem_scroll), s_reminders_box);

    gtk_stack_add_named(GTK_STACK(g_zone_center.stack), card2, "reminders");

    refresh_center_zone();
}

void refresh_center_zone(void) {
    if (!s_events_box || !s_notifs_box || !s_reminders_box) return;

    // 1. Events
    GList *ev_ch = gtk_container_get_children(GTK_CONTAINER(s_events_box));
    for (GList *l = ev_ch; l != NULL; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(ev_ch);

    char *ev_data = run_shell_output(
        "python3 -c \"import json, os; "
        "f=os.path.expanduser('~/.config/waybar/state/calendar_events.json'); "
        "print('\\n'.join(f'{e.get(\\\"date\\\",\\\"\\\")}|{e.get(\\\"time\\\",\\\"\\\")}|{e.get(\\\"title\\\",\\\"\\\")}' for e in json.load(open(f)))) if os.path.exists(f) else ''\""
    );

    if (ev_data && strlen(ev_data) > 2) {
        char *line = strtok(ev_data, "\n");
        while (line) {
            char date[32] = "", time_s[32] = "", title[128] = "";
            sscanf(line, "%31[^|]|%31[^|]|%127[^\n]", date, time_s, title);

            GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
            gtk_style_context_add_class(gtk_widget_get_style_context(row), "item-card");

            char dt_buf[64];
            snprintf(dt_buf, sizeof(dt_buf), "󰃭 %s %s", date, time_s);
            GtkWidget *lbl_dt = gtk_label_new(dt_buf);
            gtk_style_context_add_class(gtk_widget_get_style_context(lbl_dt), "item-subtitle");
            gtk_box_pack_start(GTK_BOX(row), lbl_dt, FALSE, FALSE, 0);

            GtkWidget *lbl_t = gtk_label_new(title);
            gtk_style_context_add_class(gtk_widget_get_style_context(lbl_t), "item-title");
            gtk_box_pack_start(GTK_BOX(row), lbl_t, TRUE, TRUE, 0);

            gtk_box_pack_start(GTK_BOX(s_events_box), row, FALSE, FALSE, 0);
            line = strtok(NULL, "\n");
        }
    } else {
        GtkWidget *empty = gtk_label_new("No scheduled events.");
        gtk_style_context_add_class(gtk_widget_get_style_context(empty), "item-subtitle");
        gtk_box_pack_start(GTK_BOX(s_events_box), empty, FALSE, FALSE, 8);
    }
    free(ev_data);
    gtk_widget_show_all(s_events_box);

    // 2. Notifications
    GList *n_ch = gtk_container_get_children(GTK_CONTAINER(s_notifs_box));
    for (GList *l = n_ch; l != NULL; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(n_ch);

    char *notif_data = run_shell_output(
        "dunstctl history 2>/dev/null | "
        "jq -r '.data[][] | \"\\(.id.data)|\\(.appname.data)|\\(.summary.data)|\\(.body.data)\"' 2>/dev/null"
    );

    if (notif_data && strlen(notif_data) > 2) {
        char *line = strtok(notif_data, "\n");
        while (line) {
            int nid = 0;
            char app[64] = "", sum[128] = "", body[256] = "";
            sscanf(line, "%d|%63[^|]|%127[^|]|%255[^\n]", &nid, app, sum, body);

            GtkWidget *row = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
            gtk_style_context_add_class(gtk_widget_get_style_context(row), "item-card");

            GtkWidget *h = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
            gtk_box_pack_start(GTK_BOX(row), h, FALSE, FALSE, 0);

            char app_buf[128];
            snprintf(app_buf, sizeof(app_buf), "󰂚  %s: %s", app, sum);
            GtkWidget *lbl_sum = gtk_label_new(app_buf);
            gtk_style_context_add_class(gtk_widget_get_style_context(lbl_sum), "item-title");
            gtk_widget_set_halign(lbl_sum, GTK_ALIGN_START);
            gtk_box_pack_start(GTK_BOX(h), lbl_sum, TRUE, TRUE, 0);

            GtkWidget *btn_rm = gtk_button_new_with_label("✕");
            gtk_style_context_add_class(gtk_widget_get_style_context(btn_rm), "action-btn");
            g_object_set_data(G_OBJECT(btn_rm), "notif-id", GINT_TO_POINTER(nid));
            g_signal_connect(btn_rm, "clicked", G_CALLBACK(+[](GtkButton *b, gpointer u) {
                int id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(b), "notif-id"));
                char cmd[64];
                snprintf(cmd, sizeof(cmd), "dunstctl history-rm %d 2>/dev/null", id);
                run_shell_command(cmd);
                refresh_center_zone();
            }), NULL);
            gtk_box_pack_end(GTK_BOX(h), btn_rm, FALSE, FALSE, 0);

            if (strlen(body) > 0) {
                GtkWidget *lbl_body = gtk_label_new(body);
                gtk_style_context_add_class(gtk_widget_get_style_context(lbl_body), "item-subtitle");
                gtk_label_set_line_wrap(GTK_LABEL(lbl_body), TRUE);
                gtk_widget_set_halign(lbl_body, GTK_ALIGN_START);
                gtk_box_pack_start(GTK_BOX(row), lbl_body, FALSE, FALSE, 0);
            }

            gtk_box_pack_start(GTK_BOX(s_notifs_box), row, FALSE, FALSE, 0);
            line = strtok(NULL, "\n");
        }
    } else {
        GtkWidget *empty = gtk_label_new("No notifications in history.");
        gtk_style_context_add_class(gtk_widget_get_style_context(empty), "item-subtitle");
        gtk_box_pack_start(GTK_BOX(s_notifs_box), empty, FALSE, FALSE, 8);
    }
    free(notif_data);
    gtk_widget_show_all(s_notifs_box);

    // 3. Reminders
    GList *r_ch = gtk_container_get_children(GTK_CONTAINER(s_reminders_box));
    for (GList *l = r_ch; l != NULL; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
    g_list_free(r_ch);

    char *rem_data = run_shell_output(
        "python3 -c \"import json, os; "
        "f=os.path.expanduser('~/.config/waybar/state/reminders.json'); "
        "print('\\n'.join(f'{r.get(\\\"id\\\",\\\"\\\")}|{1 if r.get(\\\"completed\\\") else 0}|{r.get(\\\"due\\\",\\\"\\\")}|{r.get(\\\"title\\\",\\\"\\\")}' for r in json.load(open(f)))) if os.path.exists(f) else ''\""
    );

    if (rem_data && strlen(rem_data) > 2) {
        char *line = strtok(rem_data, "\n");
        while (line) {
            char id[64] = "", due[64] = "", title[128] = "";
            int comp = 0;
            sscanf(line, "%63[^|]|%d|%63[^|]|%127[^\n]", id, &comp, due, title);

            GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
            gtk_style_context_add_class(gtk_widget_get_style_context(row), "item-card");

            GtkWidget *chk = gtk_check_button_new();
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk), comp);
            g_object_set_data_full(G_OBJECT(chk), "rem-id", strdup(id), free);
            g_signal_connect(chk, "toggled", G_CALLBACK(on_reminder_complete_toggled), NULL);
            gtk_box_pack_start(GTK_BOX(row), chk, FALSE, FALSE, 0);

            GtkWidget *v = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
            gtk_box_pack_start(GTK_BOX(row), v, TRUE, TRUE, 0);

            GtkWidget *lbl_t = gtk_label_new(title);
            gtk_style_context_add_class(gtk_widget_get_style_context(lbl_t), "item-title");
            gtk_widget_set_halign(lbl_t, GTK_ALIGN_START);
            gtk_box_pack_start(GTK_BOX(v), lbl_t, FALSE, FALSE, 0);

            char due_buf[64];
            snprintf(due_buf, sizeof(due_buf), "Due: %s", due);
            GtkWidget *lbl_d = gtk_label_new(due_buf);
            gtk_style_context_add_class(gtk_widget_get_style_context(lbl_d), "item-subtitle");
            gtk_widget_set_halign(lbl_d, GTK_ALIGN_START);
            gtk_box_pack_start(GTK_BOX(v), lbl_d, FALSE, FALSE, 0);

            GtkWidget *btn_del = gtk_button_new_with_label("✕");
            gtk_style_context_add_class(gtk_widget_get_style_context(btn_del), "action-btn");
            gtk_style_context_add_class(gtk_widget_get_style_context(btn_del), "danger");
            g_object_set_data_full(G_OBJECT(btn_del), "rem-id", strdup(id), free);
            g_signal_connect(btn_del, "clicked", G_CALLBACK(on_reminder_delete_clicked), NULL);
            gtk_box_pack_end(GTK_BOX(row), btn_del, FALSE, FALSE, 0);

            gtk_box_pack_start(GTK_BOX(s_reminders_box), row, FALSE, FALSE, 0);
            line = strtok(NULL, "\n");
        }
    } else {
        GtkWidget *empty = gtk_label_new("No reminders.");
        gtk_style_context_add_class(gtk_widget_get_style_context(empty), "item-subtitle");
        gtk_box_pack_start(GTK_BOX(s_reminders_box), empty, FALSE, FALSE, 8);
    }
    free(rem_data);
    gtk_widget_show_all(s_reminders_box);
    fit_zone_to_content(&g_zone_center);
}

static void on_today_clicked(GtkButton *btn, gpointer user_data) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    gtk_calendar_select_month(GTK_CALENDAR(s_calendar), tm->tm_mon, tm->tm_year + 1900);
    gtk_calendar_select_day(GTK_CALENDAR(s_calendar), tm->tm_mday);
}

static void on_calendar_day_selected(GtkCalendar *cal, gpointer user_data) {
    refresh_center_zone();
}

static void on_add_event_clicked(GtkButton *btn, gpointer user_data) {
    const char *title = gtk_entry_get_text(GTK_ENTRY(s_ev_title_entry));
    const char *time_s = gtk_entry_get_text(GTK_ENTRY(s_ev_time_entry));
    if (!title || strlen(title) == 0) return;

    guint y, m, d;
    gtk_calendar_get_date(GTK_CALENDAR(s_calendar), &y, &m, &d);

    char cmd[512];
    snprintf(cmd, sizeof(cmd),
        "python3 -c \"import json, os, uuid; "
        "f=os.path.expanduser('~/.config/waybar/state/calendar_events.json'); "
        "data = json.load(open(f)) if os.path.exists(f) else []; "
        "data.append({'id': str(uuid.uuid4()), 'title': '%s', 'date': '%04d-%02d-%02d', 'time': '%s'}); "
        "json.dump(data, open(f, 'w'), indent=2)\"",
        title, y, m + 1, d, (time_s && strlen(time_s) > 0) ? time_s : "All day");

    run_shell_command(cmd);
    gtk_entry_set_text(GTK_ENTRY(s_ev_title_entry), "");
    refresh_center_zone();
}

static void on_clear_all_notifs(GtkButton *btn, gpointer user_data) {
    run_shell_command("dunstctl history-clear 2>/dev/null");
    refresh_center_zone();
}

static void on_add_reminder_clicked(GtkButton *btn, gpointer user_data) {
    const char *title = gtk_entry_get_text(GTK_ENTRY(s_rem_title_entry));
    const char *due = gtk_entry_get_text(GTK_ENTRY(s_rem_time_entry));
    if (!title || strlen(title) == 0) return;

    char cmd[512];
    snprintf(cmd, sizeof(cmd),
        "python3 -c \"import json, os, uuid; "
        "f=os.path.expanduser('~/.config/waybar/state/reminders.json'); "
        "data = json.load(open(f)) if os.path.exists(f) else []; "
        "data.append({'id': str(uuid.uuid4()), 'title': '%s', 'due': '%s', 'completed': False, 'notified': False}); "
        "json.dump(data, open(f, 'w'), indent=2)\"",
        title, (due && strlen(due) > 0) ? due : "Today");

    run_shell_command(cmd);
    gtk_entry_set_text(GTK_ENTRY(s_rem_title_entry), "");
    refresh_center_zone();
}

static void on_reminder_complete_toggled(GtkToggleButton *btn, gpointer user_data) {
    const char *id = (const char *)g_object_get_data(G_OBJECT(btn), "rem-id");
    if (!id) return;
    gboolean comp = gtk_toggle_button_get_active(btn);

    char cmd[256];
    snprintf(cmd, sizeof(cmd),
        "python3 -c \"import json, os; "
        "f=os.path.expanduser('~/.config/waybar/state/reminders.json'); "
        "data = json.load(open(f)); "
        "for r in data: "
        "  if r.get('id') == '%s': r['completed'] = %s; "
        "json.dump(data, open(f, 'w'), indent=2)\"",
        id, comp ? "True" : "False");

    run_shell_command(cmd);
}

static void on_reminder_delete_clicked(GtkButton *btn, gpointer user_data) {
    const char *id = (const char *)g_object_get_data(G_OBJECT(btn), "rem-id");
    if (!id) return;

    char cmd[256];
    snprintf(cmd, sizeof(cmd),
        "python3 -c \"import json, os; "
        "f=os.path.expanduser('~/.config/waybar/state/reminders.json'); "
        "data = [r for r in json.load(open(f)) if r.get('id') != '%s']; "
        "json.dump(data, open(f, 'w'), indent=2)\"",
        id);

    run_shell_command(cmd);
    refresh_center_zone();
}
