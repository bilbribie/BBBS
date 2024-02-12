/* Extend the code to satisfy the assignment 1 requirements */

#include <stdio.h>
#include <math.h>
#include <memory.h>
#include <GL/glut.h>
#define WIDTH 400		
#define HEIGHT 300	//set height and width of window that show result
static GLubyte frame_buffer[HEIGHT][WIDTH][3]; //to store pixelcolor data

// to store pixel color in  to frame
void setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) {  // นายซองค์ต้องมาแก้พาร์ทนี้ ಠ_ಠ
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        frame_buffer[y][x][0] = r;
        frame_buffer[y][x][1] = g;
        frame_buffer[y][x][2] = b;
    }
}

// เอาไว้หาว่าจุดที่เราจิ้มในเมาส์อยู่ไหน ใช้ bouble sort ไม่ว่าจุจะเริ่มที่ไหนก็สามารถเอามาสร้างสามเหลี่ยมได้
void sortVertices(int points[3][2]) {
    for (int i = 0; i < 2; ++i) {
        for (int j = i + 1; j < 3; ++j) {
            if (points[i][0] > points[j][0]) {
                int tempX = points[i][0], tempY = points[i][1];
                points[i][0] = points[j][0], points[i][1] = points[j][1];
                points[j][0] = tempX, points[j][1] = tempY;
            }
        }
    }
}

// Fill each scanline
// yT = top of the scanline, yB = bottom of the scanline

void FillScanLine(int x, float yT, float rT, float gT, float bT, float yB, float rB, float gB, float bB)
{
    int yT_floor, yB_ceil;
    int y;
    float r, g, b, mr, mg, mb;

    yT_floor = floor(yT);
    yB_ceil = ceil(yB);

    float dy = yT - yB;
    if (dy == 0) {      // <-- Add this line
        dy = 1;         // <-- Add this line
    }

    mr = (rT - rB) / dy;  // Change (yT - yB) to dy
    mg = (gT - gB) / dy;  // Change (yT - yB) to dy
    mb = (bT - bB) / dy;  // Change (yT - yB) to dy
    r = rB + (yB_ceil - yB) * mr;
    g = gB + (yB_ceil - yB) * mg;
    b = bB + (yB_ceil - yB) * mb;

    for (y = yB_ceil; y <= yT_floor; y++) {
        frame_buffer[y][x][0] = (unsigned char)(r + 0.5f);
        frame_buffer[y][x][1] = (unsigned char)(g + 0.5f);
        frame_buffer[y][x][2] = (unsigned char)(b + 0.5f);

        r += mr;
        g += mg;
        b += mb;
    }
}

/*void FillScanLine(int x, float yStart, float yEnd, unsigned char r, unsigned char g, unsigned char b) {
    int y, yMin = ceil(fmin(yStart, yEnd)), yMax = floor(fmax(yStart, yEnd));
    for (y = yMin; y <= yMax; y++) {
        setPixel(x, y, r, g, b);
    }
}*/


