/* Extend the code to satisfy the assignment 1 requirements */
//ssss

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

/*void FillScanLine(int x, float yStart, float yEnd, unsigned char r, unsigned char g, unsigned char b) {
    int y, yMin = ceil(fmin(yStart, yEnd)), yMax = floor(fmax(yStart, yEnd));
    for (y = yMin; y <= yMax; y++) {
        setPixel(x, y, r, g, b);
    }
}*/
void FillScanLine(int x, float yStart, float yEnd, float rStart, float gStart, float bStart, float rEnd, float gEnd, float bEnd) {
    int y;
    float r, g, b;
    float mr, mg, mb;

    // Color gradients
    mr = (rEnd - rStart) / (yEnd - yStart);
    mg = (gEnd - gStart) / (yEnd - yStart);
    mb = (bEnd - bStart) / (yEnd - yStart);

    // Initial color values
    r = rStart + mr * (ceil(yStart) - yStart);
    g = gStart + mg * (ceil(yStart) - yStart);
    b = bStart + mb * (ceil(yStart) - yStart);

    // Loop over each pixel in the scan line
    for (y = ceil(yStart); y <= floor(yEnd); y++) {
        // Set the pixel with the current color values
        setPixel(x, y, (GLubyte)(r + 0.5f), (GLubyte)(g + 0.5f), (GLubyte)(b + 0.5f));

        // Increment the color for the next pixel
        r += mr;
        g += mg;
        b += mb;
    }
}

// The main triangle scan conversion function | เอาไว้วาดสามเหลี่ยมม
// Assumes x0 <= x1 <= x2
void ScanConvertTriangle(int vertices[3][2], unsigned char color[3][3]) {
    sortVertices(vertices); // Ensure vertices are sorted by x-coordinate

    // Extract sorted vertices and associated colors
    int x0 = vertices[0][0], y0 = vertices[0][1];
    unsigned char r0 = color[0][0], g0 = color[0][1], b0 = color[0][2];

    int x1 = vertices[1][0], y1 = vertices[1][1];
    unsigned char r1 = color[1][0], g1 = color[1][1], b1 = color[1][2];

    int x2 = vertices[2][0], y2 = vertices[2][1];
    unsigned char r2 = color[2][0], g2 = color[2][1], b2 = color[2][2];

    // Calculate the slopes of the triangle edges
    float invSlope1 = (x1 - x0) != 0 ? (float)(y1 - y0) / (x1 - x0) : FLT_MAX;
    float invSlope2 = (x2 - x1) != 0 ? (float)(y2 - y1) / (x2 - x1) : FLT_MAX;
    float invSlope3 = (x2 - x0) != 0 ? (float)(y2 - y0) / (x2 - x0) : FLT_MAX;

    // Interpolation factors for color gradients
    float factor01 = (y1 - y0) != 0 ? 1.0f / (y1 - y0) : 0;
    float factor02 = (y2 - y0) != 0 ? 1.0f / (y2 - y0) : 0;
    float factor12 = (y2 - y1) != 0 ? 1.0f / (y2 - y1) : 0;

    // Start and end points for the x-coordinates
    float curx1 = x0, curx2 = x0;
    float curry1 = y0, curry2 = y0;

    // Color values at the start and end of a scan line
    float currR1 = r0, currG1 = g0, currB1 = b0;
    float currR2 = r0, currG2 = g0, currB2 = b0;

    // Scan conversion of the triangle
    for (int scanX = x0; scanX <= x2; scanX++) {
        // Determine the y-coordinates and color at the start and end of this scan line
        if (scanX < x1) {
            curry1 = y0 + invSlope1 * (scanX - x0);
            currR1 = r0 + (r1 - r0) * (curry1 - y0) * factor01;
            currG1 = g0 + (g1 - g0) * (curry1 - y0) * factor01;
            currB1 = b0 + (b1 - b0) * (curry1 - y0) * factor01;
        }
        else {
            curry1 = y1 + invSlope2 * (scanX - x1);
            currR1 = r1 + (r2 - r1) * (curry1 - y1) * factor12;
            currG1 = g1 + (g2 - g1) * (curry1 - y1) * factor12;
            currB1 = b1 + (b2 - b1) * (curry1 - y1) * factor12;
        }
        curry2 = y0 + invSlope3 * (scanX - x0);
        currR2 = r0 + (r2 - r0) * (curry2 - y0) * factor02;
        currG2 = g0 + (g2 - g0) * (curry2 - y0) * factor02;
        currB2 = b0 + (b2 - b0) * (curry2 - y0) * factor02;

        // Fill in the scan line
        FillScanLine(scanX, curry1, currR1, currG1, currB1, curry2, currR2, currG2, currB2);
    }
}


void mousebuttonhandler(int button, int state, int x, int y) {
    static int cnt = 0;
    static int points[3][2];
    static unsigned char color[3][3] = {
        {255, 0, 0}, // Red
        {0, 255, 0}, // Green
        {0, 0, 255}  // Blue
    };

    printf("Mouse clicked=%d, x=%d, y=%d\n", cnt, x, y);

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        points[cnt][0] = x;
        points[cnt][1] = HEIGHT - y - 1; // Convert GLUT's y-coordinate to frame buffer's coordinate system
        cnt++;
        /*
        if (cnt == 1) {
            memset(frame_buffer, 0, sizeof(frame_buffer));
        }
        */
        if (cnt == 3) {
            memset(frame_buffer, 0, sizeof(frame_buffer));
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