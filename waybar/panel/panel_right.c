#include "panel_common.h"

// Forward declarations
static gboolean on_bt_toggle_clicked(GtkSwitch *sw, gboolean state, gpointer user_data);
static gboolean on_wifi_toggle_clicked(GtkSwitch *sw, gboolean state, gpointer user_data);
static void on_output_vol_changed(GtkRange *range, gpointer user_data);
static void on_output_mute_clicked(GtkButton *btn, gpointer user_data);
static void on_input_vol_changed(GtkRange *range, gpointer user_data);
static void on_input_mute_clicked(GtkButton *btn, gpointer user_data);
static void on_profile_clicked(GtkButton *btn, gpointer user_data);
static gboolean refresh_right_later(gpointer user_data);
static void on_bt_device_clicked(GtkButton *btn, gpointer user_data);
static void on_power_lock_clicked(GtkButton *btn, gpointer user_data);
static void on_power_logout_clicked(GtkButton *btn, gpointer user_data);
static void on_power_suspend_clicked(GtkButton *btn, gpointer user_data);
static void on_power_reboot_clicked(GtkButton *btn, gpointer user_data);
static void on_power_shutdown_clicked(GtkButton *btn, gpointer user_data);

// Audio
static GtkWidget *s_out_dev_lbl = NULL;
static GtkWidget *s_out_vol_scale = NULL;
static GtkWidget *s_out_mute_btn = NULL;
static GtkWidget *s_in_dev_lbl = NULL;
static GtkWidget *s_in_vol_scale = NULL;
static GtkWidget *s_in_mute_btn = NULL;

// Power mode
static GtkWidget *s_profile_lbl = NULL;
static GtkWidget *s_profile_btns[3] = {0};
static const char *s_profile_ids[3] = { "power-saver", "balanced", "performance" };

// Bluetooth
static GtkWidget *s_bt_switch = NULL;
static GtkWidget *s_bt_name_lbl = NULL;
static GtkWidget *s_bt_sub_lbl = NULL;
static GtkWidget *s_bt_devices_box = NULL;

// Wi-Fi
static GtkWidget *s_net_title_lbl = NULL;
static GtkWidget *s_net_sub_lbl = NULL;
static GtkWidget *s_net_switch = NULL;

static gboolean s_refreshing = FALSE;

static void update_profile_buttons(const char *current) {
    for (int i = 0; i < 3; i++) {
        if (!s_profile_btns[i]) continue;
        GtkStyleContext *ctx = gtk_widget_get_style_context(s_profile_btns[i]);
        if (current && strcmp(current, s_profile_ids[i]) == 0)
            gtk_style_context_add_class(ctx, "primary");
        else
            gtk_style_context_remove_class(ctx, "primary");
    }
}