// The main triangle scan conversion function | เอาไว้วาดสามเหลี่ยมม
// Assumes x0 <= x1 <= x2
void ScanConvertTriangle(int vertices[3][2], unsigned char color[3][3]) {
    sortVertices(vertices); // Ensure vertices are sorted by x-coordinate

    // Extract sorted vertices
    int x0 = vertices[0][0], y0 = vertices[0][1];
    int x1 = vertices[1][0], y1 = vertices[1][1];
    int x2 = vertices[2][0], y2 = vertices[2][1];

    // Calculate slopes for y-coordinates; handle vertical lines by checking divide-by-zero
    float slope0 = x0 != x1 ? (float)(y1 - y0) / (x1 - x0) : 0.0;
    float slope1 = x1 != x2 ? (float)(y2 - y1) / (x2 - x1) : 0.0;
    float slope2 = x0 != x2 ? (float)(y2 - y0) / (x2 - x0) : 0.0;

    // Initialize starting points for y-coordinates on edges
    float yCurr0 = y0, yCurr1 = y0;

    // For each column from x0 to x1-1, interpolate and fill pixels between edges L0,1 and L0,2
    for (int x = x0; x < x1; x++) {
        FillScanLine(x, yCurr0, yCurr1, color[0][0], color[0][1], color[0][2]); // Fill with color of vertex 0 as example
        yCurr0 += slope0; // Move up along edge L0,1
        yCurr1 += slope2; // Move up along edge L0,2
    }

    // Reset starting point for y-coordinate on edge L1,2 for the second half of the triangle
    yCurr0 = y1;

    // For each column from x1 to x2, interpolate and fill pixels between edges L1,2 and L0,2
    for (int x = x1; x <= x2; x++) {
        FillScanLine(x, yCurr0, yCurr1, color[1][0], color[1][1], color[1][2]); // Fill with color of vertex 1 as example
        yCurr0 += slope1; // Move up along edge L1,2
        yCurr1 += slope2; // Continue moving up along edge L0,2
    }
}
/*
void ScanConvertTriangle(
    int x0, int y0, int r0, int g0, int b0,
    int x1, int y1, int r1, int g1, int b1,
    int x2, int y2, int r2, int g2, int b2)
{
    float l01_y, l01_m;
    float l01_r, l01_g, l01_b, l01_mr, l01_mg, l01_mb;
    float l02_y, l02_m;
    float l02_r, l02_g, l02_b, l02_mr, l02_mg, l02_mb;
    int x;

    // Computes the slopes for L01 and L02
    l01_m = (float)(y1 - y0) / (x1 - x0);
    l01_mr = (float)(r1 - r0) / (x1 - x0);
    l01_mg = (float)(g1 - g0) / (x1 - x0);
    l01_mb = (float)(b1 - b0) / (x1 - x0);

    l02_m = (float)(y2 - y0) / (x2 - x0);
    l02_mr = (float)(r2 - r0) / (x2 - x0);
    l02_mg = (float)(g2 - g0) / (x2 - x0);
    l02_mb = (float)(b2 - b0) / (x2 - x0);

    // Initializes the edge tracing
    l01_y = y0;
    l01_r = r0;
    l01_g = g0;
    l01_b = b0;

    l02_y = y0;
    l02_r = r0;
    l02_g = g0;
    l02_b = b0;

    // Scan the first half of the triangle
    // TODO: handle the case when x0==x1
    for (x = x0; x < x1; x++) {
        if (l02_y >= l01_y) {
            FillScanLine(x, l02_y, l02_r, l02_g, l02_b, l01_y, l01_r, l01_g, l01_b);
        }
        else {
            FillScanLine(x, l01_y, l01_r, l01_g, l01_b, l02_y, l02_r, l02_g, l02_b);
        }

        l01_y += l01_m;
        l01_r += l01_mr;
        l01_g += l01_mg;
        l01_b += l01_mb;

        l02_y += l02_m;
        l02_r += l02_mr;
        l02_g += l02_mg;
        l02_b += l02_mb;
    }

    // Scan the second half of the triangle
    // TODO: handle the case when x1==x2
}
*/

