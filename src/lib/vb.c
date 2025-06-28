#include "vb.h"

/* Public API */

/* Functions */

vb_result vb_speaker_initialise(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
if(vbz_speaker_is_initialised(voice)) return vbr_already_initialised;
vb_result rc=vbz_registry_initialise(&voice->registry);
if(rc!=vbr_ok) return rc;
rc=vbz_register_internal_handlers(voice);
if(rc!=vbr_ok)
{
vbz_registry_cleanup(&voice->registry);
return rc;
}
return vbz_speaker_set_state_init(voice);
}
vb_result vb_speaker_register_handler(vb_speaker* voice, char* id, vb_handler* handler)
{
if((!voice)||(!id)||(!*id)||(!handler)) return vbr_invalid_args;
if(!vbz_speaker_is_initialised(voice)) return vbr_not_initialised;
return vbz_speaker_register_handler(voice, id, handler);
}
vb_result vb_speaker_load(vb_speaker* voice, char* engine)
{
if(!voice) return vbr_invalid_args;
if((!engine)||(!*engine)) return vbr_invalid_args;
if(!vbz_speaker_is_initialised(voice)) return vbr_not_initialised;
int id=vbz_registry_find_handler_by_id(&voice->registry, engine);
if(id<0) return vbr_initialisation_failed;
vb_result rc=vbz_handler_load(&voice->registry.handler[id]);
if(rc!=vbr_ok) return rc;
vb_speaker_unload(voice);
voice->current_handler=&voice->registry.handler[id];
return vbr_ok;
}
vb_result vb_speaker_speak(vb_speaker* voice, char* text, int interrupt)
{
if(!voice) return vbr_invalid_args;
if(!text) return vbr_invalid_args;
if((interrupt<0)||(interrupt>1)) return vbr_invalid_args;
if(!vbz_speaker_is_initialised(voice)) return vbr_not_initialised;
if(!voice->current_handler) return vbr_not_loaded;
if(!vbz_handler_callback_is_set(&voice->current_handler->implementation.speak)) return vbr_unsupported;
vb_handler_cb_speak speak=voice->current_handler->implementation.speak.exec;
if(!speak(voice->current_handler, text, interrupt)) return vbr_handler_failed;
return vbr_ok;
}
vb_result vb_speaker_stop(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
if(!vbz_speaker_is_initialised(voice)) return vbr_not_initialised;
if(!voice->current_handler) return vbr_not_loaded;
if(!vbz_handler_callback_is_set(&voice->current_handler->implementation.stop)) return vbr_unsupported;
vb_handler_cb_stop stop=voice->current_handler->implementation.stop.exec;
if(!stop(voice->current_handler)) return vbr_handler_failed;
return vbr_ok;
}
vb_result vb_speaker_pause(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
if(!vbz_speaker_is_initialised(voice)) return vbr_not_initialised;
if(!voice->current_handler) return vbr_not_loaded;
if(!vbz_handler_callback_is_set(&voice->current_handler->implementation.pause)) return vbr_unsupported;
vb_handler_cb_pause pause=voice->current_handler->implementation.pause.exec;
if(!pause(voice->current_handler)) return vbr_handler_failed;
return vbr_ok;
}
vb_result vb_speaker_resume(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
if(!vbz_speaker_is_initialised(voice)) return vbr_not_initialised;
if(!voice->current_handler) return vbr_not_loaded;
if(!vbz_handler_callback_is_set(&voice->current_handler->implementation.resume)) return vbr_unsupported;
vb_handler_cb_resume resume=voice->current_handler->implementation.resume.exec;
if(!resume(voice->current_handler)) return vbr_handler_failed;
return vbr_ok;
}
vb_result vb_speaker_is_speaking(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
if(!vbz_speaker_is_initialised(voice)) return vbr_not_initialised;
if(!voice->current_handler) return vbr_not_loaded;
if(!vbz_handler_callback_is_set(&voice->current_handler->implementation.is_speaking)) return vbr_unsupported;
vb_handler_cb_is_speaking is_speaking=voice->current_handler->implementation.is_speaking.exec;
if(!is_speaking(voice->current_handler)) return vbr_ok;
return vbr_speaking;
}
vb_result vb_speaker_unload(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
if(!voice->current_handler) return vbr_ok;
vbz_handler_unload(voice->current_handler);
voice->current_handler=NULL;
return vbr_ok;
}
void vb_speaker_cleanup(vb_speaker* voice)
{
if(!voice) return;
vb_speaker_unload(voice);
vbz_registry_cleanup(&voice->registry);
voice->begin=0;
voice->end=0;
}

