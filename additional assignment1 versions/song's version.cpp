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

void FillScanLine(int x, float yStart, float yEnd, unsigned char rStart, unsigned char gStart, unsigned char bStart, unsigned char rEnd, unsigned char gEnd, unsigned char bEnd) {
    float dy = yEnd - yStart;
    if (dy == 0) dy = 1; // Avoid division by zero

    float rStep = (rEnd - rStart) / dy;
    float gStep = (gEnd - gStart) / dy;
    float bStep = (bEnd - bStart) / dy;

    float r = rStart, g = gStart, b = bStart;
    for (int y = ceil(yStart); y <= floor(yEnd); y++) {
        setPixel(x, y, (GLubyte)r, (GLubyte)g, (GLubyte)b);
        r += rStep;
        g += gStep;
        b += bStep;
    }
}


// The main triangle scan conversion function | เอาไว้วาดสามเหลี่ยมม
// Assumes x0 <= x1 <= x2
void ScanConvertTriangle(int vertices[3][2], unsigned char color[3][3]) {
    // Sort vertices by y-coordinate for this example
    sortVertices(vertices); // Assume this sorts by y-coordinate for rasterization

    // For each horizontal scanline
    for (int y = vertices[0][1]; y <= vertices[2][1]; y++) {
        // Find the two edges intersecting this scanline
        float alpha = (float)(y - vertices[0][1]) / (vertices[2][1] - vertices[0][1]);
        float beta = (y <= vertices[1][1]) ?
            (float)(y - vertices[0][1]) / (vertices[1][1] - vertices[0][1]) :
            (float)(y - vertices[1][1]) / (vertices[2][1] - vertices[1][1]);

        // Interpolate x-coordinates along the edges
        int xLeft = vertices[0][0] + (int)(alpha * (vertices[2][0] - vertices[0][0]));
        int xRight = y <= vertices[1][1] ?
            vertices[0][0] + (int)(beta * (vertices[1][0] - vertices[0][0])) :
            vertices[1][0] + (int)((1 - beta) * (vertices[2][0] - vertices[1][0]));

        // Calculate interpolated colors
        unsigned char rLeft = color[0][0] + (unsigned char)(alpha * (color[2][0] - color[0][0]));
        unsigned char gLeft = color[0][1] + (unsigned char)(alpha * (color[2][1] - color[0][1]));
        unsigned char bLeft = color[0][2] + (unsigned char)(alpha * (color[2][2] - color[0][2]));

        unsigned char rRight, gRight, bRight;
        if (y <= vertices[1][1]) {
            rRight = color[0][0] + (unsigned char)(beta * (color[1][0] - color[0][0]));
            gRight = color[0][1] + (unsigned char)(beta * (color[1][1] - color[0][1]));
            bRight = color[0][2] + (unsigned char)(beta * (color[1][2] - color[0][2]));
        }
        else {
            rRight = color[1][0] + (unsigned char)((1 - beta) * (color[2][0] - color[1][0]));
            gRight = color[1][1] + (unsigned char)((1 - beta) * (color[2][1] - color[1][1]));
            bRight = color[1][2] + (unsigned char)((1 - beta) * (color[2][2] - color[1][2]));
        }

        // Fill scanline with interpolated colors
        for (int x = xLeft; x <= xRight; x++) {
            float gamma = (float)(x - xLeft) / (xRight - xLeft);
            unsigned char r = rLeft + (unsigned char)(gamma * (rRight - rLeft));
            unsigned char g = gLeft + (unsigned char)(gamma * (gRight - gLeft));
            unsigned char b = bLeft + (unsigned char)(gamma * (bRight - bLeft));

            setPixel(x, y, r, g, b);
        }
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

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (cnt < 3) { // Capture only the first three clicks
            points[cnt][0] = x;
            points[cnt][1] = HEIGHT - y - 1; // Correctly invert the y coordinate
            cnt++;
        }

        if (cnt == 3) {
            memset(frame_buffer, 0, sizeof(frame_buffer)); // Clear the frame buffer
            ScanConvertTriangle(points, color); // Draw the triangle
            cnt = 0; // Reset counter for the next triangle
            glutPostRedisplay(); // Request a redraw to display the drawn triangle
        }
    }
}

/* Called by GLUT when a display event occurs: */
// this function will 
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);

    glRasterPos2i(-1, -1);
    glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, frame_buffer);

    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Triangle Drawing");

    glutDisplayFunc(display);
    glutMouseFunc(mousebuttonhandler);

    memset(frame_buffer, 0, sizeof(frame_buffer)); // Initialize frame buffer to black

    glutMainLoop();

    return 0;
}