/* Called when mouse button pressed: */
/*
void mousebuttonhandler(int button, int state, int x, int y)
{
    static int cnt = 0; // use to count the number of teh triangle
    static int points[3][2]; // store point of triangle
    static unsigned char color[3][3] = {  // array to store colorvalue of triangle
      {255,   0,   0},      // r0, g0, b0
      {  0, 255,   0},      // r1, g1, b1
      {  0,   0, 255}       // r2, g2, b2
    };

    printf("Mouse button event, button=%d, state=%d, x=%d, y=%d\n", button, state, x, y);

    // set a pixel's red color value when left mouse button is pressed down:
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        points[cnt][0] = x;
        points[cnt][1] = HEIGHT - y - 1;
        cnt++;

        // printf("Mouse clicked=%d, x=%d, y=%d\n", cnt, x, y);

        if (cnt == 1) {
            memset(frame_buffer, 0, sizeof(frame_buffer));
        }


        if (cnt == 3) {
            // TODO: make sure point0, 1, 2 are passed in the correct order x0<=x1<=x2
            if (points[0][0] <= points[1][0]) {
                if (points[1][0] <= points[2][0]) { // x0 <= x1 <= x2
                    ScanConvertTriangle(
                        points[0][0], points[0][1], color[0][0], color[0][1], color[0][2],  // x0, y0, r0, g0, b0
                        points[1][0], points[1][1], color[1][0], color[1][1], color[1][2],  // x1, y1, r1, g1, b1
                        points[2][0], points[2][1], color[2][0], color[2][1], color[2][2]   // x2, y2, r2, g2, b2
                    );
                }
                else
                    if (points[0][0] <= points[2][0]) { // x0 <= x2 <= x1
                        ScanConvertTriangle(
                            points[0][0], points[0][1], color[0][0], color[0][1], color[0][2],  // x0, y0, r0, g0, b0
                            points[2][0], points[2][1], color[2][0], color[2][1], color[2][2],  // x2, y2, r2, g2, b2
                            points[1][0], points[1][1], color[1][0], color[1][1], color[1][2]   // x1, y1, r1, g1, b1
                        );
                    }
                    else { // x2 <= x0 <= x1
                        ScanConvertTriangle(
                            points[2][0], points[2][1], color[2][0], color[2][1], color[2][2],  // x2, y2, r2, g2, b2
                            points[0][0], points[0][1], color[0][0], color[0][1], color[0][2],  // x0, y0, r0, g0, b0
                            points[1][0], points[1][1], color[1][0], color[1][1], color[1][2]   // x1, y1, r1, g1, b1
                        );
                    }
            }
            else { // x1 < x0
                // TODO: fill in the rest here
                if (points[1][0] < points[2][0] && points[2][0] < points[0][0]) { // x0 <= x1 <= x2
                    ScanConvertTriangle(
                        points[0][0], points[0][1], color[0][0], color[0][1], color[0][2],  // x0, y0, r0, g0, b0
                        points[1][0], points[1][1], color[1][0], color[1][1], color[1][2],  // x1, y1, r1, g1, b1
                        points[2][0], points[2][1], color[2][0], color[2][1], color[2][2]   // x2, y2, r2, g2, b2
                    );
                }
                else
                    if (points[0][0] <= points[2][0]) { // x0 <= x2 <= x1
                        ScanConvertTriangle(
                            points[0][0], points[0][1], color[0][0], color[0][1], color[0][2],  // x0, y0, r0, g0, b0
                            points[2][0], points[2][1], color[2][0], color[2][1], color[2][2],  // x2, y2, r2, g2, b2
                            points[1][0], points[1][1], color[1][0], color[1][1], color[1][2]   // x1, y1, r1, g1, b1
                        );
                    }
                    else { // x2 <= x0 <= x1
                        ScanConvertTriangle(
                            points[1][0], points[1][1], color[1][0], color[1][1], color[1][2],  // x1, y1, r1, g1, b1
                            points[0][0], points[0][1], color[0][0], color[0][1], color[0][2],  // x0, y0, r0, g0, b0
                            points[2][0], points[2][1], color[2][0], color[2][1], color[2][2]   // x2, y2, r2, g2, b2
                        );
                    }
            }
            cnt = 0; // reset for new triangle
        }
    }


    // cause a display event to occur for GLUT:
    glutPostRedisplay(); // request for display update
}
*/
void mousebuttonhandler(int button, int state, int x, int y) {
    static int cnt = 0;
    static int points[3][2];
    static unsigned char color[3][3] = {
        {255, 0, 0}, // Red
        {0, 255, 0}, // Green
        {0, 0, 255}  // Blue
    };

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        points[cnt][0] = x;
        points[cnt][1] = HEIGHT - y - 1; // Convert GLUT's y-coordinate to frame buffer's coordinate system
        cnt++;

        if (cnt == 1) {
            memset(frame_buffer, 0, sizeof(frame_buffer));
        }

        if (cnt == 3) {
            ScanConvertTriangle(points, color); // Call the modified scan conversion function
            cnt = 0; // Reset count for the next triangle
        }
    }
    glutPostRedisplay(); // Request display update
}

/* Called by GLUT when a display event occurs: */
// this function will 
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT); // (ó﹏ò｡)

    /*	Set the raster position to the lower-left corner to avoid a problem
        (with glDrawPixels) when the window is resized to smaller dimensions.*/
    glRasterPos2i(-1, -1);

    // Write the information stored in "frame_buffer" to the color buffer
    glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, frame_buffer); // draw pixel form fram buffer
    glFlush(); // to ensure all command are execute
}

int main(int argc, char** argv) {

    glutInit(&argc, argv); //prepare environment to use glut and openGL
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); // to set display
    glutInitWindowSize(WIDTH, HEIGHT); //set window size
    glutCreateWindow("Frame Buffer Example"); // create window

    // Specify which functions get called for display and mouse events:
    glutDisplayFunc(display);
    glutMouseFunc(mousebuttonhandler);

    glutMainLoop();

    return 0;
}