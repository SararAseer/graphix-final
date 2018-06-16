#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gmath.h"
#include "matrix.h"
#include "ml6.h"

#define  Pr  .299
#define  Pg  .587
#define  Pb  .114
#define YOTE 0.0

//lighting functions
color get_lighting( double *normal, double *view, color alight, double light[2][3], double *areflect, double *dreflect, double *sreflect, double change) {
  color a, d, s, i;
  normalize(normal);
  normalize(view);
  normalize(light[LOCATION]);
  a = calculate_ambient(alight, areflect);
  d = calculate_diffuse(light, dreflect, normal);
  s = calculate_specular(light, sreflect, view, normal);
  i.red = a.red + d.red + s.red;
  i.green = a.green + d.green + s.green;
  i.blue = a.blue + d.blue + s.blue;
  limit_color(&i);
  
  double P = sqrt(i.red * i.red * Pr + i.green * i.green * Pg + i.blue * i.blue * Pb);
  
  i.red = P + (i.red - P) * change;
  i.green = P + (i.green - P) * change;
  i.blue = P + (i.blue - P) * change;
  
  return i;  
}

color calculate_ambient(color alight, double *areflect ) {
  alight.red *= areflect[RED];
  alight.green *= areflect[GREEN];
  alight.blue *= areflect[BLUE];
  return alight;
}

color calculate_diffuse(double light[2][3], double *dreflect, double *normal ) {
  color d;
  double dot = dot_product(normal, light[LOCATION]);
  d.red = light[COLOR][RED] * dreflect[RED] * dot;
  d.green = light[COLOR][GREEN] * dreflect[GREEN] * dot;
  d.blue = light[COLOR][BLUE] * dreflect[BLUE] * dot;
  return d;
}

color calculate_specular(double light[2][3], double *sreflect, double *view, double *normal ) {
  color s;
  double n[3];
  double dot = 2 * dot_product(normal, light[LOCATION]);
  n[0] = (normal[0] * dot) - light[LOCATION][0];
  n[1] = (normal[1] * dot) - light[LOCATION][1];
  n[2] = (normal[2] * dot) - light[LOCATION][2];
  dot = dot_product(n, view);
  dot = dot > 0 ? pow(dot , SPECULAR_EXP) : 0;
  s.red = light[COLOR][RED] * sreflect[RED] * dot;
  s.green = light[COLOR][GREEN] * sreflect[GREEN] * dot;
  s.blue = light[COLOR][BLUE] * sreflect[BLUE] * dot;
  return s;
}

void limit_color( color * c ) {
  c->red = c->red > 255 ? 255 : c->red < 0 ? 0 : c->red;
  c->green = c->green > 255 ? 255 : c->green < 0 ? 0 : c->green;
  c->blue = c->blue > 255 ? 255 : c->blue < 0 ? 0 : c->blue;
}

//vector functions
void normalize( double *vector ) {
  double mag = sqrt( pow(vector[0], 2) + pow(vector[1], 2) + pow(vector[2], 2) );
  int i;
  for (i=0; i<3; i++) vector[i] /= mag;
}

double dot_product( double *a, double *b ) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

double *calculate_normal(struct matrix *polygons, int i) {
  double A[3], B[3], *N = (double *)malloc(3 * sizeof(double));
  A[0] = polygons->m[0][i+1] - polygons->m[0][i];
  A[1] = polygons->m[1][i+1] - polygons->m[1][i];
  A[2] = polygons->m[2][i+1] - polygons->m[2][i];
  B[0] = polygons->m[0][i+2] - polygons->m[0][i];
  B[1] = polygons->m[1][i+2] - polygons->m[1][i];
  B[2] = polygons->m[2][i+2] - polygons->m[2][i];
  N[0] = A[1] * B[2] - A[2] * B[1];
  N[1] = A[2] * B[0] - A[0] * B[2];
  N[2] = A[0] * B[1] - A[1] * B[0];
  return N;
}
