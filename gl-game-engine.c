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
#define M_PI        3.14159265          // Stores Pi
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

typedef struct
{
    float cos[360]; // Save sin and cos values from 0-360degrees.
    float sin[360];
} math; math M;

typedef struct
{
    int x, y, z; // Player position. Z is up
    int a;       // Player angle of rotation from left to right.
    int l;       // Variable to look up and down.
} player; player P;

/*-----------------------------------------------------------------*/

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
    // Move left.
    if (K.a == 1 && K.m == 0) {
        P.a-=4;
        if (P.a < 0) { P.a+=360; }; // Loop back around incase of zero. (Look-up array)
    }

    // Move Right.
    if (K.d == 1 && K.m == 0) { 
        P.a+=4;
        if (P.a > 359) { P.a-=360; }; // Loop back around at max value. (Look-up array)
     }

    // Player needs to move with a vector. (Push and Pull)
    int dx=M.sin[P.a]*10.0;
    int dy=M.cos[P.a]*10.0;

    // Move Forwards.
    if (K.w == 1 && K.m == 0) { 
        P.x+=dx;
        P.y+=dy;
     }
     
     // Move Backwards.
    if (K.s == 1 && K.m == 0) { 
        P.x-=dx;
        P.y-=dy;
    }

    // Strafe Left.
    if (K.sr == 1) { 
        P.x+=dy;
        P.y-=dx;
     }

     // Strafe Right.
    if (K.sl == 1) { 
        P.x-=dy;
        P.y-=dx;
     }

    // Look Left.
    if (K.a == 1 && K.m == 1) { 
        P.l-=1;
     }

    // Look Right.
    if (K.d == 1 && K.m == 1) { 
        P.l+=1;
    }

    // Look Up.
    if (K.w == 1 && K.m == 1) { 
        P.z-=4;
     }

    // Look Down.
    if (K.s == 1 && K.m == 1) { 
        P.z+=4;
     }
}

void clearBackground()
{
    int x, y;

    for (y=0; y<SH; y++) { // Clear background color.
        for (x=0; x<SW; x++) { 
            pixel(x, y, 8); 
        } 
    }
}

void draw3D()
{
    int wx[4], wy[4], wz[4]; // x,y,z for the wall.
    float CS=M.cos[P.a], SN=M.sin[P.a]; // Hold cos and sin value based on player rotation.

    // Offset bottom 2 points by player.
    int x1=40-P.x, y1= 10-P.y;
    int x2=40-P.x, y2=290-P.y;

    // World X position.
    wx[0] = (x1 * CS) - (y1 * SN);
    wx[1] = (x2 * CS) - (y2 * SN);

    // World Y position. (depth)
    wy[0] = (y1 * CS) + (x1 * SN);
    wy[1] = (y2 * CS) + (x2 * SN);

    // World Z height.
    wz[0] = 0 - P.z + ((P.l*wy[0])/32.0);
    wz[1] = 0 - P.z + ((P.l*wy[1])/32.0);

    // Screen x and screen y position.
    // 200 : Field of view.
    wx[0] = wx[0] * 200/wy[0] + SW2;
    wy[0] = wz[0] * 200/wy[0] + SH2;
    wx[1] = wx[1] * 200/wy[1] + SW2;
    wy[1] = wz[1] * 200/wy[1] + SH2;

    // Draw points.
    if ( wx[0]>0 && wx[0]<SW && wy[0]>0 && wy[0]<SH ) {
        pixel(wx[0], wy[0], 0);
    }

    if ( wx[1]>1 && wx[1]<SW && wy[1]>0 && wy[1]<SH ) {
        pixel(wx[1], wy[1], 0);
    }
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
    if (key == 's' == 1) { K.s = 1; }
    if (key == 'a' == 1) { K.a = 1; }
    if (key == 'd' == 1) { K.d = 1; }
    if (key == 'm' == 1) { K.m = 1; }
    if (key == ',' == 1) { K.sr = 1; }
    if (key == '.' == 1) { K.sl = 1; }
}

void KeysUp(unsigned char key, int x, int y)
{
    if (key == 'w' == 1) { K.w = 0; }
    if (key == 's' == 1) { K.s = 0; }
    if (key == 'a' == 1) { K.a = 0; }
    if (key == 'd' == 1) { K.d = 0; }
    if (key == 'm' == 1) { K.m = 0; }
    if (key == ',' == 1) { K.sr = 0; }
    if (key == '.' == 1) { K.sl = 0; }
}

// Used to initialize variables.
void init()
{
    // Precalculating sin and cos in degrees and storing the values.
    for (int x=0; x<360; x++) 
    {
        M.cos[x]=cos(x/180.0*M_PI);
        M.sin[x]=sin(x/180.0*M_PI);
    }

    // Init player position variables
    P.x=70; P.y=-110; P.z=20; P.a=0; P.l=0;
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

    init();
    printf("Running...\n");

    glutDisplayFunc(display);
    glutKeyboardFunc(KeysDown);
    glutKeyboardUpFunc(KeysUp);
    glutMainLoop();

    return 0;
}