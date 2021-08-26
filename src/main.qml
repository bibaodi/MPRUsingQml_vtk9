import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
// import the VTK module
import VTK 9.0
import QtQuick.Shapes 1.15

Window {
  // title of the application
  title: qsTr("VTK QtQuick App")
  id: root_3d_win
  width: 800
  height: 800
  color: palette.window

  SystemPalette {
    id: palette
    colorGroup: SystemPalette.Active
  }
  Loader {
    source: "MPR_3d_view.qml"
    anchors.fill: parent
  }
  /*
  MPR_3d_view {
    id: mpr_content
    anchors.fill: parent
  }
  */
  Rectangle {
      Component.onCompleted: console.log("Completed Running!")
      Rectangle {
          Component.onCompleted: console.log("Nested Completed Running!")
      }
  }
}