vb_result vb_handler_implement_load(vb_handler* handler, vb_handler_cb_load load)
{
if(!handler) return vbr_invalid_args;
if(!load) return vbr_invalid_args;
if(!vbz_handler_is_initialised(handler)) vbz_handler_initialise(handler);
return vbz_handler_callback_set(&handler->implementation.load, load);
}
vb_result vb_handler_implement_speak(vb_handler* handler, vb_handler_cb_speak speak)
{
if(!handler) return vbr_invalid_args;
if(!speak) return vbr_invalid_args;
if(!vbz_handler_is_initialised(handler)) vbz_handler_initialise(handler);
return vbz_handler_callback_set(&handler->implementation.speak, speak);
}
vb_result vb_handler_implement_stop(vb_handler* handler, vb_handler_cb_stop stop)
{
if(!handler) return vbr_invalid_args;
if(!stop) return vbr_invalid_args;
if(!vbz_handler_is_initialised(handler)) vbz_handler_initialise(handler);
return vbz_handler_callback_set(&handler->implementation.stop, stop);
}
vb_result vb_handler_implement_is_speaking(vb_handler* handler, vb_handler_cb_is_speaking is_speaking)
{
if(!handler) return vbr_invalid_args;
if(!is_speaking) return vbr_invalid_args;
if(!vbz_handler_is_initialised(handler)) vbz_handler_initialise(handler);
return vbz_handler_callback_set(&handler->implementation.is_speaking, is_speaking);
}
vb_result vb_handler_implement_pause(vb_handler* handler, vb_handler_cb_pause pause)
{
if(!handler) return vbr_invalid_args;
if(!pause) return vbr_invalid_args;
if(!vbz_handler_is_initialised(handler)) vbz_handler_initialise(handler);
return vbz_handler_callback_set(&handler->implementation.pause, pause);
}
vb_result vb_handler_implement_resume(vb_handler* handler, vb_handler_cb_resume resume)
{
if(!handler) return vbr_invalid_args;
if(!resume) return vbr_invalid_args;
if(!vbz_handler_is_initialised(handler)) vbz_handler_initialise(handler);
return vbz_handler_callback_set(&handler->implementation.resume, resume);
}
vb_result vb_handler_implement_unload(vb_handler* handler, vb_handler_cb_unload unload)
{
if(!handler) return vbr_invalid_args;
if(!unload) return vbr_invalid_args;
if(!vbz_handler_is_initialised(handler)) vbz_handler_initialise(handler);
return vbz_handler_callback_set(&handler->implementation.unload, unload);
}

/* Internal implementation */

int vbz_speaker_is_initialised(vb_speaker* speaker)
{
if(!speaker) return 0;
if(speaker->begin!=vbz_speaker_begin) return 0;
if(speaker->end!=vbz_speaker_end) return 0;
return 1;
}
vb_result vbz_speaker_set_state_init(vb_speaker* speaker)
{
if(!speaker) return vbr_invalid_args;
speaker->begin=vbz_speaker_begin;
speaker->end=vbz_speaker_end;
return vbr_ok;
}
vb_result vbz_speaker_register_handler(vb_speaker* voice, char* id, vb_handler* handler)
{
if((!voice)||(!id)||(!*id)||(!handler)) return vbr_invalid_args;
if(!vbz_is_valid_id(id)) return vbr_handler_id_invalid;
if(vbz_registry_find_handler_by_id(&voice->registry, id)>-1) return vbr_handler_id_taken;
if(!vbz_handler_is_usable(handler)) return vbr_handler_invalid;
char* heap_id=vbz_strdup(id);
if(!heap_id) return vbr_memory;
vb_result rc=vbz_registry_ensure_capacity(&voice->registry);
if(rc!=vbr_ok)
{
free(heap_id);
return rc;
}
handler->id=heap_id;
voice->registry.handler[voice->registry.count-1]=*handler;
return vbr_ok;
}


