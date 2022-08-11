/** gl-main-engine.c
 * OpenGL Engine emulating original doom's engine.
 *
 */

#include <math.h>
#include <stdio.h>
#include <GL/glut.h>

#define res         1                   // 0=160x120 1=360x240 4=640x480.
#define SW          160*res             // Screen width.
#define SH          120*res             // Screen height.
#define SW2         (SW/2)              // Half of the screen width.
#define SH2         (SH/2)              // Half of the screen height.
#define pixelScale  4/res               // OpenGL pixel scale
#define GLSW        (SW*pixelScale)     // OpenGL window width.
#define GLSH        (SH*pixelScale)     // OpenGL window height.
/*----------------------------------------------------------------*/

typedef struct
{
    int fr1,fr2;    // Frame 1 and frame 2, to create a constant frame rate.
} time; time T;

typedef struct
{
    int w,s,a,d;    // Move up, down, left, right.
    int sl,sr;      // Strafe left, right.
    int m;          // Move up, down, look up, down.
} keys; keys K;

/*-----------------------------------------------------------------*/

int tick;

void pixel(int x, int y, int c)         // Draw a pixel at x/y with rgb.
{
    int rgb[3];

    if (c==0) { rgb[0]=255; rgb[1]=255; rgb[2]=  0;} // Yellow.
    if (c==1) { rgb[0]=160; rgb[1]=160; rgb[2]=  0;} // Yellow Darker.
    if (c==2) { rgb[0]=  0; rgb[1]=255; rgb[2]=  0;} // Green.
    if (c==3) { rgb[0]=  0; rgb[1]=160; rgb[2]=  0;} // Green Darker.
    if (c==4) { rgb[0]=  0; rgb[1]=255; rgb[2]=255;} // Cyan.
    if (c==5) { rgb[0]=  0; rgb[1]=160; rgb[2]=160;} // Cyan Darker.
    if (c==6) { rgb[0]=160; rgb[1]=100; rgb[2]=  0;} // Brown.
    if (c==7) { rgb[0]=110; rgb[1]= 50; rgb[2]=  0;} // Brown Darker.
    if (c==8) { rgb[0]= 60; rgb[1]= 60; rgb[2]=130;} // Background

    glColor3ub(rgb[0], rgb[1], rgb[2]);
    glBegin(GL_POINTS);
    glVertex2i(x*pixelScale+2, y+pixelScale+2);
    glEnd();
}

void movePlayer()
{
    // Move left, right, up, and down.
    if (K.a == 1 && K.m == 0) { printf("left\n"); }
    if (K.d == 1 && K.m == 0) { printf("right\n"); }
    if (K.w == 1 && K.m == 0) { printf("up\n"); }
    if (K.s == 1 && K.m == 0) { printf("down\n"); }

    // Strafe left and right.
    if (K.sr == 1) { printf("strafe left\n"); }
    if (K.sl == 1) { printf("strafe right\n"); }

    // Look left, right, up, and down.
    if (K.a == 1 && K.m == 1) { printf("Look left\n"); }
    if (K.d == 1 && K.m == 1) { printf("Look right\n"); }
    if (K.w == 1 && K.m == 1) { printf("Look up\n"); }
    if (K.s == 1 && K.m == 1) { printf("Look down\n"); }
}

void clearBackground()
{
    int x, y;
    for (y=0; y<SH; y++) {
        for (x=0; x<SW; x++) { pixel(x, y, 8); } // Clear background color.
    }
}

void draw3D()
{
    int x, y, c=0;
    
    for (y=0; y<SH2; y++) {
        for (x=0; x<SW2; x++) {
            pixel(x, y, c);
            c+=1; 
            if (c>8) { c=0; }
        }
    }
    
    // Frame rate.
    tick+=1; 
    if (tick>20) { tick=0; } 
    pixel(SW2, SH2+tick, 0);
}

// Renders display
void display()
{
    int x, y;

    if ((T.fr1 - T.fr2) >= 50) {               // Only draw 20 frames/second
        clearBackground();
        movePlayer();
        draw3D();

        T.fr2 = T.fr1;
        glutSwapBuffers();
        glutReshapeWindow(GLSW, GLSH);          // Prevent window scaling.
    }

    T.fr1 = glutGet(GLUT_ELAPSED_TIME);         // 1000 Milliseconds per second.
    glutPostRedisplay();
}

void KeysDown(unsigned char key, int x, int y)
{
    if (key == 'w' == 1) { K.w = 1; }
    if (key == 's' == 1) { K.w = 1; }
    if (key == 'a' == 1) { K.w = 1; }
    if (key == 'd' == 1) { K.w = 1; }
    if (key == 'm' == 1) { K.w = 1; }
    if (key == ',' == 1) { K.w = 1; }
    if (key == '.' == 1) { K.w = 1; }
}

void KeysUp(unsigned char key, int x, int y)
{
    if (key == 'w' == 1) { K.w = 0; }
    if (key == 's' == 1) { K.w = 0; }
    if (key == 'a' == 1) { K.w = 0; }
    if (key == 'd' == 1) { K.w = 0; }
    if (key == 'm' == 1) { K.w = 0; }
    if (key == ',' == 1) { K.w = 0; }
    if (key == '.' == 1) { K.w = 0; }
}

void init()
{
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv); // Init with arguments.
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(GLSW/2, GLSH/2);
    glutInitWindowSize(GLSW, GLSH);
    glutCreateWindow("");
    glPointSize(pixelScale); // Pixel Size.
    gluOrtho2D(0, GLSW, 0, GLSH); // Origin Bottom Left.
    

    init(); // Placeholder for now.

    glutDisplayFunc(display);
    glutKeyboardFunc(KeysDown);
    glutKeyboardUpFunc(KeysUp);
    glutMainLoop();

    return 0;
}