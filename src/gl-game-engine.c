/** gl-main-engine.c
 * OpenGL Engine emulating original doom's engine.
 *
 * FIXME Stop surfaces from being drawn if ontop of another sector.
 * TODO Add opengl debugging files.
 * TODO Add windows build setup.
 * TODO Add collisions.
 * TODO Add BSP (Binary Space Partitioning)
 * TODO Add Blockmap. (Collision Detection)
 * TODO Add Sidedefs.
 * TODO Add drawing floors and ceilings.
 * TODO Add drawing sprites.
 * TODO Add Lighting.
 * TODO Add Base level.
 * TODO Add physics.
 * TODO Add Gun Model
 * TODO Make movement better.
 * TODO Make menu.
 * TODO Make controls popup.
 * TODO Make drawing elements event based.
 */

#if _WIN32
#include <Windows.h>
#endif

#include <stdio.h>
#include <math.h>
#include <string.h>

#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "../include/doom_level.h"


/*-----------------------------Screen Config--------------------------------*/
#define res         4                   // 0=160x120 1=360x240 4=640x480.
#define SW          160*res             // Screen width.
#define SH          120*res             // Screen height.
#define SW2         (SW/2)              // Half of the screen width.
#define SH2         (SH/2)              // Half of the screen height.
#define pixelScale  4/res               // OpenGL pixel scale
#define GLSW        (SW*pixelScale)     // OpenGL window width.
#define GLSH        (SH*pixelScale)     // OpenGL window height.
#define FRAMERATE   20                  // 1000/50
#define numSect     12                   // Number of sectors.
#define numWall     100                  // Number of walls.

int window;
/*-----------------------------Screen Config--------------------------------*/

/*------------------------------------MAP-----------------------------------*/
int map[] =
{
1,1,1,1,1,1,1,1,
1,0,1,0,0,0,0,1,
1,0,1,0,0,0,0,1,
1,0,1,0,0,0,0,1,
1,0,0,0,0,0,0,1,
1,0,0,0,0,1,0,1,
1,0,0,0,0,0,0,1,
1,1,1,1,1,1,1,1,
};
/*------------------------------------MAP-----------------------------------*/

/*-----------------------------Engine Classes-------------------------------*/
typedef struct
{
    int fr1,fr2;
} time; time T; // Creating frame 1 and frame 2 for comparison to create a constant frame rate.

typedef struct
{
    int w,s,a,d;    // Move up, down, left, right.
    int sl,sr;      // Strafe left, right.
    int m;          // Look up, down.
    int escape;     // Leave game.
} keys; keys K;

typedef struct
{
    float cos[360];
    float sin[360];
} math; math M;     // Save sin and cos values from 0-360degrees.


typedef struct
{
    int x, y, z, withinBounds;  // Player position.
    int a;                      // Player angle of rotation from left to right. (y rotation)
    int l;                      // Variable to look up and down. (z rotation)
    int health;                 // Player stats: health. (Damage they can take)
    int speed;                  // Player stats: speed. (How fast they can move)
    int ammo;                   // Player stats: ammo. (How much ammo they have)
} player; player P;

typedef struct
{
    int x1, y1;         // Bottom line point 1.
    int x2, y2;         // Bottom line point 2.
    int c;              // Wall color;
    int d;
} walls; walls W[100];

typedef struct
{
    int ws, we;             // Wall number start and end.
    int z1, z2;             // Height of bottom and top.
    int d;                  // Add y distances to stort drawing order.
    int c1, c2;             // Bottom and top color.
    int surf[SW];           // To hold points for surfaces.
    int surface;            // Is there a surface to draw.
} sectors; sectors S[100];

typedef struct
{
    int active;             // Detect if it's a valid projectile.
    float speed = 30.0f;    // Magnitude.
    int dx, dy;             // Components for the vector.
    int angle;              // Direction it is pointed at. (angle)
    int xs, ys;             // Postion.
} bullet; bullet B;
/*-----------------------------Engine Classes-------------------------------*/

/**
 * @brief Handles closing the game properly.
 * 
 */
