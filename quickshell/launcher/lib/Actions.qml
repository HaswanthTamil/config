pragma Singleton
import QtQuick
import Quickshell
import Quickshell.Io

Singleton {
    id: root

    function execute(item, cb) {
        if (!item) { if (cb) cb(false); return; }
        var t = item.type;
        if (t === "app") {
            launchDetached(item.exec || "", item.terminal || false);
            if (cb) cb(true);
        } else if (t === "file" || t === "dir") {
            openPath(item.path || "");
            if (cb) cb(true);
        } else if (t === "calc") {
            copyText(item.value != null ? String(item.value) : (item.title || ""));
            if (cb) cb(true);
        } else {
            if (cb) cb(false);
        }
    }

    function sh(cmd) {
        var p = Qt.createQmlObject('import Quickshell.Io; Process { }', root, "dynproc");
        p.command = ["bash", "-lc", cmd];
        p.running = true;
        p.onExited.connect(function(){ p.destroy(); });
    }

    function launchDetached(execCmd, terminal) {
        if (!execCmd) return;
        // Strip desktop field codes
        var cmd = execCmd.replace(/%[fFuUdDnNickvm]/g, "").trim();
        if (terminal) cmd = "kitty -e " + cmd;
        sh("nohup " + cmd + " >/dev/null 2>&1 &");
    }

    function openPath(path) {
        if (!path) return;
        sh("xdg-open " + JSON.stringify(path) + " >/dev/null 2>&1 &");
    }

    function copyText(text) {
        if (text == null) text = "";
        sh("printf %s " + JSON.stringify(String(text)) + " | wl-copy 2>/dev/null || printf %s " + JSON.stringify(String(text)) + " | xclip -selection clipboard 2>/dev/null || true");
    }
}
