---------------
---- INPUT ----
---------------

hl.config({
    input = {
        kb_layout  = "us",
        kb_variant = "",
        kb_model   = "",
        kb_options = "",
        kb_rules   = "",

        follow_mouse = 1,

        sensitivity = 0.6, -- -1.0 - 1.0, 0 means no modification.

        touchpad = {
            natural_scroll = true,
        },
    },
})

-- Workspaces arranged as a 2x5 grid illusion (1-5 top row, 6-10 bottom row).
-- Left/right: move within the current row (1-5 or 6-10).
-- Up/down: jump by 5 with wrap (mod 10 in 1..10 space).
-- Wired to scripts to detect current workspace and switch accordingly.
local wsLeft  = os.getenv("HOME") .. "/.config/hypr/scripts/ws_grid_left.sh"
local wsRight = os.getenv("HOME") .. "/.config/hypr/scripts/ws_grid_right.sh"
local wsUp    = os.getenv("HOME") .. "/.config/hypr/scripts/ws_grid_up.sh"

hl.gesture({
    fingers = 3,
    direction = "right",
    action = function()
        hl.exec_cmd(wsLeft)
    end
})

hl.gesture({
    fingers = 3,
    direction = "left",
    action = function()
        hl.exec_cmd(wsRight)
    end
})

hl.gesture({
    fingers = 3,
    direction = "up",
    action = function()
        hl.exec_cmd(wsUp)
    end
})

hl.gesture({
    fingers = 3,
    direction = "down",
    action = function()
        hl.exec_cmd(wsUp)
    end
})

-- Example per-device config
-- See https://wiki.hypr.land/configuring/core/devices/ for more
hl.device({
    name        = "epic-mouse-v1",
    sensitivity = 0.5,
})
