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
#include "QVTKInteractor.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkImagePlaneWidget.h"
#include "vtkImplicitPlaneRepresentation.h"
#include "vtkImplicitPlaneWidget2.h"
#include "vtkOutlineFilter.h"
#include "vtkProperty.h"
#include "vtkVolume16Reader.h"
#include <list>
//-add-end
#include <QApplication>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QTimer>
#include <QUrl>

const QString k_data_dir = "/home/eton/opt/data/headsq/quarter";

class QVTKRenderItemWidgetCallback : public vtkCommand {
  public:
    static QVTKRenderItemWidgetCallback *New() { return new QVTKRenderItemWidgetCallback; }

    void Execute(vtkObject *caller, unsigned long, void *) override {
        vtkImagePlaneWidget *planeWidget = reinterpret_cast<vtkImagePlaneWidget *>(caller);
        int slice_idx = planeWidget->GetSliceIndex();
        int act_idx = -1;
        qDebug() << "callback: slice index=" << slice_idx << ", act_index" << act_idx;
        if (ipw_3d[0] == planeWidget) {
            act_idx = 0;
        } else if (ipw_3d[1] == planeWidget) {
            act_idx = 1;
        } else if (ipw_3d[2] == planeWidget) {
            act_idx = 2;
        }
        qDebug() << "callback: slice index=" << slice_idx << ", act_index" << act_idx;
        if (act_idx > -1) {
            ipw_a[act_idx]->SetSliceIndex(slice_idx);
            ipw_c[act_idx]->SetSliceIndex(slice_idx);
            ipw_t[act_idx]->SetSliceIndex(slice_idx);
        }
    }

    QVTKRenderItemWidgetCallback() {
        for (int i = 0; i < 3; i++) {
            qDebug() << "init callback:i=" << i;
            ipw_3d[i] = nullptr;
            ipw_a[i] = nullptr;
            ipw_c[i] = nullptr;
            ipw_t[i] = nullptr;
        }
    }

    vtkImagePlaneWidget *ipw_3d[3];
    vtkImagePlaneWidget *ipw_a[3];
    vtkImagePlaneWidget *ipw_c[3];
    vtkImagePlaneWidget *ipw_t[3];
};

int create_ipw_instance(vtkSmartPointer<vtkImagePlaneWidget> &ipw, QString orientation,
                        vtkSmartPointer<vtkVolume16Reader> &v16, vtkRenderer *ren,
                        vtkSmartPointer<QVTKInteractor> &iact) {
    // ipw = vtkSmartPointer<vtkImagePlaneWidget>::New();
    ipw->SetInputConnection(v16->GetOutputPort());
    ipw->SetCurrentRenderer(ren);
    ipw->SetInteractor(iact);
    // ipw->SetPicker(picker);
    ipw->RestrictPlaneToVolumeOn();

    double color[3] = {0, 0, 0};
    qDebug() << "create_ipw_instance: orientation=" << orientation;
    if (QString("x") == orientation.toLower()) {
        qDebug() << "create_ipw_instance: branch=x";
        ipw->SetPlaneOrientationToXAxes();
        ipw->SetSliceIndex(32);
        color[2] = 1;
    } else if (QString('y') == orientation.toLower()) {
        qDebug() << "create_ipw_instance: branch=y";
        ipw->SetPlaneOrientationToYAxes();
        ipw->SetSliceIndex(33);
        color[0] = 1;
    } else {
        qDebug() << "create_ipw_instance: branch= others";
        ipw->SetPlaneOrientationToZAxes();
        ipw->SetSliceIndex(35);
        color[1] = 1;
    }

    ipw->GetPlaneProperty()->SetColor(color);
    return 0;
}

int reset_3d_view_cam(vtkRenderer *ren) {
    ren->ResetCamera();
    vtkCamera *cam = ren->GetActiveCamera();
    cam->Elevation(110);
    cam->SetViewUp(0, 0, -1);
    cam->Azimuth(45);
    ren->ResetCameraClippingRange();
    return 0;
}

/*
 * direction = (0, 1, 2) x=0, y=2, z=2
 */
int reset_act_plane_view_cam(vtkRenderer *ren, int direction) {
    ren->ResetCamera();
    vtkCamera *cam = ren->GetActiveCamera();
    if (0 == direction) {
        cam->Azimuth(90);
        cam->SetViewUp(0, 0, -1);
    } else if (1 == direction) {
        cam->Elevation(90);
        cam->SetViewUp(0, 0, -1);
    } else if (2 == direction) {
        qDebug() << "default is Z direction";
        cam->SetViewUp(0, 1, 0);
    }
    cam->OrthogonalizeViewUp();
    return 0;
}

