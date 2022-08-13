/** gl-main-engine.c
 * OpenGL Engine emulating original doom's engine.
 *
 */

#if _WIN32
#include <Windows.h>
#endif

#include <math.h>
#include <stdio.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>



/*-----------------------------Screen Config--------------------------------*/
#define res         4                   // 0=160x120 1=360x240 4=640x480.
#define SW          160*res             // Screen width.
#define SH          120*res             // Screen height.
#define SW2         (SW/2)              // Half of the screen width.
#define SH2         (SH/2)              // Half of the screen height.
#define pixelScale  4/res               // OpenGL pixel scale
#define GLSW        (SW*pixelScale)     // OpenGL window width.
#define GLSH        (SH*pixelScale)     // OpenGL window height.
#define numSect     4                   // Number of sectors.
#define numWall     16                  // Number of walls.
/*-----------------------------Screen Config--------------------------------*/

typedef struct
{
    int fr1,fr2;    // Frame 1 and frame 2, to create a constant frame rate.
} time; time T;

typedef struct
{
    int w,s,a,d;    // Move up, down, left, right.
    int sl,sr;      // Strafe left, right.
    int m;          // Look up, down.
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

typedef struct
{
    int x1, y1; // Bottom line point 1.
    int x2, y2; // Bottom line point 2.
    int c;      // Wall color;
} walls; walls W[30];

typedef struct
{
    int ws, we;     // Wall number start and end.
    int z1, z2;     // Height of bottom and top.
    int d;          // Add y distances to stort drawing order.
    int c1, c2;     // Bottom and top color.
    int surf[SW];   // To hold points for surfaces.
    int surface;    // Is there a surface to draw.
} sectors; sectors S[30];


/*-----------------------------------------------------------------*/

void pixel(int x, int y, int c)         // Draw a pixel with color (c) at x/y.
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

void clipBehindPlayer (int *x1, int *y1, int *z1, int x2, int y2, int z2) // Clip Line.
{
    float da = *y1;     // Distance plane -> point a.
    float db = y2;      // Distance plane -> point b.
    float d = da-db; if (d==0) { d=1;}
    float s = da / (da-db);     // Intersection factor. (between 0 and 1)
    *x1 = *x1 + s*(x2-(*x1));;
    *y1 = *y1 + s*(y2-(*y1)); if (*y1==0) { *y1=1;} // Prevent divide by zero.
    *z1 = *z1 + s*(z2-(*z1));
}

//             [Start Position] [Bottom Y Points] [Top Lines Points] [color]
void drawWall (int x1, int x2, int b1, int b2, int t1, int t2, int c, int s)
{
    int x, y;

    // Hold the difference in distance
    int dyb = b2 - b1;                       // y distance of bottom line.
    int dyt = t2 - t1;                       // y distance of top line.
    int dx  = x2 - x1; if (dx==0) {dx=1;};   // x distance.
    int xs   = x1;                           // Holds initial x1 starting position.

    // Clipping X.
    if (x1 < 1) { x1 =      1;} // Clip left.
    if (x2 < 1) { x2 =      1;} // Clip left.
    if (x1 > SW-1) { x1 = SW - 1;} // Clip Right.
    if (x2 > SW-1) { x2 = SW - 1;} // Clip Right. 

    // NOTE You have to draw from the first
    // x value to the second x value.
    // Draw x vertical lines.
    for (x = x1; x < x2; x++) {
        // The Y start and end point.
        int y1 = dyb * (x - xs + 0.5) / dx+b1; //y bottom point.
        int y2 = dyt * (x - xs + 0.5) / dx+t1; //y bottom point.
        
        // Clipping Y.
        if (y1 < 1) { y1 =      1;} // Clip left.
        if (y2 < 1) { y2 =      1;} // Clip left.
        if (y1 > SH-1) { y1 = SH - 1;} // Clip Right.
        if (y2 > SH-1) { y2 = SH - 1;} // Clip Right. 
        
        // Surface first loop
        if (S[s].surface == 1) { S[s].surf[x]=y1; continue;} // Save bottom points if we need to draw back wall because player is below the floor.
        if (S[s].surface == 2) { S[s].surf[x]=y2; continue;} // Save top points of that back wall.

        // Surface second loop.
        if (S[s].surface == -1) { //Back bottom wall points
            for (y=S[s].surf[x]; y<y1; y++) { pixel(x,y,S[s].c1);} 
        }
        if (S[s].surface == -2) { // Previous top back wall points
            for (y=y2; y<S[s].surf[x]; y++) { pixel(x,y,S[s].c2);}
        }
        for (y=y1; y<y2; y++) { pixel(x,y,c);} // Normal front facing wall.
    }
}

int dist(int x1, int y1, int x2, int y2)
{
    int distance = sqrt( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) );
    return distance;
}

