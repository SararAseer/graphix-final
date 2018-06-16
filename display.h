#include "ml6.h"

#ifndef DISPLAY_H
#define DISPLAY_H

void plot( screen s, zbuffer zb, color c, int x, int y, double z);
screen * new_screen();
void clear_screen( screen s);
zbuffer * new_zbuffer();
void clear_zbuffer( zbuffer zb );
void save_ppm( screen s, char *file);
void save_extension( screen s, char *file);
void display( screen s);
void make_animation( char * name );

#endif
