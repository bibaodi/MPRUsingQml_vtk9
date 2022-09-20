import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
// import the VTK module
import VTK 9.1
import QtQuick.Shapes 1.15

Window {
    // title of the application
    title: qsTr("VTK QtQuick Application")
    id: root_3d_win
    width: 960
    height: 960
    property int global_view_index: 0

    MainViewPort {
        id: id_vtkMainPort
        anchors.fill: parent
    }
}
