#include "multiplanarview.h"
#include <QQuickWindow>

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
            ipw_act[act_idx]->SetSliceIndex(slice_idx);
        }
    }

    QVTKRenderItemWidgetCallback() {
        for (int i = 0; i < 3; i++) {
            qDebug() << "init callback:i=" << i;
            ipw_3d[i] = nullptr;
            ipw_act[i] = nullptr;
        }
    }

    vtkSmartPointer<vtkImagePlaneWidget> ipw_3d[3];
    vtkSmartPointer<vtkImagePlaneWidget> ipw_act[3];
};

MultiPlanarView::MultiPlanarView(vtkSmartPointer<vtkVolume16Reader> _v16, QObject *parent, QObject *root)
    : QObject(parent), m_topLevel(root) {
    qDebug() << "MPR view init~";
    m_v16 = _v16;
    if (!m_v16) {
        qDebug() << "Err:data invalid";
        return;
    }
    //--00 set default slice_position
    int *extent = m_v16->GetOutput()->GetExtent();
    double *spacing = m_v16->GetOutput()->GetSpacing();
    m_slice_pos[0] = spacing[0] * extent[1] * 0.4;
    m_slice_pos[1] = spacing[1] * extent[3] * 0.5;
    m_slice_pos[2] = spacing[2] * extent[5] * 0.6;
    //--01 get window
    if (nullptr == m_topLevel) {
        qDebug() << "qml root item is nullptr";
        return;
    }
    m_quickwin = qobject_cast<QQuickWindow *>(m_topLevel);
    //--02 get renders
    QString renderNames[] = {"MPRView_A", "MPRView_C", "MPRView_T", "MPRView_3D"};
    for (int _i = 0; _i < THREED_MPR_PLANE; _i++) {
        m_qvtkRen_arr[_i] = m_topLevel->findChild<QQuickVTKRenderItem *>(renderNames[_i]);
        if (!m_qvtkRen_arr[_i]) {
            qDebug() << "vtk widget not found! >>" << renderNames[_i];
            return;
        }
    }
    //--03 create outline
    create_outline_actor(m_qvtkRen_arr[MPR_Plane::MultiPlane_3D]->renderer());
    //--04 get window and interactor
    vtkSmartPointer<vtkRenderWindow> vtk_ren_win = m_qvtkRen_arr[0]->renderWindow()->renderWindow();
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> vtk_gl_renwin =
        static_cast<vtkGenericOpenGLRenderWindow *>(vtk_ren_win.GetPointer());
    m_iact = dynamic_cast<QVTKInteractor *>(vtk_gl_renwin->GetInteractor());
    // m_iact->Initialize();
    //--05 create ACT ipw
    int i = 0, ret = 0;
    for (i = 0; i < 3; i++) {
        m_ipw_arr[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
        ret = create_ipw_instance(m_ipw_arr[i], i, m_v16, m_qvtkRen_arr[i]->renderer(), m_iact);
        if (ret) {
            qDebug() << "act ipw create error. i=" << i << "return=" << ret;
            return;
        }
    }
    //--06 create ipw for 3d
    for (i = 0; i < 3; i++) {
        m_ipw_arr[i + 3] = vtkSmartPointer<vtkImagePlaneWidget>::New();
        ret = create_ipw_instance(m_ipw_arr[i + 3], i, m_v16, m_qvtkRen_arr[3]->renderer(), m_iact);
        if (ret) {
            qDebug() << "3d ipw create error. i="
                     << "return=" << ret;
            return;
        }
    }
    //--07 make it available
    m_render_ready = true;
    qDebug() << "01iact print self:";
#include <iostream>
    m_iact->PrintSelf(std::cout, vtkIndent(4));
    //--08  create callback for all three 3d-view's ipw
    vtkNew<QVTKRenderItemWidgetCallback> ipw_cb;
    for (i = 0; i < 3; i++) {
        ipw_cb->ipw_act[i] = m_ipw_arr[i].GetPointer();
        ipw_cb->ipw_3d[i] = m_ipw_arr[i + 3].GetPointer();
        // m_ipw_arr[i + 3]->AddObserver(vtkCommand::AnyEvent, ipw_cb);
    }
}

int MultiPlanarView::create_outline_actor(vtkRenderer *ren) {
    if (!ren) {
        return -1;
    }
    if (!m_v16) {
        return -2;
    }
    vtkNew<vtkOutlineFilter> outline;
    outline->SetInputConnection(m_v16->GetOutputPort());
    vtkNew<vtkPolyDataMapper> outlineMapper;
    outlineMapper->SetInputConnection(outline->GetOutputPort());
    vtkNew<vtkActor> outlineActor;
    outlineActor->SetMapper(outlineMapper);
    ren->AddActor(outlineActor);
    return 0;
}

int MultiPlanarView::create_ipw_instance(vtkSmartPointer<vtkImagePlaneWidget> &ipw, int orientation,
                                         vtkSmartPointer<vtkVolume16Reader> &v16, vtkRenderer *ren,
                                         QVTKInteractor *iact) {
    if (!ipw) {
        return -1;
    }
    if (!v16 || !ipw) {
        return -2;
    }
    if (!ren) {
        return -3;
    }
    if (!iact) {
        return -4;
    }
    ipw->SetInputConnection(v16->GetOutputPort());
    ipw->SetCurrentRenderer(ren);
    ipw->SetInteractor(iact);
    ipw->RestrictPlaneToVolumeOn();
    // image style not allowed 3d rotate
    vtkNew<vtkInteractorStyleImage> style;
    iact->SetInteractorStyle(style);

    double color[3] = {0, 0, 0};
    qDebug() << "create_ipw_instance: orientation=" << orientation;
    if (MPR_Plane::MultiPlane_T == orientation) {
        qDebug() << "create_ipw_instance: branch=x";
        ipw->SetPlaneOrientationToXAxes();
        color[1] = 1;
    } else if (MPR_Plane::MultiPlane_C == orientation) {
        qDebug() << "create_ipw_instance: branch=y";
        ipw->SetPlaneOrientationToYAxes();
        color[0] = 1;
    } else if (MPR_Plane::MultiPlane_A == orientation) {
        qDebug() << "create_ipw_instance: branch= others";
        ipw->SetPlaneOrientationToZAxes();
        color[2] = 1;
    } else {
        qDebug() << "error: no right orientation";
        return -1;
    }

    ipw->SetSlicePosition(m_slice_pos[orientation]);
    ipw->GetPlaneProperty()->SetColor(color);

    return 0;
}

int MultiPlanarView::reset_img_plane_view_cam(vtkRenderer *ren, int direction) {
    if (!ren) {
        return -3;
    }
    ren->ResetCamera();
    vtkCamera *cam = ren->GetActiveCamera();
    if (MPR_Plane::MultiPlane_A == direction) {
        cam->SetViewUp(0, -1, 0);
    } else if (MPR_Plane::MultiPlane_C == direction) {
        cam->Elevation(90);
        cam->SetViewUp(0, 0, -1);
    } else if (MPR_Plane::MultiPlane_T == direction) {
        cam->Azimuth(-90);
        cam->SetViewUp(0, 1, 0);
    } else { // 3d view
        cam->Elevation(-20);
        cam->SetViewUp(0, -1, 0);
        cam->Azimuth(-30);
        ren->ResetCameraClippingRange();
    }
    cam->OrthogonalizeViewUp();
    return 0;
}

int MultiPlanarView::show() {
    if (!m_render_ready) {
        return -1;
    }
    int i = 0;
    for (i = 0; i < 6; i++) {
        m_ipw_arr[i]->On();
    }
    // act view
    for (i = 0; i < 3; i++) {
        reset_img_plane_view_cam(m_qvtkRen_arr[i]->renderer(), i);
    }
    // 3d view
    reset_img_plane_view_cam(m_qvtkRen_arr[3]->renderer(), 4);
    m_iact->Initialize();
    qDebug() << "02iact print self:";
#include <iostream>
    m_iact->PrintSelf(std::cout, vtkIndent(4));

    m_quickwin->show(); // without this code, nothing will display --eton@210810
    return 0;
}
