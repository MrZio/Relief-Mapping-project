#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QOpenGLTexture>
#include <QTimer>
#include <QElapsedTimer>

class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    explicit MyGLWidget(QWidget *parent = nullptr);
    ~MyGLWidget();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

public slots:
    void setDepthScale(int value);
    void setMeshScale(int value);
    void setSearchMode(int mode);
    void setLinearSteps(int value);   // <-- NUOVO: slider passi lineari

private:
    struct VertexData {
        QVector3D position;
        QVector2D texCoord;
        QVector3D tangent;
        QVector3D normal;
    };

    void buildCube();

    float m_depthScale  = 0.10f;
    float m_meshScale   = 1.0f;
    int   m_searchMode  = 1;
    int   m_linearSteps = 16;     // <-- default: 16 passi, differenza ben visibile
    float m_lightAngle  = 0.0f;

    float     m_pitch = 0.0f;
    float     m_yaw   = 0.0f;
    float     m_zoom  = 7.0f;
    QVector3D m_panOffset;

    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;
    QMatrix4x4 m_model;

    // 6 depth maps + 6 normal maps per l'IBO
    QOpenGLTexture          *m_depthMaps[6]  = {};
    QOpenGLTexture          *m_normalMaps[6] = {};
    QOpenGLShaderProgram    *m_program       = nullptr;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer            m_vbo;

    QPoint           m_lastPos;
    Qt::MouseButton  m_dragButton = Qt::NoButton;

    QTimer        *m_animTimer = nullptr;
    QElapsedTimer  m_fpsTimer;
    int            m_frameCount = 0;
    float          m_fps        = 0.0f;
};

#endif // MYGLWIDGET_H