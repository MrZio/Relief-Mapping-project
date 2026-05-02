#ifndef MYGLWIDGET_H
#define MYGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
// Aggiungiamo le classi per gestire memoria e shader
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QOpenGLTexture>

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

    // ... dopo i metodi initializeGL, paintGL, ecc. ...

public slots:
    // Questa funzione verrà chiamata ogni volta che muovi lo slider
    void setDepthScale(int value);

private:
    float m_depthScale = 0.1f;
    float m_pitch = 0.0f; // Rotazione Su/Giù (Asse X)
    float m_yaw = 0.0f;
    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;
    QMatrix4x4 m_model;
    QOpenGLTexture *m_heightMap;
    QPoint m_lastPos;

    struct VertexData {
        QVector3D position;
        QVector2D texCoord;
        QVector3D tangent; //tangent space
        QVector3D normal;
    };



private:
    // I nostri tre strumenti fondamentali
    QOpenGLShaderProgram *m_program;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
};

#endif // MYGLWIDGET_H