#include "myglwidget.h"
#include <QDebug>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>

MyGLWidget::MyGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

    // Animazione luce + aggiornamento FPS
    m_animTimer = new QTimer(this);
    connect(m_animTimer, &QTimer::timeout, this, [this]() {
        m_lightAngle += 0.4f;
        if (m_lightAngle >= 360.0f) m_lightAngle -= 360.0f;
        update();
    });
    m_animTimer->start(16);   // ~60 fps
    m_fpsTimer.start();
}

MyGLWidget::~MyGLWidget() {}

// ----------------------------------------------------------------
//  Costruisce il VBO del cubo con tangenti e normali per faccia
// ----------------------------------------------------------------
void MyGLWidget::buildCube()
{
    // 6 facce × 6 vertici = 36 vertici
    // { position, texCoord, tangent, normal }
    VertexData verts[] = {
        // FRONT  Z=+1  N(0,0,1)  T(1,0,0)
        {{-1,-1, 1},{0,0},{1,0,0},{0,0,1}}, {{ 1,-1, 1},{1,0},{1,0,0},{0,0,1}},
        {{ 1, 1, 1},{1,1},{1,0,0},{0,0,1}}, {{-1,-1, 1},{0,0},{1,0,0},{0,0,1}},
        {{ 1, 1, 1},{1,1},{1,0,0},{0,0,1}}, {{-1, 1, 1},{0,1},{1,0,0},{0,0,1}},
        // BACK   Z=-1  N(0,0,-1) T(-1,0,0)
        {{ 1,-1,-1},{0,0},{-1,0,0},{0,0,-1}}, {{-1,-1,-1},{1,0},{-1,0,0},{0,0,-1}},
        {{-1, 1,-1},{1,1},{-1,0,0},{0,0,-1}}, {{ 1,-1,-1},{0,0},{-1,0,0},{0,0,-1}},
        {{-1, 1,-1},{1,1},{-1,0,0},{0,0,-1}}, {{ 1, 1,-1},{0,1},{-1,0,0},{0,0,-1}},
        // RIGHT  X=+1  N(1,0,0)  T(0,0,-1)
        {{ 1,-1, 1},{0,0},{0,0,-1},{1,0,0}}, {{ 1,-1,-1},{1,0},{0,0,-1},{1,0,0}},
        {{ 1, 1,-1},{1,1},{0,0,-1},{1,0,0}}, {{ 1,-1, 1},{0,0},{0,0,-1},{1,0,0}},
        {{ 1, 1,-1},{1,1},{0,0,-1},{1,0,0}}, {{ 1, 1, 1},{0,1},{0,0,-1},{1,0,0}},
        // LEFT   X=-1  N(-1,0,0) T(0,0,1)
        {{-1,-1,-1},{0,0},{0,0,1},{-1,0,0}}, {{-1,-1, 1},{1,0},{0,0,1},{-1,0,0}},
        {{-1, 1, 1},{1,1},{0,0,1},{-1,0,0}}, {{-1,-1,-1},{0,0},{0,0,1},{-1,0,0}},
        {{-1, 1, 1},{1,1},{0,0,1},{-1,0,0}}, {{-1, 1,-1},{0,1},{0,0,1},{-1,0,0}},
        // TOP    Y=+1  N(0,1,0)  T(1,0,0)
        {{-1, 1, 1},{0,0},{1,0,0},{0,1,0}}, {{ 1, 1, 1},{1,0},{1,0,0},{0,1,0}},
        {{ 1, 1,-1},{1,1},{1,0,0},{0,1,0}}, {{-1, 1, 1},{0,0},{1,0,0},{0,1,0}},
        {{ 1, 1,-1},{1,1},{1,0,0},{0,1,0}}, {{-1, 1,-1},{0,1},{1,0,0},{0,1,0}},
        // BOTTOM Y=-1  N(0,-1,0) T(1,0,0)
        {{-1,-1,-1},{0,0},{1,0,0},{0,-1,0}}, {{ 1,-1,-1},{1,0},{1,0,0},{0,-1,0}},
        {{ 1,-1, 1},{1,1},{1,0,0},{0,-1,0}}, {{-1,-1,-1},{0,0},{1,0,0},{0,-1,0}},
        {{ 1,-1, 1},{1,1},{1,0,0},{0,-1,0}}, {{-1,-1, 1},{0,1},{1,0,0},{0,-1,0}},
        };

    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(verts, sizeof(verts));

    int posLoc = m_program->attributeLocation("aPos");
    m_program->enableAttributeArray(posLoc);
    m_program->setAttributeBuffer(posLoc, GL_FLOAT, offsetof(VertexData,position), 3, sizeof(VertexData));

    int texLoc = m_program->attributeLocation("aTexCoord");
    m_program->enableAttributeArray(texLoc);
    m_program->setAttributeBuffer(texLoc, GL_FLOAT, offsetof(VertexData,texCoord), 2, sizeof(VertexData));

    int tanLoc = m_program->attributeLocation("aTangent");
    m_program->enableAttributeArray(tanLoc);
    m_program->setAttributeBuffer(tanLoc, GL_FLOAT, offsetof(VertexData,tangent), 3, sizeof(VertexData));

    int nrmLoc = m_program->attributeLocation("aNormal");
    m_program->enableAttributeArray(nrmLoc);
    m_program->setAttributeBuffer(nrmLoc, GL_FLOAT, offsetof(VertexData,normal), 3, sizeof(VertexData));
}

void MyGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);

    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/vertex.glsl");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fragment.glsl");
    if (!m_program->link())
        qWarning() << "Shader link error:" << m_program->log();

    // Texture dummy (il fragment usa noise procedurale, ma serve il binding)
    unsigned char px[4] = {128, 128, 128, 255};
    m_heightMap = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_heightMap->setSize(2, 2);
    m_heightMap->setFormat(QOpenGLTexture::RGBA8_UNorm);
    m_heightMap->allocateStorage();
    m_heightMap->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, px);
    m_heightMap->setMinificationFilter(QOpenGLTexture::Nearest);
    m_heightMap->setMagnificationFilter(QOpenGLTexture::Nearest);

    m_vao.create();
    m_vao.bind();
    buildCube();
    m_vao.release();
    m_vbo.release();
}

void MyGLWidget::resizeGL(int w, int h)
{
    m_projection.setToIdentity();
    m_projection.perspective(45.0f, float(w) / float(h), 0.1f, 200.0f);
    glViewport(0, 0, w, h);
}

void MyGLWidget::paintGL()
{
    // --- FPS counter ---
    m_frameCount++;
    qint64 elapsed = m_fpsTimer.elapsed();
    if (elapsed >= 500) {   // aggiorna ogni 0.5 s
        m_fps = float(m_frameCount) * 1000.0f / float(elapsed);
        m_frameCount = 0;
        m_fpsTimer.restart();
        // Mostra FPS nel titolo della finestra
        if (window())
            window()->setWindowTitle(QString("Relief Mapping  |  FPS: %1").arg(m_fps, 0, 'f', 1));
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    m_program->bind();

    // --- Camera ---
    // Zoom = distanza lungo Z, pan = traslazione XY
    QVector3D cameraPos(m_panOffset.x(), m_panOffset.y(), m_zoom);
    m_view.setToIdentity();
    m_view.lookAt(cameraPos,
                  QVector3D(m_panOffset.x(), m_panOffset.y(), 0.0f),
                  QVector3D(0, 1, 0));
    m_program->setUniformValue("uViewPos",  cameraPos);

    // --- Luce orbitante ---
    float rad = qDegreesToRadians(m_lightAngle);
    QVector3D lightPos(5.0f * qCos(rad), 4.0f, 5.0f * qSin(rad));
    m_program->setUniformValue("uLightPos", lightPos);

    // --- Matrici ---
    m_program->setUniformValue("uProjection", m_projection);
    m_program->setUniformValue("uView",       m_view);

    m_model.setToIdentity();
    m_model.scale(m_meshScale);                          // <-- scala mesh (slider)
    m_model.rotate(m_pitch, 1.0f, 0.0f, 0.0f);
    m_model.rotate(m_yaw,   0.0f, 1.0f, 0.0f);
    m_program->setUniformValue("uModel", m_model);

    // --- Uniforms shader ---
    m_program->setUniformValue("uDepthScale", m_depthScale);
    m_program->setUniformValue("uSearchMode", m_searchMode);

    m_heightMap->bind(0);
    m_program->setUniformValue("uHeightMap", 0);

    m_vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
    m_vao.release();
    m_program->release();
}

// ----------------------------------------------------------------
//  Slots slider
// ----------------------------------------------------------------
void MyGLWidget::setDepthScale(int value)
{
    // 0-100 → 0.0 - 0.30  (range più sicuro contro i "peli")
    m_depthScale = value / 333.0f;
    update();
}

void MyGLWidget::setMeshScale(int value)
{
    // 0-100 → 0.4 - 2.0
    m_meshScale = 0.4f + value * (1.6f / 100.0f);
    update();
}

void MyGLWidget::setSearchMode(int mode)
{
    m_searchMode = mode;   // 0 = solo linear, 1 = linear+binary
    update();
}

// ----------------------------------------------------------------
//  Mouse: left drag = ruota, right drag = pan, wheel = zoom
// ----------------------------------------------------------------
void MyGLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos   = event->pos();
    m_dragButton = event->button();
}

void MyGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->position().x() - m_lastPos.x();
    int dy = event->position().y() - m_lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        // Rotazione
        m_yaw   += dx * 0.5f;
        m_pitch += dy * 0.5f;
        m_pitch  = qBound(-89.0f, m_pitch, 89.0f);
        update();
    }
    else if (event->buttons() & Qt::RightButton) {
        // Traslazione (pan) nel piano XY — velocità proporzionale allo zoom
        float speed = m_zoom * 0.001f;
        m_panOffset.setX(m_panOffset.x() - dx * speed);
        m_panOffset.setY(m_panOffset.y() + dy * speed);
        update();
    }
    m_lastPos = event->pos();
}

void MyGLWidget::wheelEvent(QWheelEvent *event)
{
    // Zoom: avvicina/allontana la camera
    float delta = event->angleDelta().y() / 120.0f;   // un "click" = 1.0
    m_zoom -= delta * 0.4f;
    m_zoom  = qBound(1.5f, m_zoom, 30.0f);
    update();
}