#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "matrix.h"
#include "gmath.h"

int compare_yval (const void * p0, const void * p1) {
  double f = ((double*)p0)[1] - ((double*)p1)[1];
  return f ? f>0 ? 1 : -1 : 0;
}

void scanline_convert( struct matrix *points, int i, screen s, zbuffer zb, color c ) {
  int y, distance0, distance1, distance2, flip = 0;
  double x0, x1, dx0, dx1, z0, z1, dz0, dz1;
  double p[3][3] = {
    {points->m[0][i], points->m[1][i], points->m[2][i]},
    {points->m[0][i+1], points->m[1][i+1], points->m[2][i+1]},
    {points->m[0][i+2], points->m[1][i+2], points->m[2][i+2]}
  };
  qsort(p, 3, sizeof p[0], compare_yval);
  x0 = x1 = p[0][0];
  z0 = z1 = p[0][2];
  y = (int)(p[0][1]);
  distance0 = (int)(p[2][1]) - y;
  distance1 = (int)(p[1][1]) - y;
  distance2 = (int)(p[2][1]) - (int)(p[1][1]);
  dx0 = distance0 > 0 ? (p[2][0]-p[0][0])/distance0 : 0;
  dx1 = distance1 > 0 ? (p[1][0]-p[0][0])/distance1 : 0;
  dz0 = distance0 > 0 ? (p[2][2]-p[0][2])/distance0 : 0;
  dz1 = distance1 > 0 ? (p[1][2]-p[0][2])/distance1 : 0;
  while ( y <= (int)p[2][1] ) {
    draw_line(x0, y, z0, x1, y, z1, s, zb, c);
    x0+= dx0;
    x1+= dx1;
    z0+= dz0;
    z1+= dz1;
    y++;
    if ( !flip && y >= (int)(p[1][1]) ) {
      flip = 1;
      dx1 = distance2 > 0 ? (p[2][0]-p[1][0])/distance2 : 0;
      dz1 = distance2 > 0 ? (p[2][2]-p[1][2])/distance2 : 0;
      x1 = p[1][0];
      z1 = p[1][2];
    }
  }
}

void add_polygon( struct matrix *polygons, double x0, double y0, double z0, double x1, double y1, double z1,  double x2, double y2, double z2 ) {
  add_point(polygons, x0, y0, z0);
  add_point(polygons, x1, y1, z1);
  add_point(polygons, x2, y2, z2);
}

void draw_polygons(struct matrix *polygons, screen s, zbuffer zb, double *view, double light[2][3], color ambient, double *areflect, double *dreflect, double *sreflect) {
  if ( polygons->lastcol < 3 ) return; //not enough points
  int point;
  double *normal;
  for (point=0; point < polygons->lastcol-2; point+=3) {
    normal = calculate_normal(polygons, point);
    if ( dot_product(normal, view) > 0 ) {
      color c = get_lighting(normal, view, ambient, light, areflect, dreflect, sreflect);
      scanline_convert( polygons, point, s, zb, c);
      draw_line( polygons->m[0][point], polygons->m[1][point], polygons->m[2][point], polygons->m[0][point+1], polygons->m[1][point+1], polygons->m[2][point+1], s, zb, c);
      draw_line( polygons->m[0][point+2], polygons->m[1][point+2], polygons->m[2][point+2], polygons->m[0][point+1], polygons->m[1][point+1], polygons->m[2][point+1], s, zb, c);
      draw_line( polygons->m[0][point], polygons->m[1][point], polygons->m[2][point], polygons->m[0][point+2], polygons->m[1][point+2], polygons->m[2][point+2], s, zb, c);
    }
    free(normal);
  }
}

