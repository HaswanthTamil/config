import QtQuick
import QtQuick.Layouts

Column {
    id: root
    property string title: ""
    property var items: []
    property int flatOffset: 0
    property int selectedIndex: -1
    signal activated(int flatIndex)
    signal hovered(int flatIndex)

    width: parent ? parent.width : 400
    spacing: 2

    Text {
        text: root.title
        color: "#8fa1b3"
        font.family: "JetBrains Mono"
        font.pixelSize: 11
        font.weight: Font.DemiBold
        leftPadding: 8
        topPadding: 6
        bottomPadding: 2
    }

    Repeater {
        model: root.items || []
        delegate: Rectangle {
            required property var modelData
            required property int index
            property int flatIndex: root.flatOffset + index
            property bool selected: root.selectedIndex === flatIndex
            width: root.width
            height: 46
            radius: 8
            color: selected ? Qt.rgba(0.55, 0.67, 0.78, 0.18) : (ma.containsMouse ? Qt.rgba(1,1,1,0.04) : "transparent")
            border.color: selected ? Qt.rgba(0.55, 0.67, 0.78, 0.25) : "transparent"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 10

                Text {
                    text: modelData.iconText || "•"
                    color: "#c9d4de"
                    font.family: "JetBrains Mono"
                    font.pixelSize: 16
                    Layout.preferredWidth: 22
                    horizontalAlignment: Text.AlignHCenter
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    Text {
                        text: modelData.title || ""
                        color: "#e8edf2"
                        font.family: "Inter"
                        font.pixelSize: 13
                        font.weight: Font.Medium
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    Text {
                        text: modelData.subtitle || ""
                        color: "#7f8a96"
                        font.family: "Inter"
                        font.pixelSize: 11
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                        visible: (modelData.subtitle || "").length > 0
                    }
                }

                Text {
                    visible: modelData.type === "calc"
                    text: "Enter to copy"
                    color: "#6b7785"
                    font.family: "Inter"
                    font.pixelSize: 10
                }
            }

            MouseArea {
                id: ma
                anchors.fill: parent
                hoverEnabled: true
                onEntered: root.hovered(flatIndex)
                onClicked: root.activated(flatIndex)
            }
        }
    }
}