void closeGame() {
    // Reset input Key Values;
    K.a = 0;
    K.s = 0;
    K.d = 0;
    K.w = 0;
    K.m = 0;
    K.sl = 0;
    K.sr = 0;
    K.escape = 0;

    glutDestroyWindow(window);
}

/**
 * @brief Draws string at x, y, z .
 * 
 * @param x world coordinate for the UI element.
 * @param y world coordinate for the UI element.
 * @param z world coordinate for the UI element.
 * @param string What characters to draw.
 * 
 * TODO Add argument for font.
 */
void drawString(float x, float y, float z, char *string) {

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glColor3f(0.0f,0.0f,0.0f);
    glRasterPos3f(x, y, z);

    for (char* c = string; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *c); // Updated the positon
    }
}

/**
 * @brief Draws a pixel at (x, y) with color (c).
 * 
 * @param x coordinate location.
 * @param y coordinate location.
 * @param c Color the pixel will use[0-8].
 * Yellow |
 * Yellow Darker |
 * Green |
 * Green Darker |
 * Cyan |
 * Cyan Darker |
 * Brown |
 * Brown Darker |
 * Background
 */
void pixel(int x, int y, int c)
{
    int rgb[3];
    int counter = 0;

    if (c==0) { rgb[0]=255; rgb[1]=255; rgb[2]=  0;} // Yellow.
    if (c==1) { rgb[0]=160; rgb[1]=160; rgb[2]=  0;} // Yellow Darker.
    if (c==2) { rgb[0]=  0; rgb[1]=255; rgb[2]=  0;} // Green.
    if (c==3) { rgb[0]=  0; rgb[1]=160; rgb[2]=  0;} // Green Darker.
    if (c==4) { rgb[0]=  0; rgb[1]=255; rgb[2]=255;} // Cyan.
    if (c==5) { rgb[0]=  0; rgb[1]=160; rgb[2]=160;} // Cyan Darker.
    if (c==6) { rgb[0]=160; rgb[1]=100; rgb[2]=  0;} // Brown.
    if (c==7) { rgb[0]=110; rgb[1]= 50; rgb[2]=  0;} // Brown Darker.
    if (c==8) { rgb[0]= 60; rgb[1]= 60; rgb[2]=130;} // Background
    if (c==9) { rgb[0]= 66; rgb[1]= 62; rgb[2]=62;}  // Grey-with a bit of red.
    
    glColor3ub(rgb[0], rgb[1], rgb[2]);
    glBegin(GL_POINTS); // Begin drawing points.
    glVertex2i(x*pixelScale+2, y*pixelScale+2); // Draws a point.a
    glEnd(); // End drawing points.
}

int i =0;
/**
 * @brief WIP
 * 
 */
void checkCollision()
{
    int x, y;
    int dx=M.sin[P.a]*10.0; int dy=M.cos[P.a]*10.0;

    if (P.x >= 390 || P.y >= 390) {
        printf("Collision Detected\n");
        if (K.w == 1) {
            P.x -= dx;
            P.y -= dy;
        }
        if (K.s == 1) {
            P.x += dx;
            P.y += dy;
        }
    }
    if (P.x <= -390 || P.y <= -390) {
        printf("Collision Detected\n");
        if (K.w == 1) {
            P.x -= dx;
            P.y -= dy;
        }
        if (K.s == 1) {
            P.x += dx;
            P.y += dy;
        }
    }

    if(P.x >= -8 && P.x <= 70  && P.y >= -8 && P.y <=55) {
        printf("Collision detected with platform %i \n", i++);
        if (K.w == 1) {
            P.x -= dx;
            P.y -= dy;
        }
        if (K.s == 1) {
            P.x += dx;
            P.y += dy;
        }
        if (P.a > 180) {
            P.x -= dx+2;
            P.y += dy-2;
        }
        if (P.a < 180) {
            P.x += dx-2;
            P.y -= dy+2;
        }
    }

}

/**
 * @brief Handles moving the player.
 * 
 * @note This actually moves the world around the camera view.
 */
