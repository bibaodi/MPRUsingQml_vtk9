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

  //render window
  VTKRenderWindow {
    id: vtkwindow
    anchors.fill:parent
  }

  // Content area
  GridLayout {
    id: grid_layout
    columns: 2
    rows: 2
    anchors.fill: parent
    rowSpacing:0
    columnSpacing: 0

    VTKRenderItem {
      objectName: "MPRView_A"
      renderWindow: vtkwindow
      focus: true
      Layout.row:  0
      Layout.column: 0
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumWidth: 100
      Layout.preferredWidth: 200
      Text {
          id: label_A
          color: "blue"
          text: "A"
          x: parent.width - width -1.0
          y: parent.height -height + 3.0
          font.bold: true
          font.pointSize: 20
      }
      Rectangle {
            id:rect_a
            border.color: label_A.color
            color: "transparent"
            x:  10
            y: 10
            width:parent.width -20
            height:parent.height-20
      }
    }
    VTKRenderItem {
      objectName: "MPRView_C"
      renderWindow: vtkwindow
      focus: true
      Layout.row:  1
      Layout.column: 0
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumWidth: 100
      Layout.preferredWidth: 200
      Text {
          id: label_C
          color: "red"
          text: "C"
          x: parent.width - width - 3.0
          y: -8.0
          font.bold: true
          font.pointSize: 20
      }
      Rectangle {
            id:rect_c
            border.color: label_C.color
            color: "transparent"
            x:  10
            y: 10
            width:parent.width -20
            height:parent.height-20
      }
    }

    VTKRenderItem {
      objectName: "MPRView_T"
      renderWindow: vtkwindow
      focus: true
      Layout.row:  0
      Layout.column: 1
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumWidth: 100
      Layout.preferredWidth: 200
      Text {
          id: label_T
          color: "green"
          text: "T"
          x: 3.0
          y: parent.height -height + 3.0
          font.bold: true
          font.pointSize: 20
      }
      Rectangle {
            id:rect_t
            border.color: label_T.color
            color: "transparent"
            x:  10
            y: 10
            width:parent.width -20
            height:parent.height-20
      }
    }

    VTKRenderItem {
      objectName: "MPRView_3D"
      renderWindow: vtkwindow
      focus: true
      Layout.row:  1
      Layout.column: 1
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumWidth: 100
      Layout.preferredWidth: 200
      Text {
          id: label_3d
          color: "white"
          text: "3D"
          x: 3.0
          y: -8.0
          font.bold: true
          font.pointSize: 20
      }
    }
  }
}
