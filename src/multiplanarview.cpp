#include "multiplanarview.h"
#include <QQuickWindow>
#include <algorithm>

#include "vtkAlgorithm.h"
#include "vtkCellArray.h"
#include "vtkNamedColors.h"

class QVTKRenderItemWidgetCallback : public vtkCommand {
  public:
    static QVTKRenderItemWidgetCallback *New() { return new QVTKRenderItemWidgetCallback; }

    void Execute(vtkObject *caller, unsigned long event, void *calldata) override {
        if (!m_mpv) {
            qDebug() << "mpv is null";
            return;
        }
        qDebug() << "eventID=" << event << "\t calldata-ptr=" << calldata;
        int slice_delta = 0;
        if (vtkCommand::MouseWheelForwardEvent == event) {
            slice_delta = 1;
        } else if (vtkCommand::MouseWheelBackwardEvent == event) {
            slice_delta = -1;
        }
        if (vtkCommand::LeftButtonPressEvent == event) {
            m_activate_plane = (m_activate_plane + 1) % 3;
        }
        qDebug() << "plane = " << m_activate_plane << "slice delta:" << slice_delta;
        if (1 == abs(slice_delta)) {
            m_mpv->move_slice(m_activate_plane, slice_delta);
            m_mpv->update_plane_position();
        }
        // return;
        vtkImagePlaneWidget *planeWidget = reinterpret_cast<vtkImagePlaneWidget *>(caller);
        int slice_idx = planeWidget->GetSliceIndex();
        int act_idx = -1;
        qDebug() << "callback: slice index=" << slice_idx << " act_index" << act_idx;
        if (ipw_3d[0] == planeWidget) {
            act_idx = 0;
        } else if (ipw_3d[1] == planeWidget) {
            act_idx = 1;
        } else if (ipw_3d[2] == planeWidget) {
            act_idx = 2;
        }
        if (act_idx > -1) {
            ipw_act[act_idx]->SetSliceIndex(slice_idx);
        }
    }

    QVTKRenderItemWidgetCallback() {
        m_mpv = nullptr;
        m_activate_plane = 0;
        for (int i = 0; i < 3; i++) {
            qDebug() << "init callback:i=" << i;
            ipw_3d[i] = nullptr;
            ipw_act[i] = nullptr;
        }
    }

    vtkSmartPointer<vtkImagePlaneWidget> ipw_3d[3];
    vtkSmartPointer<vtkImagePlaneWidget> ipw_act[3];
    int m_activate_plane;
    MultiPlanarView *m_mpv;
};

MultiPlanarView::MultiPlanarView(vtkSmartPointer<vtkVolume16Reader> _v16, QObject *parent, QObject *root)
    : QObject(parent), m_topLevel(root) {
    m_points = vtkSmartPointer<vtkPoints>::New();
    m_linesource = vtkSmartPointer<vtkLineSource>::New();
    qDebug() << "MPR view init~";
    m_v16 = _v16;
    if (!m_v16) {
        qDebug() << "Err:data invalid";
        return;
    }
    //--00 set default slice_position
    int *extent = m_v16->GetOutput()->GetExtent();
    double *spacing = m_v16->GetOutput()->GetSpacing();
    for (int i = 0; i < 6; i++) {
        if (i < 3) {
            m_slice_pos[i] = spacing[i] * extent[2 * i + 1] * (0.4 + 0.1 * i);
            m_spacing[i] = spacing[i];
        }
        m_slice_pos_range[i] = spacing[i / 2] * extent[i];
    }
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
    create_outline_actor(m_qvtkRen_arr[MultiPlane_3D]->renderer());
    m_qvtkRen_arr[MultiPlane_3D]->update();
    //--04 get window and interactor
    vtkSmartPointer<vtkRenderWindow> vtk_ren_win = m_qvtkRen_arr[0]->renderWindow()->renderWindow();
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> vtk_gl_renwin =
        static_cast<vtkGenericOpenGLRenderWindow *>(vtk_ren_win.GetPointer());
    m_iact = (dynamic_cast<QVTKInteractor *>(vtk_gl_renwin->GetInteractor()));
    //--05 create ACT ipw

    int i = 0, ret = 0;
    vtkNew<vtkInteractorStyleImage> style;
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
    //--08  create callback for all three 3d-view's ipw
    m_ipw_cb = vtkSmartPointer<QVTKRenderItemWidgetCallback>::New();
    m_ipw_cb->m_mpv = this;
    for (i = 0; i < 3; i++) {
        m_ipw_cb->ipw_act[i] = m_ipw_arr[i].GetPointer();
        m_ipw_cb->ipw_3d[i] = m_ipw_arr[i + 3].GetPointer();
        m_ipw_arr[3 + i]->AddObserver(vtkCommand::AnyEvent, m_ipw_cb);
    }
    //--09a add probe marker
    create_probe_marker(m_qvtkRen_arr[MultiPlane_3D]->renderer());

#if 0
    qDebug() << "01iact print self:" << m_iact;
#include <iostream>
    m_iact->PrintSelf(std::cout, vtkIndent(4));
#endif
    qDebug() << "MPR view init~ Finish!";
}

