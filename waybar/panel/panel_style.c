#include "panel_common.h"

/*
 * The drawer is styled as an extension of the Waybar pill:
 *   - same warm charcoal surface and hairline border
 *   - square top edge, rounded bottom edge (it drops out of the bar)
 *   - inner capsules use the same rgba(0,0,0,.30) + hairline look
 *   - amber is the only accent, same as the active workspace
 */
static const char *PANEL_CSS =
/* Global reset */
"* {"
"  font-family: 'Symbols Nerd Font Mono', 'JetBrains Mono', 'Adwaita Mono', monospace;"
"  font-size: 12px;"
"  font-weight: 500;"
"  color: #c8bfb6;"
"  border: none;"
"  box-shadow: none;"
"  outline: none;"
"  text-shadow: none;"
"}"

/* Transparent layer-shell window; the sheet is drawn by .panel-bg */
".panel-window { background: transparent; }"
"#panel-left, #panel-center, #panel-right { background: transparent; }"

/* The sheet that grows out of the bar */
".panel-bg {"
"  background-color: rgba(28, 22, 16, 0.92);"
"  border-width: 0 1px 1px 1px;"
"  border-style: solid;"
"  border-color: rgba(255, 255, 255, 0.07);"
"  border-radius: 0 0 18px 18px;"
"}"

/* Content is clipped by the animated window while it opens/closes */
".panel-content { background: transparent; }"

/* Viewport used for clipping; never show scrollbars during the grow */
".panel-clip, .panel-clip viewport { background: transparent; border: none; }"
".panel-clip scrollbar { min-width: 0; min-height: 0; opacity: 0; background: transparent; }"

/* ── Tab bar: one inner capsule, exactly like #workspaces/#controls-group ── */
".tab-bar {"
"  background: rgba(0, 0, 0, 0.30);"
"  border: 1px solid rgba(255, 255, 255, 0.05);"
"  border-radius: 9999px;"
"  padding: 3px 4px;"
"  margin: 12px 12px 10px 12px;"
"}"

".icon-tab-btn {"
"  background: transparent;"
"  border-radius: 9999px;"
"  color: #8e8880;"
"  padding: 2px 8px;"
"  font-size: 13px;"
"  min-width: 20px;"
"  min-height: 20px;"
"  transition: background-color 150ms ease, color 150ms ease;"
"}"

".icon-tab-btn:hover {"
"  color: #fff;"
"  background: rgba(255, 255, 255, 0.09);"
"}"

/* Active tab mirrors the active workspace pill in the bar */
".icon-tab-btn.active-pill {"
"  background: rgba(190, 128, 55, 0.90);"
"  color: #fff;"
"  font-size: 11px;"
"  font-weight: 600;"
"  padding: 2px 10px;"
"}"

/* Face cards sit inside a small padded stack */
".panel-stack { margin: 0 12px 12px 12px; }"
".face-card { background: transparent; }"

/* Workspace grid tiles (5 x 2) with a tiny window preview */
".ws-tile {"
"  background: rgba(0, 0, 0, 0.30);"
"  border: 1px solid rgba(255, 255, 255, 0.05);"
"  border-radius: 10px;"
"  color: #c8bfb6;"
"  padding: 2px;"
"  min-height: 48px;"
"  transition: background-color 150ms ease, color 150ms ease;"
"}"
".ws-tile:hover {"
"  background: rgba(255, 255, 255, 0.07);"
"  color: #fff;"
"}"
".ws-tile.active {"
"  background: rgba(190, 128, 55, 0.90);"
"  border-color: rgba(190, 128, 55, 0.40);"
"  color: #fff;"
"}"
".ws-num {"
"  font-size: 13px;"
"  font-weight: 700;"
"  color: #e8e0d6;"
"}"
".ws-tile.active .ws-num { color: #fff; }"
".ws-icon {"
"  font-size: 13px;"
"  color: #c8bfb6;"
"}"
".ws-tile.active .ws-icon { color: #fff; }"
".ws-title {"
"  font-size: 8px;"
"  font-weight: 500;"
"  color: #8e8880;"
"}"
".ws-tile.active .ws-title { color: rgba(255, 255, 255, 0.82); }"

/* ── Section headers ─────────────────────────────────────────────────────── */
".card-header-title {"
"  font-size: 12px;"
"  font-weight: 600;"
"  color: #e8e0d6;"
"  letter-spacing: 0.15px;"
"}"

/* ── Rows: same capsule language as the bar ──────────────────────────────── */
".item-card {"
"  background: rgba(0, 0, 0, 0.30);"
"  border: 1px solid rgba(255, 255, 255, 0.05);"
"  border-radius: 12px;"
"  padding: 8px 10px;"
"  margin-top: 6px;"
"  transition: background-color 150ms ease;"
"}"

".item-card:hover { background: rgba(0, 0, 0, 0.22); }"

".item-title {"
"  font-size: 11.5px;"
"  font-weight: 600;"
"  color: #e8e0d6;"
"}"

".item-subtitle {"
"  font-size: 10.5px;"
"  color: #8e8880;"
"  margin-top: 1px;"
"}"

/* ── Buttons ─────────────────────────────────────────────────────────────── */
".action-btn {"
"  background: rgba(0, 0, 0, 0.30);"
"  border: 1px solid rgba(255, 255, 255, 0.06);"
"  border-radius: 9999px;"
"  color: #c0bab3;"
"  padding: 3px 11px;"
"  font-size: 11px;"
"  font-weight: 500;"
"  transition: background-color 150ms ease, color 150ms ease;"
"}"

