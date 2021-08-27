#include "threedimensionveiw.h"
#include <QDebug>

#include "multiplanarview.h"
#include "multisliceview.h"

ThreeDimensionVeiw::ThreeDimensionVeiw(vtkVolume16Reader *img_data, QObject *parent, const int v_layout, QObject *root)
    : QObject(parent), m_img_data(img_data), m_view_layout(v_layout), m_qml_root(root) {
    qDebug() << "ThreeDimensionVeiw init~";
    //--01 for event test
    QObject *btn_select = m_qml_root->findChild<QObject *>("btn_select");
    QObject *btn_translate = m_qml_root->findChild<QObject *>("btn_translate");
    if (!btn_select || !btn_translate) {
        qDebug() << "Err: btn not found!!!";
        return;
    }
    QObject::connect(btn_select, SIGNAL(qmlSignal(QString)), this, SLOT(cppSlot(QString)));
    QObject::connect(btn_translate, SIGNAL(qmlSignal2(int)), this, SLOT(cppSlot2(int)));

    //--02 create instance
    create_view_instance(m_view_layout);
}

ThreeDimensionVeiw::~ThreeDimensionVeiw() { qDebug() << "ThreeDimensionVeiw ~deconstruction~"; }

void ThreeDimensionVeiw::cppSlot(const QString &msg) {
    qDebug() << "Called the C++ slot with message:" << msg;
    release_view_instance();
    create_view_instance(m_view_layout);
    return;
}

void ThreeDimensionVeiw::cppSlot2(const int num) {
    qDebug() << "Called the C++ slot with message " << num;
    int qml_layout = 0;
    if (0 == num) {
        qml_layout = VIEW_3D_MPR4;
    } else if (1 == num) {
        qml_layout = VIEW_3D_MS3x3;
    } else if (2 == num) {
        qml_layout = VIEW_3D_MS4x4;
    }

    if (qml_layout != m_view_layout) {
        release_view_instance();
        create_view_instance(qml_layout);
    }
    return;
}

int ThreeDimensionVeiw::create_view_instance(int v_layout) {
    if (v_layout > VIEW_3D_MPR) {
        MultiPlanarView *mpr = new MultiPlanarView(m_img_data, nullptr, m_qml_root);
        mpr->show();
        qDebug() << "create new instance MPR";
        m_view_ptr = mpr;
    } else {
        int msc_count = 3;
        if (VIEW_3D_MS4x4 == v_layout) {
            msc_count = 4;
        }
        MultiSliceView *msc =
            new MultiSliceView(m_img_data, nullptr, m_qml_root, msc_count, MultiSliceView::MultiSliceVT_C);
        int ret = msc->show();
        qDebug() << "create new instance MSC:" << msc_count << " ret=" << ret;
        m_view_ptr = msc;
    }
    if (m_view_layout != v_layout) {
        qDebug() << "Error:create_V_instance: " << v_layout << m_view_layout;
        qDebug() << "instance pointer:" << m_view_ptr;
        m_view_layout = v_layout;
    }
    return 0;
}

int ThreeDimensionVeiw::release_view_instance() {
    if (m_view_ptr) {
        if (m_view_layout > VIEW_3D_MPR) {
            delete (MultiPlanarView *)m_view_ptr;
            qDebug() << "release instance mpr:" << m_view_layout << " at:" << m_view_ptr;
        } else {
            delete (MultiSliceView *)m_view_ptr;
            qDebug() << "release instance msc:" << m_view_layout << " at:" << m_view_ptr;
        }
        m_view_ptr = nullptr;
    }
    return 0;
}
