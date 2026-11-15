#include "panel_common.h"

// Forward declarations
static void on_bt_toggle_clicked(GtkSwitch *sw, gboolean state, gpointer user_data);
static void on_output_vol_changed(GtkRange *range, gpointer user_data);
static void on_output_mute_clicked(GtkButton *btn, gpointer user_data);
static void on_input_vol_changed(GtkRange *range, gpointer user_data);
static void on_input_mute_clicked(GtkButton *btn, gpointer user_data);
static void on_power_lock_clicked(GtkButton *btn, gpointer user_data);
static void on_power_logout_clicked(GtkButton *btn, gpointer user_data);
static void on_power_suspend_clicked(GtkButton *btn, gpointer user_data);
static void on_power_hibernate_clicked(GtkButton *btn, gpointer user_data);
static void on_power_reboot_clicked(GtkButton *btn, gpointer user_data);
static void on_power_shutdown_clicked(GtkButton *btn, gpointer user_data);

static gboolean on_cpu_graph_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
static gboolean on_mem_graph_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);

// Widgets for Tab 1: CPU
static GtkWidget *s_cpu_temp_lbl = NULL;
static GtkWidget *s_cpu_source_lbl = NULL;
static GtkWidget *s_cpu_util_lbl = NULL;
static GtkWidget *s_cpu_graph = NULL;
static double s_cpu_history[30] = {0};

// Widgets for Tab 2: Memory
static GtkWidget *s_mem_util_lbl = NULL;
static GtkWidget *s_mem_detail_lbl = NULL;
static GtkWidget *s_mem_swap_lbl = NULL;
static GtkWidget *s_mem_graph = NULL;
static double s_mem_history[30] = {0};

// Widgets for Tab 3: Network
static GtkWidget *s_net_ssid_lbl = NULL;
static GtkWidget *s_net_detail_lbl = NULL;
static GtkWidget *s_net_speed_lbl = NULL;
static GtkWidget *s_net_wifi_list = NULL;

// Widgets for Tab 4: Bluetooth
static GtkWidget *s_bt_switch = NULL;
static GtkWidget *s_bt_status_lbl = NULL;
static GtkWidget *s_bt_devices_box = NULL;

// Widgets for Tab 5: Output Audio
static GtkWidget *s_out_dev_lbl = NULL;
static GtkWidget *s_out_vol_scale = NULL;
static GtkWidget *s_out_mute_btn = NULL;
static GtkWidget *s_out_devices_box = NULL;

// Widgets for Tab 6: Input Audio
static GtkWidget *s_in_dev_lbl = NULL;
static GtkWidget *s_in_vol_scale = NULL;
static GtkWidget *s_in_mute_btn = NULL;
static GtkWidget *s_in_devices_box = NULL;