void add_box( struct matrix * polygons, double x, double y, double z, double width, double height, double depth ) {
  double x1 = x+width;
  double y1 = y-height;
  double z1 = z-depth;
  add_polygon(polygons, x, y, z, x1, y1, z, x1, y, z); //front
  add_polygon(polygons, x, y, z, x, y1, z, x1, y1, z);
  add_polygon(polygons, x1, y, z1, x, y1, z1, x, y, z1); //back
  add_polygon(polygons, x1, y, z1, x1, y1, z1, x, y1, z1);
  add_polygon(polygons, x1, y, z, x1, y1, z1, x1, y, z1); //right side
  add_polygon(polygons, x1, y, z, x1, y1, z, x1, y1, z1);
  add_polygon(polygons, x, y, z1, x, y1, z, x, y, z); //left side
  add_polygon(polygons, x, y, z1, x, y1, z1, x, y1, z);
  add_polygon(polygons, x, y, z1, x1, y, z, x1, y, z1); //top
  add_polygon(polygons, x, y, z1, x, y, z, x1, y, z);
  add_polygon(polygons, x, y1, z, x1, y1, z1, x1, y1, z); //bottom
  add_polygon(polygons, x, y1, z, x, y1, z1, x1, y1, z1);
}

void add_sphere( struct matrix * edges, double cx, double cy, double cz, double r, int step ) {
  struct matrix * points = generate_sphere(cx, cy, cz, r, step);
  int i, j, p0, p1, p2, p3;
  for(i=0; i<step; i++) {
    for(j=0; j<step; j++) {
      p0 = i * (step+1) + j;
      p1 = p0+1;
      p2 = (p1+step+1) % points->lastcol;
      p3 = (p0+step+1) % points->lastcol;
      if (j < step-1) add_polygon( edges, points->m[0][p0], points->m[1][p0], points->m[2][p0], points->m[0][p1], points->m[1][p1], points->m[2][p1], points->m[0][p2], points->m[1][p2], points->m[2][p2]);
      if (j) add_polygon( edges, points->m[0][p0], points->m[1][p0], points->m[2][p0], points->m[0][p2], points->m[1][p2], points->m[2][p2], points->m[0][p3], points->m[1][p3], points->m[2][p3]);
    }
  }
  free_matrix(points);
}

struct matrix * generate_sphere(double cx, double cy, double cz, double r, int step ) {
  struct matrix * points = new_matrix(4, step*step);
  int i, j;
  double s, t;
  for(i=0; i<step; i++) {
    s = (double)i/step;
    for(j=0; j<=step; j++) {
      t = (double)j/step;
      add_point(points, r*cos(M_PI*t)+cx, r*sin(M_PI*t)*cos(2*M_PI*s)+cy, r*sin(M_PI*t)*sin(2*M_PI*s)+cz);
    }
  }
  return points;
}

void add_torus( struct matrix * edges, double cx, double cy, double cz, double r1, double r2, int step ) {
  struct matrix *points = generate_torus(cx, cy, cz, r1, r2, step);
  int i, j, p0, p1, p2, p3;
  for ( i = 0; i < step; i++ ) {
    for ( j = 0; j < step; j++ ) {
      p0 = i * step + j;
      p1 = (j == step-1) ? p0-j: p0+1;
      p2 = (p1+step) % points->lastcol;
      p3 = (p0+step) % points->lastcol;
      add_polygon( edges, points->m[0][p0], points->m[1][p0], points->m[2][p0], points->m[0][p3], points->m[1][p3], points->m[2][p3], points->m[0][p2], points->m[1][p2], points->m[2][p2]);
      add_polygon( edges, points->m[0][p0], points->m[1][p0], points->m[2][p0], points->m[0][p2], points->m[1][p2], points->m[2][p2], points->m[0][p1], points->m[1][p1], points->m[2][p1]);
    }
  }
  free_matrix(points);
}

struct matrix * generate_torus( double cx, double cy, double cz, double r1, double r2, int step ) {
  struct matrix * points = new_matrix(4, step*step);
  int i, j;
  double s, t;
  for(i=0; i<step; i++) {
    s = (double)i/step;
    for(j=0; j<step; j++) {
      t = (double)j/step;
      add_point(points, cos(2*M_PI*s)*(r1*cos(2*M_PI*t)+r2)+cx, r1*sin(2*M_PI*t)+cy, -1*sin(2*M_PI*s)*(r1*cos(2*M_PI*t)+r2)+cz);
    }
  }
  return points;
}

void add_circle( struct matrix * edges, double cx, double cy, double cz, double r, int step ) {
  double x0, y0, x1, y1, t;
  int i;
  x0 = r + cx;
  y0 = cy;
  for (i=1; i<=step; i++) {
    t = (double)i/step;
    x1 = r * cos(2 * M_PI * t) + cx;
    y1 = r * sin(2 * M_PI * t) + cy;
    add_edge(edges, x0, y0, cz, x1, y1, cz);
    x0 = x1;
    y0 = y1;
  }
}

