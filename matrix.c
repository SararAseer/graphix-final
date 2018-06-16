#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "matrix.h"

struct matrix * make_bezier() {
  struct matrix *t = new_matrix(4, 4);
  t->lastcol = 4;
  t->m[0][0] = -1;
  t->m[0][1] = 3;
  t->m[0][2] = -3;
  t->m[0][3] = 1;
  t->m[1][0] = 3;
  t->m[1][1] = -6;
  t->m[1][2] = 3;
  t->m[1][3] = 0;
  t->m[2][0] = -3;
  t->m[2][1] = 3;
  t->m[2][2] = 0;
  t->m[2][3] = 0;
  t->m[3][0] = 1;
  t->m[3][1] = 0;
  t->m[3][2] = 0;
  t->m[3][3] = 0;
  return t;
}

struct matrix * make_hermite() {
  struct matrix *t = new_matrix(4, 4);
  t->lastcol = 4;
  t->m[0][0] = 2;
  t->m[0][1] = -2;
  t->m[0][2] = 1;
  t->m[0][3] = 1;
  t->m[1][0] = -3;
  t->m[1][1] = 3;
  t->m[1][2] = -2;
  t->m[1][3] = -1;
  t->m[2][0] = 0;
  t->m[2][1] = 0;
  t->m[2][2] = 1;
  t->m[2][3] = 0;
  t->m[3][0] = 1;
  t->m[3][1] = 0;
  t->m[3][2] = 0;
  t->m[3][3] = 0;
  return t;
}

struct matrix * generate_curve_coefs( double p1, double p2, double p3, double p4, int type) {
  struct matrix *curve;
  struct matrix *t = new_matrix(4, 1);
  t->lastcol = 1;
  t->m[0][0] = p1;
  t->m[1][0] = p2;
  t->m[2][0] = p3;
  t->m[3][0] = p4;
  curve = type ? make_bezier() : make_hermite();
  matrix_mult(curve, t);
  free_matrix(curve);
  return t;
}

struct matrix * make_translate(double x, double y, double z) {
  struct matrix *t = new_matrix(4, 4);
  ident(t);
  t->m[0][3] = x;
  t->m[1][3] = y;
  t->m[2][3] = z;
  return t;
}

struct matrix * make_scale(double x, double y, double z) {
  struct matrix *t = new_matrix(4, 4);
  ident(t);
  t->m[0][0] = x;
  t->m[1][1] = y;
  t->m[2][2] = z;
  return t;
}

struct matrix * make_rotX(double theta) {
  struct matrix *t = new_matrix(4, 4);
  ident(t);
  t->m[1][1] = cos(theta);
  t->m[1][2] = -1*sin(theta);
  t->m[2][1] = sin(theta);
  t->m[2][2] = cos(theta);
  return t;
}

struct matrix * make_rotY(double theta) {
  struct matrix *t = new_matrix(4, 4);
  ident(t);
  t->m[0][0] = cos(theta);
  t->m[2][0] = -1*sin(theta);
  t->m[0][2] = sin(theta);
  t->m[2][2] = cos(theta);
  return t;
}

struct matrix * make_rotZ(double theta) {
  struct matrix *t = new_matrix(4, 4);
  ident(t);
  t->m[0][0] = cos(theta);
  t->m[0][1] = -1 * sin(theta);
  t->m[1][0] = sin(theta);
  t->m[1][1] = cos(theta);
  return t;
}

void print_matrix(struct matrix *m) {
  int i, j;
  for (i = 0; i < m->rows; i++) {
    for (j = 0; j < m->lastcol; j++) printf("%0.2f\t", m->m[i][j]);
    printf("\n");
  }
}//end print_matrix

void ident(struct matrix *m) {
  int i, j;
  for (i = 0; i < m->rows; i++) for (j = 0; j < m->cols; j++) m -> m[i][j] = (i == j) ? 1 : 0;
  m->lastcol = m->cols;
}//end ident


void scalar_mult(double x, struct matrix *m) {
  int r, c;
  for (r=0; r < m->rows; r++) for (c=0; c < m->lastcol; c++) m->m[r][c] *= x;
}//end scalar_mult

void matrix_mult(struct matrix *a, struct matrix *b) {
  int i, j;
  struct matrix *tmp;
  tmp = new_matrix(4, 1);
  for (j=0; j < b->lastcol; j++) {
    for (i=0; i < b->rows; i++) tmp->m[i][0] = b->m[i][j]; //copy cur point
    for (i=0; i < b->rows; i++)
      b->m[i][j] = a->m[i][0] * tmp->m[0][0] + a->m[i][1] * tmp->m[1][0] + a->m[i][2] * tmp->m[2][0] + a->m[i][3] * tmp->m[3][0];
  }
  free_matrix(tmp);
}//end matrix_mult

struct matrix *new_matrix(int rows, int cols) {
  double **tmp;
  int i;
  struct matrix *m;
  tmp = (double **)malloc(rows * sizeof(double *));
  for (i=0;i<rows;i++) tmp[i]=(double *)malloc(cols * sizeof(double));
  m=(struct matrix *)malloc(sizeof(struct matrix));
  m->m=tmp;
  m->rows = rows;
  m->cols = cols;
  m->lastcol = 0;
  return m;
}

void free_matrix(struct matrix *m) {
  int i;
  for (i=0;i<m->rows;i++) free(m->m[i]);
  free(m->m);
  free(m);
}

void grow_matrix(struct matrix *m, int newcols) {
  int i;
  for (i=0;i<m->rows;i++) m->m[i] = realloc(m->m[i],newcols*sizeof(double));
  m->cols = newcols;
}

void copy_matrix(struct matrix *a, struct matrix *b) {
  int r, c;
  for (r=0; r < a->rows; r++) for (c=0; c < a->cols; c++) b->m[r][c] = a->m[r][c];
}
