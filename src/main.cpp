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
#include "multiplanarview.h"
#include "multisliceview.h"

const QString k_data_dir = "/home/eton/opt/data/headsq/quarter";

#define view_type 2 // 2=mpr; 3=mslice3x3; 4=mslice4x4

int main(int argc, char *argv[]) {
    QQuickVTKRenderWindow::setupGraphicsBackend();

    // QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    qDebug() << "QML2_IMPORT_PATH:" << engine.importPathList();
    engine.addImportPath("/usr/local/lib/qml");

    QLatin1String qml_name("qrc:/MPR_3d_view.qml");
    if (view_type == 3) {
        qml_name = QLatin1String("qrc:/MultiSlice3x3.qml");
    } else if (view_type == 4) {
        qml_name = QLatin1String("qrc:/MultiSlice4x4.qml");
    }
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
    QObject *topLevel = engine.rootObjects().value(0);
#if 1
    QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel);
    window->show(); // without this code, nothing will display --eton@210810
    QQuickVTKRenderItem *qvtkItem = topLevel->findChild<QQuickVTKRenderItem *>("MPRView_A");
    vtkSmartPointer<vtkRenderWindow> renw = qvtkItem->renderWindow()->renderWindow();
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renwin =
        static_cast<vtkGenericOpenGLRenderWindow *>(renw.GetPointer());
    QVTKInteractor *iact = dynamic_cast<QVTKInteractor *>(renwin->GetInteractor());
    qDebug() << "pointer in main: iact=" << iact;
#endif
    // use Volume16Reader read data
    vtkSmartPointer<vtkVolume16Reader> v16 = vtkSmartPointer<vtkVolume16Reader>::New();
    v16->SetDataDimensions(64, 64);
    v16->SetDataByteOrderToLittleEndian();
    v16->SetFilePrefix(k_data_dir.toStdString().c_str());
    v16->SetImageRange(1, 93);
    v16->SetDataSpacing(3.2, 3.2, 1.5);
    v16->Update();
    int extent[6] = {0};
    v16->GetOutput()->GetExtent(extent);
    qDebug() << "v16 info extent:" << extent[0] << extent[1] << extent[3] << extent[5];
    double spacing[3];
    v16->GetOutput()->GetSpacing(spacing);
    qDebug() << "v16 info spacing:" << spacing[0] << spacing[1] << spacing[2];
    if (view_type > 2) {
        MultiSliceView msc(v16, nullptr, topLevel, view_type, ViewType::MultiSliceVT_C); //>>>>
        qDebug() << "msc instance create finish~";
        msc.show();
        // msc.m_ipw_arr[0]->On();
    } else {
        MultiPlanarView mpr(v16, nullptr, topLevel);

        qDebug() << "pointer in class: iact=" << mpr.m_iact;
        mpr.show();
    }
    return app.exec();
}
