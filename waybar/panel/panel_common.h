#ifndef PANEL_COMMON_H
#define PANEL_COMMON_H

#include <gtk/gtk.h>
#include <gtk-layer-shell/gtk-layer-shell.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>

#define SOCKET_PATH "/tmp/waybar_zone_panel.sock"

typedef struct {
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *stack;
    GtkWidget *tab_box;
    int is_visible;
    int active_tab;
    guint hide_timer_id;
} ZoneWindow;

extern ZoneWindow g_zone_left;
extern ZoneWindow g_zone_center;
extern ZoneWindow g_zone_right;

// Helpers
void apply_panel_styles(void);
void show_zone(const char *name, int tab_idx);
void hide_zone(const char *name);
void toggle_zone(const char *name);
GtkWidget *create_styled_button(const char *label, const char *extra_class);
GtkWidget *create_tab_button(const char *label, GtkWidget *stack, const char *child_name, int tab_idx, int *active_tab_ptr);
void run_shell_command(const char *cmd);
char *run_shell_output(const char *cmd);

// Zone constructors
void build_left_zone(void);
void build_center_zone(void);
void build_right_zone(void);

// Refresh callbacks
void refresh_left_zone(void);
void refresh_center_zone(void);
void refresh_right_zone(void);

#endif // PANEL_COMMON_H
