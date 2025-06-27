#include "vb.h"

/* Public API */

/* Functions */

vb_result vb_speaker_initialise(vb_speaker* voice, char* voice_preference, int allow_fallback)
{
if(!voice) return vbr_invalid_args;
vbz_config c;
vb_result rc=vbz_config_initialise(&c, voice_preference, allow_fallback);
if(rc!=vbr_ok) return rc;
vbz_registry_reset(&voice->registry);
rc=vbz_register_internal_handlers(voice);
if(rc!=vbr_ok) return rc;
voice->config=c;
return vbr_ok;
}
vb_result vb_handler_register(vb_speaker* voice, char* id, vb_handler* handler)
{
if(!voice) return vbr_invalid_args;
if((!id)||(!*id)) return vbr_invalid_args;
if(!vbz_is_valid_id(id)) return vbr_handler_id_invalid;
if(vbz_registry_find_handler_by_id(&voice->registry, id)>-1) return vbr_handler_id_taken;
if(!handler) return vbr_invalid_args;
if(!vbz_handler_is_usable(handler)) return vbr_handler_invalid;
char* new=vbz_strdup(id);
if(!new) return vbr_memory;
vb_result rc=vbz_handler_prepare_registration(&voice->registry);
if(rc!=vbr_ok)
{
free(new);
return rc;
}
handler->id=new;
voice->registry.handler[voice->registry.count-1]=*handler;
return vbr_ok;
}
vb_result vb_speaker_start(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
return vbz_initialise_handler(voice);
}
vb_result vb_speak(vb_speaker* voice, char* text, int interrupt)
{
if(!voice) return vbr_invalid_args;
if(!text) return vbr_invalid_args;
if((interrupt<0)||(interrupt>1)) return vbr_invalid_args;
if(!voice->current_handler) return vbr_not_initialised;
if(!voice->current_handler->implementation.speak) return vbr_unsupported;
if(!voice->current_handler->implementation.speak(voice->current_handler, text, interrupt)) return vbr_handler_failed;
return vbr_ok;
}
vb_result vb_stop(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
if(!voice->current_handler) return vbr_not_initialised;
if(!voice->current_handler->implementation.stop) return vbr_unsupported;
if(!voice->current_handler->implementation.stop(voice->current_handler)) return vbr_handler_failed;
return vbr_ok;
}
vb_result vb_pause(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
if(!voice->current_handler) return vbr_not_initialised;
if(!voice->current_handler->implementation.pause) return vbr_unsupported;
if(!voice->current_handler->implementation.pause(voice->current_handler)) return vbr_handler_failed;
return vbr_ok;
}
vb_result vb_resume(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
if(!voice->current_handler) return vbr_not_initialised;
if(!voice->current_handler->implementation.resume) return vbr_unsupported;
if(!voice->current_handler->implementation.resume(voice->current_handler)) return vbr_handler_failed;
return vbr_ok;
}
vb_result vb_is_speaking(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
if(!voice->current_handler) return vbr_not_initialised;
if(!voice->current_handler->implementation.is_speaking) return vbr_unsupported;
if(!voice->current_handler->implementation.is_speaking(voice->current_handler)) return vbr_ok;
return vbr_speaking;
}
vb_result vb_speaker_stop(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
if(!voice->current_handler) return vbr_ok;
vbz_handler_cleanup(voice->current_handler);
voice->current_handler=NULL;
return vbr_ok;
}
void vb_speaker_cleanup(vb_speaker* voice)
{
if(!voice) return;
vb_speaker_stop(voice);
vbz_registry_cleanup(&voice->registry);
vbz_config_cleanup(&voice->config);
}

vb_result vb_handler_implement_initialise(vb_handler* handler, vb_handler_cb_initialise initialise)
{
if(!handler) return vbr_invalid_args;
if(!initialise) return vbr_invalid_args;
handler->implementation.initialise=initialise;
return vbr_ok;
}
vb_result vb_handler_implement_speak(vb_handler* handler, vb_handler_cb_speak speak)
{
if(!handler) return vbr_invalid_args;
if(!speak) return vbr_invalid_args;
handler->implementation.speak=speak;
return vbr_ok;
}
vb_result vb_handler_implement_stop(vb_handler* handler, vb_handler_cb_stop stop)
{
if(!handler) return vbr_invalid_args;
if(!stop) return vbr_invalid_args;
handler->implementation.stop=stop;
return vbr_ok;
}
vb_result vb_handler_implement_is_speaking(vb_handler* handler, vb_handler_cb_is_speaking is_speaking)
{
if(!handler) return vbr_invalid_args;
if(!is_speaking) return vbr_invalid_args;
handler->implementation.is_speaking=is_speaking;
return vbr_ok;
}
vb_result vb_handler_implement_pause(vb_handler* handler, vb_handler_cb_pause pause)
{
if(!handler) return vbr_invalid_args;
if(!pause) return vbr_invalid_args;
handler->implementation.pause=pause;
return vbr_ok;
}
vb_result vb_handler_implement_resume(vb_handler* handler, vb_handler_cb_resume resume)
{
if(!handler) return vbr_invalid_args;
if(!resume) return vbr_invalid_args;
handler->implementation.resume=resume;
return vbr_ok;
}
vb_result vb_handler_implement_cleanup(vb_handler* handler, vb_handler_cb_cleanup cleanup)
{
if(!handler) return vbr_invalid_args;
if(!cleanup) return vbr_invalid_args;
handler->implementation.cleanup=cleanup;
return vbr_ok;
}

