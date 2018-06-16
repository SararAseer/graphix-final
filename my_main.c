#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include "parser.h"
#include "symtab.h"
#include "y.tab.h"

#include "matrix.h"
#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "stack.h"
#include "thpool.h"

void first_pass() {
  int i;
  char vary_check = 0, name_check = 0, frame_check = 0;
  extern int num_frames;
  extern char name[128];
  num_frames = 1;
  for (i=0;i<lastop;i++) {
    if (op[i].opcode == FRAMES) {
      num_frames = op[i].op.frames.num_frames;
      frame_check = 1;
      struct stat sb;
      if (stat("anim/", &sb) == -1) {
        mkdir("anim", 0755);
      }
    } else if (op[i].opcode == BASENAME) {
      strncpy( name, op[i].op.basename.p->name, sizeof( name ) );
      name_check = 1;
    } else if (op[i].opcode == VARY) {
      vary_check = 1;
    }
    if (vary_check && !frame_check) {
      printf("Error: Vary command found but number of frames not set\n");
      exit(-1);
    }
    if (frame_check && vary_check && !name_check) {
      printf("Warning: Animation code present but basename not set. Using \"frame\" as basename\n");
      strncpy(name, "frame", sizeof(name));
    }
  }
}

struct vary_node ** second_pass() {
  int i, k;
  int start_frame, end_frame;
  double start_value, end_value, delta;
  struct vary_node *curr = NULL;
  struct vary_node **knobs = (struct vary_node **)calloc(num_frames, sizeof(struct vary_node *));
  for (i=0; i<lastop; i++) {
    if (op[i].opcode == VARY) {
      start_frame = op[i].op.vary.start_frame;
      end_frame = op[i].op.vary.end_frame;
      start_value = op[i].op.vary.start_val;
      end_value = op[i].op.vary.end_val;
      if (start_frame < 0 || end_frame < 0 || end_frame < start_frame || end_frame > num_frames) {
        printf("Error: invalid vary command for knob: %s\n", op[i].op.vary.p->name);
        exit(-1);
      }
      delta = (end_value - start_value) / (end_frame - start_frame);
      for (k=0; k < num_frames; k++ ) {
        char found = 0;
        curr = knobs[k];
        while ( curr ) {
          if (!strcmp( curr->name, op[i].op.vary.p->name)) {
            found = 1;
            break;
          }
          curr = curr->next;
        }
        if ( !found )  {
          curr = (struct vary_node*)calloc(1, sizeof(struct vary_node));
          strncpy( curr->name, op[i].op.vary.p->name, sizeof(curr->name));
          curr->value = 0;
          curr->next = knobs[k];
          knobs[k] = curr;
        }
        if (k >= start_frame && k <= end_frame) curr->value = start_value + (k - start_frame) * delta;
      }
    }
  }
  return knobs;
}