void draw3D()
{
    int s, w, loop, wx[4], wy[4], wz[4]; // x,y,z for the wall.
    float CS=M.cos[P.a], SN=M.sin[P.a]; // Hold cos and sin value based on player rotation.

    // Order sectors by distance;
    for (s=0; s<numSect-1; s++) {
        for (w=0; w<numSect-s-1; w++) {
            if (S[w].d<S[w+1].d) {
                sectors st=S[w]; S[w]=S[w+1]; S[w+1]=st;
            }
        }
    }

    // Draw Sectors.
    for (s=0;s<numSect;s++) {

        S[s].d=0;   // Clear distance.

         if (P.z<S[s].z1) { S[s].surface=1;}     // Bottom surface.
    else if (P.z>S[s].z2) { S[s].surface=2;}     // Top surface.
    else                  { S[s].surface=0;}     // no surface.



        for (loop=0; loop<2; loop++) {

            // Draw each sector's walls twice. 
            // Once for the back walls and once again for the front walls. 
            for (w=S[s].ws; w<S[s].we; w++) { 

                // Offset bottom 2 points by player.
                int x1=W[w].x1-P.x, y1=W[w].y1-P.y;
                int x2=W[w].x2-P.x, y2=W[w].y2-P.y;

                // Swap for surface.
                if (loop == 0) {
                    int swp=x1; x1=x2; x2=swp; swp=y1; y1=y2; y2=swp;
                }

            /* --------------------World Position--------------------------- */
                // World X position.
                wx[0] = (x1 * CS) - (y1 * SN);
                wx[1] = (x2 * CS) - (y2 * SN);
                wx[2] = wx[0]; // Top line has the same x.
                wx[3] = wx[1];
                

                // World Y position. (depth)
                wy[0] = (y1 * CS) + (x1 * SN);
                wy[1] = (y2 * CS) + (x2 * SN);
                wy[2] = wy[0]; // Top line has the same y.
                wy[3] = wy[1];
                S[s].d+=dist(0,0, (wx[0] + wx[1])/2, (wy[0] + wy[1])/2 );   // Store this wall distance.

                // World Z height.
                wz[0] = S[s].z1-P.z + ((P.l*wy[0])/32.0);
                wz[1] = S[s].z1-P.z + ((P.l*wy[1])/32.0);
                wz[2] = wz[0] + S[s].z2;
                wz[3] = wz[1] + S[s].z2;
            /* --------------------World Position--------------------------- */

            // Don't draw if behind player.
            if (wy[0]<1 && wy[1]<1) { continue;} // Wall behind player

            // Point 1 behind player, clip.
            if (wy[0]<1) {
                clipBehindPlayer(&wx[0], &wy[0], &wz[0], wx[1], wy[1], wz[1]); // Bottom Line.
                clipBehindPlayer(&wx[2], &wy[2], &wz[2], wx[3], wy[3], wz[3]); // Top Line.
            }

            // Point 2 behind player, clip.
            if (wy[1] < 1) {
                clipBehindPlayer(&wx[1], &wy[1], &wz[1], wx[0], wy[0], wz[0]); // Bottom Line.
                clipBehindPlayer(&wx[3], &wy[3], &wz[3], wx[2], wy[2], wz[2]); // Top Line.
            }

            /* --------------------Screen Position--------------------------- */
                // Screen x and screen y position.
                // 200 : Field of view.
                wx[0] = wx[0] * 200/wy[0] + SW2;
                wy[0] = wz[0] * 200/wy[0] + SH2;
                wx[1] = wx[1] * 200/wy[1] + SW2;
                wy[1] = wz[1] * 200/wy[1] + SH2;
                wx[2] = wx[2] * 200/wy[2] + SW2;
                wy[2] = wz[2] * 200/wy[2] + SH2;
                wx[3] = wx[3] * 200/wy[3] + SW2;
                wy[3] = wz[3] * 200/wy[3] + SH2;
            /* --------------------Screen Position--------------------------- */

                // Draw points.
                // if ( wx[0]>0 && wx[0]<SW && wy[0]>0 && wy[0]<SH ) {
                //     pixel(wx[0], wy[0], 0);
                // }

                // if ( wx[1]>1 && wx[1]<SW && wy[1]>0 && wy[1]<SH ) {
                //     pixel(wx[1], wy[1], 0);
                // }

                drawWall(wx[0], wx[1], wy[0], wy[1], wy[2], wy[3], W[w].c, s);
            }

            S[s].d/=(S[s].we-S[s].ws);  // Find average sector distance.
            S[s].surface*=-1;           // Flip to negative to draw surface.
        }
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

// Update this when updating struct sectors.
int loadSectors[] =
{ // wall start, wall end, z1 height, z2 height, bottom color, top color
    0,  4,  0,  40, 2,  3, // Sector 1
    4,  8,  0,  40, 4,  5, // Sector 2
    8,  12, 0,  40, 6,  7, // Sector 3
    12, 16, 0,  40, 0,  1, // Sector 4
};

int loadWalls[] =
{ // x1, y1, x2, y2, color
    0,  0,  32, 0,  0,
    32, 0,  32, 32, 1,
    32, 32, 0,  32, 0,
    0,  32, 0,  0,  1,

    64, 0,  96, 0,  2,
    96, 0,  96, 32, 3,
    96, 32, 64, 32, 2,
    64, 32, 64, 0,  3,

    64, 64, 96, 64, 4,
    96, 64, 96, 96, 5,
    96, 96, 64, 96, 4,
    64, 96, 64, 64, 5,

    0,  64, 32, 64, 6,
    32, 64, 32, 96, 7,
    32, 96, 0,  96, 6,
    0,  96, 0,  64, 7,
};


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

    // Load sectors
    int s, w, v1=0, v2=0;
    for (s=0; s<numSect; s++) {
        S[s].ws=loadSectors[v1+0];                      // Wall start number.
        S[s].we=loadSectors[v1+1];                      // Wall end number.
        S[s].z1=loadSectors[v1+2];                      // Sector bottom height.
        S[s].z2=loadSectors[v1+3] - loadSectors[v1+2];  // Sector top height.
        S[s].c1=loadSectors[v1+4];                      // Sector top color.
        S[s].c2=loadSectors[v1+5];                      // Secot bottom color.
        v1 += 6;

        // Laod Wall values.
        for (w=S[s].ws; w<S[s].we; w++) {
            W[w].x1=loadWalls[v2+0];    // Bottom x1
            W[w].y1=loadWalls[v2+1];    // Bottom y1
            W[w].x2=loadWalls[v2+2];    // Top    x2
            W[w].y2=loadWalls[v2+3];    // Top    y2
            W[w].c =loadWalls[v2+4];    // Wall color
            v2+=5;
        }
    }

    // Get cursor position.
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