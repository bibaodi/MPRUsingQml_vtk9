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

const QString k_data_dir = "/home/eton/opt/data/headsq/quarter";

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

int create_ipw_instance(vtkSmartPointer<vtkImagePlaneWidget> &ipw, QString orientation,
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
    if (QString("x") == orientation.toLower()) {
        qDebug() << "create_ipw_instance: branch=x";
        ipw->SetPlaneOrientationToXAxes();
        ipw->SetSliceIndex(slice_idx);
        color[2] = 1;
    } else if (QString('y') == orientation.toLower()) {
        qDebug() << "create_ipw_instance: branch=y";
        ipw->SetPlaneOrientationToYAxes();
        ipw->SetSliceIndex(slice_idx);
        color[0] = 1;
    } else {
        qDebug() << "create_ipw_instance: branch= others";
        ipw->SetPlaneOrientationToZAxes();
        ipw->SetSliceIndex(slice_idx);
        color[1] = 1;
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
    // cam->OrthogonalizeViewUp();
    return 0;
}

int create_slice_pos_line(float slice_pos, vtkSmartPointer<vtkImagePlaneWidget> &ipw0, QString orientation,
                          vtkRenderer *ren) {

    double ipw0_pos = ipw0->GetSlicePosition(); // when ipw0 == A then this is a x;

    // Create two points, P0 and P1
    double p0[3] = {ipw0_pos, slice_pos, 0.0}; // when slice ==C then pos is a y; z from 0 to max
    double p1[3] = {ipw0_pos, slice_pos, 138.0};
    vtkNew<vtkLineSource> lineSource;
    lineSource->SetPoint1(p0);
    lineSource->SetPoint2(p1);

    // Visualize
    double color[3] = {0, 0, 0};
    if (QString("x") == orientation.toLower()) {
        color[2] = 1;
    } else if (QString('y') == orientation.toLower()) {
        color[0] = 1;
    } else {
        color[1] = 1;
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
    const int row_cnt = 4;
    const int col_cnt = 4;
    float slice_pos[row_cnt * col_cnt - 1] = {0.0};
    QQuickVTKRenderItem *qvtkItem[row_cnt * col_cnt] = {nullptr};
    QString renderNames = "MultiSlice";
    enum ViewType { A, C, T, D3 };
    const int current_view = 1; // 0=a,1=c,2=t,3=3d;
    int current_view_ortho = current_view;
    int slice_idx_base = 0;

    // make the first view orthogonal to current view
    if (ViewType::A == current_view) {
        current_view_ortho = ViewType::T;
    } else if (ViewType::C == current_view) {
        current_view_ortho = ViewType::A;
    } else if (ViewType::T == current_view) {
        current_view_ortho = ViewType::C;
    }

    for (int _r = 0; _r < row_cnt; _r++) {
        for (int _c = 0; _c < col_cnt; _c++) {
            qvtkItem[_r * row_cnt + _c] =
                topLevel->findChild<QQuickVTKRenderItem *>(QString("%1%2%3").arg(renderNames).arg(_r).arg(_c));
            if (!qvtkItem[_r * row_cnt + _c]) {
                qDebug() << "vtk widget not found! >>" << renderNames[_r * row_cnt];
                return -1;
            }
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
    // vtkRenderer *ren = qvtkItem[3]->renderer();
    // vtkNew<QVTKInteractor> iact;

    vtkSmartPointer<vtkRenderWindow> vtk_ren_win = qvtkItem[0]->renderWindow()->renderWindow();
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> vtk_gl_renwin =
        static_cast<vtkGenericOpenGLRenderWindow *>(vtk_ren_win.GetPointer());
    QVTKInteractor *iact = dynamic_cast<QVTKInteractor *>(vtk_gl_renwin->GetInteractor());
    vtkSmartPointer<vtkImagePlaneWidget> ipw[row_cnt * col_cnt * 2 - 1];

    int i = 0;
    // create ipw
    for (i = 0; i < row_cnt * col_cnt; i++) {
        qDebug() << "i=" << i;
        ipw[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
        QString _o;
        int _view = current_view;
        if (0 == i) {
            _view = current_view_ortho;
        }
        switch (_view) {
        case ViewType::A:
            _o = QString("x");
            break;
        case ViewType::C:
            _o = QString("y");
            break;
        case ViewType::T:
            _o = QString("z");
            break;
        default:
            qDebug() << "current_view_ortho not in range:" << _view;
        }

        qDebug() << "current_view_ortho=" << _view << ", orentation=" << _o;
        create_ipw_instance(ipw[i], _o, v16, qvtkItem[i]->renderer(), iact, slice_idx_base + i * 3 - 1);
        if (i > 0)
            create_ipw_instance(ipw[i], _o, v16, qvtkItem[i]->renderer(), iact, slice_idx_base + i * 3 - 1);
        if (i > 0) {
            slice_pos[i - 1] = ipw[i]->GetSlicePosition();
            create_slice_pos_line(slice_pos[i - 1], ipw[0], _o, qvtkItem[0]->renderer());
        }
    }

    // qvtkItem[3]->update();
    for (i = 0; i < row_cnt * col_cnt; i++) {
        ipw[i]->On();
    }
    reset_img_plane_view_cam(qvtkItem[0]->renderer(), current_view_ortho);
    for (i = 1; i < row_cnt * col_cnt; i++) {
        reset_img_plane_view_cam(qvtkItem[i]->renderer(), current_view);
    }

    return app.exec();
}
