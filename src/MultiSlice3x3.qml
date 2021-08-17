// import related modules
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
// import the VTK module
import VTK 9.0
import QtQuick.Shapes 1.15

// window containing the application
Window {
  // title of the application
  title: qsTr("VTK QtQuick App")
  id: root_3d_win
  width: 800
  height: 800
  color: palette.window
  property color outline_color: "red"

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
    columns: 3
    rows: 3
    anchors.fill: parent
    rowSpacing:0
    columnSpacing: 0

    MultiSliceRenderItem {
      Layout.row:  0
      Layout.column: 0
      objectName: "MultiSlice"+Layout.row+Layout.column
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumWidth: 100
      Layout.preferredWidth: 200
    }

    MultiSliceRenderItem {
      Layout.row:  0
      Layout.column: 1
      objectName: "MultiSlice"+Layout.row+Layout.column
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumWidth: 100
      Layout.preferredWidth: 200
    }
    MultiSliceRenderItem {
      Layout.row:  0
      Layout.column: 2
      objectName: "MultiSlice"+Layout.row+Layout.column
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumWidth: 100
      Layout.preferredWidth: 200
    }

    MultiSliceRenderItem {
      Layout.row:  1
      Layout.column: 0
      objectName: "MultiSlice"+Layout.row+Layout.column
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumWidth: 100
      Layout.preferredWidth: 200
    }
    MultiSliceRenderItem {
      Layout.row:  1
      Layout.column: 1
      objectName: "MultiSlice"+Layout.row+Layout.column
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumWidth: 100
      Layout.preferredWidth: 200
    }
    MultiSliceRenderItem {
      Layout.row:  1
      Layout.column: 2
      objectName: "MultiSlice"+Layout.row+Layout.column
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumWidth: 100
      Layout.preferredWidth: 200
    }
    MultiSliceRenderItem {
      Layout.row:  2
      Layout.column: 0
      objectName: "MultiSlice"+Layout.row+Layout.column
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumWidth: 100
      Layout.preferredWidth: 200
    }
    MultiSliceRenderItem {
      Layout.row:  2
      Layout.column: 1
      objectName: "MultiSlice"+Layout.row+Layout.column
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumWidth: 100
      Layout.preferredWidth: 200
    }
    MultiSliceRenderItem {
      Layout.row:  2
      Layout.column: 2
      objectName: "MultiSlice"+Layout.row+Layout.column
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumWidth: 100
      Layout.preferredWidth: 200
    }

  }
  Shape{
      objectName: "MultiSliceOutliner3x3"
      ShapePath {
          fillColor: "transparent"
          strokeColor: "white"
          strokeWidth: 1
          strokeStyle: ShapePath.SolidLine
          //horizon line 1
          startX: 0; startY: root_3d_win.height / 3.0
          PathLine { x: root_3d_win.width; y: root_3d_win.height /3.0 }
          //horizon line 2
          PathMove { x: 0; y: 2.0*root_3d_win.height / 3.0}
          PathLine { x: root_3d_win.width; y: 2.0*root_3d_win.height /3.0 }
          //vertical line 1
          PathMove { x: root_3d_win.width* 1.0 / 3.0 ; y: 0}
          PathLine { x: root_3d_win.width* 1.0 / 3.0 ; y: root_3d_win.height }
          //vertical line 2
          PathMove { x: root_3d_win.width* 2.0 / 3.0 ; y: 0}
          PathLine { x: root_3d_win.width* 2.0 / 3.0 ; y: root_3d_win.height }
      }
  }
}
