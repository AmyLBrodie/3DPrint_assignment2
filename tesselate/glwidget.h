// GLWidget class for managing rendering
// Originally built from Qt Toolkit example
// (c) James Gain, 2014

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "glheaders.h" // Must be included before QT opengl headers
#include <QGLWidget>
#include <QTimer>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPushButton>
#include <list>
#include <common/debug_vector.h>
#include <common/debug_list.h>

#include "view.h"
#include "csg.h"
#include "renderer.h"

//! [0]
using namespace std;

class Window;

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:

    /// constructor
    GLWidget(const QGLFormat& format, QWidget *parent = 0);

    /// destructor
    ~GLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    /// getter for currently active view
    View * getView(){ return &view; }

    /// getter for renderer
    Renderer * getRenderer(){ return renderer; }

    /// getter for scene
    Scene * getScene(){ return &scene; }

    /// setter for geometry updating
    void setGeometryUpdate(bool update){ updateGeometry = update; }

    /// setter for drawing intersection mesh
    void setMeshVisible(bool vis){ meshVisible = vis; setGeometryUpdate(true); }

    /// respond to key press events
    void keyPressEvent(QKeyEvent *event);

signals:

    /// signal that the OpenGL canvas should be repainted
    void signalRepaintAllGL();

protected:
    /// Setup OpenGL state
    void initializeGL();

    /// OpenGL draw call
    void paintGL();

    /// Resize OpenGL canvas
    void resizeGL(int width, int height);

    /// Handle mouse button press
    void mousePressEvent(QMouseEvent *event);

    /// Handle mouse button release
    void mouseReleaseEvent(QMouseEvent *event);

    /// Handle mouse movement
    void mouseMoveEvent(QMouseEvent *event);

    /// Handle mouse wheel scrolling
    void wheelEvent(QWheelEvent * wheel);

private:

    // scene control
    Scene scene;                        ///< scene represented as CSG tree
    View view;                          ///< current viewpoint
    vector<ShapeDrawData> drawParams;   ///< OpenGL drawing parameters
    bool updateGeometry;                ///< recreate render buffers on change
    bool meshVisible;                   ///< render intersection mesh

    // render variables
    Renderer * renderer;                ///< OpenGL renderer

    // gui variables
    bool viewing;                       ///< is the user adjusting the viewing direction?
    bool glewSetupDone;                 ///< is OpenGL initialisation finished

    QPoint lastPos;                     ///< previous mouse position in 2D
};

#endif
