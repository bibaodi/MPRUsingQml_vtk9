#ifndef THREEDIMENSIONVEIW_H
#define THREEDIMENSIONVEIW_H

#include "vtkImageData.h"
#include "vtkMeta.h"
#include "vtkSmartPointer.h"
#include "vtkVolume16Reader.h"
#include <QObject>

enum VIEW_3D_LAYOUT {
    VIEW_3D_MS3x3 = 9,
    VIEW_3D_MS4x4 = 16,
    VIEW_3D_MPR = 40,
    VIEW_3D_MPR1,
    VIEW_3D_MPR2,
    VIEW_3D_MPR3,
    VIEW_3D_MPR4
};

class ThreeDimensionVeiw : public QObject {
    Q_OBJECT
  public:
    explicit ThreeDimensionVeiw(vtkVolume16Reader *imgdata, QObject *parent = nullptr,
                                const int v_layout = VIEW_3D_MPR4, QObject *root = nullptr);

    ~ThreeDimensionVeiw();

  private:
    vtkSmartPointer<vtkVolume16Reader> m_img_data;
    int m_view_layout;
    QObject *m_qml_root;
    void *m_view_ptr;
    int release_view_instance();
    int create_view_instance(int);

  public slots:
    void cppSlot(const QString &);
    void cppSlot2(const int);
};

#endif // THREEDIMENSIONVEIW_H