void *draw_pass(void *line) {
  int f = (long)line;
  screen *s = new_screen();
  clear_screen(*s);
  zbuffer *zb = new_zbuffer();
  clear_zbuffer(*zb);
  color c;
  c.red = c.green = c.blue = 0;
  double knob_value, x, y, z, theta;
  int step = 40, i, j, code;
  struct stack * cstack = new_stack();
  struct matrix *tmp;
  color ambient;
  ambient.red = 50;
  ambient.green = 50;
  ambient.blue = 50;
  double light[2][3] = {{0.5, 0.75, 1}, {0, 255, 255}};
  double view[3] = {0, 0, 1};
  double areflect[3] = {0.1, 0.1, 0.1};
  double dreflect[3] = {0.5, 0.5, 0.5};
  double sreflect[3] = {0.5, 0.5, 0.5};
  struct vary_node * vn;
  char frame_name[128];
  for (i = 0; i < lastop; i++) {
    code = op[i].opcode;
    if (code == SET) {
      set_value( lookup_symbol(op[i].op.set.p->name), op[i].op.set.p->s.value);
    } else if (code == SETKNOBS){
      for (j=0; j < lastsym; j++) if (symtab[j].type == SYM_VALUE) symtab[j].s.value = op[i].op.setknobs.value;
    } else if (code == BOX || code == SPHERE || code == TORUS) {
      tmp = new_matrix(4, 4);
      if (code == BOX) {
        add_box(tmp, op[i].op.box.d0[0],op[i].op.box.d0[1], op[i].op.box.d0[2], op[i].op.box.d1[0],op[i].op.box.d1[1], op[i].op.box.d1[2]);
      } else if (code == SPHERE) {
        add_sphere(tmp, op[i].op.sphere.d[0], op[i].op.sphere.d[1], op[i].op.sphere.d[2], op[i].op.sphere.r, step);
      } else {
        add_torus(tmp, op[i].op.torus.d[0], op[i].op.torus.d[1], op[i].op.torus.d[2], op[i].op.torus.r0,op[i].op.torus.r1, step);
      }
      matrix_mult(peek(cstack), tmp);
      draw_polygons(tmp, *s, *zb, view, light, ambient, areflect, dreflect, sreflect);
      free_matrix(tmp);
    } else if (code == LINE) {
      tmp = new_matrix(4, 4);
      add_edge(tmp, op[i].op.line.p0[0],op[i].op.line.p0[1], op[i].op.line.p0[1], op[i].op.line.p1[0],op[i].op.line.p1[1], op[i].op.line.p1[1]);
      matrix_mult(peek(cstack), tmp);
      draw_lines(tmp, *s, *zb, c);
      free_matrix(tmp);
    } else if (code == MOVE || code == ROTATE || code == SCALE) {
      knob_value = 1;
      if (code == MOVE) {
        x = op[i].op.move.d[0];
        y = op[i].op.move.d[1];
        z = op[i].op.move.d[2];
        if (op[i].op.move.p) {
          if ((vn = knobs[f])) {
            while(strcmp(vn->name, op[i].op.move.p->name)) vn = vn->next;
            knob_value = vn->value;
          }
        }
        tmp = make_translate(x * knob_value, y * knob_value, z * knob_value);
      } else if (code == ROTATE) {
        if (op[i].op.rotate.p) {
          if ((vn = knobs[f])) {
            while(strcmp(vn->name, op[i].op.rotate.p->name)) vn = vn->next;
            knob_value = vn->value;
          }
        }
        theta = op[i].op.rotate.degrees * (M_PI / 180) * knob_value;
        if (op[i].op.rotate.axis == 0) tmp = make_rotX(theta);
        else if (op[i].op.rotate.axis == 1) tmp = make_rotY(theta);
        else tmp = make_rotZ(theta);
      } else {
        x = op[i].op.scale.d[0];
        y = op[i].op.scale.d[1];
        z = op[i].op.scale.d[2];
        if (op[i].op.scale.p) {
          if ((vn = knobs[f])) {
            while(strcmp(vn->name, op[i].op.scale.p->name)) vn = vn->next;
            knob_value = vn->value;
            x *= knob_value;
            y *= knob_value;
            z *= knob_value;
          }
        }
        tmp = make_scale(x, y, z);
      }
      matrix_mult(peek(cstack), tmp);
      copy_matrix(tmp, peek(cstack));
      free_matrix(tmp);
    } else if (code == PUSH) {
      push(cstack);
    } else if (code == POP) {
      pop(cstack);
    } else if (code == DISPLAY) {
      display(*s);
    } else if (code == SAVE) {
      save_extension(*s, op[i].op.save.p->name);
    } else if (code == CLEAR) {
      clear_screen(*s);
      clear_zbuffer(*zb);
    }
  }
  if (num_frames > 1) {
    sprintf(frame_name, "anim/%s%05d.png", name, f);
    save_extension( *s, frame_name );
    printf("Saving frame %s\n", frame_name);
  }
  free_stack(cstack);
  free(*s);
  free(*zb);
  return 0;
}

void my_main() {
  extern struct vary_node ** knobs;
  first_pass();
  knobs = second_pass();
  long f;
  threadpool thpool = thpool_init(get_nprocs() * 2);
  for (f = 0; f < num_frames; f++) thpool_add_work(thpool, (void*)draw_pass, (void*)f);
  thpool_wait(thpool);
  thpool_destroy(thpool);
  if (num_frames > 1) make_animation(name);
}
