#ifndef DOOM_LEVEL_H
#define DOOM_LEVEL_H

typedef struct
{
    int boundsP, boundsN;
} boundsX; boundsX mapX;

typedef struct
{
    int boundsP, boundsN;
} boundsY; boundsY mapY;

typedef struct
{
    int boundsP, boundsN;
} boundsZ; boundsZ mapZ;

extern int loadSectors[];
extern int loadWalls[];

#endif