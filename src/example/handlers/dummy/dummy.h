#ifndef vbz_dummy_handler_h
#define vbz_dummy_handler_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "vb.h"

typedef struct
{
clock_t clock;
int time;
}
vbz_dummy_handler;

int vbz_dummy_initialise(vb_handler* handler);
int vbz_dummy_speak(vb_handler* handler, char* text, int interrupt);
int vbz_dummy_stop(vb_handler* handler);
int vbz_dummy_is_speaking(vb_handler* handler);
void vbz_dummy_cleanup(vb_handler* handler);
vb_result vbz_dummy_handler_register(vb_speaker* voice);

#endif
