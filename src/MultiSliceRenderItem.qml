import QtQuick 2.0
// import the VTK module
import VTK 9.0


VTKRenderItem {
  property int row_idx: 0
  property int col_idx: 0
  objectName: "MultiSlice00"
  renderWindow: vtkwindow
  focus: true
  Rectangle {
        id:rect_0
        border.color: "transparent"
        color: "transparent"
        x:  10
        y: 10
        width:parent.width -20
        height:parent.height-20
  }
  ScaleBar3D {
    x: parent.children[0].width + parent.children[0].x
    y: parent.children[0].y
    width: 20
    height: parent.children[0].height
  }
}
