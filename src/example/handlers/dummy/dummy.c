#include "dummy.h"

int vbz_dummy_initialise(vb_handler* handler)
{
if(!handler) return 0;
vbz_dummy_handler* data=malloc(sizeof(vbz_dummy_handler));
if(!data) return 0;
handler->data=data;
return 1;
}
int vbz_dummy_speak(vb_handler* handler, char* text, int interrupt)
{
if(!handler) return 0;
if(!text) return 0;
if(text[0]==0) return 0;
if((interrupt<0)||(interrupt>1)) return 0;
if(!interrupt) return 0;
vbz_dummy_handler* data=handler->data;
if(!data) return 0;
data->time=(strlen(text)*50);
data->clock=clock();
return 1;
}
int vbz_dummy_stop(vb_handler* handler)
{
if(!handler) return 0;
vbz_dummy_handler* data=handler->data;
if(!data) return 0;
data->clock=0;
data->time=0;
return 1;
}
int vbz_dummy_is_speaking(vb_handler* handler)
{
if(!handler) return 0;
vbz_dummy_handler* data=handler->data;
if(!data) return 0;
if(data->time<=0) return 0;
clock_t elapsed=(clock()-data->clock)*1000/CLOCKS_PER_SEC;
if(elapsed<data->time) return 1;
vbz_dummy_stop(handler);
return 0;
}
void vbz_dummy_cleanup(vb_handler* handler)
{
if(!handler) return;
if(!handler->data) return;
free(handler->data);
}

vb_result vbz_dummy_handler_register(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
vb_handler handler;
handler.implementation.load=vbz_dummy_initialise;
handler.implementation.speak=vbz_dummy_speak;
handler.implementation.stop=vbz_dummy_stop;
handler.implementation.is_speaking=vbz_dummy_is_speaking;
handler.implementation.unload=vbz_dummy_cleanup;
return vb_speaker_register_handler(voice, "Dummy", &handler);
}