vb_result vbz_registry_initialise(vbz_registry* registry)
{
if(!registry) return vbr_invalid_args;
if(vbz_registry_is_initialised(registry)) return vbr_already_initialised;
vbz_registry_reset(registry);
return vbz_registry_set_state_init(registry);
}
int vbz_registry_is_initialised(vbz_registry* registry)
{
if(!registry) return 0;
if(registry->begin!=vbz_registry_begin) return 0;
if(registry->end!=vbz_registry_end) return 0;
return 1;
}
vb_result vbz_registry_set_state_init(vbz_registry* registry)
{
if(!registry) return vbr_invalid_args;
registry->begin=vbz_registry_begin;
registry->end=vbz_registry_end;
return vbr_ok;
}
int vbz_registry_find_handler_by_id(vbz_registry* manager, char* id)
{
if(!manager) return -1;
for(int x=0; x<manager->count; x++)
{
if(vbz_strcmp(id, manager->handler[x].id, 0)!=0) continue;
return x;
}
return -1;
}
vb_result vbz_registry_ensure_capacity(vbz_registry* registry)
{
if(!registry) return vbr_invalid_args;
int c=registry->count+1;
vb_handler* handler=realloc(registry->handler, sizeof(vb_handler)*c);
if(!handler) return vbr_memory;
registry->handler=handler;
registry->count=c;
return vbr_ok;
}
void vbz_registry_cleanup(vbz_registry* manager)
{
if(!manager) return;
if(!manager->handler)
{
vbz_registry_reset(manager);
return;
}
for(int x=0; x<manager->count; x++)
{
vbz_handler_cleanup(&manager->handler[x]);
}
free(manager->handler);
vbz_registry_reset(manager);
}
void vbz_registry_reset(vbz_registry* manager)
{
if(!manager) return;
manager->handler=NULL;
manager->count=0;
manager->begin=0;
manager->end=0;
}
vb_result vbz_handler_initialise(vb_handler* handler)
{
if(!handler) return vbr_invalid_args;
if(vbz_handler_is_initialised(handler)) return vbr_already_initialised;
vbz_handler_reset(handler);
return vbz_handler_set_state_init(handler);
}
int vbz_handler_is_initialised(vb_handler* handler)
{
if(!handler) return 0;
if(handler->begin!=vbz_handler_begin) return 0;
if(handler->end!=vbz_handler_end) return 0;
return 1;
}
vb_result vbz_handler_set_state_init(vb_handler* handler)
{
if(!handler) return vbr_invalid_args;
handler->begin=vbz_handler_begin;
handler->end=vbz_handler_end;
return vbr_ok;
}
vb_result vbz_handler_load(vb_handler* handler)
{
if(!handler) return vbr_invalid_args;
if(!vbz_handler_is_initialised(handler)) return vbr_not_initialised;
if(!vbz_handler_callback_is_set(&handler->implementation.load)) return vbr_initialisation_failed;
vb_handler_cb_load load=handler->implementation.load.exec;
if(!load(handler)) return vbr_initialisation_failed;
return vbr_ok;
}
int vbz_handler_is_usable(vb_handler* handler)
{
if(!handler) return 0;
if(!vbz_handler_callback_is_set(&handler->implementation.load)) return 0;
if(!vbz_handler_callback_is_set(&handler->implementation.unload)) return 0;
return 1;
}
void vbz_handler_unload(vb_handler* handler)
{
if(!handler) return;
if(!vbz_handler_callback_is_set(&handler->implementation.unload)) return;
vb_handler_cb_unload unload=handler->implementation.unload.exec;
unload(handler);
}
void vbz_handler_cleanup(vb_handler* handler)
{
if(!handler) return;
if(handler->id) free(handler->id);
vbz_handler_reset(handler);
}
void vbz_handler_reset(vb_handler* handler)
{
handler->id=NULL;
vbz_handler_implementation_reset(&handler->implementation);
handler->begin=0;
handler->end=0;
}
void vbz_handler_implementation_reset(vbz_handler_interface* i)
{
if(!i) return;
vbz_handler_callback_unset(&i->load);
vbz_handler_callback_unset(&i->speak);
vbz_handler_callback_unset(&i->stop);
vbz_handler_callback_unset(&i->pause);
vbz_handler_callback_unset(&i->resume);
vbz_handler_callback_unset(&i->is_speaking);
vbz_handler_callback_unset(&i->unload);
}
vb_result vbz_handler_callback_set(vbz_handler_callback* cb, void* exec)
{
if((!cb)||(!exec)) return vbr_invalid_args;
cb->exec=exec;
cb->begin=vbz_api_begin;
cb->end=vbz_api_end;
return vbr_ok;
}
int vbz_handler_callback_is_set(vbz_handler_callback* cb)
{
if(!cb) return 0;
if(cb->begin!=vbz_api_begin) return 0;
if(cb->end!=vbz_api_end) return 0;
return 1;
}
void vbz_handler_callback_unset(vbz_handler_callback* cb)
{
if(!cb) return;
cb->exec=NULL;
cb->begin=0;
cb->end=0;
}

