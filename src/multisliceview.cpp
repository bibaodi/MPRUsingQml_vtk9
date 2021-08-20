#include "multisliceview.h"
#include "QDebug"
#include <QQuickWindow>

MultiSliceView::MultiSliceView(vtkSmartPointer<vtkVolume16Reader> _v16, QObject *parent, QObject *root,
                               const int layout, const int view)
    : QObject(parent), row_cnt(layout), col_cnt(layout), topLevel(root), current_view(view) {
    qDebug() << "init~~~row=" << row_cnt << "col=" << col_cnt << "view=" << view;
    v16 = _v16;
    slice_idx_base = 0;
    slice_step = 3;
    // make the first view orthogonal to current view
    if (ViewType::A == current_view) {
        view_ortho = ViewType::T;
    } else if (ViewType::C == current_view) {
        view_ortho = ViewType::A;
    } else if (ViewType::T == current_view) {
        view_ortho = ViewType::C;
    }
    if (nullptr == topLevel) {
        qDebug() << "qml root item is nullptr";
        return;
    }
    quickwin = qobject_cast<QQuickWindow *>(topLevel);

    QString renderNames = "MultiSlice";
    int _r = 0, _c = 0, i = 0, ret = 0;
    for (_r = 0; _r < row_cnt; _r++) {
        for (_c = 0; _c < col_cnt; _c++) {
            qvtkRen_arr[_r * row_cnt + _c] =
                topLevel->findChild<QQuickVTKRenderItem *>(QString("%1%2%3").arg(renderNames).arg(_r).arg(_c));
            if (!qvtkRen_arr[_r * row_cnt + _c]) {
                qDebug() << "vtk widget not found! >>" << renderNames << _r << _c;
                return;
            }
        }
    }

    vtkSmartPointer<vtkRenderWindow> vtk_ren_win = qvtkRen_arr[0]->renderWindow()->renderWindow();
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> vtk_gl_renwin =
        static_cast<vtkGenericOpenGLRenderWindow *>(vtk_ren_win.GetPointer());
    iact = dynamic_cast<QVTKInteractor *>(vtk_gl_renwin->GetInteractor());

    if (_r == row_cnt && _c == col_cnt) {
        render_ready = true;
    }
    for (i = 0; i < row_cnt * col_cnt; i++) {
        qDebug() << "i=" << i;
        ipw_arr[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();

        int _view = current_view;
        if (0 == i) {
            _view = view_ortho;
        }

        if (i > 0) {
            float slice_pos;
            ret = create_ipw_instance(ipw_arr[i], _view, v16, qvtkRen_arr[i]->renderer(), iact,
                                      slice_idx_base + i * slice_step);
            if (ret) {
                qDebug() << "ipw create error. i=" << i;
                return;
            }
            slice_pos = ipw_arr[i]->GetSlicePosition();
            ret = create_slice_pos_line(slice_pos, ipw_arr[0], _view, qvtkRen_arr[0]->renderer());
            if (ret) {
                qDebug() << "slice line create error. i=" << i;
                return;
            }
        } else {
            // view-0 first view which is orthonocal to current-view
            qDebug() << "current_view_ortho=" << _view;
            create_ipw_instance(ipw_arr[i], _view, v16, qvtkRen_arr[i]->renderer(), iact,
                                slice_idx_base + i * slice_step);
            if (ret) {
                qDebug() << "ipw create error. i=0";
                return;
            }
        }
    }
}

int MultiSliceView::create_ipw_instance(vtkSmartPointer<vtkImagePlaneWidget> &ipw, int orientation,
                                        vtkSmartPointer<vtkVolume16Reader> &v16, vtkRenderer *ren, QVTKInteractor *iact,
                                        int slice_idx) {
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

int MultiSliceView::create_slice_pos_line(float slice_pos, vtkSmartPointer<vtkImagePlaneWidget> &ipw0, int orientation,
                                          vtkRenderer *ren) {
    if (!ipw0) {
        return -1;
    }
    if (!ren) {
        return -3;
    }
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

int MultiSliceView::reset_img_plane_view_cam(vtkRenderer *ren, int direction) {
    if (!ren) {
        return -3;
    }
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

int MultiSliceView::show() {
    if (!render_ready) {
        return -1;
    }
    int i = 0;
    for (i = 0; i < row_cnt * col_cnt; i++) {
        ipw_arr[i]->On();
    }
    reset_img_plane_view_cam(qvtkRen_arr[0]->renderer(), view_ortho);
    for (i = 1; i < row_cnt * col_cnt; i++) {
        reset_img_plane_view_cam(qvtkRen_arr[i]->renderer(), current_view);
    }
    quickwin->show(); // without this code, nothing will display --eton@210810
    return 0;
}
