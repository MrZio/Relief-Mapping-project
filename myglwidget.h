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

    // Mouse: rotazione (left) + traslazione (right) + zoom (wheel)
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

public slots:
    void setDepthScale(int value);      // slider depth (0-100 → 0.0-0.3)
    void setMeshScale(int value);       // slider scala mesh (0-100 → 0.5-2.0)
    void setSearchMode(int mode);       // 0=linear, 1=linear+binary

private:
    struct VertexData {
        QVector3D position;
        QVector2D texCoord;
        QVector3D tangent;
        QVector3D normal;
    };

    void buildCube();

    // Parametri di rendering
    float m_depthScale  = 0.10f;
    float m_meshScale   = 1.0f;
    int   m_searchMode  = 1;        // default: linear + binary
    float m_lightAngle  = 0.0f;

    // Camera
    float m_pitch = 0.0f;
    float m_yaw   = 0.0f;
    float m_zoom  = 7.0f;           // distanza camera dal centro
    QVector3D m_panOffset;          // traslazione XY

    // Matrici
    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;
    QMatrix4x4 m_model;

    // OpenGL objects
    QOpenGLTexture *m_depthMaps[6] = {nullptr};
    QOpenGLTexture *m_normalMaps[6] = {nullptr};
    QOpenGLShaderProgram    *m_program   = nullptr;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer            m_vbo;

    // Input
    QPoint m_lastPos;
    Qt::MouseButton m_dragButton = Qt::NoButton;

    // Timers
    QTimer        *m_animTimer = nullptr;
    QElapsedTimer  m_fpsTimer;
    int            m_frameCount = 0;
    float          m_fps        = 0.0f;
};

#endif // MYGLWIDGET_H