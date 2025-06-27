#ifndef el_vb_h
#define el_vb_h

/* C standard library includes */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Platform specific systems */

#ifdef _WIN32
#include <windows.h>
#include <sapi.h>
#endif

#ifdef __APPLE__
#include <dlfcn.h>
#include <objc/objc.h>
#include <objc/message.h>
#endif

/* This file is separated into sections using the plus symbol. */

/* + Public API */

/*
The API is designed in such a way that the library does a lot of the legwork.
The only object you need to store is the vb_speaker object.
Everything else (strings, configs, handlers) is copied internally.
*/

/* ++ Typedefs */

/* +++ Aliases to internal structs */
/* This is probably the closest to opaqueness we'll get in a single .c/.h pair. */

/*
vb_speaker is the main object you will be working with.
*/

typedef struct vbz_speaker vb_speaker;

/*
vb_handler is what you will use to build your own handler.
How to use:
1. Implement your functions
2. Pass them to the appropriate vb_handler_implement functions.
3. Call vb_handler_register.
See the dummy handler example in the example/handlers directory.
Note that you may not need to do this, unless you need to support a specific handler that isn't supported here.
*/

typedef struct vbz_handler vb_handler;

/* +++ Enums */

/* ++++ Error codes */

/* Todo: Handlers currently return bools. Failure at the API level is represented by init_failed or handler_failed. This needs a redesign. */

typedef enum
{
vbr_ok=0,
vbr_internal, /* Unknown error */
vbr_memory,
vbr_unsupported,
vbr_invalid_args,
vbr_not_initialised,
vbr_init_failed,
vbr_handler_invalid,
vbr_handler_id_invalid,
vbr_handler_id_taken,
vbr_handler_failed,
vbr_speaking, /* Used to query speaking status. */
}
vb_result;

/* +++ Function pointers */
/* These are used for the handler implementation. */

typedef int (*vb_handler_cb_initialise) (vb_handler* handler);
typedef int (*vb_handler_cb_speak) (vb_handler* handler, char* text, int interrupt);
typedef int (*vb_handler_cb_stop) (vb_handler* handler);
typedef int (*vb_handler_cb_is_speaking) (vb_handler* handler);
typedef int (*vb_handler_cb_pause) (vb_handler* handler);
typedef int (*vb_handler_cb_resume) (vb_handler* handler);
typedef void (*vb_handler_cb_cleanup) (vb_handler* handler);

/* ++ Functions */

/* +++ Speaker API */

/*
vb_speaker_initialise
Registers builtin handlers and configures the system.
*/

vb_result vb_speaker_initialise(vb_speaker* voice, char* handler, int allow_fallback);

/*
vb_handler_register
Registers external handlers.
This must be called after vb_speaker_initialise and before vb_speaker_start.
For a handler to be considered usable, it must at least have initialise, speak, and cleanup functions.
If any of these functions are missing, registration will fail with vbr_handler_invalid.
*/

vb_result vb_handler_register(vb_speaker* voice, char* id, vb_handler* handler);

/*
vb_speaker_start
Initialises an appropriate handler ready for use.
*/

vb_result vb_speaker_start(vb_speaker* voice);

/*
Please note that the below functions are merely wrappers to call the handler's equivalent function.
Not all functions will be supported by all handlers.
Be sure to check for vbr_unsupported when checking your return types.
*/

/*
vb_speak
Speaks a string of text.
Interrupt: 0 to queue, 1 to interrupt.
*/

vb_result vb_speak(vb_speaker* voice, char* text, int interrupt);

/*
vb_stop
Stops speech.
*/

vb_result vb_stop(vb_speaker* voice);

/*
vb_pause
Pauses speech.
*/

vb_result vb_pause(vb_speaker* voice);

/*
vb_resume
Resumes speech.
*/

vb_result vb_resume(vb_speaker* voice);

/*
vb_is_speaking
Queries the speaking status of the engine.
*/

vb_result vb_is_speaking(vb_speaker* voice);

/*
vb_speaker_stop
Closes the currently speech engine.
This is useful if you want to change handlers.
Todo: create change_handler wrapper.
*/

vb_result vb_speaker_stop(vb_speaker* voice);

/*
vb_speaker_cleanup
Completely cleans up and destroys the speaker object.
*/

void vb_speaker_cleanup(vb_speaker* voice);

/* +++ Handler API */
/* These functions should be used to implement the handler interface. */