void movePlayer()
{
    checkCollision();
    if (P.x > mapX.boundsP || P.x < mapX.boundsN) { P.withinBounds = 0;}
    if (P.y > mapY.boundsP || P.y < mapY.boundsN) { P.withinBounds = 0;}
    if (P.z > mapZ.boundsP || P.z < mapZ.boundsN) { P.withinBounds = 0;}

    // if (!P.withinBounds) { printf("Not within bounds!\n");}

    // Look left.
    if (K.a == 1 && K.m == 0) {
        P.a-=4;
        if (P.a < 0) { P.a+=360; }; // Loop back around incase of zero. (Look-up array)
    }
    // Look Right.
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
        P.y+=dx;
     }
    // Look Down.
    if (K.a == 1 && K.m == 1) { 
        P.l-=1;
     }
    // Look Up.
    if (K.d == 1 && K.m == 1) { 
        P.l+=1;
    }
    // Move Up.
    if (K.w == 1 && K.m == 1) { 
        P.z-=4;
     }
    // Move Down.
    if (K.s == 1 && K.m == 1) { 
        P.z+=4;
     }
}

/**
 * @brief Handles configuring the bullet.
 * 
 * @remark Gun system needs fleshing out.
 */
void bulletHandler()
{
    B.active = 0; // Reset condition.

    // Init Bullet.
    if (!B.active) {
        B.active = 1;
        B.xs = P.x;
        B.ys = P.y;
        B.angle = P.a;
        B.dx = -B.speed * sin(P.a);
        B.dy = B.speed * cos(P.a); 
    }
}

/**
 * @brief Handles mouse input events. (GLUT mouse callback function)
 * 
 * @param button One of GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, GLUT_RIGHT_BUTTON.
 * @param state One of GLUT_UP or GLUT_DOWN.
 * @param mouse_x Window relative coordinates when the mouse button state changed.
 * @param mouse_y Window relative coordinates when the mouse button state changed.
 */
void mouseHandler(int button, int state, int mouse_x, int mouse_y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        printf("Mouse Down Press Detected at %i, %i\n", mouse_x, mouse_y);
    }
}

/**
 * @brief Handles clearing the background with filler pixels.
 * 
 */
void clearBackground()
{
    int x, y;

    for (y=0; y<SH; y++) { // Clear background color.
        for (x=0; x<SW; x++) { 
            pixel(x, y, 8); 
        } 
    }
}

/**
 * @brief Handles taking in top and bottom wall world coordinates 
 * that are behind the player to clip them.
 * 
 * @param x1 wall world coordinate
 * @param y1 wall world coordinate
 * @param z1 wall world coordinate
 * @param x2 wall world coordinate
 * @param y2 wall world coordinate
 * @param z2 wall world coordinate
 */
void clipBehindPlayer (int *x1, int *y1, int *z1, int x2, int y2, int z2) // Clip Line.
{
    float da = *y1;     // Distance plane -> point a.
    float db = y2;      // Distance plane -> point b.
    float d = da-db; if (d==0) { d=1;}
    float s = da / (da-db);     // Intersection factor. (between 0 and 1)
    *x1 = *x1 + s*(x2-(*x1));
    *y1 = *y1 + s*(y2-(*y1)); if (*y1==0) { *y1=1;} // Prevent divide by zero.
    *z1 = *z1 + s*(z2-(*z1));
}

/**
 * @brief Handles drawing the walls of a sector.
 * 
 * @param x1 wall world coordinate
 * @param x2 wall world coordinate
 * @param b1 bottom line wall coordinate
 * @param b2 bottom line wall coordinate
 * @param t1 top line wall coordinate
 * @param t2 top line wall coordinate
 * @param c color
 * @param s sector
 */
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
    if (x2 > SW-1) { x2 = SW - 2;} // Clip Right. 

    // NOTE You have to draw from the first
    // x value to the second x value.
    // Draw x vertical lines.
    for (x = x1; x < x2; x++) {
        // y1 and y2 .
        int y1 = dyb * (x - xs + 0.5) / dx+b1; //y1 bottom point.
        int y2 = dyt * (x - xs + 0.5) / dx+t1; //y2 top point.
        
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

/**
 * @brief Simple distance calculator between points.
 * 
 * @param x1 coordinate for point 1
 * @param y1 coordinate for point 1
 * @param x2 coordinate for point 2
 * @param y2 coordinate for point 2
 * @return int 
 */
int dist(int x1, int y1, int x2, int y2)
{
    int distance = sqrt( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) );
    return distance;
}

