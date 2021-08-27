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
  property int global_view_index:0

  SystemPalette {
    id: palette
    colorGroup: SystemPalette.Active
  }


  Loader {
    id:id_3d_view_loader
    source: "MultiSlice4x4.qml"
    anchors.fill: parent
    onLoaded:{
        console.log("loaded signal for", id_3d_view_loader.source);
        create_view_instance();
    }
  }

  RowLayout {
      width:parent.width / 2
      height: 20
      x:0
      y: parent.height - height - 10
    Button {
        id: btn_select
        objectName: "btn_select"
        text: "act-plane"
        Layout.preferredHeight:parent.height
        signal qmlSignal(msg: string)
        onClicked: {
            btn_select.qmlSignal("Hello from QML")
            global_view_index = global_view_index % 3
            if (0 === global_view_index) {
                id_3d_view_loader.source="MPR_3d_view.qml"
            } else if (1=== global_view_index) {
                id_3d_view_loader.source="MultiSlice3x3.qml"
            } else {
                id_3d_view_loader.source="MultiSlice4x4.qml"
            }
            global_view_index +=1
            console.log("id_3d_view_index=", global_view_index)
        }
    }
    Button {
        id: btn_translate
        objectName: "btn_translate"
        text:"translate"
        Layout.preferredHeight:parent.height
        signal qmlSignal2(msg:int )
        onClicked: {
            global_view_index +=1
            global_view_index = global_view_index % 3
            if (0 === global_view_index) {
                id_3d_view_loader.source="MPR_3d_view.qml"
            } else if (1=== global_view_index) {
                id_3d_view_loader.source="MultiSlice3x3.qml"
            } else {
                id_3d_view_loader.source="MultiSlice4x4.qml"
            }
            console.log("change 3d view layout:id_3d_view_index=", global_view_index)
        }
    }
  }
  function create_view_instance() {
        btn_translate.qmlSignal2(global_view_index)
  }
}