void build_right_zone(void) {
    g_zone_right.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_name(g_zone_right.window, "panel-right");
    gtk_style_context_add_class(gtk_widget_get_style_context(g_zone_right.window), "panel-window");

    // Configure Layer Shell
    gtk_layer_init_for_window(GTK_WINDOW(g_zone_right.window));
    gtk_layer_set_layer(GTK_WINDOW(g_zone_right.window), GTK_LAYER_SHELL_LAYER_TOP);
    gtk_layer_set_anchor(GTK_WINDOW(g_zone_right.window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
    gtk_layer_set_anchor(GTK_WINDOW(g_zone_right.window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
    gtk_layer_set_margin(GTK_WINDOW(g_zone_right.window), GTK_LAYER_SHELL_EDGE_TOP, 37);
    gtk_layer_set_margin(GTK_WINDOW(g_zone_right.window), GTK_LAYER_SHELL_EDGE_RIGHT, 12);
    gtk_layer_set_keyboard_mode(GTK_WINDOW(g_zone_right.window), GTK_LAYER_SHELL_KEYBOARD_MODE_NONE);

    // Outer container
    GtkWidget *container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(container), "panel-container");
    gtk_widget_set_size_request(container, 460, 520);
    gtk_container_add(GTK_CONTAINER(g_zone_right.window), container);

    // Tab bar
    g_zone_right.tab_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    gtk_style_context_add_class(gtk_widget_get_style_context(g_zone_right.tab_box), "tab-bar");
    gtk_box_pack_start(GTK_BOX(container), g_zone_right.tab_box, FALSE, FALSE, 0);

    // Stack for face cards
    g_zone_right.stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(g_zone_right.stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_duration(GTK_STACK(g_zone_right.stack), 150);
    gtk_box_pack_start(GTK_BOX(container), g_zone_right.stack, TRUE, TRUE, 0);

    // Tab buttons
    GtkWidget *b1 = create_tab_button("CPU", g_zone_right.stack, "cpu", 0, &g_zone_right.active_tab);
    GtkWidget *b2 = create_tab_button("RAM", g_zone_right.stack, "memory", 1, &g_zone_right.active_tab);
    GtkWidget *b3 = create_tab_button("Wi-Fi", g_zone_right.stack, "network", 2, &g_zone_right.active_tab);
    GtkWidget *b4 = create_tab_button("BT", g_zone_right.stack, "bluetooth", 3, &g_zone_right.active_tab);
    GtkWidget *b5 = create_tab_button("Audio", g_zone_right.stack, "output", 4, &g_zone_right.active_tab);
    GtkWidget *b6 = create_tab_button("Mic", g_zone_right.stack, "input", 5, &g_zone_right.active_tab);
    GtkWidget *b7 = create_tab_button("Power", g_zone_right.stack, "power", 6, &g_zone_right.active_tab);
    gtk_box_pack_start(GTK_BOX(g_zone_right.tab_box), b1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(g_zone_right.tab_box), b2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(g_zone_right.tab_box), b3, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(g_zone_right.tab_box), b4, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(g_zone_right.tab_box), b5, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(g_zone_right.tab_box), b6, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(g_zone_right.tab_box), b7, TRUE, TRUE, 0);

    /* -------------------------------------------------------------
       Tab 1: CPU Face Card
       ------------------------------------------------------------- */
    GtkWidget *card1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(card1), "face-card");

    GtkWidget *cpu_sec = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_style_context_add_class(gtk_widget_get_style_context(cpu_sec), "card-section");
    gtk_box_pack_start(GTK_BOX(card1), cpu_sec, FALSE, FALSE, 0);

    s_cpu_util_lbl = gtk_label_new("CPU Total Utilization: --%");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_cpu_util_lbl), "card-title");
    gtk_widget_set_halign(s_cpu_util_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(cpu_sec), s_cpu_util_lbl, FALSE, FALSE, 0);

    s_cpu_temp_lbl = gtk_label_new("Temperature: --°C");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_cpu_temp_lbl), "card-subtitle");
    gtk_widget_set_halign(s_cpu_temp_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(cpu_sec), s_cpu_temp_lbl, FALSE, FALSE, 0);

    s_cpu_source_lbl = gtk_label_new("Sensor Source: Detecting...");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_cpu_source_lbl), "card-subtitle");
    gtk_widget_set_halign(s_cpu_source_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(cpu_sec), s_cpu_source_lbl, FALSE, FALSE, 0);

    // CPU Usage Graph
    GtkWidget *g_title = gtk_label_new("Historical Utilization (last 30s):");
    gtk_style_context_add_class(gtk_widget_get_style_context(g_title), "card-subtitle");
    gtk_widget_set_halign(g_title, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(card1), g_title, FALSE, FALSE, 0);

    s_cpu_graph = gtk_drawing_area_new();
    gtk_widget_set_size_request(s_cpu_graph, 420, 180);
    gtk_style_context_add_class(gtk_widget_get_style_context(s_cpu_graph), "card-section");
    g_signal_connect(s_cpu_graph, "draw", G_CALLBACK(on_cpu_graph_draw), NULL);
    gtk_box_pack_start(GTK_BOX(card1), s_cpu_graph, TRUE, TRUE, 0);

    gtk_stack_add_named(GTK_STACK(g_zone_right.stack), card1, "cpu");

    /* -------------------------------------------------------------
       Tab 2: Memory Face Card
       ------------------------------------------------------------- */
    GtkWidget *card2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(card2), "face-card");

    GtkWidget *mem_sec = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_style_context_add_class(gtk_widget_get_style_context(mem_sec), "card-section");
    gtk_box_pack_start(GTK_BOX(card2), mem_sec, FALSE, FALSE, 0);

    s_mem_util_lbl = gtk_label_new("RAM Usage: --% (-- GB / -- GB)");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_mem_util_lbl), "card-title");
    gtk_widget_set_halign(s_mem_util_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(mem_sec), s_mem_util_lbl, FALSE, FALSE, 0);

    s_mem_detail_lbl = gtk_label_new("Available: -- GB | Buffers/Cached: -- GB");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_mem_detail_lbl), "card-subtitle");
    gtk_widget_set_halign(s_mem_detail_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(mem_sec), s_mem_detail_lbl, FALSE, FALSE, 0);

    s_mem_swap_lbl = gtk_label_new("Swap Usage: -- MB / -- MB");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_mem_swap_lbl), "card-subtitle");
    gtk_widget_set_halign(s_mem_swap_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(mem_sec), s_mem_swap_lbl, FALSE, FALSE, 0);

    // RAM Usage Graph
    s_mem_graph = gtk_drawing_area_new();
    gtk_widget_set_size_request(s_mem_graph, 420, 180);
    gtk_style_context_add_class(gtk_widget_get_style_context(s_mem_graph), "card-section");
    g_signal_connect(s_mem_graph, "draw", G_CALLBACK(on_mem_graph_draw), NULL);
    gtk_box_pack_start(GTK_BOX(card2), s_mem_graph, TRUE, TRUE, 0);

    gtk_stack_add_named(GTK_STACK(g_zone_right.stack), card2, "memory");

    /* -------------------------------------------------------------
       Tab 3: Network Face Card
       ------------------------------------------------------------- */
    GtkWidget *card3 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(card3), "face-card");

    GtkWidget *net_sec = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_style_context_add_class(gtk_widget_get_style_context(net_sec), "card-section");
    gtk_box_pack_start(GTK_BOX(card3), net_sec, FALSE, FALSE, 0);

    s_net_ssid_lbl = gtk_label_new("Network: Disconnected");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_net_ssid_lbl), "card-title");
    gtk_widget_set_halign(s_net_ssid_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(net_sec), s_net_ssid_lbl, FALSE, FALSE, 0);

    s_net_detail_lbl = gtk_label_new("Interface: -- | IP: --");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_net_detail_lbl), "card-subtitle");
    gtk_widget_set_halign(s_net_detail_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(net_sec), s_net_detail_lbl, FALSE, FALSE, 0);

    s_net_speed_lbl = gtk_label_new("▼ Down: 0 KB/s  ▲ Up: 0 KB/s");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_net_speed_lbl), "card-subtitle");
    gtk_widget_set_halign(s_net_speed_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(net_sec), s_net_speed_lbl, FALSE, FALSE, 0);

    GtkWidget *wifi_hdr = gtk_label_new("Available Wi-Fi Networks:");
    gtk_style_context_add_class(gtk_widget_get_style_context(wifi_hdr), "card-subtitle");
    gtk_widget_set_halign(wifi_hdr, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(card3), wifi_hdr, FALSE, FALSE, 0);

    GtkWidget *wifi_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(wifi_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(card3), wifi_scroll, TRUE, TRUE, 0);

    s_net_wifi_list = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_add(GTK_CONTAINER(wifi_scroll), s_net_wifi_list);

    gtk_stack_add_named(GTK_STACK(g_zone_right.stack), card3, "network");

    /* -------------------------------------------------------------
       Tab 4: Bluetooth Face Card
       ------------------------------------------------------------- */
    GtkWidget *card4 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(card4), "face-card");

    GtkWidget *bt_hdr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(card4), bt_hdr, FALSE, FALSE, 0);

    GtkWidget *bt_title = gtk_label_new("Bluetooth Devices");
    gtk_style_context_add_class(gtk_widget_get_style_context(bt_title), "card-title");
    gtk_box_pack_start(GTK_BOX(bt_hdr), bt_title, FALSE, FALSE, 0);

    s_bt_switch = gtk_switch_new();
    g_signal_connect(s_bt_switch, "state-set", G_CALLBACK(on_bt_toggle_clicked), NULL);
    gtk_box_pack_end(GTK_BOX(bt_hdr), s_bt_switch, FALSE, FALSE, 0);

    s_bt_status_lbl = gtk_label_new("Status: Initializing");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_bt_status_lbl), "card-subtitle");
    gtk_widget_set_halign(s_bt_status_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(card4), s_bt_status_lbl, FALSE, FALSE, 0);

    GtkWidget *bt_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(bt_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(card4), bt_scroll, TRUE, TRUE, 0);

    s_bt_devices_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_add(GTK_CONTAINER(bt_scroll), s_bt_devices_box);

    gtk_stack_add_named(GTK_STACK(g_zone_right.stack), card4, "bluetooth");

    /* -------------------------------------------------------------
       Tab 5: Output Audio Face Card
       ------------------------------------------------------------- */
    GtkWidget *card5 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(card5), "face-card");

    GtkWidget *out_sec = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(out_sec), "card-section");
    gtk_box_pack_start(GTK_BOX(card5), out_sec, FALSE, FALSE, 0);

    s_out_dev_lbl = gtk_label_new("Output Device: Default Audio Sink");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_out_dev_lbl), "card-title");
    gtk_widget_set_halign(s_out_dev_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(out_sec), s_out_dev_lbl, FALSE, FALSE, 0);

    GtkWidget *vol_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(out_sec), vol_row, FALSE, FALSE, 4);

    s_out_vol_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(s_out_vol_scale), 50);
    g_signal_connect(s_out_vol_scale, "value-changed", G_CALLBACK(on_output_vol_changed), NULL);
    gtk_box_pack_start(GTK_BOX(vol_row), s_out_vol_scale, TRUE, TRUE, 0);

    s_out_mute_btn = gtk_button_new_with_label("Mute");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_out_mute_btn), "action-btn");
    g_signal_connect(s_out_mute_btn, "clicked", G_CALLBACK(on_output_mute_clicked), NULL);
    gtk_box_pack_end(GTK_BOX(vol_row), s_out_mute_btn, FALSE, FALSE, 0);

    GtkWidget *out_dev_hdr = gtk_label_new("Available Output Devices:");
    gtk_style_context_add_class(gtk_widget_get_style_context(out_dev_hdr), "card-subtitle");
    gtk_widget_set_halign(out_dev_hdr, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(card5), out_dev_hdr, FALSE, FALSE, 0);

    GtkWidget *out_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(out_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(card5), out_scroll, TRUE, TRUE, 0);

    s_out_devices_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_add(GTK_CONTAINER(out_scroll), s_out_devices_box);

    gtk_stack_add_named(GTK_STACK(g_zone_right.stack), card5, "output");

    /* -------------------------------------------------------------
       Tab 6: Input Audio Face Card
       ------------------------------------------------------------- */
    GtkWidget *card6 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(card6), "face-card");

    GtkWidget *in_sec = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(in_sec), "card-section");
    gtk_box_pack_start(GTK_BOX(card6), in_sec, FALSE, FALSE, 0);

    s_in_dev_lbl = gtk_label_new("Input Device: Default Audio Source");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_in_dev_lbl), "card-title");
    gtk_widget_set_halign(s_in_dev_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(in_sec), s_in_dev_lbl, FALSE, FALSE, 0);

    GtkWidget *in_vol_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(in_sec), in_vol_row, FALSE, FALSE, 4);

    s_in_vol_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(s_in_vol_scale), 65);
    g_signal_connect(s_in_vol_scale, "value-changed", G_CALLBACK(on_input_vol_changed), NULL);
    gtk_box_pack_start(GTK_BOX(in_vol_row), s_in_vol_scale, TRUE, TRUE, 0);

    s_in_mute_btn = gtk_button_new_with_label("Mute");
    gtk_style_context_add_class(gtk_widget_get_style_context(s_in_mute_btn), "action-btn");
    g_signal_connect(s_in_mute_btn, "clicked", G_CALLBACK(on_input_mute_clicked), NULL);
    gtk_box_pack_end(GTK_BOX(in_vol_row), s_in_mute_btn, FALSE, FALSE, 0);

    GtkWidget *in_dev_hdr = gtk_label_new("Available Input Devices:");
    gtk_style_context_add_class(gtk_widget_get_style_context(in_dev_hdr), "card-subtitle");
    gtk_widget_set_halign(in_dev_hdr, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(card6), in_dev_hdr, FALSE, FALSE, 0);

    GtkWidget *in_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(in_scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(card6), in_scroll, TRUE, TRUE, 0);

    s_in_devices_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_add(GTK_CONTAINER(in_scroll), s_in_devices_box);

    gtk_stack_add_named(GTK_STACK(g_zone_right.stack), card6, "input");

    /* -------------------------------------------------------------
       Tab 7: Power Controls Face Card
       ------------------------------------------------------------- */
    GtkWidget *card7 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_style_context_add_class(gtk_widget_get_style_context(card7), "face-card");

    GtkWidget *p_title = gtk_label_new("Power & Session Controls");
    gtk_style_context_add_class(gtk_widget_get_style_context(p_title), "card-title");
    gtk_box_pack_start(GTK_BOX(card7), p_title, FALSE, FALSE, 0);

    // Lock screen (immediate)
    GtkWidget *btn_lock = gtk_button_new_with_label("   Lock Screen (Immediate)");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_lock), "action-btn");
    g_signal_connect(btn_lock, "clicked", G_CALLBACK(on_power_lock_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(card7), btn_lock, FALSE, FALSE, 2);

    // Logout
    GtkWidget *btn_logout = gtk_button_new_with_label("󰍃   Log Out Session");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_logout), "action-btn");
    g_signal_connect(btn_logout, "clicked", G_CALLBACK(on_power_logout_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(card7), btn_logout, FALSE, FALSE, 2);

    // Suspend
    GtkWidget *btn_suspend = gtk_button_new_with_label("󰒲   Suspend");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_suspend), "action-btn");
    g_signal_connect(btn_suspend, "clicked", G_CALLBACK(on_power_suspend_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(card7), btn_suspend, FALSE, FALSE, 2);

    // Hibernate
    GtkWidget *btn_hibernate = gtk_button_new_with_label("󰋊   Hibernate");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_hibernate), "action-btn");
    g_signal_connect(btn_hibernate, "clicked", G_CALLBACK(on_power_hibernate_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(card7), btn_hibernate, FALSE, FALSE, 2);

    // Restart (with confirmation)
    GtkWidget *btn_reboot = gtk_button_new_with_label("󰜉   Restart System...");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_reboot), "action-btn");
    g_signal_connect(btn_reboot, "clicked", G_CALLBACK(on_power_reboot_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(card7), btn_reboot, FALSE, FALSE, 2);

    // Shutdown (with confirmation)
    GtkWidget *btn_shutdown = gtk_button_new_with_label("⏻   Shut Down System...");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_shutdown), "action-btn");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_shutdown), "danger");
    g_signal_connect(btn_shutdown, "clicked", G_CALLBACK(on_power_shutdown_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(card7), btn_shutdown, FALSE, FALSE, 2);

    gtk_stack_add_named(GTK_STACK(g_zone_right.stack), card7, "power");

    // Connect mouse enter/leave
    gtk_widget_add_events(g_zone_right.window, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
    g_signal_connect(g_zone_right.window, "enter-notify-event", G_CALLBACK(+[](GtkWidget *w, GdkEventCrossing *e, gpointer u) -> gboolean {
        if (g_zone_right.hide_timer_id > 0) {
            g_source_remove(g_zone_right.hide_timer_id);
            g_zone_right.hide_timer_id = 0;
        }
        return FALSE;
    }), NULL);
    g_signal_connect(g_zone_right.window, "leave-notify-event", G_CALLBACK(+[](GtkWidget *w, GdkEventCrossing *e, gpointer u) -> gboolean {
        if (e->detail != GDK_NOTIFY_INFERIOR) {
            if (g_zone_right.hide_timer_id > 0) g_source_remove(g_zone_right.hide_timer_id);
            g_zone_right.hide_timer_id = g_timeout_add(350, +[](gpointer data) -> gboolean {
                hide_zone("right");
                g_zone_right.hide_timer_id = 0;
                return FALSE;
            }, NULL);
        }
        return FALSE;
    }), NULL);

    refresh_right_zone();
}

void refresh_right_zone(void) {
    // 1. Refresh CPU
    char *temp_json = run_shell_output("~/.config/waybar/scripts/system/cpu_temp.sh 2>/dev/null");
    if (temp_json) {
        char temp_val[32] = "N/A";
        char *p = strstr(temp_json, "\"text\": \"");
        if (p) sscanf(p + 9, "%31[^\"]", temp_val);
        char buf[64];
        snprintf(buf, sizeof(buf), "Temperature: %s", temp_val);
        gtk_label_set_text(GTK_LABEL(s_cpu_temp_lbl), buf);

        char *psrc = strstr(temp_json, "Source: ");
        if (psrc) {
            char src_buf[64] = "";
            sscanf(psrc + 8, "%63[^\\]", src_buf);
            char sbuf[96];
            snprintf(sbuf, sizeof(sbuf), "Sensor Source: %s", src_buf);
            gtk_label_set_text(GTK_LABEL(s_cpu_source_lbl), sbuf);
        }
        free(temp_json);
    }

    // CPU utilization calculation from /proc/stat
    static unsigned long long prev_user = 0, prev_nice = 0, prev_system = 0, prev_idle = 0;
    FILE *fp = fopen("/proc/stat", "r");
    if (fp) {
        unsigned long long u = 0, n = 0, s = 0, id = 0;
        if (fscanf(fp, "cpu %llu %llu %llu %llu", &u, &n, &s, &id) == 4) {
            unsigned long long diff_work = (u - prev_user) + (n - prev_nice) + (s - prev_system);
            unsigned long long diff_total = diff_work + (id - prev_idle);
            double pct = (diff_total > 0) ? ((double)diff_work / (double)diff_total) * 100.0 : 0.0;
            prev_user = u; prev_nice = n; prev_system = s; prev_idle = id;

            char ubuf[64];
            snprintf(ubuf, sizeof(ubuf), "CPU Total Utilization: %.1f%%", pct);
            gtk_label_set_text(GTK_LABEL(s_cpu_util_lbl), ubuf);

            // Shift history
            for (int i = 0; i < 29; i++) s_cpu_history[i] = s_cpu_history[i + 1];
            s_cpu_history[29] = pct;
        }
        fclose(fp);
    }
    if (s_cpu_graph) gtk_widget_queue_draw(s_cpu_graph);

    // 2. Refresh Memory
    char *ram_json = run_shell_output("~/.config/waybar/scripts/system/ram_usage.sh 2>/dev/null");
    if (ram_json) {
        char *pt = strstr(ram_json, "\"text\": \"");
        char ram_pct[16] = "0%";
        if (pt) sscanf(pt + 9, "%15[^\"]", ram_pct);

        char *pu = strstr(ram_json, "Used: ");
        char used_str[64] = "";
        if (pu) sscanf(pu + 6, "%63[^\\]", used_str);

        char mbuf[96];
        snprintf(mbuf, sizeof(mbuf), "RAM Usage: %s (%s)", ram_pct, used_str);
        gtk_label_set_text(GTK_LABEL(s_mem_util_lbl), mbuf);

        double pct_val = atof(ram_pct);
        for (int i = 0; i < 29; i++) s_mem_history[i] = s_mem_history[i + 1];
        s_mem_history[29] = pct_val;
        free(ram_json);
    }
    if (s_mem_graph) gtk_widget_queue_draw(s_mem_graph);

    // 3. Refresh Network
    char *net_json = run_shell_output("~/.config/waybar/scripts/network/wifi_status.sh 2>/dev/null");
    if (net_json) {
        char *pssid = strstr(net_json, "Wi-Fi: Connected to ");
        if (pssid) {
            char ssid[64] = "";
            sscanf(pssid + 20, "%63[^\n]", ssid);
            char nbuf[96];
            snprintf(nbuf, sizeof(nbuf), "Wi-Fi: %s", ssid);
            gtk_label_set_text(GTK_LABEL(s_net_ssid_lbl), nbuf);
        } else {
            gtk_label_set_text(GTK_LABEL(s_net_ssid_lbl), "Wi-Fi: Disconnected / Offline");
        }
        free(net_json);
    }

    // 4. Refresh Bluetooth
    char *bt_json = run_shell_output("~/.config/waybar/scripts/bluetooth/bt_status.sh 2>/dev/null");
    if (bt_json) {
        char *pt = strstr(bt_json, "Bluetooth: ");
        if (pt) {
            char bt_s[128] = "";
            sscanf(pt + 11, "%127[^\\]", bt_s);
            gtk_label_set_text(GTK_LABEL(s_bt_status_lbl), bt_s);
        }
        free(bt_json);
    }
}

static gboolean on_cpu_graph_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    GtkAllocation alloc;
    gtk_widget_get_allocation(widget, &alloc);

    cairo_set_source_rgba(cr, 0.08, 0.09, 0.11, 0.95);
    cairo_paint(cr);

    // Draw grid lines
    cairo_set_source_rgba(cr, 1, 1, 1, 0.05);
    cairo_set_line_width(cr, 1.0);
    for (int y = 0; y <= 4; y++) {
        double ypos = 10 + y * (alloc.height - 20) / 4.0;
        cairo_move_to(cr, 10, ypos);
        cairo_line_to(cr, alloc.width - 10, ypos);
        cairo_stroke(cr);
    }

    // Draw curve
    cairo_set_source_rgba(cr, 0.48, 0.55, 0.62, 0.85); // Muted cool blue-grey
    cairo_set_line_width(cr, 2.0);

    double step = ((double)alloc.width - 20.0) / 29.0;
    for (int i = 0; i < 30; i++) {
        double val = s_cpu_history[i];
        if (val < 0.0) val = 0.0;
        if (val > 100.0) val = 100.0;
        double x = 10 + i * step;
        double y = alloc.height - 10 - (val / 100.0) * (alloc.height - 20);
        if (i == 0) cairo_move_to(cr, x, y);
        else cairo_line_to(cr, x, y);
    }
    cairo_stroke_preserve(cr);

    // Fill underneath curve
    cairo_line_to(cr, alloc.width - 10, alloc.height - 10);
    cairo_line_to(cr, 10, alloc.height - 10);
    cairo_close_path(cr);
    cairo_set_source_rgba(cr, 0.48, 0.55, 0.62, 0.15);
    cairo_fill(cr);

    return FALSE;
}