void add_curve( struct matrix *edges, double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3, int step, int type ) {
  int i;
  double t;
  struct matrix *x = generate_curve_coefs(x0, x1, x2, x3, type);
  struct matrix *y = generate_curve_coefs(y0, y1, y2, y3, type);
  for (i=1; i<=step; i++) {
    t = (double)i/step;
    x1 = x->m[0][0]*pow(t,3) + x->m[1][0]*pow(t,2) + x->m[2][0]*t + x->m[3][0];
    y1 = y->m[0][0]*pow(t,3) + y->m[1][0]*pow(t,2) + y->m[2][0]*t + y->m[3][0];
    add_edge(edges, x0, y0, 0, x1, y1, 0);
    x0 = x1;
    y0 = y1;
  }
  free_matrix(x);
  free_matrix(y);
}

void add_point( struct matrix * points, double x, double y, double z) {
  if ( points->lastcol == points->cols ) grow_matrix( points, points->lastcol + 1 );
  points->m[0][ points->lastcol ] = x;
  points->m[1][ points->lastcol ] = y;
  points->m[2][ points->lastcol ] = z;
  points->m[3][ points->lastcol ] = 1;
  points->lastcol++;
}

void add_edge( struct matrix * points, double x0, double y0, double z0, double x1, double y1, double z1) {
  add_point( points, x0, y0, z0 );
  add_point( points, x1, y1, z1 );
}

void draw_lines( struct matrix * points, screen s, zbuffer zb, color c) {
  if ( points->lastcol < 2 ) return; //not enough points
  int point;
  for (point=0; point < points->lastcol-1; point+=2) draw_line( points->m[0][point], points->m[1][point], points->m[2][point], points->m[0][point+1], points->m[1][point+1], points->m[2][point+1], s, zb, c);
}

void draw_line(int x0, int y0, double z0, int x1, int y1, double z1, screen s, zbuffer zb, color c) {
  if ( x0 > x1 ) return draw_line(x1, y1, z1, x0, y0, z0, s, zb, c); // swap coordinates if necessary
  int d, A, B, dy_east, dy_northeast, dx_east, dx_northeast, d_east, d_northeast, loop_start, loop_end, wide=0, tall=0;
  double distance, dz;
  A = 2 * (y1 - y0);
  B = -2 * (x1 - x0);
  if ( abs(x1 - x0) >= abs(y1 - y0) ) { //octant 1/8
    wide = 1;
    loop_start = x0;
    loop_end = x1;
    dx_east = dx_northeast = 1;
    dy_east = 0;
    d_east = A;
    if ( A > 0 ) { //octant 1
      d = A + B/2;
      dy_northeast = 1;
      d_northeast = A + B;
    }
    else { //octant 8
      d = A - B/2;
      dy_northeast = -1;
      d_northeast = A - B;
    }
  }//end octant 1/8
  else { //octant 2/7
    tall = 1;
    dx_east = 0;
    dx_northeast = 1;
    if ( A > 0 ) {     //octant 2
      d = A/2 + B;
      dy_east = dy_northeast = 1;
      d_northeast = A + B;
      d_east = B;
      loop_start = y0;
      loop_end = y1;
    }
    else {     //octant 7
      d = A/2 - B;
      dy_east = dy_northeast = -1;
      d_northeast = A - B;
      d_east = -1 * B;
      loop_start = y1;
      loop_end = y0;
    }
  }
  distance = abs(loop_end - loop_start);
  dz = distance > 0 ? (z1 - z0) / distance : 0;
  while ( loop_start++ < loop_end ) {
    plot( s, zb, c, x0, y0, z0);
    if ((wide && ((A > 0 && d > 0) || (A < 0 && d < 0))) || (tall && ((A > 0 && d < 0 ) || (A < 0 && d > 0) ))) {
      x0+=dx_northeast;
      y0+=dy_northeast;
      d+=d_northeast;
    }
    else {
      x0+=dx_east;
      y0+=dy_east;
      d+=d_east;
    }
    z0+=dz;
  } //end drawing loop
  plot( s, zb, c, x1, y1, z0 );
} //end draw_line