vb_result vb_handler_implement_initialise(vb_handler* handler, vb_handler_cb_initialise initialise);
vb_result vb_handler_implement_speak(vb_handler* handler, vb_handler_cb_speak speak);
vb_result vb_handler_implement_stop(vb_handler* handler, vb_handler_cb_stop stop);
vb_result vb_handler_implement_is_speaking(vb_handler* handler, vb_handler_cb_is_speaking is_speaking);
vb_result vb_handler_implement_pause(vb_handler* handler, vb_handler_cb_pause pause);
vb_result vb_handler_implement_resume(vb_handler* handler, vb_handler_cb_resume resume);
vb_result vb_handler_implement_cleanup(vb_handler* handler, vb_handler_cb_cleanup cleanup);

/* + Internal data: All internal data starts with "vbz_". */

/* ++ Structures */

/*
vbz_config is a way to configure the speech system you're about to initialise.
This is small at the moment but may grow in time.
*/
typedef struct
{
char* handler_preference; /* Internal, controlled with parameter to config_initialise */
int handler_fallback; /* 0 to disallow fallbacks, or non-zero to allow them. */
}
vbz_config;

/*
The vb_handler_interface struct is what holds pointers to all the functions that a handler should implement.
These should be implemented using the functions laid out in the public API above.
The speaker API merely initialises a handler and uses its interface to call the underlying functions.
For a handler to be considered usable, it must at least have initialise, speak, and cleanup functions.
*/
typedef struct
{
vb_handler_cb_initialise initialise;
vb_handler_cb_speak speak;
vb_handler_cb_stop stop;
vb_handler_cb_is_speaking is_speaking;
vb_handler_cb_pause pause;
vb_handler_cb_resume resume;
vb_handler_cb_cleanup cleanup;
}
vbz_handler_interface;

/*
vbz_handler: Internal definition of vb_handler declared above.
Internally, it has a name, an interface, and any extra data used for controlling the TTS engine.
*/

struct vbz_handler
{
char* id;
vbz_handler_interface implementation;
void* data;
};

/*
vbz_registry is what stores all the handlers.
*/

typedef struct
{
vb_handler* handler;
int count;
}
vbz_registry;

/*
vbz_speaker: Internal definition of alias vb_speaker above.
Internally, it holds a copy of the configuration, the handler registry, and a reference to the handler currently in use.
*/

struct vbz_speaker
{
vbz_config config;
vbz_registry registry;
vb_handler* current_handler;
};

/* ++ Functions */

/* +++ Config methods */

/*
vbz_config_initialise
The config structure is initialised to sensible defaults.
*/

vb_result vbz_config_initialise(vbz_config* config, char* handler, int allow_fallback);

/* +++ Registry methods */

/*
vbz_find_handler_by_id
Attempts to find a handler index by its textual ID.
Returns -1 if no handler is found.
*/

int vbz_registry_find_handler_by_id(vbz_registry* manager, char* id);

/*
vbz_handler_prepare_registration
Allocates memory for a new handler to be registered in the registry.
*/

vb_result vbz_handler_prepare_registration(vbz_registry* registry);

/*
vbz_registry_cleanup
Cleans up a registry.
*/

void vbz_registry_cleanup(vbz_registry* manager);

/*
vbz_registry_reset
Used before initialisation and after cleanup: Simply initialises all the object properties to NULL, 0 etc.
*/

void vbz_registry_reset(vbz_registry* manager);

/* +++ Handler methods */

/*
vbz_handler_is_usable
Looks for certain functions in the handler. If they are missing, it is considered unusable.
See "vb_handler_register" above.
*/

int vbz_handler_is_usable(vb_handler* handler);

/*
vbz_handler_unregister
Unregisters a handler from the registry. This is used during cleanup.
Note this clears out everything to do with a handle, including its textual ID and interface.
*/

void vbz_handler_unregister(vb_handler* handler);

/*
vbz_handler_cleanup
Cleans up data from a single handler.
Note it doesn't clear out the name or implementation, in case we want to use it again.
Its only purpose is to clear out the engine and data associated with it.
*/

void vbz_handler_cleanup(vb_handler* handler);

/* +++ Handler implementation methods */

/*
vbz_handler_implementation_reset
Used before initialisation and after cleanup: Simply initialises all the pointers to NULL.
*/

void vbz_handler_implementation_reset(vbz_handler_interface* i);

/* +++ Speaker methods */

/*
vbz_initialise_handler
Initialises an appropriate handler for use based on the configuration settings.
At the moment, the public-facing vb_speaker_start wraps this function.
*/

