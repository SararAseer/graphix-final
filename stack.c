#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"
#include "stack.h"

struct stack * new_stack() {
  struct stack *s = (struct stack *)malloc(sizeof(struct stack));
  struct matrix **m = (struct matrix **)malloc( STACK_SIZE * sizeof(struct matrix *));
  struct matrix *i = new_matrix(4, 4);
  ident( i );
  s->size = STACK_SIZE;
  s->top = 0;
  s->data = m;
  s->data[ s->top ] = i;
  return s;
}

void push( struct stack *s ) {
  struct matrix *m = new_matrix(4, 4);
  if ( s->top == s->size - 1 ) {
    s->data = (struct matrix **)realloc( s->data, (s->size + STACK_SIZE) * sizeof(struct matrix *));
    s->size = s->size + STACK_SIZE;
  }
  copy_matrix(s->data[s->top], m);
  s->top++;
  s->data[s->top] = m;
}

struct matrix * peek( struct stack *s ) {
  return s->data[s->top];
}

void pop( struct stack * s) {
  free_matrix(s->data[s->top]);
  s->top--;
}

void free_stack( struct stack *s) {
  int i;
  for (i=0; i <= s->top; i++) free_matrix(s->data[i]);
  free(s->data);
  free(s);
}

void print_stack(struct stack *s) {
  int i;
  for (i=s->top; i >= 0; i--) {
    print_matrix(s->data[i]);
    printf("\n");
  }
}