void build_right_zone(void) {
    GtkWidget *content = create_panel_shell(&g_zone_right, "panel-right", 350, 440, 1);

    // Horizontal tab row matching the bar's control capsule
    g_zone_right.tab_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_style_context_add_class(gtk_widget_get_style_context(g_zone_right.tab_box), "tab-bar");
    gtk_box_pack_start(GTK_BOX(content), g_zone_right.tab_box, FALSE, FALSE, 0);

    // Stack for face cards
    g_zone_right.stack = gtk_stack_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(g_zone_right.stack), "panel-stack");
    gtk_stack_set_transition_type(GTK_STACK(g_zone_right.stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_duration(GTK_STACK(g_zone_right.stack), 180);
    gtk_box_pack_start(GTK_BOX(content), g_zone_right.stack, TRUE, TRUE, 0);
    g_signal_connect(g_zone_right.stack, "notify::visible-child", G_CALLBACK(on_zone_stack_child_changed), &g_zone_right);

    g_zone_right.active_tab = 0;

    GtkWidget *b0 = create_icon_tab_button("󰕾", "Audio", g_zone_right.stack, "audio", 0, &g_zone_right.active_tab);
    GtkWidget *b1 = create_icon_tab_button("󰓅", "Power Mode", g_zone_right.stack, "powermode", 1, &g_zone_right.active_tab);
    GtkWidget *b2 = create_icon_tab_button("󰂯", "Bluetooth", g_zone_right.stack, "bluetooth", 2, &g_zone_right.active_tab);
    GtkWidget *b3 = create_icon_tab_button("󰖩", "Wi-Fi", g_zone_right.stack, "network", 3, &g_zone_right.active_tab);
    GtkWidget *b4 = create_icon_tab_button("󰐥", "Power", g_zone_right.stack, "power", 4, &g_zone_right.active_tab);

    gtk_box_pack_start(GTK_BOX(g_zone_right.tab_box), b0, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(g_zone_right.tab_box), b1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(g_zone_right.tab_box), b2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(g_zone_right.tab_box), b3, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(g_zone_right.tab_box), b4, FALSE, FALSE, 0);

    /* -------------------------------------------------------------
       Tab 0: Audio input + output
       ------------------------------------------------------------- */
    GtkWidget *card0 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(card0), "face-card");

    GtkWidget *audio_hdr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(card0), audio_hdr, FALSE, FALSE, 0);
    GtkWidget *audio_title = gtk_label_new("Audio");
    gtk_style_context_add_class(gtk_widget_get_style_context(audio_title), "card-header-title");
    gtk_box_pack_start(GTK_BOX(audio_hdr), audio_title, FALSE, FALSE, 0);

    GtkWidget *out_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(out_card), "item-card");
    gtk_box_pack_start(GTK_BOX(card0), out_card, FALSE, FALSE, 0);

    s_out_dev_lbl = gtk_label_new("Output");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_out_dev_lbl), "item-title");
    gtk_widget_set_halign(s_out_dev_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(out_card), s_out_dev_lbl, FALSE, FALSE, 0);

    GtkWidget *vol_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(out_card), vol_row, FALSE, FALSE, 4);

    s_out_vol_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(s_out_vol_scale), 50);
    g_signal_connect(s_out_vol_scale, "value-changed", G_CALLBACK(on_output_vol_changed), NULL);
    gtk_box_pack_start(GTK_BOX(vol_row), s_out_vol_scale, TRUE, TRUE, 0);

    s_out_mute_btn = gtk_button_new_with_label("Mute");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_out_mute_btn), "action-btn");
    g_signal_connect(s_out_mute_btn, "clicked", G_CALLBACK(on_output_mute_clicked), NULL);
    gtk_box_pack_end(GTK_BOX(vol_row), s_out_mute_btn, FALSE, FALSE, 0);

    GtkWidget *in_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(in_card), "item-card");
    gtk_box_pack_start(GTK_BOX(card0), in_card, FALSE, FALSE, 0);

    s_in_dev_lbl = gtk_label_new("Input");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_in_dev_lbl), "item-title");
    gtk_widget_set_halign(s_in_dev_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(in_card), s_in_dev_lbl, FALSE, FALSE, 0);

    GtkWidget *in_vol_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(in_card), in_vol_row, FALSE, FALSE, 4);

    s_in_vol_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(s_in_vol_scale), 65);
    g_signal_connect(s_in_vol_scale, "value-changed", G_CALLBACK(on_input_vol_changed), NULL);
    gtk_box_pack_start(GTK_BOX(in_vol_row), s_in_vol_scale, TRUE, TRUE, 0);

    s_in_mute_btn = gtk_button_new_with_label("Mute");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_in_mute_btn), "action-btn");
    g_signal_connect(s_in_mute_btn, "clicked", G_CALLBACK(on_input_mute_clicked), NULL);
    gtk_box_pack_end(GTK_BOX(in_vol_row), s_in_mute_btn, FALSE, FALSE, 0);

    gtk_stack_add_named(GTK_STACK(g_zone_right.stack), card0, "audio");

    /* -------------------------------------------------------------
       Tab 1: Power Mode
       ------------------------------------------------------------- */
    GtkWidget *card1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(card1), "face-card");

    GtkWidget *pm_hdr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(card1), pm_hdr, FALSE, FALSE, 0);
    GtkWidget *pm_title = gtk_label_new("Power Mode");
    gtk_style_context_add_class(gtk_widget_get_style_context(pm_title), "card-header-title");
    gtk_box_pack_start(GTK_BOX(pm_hdr), pm_title, FALSE, FALSE, 0);

    s_profile_lbl = gtk_label_new("Current: --");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_profile_lbl), "item-subtitle");
    gtk_box_pack_end(GTK_BOX(pm_hdr), s_profile_lbl, FALSE, FALSE, 0);

    GtkWidget *pm_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_style_context_add_class(gtk_widget_get_style_context(pm_card), "item-card");
    gtk_box_pack_start(GTK_BOX(card1), pm_card, FALSE, FALSE, 0);

    const char *labels[3] = { "󰌪  Power Saver", "󰓅  Balanced", "󰓅  Performance" };
    for (int i = 0; i < 3; i++) {
        s_profile_btns[i] = gtk_button_new_with_label(labels[i]);
        gtk_style_context_add_class(gtk_widget_get_style_context(s_profile_btns[i]), "action-btn");
        g_object_set_data(G_OBJECT(s_profile_btns[i]), "profile-idx", GINT_TO_POINTER(i));
        g_signal_connect(s_profile_btns[i], "clicked", G_CALLBACK(on_profile_clicked), NULL);
        gtk_widget_set_halign(s_profile_btns[i], GTK_ALIGN_FILL);
        gtk_box_pack_start(GTK_BOX(pm_card), s_profile_btns[i], FALSE, FALSE, 2);
    }

    GtkWidget *pm_hint = gtk_label_new("Lower profiles reduce power draw and fan noise.");
    gtk_style_context_add_class(gtk_widget_get_style_context(pm_hint), "item-subtitle");
    gtk_label_set_line_wrap(GTK_LABEL(pm_hint), TRUE);
    gtk_widget_set_halign(pm_hint, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(card1), pm_hint, FALSE, FALSE, 2);

    gtk_stack_add_named(GTK_STACK(g_zone_right.stack), card1, "powermode");

    /* -------------------------------------------------------------
       Tab 2: Bluetooth
       ------------------------------------------------------------- */
    GtkWidget *card2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(card2), "face-card");

    GtkWidget *bt_hdr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(card2), bt_hdr, FALSE, FALSE, 0);
    GtkWidget *bt_title = gtk_label_new("Bluetooth");
    gtk_style_context_add_class(gtk_widget_get_style_context(bt_title), "card-header-title");
    gtk_box_pack_start(GTK_BOX(bt_hdr), bt_title, FALSE, FALSE, 0);

    s_bt_switch = gtk_switch_new();
    g_signal_connect(s_bt_switch, "state-set", G_CALLBACK(on_bt_toggle_clicked), NULL);
    gtk_box_pack_end(GTK_BOX(bt_hdr), s_bt_switch, FALSE, FALSE, 0);

    GtkWidget *bt_card = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_style_context_add_class(gtk_widget_get_style_context(bt_card), "item-card");
    gtk_box_pack_start(GTK_BOX(card2), bt_card, FALSE, FALSE, 0);

    GtkWidget *bt_icon = gtk_label_new("󰂯");
    gtk_style_context_add_class(gtk_widget_get_style_context(bt_icon), "item-title");
    gtk_box_pack_start(GTK_BOX(bt_card), bt_icon, FALSE, FALSE, 2);

    GtkWidget *bt_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_box_pack_start(GTK_BOX(bt_card), bt_vbox, TRUE, TRUE, 0);

    s_bt_name_lbl = gtk_label_new("Bluetooth Controller");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_bt_name_lbl), "item-title");
    gtk_widget_set_halign(s_bt_name_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(bt_vbox), s_bt_name_lbl, FALSE, FALSE, 0);

    s_bt_sub_lbl = gtk_label_new("Checking status...");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_bt_sub_lbl), "item-subtitle");
    gtk_widget_set_halign(s_bt_sub_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(bt_vbox), s_bt_sub_lbl, FALSE, FALSE, 0);

    GtkWidget *bt_dev_title = gtk_label_new("Paired devices");
    gtk_style_context_add_class(gtk_widget_get_style_context(bt_dev_title), "card-header-title");
    gtk_box_pack_start(GTK_BOX(card2), bt_dev_title, FALSE, FALSE, 4);

    GtkWidget *bt_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(bt_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(card2), bt_scroll, TRUE, TRUE, 0);

    s_bt_devices_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_add(GTK_CONTAINER(bt_scroll), s_bt_devices_box);

    gtk_stack_add_named(GTK_STACK(g_zone_right.stack), card2, "bluetooth");

    /* -------------------------------------------------------------
       Tab 3: Wi-Fi
       ------------------------------------------------------------- */
    GtkWidget *card3 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(card3), "face-card");

    GtkWidget *net_hdr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(card3), net_hdr, FALSE, FALSE, 0);
    GtkWidget *net_title = gtk_label_new("Wi-Fi");
    gtk_style_context_add_class(gtk_widget_get_style_context(net_title), "card-header-title");
    gtk_box_pack_start(GTK_BOX(net_hdr), net_title, FALSE, FALSE, 0);

    s_net_switch = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(s_net_switch), TRUE);
    g_signal_connect(s_net_switch, "state-set", G_CALLBACK(on_wifi_toggle_clicked), NULL);
    gtk_box_pack_end(GTK_BOX(net_hdr), s_net_switch, FALSE, FALSE, 0);

    GtkWidget *net_card = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_style_context_add_class(gtk_widget_get_style_context(net_card), "item-card");
    gtk_box_pack_start(GTK_BOX(card3), net_card, FALSE, FALSE, 0);

    GtkWidget *net_icon = gtk_label_new("󰖩");
    gtk_style_context_add_class(gtk_widget_get_style_context(net_icon), "item-title");
    gtk_box_pack_start(GTK_BOX(net_card), net_icon, FALSE, FALSE, 2);

    GtkWidget *net_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_box_pack_start(GTK_BOX(net_card), net_vbox, TRUE, TRUE, 0);

    s_net_title_lbl = gtk_label_new("Not connected");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_net_title_lbl), "item-title");
    gtk_widget_set_halign(s_net_title_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(net_vbox), s_net_title_lbl, FALSE, FALSE, 0);

    s_net_sub_lbl = gtk_label_new("--");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_net_sub_lbl), "item-subtitle");
    gtk_widget_set_halign(s_net_sub_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(net_vbox), s_net_sub_lbl, FALSE, FALSE, 0);

    GtkWidget *chk_icon = gtk_label_new("󰄵");
    gtk_style_context_add_class(gtk_widget_get_style_context(chk_icon), "item-subtitle");
    gtk_box_pack_end(GTK_BOX(net_card), chk_icon, FALSE, FALSE, 2);

    gtk_stack_add_named(GTK_STACK(g_zone_right.stack), card3, "network");

    /* -------------------------------------------------------------
       Tab 4: Power Controls
       ------------------------------------------------------------- */
    GtkWidget *card4 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(card4), "face-card");

    GtkWidget *p_title = gtk_label_new("Session & Power");
    gtk_style_context_add_class(gtk_widget_get_style_context(p_title), "card-header-title");
    gtk_box_pack_start(GTK_BOX(card4), p_title, FALSE, FALSE, 0);

    GtkWidget *btn_lock = gtk_button_new_with_label("   Lock Screen");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_lock), "action-btn");
    g_signal_connect(btn_lock, "clicked", G_CALLBACK(on_power_lock_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(card4), btn_lock, FALSE, FALSE, 2);

    GtkWidget *btn_logout = gtk_button_new_with_label("󰍃   Log Out");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_logout), "action-btn");
    g_signal_connect(btn_logout, "clicked", G_CALLBACK(on_power_logout_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(card4), btn_logout, FALSE, FALSE, 2);

    GtkWidget *btn_suspend = gtk_button_new_with_label("󰒲   Suspend");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_suspend), "action-btn");
    g_signal_connect(btn_suspend, "clicked", G_CALLBACK(on_power_suspend_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(card4), btn_suspend, FALSE, FALSE, 2);

    GtkWidget *btn_reboot = gtk_button_new_with_label("󰜉   Restart...");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_reboot), "action-btn");
    g_signal_connect(btn_reboot, "clicked", G_CALLBACK(on_power_reboot_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(card4), btn_reboot, FALSE, FALSE, 2);

    GtkWidget *btn_shutdown = gtk_button_new_with_label("󰐥   Shut Down...");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_shutdown), "action-btn");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_shutdown), "danger");
    g_signal_connect(btn_shutdown, "clicked", G_CALLBACK(on_power_shutdown_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(card4), btn_shutdown, FALSE, FALSE, 2);

    gtk_stack_add_named(GTK_STACK(g_zone_right.stack), card4, "power");

    gtk_stack_set_visible_child_name(GTK_STACK(g_zone_right.stack), "audio");

    refresh_right_zone();
}

