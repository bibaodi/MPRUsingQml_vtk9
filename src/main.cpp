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
#include "multisliceview.h"

const QString k_data_dir = "/home/eton/opt/data/headsq/quarter";
// enum ViewType { A, C, T, D3 };

class QVTKRenderItemWidgetCallback : public vtkCommand {
  public:
    static QVTKRenderItemWidgetCallback *New() { return new QVTKRenderItemWidgetCallback; }

    void Execute(vtkObject *caller, unsigned long, void *) override {
        vtkImagePlaneWidget *planeWidget = reinterpret_cast<vtkImagePlaneWidget *>(caller);
        int slice_idx = planeWidget->GetSliceIndex();
        int act_idx = -1;
        qDebug() << "callback: slice index=" << slice_idx;
        if (ipw_3d[0] == planeWidget) {
            act_idx = 0;
        } else if (ipw_3d[1] == planeWidget) {
            act_idx = 1;
        } else if (ipw_3d[2] == planeWidget) {
            act_idx = 2;
        }
        qDebug() << "callback: "
                 << " act_index" << act_idx;
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

    vtkSmartPointer<vtkImagePlaneWidget> ipw_3d[3];
    vtkSmartPointer<vtkImagePlaneWidget> ipw_a[3];
    vtkSmartPointer<vtkImagePlaneWidget> ipw_c[3];
    vtkSmartPointer<vtkImagePlaneWidget> ipw_t[3];
};

int create_ipw_instance(vtkSmartPointer<vtkImagePlaneWidget> &ipw, int orientation,
                        vtkSmartPointer<vtkVolume16Reader> &v16, vtkRenderer *ren, QVTKInteractor *iact,
                        int slice_idx) {
    ipw->SetInputConnection(v16->GetOutputPort());
    ipw->SetCurrentRenderer(ren);
    ipw->SetInteractor(iact);
    ipw->RestrictPlaneToVolumeOn();
    // image style not allowed 3d rotate
    vtkNew<vtkInteractorStyleImage> style;
    iact->SetInteractorStyle(style);

    double color[3] = {0, 0, 0};
    qDebug() << "create_ipw_instance: orientation=" << orientation;
    if (ViewType::T == orientation) {
        qDebug() << "create_ipw_instance: branch=x";
        ipw->SetPlaneOrientationToXAxes();
        ipw->SetSliceIndex(slice_idx);
        color[1] = 1;
    } else if (ViewType::C == orientation) {
        qDebug() << "create_ipw_instance: branch=y";
        ipw->SetPlaneOrientationToYAxes();
        ipw->SetSliceIndex(slice_idx);
        color[0] = 1;
    } else if (ViewType::A == orientation) {
        qDebug() << "create_ipw_instance: branch= others";
        ipw->SetPlaneOrientationToZAxes();
        ipw->SetSliceIndex(slice_idx);
        color[2] = 1;
    } else {
        qDebug() << "error: no right orientation";
        return -1;
    }

    ipw->GetPlaneProperty()->SetColor(color);
    return 0;
}

/*
 * direction = (0, 1, 2) x=0, y=2, z=2
 */
int reset_img_plane_view_cam(vtkRenderer *ren, int direction) {
    ren->ResetCamera();
    vtkCamera *cam = ren->GetActiveCamera();
    if (ViewType::A == direction) {
        cam->SetViewUp(0, -1, 0);
    } else if (ViewType::C == direction) {
        cam->Elevation(90);
        cam->SetViewUp(0, 0, -1);
    } else if (ViewType::T == direction) {
        qDebug() << "default is Z direction";
        cam->Azimuth(-90);
        cam->SetViewUp(0, 1, 0);
    }
    cam->OrthogonalizeViewUp();
    return 0;
}

int create_slice_pos_line(float slice_pos, vtkSmartPointer<vtkImagePlaneWidget> &ipw0, int orientation,
                          vtkRenderer *ren) {

    double ipw0_pos = ipw0->GetSlicePosition(); // when ipw0 == A then this is a z;

    // Create two points, P0 and P1
    double p0[3] = {0.0, slice_pos, ipw0_pos}; // when slice ==C then pos is a y; z from 0 to max
    double p1[3] = {201.6, slice_pos, ipw0_pos};
    vtkNew<vtkLineSource> lineSource;
    lineSource->SetPoint1(p0);
    lineSource->SetPoint2(p1);

    // Visualize
    double color[3] = {0, 0, 0};
    if (ViewType::T == orientation) {
        color[1] = 1;
    } else if (ViewType::C == orientation) {
        color[0] = 1;
    } else if (ViewType::A == orientation) {
        color[2] = 1;
    }

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(lineSource->GetOutputPort());
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetLineWidth(1);
    actor->GetProperty()->SetColor(color);
    ren->AddActor(actor);
    return 0;
}

int main(int argc, char *argv[]) {
    QQuickVTKRenderWindow::setupGraphicsBackend();

    // QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    qDebug() << "QML2_IMPORT_PATH:" << engine.importPathList();
    engine.addImportPath("/usr/local/lib/qml");
    const QUrl url(QStringLiteral("qrc:/MultiSlice3x3.qml"));
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
    MultiSliceView msc(v16, nullptr, topLevel, 3, ViewType::C); //>>>>
    qDebug() << "msc instance create finish~";

    msc.show();
    return app.exec();
}