/* Builtin handler implementations */

/* Windows specific handlers */

#ifdef _WIN32

/* Com helpers, for those handlers that need it. */

int vbz_com_initialise(vbz_com* com)
{
if(!com) return 0;
vbz_com_reset(com);
com->ole=LoadLibrary("Ole32.dll");
if(!com->ole) return 0;
com->CoInitializeEx=(HRESULT(WINAPI*)(LPVOID, DWORD)) GetProcAddress(com->ole, "CoInitializeEx");
if(!com->CoInitializeEx)
{
FreeLibrary(com->ole);
vbz_com_reset(com);
return 0;
}
com->CoCreateInstance=(HRESULT(WINAPI*)(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID*)) GetProcAddress(com->ole, "CoCreateInstance");
if(!com->CoCreateInstance)
{
FreeLibrary(com->ole);
vbz_com_reset(com);
return 0;
}
com->CoUninitialize=(HRESULT(WINAPI*)(void)) GetProcAddress(com->ole, "CoUninitialize");
if(!com->CoUninitialize)
{
FreeLibrary(com->ole);
vbz_com_reset(com);
return 0;
}
HRESULT hr=com->CoInitializeEx(NULL, COINIT_MULTITHREADED);
if(hr==S_OK)
{
com->autoinit=1; /* This will tell us whether we need to uninitialise. */
return 1; /* Initialisation succeeded. */
}
if((hr==S_FALSE)||(hr==RPC_E_CHANGED_MODE)) return 1; /* Already initialised. */
FreeLibrary(com->ole);
vbz_com_reset(com);
return 0;
}
void vbz_com_reset(vbz_com* com)
{
if(!com) return;
com->ole=NULL;
com->CoInitializeEx=NULL;
com->CoCreateInstance=NULL;
com->CoUninitialize=NULL;
com->autoinit=0;
}
int vbz_com_create_instance(vbz_com* com, CLSID* clsid, IID* iid, void** data)
{
if(!com) return 0;
HRESULT hr=com->CoCreateInstance(clsid, NULL, CLSCTX_ALL, iid, data);
if(FAILED(hr)) return 0;
return 1;
}
void vbz_com_cleanup(vbz_com* com)
{
if(!com) return;
if(com->autoinit) com->CoUninitialize();
FreeLibrary(com->ole);
vbz_com_reset(com);
}

/* SAPI */

