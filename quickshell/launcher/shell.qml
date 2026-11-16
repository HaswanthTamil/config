import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Quickshell
import Quickshell.Io
import Quickshell.Wayland
import "ui"
import "lib"

ShellRoot {
    id: root

    property bool launcherVisible: true
    property string query: ""
    property int selectedIndex: 0
    property var groupedResults: ({ apps: [], files: [], calc: [] })
    property var flatResults: []

    function openLauncher() {
        launcherVisible = true;
        query = "";
        selectedIndex = 0;
        if (loader.item && loader.item.searchField) loader.item.searchField.text = "";
        orchestrator.search("");
        focusTimer.restart();
    }

    function closeLauncher() {
        launcherVisible = false;
        query = "";
    }

    function toggleLauncher() {
        if (launcherVisible) closeLauncher(); else openLauncher();
    }

    Timer {
        id: focusTimer
        interval: 50
        repeat: false
        onTriggered: {
            if (loader.item && loader.item.searchField) {
                loader.item.searchField.forceActiveFocus();
                loader.item.searchField.selectAll();
            }
        }
    }

    function rebuildFlat() {
        var flat = [];
        var groups = [
            { key: "calc", label: "Calculator", items: groupedResults.calc || [] },
            { key: "apps", label: "Applications", items: groupedResults.apps || [] },
            { key: "files", label: "Files", items: groupedResults.files || [] }
        ];
        for (var g = 0; g < groups.length; g++) {
            var grp = groups[g];
            for (var i = 0; i < grp.items.length; i++) {
                flat.push({ group: grp.label, groupKey: grp.key, item: grp.items[i] });
            }
        }
        flatResults = flat;
        if (flatResults.length === 0) selectedIndex = 0;
        else if (selectedIndex >= flatResults.length) selectedIndex = flatResults.length - 1;
        else if (selectedIndex < 0) selectedIndex = 0;
    }

    function executeSelected() {
        if (flatResults.length === 0) return;
        var idx = Math.max(0, Math.min(selectedIndex, flatResults.length - 1));
        var entry = flatResults[idx];
        if (!entry || !entry.item) return;
        Actions.execute(entry.item, function(ok) {
            if (ok !== false) closeLauncher();
        });
    }

    SearchOrchestrator {
        id: orchestrator
        onResultsReady: function(res) {
            root.groupedResults = res || { apps: [], files: [], calc: [] };
            root.rebuildFlat();
        }
    }

    IpcHandler {
        target: "launcher"
        function toggle() { root.toggleLauncher(); }
        function open() { root.openLauncher(); }
        function close() { root.closeLauncher(); }
    }

    LazyLoader {
        id: loader
        active: root.launcherVisible
        component: PanelWindow {
            id: win
            property alias searchField: searchInput
            visible: root.launcherVisible
            color: "transparent"
            WlrLayershell.layer: WlrLayer.Overlay
            WlrLayershell.keyboardFocus: WlrKeyboardFocus.Exclusive
            WlrLayershell.namespace: "unified-launcher"
            exclusionMode: ExclusionMode.Ignore
            anchors { top: true; bottom: true; left: true; right: true }

            Rectangle {
                anchors.fill: parent
                color: Qt.rgba(0, 0, 0, 0.10)
                MouseArea { anchors.fill: parent; onClicked: root.closeLauncher() }

                Rectangle {
                    id: panel
                    width: Math.min(720, parent.width - 80)
                    height: Math.min(560, contentCol.implicitHeight + 28)
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: Math.max(70, parent.height * 0.12)
                    radius: 14
                    color: Qt.rgba(0.11, 0.12, 0.14, 0.92)
                    border.color: Qt.rgba(1, 1, 1, 0.08)
                    border.width: 1

                    MouseArea { anchors.fill: parent; acceptedButtons: Qt.AllButtons; onPressed: function(mouse){ mouse.accepted = true; } }

                    ColumnLayout {
                        id: contentCol
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 10

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 46
                            radius: 10
                            color: Qt.rgba(1, 1, 1, 0.04)
                            border.color: searchInput.activeFocus ? Qt.rgba(0.58, 0.69, 0.80, 0.75) : Qt.rgba(1, 1, 1, 0.06)
                            border.width: 1

                            TextInput {
                                id: searchInput
                                anchors.fill: parent
                                anchors.leftMargin: 14
                                anchors.rightMargin: 14
                                verticalAlignment: Text.AlignVCenter
                                color: "#e8edf2"
                                selectionColor: Qt.rgba(0.55, 0.67, 0.78, 0.4)
                                selectedTextColor: "#ffffff"
                                font.family: "Inter"
                                font.pixelSize: 15
                                focus: true
                                selectByMouse: true
                                clip: true

                                Text {
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: "Search apps, files, or calculate..."
                                    color: "#7f8a96"
                                    font: parent.font
                                    visible: parent.text.length === 0
                                }

                                onTextChanged: {
                                    root.query = text;
                                    root.selectedIndex = 0;
                                    orchestrator.search(text);
                                }

                                Keys.onPressed: function(event) {
                                    if (event.key === Qt.Key_Escape) {
                                        root.closeLauncher();
                                        event.accepted = true;
                                    } else if (event.key === Qt.Key_Down) {
                                        if (root.flatResults.length > 0)
                                            root.selectedIndex = Math.min(root.selectedIndex + 1, root.flatResults.length - 1);
                                        event.accepted = true;
                                    } else if (event.key === Qt.Key_Up) {
                                        if (root.flatResults.length > 0)
                                            root.selectedIndex = Math.max(root.selectedIndex - 1, 0);
                                        event.accepted = true;
                                    } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                                        root.executeSelected();
                                        event.accepted = true;
                                    } else if (event.key === Qt.Key_A && (event.modifiers & Qt.ControlModifier)) {
                                        searchInput.selectAll();
                                        event.accepted = true;
                                    }
                                }
                            }
                        }

                        Text {
                            visible: root.query.length > 0
                            text: "Results: " + root.flatResults.length + " (calc " + ((root.groupedResults.calc||[]).length) + ", apps " + ((root.groupedResults.apps||[]).length) + ", files " + ((root.groupedResults.files||[]).length) + ")"
                            color: "#6b7785"
                            font.family: "Inter"
                            font.pixelSize: 10
                            leftPadding: 4
                        }

                        Flickable {
                            id: flick
                            Layout.fillWidth: true
                            Layout.preferredHeight: Math.min(420, Math.max(40, resultsCol.implicitHeight))
                            contentHeight: resultsCol.implicitHeight
                            clip: true
                            boundsBehavior: Flickable.StopAtBounds

                            Column {
                                id: resultsCol
                                width: flick.width
                                spacing: 4

                                ResultSection {
                                    visible: (root.groupedResults.calc || []).length > 0
                                    width: parent.width
                                    title: "Calculator"
                                    items: root.groupedResults.calc || []
                                    flatOffset: 0
                                    selectedIndex: root.selectedIndex
                                    onActivated: function(i) { root.selectedIndex = i; root.executeSelected(); }
                                    onHovered: function(i) { root.selectedIndex = i; }
                                }

                                ResultSection {
                                    visible: (root.groupedResults.apps || []).length > 0
                                    width: parent.width
                                    title: "Applications"
                                    items: root.groupedResults.apps || []
                                    flatOffset: (root.groupedResults.calc || []).length
                                    selectedIndex: root.selectedIndex
                                    onActivated: function(i) { root.selectedIndex = i; root.executeSelected(); }
                                    onHovered: function(i) { root.selectedIndex = i; }
                                }

                                ResultSection {
                                    visible: (root.groupedResults.files || []).length > 0
                                    width: parent.width
                                    title: "Files"
                                    items: root.groupedResults.files || []
                                    flatOffset: (root.groupedResults.calc || []).length + (root.groupedResults.apps || []).length
                                    selectedIndex: root.selectedIndex
                                    onActivated: function(i) { root.selectedIndex = i; root.executeSelected(); }
                                    onHovered: function(i) { root.selectedIndex = i; }
                                }

                                Text {
                                    visible: root.query.length > 0 && root.flatResults.length === 0
                                    text: "No results"
                                    color: "#7f8a96"
                                    font.family: "Inter"
                                    font.pixelSize: 13
                                    leftPadding: 8
                                    topPadding: 10
                                }

                                Text {
                                    visible: root.query.length === 0
                                    text: "Type to search applications, files, or evaluate math expressions"
                                    color: "#6b7785"
                                    font.family: "Inter"
                                    font.pixelSize: 12
                                    leftPadding: 8
                                    topPadding: 10
                                }
                            }
                        }
                    }
                }
            }

            Component.onCompleted: focusTimer.restart()
        }
    }

    Component.onCompleted: openLauncher()
}