int MultiPlanarView::update_probe_point(double p) {
    if (!m_points) {
        return -1;
    }
    double pt0[3] = {m_slice_pos_range[1], m_slice_pos_range[2], p};
    double pt1[3] = {m_slice_pos_range[0], m_slice_pos_range[2], p};
    m_points->SetPoint(0, pt0);
    m_points->Modified();

    // line
    m_linesource->SetPoint1(pt0);
    m_linesource->SetPoint2(pt1);
    m_linesource->Modified();
    return 0;
}

int MultiPlanarView::create_probe_marker(vtkSmartPointer<vtkRenderer> ren) {
    if (!ren) {
        return -1;
    }
    if (!m_points) {
        return -2;
    }
    if (!m_linesource) {
        return -3;
    }
    vtkNew<vtkNamedColors> colors;
    double pt0[3] = {m_slice_pos_range[1], m_slice_pos_range[2], m_slice_pos[2]};
    double pt1[3] = {m_slice_pos_range[0], m_slice_pos_range[2], m_slice_pos[2]};
    vtkIdType point_id[1] = {0};
    // vtkNew<vtkPoints> m_points;
    if (m_points->GetNumberOfPoints() < 1) {
        point_id[0] = m_points->InsertNextPoint(pt0);
    } else {
        m_points->SetPoint(point_id[0], pt0);
    }
    vtkNew<vtkCellArray> vertices;
    vertices->InsertNextCell(1, point_id);

    // Create a polydata object
    vtkNew<vtkPolyData> point_pd;

    // Set the points and vertices we created as the geometry and topology of the
    // polydata
    point_pd->SetPoints(m_points);
    point_pd->SetVerts(vertices);

    // Visualize
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(point_pd);

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(colors->GetColor3d("yellow_ochre").GetData());
    actor->GetProperty()->SetPointSize(20);

    ren->AddActor(actor);

    //--line
    m_linesource->SetPoint1(pt0);
    m_linesource->SetPoint2(pt1);
    vtkNew<vtkPolyDataMapper> mapper_line;
    mapper_line->SetInputConnection(m_linesource->GetOutputPort());
    vtkNew<vtkActor> actor_line;
    actor_line->SetMapper(mapper_line);
    actor_line->GetProperty()->SetLineWidth(4);
    actor_line->GetProperty()->SetColor(colors->GetColor3d("yellow_ochre").GetData());
    ren->AddActor(actor_line);
    return 0;
}

