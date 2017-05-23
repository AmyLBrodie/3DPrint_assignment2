#include <GL/glew.h>
#include "glwidget.h"

#include <math.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <QGridLayout>
#include <QGLFramebufferObject>
#include <QImage>
#include <QCoreApplication>
#include <QMessageBox>

#include <fstream>

// include headers that implement a archive in simple text format
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/list.hpp>

using namespace std;

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

////
// GLWidget
////

GLWidget::GLWidget(const QGLFormat& format, QWidget *parent)
    : QGLWidget(format, parent)
{
    renderer = new Renderer(NULL, "../tesselate/shaders/");

    viewing = false;
    glewSetupDone = false;
    updateGeometry = true;
    meshVisible = false;

    scene.sampleScene();
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

GLWidget::~GLWidget()
{
    if (renderer) delete renderer;
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(600, 600);
}

void GLWidget::initializeGL()
{
    // get context opengl-version
    qDebug() << "Widget OpenGl: " << format().majorVersion() << "." << format().minorVersion();
    qDebug() << "Context valid: " << context()->isValid();
    qDebug() << "Really used OpenGl: " << context()->format().majorVersion() << "." <<
              context()->format().minorVersion();
    qDebug() << "OpenGl information: VENDOR:       " << (const char*)glGetString(GL_VENDOR);
    qDebug() << "                    RENDERDER:    " << (const char*)glGetString(GL_RENDERER);
    qDebug() << "                    VERSION:      " << (const char*)glGetString(GL_VERSION);
    qDebug() << "                    GLSL VERSION: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    QGLFormat glFormat = QGLWidget::format();
    if ( !glFormat.sampleBuffers() )
        qWarning() << "Could not enable sample buffers";

    QColor qtWhite = QColor::fromCmykF(0.0, 0.0, 0.0, 0.0);
    qglClearColor(qtWhite.light());

    int mu;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &mu);
    cerr << "max texture units = " << mu << endl;

    // set up light
    cgp::Vector dl = cgp::Vector(0.6f, 1.0f, 0.6f);
    dl.normalize();

    GLfloat pointLight[3] = { 0.5, 5.0, 7.0}; // side panel + BASIC lighting
    GLfloat dirLight0[3] = { dl.i, dl.j, dl.k}; // for radiance lighting
    GLfloat dirLight1[3] = { -dl.i, dl.j, -dl.k}; // for radiance lighting

    renderer->setPointLight(pointLight[0],pointLight[1],pointLight[2]);
    renderer->setDirectionalLight(0, dirLight0[0], dirLight0[1], dirLight0[2]);
    renderer->setDirectionalLight(1, dirLight1[0], dirLight1[1], dirLight1[2]);

    // initialise renderer/compile shaders
    renderer->initShaders();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_CLAMP);
    glEnable(GL_TEXTURE_2D);
}

void GLWidget::paintGL()
{
    ShapeDrawData sdd;

    glewExperimental = GL_TRUE;
    if(!glewSetupDone)
    {
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
            std::cerr<< "GLEW: initialization failed\n\n";
        }
        glewSetupDone = true;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(updateGeometry)
    {
        drawParams.clear();

        if(meshVisible)
        {
            if(scene.bindGeometry(getView(), sdd))
                drawParams.push_back(sdd);
        }
        updateGeometry = false;
    }

    // pass in draw params for geometry
    renderer->setDrawParams(drawParams);
    renderer->draw(getView());
}

void GLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);


    view.setDim((float) ((width - side) / 2), (float) ((height - side) / 2), (float) side, (float) side);
    view.apply();
}


void GLWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_A)
    {
        // stub for key based input
    }
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    float nx, ny;
    cgp::Point pnt;
    
    int x = event->x(); int y = event->y();
    float W = (float) width(); float H = (float) height();

    update(); // ensure this viewport is current for unproject

    // control view orientation with right mouse button or ctrl/alt modifier key and left mouse
    if((event->modifiers() == Qt::MetaModifier || event->modifiers() == Qt::AltModifier || event->buttons() == Qt::RightButton))
    {
        // arc rotate in perspective mode
  
        // convert to [0,1] X [0,1] domain
        nx = (2.0f * (float) x - W) / W;
        ny = (H - 2.0f * (float) y) / H;
        lastPos = event->pos();
        getView()->startArcRotate(nx, ny);
        viewing = true;
    }
    lastPos = event->pos();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    viewing = false;
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    float nx, ny, W, H;

    int x = event->x();
    int y = event->y();

    W = (float) width();
    H = (float) height();

    // control view orientation with right mouse button or ctrl modifier key and left mouse
    if(((event->modifiers() == Qt::MetaModifier && event->buttons() == Qt::LeftButton) || (event->modifiers() == Qt::AltModifier && event->buttons() == Qt::LeftButton) || event->buttons() == Qt::RightButton))
    {
        // convert to [0,1] X [0,1] domain
        nx = (2.0f * (float) x - W) / W;
        ny = (H - 2.0f * (float) y) / H;
        getView()->arcRotate(nx, ny);
        update();
        lastPos = event->pos();
    }
}

void GLWidget::wheelEvent(QWheelEvent * wheel)
{
    float del;
 
    QPoint pix = wheel->pixelDelta();
    QPoint deg = wheel->angleDelta();

    if(!pix.isNull()) // screen resolution tracking, e.g., from magic mouse
    {
        del = (float) pix.y();
        getView()->incrZoom(del);
        update();

    }
    else if(!deg.isNull()) // mouse wheel instead
    {
        del = (float) deg.y();
        getView()->incrZoom(del);
        update();
    }
}
