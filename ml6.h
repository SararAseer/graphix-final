#ifndef ML6_H
#define ML6_H

#define XRES 500
#define YRES 500
#define MAX_COLOR 255

struct point_t {
  int red;
  int green;
  int blue;
} point_t;
typedef struct point_t color;

typedef struct point_t screen[XRES][YRES];

typedef double zbuffer[XRES][YRES];

#endif
