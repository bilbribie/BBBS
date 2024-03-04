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
const float CAM_FOV = 25.0f;     // Field of view in degs

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

// Transformation matrices applied to plane and camera poses
gmtl::Matrix44f ztransp_mat;
gmtl::Matrix44f ztransn_mat;
gmtl::Matrix44f zrotp_mat;
gmtl::Matrix44f zrotn_mat;

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
void DrawPlane(const float width, const float length, const float height);

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
    gmtl::invert(view_mat, cam_pose);                 // View transform is the inverse of the camera pose
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
//| Function: DrawPlane
//|
//! \param width       [in] Width  of the plane.
//! \param length      [in] Length of the plane.
//! \param height      [in] Height of the plane.
//! \return None.
//!
//! Draws the plane.
//|____________________________________________________________________

void DrawPlane(float baseWidth, float length, float height) {
    float halfBaseWidth = baseWidth / 2.0f;

    // Begin drawing primitives
    glBegin(GL_TRIANGLES);

    // Body of the plane - triangular prism
    // Base of the prism
    glColor3f(0.8f, 0.8f, 0.8f); // Light gray color for the body base
    glVertex3f(-halfBaseWidth, 0.0f, 0.0f); // Left back corner of the base
    glVertex3f(halfBaseWidth, 0.0f, 0.0f); // Right back corner of the base
    glVertex3f(0.0f, 0.0f, -length); // Front center of the base (nose)

    // Left side of the prism
    glColor3f(0.6f, 0.6f, 0.6f); // Darker shade for the side
    glVertex3f(-halfBaseWidth, 0.0f, 0.0f); // Left back corner of the base
    glVertex3f(0.0f, height, -length / 2.0f); // Top center of the prism
    glVertex3f(0.0f, 0.0f, -length); // Front center of the base (nose)

    // Right side of the prism
    glColor3f(0.6f, 0.6f, 0.6f); // Darker shade for the side
    glVertex3f(halfBaseWidth, 0.0f, 0.0f); // Right back corner of the base
    glVertex3f(0.0f, 0.0f, -length); // Front center of the base (nose)
    glVertex3f(0.0f, height, -length / 2.0f); // Top center of the prism

    // Back side of the prism (not visible if looking from the front)
    glColor3f(0.6f, 0.6f, 0.6f); // Darker shade for the back side
    glVertex3f(-halfBaseWidth, 0.0f, 0.0f); // Left back corner of the base
    glVertex3f(halfBaseWidth, 0.0f, 0.0f); // Right back corner of the base
    glVertex3f(0.0f, height, -length / 2.0f); // Top center of the prism

    // Wings of the plane (unchanged from previous example)
    // ... (wing drawing code remains the same)

    glEnd();
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

void DisplayFunc(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, w_width, w_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(CAM_FOV, (GLfloat)w_width / (GLfloat)w_height, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Apply the inverse of camera pose as view transformation
    gmtl::Matrix44f invCamPose;
    gmtl::invert(invCamPose, cam_pose);
    glLoadMatrixf(invCamPose.getData());

    // Apply the plane's pose as model transformation
    glMultMatrixf(plane_pose.getData());

    // Draw the paper plane
    DrawPlane(1.0f, 2.0f, 0.2f); // Example call with specific dimensions

    glutSwapBuffers();
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

void KeyboardFunc(unsigned char key, int x, int y) {
    // Define transformation amounts
    const float MOVE_STEP = 0.1f; // The amount by which the plane moves forward/backward
    const float ROLL_STEP = gmtl::Math::deg2Rad(5.0f); // The amount by which the plane rolls (converted to radians)

    switch (key) {
    case 'w': // Move forward
        plane_pose(0, 3) += MOVE_STEP; // Move along the X-axis
        break;
    case 's': // Move backward
        plane_pose(0, 3) -= MOVE_STEP; // Move along the X-axis
        break;
    case 'a': // Roll left
    {
        gmtl::Matrix44f rollMatrix;
        rollMatrix.set(cos(ROLL_STEP), -sin(ROLL_STEP), 0, 0,
            sin(ROLL_STEP), cos(ROLL_STEP), 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1);
        plane_pose = plane_pose * rollMatrix;
    }
    break;
    case 'd': // Roll right
    {
        gmtl::Matrix44f rollMatrix;
        rollMatrix.set(cos(-ROLL_STEP), -sin(-ROLL_STEP), 0, 0,
            sin(-ROLL_STEP), cos(-ROLL_STEP), 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1);
        plane_pose = plane_pose * rollMatrix;
    }
    break;
    // Add more controls as needed
    }

    glutPostRedisplay(); // Redraw the scene with the new transformations
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