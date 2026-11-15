#include "panel_common.h"

static const char *PANEL_CSS =
"* {"
"    font-family: 'JetBrains Mono', 'Inter', 'Adwaita Sans', 'Adwaita Mono', sans-serif;"
"    font-size: 12px;"
"    color: #e6e8eb;"
"}"
""
".panel-window {"
"    background-color: transparent;"
"}"
""
".panel-container {"
"    background-color: rgba(22, 24, 29, 0.88);"
"    border: 1px solid rgba(255, 255, 255, 0.08);"
"    border-radius: 14px;"
"    box-shadow: 0 8px 32px rgba(0, 0, 0, 0.35);"
"    padding: 12px;"
"}"
""
".tab-bar {"
"    background-color: rgba(255, 255, 255, 0.03);"
"    border-radius: 8px;"
"    padding: 3px;"
"    margin-bottom: 12px;"
"    border: 1px solid rgba(255, 255, 255, 0.04);"
"}"
""
".tab-btn {"
"    background: transparent;"
"    border: none;"
"    border-radius: 6px;"
"    color: #9399a2;"
"    padding: 5px 12px;"
"    font-weight: 500;"
"    transition: all 0.2s ease;"
"}"
""
".tab-btn:hover {"
"    color: #e6e8eb;"
"    background-color: rgba(255, 255, 255, 0.05);"
"}"
""
".tab-btn.active {"
"    color: #ffffff;"
"    background-color: rgba(122, 140, 158, 0.25);"
"    border: 1px solid rgba(255, 255, 255, 0.1);"
"}"
""
".face-card {"
"    background: transparent;"
"    padding: 2px;"
"}"
""
".card-section {"
"    background-color: rgba(255, 255, 255, 0.025);"
"    border: 1px solid rgba(255, 255, 255, 0.05);"
"    border-radius: 10px;"
"    padding: 10px;"
"    margin-bottom: 8px;"
"}"
""
".card-title {"
"    font-size: 13px;"
"    font-weight: 600;"
"    color: #ffffff;"
"    margin-bottom: 6px;"
"}"
""
".card-subtitle {"
"    font-size: 11px;"
"    color: #9399a2;"
"}"
""
".action-btn {"
"    background-color: rgba(255, 255, 255, 0.04);"
"    border: 1px solid rgba(255, 255, 255, 0.08);"
"    border-radius: 6px;"
"    color: #e6e8eb;"
"    padding: 6px 12px;"
"    font-weight: 500;"
"    transition: all 0.15s ease;"
"}"
""
".action-btn:hover {"
"    background-color: rgba(255, 255, 255, 0.09);"
"    color: #ffffff;"
"}"
""
".action-btn.primary {"
"    background-color: rgba(122, 140, 158, 0.35);"
"    border-color: rgba(122, 140, 158, 0.5);"
"    color: #ffffff;"
"}"
""
".action-btn.primary:hover {"
"    background-color: rgba(122, 140, 158, 0.5);"
"}"
""
".action-btn.danger {"
"    background-color: rgba(224, 108, 117, 0.15);"
"    border-color: rgba(224, 108, 117, 0.3);"
"    color: #e06c75;"
"}"
""
".action-btn.danger:hover {"
"    background-color: rgba(224, 108, 117, 0.3);"
"    color: #ffffff;"
"}"
""
"entry {"
"    background-color: rgba(0, 0, 0, 0.25);"
"    border: 1px solid rgba(255, 255, 255, 0.08);"
"    border-radius: 6px;"
"    color: #e6e8eb;"
"    padding: 5px 8px;"
"}"
""
"entry:focus {"
"    border-color: rgba(122, 140, 158, 0.6);"
"}"
""
"scale {"
"    padding: 4px 0;"
"}"
""
"scale trough {"
"    background-color: rgba(255, 255, 255, 0.08);"
"    border-radius: 4px;"
"    min-height: 4px;"
"}"
""
"scale highlight {"
"    background-color: #7a8c9e;"
"    border-radius: 4px;"
"}"
""
"scale slider {"
"    background-color: #ffffff;"
"    border-radius: 50%;"
"    min-width: 12px;"
"    min-height: 12px;"
"    margin: -4px 0;"
"}"
""
"scrollbar {"
"    background: transparent;"
"}"
""
"scrollbar slider {"
"    background-color: rgba(255, 255, 255, 0.1);"
"    border-radius: 4px;"
"    min-width: 4px;"
"}"
""
"scrollbar slider:hover {"
"    background-color: rgba(255, 255, 255, 0.25);"
"}"
""
"calendar {"
"    background-color: transparent;"
"    border: none;"
"    color: #e6e8eb;"
"}"
""
"calendar:selected {"
"    background-color: rgba(122, 140, 158, 0.4);"
"    border-radius: 6px;"
"    color: #ffffff;"
"}"
""
"calendar.header {"
"    color: #7a8c9e;"
"    font-weight: 600;"
"}"
""
"calendar.highlight {"
"    color: #e5c07b;"
"    font-weight: bold;"
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
