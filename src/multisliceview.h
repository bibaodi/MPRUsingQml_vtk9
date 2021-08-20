#ifndef MULTISLICEVIEW_H
#define MULTISLICEVIEW_H

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
#include "vtkNew.h"
#include "vtkPNGWriter.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
//--end

#define ThreeDMultiSliceLayout3 3
#define ThreeDMultiSliceLayout4 (ThreeDMultiSliceLayout3 + 1)

enum ViewType { A = 0, C, T, D3 };

class MultiSliceView : public QObject {
    Q_OBJECT
  public:
    explicit MultiSliceView(vtkSmartPointer<vtkVolume16Reader> v16, QObject *parent = nullptr, QObject *root = nullptr,
                            const int layout = 3, const int view = 1);
    void print();
    int show();

  private:
    vtkSmartPointer<vtkVolume16Reader> v16; // this is the data pointer, will be replaced by general dataType;
    bool render_ready = false;
    const int row_cnt;
    const int col_cnt;
    QObject *topLevel;
    QQuickWindow *quickwin;
    QVTKInteractor *iact;
    vtkSmartPointer<vtkImagePlaneWidget> ipw_arr[ThreeDMultiSliceLayout4 * ThreeDMultiSliceLayout4];
    QQuickVTKRenderItem *qvtkRen_arr[ThreeDMultiSliceLayout4 * ThreeDMultiSliceLayout4];
    const int current_view;
    int view_ortho;
    int slice_idx_base;
    int slice_step;
    int create_ipw_instance(vtkSmartPointer<vtkImagePlaneWidget> &ipw, int orientation,
                            vtkSmartPointer<vtkVolume16Reader> &v16, vtkRenderer *ren, QVTKInteractor *iact,
                            int slice_idx);
    int create_slice_pos_line(float slice_pos, vtkSmartPointer<vtkImagePlaneWidget> &ipw0, int orientation,
                              vtkRenderer *ren);
    int reset_img_plane_view_cam(vtkRenderer *ren, int direction);
  signals:
};

#endif // MULTISLICEVIEW_H