int vbz_sapi_initialise(vb_handler* handler)
{
if(!handler) return 0;
vbz_sapi_handler* data=malloc(sizeof(vbz_sapi_handler));
if(!data) return 0;

/* Unfortunately, because we're runtime linking, we have to specify the COM ID data manually. */
CLSID CLSID_SpVoice={0x96749377, 0x3391, 0x11D2, {0x9E, 0xE3, 0x00, 0xC0, 0x4F, 0x79, 0x73, 0x96}};
IID IID_ISpVoice={0x6C44DF74, 0x72B9, 0x4992, {0xA1, 0xEC, 0xEF, 0x99, 0x6E, 0x04, 0x22, 0xD4}};
if(!vbz_com_initialise(&data->com))
{
free(data);
return 0;
}
if(!vbz_com_create_instance(&data->com, &CLSID_SpVoice, &IID_ISpVoice, (void**) &data->voice))
{
vbz_com_cleanup(&data->com);
free(data);
return 0;
}
data->text_to_speak=NULL;
handler->data=data;
return 1;
}
int vbz_sapi_speak(vb_handler* handler, char* text, int interrupt)
{
if(!handler) return 0;
if((!text)||(!*text)) return 0;
vbz_sapi_handler* data=handler->data;
if(!data) return 0;
int source_length=strlen(text);
int destination_length=MultiByteToWideChar(CP_ACP, 0, text, source_length, NULL, 0);
WCHAR* wtext=malloc((destination_length+1)*sizeof(WCHAR));
if(!wtext) return 0;
MultiByteToWideChar(CP_ACP, 0, text, source_length, wtext, destination_length);
wtext[destination_length]=0;
if(interrupt) vbz_sapi_stop(handler);
HRESULT hr=data->voice->lpVtbl->Speak(data->voice, wtext, SPF_DEFAULT|SPF_ASYNC, NULL);
if(FAILED(hr))
{
free(wtext);
return 0;
}
data->text_to_speak=wtext;
return 1;
}
int vbz_sapi_is_speaking(vb_handler* handler)
{
if(!handler) return 0;
vbz_sapi_handler* data=handler->data;
if(!data) return 0;
SPVOICESTATUS status;
HRESULT hr=data->voice->lpVtbl->GetStatus(data->voice, &status, NULL);
if(FAILED(hr)) return 0;
if(status.dwRunningState!=SPRS_DONE) return 1;
if(data->text_to_speak) free(data->text_to_speak);
data->text_to_speak=NULL;
return 0;
}
int vbz_sapi_stop(vb_handler* handler)
{
if(!handler) return 0;
vbz_sapi_handler* data=handler->data;
if(!data) return 0;
data->voice->lpVtbl->Speak(data->voice, NULL, SPF_PURGEBEFORESPEAK, NULL);
if(data->text_to_speak) free(data->text_to_speak);
data->text_to_speak=NULL;
return 1;
}
int vbz_sapi_pause(vb_handler* handler)
{
if(!handler) return 0;
vbz_sapi_handler* data=handler->data;
if(!data) return 0;
HRESULT hr=data->voice->lpVtbl->Pause(data->voice);
if(FAILED(hr)) return 0;
return 1;
}
int vbz_sapi_resume(vb_handler* handler)
{
if(!handler) return 0;
vbz_sapi_handler* data=handler->data;
if(!data) return 0;
HRESULT hr=data->voice->lpVtbl->Resume(data->voice);
if(FAILED(hr)) return 0;
return 1;
}
void vbz_sapi_cleanup(vb_handler* handler)
{
if(!handler) return;
vbz_sapi_handler* data=handler->data;
data->voice->lpVtbl->Release(data->voice);
data->voice = NULL;
vbz_com_cleanup(&data->com);
free(data);
handler->data=NULL;
}

#endif

/* Apple-specific handlers */

#ifdef __APPLE__

int vbz_objc_initialise(vbz_objc* objc)
{
if(!objc) return 0;
vbz_objc_reset(objc);
objc->objc=dlopen("/usr/lib/libobjc.dylib", RTLD_NOW);
if(!objc->objc) return 0;
objc->getClass=dlsym(objc->objc, "objc_getClass");
if(!objc->getClass)
{
dlclose(objc->objc);
vbz_objc_reset(objc);
return 0;
}
objc->msgSend=dlsym(objc->objc, "objc_msgSend");
if(!objc->msgSend)
{
dlclose(objc->objc);
vbz_objc_reset(objc);
return 0;
}
objc->sel_registerName=dlsym(objc->objc, "sel_registerName");
if(!objc->sel_registerName)
{
dlclose(objc->objc);
vbz_objc_reset(objc);
return 0;
}
return 1;
}
void vbz_objc_reset(vbz_objc* objc)
{
if(!objc) return;
objc->objc=NULL;
objc->getClass=NULL;
objc->msgSend=NULL;
objc->sel_registerName=NULL;
}
void vbz_objc_cleanup(vbz_objc* objc)
{
if(!objc) return;
if(!objc->objc) return;
dlclose(objc->objc);
vbz_objc_reset(objc);
}

