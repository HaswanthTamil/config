import QtQuick
import Quickshell.Io

Item {
    id: root
    signal resultsReady(var results)

    property string currentQuery: ""
    property int generation: 0
    property var pending: ({ apps: undefined, files: undefined, calc: undefined })
    property string baseDir: "/home/haswanthtamil/.config/quickshell/launcher/providers/"

    Timer {
        id: debounce
        interval: 120
        repeat: false
        onTriggered: root.runSearch(root.currentQuery, root.generation)
    }

    function log(msg) {
        var p = Qt.createQmlObject('import Quickshell.Io; Process {}', root, "logp");
        p.command = ["bash", "-lc", "printf '%s\\n' " + JSON.stringify(String(msg)) + " >> /tmp/qs-launcher.log"];
        p.running = true;
        p.onExited.connect(function(){ p.destroy(); });
    }

    function search(q) {
        currentQuery = q || "";
        generation++;
        debounce.restart();
    }

    function emptyResults() { return { apps: [], files: [], calc: [] }; }

    function runSearch(q, gen) {
        if (q.trim().length === 0) {
            resultsReady(emptyResults());
            return;
        }
        pending = { apps: undefined, files: undefined, calc: undefined };
        runProvider("calc", baseDir + "calc.py", q, gen);
        runProvider("apps", baseDir + "search_apps.py", q, gen);
        runProvider("files", baseDir + "search_files.py", q, gen);
    }

    function maybeEmit(gen) {
        if (gen !== generation) return;
        if (pending.apps === undefined || pending.files === undefined || pending.calc === undefined) return;
        var seen = {};
        function dedupe(arr) {
            var out = [];
            for (var i = 0; i < (arr || []).length; i++) {
                var it = arr[i];
                var k = (it.type||"") + "|" + (it.title||"") + "|" + (it.path||it.exec||it.value||"");
                if (seen[k]) continue;
                seen[k] = 1;
                out.push(it);
            }
            return out;
        }
        var res = { calc: dedupe(pending.calc||[]), apps: dedupe(pending.apps||[]), files: dedupe(pending.files||[]) };
        log("emit q=" + currentQuery + " calc=" + res.calc.length + " apps=" + res.apps.length + " files=" + res.files.length);
        resultsReady(res);
    }

    function parseJson(buf) {
        var s = (buf || "").trim();
        if (!s) return [];
        try { var v = JSON.parse(s); return Array.isArray(v) ? v : []; } catch (e) {}
        var lines = s.split("\n");
        for (var i = lines.length - 1; i >= 0; i--) {
            var l = lines[i].trim();
            if (!l) continue;
            try { var v2 = JSON.parse(l); return Array.isArray(v2) ? v2 : []; } catch (e2) {}
        }
        return [];
    }

    Component {
        id: procComp
        Process {
            id: p
            property string key: ""
            property int gen: 0
            property string queryStr: ""
            stdout: StdioCollector { id: out }
            stderr: StdioCollector { id: err }
            onExited: function(code) {
                if (gen === root.generation) {
                    var text = (out.text !== undefined ? out.text : "");
                    pending[key] = root.parseJson(text);
                    root.maybeEmit(gen);
                }
                p.destroy();
            }
        }
    }

    function runProvider(key, script, q, gen) {
        var p = procComp.createObject(root, { key: key, gen: gen, queryStr: q });
        p.command = ["python3", script, q];
        p.running = true;
    }
}
