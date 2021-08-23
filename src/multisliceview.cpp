#include "multisliceview.h"
#include "QDebug"
#include <QQuickWindow>

MultiSliceView::MultiSliceView(vtkSmartPointer<vtkVolume16Reader> _v16, QObject *parent, QObject *root,
                               const int layout, const int view)
    : QObject(parent), row_cnt(layout), col_cnt(layout), m_topLevel(root), current_view(view) {
    qDebug() << "init~~~row=" << row_cnt << "col=" << col_cnt << "view=" << view;
    m_v16 = _v16;
    slice_idx_base = 0;
    slice_step = 3;
    // make the first view orthogonal to current view
    if (ViewType::MultiSliceVT_A == current_view) {
        m_view_ortho = ViewType::MultiSliceVT_T;
    } else if (ViewType::MultiSliceVT_C == current_view) {
        m_view_ortho = ViewType::MultiSliceVT_A;
    } else if (ViewType::MultiSliceVT_T == current_view) {
        m_view_ortho = ViewType::MultiSliceVT_C;
    }
    if (nullptr == m_topLevel) {
        qDebug() << "qml root item is nullptr";
        return;
    }
    m_quickwin = qobject_cast<QQuickWindow *>(m_topLevel);

    QString renderNames = "MultiSlice";
    int _r = 0, _c = 0, i = 0, ret = 0;
    for (_r = 0; _r < row_cnt; _r++) {
        for (_c = 0; _c < col_cnt; _c++) {
            m_qvtkRen_arr[_r * row_cnt + _c] =
                m_topLevel->findChild<QQuickVTKRenderItem *>(QString("%1%2%3").arg(renderNames).arg(_r).arg(_c));
            if (!m_qvtkRen_arr[_r * row_cnt + _c]) {
                qDebug() << "vtk widget not found! >>" << renderNames << _r << _c;
                return;
            }
        }
    }

    vtkSmartPointer<vtkRenderWindow> vtk_ren_win = m_qvtkRen_arr[0]->renderWindow()->renderWindow();
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> vtk_gl_renwin =
        static_cast<vtkGenericOpenGLRenderWindow *>(vtk_ren_win.GetPointer());
    m_iact = dynamic_cast<QVTKInteractor *>(vtk_gl_renwin->GetInteractor());

    if (_r == row_cnt && _c == col_cnt) {
        m_render_ready = true;
    }
    for (i = 0; i < row_cnt * col_cnt; i++) {
        qDebug() << "i=" << i;
        m_ipw_arr[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();

        int _view = current_view;
        if (0 == i) {
            _view = m_view_ortho;
        }

        if (i > 0) {
            float m_slice_pos;
            ret = create_ipw_instance(m_ipw_arr[i], _view, m_v16, m_qvtkRen_arr[i]->renderer(), m_iact,
                                      slice_idx_base + i * slice_step);
            if (ret) {
                qDebug() << "ipw create error. i=" << i;
                return;
            }
            m_slice_pos = m_ipw_arr[i]->GetSlicePosition();
            ret = create_slice_pos_line(m_slice_pos, m_ipw_arr[0], _view, m_qvtkRen_arr[0]->renderer());
            if (ret) {
                qDebug() << "slice line create error. i=" << i;
                return;
            }
        } else {
            // view-0 first view which is orthonocal to current-view
            qDebug() << "current_view_ortho=" << _view;
            create_ipw_instance(m_ipw_arr[i], _view, m_v16, m_qvtkRen_arr[i]->renderer(), m_iact,
                                slice_idx_base + i * slice_step);
            if (ret) {
                qDebug() << "ipw create error. i=0";
                return;
            }
        }
    }
}

int MultiSliceView::create_ipw_instance(vtkSmartPointer<vtkImagePlaneWidget> &ipw, int orientation,
                                        vtkSmartPointer<vtkVolume16Reader> &m_v16, vtkRenderer *ren,
                                        QVTKInteractor *m_iact, int slice_idx) {
    if (!ipw) {
        return -1;
    }
    if (!m_v16 || !ipw) {
        return -2;
    }
    if (!ren) {
        return -3;
    }
    if (!m_iact) {
        return -4;
    }
    ipw->SetInputConnection(m_v16->GetOutputPort());
    ipw->SetCurrentRenderer(ren);
    ipw->SetInteractor(m_iact);
    ipw->RestrictPlaneToVolumeOn();
    // image style not allowed 3d rotate
    vtkNew<vtkInteractorStyleImage> style;
    m_iact->SetInteractorStyle(style);

    double color[3] = {0, 0, 0};
    qDebug() << "create_ipw_instance: orientation=" << orientation;
    if (ViewType::MultiSliceVT_T == orientation) {
        qDebug() << "create_ipw_instance: branch=x";
        ipw->SetPlaneOrientationToXAxes();
        ipw->SetSliceIndex(slice_idx);
        color[1] = 1;
    } else if (ViewType::MultiSliceVT_C == orientation) {
        qDebug() << "create_ipw_instance: branch=y";
        ipw->SetPlaneOrientationToYAxes();
        ipw->SetSliceIndex(slice_idx);
        color[0] = 1;
    } else if (ViewType::MultiSliceVT_A == orientation) {
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

int MultiSliceView::create_slice_pos_line(float m_slice_pos, vtkSmartPointer<vtkImagePlaneWidget> &ipw0,
                                          int orientation, vtkRenderer *ren) {
    if (!ipw0) {
        return -1;
    }
    if (!ren) {
        return -3;
    }
    if (orientation < ViewType::MultiSliceVT_A || orientation >= ViewType::MultiSliceVT_3D) {
        return -4;
    }
    // TODO: data's coordination bounding need to be variant
    double ipw0_pos = ipw0->GetSlicePosition(); // when ipw0 == A then this is a z;
    double p0[3] = {0.0, 0.0, 0.0};
    double p1[3] = {201.6, 201.6, 138.0};
    // Visualize
    double color[3] = {0, 0, 0};
    if (ViewType::MultiSliceVT_T == orientation) {
        color[1] = 1;
        p1[0] = p0[0] = m_slice_pos;
        p1[1] = p0[1] = ipw0_pos;
    } else if (ViewType::MultiSliceVT_C == orientation) {
        color[0] = 1;
        p1[1] = p0[1] = m_slice_pos;
        p1[2] = p0[2] = ipw0_pos;
    } else if (ViewType::MultiSliceVT_A == orientation) {
        color[2] = 1;
        p1[2] = p0[2] = m_slice_pos;
        p1[0] = p0[0] = ipw0_pos;
    }
    // Create two points, P0 and P1
    vtkNew<vtkLineSource> lineSource;
    lineSource->SetPoint1(p0);
    lineSource->SetPoint2(p1);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(lineSource->GetOutputPort());
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetLineWidth(1);
    actor->GetProperty()->SetColor(color);
    ren->AddActor(actor);
    return 0;
}

int MultiSliceView::reset_img_plane_view_cam(vtkRenderer *ren, int direction) {
    if (!ren) {
        return -3;
    }
    ren->ResetCamera();
    vtkCamera *cam = ren->GetActiveCamera();
    if (ViewType::MultiSliceVT_A == direction) {
        cam->SetViewUp(0, -1, 0);
    } else if (ViewType::MultiSliceVT_C == direction) {
        cam->Elevation(90);
        cam->SetViewUp(0, 0, -1);
    } else if (ViewType::MultiSliceVT_T == direction) {
        qDebug() << "default is Z direction";
        cam->Azimuth(-90);
        cam->SetViewUp(0, 1, 0);
    }
    cam->OrthogonalizeViewUp();
    return 0;
}

int MultiSliceView::show() {
    if (!m_render_ready) {
        return -1;
    }
    int i = 0;
    for (i = 0; i < row_cnt * col_cnt; i++) {
        m_ipw_arr[i]->On();
    }
    reset_img_plane_view_cam(m_qvtkRen_arr[0]->renderer(), m_view_ortho);
    for (i = 1; i < row_cnt * col_cnt; i++) {
        reset_img_plane_view_cam(m_qvtkRen_arr[i]->renderer(), current_view);
    }
    m_quickwin->show(); // without this code, nothing will display --eton@210810
    return 0;
}
