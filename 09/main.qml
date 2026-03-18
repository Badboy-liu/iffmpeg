import QtQuick
import QtQuick.Window
import QtQuick.Controls

import VideoItem 1.0

Window {
    visible: true
    width: 650
    height: 1400
    title: "Hello World"

    VideoItem {
        width: 592
        height: 1280
        id: videoitem
        anchors.fill: parent
    }

    Button {
        x: 29
        y: 27
        text: "Play"

        onClicked: {
            videoitem.setUrl("E:\\project\\cpp\\iqt\\input.mp4")
            videoitem.start()
        }
    }
}