#include <QFile>
#include <QTextStream>
#include <limits>

#include "openglwidget.h"

OpenGLWidget::OpenGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    numVertices = 0;
    vertices = nullptr;
    indices = nullptr;
    timer = new QTimer;
    time = new QTime;

    shaderOK = vboOK = false;
    vertexShader = fragmentShader = nullptr;
    shaderProgram = nullptr;
    vboVertices = vboIndices = nullptr;
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);

    createShaders();
    createVBOs();

    qDebug("OpenGL version: %s", glGetString(GL_VERSION));
    qDebug("GLSL %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    createModel();
    createShaders();
    createVBOs();

    for(int i=0; i<NUM_STARS; ++i)
    {
        QVector3D pos = starPos[i];

        float ang = (qrand() / (float)RAND_MAX) * 2 * 3.14159265f;
        float radius = 1 + (qrand() / (float)RAND_MAX) * 2;
        float x = cos(ang) * radius;
        float y = sin(ang) * radius;
        pos.setX(x);
        pos.setY(y);
        pos.setZ(((qrand() / (float)RAND_MAX) * 2.0f) - 1.0f);
        starPos[i] = pos;
    }

    glClearColor(0.0f, 0.0, 0.0f, 1);

    time->start();
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(animate()));

    glEnable(GL_DEPTH_TEST);
}

void OpenGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    if(!shaderOK || !vboOK)
        return;


    shaderProgram->bind();

    vboVertices->bind();
    shaderProgram->enableAttributeArray("vPosition");
    shaderProgram->setAttributeBuffer("vPosition", GL_FLOAT, 0, 4, 0);

    vboIndices->bind();
    for(int i=0; i<NUM_STARS; ++i)
    {
        shaderProgram->setUniformValue("translation", starPos[i].toVector4D());
        glDrawElements(GL_TRIANGLES, numFaces*3, GL_UNSIGNED_INT, nullptr);
    }

    vboIndices->release();
    vboVertices->release();
    shaderProgram->release();
}

void OpenGLWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
}

void OpenGLWidget::createModel()
{
    makeCurrent();
    numVertices = 4;
    numFaces = 2;

    // Increase maxSize so the model becomes very small in NDC
    float scale = 1 / 20.0f;

    // Create four vertices to define a square
    if(vertices)
        delete []vertices;
    vertices = new QVector4D[numVertices];
    vertices[0] = QVector4D(-0.5, -0.5, 0, 1) * scale;
    vertices[1] = QVector4D( 0.5, -0.5, 0, 1) * scale;
    vertices[2] = QVector4D( 0.5,  0.5, 0, 1) * scale;
    vertices[3] = QVector4D(-0.5,  0.5, 0, 1) * scale;

    // Topology of the mesh (square)
    if (indices)
        delete []indices;
    indices = new unsigned int[numFaces*3];
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 2;
    indices[4] = 3;
    indices[5] = 0;
}

void OpenGLWidget::animate()
{   
    makeCurrent();
    for(int i=0; i<NUM_STARS; ++i)
    {
        QVector3D pos = starPos[i];
        pos.setZ(pos.z() + (time->elapsed() / 50000.0f));
        if (pos.z() >= 1.0f)
        {
            float ang = (qrand() / (float)RAND_MAX) * 2 * 3.14159265f;
            float radius = 1 + (qrand() / (float)RAND_MAX) * 2;
            float x = cos(ang) * radius;
            float y = sin(ang) * radius;
            pos.setX(x);
            pos.setY(y);
            pos.setZ(-1.0f);
        }
        starPos[i] = pos;
    }
    time->restart();

    update();
}

void OpenGLWidget::createShaders()
{
    makeCurrent();
    shaderOK = false;

    destroyShaders();

    QString vShaderFileName = ":/shaders/vshader.glsl";
    QString fShaderFileName = ":/shaders/fshader.glsl";

    vertexShader = new QOpenGLShader(QOpenGLShader::Vertex);
    fragmentShader = new QOpenGLShader(QOpenGLShader::Fragment);

    if(!vertexShader->compileSourceFile(vShaderFileName))
    {
        qDebug() << vertexShader->log();
        return;
    }
    if(!fragmentShader->compileSourceFile(fShaderFileName))
    {
        qDebug() << fragmentShader->log();
        return;
    }

    shaderProgram = new QOpenGLShaderProgram();
    shaderProgram->addShader(vertexShader);
    shaderProgram->addShader(fragmentShader);
    if(!shaderProgram->link())
    {
        qDebug() << shaderProgram->log();
        return;
    }

    shaderOK = true;
}

void OpenGLWidget::destroyShaders()
{
    makeCurrent();
    if(shaderProgram)
    {
        shaderProgram->release();
        delete shaderProgram;
        shaderProgram = nullptr;
    }
    if(vertexShader)
    {
        delete vertexShader;
        vertexShader = nullptr;
    }
    if(fragmentShader)
    {
        delete fragmentShader;
        fragmentShader = nullptr;
    }
}

void OpenGLWidget::createVBOs()
{
    makeCurrent();
    vboOK = false;

    destroyVBOs();

    if(!vertices || !indices)
        return;

    vboVertices = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboVertices->create();
    vboVertices->bind();
    vboVertices->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboVertices->allocate(vertices, sizeof(QVector4D)*numVertices);
    vboVertices->release();

    vboIndices = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    vboIndices->create();
    vboIndices->bind();
    vboIndices->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vboIndices->allocate(indices, sizeof(unsigned int)*numFaces*3);
    vboIndices->release();

    vboOK = true;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
}

void OpenGLWidget::destroyVBOs()
{
    makeCurrent();
    if(vboVertices)
    {
        vboVertices->release();
        vboVertices->destroy();
        delete vboVertices;
        vboVertices = nullptr;
    }
    if(vboIndices)
    {
        vboIndices->release();
        vboIndices->destroy();
        delete vboIndices;
        vboIndices = nullptr;
    }
}
