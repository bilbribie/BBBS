//|___________________________________________________________________
//!
//! \file plane1_base.cpp
//!
//! \brief Base source code for the first plane assignment.
//!
//! Author: Mores Prachyabrued.
//!
//! Keyboard controls:
//!   s   = moves the plane forward
//!   f   = moves the plane backward
//!   q,e = rolls the plane
//!
//!   k   = moves the camera forward
//!   ;   = moves the camera backward
//!
//! TODO: Extend the code to satisfy the requirements given in the assignment handout
//!
//! Note: Good programmer uses good comments! :)
//|___________________________________________________________________

//|___________________
//|
//| Includes
//|___________________

#include <math.h>

#include <gmtl/gmtl.h>

#include <GL/glut.h>

//|___________________
//|
//| Constants
//|___________________

// Plane dimensions
const float P_WIDTH = 3;
const float P_LENGTH = 3;
const float P_HEIGHT = 1.5;

// Camera's view frustum 
const float CAM_FOV = 60.0f;     // Field of view in degs

//|___________________
//|
//| Global Variables
//|___________________

// Track window dimensions, initialized to 800x600
int w_width = 800;
int w_height = 600;

// Plane pose (position & orientation)
gmtl::Matrix44f plane_pose; // T, as defined in the handout, initialized to IDENTITY by default

// Camera pose
gmtl::Matrix44f cam_pose;   // C, as defined in the handout
gmtl::Matrix44f view_mat;   // View transform is C^-1 (inverse of the camera transform C)

// Top
gmtl::Matrix44f cam_pose_fixed;   // F, as defined in the handout 0_0
gmtl::Matrix44f view_mat_fixed;   // View transform is F^-1 (inverse of the camera transform F)

// Transformation matrices applied to plane and camera poses
gmtl::Matrix44f ztransp_mat;
gmtl::Matrix44f ztransn_mat;
gmtl::Matrix44f zrotp_mat;
gmtl::Matrix44f zrotn_mat;
gmtl::Matrix44f xrotp_mat;
gmtl::Matrix44f xrotn_mat;
gmtl::Matrix44f yrotp_mat;
gmtl::Matrix44f yrotn_mat;

//|___________________
//|
//| Function Prototypes
//|___________________

void InitMatrices();
void InitGL(void);
void DisplayFunc(void);
void KeyboardFunc(unsigned char key, int x, int y);
void ReshapeFunc(int w, int h);
void DrawCoordinateFrame(const float l);
void DrawPlane(float wingWidth, float wingLength, float bodyWidth, float bodyLength, float bodyHeight);

//|____________________________________________________________________
//|
//| Function: InitMatrices
//|
//! \param None.
//! \return None.
//!
//! Initializes all the matrices
//|____________________________________________________________________

void InitMatrices()
{
    const float TRANS_AMOUNT = 1.0f;
    const float ROT_AMOUNT = gmtl::Math::deg2Rad(5.0f); // specified in degs, but get converted to radians

    const float COSTHETA = cos(ROT_AMOUNT);
    const float SINTHETA = sin(ROT_AMOUNT);

    // Positive Z-Translation
    ztransp_mat.set(1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, TRANS_AMOUNT,
        0, 0, 0, 1);
    ztransp_mat.setState(gmtl::Matrix44f::TRANS);

    gmtl::invert(ztransn_mat, ztransp_mat);

    // Positive Z-rotation (roll)
    zrotp_mat.set(COSTHETA, -SINTHETA, 0, 0,
        SINTHETA, COSTHETA, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1);
    zrotp_mat.setState(gmtl::Matrix44f::ORTHOGONAL);

    // Negative Z-rotation (roll)
    gmtl::invert(zrotn_mat, zrotp_mat);

    // Positive X-rotation (pitch)
    xrotp_mat.set(1, 0, 0, 0,
        0, COSTHETA, -SINTHETA, 0,
        0, SINTHETA, COSTHETA, 0,
        0, 0, 0, 1);
    xrotp_mat.setState(gmtl::Matrix44f::ORTHOGONAL);

    // Negative X-rotation (pitch)
    gmtl::invert(xrotn_mat, xrotp_mat);

    // Positive Y-rotation (yaw)
    yrotp_mat.set(
        COSTHETA, 0, -SINTHETA, 0,
        0, 1, 0, 0,
        SINTHETA, 0, COSTHETA, 0,
        0, 0, 0, 1);
    yrotp_mat.setState(gmtl::Matrix44f::ORTHOGONAL);

    // Negative Y-rotation (yaw)
    gmtl::invert(yrotn_mat, yrotp_mat);

    // Inits plane pose
    plane_pose.set(1, 0, 0, 1.0f,
        0, 1, 0, 0.0f,
        0, 0, 1, 4.0f,
        0, 0, 0, 1.0f);
    plane_pose.setState(gmtl::Matrix44f::AFFINE);     // AFFINE because the plane pose can contain both translation and rotation         

    // Inits camera pose and view transform
    cam_pose.set(1, 0, 0, 2.0f,
        0, 1, 0, 1.0f,
        0, 0, 1, 15.0f,
        0, 0, 0, 1.0f);
    cam_pose.setState(gmtl::Matrix44f::AFFINE);
    gmtl::invert(view_mat, cam_pose);                 // View transform is the inverse of the camera posex  

    // precalculated it to help with performance
    cam_pose_fixed.set(
        1, 0, 0, 0.0f,
        0, 0, 1, 40.0f,
        0, -1, 0, 0.0f,
        0, 0, 0, 1.0f
    );
    gmtl::invert(view_mat_fixed, cam_pose_fixed);  // View transform is the inverse of the camera pose
}