int vbz_mac_initialise(vb_handler* handler)
{
if(!handler) return 0;
vbz_mac_handler* data=malloc(sizeof(vbz_mac_handler));
if(!data) return 0;
if(!vbz_objc_initialise(&data->objc))
{
free(data);
return 0;
}
data->foundation=dlopen("/System/Library/Frameworks/Foundation.framework/Foundation", RTLD_LOCAL);
if(!data->foundation)
{
vbz_objc_cleanup(&data->objc);
free(data);
return 0;
}
data->appkit=dlopen("/System/Library/Frameworks/AppKit.framework/AppKit", RTLD_LOCAL);
if(!data->appkit)
{
dlclose(data->foundation);
vbz_objc_cleanup(&data->objc);
free(data);
return 0;
}
Class synthobj=data->objc.getClass("NSSpeechSynthesizer");
if(!synthobj)
{
dlclose(data->appkit);
dlclose(data->foundation);
vbz_objc_cleanup(&data->objc);
free(data);
return 0;
}
SEL alloc_selector=data->objc.sel_registerName("alloc");
if(!alloc_selector)
{
dlclose(data->appkit);
dlclose(data->foundation);
vbz_objc_cleanup(&data->objc);
free(data);
return 0;
}
SEL init_selector=data->objc.sel_registerName("init");
if(!init_selector)
{
dlclose(data->appkit);
dlclose(data->foundation);
vbz_objc_cleanup(&data->objc);
free(data);
return 0;
}
data->voice=data->objc.msgSend((id) synthobj, alloc_selector);
if(!data->voice)
{
dlclose(data->appkit);
dlclose(data->foundation);
vbz_objc_cleanup(&data->objc);
free(data);
return 0;
}
id init_voice=data->objc.msgSend(data->voice, init_selector);
if(!init_voice)
{
SEL release_selector=data->objc.sel_registerName("release");
data->objc.msgSend(data->voice, release_selector);
data->voice=NULL;
dlclose(data->appkit);
dlclose(data->foundation);
vbz_objc_cleanup(&data->objc);
free(data);
return 0;
}
data->voice=init_voice;
handler->data=data;
return 1;
}

int vbz_mac_speak(vb_handler* handler, char* text, int interrupt)
{
if(!handler) return 0;
if(!text) return 0;
if(text[0]==0) return 0;
if((interrupt<0)||(interrupt>1)) return 0;
vbz_mac_handler* data=handler->data;
if(!data) return 0;
if(!data->voice) return 0;
if(interrupt) vbz_mac_stop(handler);
Class stringobj=data->objc.getClass("NSString");
if(!stringobj) return 0;
SEL utf_selector=data->objc.sel_registerName("stringWithUTF8String:");
id wtext=data->objc.msgSend((id) stringobj, utf_selector, text);
if(!wtext) return 0;
SEL speak_selector=data->objc.sel_registerName("startSpeakingString:");
if(!speak_selector) return 0;
BOOL result=(BOOL) (intptr_t) data->objc.msgSend(data->voice, speak_selector, wtext);
return result;
}

int vbz_mac_stop(vb_handler* handler)
{
if(!handler) return 0;
vbz_mac_handler* data=handler->data;
if(!data) return 0;
if(!data->voice) return 0;
SEL stop_selector=data->objc.sel_registerName("stopSpeaking");
if(!stop_selector) return 0;
data->objc.msgSend(data->voice, stop_selector);
return 1;
}