void refresh_right_zone(void) {
    if (!s_out_dev_lbl) return;
    s_refreshing = TRUE;

    // Audio output
    char *out_json = run_shell_output("~/.config/waybar/scripts/audio/volume_status.sh 2>/dev/null");
    if (out_json) {
        char *p = strstr(out_json, "Audio Output: ");
        char info[64] = "Output";
        if (p) sscanf(p + 14, "%63[^\"]", info);
        char buf[96];
        snprintf(buf, sizeof(buf), "󰕾  %s", info);
        gtk_label_set_text(GTK_LABEL(s_out_dev_lbl), buf);
        free(out_json);
    }

    char *out_vol = run_shell_output("wpctl get-volume @DEFAULT_AUDIO_SINK@ 2>/dev/null | awk '{printf \"%.0f\", $2*100}'");
    if (out_vol && strlen(out_vol) > 0)
        gtk_range_set_value(GTK_RANGE(s_out_vol_scale), atof(out_vol));
    free(out_vol);

    // Audio input
    char *in_json = run_shell_output("~/.config/waybar/scripts/audio/input_status.sh 2>/dev/null");
    if (in_json) {
        char *p = strstr(in_json, "Microphone: ");
        char info[64] = "Microphone";
        if (p) sscanf(p + 12, "%63[^\"]", info);
        char buf[96];
        snprintf(buf, sizeof(buf), "󰍬  %s", info);
        gtk_label_set_text(GTK_LABEL(s_in_dev_lbl), buf);
        free(in_json);
    }

    char *in_vol = run_shell_output("wpctl get-volume @DEFAULT_AUDIO_SOURCE@ 2>/dev/null | awk '{printf \"%.0f\", $2*100}'");
    if (in_vol && strlen(in_vol) > 0)
        gtk_range_set_value(GTK_RANGE(s_in_vol_scale), atof(in_vol));
    free(in_vol);

    // Power profile: power-profiles-daemon first, ACPI platform_profile fallback.
    char profile[64] = "";
    char *pp = run_shell_output("powerprofilesctl get 2>/dev/null");
    if (pp && strlen(pp) > 0) {
        char *nl = strchr(pp, '\n');
        if (nl) *nl = '\0';
        snprintf(profile, sizeof(profile), "%s", pp);
    }
    free(pp);

    if (strlen(profile) == 0) {
        char *platform = run_shell_output("cat /sys/firmware/acpi/platform_profile 2>/dev/null");
        if (platform) {
            char *nl = strchr(platform, '\n');
            if (nl) *nl = '\0';
            if (strcmp(platform, "performance") == 0)
                strcpy(profile, "performance");
            else if (strcmp(platform, "balanced") == 0)
                strcpy(profile, "balanced");
            else if (strcmp(platform, "quiet") == 0 ||
                     strcmp(platform, "cool") == 0 ||
                     strcmp(platform, "low-power") == 0)
                strcpy(profile, "power-saver");
            else if (strlen(platform) > 0)
                snprintf(profile, sizeof(profile), "%s", platform);
            free(platform);
        }
    }

    if (strlen(profile) > 0) {
        char buf[96];
        snprintf(buf, sizeof(buf), "Current: %s", profile);
        gtk_label_set_text(GTK_LABEL(s_profile_lbl), buf);
        update_profile_buttons(profile);
    } else {
        gtk_label_set_text(GTK_LABEL(s_profile_lbl), "Unavailable");
        update_profile_buttons(NULL);
    }

    // Bluetooth status
    int bt_powered = 0;
    char bt_name[128] = "Bluetooth Controller";
    char bt_sub[128] = "Checking status...";

    char *bt_json = run_shell_output("~/.config/waybar/scripts/bluetooth/bt_status.sh 2>/dev/null");
    if (bt_json) {
        if (strstr(bt_json, "Disabled")) {
            strcpy(bt_sub, "Disabled");
        } else if (strstr(bt_json, "Connected to")) {
            char *p = strstr(bt_json, "Connected to ");
            if (p) sscanf(p + 13, "%127[^\"]", bt_name);
            snprintf(bt_sub, sizeof(bt_sub), "Connected to %s", bt_name);
            bt_powered = 1;
        } else {
            strcpy(bt_sub, "Enabled · no device connected");
            bt_powered = 1;
        }
        free(bt_json);
    }
    if (s_bt_switch) gtk_switch_set_active(GTK_SWITCH(s_bt_switch), bt_powered != 0);
    gtk_label_set_text(GTK_LABEL(s_bt_name_lbl), bt_name);
    gtk_label_set_text(GTK_LABEL(s_bt_sub_lbl), bt_sub);

    // Bluetooth paired devices
    if (s_bt_devices_box) {
        GList *ch = gtk_container_get_children(GTK_CONTAINER(s_bt_devices_box));
        for (GList *l = ch; l != NULL; l = l->next) gtk_widget_destroy(GTK_WIDGET(l->data));
        g_list_free(ch);

        char *devices = run_shell_output(
            "bluetoothctl devices Paired 2>/dev/null | sed 's/^Device //'");
        int shown = 0;
        if (devices) {
            char *line = strtok(devices, "\n");
            while (line) {
                char mac[32] = "", name[96] = "";
                sscanf(line, "%31s %95[^\n]", mac, name);
                if (strlen(mac) > 0) {
                    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
                    gtk_style_context_add_class(gtk_widget_get_style_context(row), "item-card");

                    GtkWidget *lbl = gtk_label_new(strlen(name) ? name : mac);
                    gtk_style_context_add_class(gtk_widget_get_style_context(lbl), "item-title");
                    gtk_box_pack_start(GTK_BOX(row), lbl, TRUE, TRUE, 0);

                    GtkWidget *btn = gtk_button_new_with_label("Connect");
                    gtk_style_context_add_class(gtk_widget_get_style_context(btn), "action-btn");
                    g_object_set_data_full(G_OBJECT(btn), "bt-mac", strdup(mac), free);
                    g_signal_connect(btn, "clicked", G_CALLBACK(on_bt_device_clicked), NULL);
                    gtk_box_pack_end(GTK_BOX(row), btn, FALSE, FALSE, 0);

                    gtk_box_pack_start(GTK_BOX(s_bt_devices_box), row, FALSE, FALSE, 0);
                    shown++;
                }
                line = strtok(NULL, "\n");
            }
            free(devices);
        }
        if (shown == 0) {
            GtkWidget *empty = gtk_label_new("No paired devices.");
            gtk_style_context_add_class(gtk_widget_get_style_context(empty), "item-subtitle");
            gtk_box_pack_start(GTK_BOX(s_bt_devices_box), empty, FALSE, FALSE, 8);
        }
        gtk_widget_show_all(s_bt_devices_box);
    }

    // Wi-Fi / network
    char *ip_info = run_shell_output("ip -brief addr 2>/dev/null | grep -v LOOPBACK | grep -E 'UP' | head -n 1");
    if (ip_info && strlen(ip_info) > 2) {
        char iface[32] = "", st[16] = "", ip[64] = "";
        sscanf(ip_info, "%31s %15s %63s", iface, st, ip);
        gtk_label_set_text(GTK_LABEL(s_net_title_lbl), iface);
        gtk_label_set_text(GTK_LABEL(s_net_sub_lbl), ip);
    } else {
        gtk_label_set_text(GTK_LABEL(s_net_title_lbl), "Not connected");
        gtk_label_set_text(GTK_LABEL(s_net_sub_lbl), "--");
    }
    free(ip_info);

    char *wifi = run_shell_output("nmcli -t -f ACTIVE,SSID device wifi 2>/dev/null | grep '^yes' | head -n 1 | cut -d: -f2-");
    if (wifi && strlen(wifi) > 0) {
        char buf[96];
        snprintf(buf, sizeof(buf), "Connected · %s", wifi);
        gtk_label_set_text(GTK_LABEL(s_net_sub_lbl), buf);
    }
    free(wifi);

    char *radio = run_shell_output("nmcli radio wifi 2>/dev/null");
    if (radio && s_net_switch)
        gtk_switch_set_active(GTK_SWITCH(s_net_switch), strstr(radio, "enabled") != NULL);
    free(radio);

    s_refreshing = FALSE;
    fit_zone_to_content(&g_zone_right);
}