//|____________________________________________________________________
//|
//| Function: InitGL
//|
//! \param None.
//! \return None.
//!
//! OpenGL initializations
//|____________________________________________________________________

void InitGL(void)
{
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
}

//|____________________________________________________________________
//|
//| Function: DisplayFunc
//|
//! \param None.
//! \return None.
//!
//! GLUT display callback function: called for every redraw event.
//|____________________________________________________________________

void DisplayFunc(void)
{
    // Modelview matrix
    gmtl::Matrix44f modelview_mat;        // M, as defined in the handout

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //|____________________________________________________________________
    //|
    //| Viewport 1 rendering: shows the moving camera's view
    //|____________________________________________________________________

    glViewport(0, 0, (GLsizei)w_width / 2, (GLsizei)w_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(CAM_FOV, (float)w_width / (2 * w_height), 0.1f, 100.0f);     // Check MSDN: google "gluPerspective msdn"

    // Approach1
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();                          // A good practice for beginner

    // Draws world coordinate frame
    modelview_mat = view_mat;                  // M = C^-1
    glLoadMatrixf(modelview_mat.mData);
    DrawCoordinateFrame(10);

    // Draws plane and its local frame
    modelview_mat *= plane_pose;               // M = C^-1 * T
    glLoadMatrixf(modelview_mat.mData);
    DrawPlane(9.0f, 6.0f, 2.0f, 4.0f, 3.0f);//void DrawPlane(float wingWidth, float wingLength, float bodyWidth, float bodyLength, float bodyHeight);
    DrawCoordinateFrame(3);

    /*
      // Approach 2 (gives the same results as the approach 1)
      glMatrixMode(GL_MODELVIEW);

      // Draws world coordinate frame
      glLoadMatrixf(view_mat.mData);             // M = C^-1
      DrawCoordinateFrame(10);

      // Draws plane and its local frame
      glMultMatrixf(plane_pose.mData);           // M = C^-1 * T (OpenGL calls build transforms in left-to-right order)
      DrawPlane(P_WIDTH, P_LENGTH, P_HEIGHT);
      DrawCoordinateFrame(3);
    */

    //|____________________________________________________________________
    //|
    //| TODO: Viewport 2 rendering: shows the fixed top-down view
    //|____________________________________________________________________


    glViewport((GLsizei)w_width / 2, 0, (GLsizei)w_width / 2, (GLsizei)w_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(CAM_FOV, (float)w_width / (2 * w_height), 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();                          // A good practice for beginner

    // Draws world coordinate frame
    modelview_mat = view_mat_fixed;                  // M = C^-1
    glLoadMatrixf(modelview_mat.mData);
    DrawCoordinateFrame(10);

    // Draws plane and its local frame
    modelview_mat *= plane_pose;               // M = C^-1 * T
    glLoadMatrixf(modelview_mat.mData);
    DrawPlane(9.0f, 6.0f, 2.0f, 4.0f, 3.0f);//void DrawPlane(float wingWidth, float wingLength, float bodyWidth, float bodyLength, float bodyHeight);
    DrawCoordinateFrame(3);

    // Draws the camera
    modelview_mat = view_mat_fixed * cam_pose;
    glLoadMatrixf(modelview_mat.mData);
    DrawCoordinateFrame(3);

    // glMatrixMode(GL_MODELVIEW);
    // glLoadIdentity(); 
    // ...

    glFlush();
}

//|____________________________________________________________________
//|
//| Function: KeyboardFunc
//|
//! \param None.
//! \return None.
//!
//! GLUT keyboard callback function: called for every key press event.
//|____________________________________________________________________

void KeyboardFunc(unsigned char key, int x, int y)
{
    switch (key) {
        //|____________________________________________________________________
        //|
        //| Plane controls
        //|____________________________________________________________________

    case 's': // Forward translation of the plane (positive Z-translation)
        plane_pose = plane_pose * ztransp_mat; // 0_0
        break;
    case 'f': // Backward translation of the plane
        plane_pose = plane_pose * ztransn_mat;
        break;

        // TODO: Add the remaining controls/transforms        
    case 'e': // Rolls the plane (+ Z-rot)
        plane_pose = plane_pose * zrotp_mat;
        break;
    case 'q': // Rolls the plane (- Z-rot)
        plane_pose = plane_pose * zrotn_mat;
        break;

    case 'd': // Yaws the plane (+ Y-rot)
        plane_pose = plane_pose * yrotp_mat;
        break;
    case 'a': // Yaws the plane (- Y-rot)
        plane_pose = plane_pose * yrotn_mat;
        break;

    case 'x': // Pitches the plane (+ X-rot)
        plane_pose = plane_pose * xrotp_mat;
        break;
    case 'w': // Pitches the plane (+ X-rot)
        plane_pose = plane_pose * xrotn_mat;
        break;

        //|____________________________________________________________________
        //|
        //| Camera controls
        //|____________________________________________________________________

    case 'k': // Forward translation of the camera (negative Z-translation - cameras looks in its (local) -Z direction)
        cam_pose = cam_pose * ztransn_mat;
        break;
    case ';': // Backward translation of the camera
        cam_pose = cam_pose * ztransp_mat;
        break;

        // TODO: Add the remaining controls
    case 'o': // Rolls the camara (+ Z-rot)
        cam_pose = cam_pose * zrotp_mat;
        break;
    case 'u': // Rolls the camara (- Z-rot)
        cam_pose = cam_pose * zrotp_mat;
        break;

    case 'l': // Yaws the camara (+ Y-rot)
        cam_pose = cam_pose * yrotp_mat;
        break;
    case 'j': // Yaws the camara (- Y-rot)
        cam_pose = cam_pose * yrotp_mat;
        break;

    case ',': // Pitches the camara (+ X-rot)
        cam_pose = cam_pose * xrotp_mat;
        break;
    case 'i': // Pitchs the camara (- X-rot)
        cam_pose = cam_pose * xrotp_mat;
        break;
    }

    gmtl::invert(view_mat, cam_pose);       // Updates view transform to reflect the change in camera transform
    glutPostRedisplay();                    // Asks GLUT to redraw the screen
}

//|____________________________________________________________________
//|
//| Function: ReshapeFunc
//|
//! \param None.
//! \return None.
//!
//! GLUT reshape callback function: called everytime the window is resized.
//|____________________________________________________________________

void ReshapeFunc(int w, int h)
{
    // Track the current window dimensions
    w_width = w;
    w_height = h;
}

//|____________________________________________________________________
//|
//| Function: DrawCoordinateFrame
//|
//! \param l      [in] length of the three axes.
//! \return None.
//!
//! Draws coordinate frame consisting of the three principal axes.
//|____________________________________________________________________

void DrawCoordinateFrame(const float l)
{
    glBegin(GL_LINES);
    // X axis is red
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(l, 0.0f, 0.0f);

    // Y axis is green
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, l, 0.0f);

    // Z axis is blue
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, l);
    glEnd();
}

//|____________________________________________________________________
//|
//| Function: DrawPlane
//|
//! \param width       [in] Width  of the plane.
//! \param length      [in] Length of the plane.
//! \param height      [in] Height of the plane.
//! \return None.
//!
//! Draws the plane.
//|____________________________________________________________________

void DrawPlane(float wingWidth, float wingLength, float bodyWidth, float bodyLength, float bodyHeight) {

    // Set the Z-offset for the entire tail section to move it behind the body
    float tailZOffset = -bodyLength; // This will place the tail starting from the back of the airplane body

    float halfBodyWidth = bodyWidth / 2.0f;
    float halfWingWidth = wingWidth / 2.0f;
    float wingBodyIntersectionLength = bodyLength * 0.75f; // Adjust this value to control where the wing blends into the body

    // Start drawing
    glBegin(GL_TRIANGLES);

    // Set color for the wings
    glColor3f(1.0f, 1.0f, 1.0f); // White for the wing

    // Assume bodyLength is the length from the center to the front tip of the plane
    float frontTipX = 0.0f; // X coordinate for the front tip of the plane
    float frontTipY = 0.0f; // Y coordinate for the front tip of the plane
    float frontTipZ = -bodyLength; // Z coordinate for the front tip of the plane (assuming the front tip is along the negative Z-axis)

    // Left Wing - modified to connect smoothly
    glVertex3f(-halfBodyWidth, 0.0f, wingBodyIntersectionLength); // Inner point of the wing close to the body's front
    glVertex3f(-halfWingWidth, 0.0f, wingLength); // Wing tip
    glVertex3f(-halfBodyWidth, 0.0f, 0.0f); // Adjusted to make a smooth connection

    // Add vertices to connect the left wing to the front tip
    glVertex3f(-halfBodyWidth, 0.0f, 0.0f); // Back inner corner of the left wing
    glVertex3f(-halfWingWidth, 0.0f, wingLength); // Left wing tip
    glVertex3f(frontTipX, frontTipY, frontTipZ); // Front tip of the plane


    // Right Wing - modified to connect smoothly
    glVertex3f(halfBodyWidth, 0.0f, wingBodyIntersectionLength); // Inner point of the wing close to the body's front
    glVertex3f(halfWingWidth, 0.0f, wingLength); // Wing tip
    glVertex3f(halfBodyWidth, 0.0f, 0.0f); // Adjusted to make a smooth connection

    glVertex3f(halfBodyWidth, 0.0f, 0.0f); // Back inner corner of the right wing
    glVertex3f(halfWingWidth, 0.0f, wingLength); // Right wing tip
    glVertex3f(frontTipX, frontTipY, frontTipZ); // Front tip of the plane



    // Connecting slope to wings and front tip with a gradient
// Assume darker color at the base and lighter at the top
    float baseColor = 0.6f; // Dark shade for the base of the slope
    float topColor = 0.9f; // Lighter shade for the top of the slope

    // Slope - Left side
    glColor3f(baseColor, baseColor, baseColor);
    glVertex3f(-halfBodyWidth, 0.0f, 0.0f); // Base of the slope
    glVertex3f(-halfBodyWidth, 0.0f, wingBodyIntersectionLength);

    glColor3f(topColor, topColor, topColor);
    glVertex3f(0.0f, -bodyHeight, bodyLength / 2.0f); // Center top of the body slope, lighter color

    // Slope - Left side (wing tip to center top)
    glColor3f(baseColor, baseColor, baseColor);
    glVertex3f(-halfBodyWidth, 0.0f, wingBodyIntersectionLength);

    glColor3f(baseColor, baseColor, baseColor);
    glVertex3f(-halfWingWidth, 0.0f, wingLength); // Left wing tip

    glColor3f(topColor, topColor, topColor);
    glVertex3f(0.0f, -bodyHeight, bodyLength / 2.0f); // Center top of the body slope, lighter color

    // Connect wing tip to front tip
    glColor3f(baseColor, baseColor, baseColor);
    glVertex3f(-halfWingWidth, 0.0f, wingLength); // Left wing tip

    glColor3f(topColor, topColor, topColor);
    glVertex3f(frontTipX, frontTipY, frontTipZ); // Front tip of the plane, lighter color

    glVertex3f(0.0f, -bodyHeight, bodyLength / 2.0f); // Center top of the body slope, lighter color

    // Slope - Right side
    glColor3f(baseColor, baseColor, baseColor); // Darker color at the base of the slope
    glVertex3f(halfBodyWidth, 0.0f, 0.0f); // Base of the slope

    glVertex3f(halfBodyWidth, 0.0f, wingBodyIntersectionLength);

    glColor3f(topColor, topColor, topColor); // Lighter color at the top of the slope
    glVertex3f(0.0f, -bodyHeight, bodyLength / 2.0f); // Center top of the body slope

    // Slope - Right side (wing tip to center top)
    glColor3f(baseColor, baseColor, baseColor); // Base color for the wingBodyIntersectionLength
    glVertex3f(halfBodyWidth, 0.0f, wingBodyIntersectionLength);

    glColor3f(baseColor, baseColor, baseColor); // Base color for the wing tip
    glVertex3f(halfWingWidth, 0.0f, wingLength); // Right wing tip

    glColor3f(topColor, topColor, topColor); // Lighter color at the top
    glVertex3f(0.0f, -bodyHeight, bodyLength / 2.0f); // Center top of the body slope

    // Connect wing tip to front tip
    glColor3f(baseColor, baseColor, baseColor); // Base color for the wing tip
    glVertex3f(halfWingWidth, 0.0f, wingLength); // Right wing tip

    glColor3f(topColor, topColor, topColor); // Lighter color at the top
    glVertex3f(frontTipX, frontTipY, frontTipZ); // Front tip of the plane

    glVertex3f(0.0f, -bodyHeight, bodyLength / 2.0f); // Center top of the body slope

    // Adjusted variables for a smaller tail
    float tailFinHeight = bodyHeight * 0.5f; // Decrease for a shorter tail fin
    float horizontalStabilizerLength = wingLength * 0.25f; // Decrease for shorter horizontal stabilizers
    float horizontalStabilizerWidth = halfWingWidth * 0.5f; // Decrease for narrower horizontal stabilizers
    float lineExtensionLength = horizontalStabilizerLength * 3;

    // Variables for the tail position remain the same
    float tailBaseX = 0.0f;
    float tailBaseY = -bodyHeight / 2.0f;
    float tailBaseZ = bodyLength + lineExtensionLength;

    glBegin(GL_TRIANGLES);

    // Vertical Stabilizer (Tail Fin) - Adjusted for smaller size
    glColor3f(1.0f, 1.0f, 1.0f); // White for the tail
    // Vertical Stabilizer (Tail Fin) - Positioned at the end of the extended line
    glVertex3f(tailBaseX, tailBaseY, tailBaseZ); // Base center of the tail fin
    glVertex3f(tailBaseX, tailBaseY + tailFinHeight, tailBaseZ); // Top of the tail fin
    glVertex3f(tailBaseX, tailBaseY, tailBaseZ + horizontalStabilizerWidth); // Base of the tail fin

    // Left Horizontal Stabilizer - Adjusted for the new tail position
    glVertex3f(tailBaseX - horizontalStabilizerWidth, tailBaseY, tailBaseZ); // Outer left
    glVertex3f(tailBaseX, tailBaseY, tailBaseZ); // Base center of the tail fin
    glVertex3f(tailBaseX - horizontalStabilizerWidth, tailBaseY, tailBaseZ + horizontalStabilizerLength); // Front left

    // Right Horizontal Stabilizer - Adjusted for the new tail position
    glVertex3f(tailBaseX, tailBaseY, tailBaseZ); // Base center of the tail fin
    glVertex3f(tailBaseX + horizontalStabilizerWidth, tailBaseY, tailBaseZ); // Outer right
    glVertex3f(tailBaseX + horizontalStabilizerWidth, tailBaseY, tailBaseZ + horizontalStabilizerLength); // Front right

    glEnd(); // Finish drawing the smaller tail

    // Set the desired line thickness
    glLineWidth(5.0f); // Example: set the line width to 5 pixels

    // Draw line from body to tail for visual connection
    glBegin(GL_LINES);
    // Starting point of the line remains at the center of the body
    glVertex3f(0.0f, -bodyHeight / 2.0f, 0.0f);
    // Extend the endpoint further, with the increased line extension length
    glVertex3f(tailBaseX, tailBaseY, tailBaseZ);
    glEnd(); // End GL_LINES primitive

    // Reset line width to default if necessary for other drawings
    glLineWidth(1.0f);
}




//|____________________________________________________________________
//|
//| Function: main
//|
//! \param None.
//! \return None.
//!
//! Program entry point
//|____________________________________________________________________

int main(int argc, char** argv)
{
    InitMatrices();

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(w_width, w_height);

    glutCreateWindow("Plane Episode 1");

    glutDisplayFunc(DisplayFunc);
    glutReshapeFunc(ReshapeFunc);
    glutKeyboardFunc(KeyboardFunc);

    InitGL();

    glutMainLoop();

    return 0;
}
