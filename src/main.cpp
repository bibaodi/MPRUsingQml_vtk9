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
#include "vtkCommand.h"
#include "vtkImagePlaneWidget.h"
#include "vtkImplicitPlaneRepresentation.h"
#include "vtkImplicitPlaneWidget2.h"
#include "vtkOutlineFilter.h"
#include "vtkProperty.h"
#include "vtkVolume16Reader.h"
//-add-end
#include <QApplication>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QTimer>
#include <QUrl>

const QString k_data_dir = "/home/eton/opt/data/headsq/quarter";

class TestQQuickVTKRenderItemWidgetCallback : public vtkCommand {
  public:
    static TestQQuickVTKRenderItemWidgetCallback *New() { return new TestQQuickVTKRenderItemWidgetCallback; }

    void Execute(vtkObject *caller, unsigned long, void *) override {
        vtkImagePlaneWidget *planeWidget = reinterpret_cast<vtkImagePlaneWidget *>(caller);
        // zan shi bu hui yong
        this->Actor->VisibilityOn();
    }

    TestQQuickVTKRenderItemWidgetCallback() : Plane(nullptr), Actor(nullptr) {}
    vtkPlane *Plane;
    vtkActor *Actor;
};

int create_ipw_instance(vtkSmartPointer<vtkImagePlaneWidget> &ipw, QString orientation,
                        vtkSmartPointer<vtkVolume16Reader> &v16, vtkRenderer *ren,
                        vtkSmartPointer<QVTKInteractor> &iact) {
    ipw = vtkSmartPointer<vtkImagePlaneWidget>::New();
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

int main(int argc, char *argv[]) {
    QQuickVTKRenderWindow::setupGraphicsBackend();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

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
    QObject *topLevel = engine.rootObjects().value(0);
    QQuickWindow *window = qobject_cast<QQuickWindow *>(topLevel);

    window->show(); // without this code, nothing will display --eton@210810

    // Fetch the QQuick window using the standard object name set up in the constructor
    QQuickVTKRenderItem *qquickvtkItem = topLevel->findChild<QQuickVTKRenderItem *>("MPRView_A");
    if (!qquickvtkItem) {
        qDebug() << "vtk widget not found!";
        return -1;
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

    vtkSmartPointer<QVTKInteractor> iact = vtkSmartPointer<QVTKInteractor>::New();
    iact->SetRenderWindow(qquickvtkItem->renderWindow()->renderWindow());
    vtkSmartPointer<vtkImagePlaneWidget> planeWidget[3];

    int i = 0;

    create_ipw_instance(planeWidget[0], QString("x"), v16, qquickvtkItem->renderer(), iact);
    create_ipw_instance(planeWidget[1], QString("y"), v16, qquickvtkItem->renderer(), iact);
    create_ipw_instance(planeWidget[2], QString("z"), v16, qquickvtkItem->renderer(), iact);

    qquickvtkItem->renderer()->AddActor(outlineActor);
    qquickvtkItem->renderer()->SetViewport(0.51, 0.0, 1.0, 0.49);
    // qquickvtkItem->renderer()->AddActor(actor);
    // qquickvtkItem->renderer()->ResetCamera();
    qquickvtkItem->renderer()->SetBackground(0.5, 0.5, 0.7);
    qquickvtkItem->renderer()->SetBackground2(0.7, 0.7, 0.7);
    //  qquickvtkItem->renderer()->SetGradientBackground(true);
    qquickvtkItem->update();
    for (i = 0; i < 3; i++) {
        planeWidget[i]->On();
    }
    iact->Initialize();
    iact->Start();

    return app.exec();
}
