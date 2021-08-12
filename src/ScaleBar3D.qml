import QtQuick 2.4
import QtQuick.Shapes 1.15


Rectangle {
    id: root
    color: "transparent"
    Shape{
        ShapePath {
            id: shapepath
            fillColor: "transparent"
            strokeColor: "white"
            strokeWidth: 2
            strokeStyle: ShapePath.SolidLine
            property int scale_len: 100 //to be filled by outside
            property int x0: 0
            property int y0: (parent.height - scale_len) *0.5
            property int y1: (parent.height + scale_len) *0.5
            startX: 0; startY: (parent.height - scale_len) *0.5
            PathLine { x: 5; y: shapepath.y0 }
            PathLine { x: 5; y: shapepath.y1 }
            PathLine { x: 0; y: shapepath.y1 }
            function f() {
                console.log("y0:",  y0);
            }
        }
    }
    Text {
        id: scale_content
        color: "white"
        text: "10.0mm"
        anchors.right: parent.right
        //anchors.verticalCenter: parent.verticalCenter
        y: shapepath.y1+2
        font.pointSize: 9
    }
}