int MultiPlanarView::create_outline_actor(vtkSmartPointer<vtkRenderer> ren) {
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

    double color[3] = {0, 0, 0};
    // qDebug() << "create_ipw_instance: orientation=" << orientation;
    if (MultiPlane_T == orientation) {
        // qDebug() << "create_ipw_instance: branch=x";
        ipw->SetPlaneOrientationToXAxes();
        color[1] = 1;
    } else if (MultiPlane_C == orientation) {
        // qDebug() << "create_ipw_instance: branch=y";
        ipw->SetPlaneOrientationToYAxes();
        color[0] = 1;
    } else if (MultiPlane_A == orientation) {
        // qDebug() << "create_ipw_instance: branch= others";
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

void MultiPlanarView::move_slice(int plane, int direction) {
    int p = abs(plane % 3);
    double delta = (double)direction / abs(direction);
    delta *= m_spacing[p];
    m_slice_pos[p] += delta;
    m_slice_pos[p] = std::max(m_slice_pos_range[p * 2], std::min(m_slice_pos_range[p * 2 + 1], m_slice_pos[p]));
    if (MultiPlane_A == (2 - plane)) {
        update_probe_point(m_slice_pos[p]);
    }
    return;
}

void MultiPlanarView::update_plane_position() {
    for (int i = 0; i < 3; i++) {
        qDebug() << "ipw pointer:" << m_ipw_arr[i].GetPointer() << "3d:" << m_ipw_arr[3 + i]
                 << "\tposition=" << m_slice_pos[2 - i];
        m_ipw_arr[i]->SetSlicePosition(m_slice_pos[2 - i]);
        m_ipw_arr[3 + i]->SetSlicePosition(m_slice_pos[2 - i]);
        m_ipw_arr[i]->GetCurrentRenderer()->ResetCamera();
    }
    return;
}

int MultiPlanarView::reset_img_plane_view_cam(vtkRenderer *ren, int direction) {
    if (!ren) {
        return -3;
    }
    ren->ResetCamera();
    vtkCamera *cam = ren->GetActiveCamera();
    if (MultiPlane_A == direction) {
        cam->SetViewUp(0, -1, 0);
    } else if (MultiPlane_C == direction) {
        cam->Elevation(90);
        cam->SetViewUp(0, 0, -1);
    } else if (MultiPlane_T == direction) {
        cam->Azimuth(-90);
        cam->SetViewUp(0, 1, 0);
    } else { // 3d view
        cam->Elevation(-20);
        cam->SetViewUp(0, -1, 0);
        cam->Azimuth(-30);
    }

    ren->ResetCameraClippingRange();
    cam->OrthogonalizeViewUp();
    return 0;
}

int MultiPlanarView::show() {
    if (!m_render_ready) {
        return -1;
    }

    m_quickwin->show(); // without this code, nothing will display --eton@210810
    int i = 0;
    for (i = 0; i < 6; i++) {
        m_ipw_arr[i]->On();
        if (i < 3) {
            m_ipw_arr[i]->GetCurrentRenderer()->InteractiveOff();
        }
        qDebug() << i << ":get interaction=" << m_ipw_arr[i]->GetInteraction();
    }

    // act view
    for (i = 0; i < 3; i++) {
        reset_img_plane_view_cam(m_qvtkRen_arr[i]->renderer(), i);
    }
    // 3d view
    reset_img_plane_view_cam(m_qvtkRen_arr[3]->renderer(), 4);
#if 1
    qDebug() << "02iact print self:" << m_iact;
#include <iostream>
    m_iact->RemoveAllObservers();

    m_iact->AddObserver(vtkCommand::LeftButtonPressEvent, m_ipw_cb);
    // m_iact->AddObserver(vtkCommand::MouseMoveEvent, m_ipw_cb);
    m_iact->AddObserver(vtkCommand::MouseWheelForwardEvent, m_ipw_cb);
    m_iact->AddObserver(vtkCommand::MouseWheelBackwardEvent, m_ipw_cb);
    // m_iact->PrintSelf(std::cout, vtkIndent(4));
#endif
    return 0;
}