int main(int argc, char *argv[]) {
    QQuickVTKRenderWindow::setupGraphicsBackend();

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    qDebug() << "QML2_IMPORT_PATH:" << engine.importPathList();
    engine.addImportPath("/usr/local/lib/qml");
    const QUrl url(QStringLiteral("qrc:/main.qml"));
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
    QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel);
    window->show(); // without this code, nothing will display --eton@210810

    // Fetch the QQuick window using the standard object name set up in the constructor
    QQuickVTKRenderItem *qvtkItem[4] = {nullptr};
    QString renderNames[] = {"MPRView_A", "MPRView_C", "MPRView_T", "MPRView_3D"};
    for (int _i = 0; _i < 4; _i++) {
        qvtkItem[_i] = topLevel->findChild<QQuickVTKRenderItem *>(renderNames[_i]);
        if (!qvtkItem[_i]) {
            qDebug() << "vtk widget not found! >>" << renderNames[_i];
            return -1;
        }
    }
    /*
        // Create a cone pipeline and add it to the view
        vtkNew<vtkActor> actor;
        vtkNew<vtkPolyDataMapper> mapper;
        vtkNew<vtkConeSource> cone;
        mapper->SetInputConnection(cone->GetOutputPort());
        actor->SetMapper(mapper);
        // cone --end
    */
    // use Volume16Reader read data
    vtkSmartPointer<vtkVolume16Reader> v16 = vtkSmartPointer<vtkVolume16Reader>::New();
    v16->SetDataDimensions(64, 64);
    v16->SetDataByteOrderToLittleEndian();
    v16->SetFilePrefix(k_data_dir.toStdString().c_str());
    v16->SetImageRange(1, 93);
    v16->SetDataSpacing(3.2, 3.2, 1.5);
    v16->Update();
    int *extent = v16->GetOutput()->GetExtent();
    qDebug() << "v16 info:" << extent[0] << extent[1] << extent[3] << extent[5];

    // create outline for 3d-view
    vtkNew<vtkOutlineFilter> outline;
    outline->SetInputConnection(v16->GetOutputPort());
    vtkNew<vtkPolyDataMapper> outlineMapper;
    outlineMapper->SetInputConnection(outline->GetOutputPort());
    vtkNew<vtkActor> outlineActor;
    outlineActor->SetMapper(outlineMapper);

    // 3d-view
    vtkRenderer *ren = qvtkItem[3]->renderer();
    vtkSmartPointer<QVTKInteractor> iact = vtkSmartPointer<QVTKInteractor>::New();
    iact->SetRenderWindow(qvtkItem[3]->renderWindow()->renderWindow());
    vtkSmartPointer<vtkImagePlaneWidget> planeWidget[3];
    vtkSmartPointer<vtkImagePlaneWidget> ipw_a[3];
    vtkSmartPointer<vtkImagePlaneWidget> ipw_c[3];
    vtkSmartPointer<vtkImagePlaneWidget> ipw_t[3];

    int i = 0;
    // create ipw
    for (i = 0; i < 3; i++) {
        planeWidget[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
        ipw_a[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
        ipw_c[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
        ipw_t[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
        QString _o;
        if (0 == i)
            _o = QString("x");
        else if (1 == i)
            _o = QString("y");
        else if (2 == i)
            _o = QString("z");

        create_ipw_instance(planeWidget[i], _o, v16, ren, iact);
        create_ipw_instance(ipw_a[i], _o, v16, qvtkItem[0]->renderer(), iact);
        create_ipw_instance(ipw_c[i], _o, v16, qvtkItem[1]->renderer(), iact);
        create_ipw_instance(ipw_t[i], _o, v16, qvtkItem[2]->renderer(), iact);
    }
    // create callback for all three 3d-view's ipw
    vtkNew<QVTKRenderItemWidgetCallback> ipw_cb;

    for (int i = 0; i < 3; i++) {
        ipw_cb->ipw_a[i] = ipw_a[i].GetPointer();
        ipw_cb->ipw_c[i] = ipw_c[i].GetPointer();
        ipw_cb->ipw_t[i] = ipw_t[i].GetPointer();
        ipw_cb->ipw_3d[i] = planeWidget[i].GetPointer();
        planeWidget[i]->AddObserver(vtkCommand::AnyEvent, ipw_cb);
    }

    ren->AddActor(outlineActor);
    // ren->SetViewport(0.51, 0.0, 1.0, 0.49);
    // ren->SetBackground(0.5, 0.5, 0.7);
    // ren->SetBackground2(0.7, 0.7, 0.7);

    // ren->SetGradientBackground(true);
    // qvtkItem[3]->update();
    for (i = 0; i < 3; i++) {
        ipw_a[i]->On();
        ipw_c[i]->On();
        ipw_t[i]->On();
        planeWidget[i]->On();
    }
    reset_3d_view_cam(ren);
    reset_act_plane_view_cam(qvtkItem[0]->renderer(), 0);
    reset_act_plane_view_cam(qvtkItem[1]->renderer(), 1);
    reset_act_plane_view_cam(qvtkItem[2]->renderer(), 2);
    iact->Initialize();
    iact->Start();

    return app.exec();
}
