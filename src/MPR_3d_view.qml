// entrance for MPR view
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
// import the VTK module
import VTK 9.0
import QtQuick.Shapes 1.15

// window containing the application
Item {
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
      ScaleBar3D {
        x: rect_a.x + rect_a.width
        y: rect_a.y
        width: 20
        height: rect_a.height
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
      ScaleBar3D {
        x: rect_c.x + rect_c.width
        y: rect_c.y
        width: 20
        height: rect_c.height
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
      ScaleBar3D {
        x: rect_t.x + rect_t.width
        y: rect_t.y
        width: 20
        height: rect_t.height
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
  Shape{
      ShapePath {
          fillColor: "transparent"
          strokeColor: "white"
          strokeWidth: 1
          strokeStyle: ShapePath.SolidLine
          startX: 0; startY: root_3d_win.height *0.5
          PathLine { x: root_3d_win.width; y: root_3d_win.height *0.5 }
          PathMove {x: root_3d_win.width*0.5; y: 0}
          PathLine { x: root_3d_win.width*0.5; y: root_3d_win.height }
      }
  }
}
