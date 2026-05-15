#include "myglwidget.h"
#include <QDebug>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>

MyGLWidget::MyGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);


    m_fpsTimer.start();
}

MyGLWidget::~MyGLWidget() {}

void MyGLWidget::buildCube()
{
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

    // ---------------------------------------------------------
    // CARICAMENTO IBO (Image-Based Object a 6 facce)
    // L'ordine DEVE riflettere quello di buildCube(): Front, Back, Right, Left, Top, Bottom
    // ---------------------------------------------------------
    QString faceNames[6] = {"Front", "Back", "Right", "Left", "Top", "Bottom"};

    for (int i = 0; i < 6; i++) {
        // Carica la mappa di profondità (Z-Buffer)
        QString depthPath = QString(":/depth_%1.png").arg(faceNames[i]);
        QImage dImg(depthPath);
        if (dImg.isNull()) qWarning() << "Errore: Impossibile caricare" << depthPath;

        m_depthMaps[i] = new QOpenGLTexture(dImg.flipped(Qt::Vertical));
        m_depthMaps[i]->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        m_depthMaps[i]->setMagnificationFilter(QOpenGLTexture::Linear);
        m_depthMaps[i]->setWrapMode(QOpenGLTexture::ClampToEdge); // FONDAMENTALE per sigillare i bordi
        m_depthMaps[i]->generateMipMaps();

        // Carica la mappa delle normali (Geometry)
        QString normalPath = QString(":/normal_%1.png").arg(faceNames[i]);
        QImage nImg(normalPath);
        if (nImg.isNull()) qWarning() << "Errore: Impossibile caricare" << normalPath;

        m_normalMaps[i] = new QOpenGLTexture(nImg.flipped(Qt::Vertical));
        m_normalMaps[i]->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        m_normalMaps[i]->setMagnificationFilter(QOpenGLTexture::Linear);
        m_normalMaps[i]->setWrapMode(QOpenGLTexture::ClampToEdge);
        m_normalMaps[i]->generateMipMaps();
    }

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
    // --- FPS counter nel titolo ---
    m_frameCount++;
    qint64 elapsed = m_fpsTimer.elapsed();
    if (elapsed >= 500) {
        m_fps = float(m_frameCount) * 1000.0f / float(elapsed);
        m_frameCount = 0;
        m_fpsTimer.restart();
        if (window())
            window()->setWindowTitle(
                QString("Relief Mapping  |  FPS: %1  |  Mode: %2")
                    .arg(m_fps, 0, 'f', 1)
                    .arg(m_searchMode == 0 ? "Linear only" : "Linear + Binary"));
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    m_program->bind();

    // 1. --- MATRICI E CAMERA ---
    QVector3D cameraPos(m_panOffset.x(), m_panOffset.y(), m_zoom);
    m_view.setToIdentity();
    m_view.lookAt(cameraPos,
                  QVector3D(m_panOffset.x(), m_panOffset.y(), 0.0f),
                  QVector3D(0, 1, 0));
    m_program->setUniformValue("uViewPos",  cameraPos);
    m_program->setUniformValue("uProjection", m_projection);
    m_program->setUniformValue("uView",       m_view);

    // 2. --- LUCE ---
    float rad = qDegreesToRadians(m_lightAngle);
    QVector3D lightPos(6.0f * qCos(rad), 5.0f, 6.0f * qSin(rad));
    m_program->setUniformValue("uLightPos", lightPos);

    // 3. --- MODELLO E ROTAZIONE (Mouse) ---
    m_model.setToIdentity();
    m_model.scale(m_meshScale);
    m_model.rotate(m_pitch, 1.0f, 0.0f, 0.0f);
    m_model.rotate(m_yaw,   0.0f, 1.0f, 0.0f);
    m_program->setUniformValue("uModel", m_model);

    // 4. --- UNIFORMS RELIEF MAPPING ---
    m_program->setUniformValue("uDepthScale", m_depthScale);
    m_program->setUniformValue("uSearchMode", m_searchMode);

    m_vao.bind();

    // 5. --- RENDERING FRAZIONATO DELL'IBO ---
    for (int i = 0; i < 6; i++) {
        // Controllo di sicurezza per evitare crash se la texture non è caricata
        if(m_depthMaps[i] && m_normalMaps[i]) {
            m_depthMaps[i]->bind(0);
            m_program->setUniformValue("uHeightMap", 0);

            m_normalMaps[i]->bind(1);
            m_program->setUniformValue("uDiffuseMap", 1);

            // Disegna 6 vertici partendo dall'offset (i * 6)
            glDrawArrays(GL_TRIANGLES, i * 6, 6);
        }
    }

    m_vao.release();
    m_program->release();
}

void MyGLWidget::setDepthScale(int value)
{
    // Il 100% dello slider ora corrisponde alla sincronizzazione perfetta di 0.625
    m_depthScale = (value / 100.0f) * 0.625f;
    update();
}

void MyGLWidget::setMeshScale(int value)
{
    m_meshScale = 0.4f + value * (1.6f / 100.0f);
    update();
}

void MyGLWidget::setSearchMode(int mode)
{
    m_searchMode = mode;
    update();
}

void MyGLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos    = event->pos();
    m_dragButton = event->button();
}

void MyGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->position().x() - m_lastPos.x();
    int dy = event->position().y() - m_lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        m_yaw   += dx * 0.5f;
        m_pitch += dy * 0.5f;
        m_pitch  = qBound(-89.0f, m_pitch, 89.0f);
        update();
    }
    else if (event->buttons() & Qt::RightButton) {
        float speed = m_zoom * 0.001f;
        m_panOffset.setX(m_panOffset.x() - dx * speed);
        m_panOffset.setY(m_panOffset.y() + dy * speed);
        update();
    }
    m_lastPos = event->pos();
}

void MyGLWidget::wheelEvent(QWheelEvent *event)
{
    float delta = event->angleDelta().y() / 120.0f;
    m_zoom -= delta * 0.4f;
    m_zoom  = qBound(1.5f, m_zoom, 30.0f);
    update();
}