int vbz_mac_pause(vb_handler* handler)
{
if(!handler) return 0;
vbz_mac_handler* data=handler->data;
if(!data) return 0;
if(!data->voice) return 0;
SEL pause_selector=data->objc.sel_registerName("pauseSpeakingAtBoundary");
if(!pause_selector) return 0;
data->objc.msgSend(data->voice, pause_selector, 0);
return 1;
}
int vbz_mac_resume(vb_handler* handler)
{
if(!handler) return 0;
vbz_mac_handler* data=handler->data;
if(!data) return 0;
if(!data->voice) return 0;
SEL continue_selector=data->objc.sel_registerName("continueSpeaking");
if(!continue_selector) return 0;
data->objc.msgSend(data->voice, continue_selector);
return 1;
}
int vbz_mac_is_speaking(vb_handler* handler)
{
if(!handler) return 0;
vbz_mac_handler* data=handler->data;
if(!data) return 0;
if(!data->voice) return 0;
SEL stat_selector=data->objc.sel_registerName("isSpeaking");
if(!stat_selector) return 0;
BOOL result=(BOOL) (intptr_t) data->objc.msgSend(data->voice, stat_selector);
return result;
}
void vbz_mac_cleanup(vb_handler* handler)
{
if(!handler) return;
vbz_mac_handler* data=handler->data;
if(!data) return;
if(!data->voice) return;
SEL release_selector=data->objc.sel_registerName("release");
data->objc.msgSend(data->voice, release_selector);
data->voice=NULL;
dlclose(data->appkit);
dlclose(data->foundation);
vbz_objc_cleanup(&data->objc);
free(data);
handler->data=NULL;
}

#endif

/* Builtin handler registrations */

vb_result vbz_register_internal_handlers(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
vb_result result=vbr_no_voices;
vb_result rc=vbz_sapi_register_handler(voice);
if(rc==vbr_ok) result=rc;
rc=vbz_mac_register_handler(voice);
if(rc==vbr_ok) result=rc;
return result;
}

/* Windows specific handlers */

vb_result vbz_sapi_register_handler(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
#ifdef _WIN32
vb_handler sapi;
vb_handler_implement_load(&sapi, vbz_sapi_initialise);
vb_handler_implement_speak(&sapi, vbz_sapi_speak);
vb_handler_implement_stop(&sapi, vbz_sapi_stop);
vb_handler_implement_pause(&sapi, vbz_sapi_pause);
vb_handler_implement_resume(&sapi, vbz_sapi_resume);
vb_handler_implement_is_speaking(&sapi, vbz_sapi_is_speaking);
vb_handler_implement_unload(&sapi, vbz_sapi_cleanup);
return vbz_speaker_register_handler(voice, "system", &sapi);
#else
return vbr_unsupported;
#endif
}

/* Apple specific handlers. */

vb_result vbz_mac_register_handler(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
#ifdef __APPLE__
vb_handler mac;
vb_handler_implement_load(&mac, vbz_mac_initialise);
vb_handler_implement_speak(&mac, vbz_mac_speak);
vb_handler_implement_stop(&mac, vbz_mac_stop);
vb_handler_implement_pause(&mac, vbz_mac_pause);
vb_handler_implement_resume(&mac, vbz_mac_resume);
vb_handler_implement_is_speaking(&mac, vbz_mac_is_speaking);
vb_handler_implement_unload(&mac, vbz_mac_cleanup);
return vbz_speaker_register_handler(voice, "system", &mac);
#else
return vbr_unsupported;
#endif
}

/* Helper functions */

int vbz_is_valid_id(char* id)
{
if((!id)||(!*id)) return 0;
for(int x=0; x<strlen(id); x++)
{
if(isalnum(id[x])) continue;
char c=id[x];
if(c==45) continue; /* dash */
if(c==46) continue; /* dot */
if(c==95) continue; /* underscore */
return 0;
}
return 1;
}
int vbz_strcmp(char* a, char* b, int cs)
{
if((a==NULL)&&(b==NULL)) return 0;
if(!a) return -1;
if(!b) return 1;

if(cs) return strcmp(a, b);

/* We don't need a condition because the comparison checks for us. */
for(int x=0; ; x++)
{
char c1=tolower(a[x]);
char c2=tolower(b[x]);
if(c1<c2) return -1;
if(c1>c2) return 1;

/* If this far, the characters are equal, so we only need one check for NULL. */
if(c1==0) return 0;
}
return 0; /* We should theoretically never reach this point. */
}
char* vbz_strdup(char* str)
{
if((!str)||(!*str)) return NULL;
char* tmp=malloc(strlen(str)+1);
if(!tmp) return NULL;
strcpy(tmp, str);
return tmp;
}