static gboolean on_bt_toggle_clicked(GtkSwitch *sw, gboolean state, gpointer user_data) {
    if (s_refreshing) return FALSE;
    if (state) run_shell_command("bluetoothctl power on 2>/dev/null");
    else run_shell_command("bluetoothctl power off 2>/dev/null");
    refresh_right_zone();
    return FALSE;
}

static gboolean on_wifi_toggle_clicked(GtkSwitch *sw, gboolean state, gpointer user_data) {
    if (s_refreshing) return FALSE;
    if (state) run_shell_command("nmcli radio wifi on 2>/dev/null");
    else run_shell_command("nmcli radio wifi off 2>/dev/null");
    refresh_right_zone();
    return FALSE;
}

static void on_output_vol_changed(GtkRange *range, gpointer user_data) {
    if (s_refreshing) return;
    int val = (int)gtk_range_get_value(range);
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "~/.config/waybar/scripts/audio/audio_control.sh output set %d", val);
    run_shell_command(cmd);
}

static void on_output_mute_clicked(GtkButton *btn, gpointer user_data) {
    run_shell_command("~/.config/waybar/scripts/audio/audio_control.sh output mute");
}

static void on_input_vol_changed(GtkRange *range, gpointer user_data) {
    if (s_refreshing) return;
    int val = (int)gtk_range_get_value(range);
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "~/.config/waybar/scripts/audio/audio_control.sh input set %d", val);
    run_shell_command(cmd);
}

