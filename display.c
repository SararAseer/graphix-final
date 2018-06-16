#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include "ml6.h"
#include "display.h"

void plot(screen s, zbuffer zb, color c, int x, int y, double z) {
  y = YRES - 1 - y;
  z = (int)(z * 1000) / 1000;
  if ( x >= 0 && x < XRES && y >=0 && y < YRES && zb[x][y] <= z ) {
    s[x][y] = c;
    zb[x][y] = z;
  }
}

screen * new_screen() {
  return calloc(XRES * YRES, sizeof(color));
}

void clear_screen( screen s ) {
  int x, y;
  color c;
  c.red = 255;
  c.green = 255;
  c.blue = 255;
  for ( y=0; y < YRES; y++ ) for ( x=0; x < XRES; x++) s[x][y] = c;
}

zbuffer * new_zbuffer() {
  return calloc(XRES * YRES, sizeof(double));
}

void clear_zbuffer( zbuffer zb ) {
  int x, y;
  for ( y=0; y < YRES; y++ ) for ( x=0; x < XRES; x++) zb[x][y] = LONG_MIN;
}

void save_ppm( screen s, char *file) {
  int x, y;
  FILE *f;
  f = fopen(file, "w");
  fprintf(f, "P3\n%d %d\n%d\n", XRES, YRES, MAX_COLOR);
  for ( y=0; y < YRES; y++ ) {
    for ( x=0; x < XRES; x++)
      fprintf(f, "%d %d %d ", s[x][y].red, s[x][y].green, s[x][y].blue);
    fprintf(f, "\n");
  }
  fclose(f);
}

void save_extension( screen s, char *file) {
  int x, y;
  FILE *f;
  char line[256];
  sprintf(line, "convert - %s", file);
  f = popen(line, "w");
  fprintf(f, "P3\n%d %d\n%d\n", XRES, YRES, MAX_COLOR);
  for ( y=0; y < YRES; y++ ) {
    for ( x=0; x < XRES; x++)
      fprintf(f, "%d %d %d ", s[x][y].red, s[x][y].green, s[x][y].blue);
    fprintf(f, "\n");
  }
  pclose(f);
}

void display( screen s) {
  int x, y;
  FILE *f;
  f = popen("display", "w");
  fprintf(f, "P3\n%d %d\n%d\n", XRES, YRES, MAX_COLOR);
  for ( y=0; y < YRES; y++ ) {
    for ( x=0; x < XRES; x++)
      fprintf(f, "%d %d %d ", s[x][y].red, s[x][y].green, s[x][y].blue);
    fprintf(f, "\n");
  }
  pclose(f);
}

void make_animation( char * name ) {
  int f;
  char name_arg[128];
  sprintf(name_arg, "anim/%s*", name);
  strncat(name, ".gif", 128);
  printf("Making animation: %s\n", name);
  f = fork();
  if (f == 0) {
    if (execlp("convert", "convert", "-delay", "3", name_arg, name, NULL) == -1) {
      printf("errno: %d: %s\n", errno, strerror(errno));
    }
  }
  wait(0);
}
