#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QTimer>
#include <QTime>
#include <QtOpenGL>
#include <QOpenGLExtraFunctions>

#define NUM_STARS 5000

class OpenGLWidget : public QOpenGLWidget, public QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    OpenGLWidget(QWidget *parent = nullptr);

    void createModel();

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

private:
    unsigned int numVertices, numFaces;
    QVector4D *vertices;
    unsigned int *indices;
    QTimer *timer;
    QTime *time;

    QVector3D starPos[NUM_STARS];

    bool shaderOK, vboOK;
    QOpenGLShader *vertexShader, *fragmentShader;
    QOpenGLShaderProgram *shaderProgram;

    QOpenGLBuffer *vboVertices, *vboIndices;

    GLuint vao = 0;


    void createShaders();
    void destroyShaders();
    void createVBOs();
    void destroyVBOs();

public slots:
    void animate();
};

#endif // OPENGLWIDGET_H