static gboolean on_mem_graph_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    GtkAllocation alloc;
    gtk_widget_get_allocation(widget, &alloc);

    cairo_set_source_rgba(cr, 0.08, 0.09, 0.11, 0.95);
    cairo_paint(cr);

    cairo_set_source_rgba(cr, 1, 1, 1, 0.05);
    cairo_set_line_width(cr, 1.0);
    for (int y = 0; y <= 4; y++) {
        double ypos = 10 + y * (alloc.height - 20) / 4.0;
        cairo_move_to(cr, 10, ypos);
        cairo_line_to(cr, alloc.width - 10, ypos);
        cairo_stroke(cr);
    }

    cairo_set_source_rgba(cr, 0.55, 0.62, 0.72, 0.85);
    cairo_set_line_width(cr, 2.0);

    double step = ((double)alloc.width - 20.0) / 29.0;
    for (int i = 0; i < 30; i++) {
        double val = s_mem_history[i];
        if (val < 0.0) val = 0.0;
        if (val > 100.0) val = 100.0;
        double x = 10 + i * step;
        double y = alloc.height - 10 - (val / 100.0) * (alloc.height - 20);
        if (i == 0) cairo_move_to(cr, x, y);
        else cairo_line_to(cr, x, y);
    }
    cairo_stroke_preserve(cr);

    cairo_line_to(cr, alloc.width - 10, alloc.height - 10);
    cairo_line_to(cr, 10, alloc.height - 10);
    cairo_close_path(cr);
    cairo_set_source_rgba(cr, 0.55, 0.62, 0.72, 0.15);
    cairo_fill(cr);

    return FALSE;
}

