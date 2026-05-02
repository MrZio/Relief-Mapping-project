#include "myglwidget.h"
#include <QDebug>
#include <QMouseEvent>

// Qui stiamo passando la variabile 'parent' direttamente al costruttore della classe base
MyGLWidget::MyGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
}

MyGLWidget::~MyGLWidget()
{
}

void MyGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex.glsl");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fragment.glsl");
    m_program->link();

    unsigned char data[4] = {0, 255, 255, 0};

    m_heightMap = new QOpenGLTexture(QOpenGLTexture::Target2D);    m_heightMap->setSize(2, 2);
    m_heightMap->setFormat(QOpenGLTexture::R8_UNorm);
    m_heightMap->allocateStorage();
    m_heightMap->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, data);


    m_heightMap->setMinificationFilter(QOpenGLTexture::Nearest);
    m_heightMap->setMagnificationFilter(QOpenGLTexture::Nearest);
    m_heightMap->setWrapMode(QOpenGLTexture::Repeat);

    qDebug() << "Texture di test creata con successo!";


    VertexData vertices[] = {
        // FACCIA FRONTALE (Z = 1) | Normale: (0,0,1) | Tangente: (1,0,0)
        {QVector3D(-1.0f, -1.0f,  1.0f), QVector2D(0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f)},
        {QVector3D( 1.0f, -1.0f,  1.0f), QVector2D(1.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f)},
        {QVector3D( 1.0f,  1.0f,  1.0f), QVector2D(1.0f, 1.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f)},
        {QVector3D(-1.0f, -1.0f,  1.0f), QVector2D(0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f)},
        {QVector3D( 1.0f,  1.0f,  1.0f), QVector2D(1.0f, 1.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f)},
        {QVector3D(-1.0f,  1.0f,  1.0f), QVector2D(0.0f, 1.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f)},

        // FACCIA DESTRA (X = 1) | Normale: (1,0,0) | Tangente: (0,0,-1)
        {QVector3D( 1.0f, -1.0f,  1.0f), QVector2D(0.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f), QVector3D(1.0f, 0.0f, 0.0f)},
        {QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(1.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f), QVector3D(1.0f, 0.0f, 0.0f)},
        {QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(1.0f, 1.0f), QVector3D(0.0f, 0.0f, -1.0f), QVector3D(1.0f, 0.0f, 0.0f)},
        {QVector3D( 1.0f, -1.0f,  1.0f), QVector2D(0.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f), QVector3D(1.0f, 0.0f, 0.0f)},
        {QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(1.0f, 1.0f), QVector3D(0.0f, 0.0f, -1.0f), QVector3D(1.0f, 0.0f, 0.0f)},
        {QVector3D( 1.0f,  1.0f,  1.0f), QVector2D(0.0f, 1.0f), QVector3D(0.0f, 0.0f, -1.0f), QVector3D(1.0f, 0.0f, 0.0f)},

        // FACCIA POSTERIORE (Z = -1) | Normale: (0,0,-1) | Tangente: (-1,0,0)
        {QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(0.0f, 0.0f), QVector3D(-1.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f)},
        {QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(1.0f, 0.0f), QVector3D(-1.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f)},
        {QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(1.0f, 1.0f), QVector3D(-1.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f)},
        {QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(0.0f, 0.0f), QVector3D(-1.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f)},
        {QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(1.0f, 1.0f), QVector3D(-1.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f)},
        {QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(0.0f, 1.0f), QVector3D(-1.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f)},

        // FACCIA SINISTRA (X = -1) | Normale: (-1,0,0) | Tangente: (0,0,1)
        {QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(0.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f), QVector3D(-1.0f, 0.0f, 0.0f)},
        {QVector3D(-1.0f, -1.0f,  1.0f), QVector2D(1.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f), QVector3D(-1.0f, 0.0f, 0.0f)},
        {QVector3D(-1.0f,  1.0f,  1.0f), QVector2D(1.0f, 1.0f), QVector3D(0.0f, 0.0f, 1.0f), QVector3D(-1.0f, 0.0f, 0.0f)},
        {QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(0.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f), QVector3D(-1.0f, 0.0f, 0.0f)},
        {QVector3D(-1.0f,  1.0f,  1.0f), QVector2D(1.0f, 1.0f), QVector3D(0.0f, 0.0f, 1.0f), QVector3D(-1.0f, 0.0f, 0.0f)},
        {QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(0.0f, 1.0f), QVector3D(0.0f, 0.0f, 1.0f), QVector3D(-1.0f, 0.0f, 0.0f)},

        // FACCIA SUPERIORE (Y = 1) | Normale: (0,1,0) | Tangente: (1,0,0)
        {QVector3D(-1.0f,  1.0f,  1.0f), QVector2D(0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f)},
        {QVector3D( 1.0f,  1.0f,  1.0f), QVector2D(1.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f)},
        {QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(1.0f, 1.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f)},
        {QVector3D(-1.0f,  1.0f,  1.0f), QVector2D(0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f)},
        {QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(1.0f, 1.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f)},
        {QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(0.0f, 1.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f)},

        // FACCIA INFERIORE (Y = -1) | Normale: (0,-1,0) | Tangente: (1,0,0)
        {QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f)},
        {QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(1.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f)},
        {QVector3D( 1.0f, -1.0f,  1.0f), QVector2D(1.0f, 1.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f)},
        {QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f)},
        {QVector3D( 1.0f, -1.0f,  1.0f), QVector2D(1.0f, 1.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f)},
        {QVector3D(-1.0f, -1.0f,  1.0f), QVector2D(0.0f, 1.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f)}
    };

    m_vao.create();
    m_vao.bind();

    // CREIAMO E LEGIAMO IL VBO
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(vertices, sizeof(vertices));

    // Ora gli attributi sanno che devono leggere da m_vbo perché è legato
    int posLocation = m_program->attributeLocation("aPos");
    m_program->enableAttributeArray(posLocation);
    m_program->setAttributeBuffer(posLocation, GL_FLOAT, offsetof(VertexData, position), 3, sizeof(VertexData));

    int texLocation = m_program->attributeLocation("aTexCoord");
    m_program->enableAttributeArray(texLocation);
    m_program->setAttributeBuffer(texLocation, GL_FLOAT, offsetof(VertexData, texCoord), 2, sizeof(VertexData));

    int tangentLocation = m_program->attributeLocation("aTangent");
    m_program->enableAttributeArray(tangentLocation);
    m_program->setAttributeBuffer(tangentLocation, GL_FLOAT, offsetof(VertexData, tangent), 3, sizeof(VertexData));

    // ATTRIBUTO 3: Normale (3 float)
    int normalLocation = m_program->attributeLocation("aNormal");
    m_program->enableAttributeArray(normalLocation);
    m_program->setAttributeBuffer(normalLocation, GL_FLOAT, offsetof(VertexData, normal), 3, sizeof(VertexData));

    m_vao.release();
    m_vbo.release(); // Buona pratica rilasciare
}

