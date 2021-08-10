// import related modules
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
// import the VTK module
import VTK 9.0

// window containing the application
Window {
  // title of the application
  title: qsTr("VTK QtQuick App")
  width: 800
  height: 800
  color: palette.window

  SystemPalette {
    id: palette
    colorGroup: SystemPalette.Active
  }

  // Content area
  RowLayout {
      id:layout_row
      anchors.fill: parent
    spacing:0
    ColumnLayout{
        id:row_top
        spacing: 0
        Rectangle {
            id: leftop
            Layout.alignment: Qt.AlignCenter
            color: "black"
            Layout.preferredWidth: layout_row.width*0.5
            Layout.preferredHeight: layout_row.height*0.5
            border.color: "white"

            Text {
                id: label_A
                color: "blue"
                text: "A"
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                font.bold: true
                font.pointSize: 20
            }
        }

        Rectangle {
            id:leftbottom
            Layout.alignment: Qt.AlignBottom
            Layout.fillHeight: true
            color: "black"
            Layout.preferredWidth: layout_row.width*0.5
            Layout.preferredHeight: layout_row.height*0.5
                        border.color: "white"
            Text {
                id: label_C
                color: "red"
                text: "C"
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottomMargin: 0
                font.bold: true
                font.pointSize: 20
            }
        }
    }
    ColumnLayout{
        id:row_bottom
        spacing: 0
        Rectangle {
            Layout.alignment: Qt.AlignCenter
            color: "black"
            Layout.preferredWidth: layout_row.width*0.5
            Layout.preferredHeight: layout_row.height*0.5
                        border.color: "white"
            Text {
                id: label_T
                color: "green"
                text: "T"
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                font.bold: true
                font.pointSize: 20
            }
        }

        Rectangle {
            Layout.alignment: Qt.AlignBottom
            Layout.fillHeight: true
            color: "black"
            Layout.preferredWidth: layout_row.width*0.5
            Layout.preferredHeight: layout_row.height*0.5
                        border.color: "white"
                        Text {
                            id: label_3d
                            color: "white"
                            text: "3D"
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottomMargin: 0
                            font.bold: true
                            font.pointSize: 20
                        }
        }
    }
  }

  // a rectangle in the middle of the content area
  Rectangle {
    width: 300
    height: 50
    color: "blue"
    border.color: "red"
    border.width: 5
    radius: 10
    Text {
        id: label
        color: "white"
        wrapMode: Text.WordWrap
        text: "Custom QML rectangle & text"
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 10
        width: 100
    }
  }


  VTKRenderWindow {
    id: vtkwindow
    width: 400
    height: 400
  }

  VTKRenderItem {
    objectName: "ConeView"
    x: 100
    y: 100
    width: 200
    height: 200
    renderWindow: vtkwindow
    focus: true
  }
}
