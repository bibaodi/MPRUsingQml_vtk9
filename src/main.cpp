#include "QQuickVTKRenderItem.h"
#include "QQuickVTKRenderWindow.h"
#include "vtkActor.h"
#include "vtkConeSource.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkNew.h"
#include "vtkPNGWriter.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkTestUtilities.h"
#include "vtkTesting.h"
#include "vtkWindowToImageFilter.h"
//-add-begin
#include "QQuickVTKInteractiveWidget.h"
#include "QQuickVTKInteractorAdapter.h"
#include "QVTKInteractor.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkImagePlaneWidget.h"
#include "vtkImplicitPlaneRepresentation.h"
#include "vtkImplicitPlaneWidget2.h"
#include "vtkInteractorStyleImage.h"
#include "vtkOutlineFilter.h"
#include "vtkProperty.h"
#include "vtkVolume16Reader.h"
#include <vtkLineSource.h>
#include <vtkNamedColors.h>
//-add-end
#include <QApplication>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QTimer>
#include <QUrl>
//---
#include "threedimensionveiw.h"

const QString k_data_dir =
    "/media/eton/statics/08.dicom-datas/vtk-data/headsq/quarter"; //"/home/eton/opt/data/headsq/quarter";

int main(int argc, char *argv[]) {
    QQuickVTKRenderWindow::setupGraphicsBackend();

    // QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    qDebug() << "QML2_IMPORT_PATH:" << engine.importPathList();
    engine.addImportPath("/usr/local/lib/qml");

    QLatin1String qml_name("qrc:/main.qml");

    const QUrl url(qml_name);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);
    // get root window
    QObject *root = engine.rootObjects().value(0);
    QObject *topLevel = root->findChild<QObject *>("3d_view_loader");
    if (!topLevel) {
        qDebug() << "Error: not found loader!";
        return -1;
    }
    topLevel = root;

    // use Volume16Reader read data
    vtkSmartPointer<vtkVolume16Reader> v16 = vtkSmartPointer<vtkVolume16Reader>::New();
    v16->SetDataDimensions(64, 64);
    v16->SetDataByteOrderToLittleEndian();
    v16->SetFilePrefix(k_data_dir.toStdString().c_str());
    v16->SetImageRange(1, 93);
    v16->SetDataSpacing(3.2, 3.2, 1.5);
    v16->Update();
    // v16->PrintSelf(std::cout, vtkIndent(4));
    // v16->GetOutput()->PrintSelf(std::cout, vtkIndent(4));
    int extent[6] = {0};
    v16->GetOutput()->GetExtent(extent);
    qDebug() << "v16 info extent:" << extent[0] << extent[1] << extent[3] << extent[5];
    double spacing[3];
    v16->GetOutput()->GetSpacing(spacing);
    qDebug() << "v16 info spacing:" << spacing[0] << spacing[1] << spacing[2];

    ThreeDimensionVeiw tdv(v16, nullptr, VIEW_3D_MS4x4, topLevel);

    return app.exec();
}