void MyGLWidget::resizeGL(int w, int h)
{
    m_projection.setToIdentity();
    m_projection.perspective(45.0f, GLfloat(w) / h, 0.7f, 100.0f);
    glViewport(0, 0, w, h);
}

void MyGLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Diciamo a OpenGL di non disegnare l'interno del solido
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    m_program->bind();

    // --- CORREZIONE TELECAMERA ---
    QVector3D cameraPos(0.0f, 0.0f, 7.0f); // Spostiamola a 7 per vedere bene tutto

    m_view.setToIdentity();
    m_view.lookAt(cameraPos,           // Posizione occhio
                  QVector3D(0, 0, 0),  // Punto guardato (centro del cubo)
                  QVector3D(0, 1, 0)); // Vettore "Up" (l'asse Y punta in alto)

    m_program->setUniformValue("uViewPos", cameraPos);
    // Ora lo shader e OpenGL concordano su dove sia la telecamera
    // -----------------------------

    m_heightMap->bind(0);
    m_program->setUniformValue("uHeightMap", 0);

    m_program->setUniformValue("uProjection", m_projection);
    m_program->setUniformValue("uView", m_view);
    m_model.setToIdentity();
    m_model.rotate(m_pitch, 1.0f, 0.0f, 0.0f); // 1. Inclina su/giù
    m_model.rotate(m_yaw, 0.0f, 1.0f, 0.0f);   // 2. Gira a destra/sinistra

    m_program->setUniformValue("uModel", m_model);
    m_program->setUniformValue("uDepthScale", m_depthScale);

    m_vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
    m_vao.release();
    m_program->release();
}
void MyGLWidget::setDepthScale(int value)
{
    // Convertiamo l'intero (es. 0-100) in un float (es. 0.0 - 1.0)
    m_depthScale = value / 100.0f;

    // FONDAMENTALE: chiediamo a Qt di ridisegnare la scena.
    // Senza update(), il valore cambierebbe ma il triangolo resterebbe fermo
    // finché non ridimensioni la finestra a mano.
    update();

    qDebug() << "Nuova profondità impostata:" << m_depthScale;
}

void MyGLWidget::mousePressEvent(QMouseEvent *event) {
    m_lastPos = event->pos();
}

void MyGLWidget::mouseMoveEvent(QMouseEvent *event) {
    int dx = event->position().x() - m_lastPos.x();
    int dy = event->position().y() - m_lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        float sensibilita = 0.5f;

        // Sommiamo o sottraiamo agli angoli ASSOLUTI
        m_yaw += dx * sensibilita;
        m_pitch += dy * sensibilita;

        // BLOCCO ANTIRIBALTAMENTO (Impedisce il Gimbal Lock estremo)
        if(m_pitch > 89.0f) m_pitch = 89.0f;
        if(m_pitch < -89.0f) m_pitch = -89.0f;

        update();
    }
    m_lastPos = event->pos();
}