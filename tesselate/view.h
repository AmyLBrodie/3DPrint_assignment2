#ifndef _INC_VIEW
#define _INC_VIEW
/**
 * @file
 *
 * Controlling viewpoint changes.
*/



#include "vecpnt.h"
#include "timer.h"
#include <common/debug_vector.h>

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <glm/glm.hpp>

using namespace std;

enum viewType {perspective, frontOrtho, upOrtho, leftOrtho};

// #define FIGURE

#define VIEW_WIDTH 1024
#define VIEW_HEIGHT 768

#define RAD2DEG 57.2957795f
#define DEG2RAD 0.0174532925f
#define ACTUAL_ASPECT 1.4
#define spinsteps 481.0f

void trackball(float q[4], float p1x, float p1y, float p2x, float p2y);
void axis_to_quat(float a[3], float phi, float q[4]);

/// Information structure for view control
class View
{
private:
    cgp::Point cop;                 ///< centre of projection
    cgp::Point light;               ///< light position for default diffuse rendering
    cgp::Vector dir;                ///< forward camera direction for current view
    cgp::Point focus, prevfocus, currfocus; ///< controls for transition of focal point
    int focalstep;                  ///< linear interpolation value for focal transition
    float zoomdist;                 ///< distance to origin
    float perspwidth;               ///< back plane width of perspective projection
    float bu, bv;                   ///< arcball position parameters in normalized coordinate image plane
    float lastquat[4], curquat[4];  ///< quaternions for arcball
    float viewscale;                ///< scale adaptation to terrain size

    /// Recalculate viewing direction and up vector
    void updateDir();

public:

    float width, height;        ///< viewport dimensions
    float startx, starty;       ///< bottom left corner of viewport

    /// default constructor
    View(){ reset(); }

    /// constructor with a viewing scale set to extent
    View(float extent){ reset(extent); }

    /// destructor
    ~View(){}

    /// Set view parameters, with scale factor as extent
    void reset(float extent = 10000.0f)
    {
        perspwidth = 6.0f * tanf(0.5f * DEG2RAD * 45.0);
        resetLight();
        setDim(0.0f, 0.0f, 600.0f, 600.0f);
        focus = cgp::Point(0.0f, 0.0f, 0.0f);
        prevfocus = currfocus = focus;
        focalstep = 0;
        viewscale = extent;
        
        zoomdist = 60.0f * viewscale;

        bu = 0.0f;
        bv = 0.0f;
        trackball(curquat, 0.0f, 0.0f, 0.0f, -0.5f);
        cop = cgp::Point(0.0f, 0.0f, 1.0f);
        updateDir();
    }

    /// Return the center of projection of the view, recalculated as necessary
    inline cgp::Point getCOP(){ return cop; }

    /// Set the center of rotation to @a pnt, enabling an animated shift in the focal point
    void setAnimFocus(cgp::Point pnt);
    
    /// Set the center of rotation to @a pnt, with immediate non-animated transition
    void setForcedFocus(cgp::Point pnt);
    
    /// Return the center of rotation
    inline cgp::Point getFocus(){ return currfocus; }

    /// Start spinning the viewpoint
    void startSpin();

    /// Set the terrain scaling factor
    inline void setViewScale(float extent)
    {
        zoomdist = zoomdist / viewscale * extent; // adjust zoomdist
        viewscale = extent;
    }
    
    /// Return the view vector from the center of projection to the focal point
    inline cgp::Vector getDir()
    {
        updateDir();
        return dir;
    }

    /// Set viewport dimensions @a width and @a height
    inline void setDim(float ox, float oy, float w, float h){ startx = ox, starty = oy; width = w; height = h;}
    
    /// Increment the current zoom distance by @a delta
    inline void incrZoom(float delta)
    {
        zoomdist += (delta * viewscale / 100.0f);
        if(zoomdist < 0.5f * viewscale)
            zoomdist = 0.5f * viewscale;
        if(zoomdist > 100.0f * viewscale)
            zoomdist = 100.0f * viewscale;
        apply();
    }

    /// Get the current zoom distance
    inline float getZoom()
    {
        return zoomdist;
    }

    /// Reset light position to default
    inline void resetLight(){ light = cgp::Point(0.0f, 0.5f, 1.0f); };

    /// Set light position
    inline void setLight(cgp::Point p){ light = p; }

    /// Begin rotation on button down
    void startArcRotate(float u, float v);

    /// Rotate view according to current normalized windows coordinates @a u, @a v
    void arcRotate(float u, float v);

    /// Find the ray starting at the viewing and intersecting the screen coordinates @a sx, @a sy.
    //                 Return the start position and direction vector @a dirn of the ray
    void projectingRay(int sx, int sy, cgp::Point & start, cgp::Vector & dirn);

    /**
     * find the world coordinate position corresponding to screen space coordinates.
     * @param sx, sy    Screen space coordinates
     * @param[out] pnt  Screen space point but now in world coordinates
     */
    void projectingPoint(int sx, int sy, cgp::Point & pnt);

    /**
     * find the screen coordinate position corresponding to mouse input.
     * @param sx, sy    Screen space coordinates
     * @param[out] pnt  mouse point but now in screen coordinates
     */
    void inscreenPoint(int sx, int sy, cgp::Point & pnt);
    
    /**
     * project a the pick intersection point to the closest point on the manipulator line
     * @pre mdirn is a unit vector
     * @param pick      Pick intersection point with manipulator plane
     * @param mpnt      Point on manipulator arm
     * @param mdirn     Direction of manipulator arm
     * @param[out] mpick    Pick point projected onto manipulator arm
     */
    void projectOntoManip(cgp::Point pick, cgp::Point mpnt, cgp::Vector mdirn, cgp::Point & mpick);
    
    /// Find a position change vector in world coordinates @a del given a change
    ///              in screen coordinates from old position @a ox, @a oy to new position @a nx, @a ny
    void projectMove(int ox, int oy, int nx, int ny, cgp::Point cp, cgp::Vector & del);
    
    /// Retrieve composite model-view transformation
    glm::mat4x4 getMatrix();

    /// Retrieve the glm projection matrix
    glm::mat4x4 getProjMtx();

    /// Retrieve glm view transformation matrix
    glm::mat4x4 getViewMtx();

    /// Retrieve inverse transpose of the view transform
    glm::mat3x3 getNormalMtx();

    /// Provide a scaling factor for manipulators, which depends on zoom
    float getScaleFactor();

    /// Provide a scaling factor for brush ring, which depends on terrain scale but not zoom
    float getScaleConst();

    /// Set the current OpenGL viewing state to accord with the stored viewing parameters
    ///          Note - this does not clear the current view.
    void apply();
    
    /// Shift the focal point gradually over a number of frames. Returns true if their is an animated change in focus
    bool animate();

    /// Rotate the view around the focal point over a number of frames. Returns true if spin is active
    bool spin();
    
    /// Save the current view to the file named @a filename
    ///              return true if operation is successful, otherwise false
    bool save(const char * filename);
    
    /// Load the current view from the file named @a filename
    ///              return true if operation is successful, otherwise false
    bool load(const char * filename);

    /// print view stats to cerr
    void print();
};
# endif // _INC_VIEW