static void on_input_mute_clicked(GtkButton *btn, gpointer user_data) {
    run_shell_command("~/.config/waybar/scripts/audio/audio_control.sh input mute");
}

static void on_profile_clicked(GtkButton *btn, gpointer user_data) {
    int idx = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "profile-idx"));
    if (idx < 0 || idx > 2) return;
    char cmd[192];
    snprintf(cmd, sizeof(cmd),
             "~/.config/waybar/scripts/power/power_mode.sh %s >/dev/null 2>&1 &",
             s_profile_ids[idx]);
    run_shell_command(cmd);
    g_timeout_add(1800, refresh_right_later, NULL);
}

static gboolean refresh_right_later(gpointer user_data) {
    (void)user_data;
    refresh_right_zone();
    return G_SOURCE_REMOVE;
}

static void on_bt_device_clicked(GtkButton *btn, gpointer user_data) {
    const char *mac = (const char *)g_object_get_data(G_OBJECT(btn), "bt-mac");
    if (!mac) return;
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "bluetoothctl connect %s 2>/dev/null", mac);
    run_shell_command(cmd);
    refresh_right_zone();
}

static void on_power_lock_clicked(GtkButton *btn, gpointer user_data) {
    run_shell_command("~/.config/waybar/scripts/power/power_action.sh lock");
    hide_zone("right");
}

static void on_power_logout_clicked(GtkButton *btn, gpointer user_data) {
    run_shell_command("~/.config/waybar/scripts/power/power_action.sh logout");
}

static void on_power_suspend_clicked(GtkButton *btn, gpointer user_data) {
    run_shell_command("~/.config/waybar/scripts/power/power_action.sh suspend");
    hide_zone("right");
}

static void on_power_reboot_clicked(GtkButton *btn, gpointer user_data) {
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(g_zone_right.window),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_YES_NO,
        "Restart the system?"
    );
    gtk_window_set_title(GTK_WINDOW(dialog), "Restart");
    gint resp = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    if (resp == GTK_RESPONSE_YES) {
        run_shell_command("~/.config/waybar/scripts/power/power_action.sh reboot yes");
    }
}

static void on_power_shutdown_clicked(GtkButton *btn, gpointer user_data) {
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(g_zone_right.window),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_WARNING,
        GTK_BUTTONS_YES_NO,
        "Shut down the system?"
    );
    gtk_window_set_title(GTK_WINDOW(dialog), "Shut Down");
    gint resp = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    if (resp == GTK_RESPONSE_YES) {
        run_shell_command("~/.config/waybar/scripts/power/power_action.sh shutdown yes");
    }
}