/* Internal implementation */

vb_result vbz_config_initialise(vbz_config* config, char* handler, int allow_fallback)
{
if(!config) return vbr_invalid_args;
if(vbz_config_is_initialised(config)) return vbr_already_initialised;
config->handler_preference=NULL;
config->handler_fallback=(allow_fallback? 1: 0);
if((!handler)||(!*handler)) return vbz_config_set_state_init(config);
char* handler_preference=vbz_strdup(handler);
if(!handler_preference) return vbr_memory;
config->handler_preference=handler_preference;
return vbz_config_set_state_init(config);
}
int vbz_config_is_initialised(vbz_config* config)
{
if(!config) return 0;
if(config->begin!=vbz_config_begin) return 0;
if(config->end!=vbz_config_end) return 0;
return 1;
}
vb_result vbz_config_set_state_init(vbz_config* config)
{
if(!config) return vbr_invalid_args;
config->begin=vbz_config_begin;
config->end=vbz_config_end;
return vbr_ok;
}
void vbz_config_cleanup(vbz_config* config)
{
if(!vbz_config_is_initialised(config)) return;
free(config->handler_preference);
config->handler_preference=NULL;
config->handler_fallback=0;
config->begin=0;
config->end=0;
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
vbz_handler_unregister(&manager->handler[x]);
}
free(manager->handler);
vbz_registry_reset(manager);
}
void vbz_handler_unregister(vb_handler* handler)
{
if(!handler) return;
if(handler->id) free(handler->id);
handler->id=NULL;
vbz_handler_implementation_reset(&handler->implementation);
}
void vbz_handler_implementation_reset(vbz_handler_interface* i)
{
if(!i) return;
i->initialise=NULL;
i->speak=NULL;
i->stop=NULL;
i->is_speaking=NULL;
i->cleanup=NULL;
}
void vbz_handler_cleanup(vb_handler* handler)
{
if(!handler) return;
if(!handler->implementation.cleanup) return;
handler->implementation.cleanup(handler);
}
void vbz_registry_reset(vbz_registry* manager)
{
if(!manager) return;
manager->handler=NULL;
manager->count=0;
}
vb_result vbz_initialise_handler(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
if(!voice->config.handler_preference) return vbz_initialise_any_handler(voice);
vb_result rc=vbz_initialise_preferred_handler(voice);
if(rc==vbr_ok) return rc;
if(!voice->config.handler_fallback) return vbr_initialisation_failed;
return vbz_initialise_any_handler(voice);
}
vb_result vbz_initialise_preferred_handler(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
if(!voice->config.handler_preference) return vbr_initialisation_failed;
int id=vbz_registry_find_handler_by_id(&voice->registry, voice->config.handler_preference);
if(id<0) return vbr_initialisation_failed;
if(!voice->registry.handler[id].implementation.initialise) return vbr_initialisation_failed;
if(!voice->registry.handler[id].implementation.initialise(&voice->registry.handler[id])) return vbr_initialisation_failed;
voice->current_handler=&voice->registry.handler[id];
return vbr_ok;
}
vb_result vbz_initialise_any_handler(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
for(int x=0; x<voice->registry.count; x++)
{
if(!voice->registry.handler[x].implementation.initialise) continue;
if(!voice->registry.handler[x].implementation.initialise(&voice->registry.handler[x])) continue;
voice->current_handler=&voice->registry.handler[x];
return vbr_ok;
}
return vbr_initialisation_failed;
}
int vbz_handler_is_usable(vb_handler* handler)
{
if(!handler) return 0;
if(!handler->implementation.initialise) return 0;
if(!handler->implementation.cleanup) return 0;
return 1;
}
vb_result vbz_handler_prepare_registration(vbz_registry* registry)
{
if(!registry) return vbr_invalid_args;
int c=registry->count+1;
vb_handler* handler=realloc(registry->handler, sizeof(vb_handler)*c);
if(!handler) return vbr_memory;
registry->handler=handler;
registry->count=c;
return vbr_ok;
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
vb_result result=vbr_ok;
vb_result rc=result;
rc=vbz_sapi_register_handler(voice);
rc=vbz_mac_register_handler(voice);
return result;
}

/* Windows specific handlers */

vb_result vbz_sapi_register_handler(vb_speaker* voice)
{
if(!voice) return vbr_invalid_args;
#ifdef _WIN32
vb_handler sapi;
sapi.implementation.initialise=vbz_sapi_initialise;
sapi.implementation.speak=vbz_sapi_speak;
sapi.implementation.stop=vbz_sapi_stop;
sapi.implementation.pause=vbz_sapi_pause;
sapi.implementation.resume=vbz_sapi_resume;
sapi.implementation.is_speaking=vbz_sapi_is_speaking;
sapi.implementation.cleanup=vbz_sapi_cleanup;
return vb_handler_register(voice, "system", &sapi);
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
mac.implementation.initialise=vbz_mac_initialise;
mac.implementation.speak=vbz_mac_speak;
mac.implementation.stop=vbz_mac_stop;
mac.implementation.pause=vbz_mac_pause;
mac.implementation.resume=vbz_mac_resume;
mac.implementation.is_speaking=vbz_mac_is_speaking;
mac.implementation.cleanup=vbz_mac_cleanup;
return vb_handler_register(voice, "system", &mac);
#else
return vbr_unsupported;
#endif
}

/* Helper functions */

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