/**
 * @brief WIP
 * 
 */
void drawMap2D()
{
    int x,y, xo, yo;
    for(y=0; y<8;y++) {
        for(x=0;x<8;x++) {
            if(map[y*8+x]==1) { glColor3f(1,1,1); } else { glColor3f(0,0,0);}
            xo=x*64; yo=y*64;
            glBegin(GL_QUADS);
            glVertex2i(xo+1,yo+1);
            glVertex2i(xo+1, yo+64-1);
            glVertex2i(xo+64-1, yo+64-1);
            glVertex2i(xo+64-1, yo+1);
            glEnd();
        }
    }

}

/**
 * @brief WIP
 * 
 */
void drawRays2D()
{
    int r, mx, my, mp, dof;
    float rx, ry, ra, xo, yo;

    ra=P.a;
    for(r=0; r<1; r++) {
        //---Check Horizontal Lines---
        dof=0;
        float aTan=-1/tan(ra);
        if (ra > M_PI) { // Looking down
            ry=(((int)P.y>>6)<<6)-0.0001; 
            rx=(P.y-ry)*aTan+P.x;
            yo=-64;
            xo=-yo*aTan;
        }

        if (ra < M_PI) { // Looking up
            ry=(((int)P.y>>6)<<6)+64; 
            rx=(P.y-ry)*aTan+P.x;
            yo=64;
            xo=-yo*aTan;
        }

        if(ra==0 || ra==M_PI) { rx=P.x; ry=P.y; dof=8;} // Looking straight left or right.
        while(dof < 2)
        {
            mx = (int) (rx) >> 6;
            my = (int)(ry)>>6;
            mp=my*8+mx;
            if(mp<8*8 && map[mp]==1) { dof=8;} // hit wall
            else { // next line
                rx+=xo;
                ry+= yo;
                dof+=1;
            }
        }

        glColor3f(0,1,0);
        glLineWidth(1);
        glBegin(GL_LINES);
        glVertex2i(P.x,P.y);
        glVertex2i(rx,ry);
        glEnd();
    }
}

/**
 * @brief WIP (minimap)
 * 
 */
void drawPlayer()
{
    int dx=M.sin[P.a]*10.0;
    int dy=M.cos[P.a]*10.0;
    glColor3f(1,1,0);
    glPointSize(8);
    glBegin(GL_POINTS);
    glVertex2i(P.x, P.y);
    glEnd();

    glLineWidth(3);
    glBegin(GL_LINES);
    glVertex2i(P.x, P.y);
    glVertex2i(P.x+dx*10, P.y+dy*10);
    glEnd();
}

/**
 * @brief Main loop for handling drawing sectors.
 * 
 */