vb_result vbz_initialise_handler(vb_speaker* voice);

/*
vbz_initialise_preferred_handler
Attempts to initialise the preferred handler.
If this function fails, it returns vbr_init_failed.
This includes if a preferred ID can't be found, or even is unset.
*/

vb_result vbz_initialise_preferred_handler(vb_speaker* voice);

/*
vbz_initialise_any_handler
Attempts to initialise any handler by walking through the registry in order.
If this function fails, it means there are no available speech engine handlers that can be used.
*/

vb_result vbz_initialise_any_handler(vb_speaker* voice);

/* +++ Helper functions */

/*
These are mainly string/text-based functions, used for ID validation and comparison.
*/

/*
vbz_handler_is_valid_id
Checks if text matches a valid handler ID format (alphanumeric, dash, dot, or underscore).
*/

int vbz_is_valid_id(char* id);

/* Case-insensitive enabled string compare */

int vbz_strcmp(char* a, char* b, int cs);

/* Not all compilers have strdup, so we implement it. */

char* vbz_strdup(char* str);

/* + Builtin handler implementations */

/*
Since we are using runtime linking rather than compiletime, we have linking data in here as well.
These linkers contain pointers to relevant libraries and procedures that will be loaded.
*/

/* ++ Windows-specific handlers */

#ifdef _WIN32

/* +++ Structures */

/* vbz_com is a linker struct to help us deal with COM more efficiently. */

typedef struct
{
HMODULE ole;
HRESULT(WINAPI* CoInitializeEx)(LPVOID, DWORD);
HRESULT(WINAPI* CoCreateInstance)(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID*);
HRESULT(WINAPI* CoUninitialize)(void);
int autoinit;
}
vbz_com;

typedef struct
{
vbz_com com;
ISpVoice* voice;
WCHAR* text_to_speak;
}
vbz_sapi_handler;

/* +++ Functions */

/* ++++ Com helpers */

int vbz_com_initialise(vbz_com* com);
void vbz_com_reset(vbz_com* com);
int vbz_com_create_instance(vbz_com* com, CLSID* clsid, IID* iid, void** data);
void vbz_com_cleanup(vbz_com* com);

/* ++++ SAPI handler */

int vbz_sapi_initialise(vb_handler* handler);
int vbz_sapi_speak(vb_handler* handler, char* text, int interrupt);
int vbz_sapi_stop(vb_handler* handler);
int vbz_sapi_pause(vb_handler* handler);
int vbz_sapi_resume(vb_handler* handler);
int vbz_sapi_is_speaking(vb_handler* handler);
void vbz_sapi_cleanup(vb_handler* handler);

#endif

/* ++ Apple-specific handlers */

#ifdef __APPLE__

/* +++ Structures */

/*
Just like we did with COM above, we now have a linker for Objective-C functionality.
*/

typedef struct
{
void* objc;
Class (*getClass)(char* name);
id (*msgSend)(id self, SEL op, ...);
SEL (*sel_registerName)(char* name);
}
vbz_objc;

/* And now the Mac-TTS handler, using the NS framework. */

typedef struct
{
vbz_objc objc;
void* foundation;
void* appkit;
id voice;
}
vbz_mac_handler;

/* +++ Functions */

/* ++++ ObjC helpers */

int vbz_objc_initialise(vbz_objc* objc);
void vbz_objc_reset(vbz_objc* objc);
void vbz_objc_cleanup(vbz_objc* objc);

/* ++++ Mac handler */

int vbz_mac_initialise(vb_handler* handler);
int vbz_mac_speak(vb_handler* handler, char* text, int interrupt);
int vbz_mac_stop(vb_handler* handler);
int vbz_mac_pause(vb_handler* handler);
int vbz_mac_resume(vb_handler* handler);
int vbz_mac_is_speaking(vb_handler* handler);
void vbz_mac_cleanup(vb_handler* handler);

#endif

/* ++ Linux-specific handlers */

/*
Todo: Find a decent speech library to integrate here.
The key is finding one that is permissively licensed (preferably zlib or freer).
*/

/* + Builtin handler registrations */

/* We don't check platform here, that's done at the code level. */

/*
vbz_register_internal_handlers
Attempts to register all builtin handlers.
*/

vb_result vbz_register_internal_handlers(vb_speaker* voice);

/* Individual handlers */

vb_result vbz_mac_register_handler(vb_speaker* voice);
vb_result vbz_sapi_register_handler(vb_speaker* voice);

#endif