".action-btn:hover {"
"  background: rgba(255, 255, 255, 0.07);"
"  color: #fff;"
"}"

".action-btn.primary {"
"  background: rgba(190, 128, 55, 0.90);"
"  border-color: rgba(190, 128, 55, 0.40);"
"  color: #fff;"
"}"

".action-btn.primary:hover { background: rgba(205, 143, 66, 0.98); }"

".action-btn.danger {"
"  background: rgba(200, 70, 70, 0.12);"
"  border-color: rgba(200, 70, 70, 0.25);"
"  color: #c05858;"
"}"

".action-btn.danger:hover {"
"  background: rgba(200, 70, 70, 0.28);"
"  color: #fff;"
"}"

/* ── Toggle switch: black capsule, amber when on ─────────────────────────── */
"switch {"
"  background: rgba(0, 0, 0, 0.30);"
"  border: 1px solid rgba(255, 255, 255, 0.06);"
"  border-radius: 9999px;"
"  min-width: 40px;"
"  min-height: 22px;"
"  transition: background-color 160ms ease;"
"}"

"switch:checked { background: rgba(190, 128, 55, 0.90); }"

"switch slider {"
"  background: #f0ece7;"
"  border-radius: 50%;"
"  min-width: 16px;"
"  min-height: 16px;"
"  margin: 2px;"
"}"

/* ── Sliders ─────────────────────────────────────────────────────────────── */
"scale trough {"
"  background: rgba(0, 0, 0, 0.35);"
"  border: 1px solid rgba(255, 255, 255, 0.05);"
"  border-radius: 9999px;"
"  min-height: 5px;"
"}"

"scale highlight {"
"  background: rgba(190, 128, 55, 0.90);"
"  border-radius: 9999px;"
"}"

"scale slider {"
"  background: #f0ece7;"
"  border-radius: 50%;"
"  min-width: 12px;"
"  min-height: 12px;"
"  margin: -4px 0;"
"}"

/* ── Scrollbar ───────────────────────────────────────────────────────────── */
"scrollbar { background: transparent; }"
"scrollbar slider {"
"  background: rgba(255, 255, 255, 0.09);"
"  border-radius: 9999px;"
"  min-width: 3px;"
"  min-height: 24px;"
"}"

/* ── Entry ───────────────────────────────────────────────────────────────── */
"entry {"
"  background: rgba(0, 0, 0, 0.30);"
"  border: 1px solid rgba(255, 255, 255, 0.06);"
"  border-radius: 9999px;"
"  color: #c0bab3;"
"  caret-color: #d4a85a;"
"  padding: 3px 10px;"
"  font-size: 11px;"
"  transition: border-color 150ms ease;"
"}"

"entry:focus { border-color: rgba(190, 128, 55, 0.55); }"
"entry selection { background: rgba(190, 128, 55, 0.45); color: #fff; }"

/* ── Checkbox ────────────────────────────────────────────────────────────── */
"checkbutton check {"
"  background: rgba(0, 0, 0, 0.30);"
"  border: 1px solid rgba(255, 255, 255, 0.10);"
"  border-radius: 5px;"
"  min-width: 14px;"
"  min-height: 14px;"
"}"

"checkbutton check:checked {"
"  background: rgba(190, 128, 55, 0.90);"
"  border-color: rgba(190, 128, 55, 0.90);"
"  color: #fff;"
"}"

/* ── Calendar ────────────────────────────────────────────────────────────── */
"calendar {"
"  background: transparent;"
"  color: #c8bfb6;"
"  font-size: 11px;"
"  border-radius: 12px;"
"  padding: 2px;"
"}"

"calendar:selected {"
"  background: rgba(190, 128, 55, 0.90);"
"  border-radius: 9999px;"
"  color: #fff;"
"}"

"calendar.header {"
"  color: #e8e0d6;"
"  font-weight: 600;"
"}"

"calendar.highlight {"
"  color: #d4a85a;"
"  font-weight: 600;"
"}"

"calendar:indeterminate { color: #6e6660; }"

/* ── Tooltip / dialogs ───────────────────────────────────────────────────── */
"tooltip {"
"  background: rgba(28, 22, 16, 0.96);"
"  border: 1px solid rgba(255, 255, 255, 0.08);"
"  border-radius: 10px;"
"  color: #c8bfb6;"
"  padding: 5px 9px;"
"  font-size: 11px;"
"}"

"dialog, messagedialog {"
"  background-color: #1c1610;"
"  color: #c8bfb6;"
"}";

void apply_panel_styles(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, PANEL_CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

void run_shell_command(const char *cmd) {
    if (!cmd) return;
    int ret = system(cmd);
    (void)ret;
}

char *run_shell_output(const char *cmd) {
    if (!cmd) return strdup("");
    FILE *fp = popen(cmd, "r");
    if (!fp) return strdup("");

    char buffer[4096];
    size_t cap = 4096;
    size_t len = 0;
    char *result = (char *)malloc(cap);
    if (!result) { pclose(fp); return strdup(""); }
    result[0] = '\0';

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t blen = strlen(buffer);
        if (len + blen + 1 >= cap) {
            cap *= 2;
            char *n = (char *)realloc(result, cap);
            if (!n) break;
            result = n;
        }
        memcpy(result + len, buffer, blen);
        len += blen;
        result[len] = '\0';
    }
    pclose(fp);
    return result;
}