void draw3D()
{
    int s, w, loop, wx[4], wy[4], wz[4]; // x,y,z for the wall.
    float CS=M.cos[P.a], SN=M.sin[P.a]; // Hold cos and sin value based on player rotation.

    // Order sectors by distance;
    for (s=0; s<numSect-1; s++) {
        for (w=0; w<numSect-s-1; w++) {
            if (S[w].d < S[w+1].d) {
                sectors st=S[w]; 
                S[w]=S[w+1]; 
                S[w+1]=st;
            }
        }
    }

    // Draw Sectors.
    for (s=0;s<numSect;s++) {

        S[s].d=0;   // Clear distance.

         if (P.z<S[s].z1) { S[s].surface=1;}     // if player is below sectors floor value -> Bottom surface.
    else if (P.z>S[s].z2) { S[s].surface=2;}     // if player is on top of sectors ceiling value -> Top surface.
    else                  { S[s].surface=0;}     // if inbetween -> dont draw any surface.

        for (loop=0; loop<2; loop++) { // Once for the back walls and once again for the front walls. 
            for (w=S[s].ws; w<S[s].we; w++) { 
                // Offset bottom 2 points by player.
                int x1=W[w].x1-P.x, y1=W[w].y1-P.y;
                int x2=W[w].x2-P.x, y2=W[w].y2-P.y;

                // First draw the flipped faces. (back walls)
                if (loop == 0) {
                    int swp=x1; x1=x2; x2=swp; swp=y1; y1=y2; y2=swp;
                }

                /* --------------------World Position--------------------------- */
                // World X position.
                wx[0] = x1 * CS - y1 * SN;
                wx[1] = x2 * CS - y2 * SN;
                wx[2] = wx[0]; // Top line has the same x.
                wx[3] = wx[1];
                
                // World Y position. (depth)
                wy[0] = y1 * CS + x1 * SN;
                wy[1] = y2 * CS + x2 * SN;
                wy[2] = wy[0]; // Top line has the same y.
                wy[3] = wy[1];
                S[s].d+=dist(0,0, (wx[0] + wx[1])/2, (wy[0] + wy[1])/2 );   // Store this wall distance.
                W[w].d =dist(P.x,P.y, wx[0], wy[0]); 

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


                drawWall(wx[0], wx[1], wy[0], wy[1], wy[2], wy[3], W[w].c, s);
            }

            S[s].d/=(S[s].we-S[s].ws);  // Find average sector distance.
            S[s].surface*=-1;           // Flip to negative to draw surface.
        }
    }
}

/**
 * @brief Main loop for handling draw UI elements.
 * 
 */
void drawUI()
{

    char player_x[50]; sprintf(player_x, "%d", P.x);
    char player_y[50]; sprintf(player_y, "%d", P.y);
    char player_z[50]; sprintf(player_z, "%d", P.z);
    char player_a[50]; sprintf(player_a, "%d", P.a);
    char player_l[50]; sprintf(player_l, "%d", P.l);
    char ui_player_x[100] = "P.x: ";
    char ui_player_y[100] = "P.y: ";
    char ui_player_z[100] = "P.z: ";
    char ui_player_a[100] = "P.a: ";
    char ui_player_l[100] = "P.l: ";
    strcat(ui_player_x, player_x);
    strcat(ui_player_y, player_y);
    strcat(ui_player_z, player_z);
    strcat(ui_player_a, player_a);
    strcat(ui_player_l, player_l);

    drawString(5,  SH-15, 0, ui_player_x);
    drawString(80, SH-15, 0, ui_player_y);
    drawString(160, SH-15, 0, ui_player_z);
    drawString(240, SH-15, 0, ui_player_a);
    drawString(320, SH-15, 0, ui_player_l);

}

/**
 * @brief Main openGL loop that handles 
 * rendering all the components at a fixed fps
 * 
 */
void display(void)
{
    int x, y;

    if ((T.fr1 - T.fr2) >= 50) {               // Only draw 20 frames/second
        clearBackground();
        movePlayer();
        draw3D(); // NOTE anything after this need a small delay because it messed with variables.
        // drawMap2D();
        // drawPlayer();
        // drawRays2D();

        T.fr2 = T.fr1;
        drawUI();
        glutSwapBuffers();
        glutReshapeWindow(GLSW, GLSH);          // Prevent window scaling.
    }

    T.fr1 = glutGet(GLUT_ELAPSED_TIME);         // 1000 Milliseconds per second.
    glutPostRedisplay();
}

/**
 * @brief Handles key down input states.
 * 
 * @param key that was pressed
 * @param x 
 * @param y 
 */
