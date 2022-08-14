#include "../include/doom_level.h"

//---doom level---
int loadSectors[]=
{ // wall number start, wall number end, height of bottom and top, color of bottom and top.
0,8, 40,50, 9,9,
8,16, 0,40, 6,6,
16,24, 80,110, 0,0,
24,28, 0,30, 6,6,
28,32, 0,20, 6,6,
32,36, 0,10, 6,6,
36,40, 0,30, 5,5,
40,44, 0,30, 5,5,
44,48, 30,110, 0,0,
48,52, 30,110, 0,0,
};

int loadWalls[]=
{ // bottomline point (x1, y1), bottom line point (x2, y2), wall color
160,228, 168,228, 4,
168,228, 176,236, 5,
176,236, 176,244, 4,
176,244, 168,252, 5,
168,252, 160,252, 4,
160,252, 152,244, 5,
152,244, 152,236, 4,
152,236, 160,228, 5,
104,224, 152,184, 1,
152,184, 176,184, 3,
176,184, 224,224, 1,
224,224, 224,256, 0,
224,256, 192,288, 1,
192,288, 136,288, 0,
136,288, 104,256, 1,
104,256, 104,224, 0,
104,224, 152,184, 1,
152,184, 176,184, 0,
176,184, 224,224, 1,
224,224, 224,256, 0,
224,256, 192,288, 1,
192,288, 136,288, 0,
136,288, 104,256, 1,
104,256, 104,224, 0,
152,168, 176,168, 2,
176,168, 176,184, 3,
176,184, 152,184, 2,
152,184, 152,168, 3,
152,152, 176,152, 2,
176,152, 176,168, 3,
176,168, 152,168, 2,
152,168, 152,152, 3,
152,136, 176,136, 2,
176,136, 176,152, 3,
176,152, 152,152, 2,
152,152, 152,136, 3,
208,160, 208,136, 5,
208,136, 232,136, 4,
232,136, 232,160, 5,
232,160, 208,160, 4,
96,136, 120,136, 4,
120,136, 120,160, 5,
120,160, 96,160, 4,
96,160, 96,136, 5,
216,144, 224,144, 4,
224,144, 224,152, 5,
224,152, 216,152, 4,
216,152, 216,144, 5,
104,144, 112,144, 4,
112,144, 112,152, 5,
112,152, 104,152, 4,
104,152, 104,144, 5,
};