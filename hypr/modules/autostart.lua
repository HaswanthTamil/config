-------------------
---- AUTOSTART ----
-------------------

-- See https://wiki.hypr.land/configuring/core/autostart/

-- Autostart necessary processes (like notifications daemons, status bars, etc.)
-- Or execute your favorite apps at launch like this:
--
hl.on("hyprland.start", function () 
  hl.exec_cmd("dunst")
  hl.exec_cmd("nm-applet")
  hl.exec_cmd("waybar")
  hl.exec_cmd(os.getenv("HOME") .. "/.config/waybar/panel/zone-panel")
  hl.exec_cmd("python3 " .. os.getenv("HOME") .. "/.config/waybar/scripts/center/reminders_daemon.py")
  hl.exec_cmd("hyprpaper")
end)