void KeysDown(unsigned char key, int x, int y)
{
    if ((key == 'w') == 1) { K.w = 1; }
    if ((key == 's') == 1) { K.s = 1; }
    if ((key == 'a') == 1) { K.a = 1; }
    if ((key == 'd') == 1) { K.d = 1; }
    if ((key == 'm') == 1) { K.m = 1; }
    if ((key == ',') == 1) { K.sr = 1; }
    if ((key == '.') == 1) { K.sl = 1; }
    if ((key == 27) == 1) { K.escape = 1; printf("Exitting...\n"); closeGame();}
}

/**
 * @brief Handles key up input state.
 * 
 * @param key that was pressed
 * @param x 
 * @param y 
 */
void KeysUp(unsigned char key, int x, int y)
{
    if (key == 'w' == 1) { K.w = 0; }
    if (key == 's' == 1) { K.s = 0; }
    if (key == 'a' == 1) { K.a = 0; }
    if (key == 'd' == 1) { K.d = 0; }
    if (key == 'm' == 1) { K.m = 0; }
    if (key == ',' == 1) { K.sr = 0; }
    if (key == '.' == 1) { K.sl = 0; }
    if (key == 27 == 1) { K.escape = 0;}
}

/**
 * @brief Initializes the map configurations.
 * 
 */
void mapInit() {

    mapX.boundsN = -300; mapX.boundsP = 300;
    mapY.boundsN = -300; mapY.boundsP = 300;
    mapZ.boundsN = -300; mapZ.boundsP = 300;    
}

/**
 * @brief Initializes game configurations.
 * 
 */
void init()
{
    printf("***     OpenGL Version: %s      ***", glGetString(GL_VERSION));

    mapInit(); //Initializing the doom_level.

    // Precalculating sin and cos in degrees and storing the values.
    for (int x=0; x<360; x++) 
    {
        M.cos[x]=cos(x/180.0*M_PI);
        M.sin[x]=sin(x/180.0*M_PI);
    }

    // Init player position (aka map position) variables
    P.x=70; P.y=-110; P.z=20; P.a=0; P.l=0, P.withinBounds=1;

    // Load sectors
    int s, w, v1=0, v2=0;
    for (s=0; s<numSect; s++) {
        S[s].ws=loadSectors[v1+0];                      // Wall start number.
        S[s].we=loadSectors[v1+1];                      // Wall end number.
        S[s].z1=loadSectors[v1+2];                      // Sector bottom height.
        S[s].z2=loadSectors[v1+3] - loadSectors[v1+2];  // Sector top height.
        S[s].c1=loadSectors[v1+4];                      // Sector top color.
        S[s].c2=loadSectors[v1+5];                      // Secot bottom color.
        v1 += 6; // Go to the next sector

        // Laod Wall values.
        for (w=S[s].ws; w<S[s].we; w++) {
            W[w].x1=loadWalls[v2+0];    // Bottom x1
            W[w].y1=loadWalls[v2+1];    // Bottom y1
            W[w].x2=loadWalls[v2+2];    // Top    x2
            W[w].y2=loadWalls[v2+3];    // Top    y2
            W[w].c =loadWalls[v2+4];    // Wall color
            v2+=5; // Go to the next wall.
        }
    }

    // Reset input key values.
    K.a = 0;
    K.s = 0;
    K.d = 0;
    K.w = 0;
    K.m = 0;
    K.sl = 0;
    K.sr = 0;
    K.escape = 0;
}

/**
 * @brief Handles windowing and starting loops.
 * 
 * @param argc arg count
 * @param argv arg 
 * @return int 
 */
int main(int argc, char* argv[])
{
    glutInit(&argc, argv); // Init with arguments.
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(GLSW/2, GLSH/2);
    glutInitWindowSize(GLSW, GLSH);
    window = glutCreateWindow("Doom OpenGL Game Engine");
    glPointSize(pixelScale); // Pixel Size.
    gluOrtho2D(0, GLSW, 0, GLSH); // Origin Bottom Left.

    init();
    printf("\nGame Engine Running...\n");

    glutDisplayFunc(display);
    glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
    glutKeyboardFunc(KeysDown);
    glutKeyboardUpFunc(KeysUp);
    glutMouseFunc(mouseHandler);
    glutMainLoop();

    return 0;
}