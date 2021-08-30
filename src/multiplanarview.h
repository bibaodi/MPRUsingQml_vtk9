#ifndef MULTIPLANARVIEW_H
#define MULTIPLANARVIEW_H

#include <QObject>
//--begin
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
//--2
#include "QQuickVTKRenderItem.h"
#include "QQuickVTKRenderWindow.h"
#include "vtkActor.h"
#include "vtkConeSource.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkImageData.h"
#include "vtkLineSource.h"
#include "vtkNew.h"
#include "vtkPNGWriter.h"
#include "vtkPoints.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
//--end

#define THREED_MPR_PLANE 4
#define THREED_MPR_PLANE_3D 3
#define THREED_MPR_PLANE_ACT 3
#define THREED_MPR_PLANE_ACT3D (THREED_MPR_PLANE_ACT + THREED_MPR_PLANE_3D)

class QVTKRenderItemWidgetCallback;

class MultiPlanarView : public QObject {
    Q_OBJECT
  public:
    explicit MultiPlanarView(vtkVolume16Reader *v16, QObject *parent = nullptr, QObject *root = nullptr);
    void move_slice(int plane, int direction);
    void update_plane_position();
    void print();
    int show();
    enum MPR_Plane { MultiPlane_A = 0, MultiPlane_C, MultiPlane_T, MultiPlane_3D };

  public:
    int m_view_ortho;
    bool m_render_ready = false;
    double m_slice_pos[3];
    double m_slice_pos_range[6];
    double m_spacing[3];
    QObject *m_topLevel;
    QQuickWindow *m_quickwin;
    QVTKInteractor *m_iact;
    QQuickVTKRenderItem *m_qvtkRen_arr[THREED_MPR_PLANE]; // 4 ren, the last has 3 ipw

    vtkSmartPointer<vtkVolume16Reader> m_v16; // this is the data pointer, will be replaced by general dataType;
    vtkSmartPointer<vtkImagePlaneWidget> m_ipw_arr[THREED_MPR_PLANE_ACT3D];

  private:
    vtkNew<vtkPoints> m_points;
    vtkNew<vtkLineSource> m_linesource;
    int update_probe_point(double);
    int create_probe_marker(vtkRenderer *ren);
    int reset_img_plane_view_cam(vtkRenderer *ren, int direction);
    int create_outline_actor(vtkRenderer *ren);
    int create_ipw_instance(vtkImagePlaneWidget *ipw, int orientation, vtkVolume16Reader *v16, vtkRenderer *ren,
                            QVTKInteractor *iact);

  private:
    vtkSmartPointer<QVTKRenderItemWidgetCallback> m_ipw_cb;
};

#endif // MULTIPLANARVIEW_H
