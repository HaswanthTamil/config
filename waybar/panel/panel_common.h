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
    GtkWidget *fixed;
    GtkWidget *container;   // background sheet (animated height)
    GtkWidget *clip;        // viewport that clips the full-height content
    GtkWidget *box;         // content (full height, clipped while growing)
    GtkWidget *stack;
    GtkWidget *tab_box;
    int is_visible;
    int active_tab;
    guint hide_timer_id;
    int width;
    int height;
    int anchor;             // -1 = left, 0 = center, 1 = right
    int current_height;
    guint anim_id;
    gint64 anim_start;
    int anim_from;
    int anim_to;
    int animating;
} ZoneWindow;

extern ZoneWindow g_zone_left;
extern ZoneWindow g_zone_center;
extern ZoneWindow g_zone_right;

// Helpers
void apply_panel_styles(void);
GtkWidget *create_panel_shell(ZoneWindow *zw, const char *name, int width, int height, int anchor);
void animate_zone(ZoneWindow *zw, int target_height);
void fit_zone_to_content(ZoneWindow *zw);
void on_zone_stack_child_changed(GtkStack *stack, GParamSpec *pspec, gpointer user_data);
void show_zone(const char *name, int tab_idx);
void hide_zone(const char *name);
void toggle_zone(const char *name);
GtkWidget *create_styled_button(const char *label, const char *extra_class);
GtkWidget *create_icon_tab_button(const char *icon, const char *label, GtkWidget *stack, const char *child_name, int tab_idx, int *active_tab_ptr);
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
