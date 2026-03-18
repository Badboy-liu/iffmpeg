import QtQuick
import QtQuick.Window
import QtQuick.Controls

import VideoItem 1.0

Window {
    visible: true
    width: 350
    height: 750
    title: "Hello World"

    VideoItem {
        width: parent.width
        height: parent.height - 60
        id: videoitem
    }

    // 背景框
    Rectangle {
        width: parent.width
        height: 50
        color: "white"
        border.color: "#888"
        radius: 4

        Row{
            anchors.fill: parent       // 填满白色背景框
            anchors.margins: 8
            spacing: 10
            // 真正的输入框
            TextInput {
                id: filePath
                width: parent.width - playBtn.width - 20   // ⭐关键
                height: parent.height
                verticalAlignment: Text.AlignVCenter
            }
            Button {
                width: 80
                id:playBtn
                text: "Play"
                onClicked: {
                    videoitem.setUrl(filePath.text)
                    videoitem.start()
                }
            }
        }

    }


}