static void on_bt_toggle_clicked(GtkSwitch *sw, gboolean state, gpointer user_data) {
    if (state) run_shell_command("bluetoothctl power on 2>/dev/null");
    else run_shell_command("bluetoothctl power off 2>/dev/null");
    refresh_right_zone();
}

static void on_output_vol_changed(GtkRange *range, gpointer user_data) {
    int val = (int)gtk_range_get_value(range);
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "~/.config/waybar/scripts/audio/audio_control.sh output set %d", val);
    run_shell_command(cmd);
}

static void on_output_mute_clicked(GtkButton *btn, gpointer user_data) {
    run_shell_command("~/.config/waybar/scripts/audio/audio_control.sh output mute");
}

static void on_input_vol_changed(GtkRange *range, gpointer user_data) {
    int val = (int)gtk_range_get_value(range);
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "~/.config/waybar/scripts/audio/audio_control.sh input set %d", val);
    run_shell_command(cmd);
}

static void on_input_mute_clicked(GtkButton *btn, gpointer user_data) {
    run_shell_command("~/.config/waybar/scripts/audio/audio_control.sh input mute");
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

static void on_power_hibernate_clicked(GtkButton *btn, gpointer user_data) {
    run_shell_command("~/.config/waybar/scripts/power/power_action.sh hibernate");
    hide_zone("right");
}

static void on_power_reboot_clicked(GtkButton *btn, gpointer user_data) {
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(g_zone_right.window),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_YES_NO,
        "Are you sure you want to Restart the system?"
    );
    gtk_window_set_title(GTK_WINDOW(dialog), "Restart Confirmation");
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
        "Are you sure you want to Shut Down the system?"
    );
    gtk_window_set_title(GTK_WINDOW(dialog), "Shutdown Confirmation");
    gint resp = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    if (resp == GTK_RESPONSE_YES) {
        run_shell_command("~/.config/waybar/scripts/power/power_action.sh shutdown yes");
    }
}
