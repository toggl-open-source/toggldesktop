/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Solutions component.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

// see http://www.mozilla.org/projects/plugins/ for details regarding the structs and API prototypes

#ifndef QTNPAPI_H
#define QTNPAPI_H

// Plugin API version
#define NP_VERSION_MAJOR 0
#define NP_VERSION_MINOR 17

// basic types
typedef unsigned short uint16;
typedef short int16;
typedef unsigned int uint32;
typedef int int32;

typedef unsigned char NPBool;
typedef int16 NPError;
typedef int16 NPReason;
typedef char* NPMIMEType;

typedef void *NPRegion;
typedef void *NPIdentifier;

// Java stuff
typedef void* jref;
typedef void* JRIGlobalRef;
typedef void* JRIEnv; // ### not quite correct, but we don't use it anyway

// NP-types
struct NPP_t
{
    void* pdata; // plug-in private data
    void* ndata; // browser private data
};
typedef NPP_t* NPP;

struct NPRect
{
    uint16	top;
    uint16	left;
    uint16	bottom;
    uint16	right;
};

#ifdef Q_WS_WIN
struct NPEvent
{
    uint16 event;
    uint32 wParam;
    uint32 lParam;
};
#elif defined(Q_WS_X11)
#  include <X11/Xlib.h>
typedef XEvent NPEvent;
#elif defined (Q_WS_MAC)
typedef struct EventRecord NPEvent;
#endif

// Variable names for NPP_GetValue
enum NPPVariable {
    NPPVpluginNameString = 1,
    NPPVpluginDescriptionString,
    NPPVpluginWindowBool,
    NPPVpluginTransparentBool,
    NPPVjavaClass,
    NPPVpluginWindowSize,
    NPPVpluginTimerInterval,

    NPPVpluginScriptableInstance = 10,
    NPPVpluginScriptableIID = 11,

    // Introduced in Mozilla 0.9.9
    NPPVjavascriptPushCallerBool = 12,

    // Introduced in Mozilla 1.0
    NPPVpluginKeepLibraryInMemory = 13,
    NPPVpluginNeedsXEmbed         = 14,

    // Introduced in Firefox 1.0
    NPPVpluginScriptableNPObject  = 15,
    NPPVformValue = 16
} ;

// Variable names for NPN_GetValue
enum NPNVariable {
    NPNVxDisplay = 1,
    NPNVxtAppContext,
    NPNVnetscapeWindow,
    NPNVjavascriptEnabledBool,
    NPNVasdEnabledBool,
    NPNVisOfflineBool,

    // Introduced in Mozilla 0.9.4
    NPNVserviceManager = 10,
    NPNVDOMElement     = 11,
    // Introduced in Mozilla 1.2
    NPNVDOMWindow      = 12,
    NPNVToolkit        = 13,
    NPNVSupportsXEmbedBool = 14,

    NPNVWindowNPObject = 15,
    NPNVPluginElementNPObject = 16
};


enum NPWindowType {
    NPWindowTypeWindow = 1, // Windowed plug-in. The window field holds a platform-specific handle to a window.
    NPWindowTypeDrawable    // Windows: HDC; Mac OS: pointer to NP_Port structure.
};

struct NPWindow
{
    // Platform-specific handle to a native window element in the browser's window hierarchy
    // XEmbed: "In the NPP_SetWindow call, the window parameter will be the XID of the hosting
    // XEmbed window. As an implementation note, this is really the XID of a GtkSocket window."
    void* window;
    // The x and y coordinates for the top left corner of the plug-in relative to the page
    // (and thus relative to the origin of the drawable)
    uint32 x, y;
    // The height and width of the plug-in area. Should not be modified by the plug-in.
    uint32 width, height;
    // Used by MAC only (Clipping rectangle in port coordinates)
    NPRect clipRect;
#ifdef Q_WS_X11
    // Contains information about the plug-in's Unix window environment
    // points to an NPSetWindowCallbackStruct
    void* ws_info; // probably obsolete with XEmbed
#endif
    // The type field indicates the NPWindow type of the target area
    NPWindowType type;
};

struct NPPort
{
    void *port;
    int32 portx;
    int32 porty;
};

struct NPFullPrint
{
    NPBool pluginPrinted;	// true if plugin handled fullscreen printing
    NPBool printOne;            // true if plugin should print one copy to default printer
    void* platformPrint;	// Platform-specific printing info
};

struct NPEmbedPrint
{
    NPWindow window;
    void* platformPrint;	// Platform-specific printing info
};

struct NPPrint
{
    uint16 mode; // NP_FULL or NP_EMBED
    union {
        NPFullPrint fullPrint;
        NPEmbedPrint embedPrint;
    } print;
};

struct NPSavedData
{
    int32 len;
    void* buf;
};

struct NPStream
{
    void* pdata;
    void* ndata;
    const char* url;
    uint32 end;
    uint32 lastmodified;
    void* notifyData;
};

struct NPByteRange
{
    int32 offset; // negative offset means from the end
    uint32 length;
    NPByteRange* next;
};

// Values for mode passed to NPP_New:
#define NP_EMBED      1
#define NP_FULL       2

// Values for stream type passed to NPP_NewStream:
#define NP_NORMAL     1
#define NP_SEEK       2
#define NP_ASFILE     3
#define NP_ASFILEONLY 4

#define NP_MAXREADY (((unsigned)(~0)<<1)>>1)

// Values of type NPError:
#define NPERR_NO_ERROR                      0
#define NPERR_GENERIC_ERROR                 1
#define NPERR_INVALID_INSTANCE_ERROR        2
#define NPERR_INVALID_FUNCTABLE_ERROR       3
#define NPERR_MODULE_LOAD_FAILED_ERROR      4
#define NPERR_OUT_OF_MEMORY_ERROR           5
#define NPERR_INVALID_PLUGIN_ERROR          6
#define NPERR_INVALID_PLUGIN_DIR_ERROR      7
#define NPERR_INCOMPATIBLE_VERSION_ERROR    8
#define NPERR_INVALID_PARAM                 9
#define NPERR_INVALID_URL                   10
#define NPERR_FILE_NOT_FOUND                11
#define NPERR_NO_DATA                       12
#define NPERR_STREAM_NOT_SEEKABLE           13

// Values of type NPReason:
#define NPRES_DONE                          0
#define NPRES_NETWORK_ERR                   1
#define NPRES_USER_BREAK                    2

// Version feature information
#define NPVERS_HAS_STREAMOUTPUT	            8
#define NPVERS_HAS_NOTIFICATION             9
#define NPVERS_HAS_LIVECONNECT              9
#define NPVERS_WIN16_HAS_LIVECONNECT        10

// Mac specifics
#ifdef Q_WS_MAC
# define getFocusEvent       (osEvt + 16)
# define loseFocusEvent      (osEvt + 17)
# define adjustCursorEvent   (osEvt + 18)
# define QTBROWSER_USE_CFM
#endif

#ifdef QTBROWSER_USE_CFM
 extern void *CFMFunctionPointerForMachOFunctionPointer(void*);
 extern void DisposeCFMFunctionPointer(void *);
 extern void* MachOFunctionPointerForCFMFunctionPointer(void*);
# define FUNCTION_POINTER(t) void*
# define MAKE_FUNCTION_POINTER(f) CFMFunctionPointerForMachOFunctionPointer((void*)f)
# define DESTROY_FUNCTION_POINTER(n) DisposeCFMFunctionPointer(n)
# define FIND_FUNCTION_POINTER(t, n) (*(t)MachOFunctionPointerForCFMFunctionPointer(n))
#else
# define FUNCTION_POINTER(t) t
# define MAKE_FUNCTION_POINTER(f) f
# define DESTROY_FUNCTION_POINTER(n)
# define FIND_FUNCTION_POINTER(t, n) (*n)
#endif

// Plugin function prototypes
typedef NPError	(*NPP_NewFP)(NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved);
typedef NPError	(*NPP_DestroyFP)(NPP instance, NPSavedData** save);
typedef NPError	(*NPP_SetWindowFP)(NPP instance, NPWindow* window);
typedef NPError	(*NPP_NewStreamFP)(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype);
typedef NPError	(*NPP_DestroyStreamFP)(NPP instance, NPStream* stream, NPReason reason);
typedef void (*NPP_StreamAsFileFP)(NPP instance, NPStream* stream, const char* fname);
typedef int32 (*NPP_WriteReadyFP)(NPP instance, NPStream* stream);
typedef int32 (*NPP_WriteFP)(NPP instance, NPStream* stream, int32 offset, int32 len, void* buffer);
typedef void (*NPP_PrintFP)(NPP instance, NPPrint* platformPrint);
typedef int16 (*NPP_HandleEventFP)(NPP instance, NPEvent* event);
typedef void (*NPP_URLNotifyFP)(NPP instance, const char* url, NPReason reason, void* notifyData);
typedef NPError (*NPP_GetValueFP)(NPP instance, NPPVariable variable, void *value);
typedef NPError (*NPP_SetValueFP)(NPP instance, NPPVariable variable, void *value);

// table of functions implemented by the plugin
struct NPPluginFuncs {
    uint16 size;
    uint16 version;
    FUNCTION_POINTER(NPP_NewFP) newp;
    FUNCTION_POINTER(NPP_DestroyFP) destroy;
    FUNCTION_POINTER(NPP_SetWindowFP) setwindow;
    FUNCTION_POINTER(NPP_NewStreamFP) newstream;
    FUNCTION_POINTER(NPP_DestroyStreamFP) destroystream;
    FUNCTION_POINTER(NPP_StreamAsFileFP) asfile;
    FUNCTION_POINTER(NPP_WriteReadyFP) writeready;
    FUNCTION_POINTER(NPP_WriteFP) write;
    FUNCTION_POINTER(NPP_PrintFP) print;
    FUNCTION_POINTER(NPP_HandleEventFP) event;
    FUNCTION_POINTER(NPP_URLNotifyFP) urlnotify;
    JRIGlobalRef javaClass;
    FUNCTION_POINTER(NPP_GetValueFP) getvalue;
    FUNCTION_POINTER(NPP_SetValueFP) setvalue;
} ;


// forward declarations
struct NPObject;
struct NPClass;
struct NPVariant;
struct NPString;
struct QtNPInstance;

// NPObject is the type used to express objects exposed by either
// the plugin or by the browser. Implementation specific  (i.e. plugin
// specific, or browser specific) members can come after the struct.
// In our case, the plugin specific member (aka QObject) lives in NPClass.
struct NPObject
{
    NPClass *_class;
    uint32 refCount;
};

// NPClass is what virtual function tables would look like if
// there was no C++...
typedef NPObject *(*NPAllocateFP)(NPP npp, NPClass *aClass);
typedef void (*NPDeallocateFP)(NPObject *npobj);
typedef void (*NPInvalidateFP)(NPObject *npobj);
typedef bool (*NPHasMethodFP)(NPObject *npobj, NPIdentifier name);
typedef bool (*NPInvokeFP)(NPObject *npobj, NPIdentifier name,const NPVariant *args, uint32 argCount,NPVariant *result);
typedef bool (*NPInvokeDefaultFP)(NPObject *npobj,const NPVariant *args,uint32 argCount,NPVariant *result);
typedef bool (*NPHasPropertyFP)(NPObject *npobj, NPIdentifier name);
typedef bool (*NPGetPropertyFP)(NPObject *npobj, NPIdentifier name, NPVariant *result);
typedef bool (*NPSetPropertyFP)(NPObject *npobj, NPIdentifier name, const NPVariant *value);
typedef bool (*NPRemovePropertyFP)(NPObject *npobj, NPIdentifier name);

#define NP_CLASS_STRUCT_VERSION 1

struct NPClass
{
    NPClass(QtNPInstance *qtnp);
    ~NPClass();

    // NP API
    uint32 structVersion;
    NPAllocateFP allocate;
    NPDeallocateFP deallocate;
    NPInvalidateFP invalidate;
    NPHasMethodFP hasMethod;
    NPInvokeFP invoke;
    NPInvokeDefaultFP invokeDefault;
    NPHasPropertyFP hasProperty;
    NPGetPropertyFP getProperty;
    NPSetPropertyFP setProperty;
    NPRemovePropertyFP removeProperty;

    // User data lives here
    QtNPInstance *qtnp;
    bool delete_qtnp;
};

struct NPString {
    const char *utf8characters;
    uint32 utf8length;

    // Qt specific conversion routines
    // (no c'tor as it would be misleading that there is no d'tor in spite of memory allocation)
    static NPString fromQString(const QString &qstr);
    operator QString() const;
};

struct NPVariant {
    enum Type {
        Void,
        Null,
        Boolean,
        Int32,
        Double,
        String,
        Object
    };
    Type type;
    union {
        bool boolValue;
        uint32 intValue;
        double doubleValue;
        NPString stringValue;
        NPObject *objectValue;
    } value;

    NPVariant()
        : type(Null)
    {}

    // Qt specific conversion routines
    // (no c'tor as the NPP instance is required)
    static NPVariant fromQVariant(QtNPInstance *This, const QVariant &qvariant);
    operator QVariant() const;

private:
};

#ifdef Q_WS_X11
extern "C" {
#endif

// Browser function prototypes
typedef NPError	(*NPN_GetURLFP)(NPP instance, const char* url, const char* window);
typedef NPError (*NPN_PostURLFP)(NPP instance, const char* url, const char* window, uint32 len, const char* buf, NPBool file);
typedef NPError	(*NPN_RequestReadFP)(NPStream* stream, NPByteRange* rangeList);
typedef NPError	(*NPN_NewStreamFP)(NPP instance, NPMIMEType type, const char* window, NPStream** stream);
typedef int32 (*NPN_WriteFP)(NPP instance, NPStream* stream, int32 len, void* buffer);
typedef NPError (*NPN_DestroyStreamFP)(NPP instance, NPStream* stream, NPReason reason);
typedef void (*NPN_StatusFP)(NPP instance, const char* message);
typedef const char* (*NPN_UserAgentFP)(NPP instance);
typedef void* (*NPN_MemAllocFP)(uint32 size);
typedef void (*NPN_MemFreeFP)(void* ptr);
typedef uint32 (*NPN_MemFlushFP)(uint32 size);
typedef void (*NPN_ReloadPluginsFP)(NPBool reloadPages);
typedef JRIEnv* (*NPN_GetJavaEnvFP)(void);
typedef jref (*NPN_GetJavaPeerFP)(NPP instance);
typedef NPError	(*NPN_GetURLNotifyFP)(NPP instance, const char* url, const char* window, void* notifyData);
typedef NPError (*NPN_PostURLNotifyFP)(NPP instance, const char* url, const char* window, uint32 len, const char* buf, NPBool file, void* notifyData);
typedef NPError	(*NPN_GetValueFP)(NPP instance, NPNVariable variable, void *ret_value);
typedef NPError (*NPN_SetValueFP)(NPP instance, NPPVariable variable, void *ret_value);
typedef void (*NPN_InvalidateRectFP)(NPP instance, NPRect *rect);
typedef void (*NPN_InvalidateRegionFP)(NPP instance, NPRegion *region);
typedef void (*NPN_ForceRedrawFP)(NPP instance);
typedef NPIdentifier (*NPN_GetStringIdentifierFP)(const char* name);
typedef void (*NPN_GetStringIdentifiersFP)(const char** names, int32 nameCount, NPIdentifier* identifiers);
typedef NPIdentifier (*NPN_GetIntIdentifierFP)(int32 intid);
typedef bool (*NPN_IdentifierIsStringFP)(NPIdentifier identifier);
typedef char* (*NPN_UTF8FromIdentifierFP)(NPIdentifier identifier);
typedef int32 (*NPN_IntFromIdentifierFP)(NPIdentifier identifier);
typedef NPObject* (*NPN_CreateObjectFP)(NPP npp, NPClass *aClass);
typedef NPObject* (*NPN_RetainObjectFP)(NPObject *obj);
typedef void (*NPN_ReleaseObjectFP)(NPObject *obj);
typedef bool (*NPN_InvokeFP)(NPP npp, NPObject* obj, NPIdentifier methodName, const NPVariant *args, int32 argCount, NPVariant *result);
typedef bool (*NPN_InvokeDefaultFP)(NPP npp, NPObject* obj, const NPVariant *args, int32 argCount, NPVariant *result);
typedef bool (*NPN_EvaluateFP)(NPP npp, NPObject *obj, NPString *script, NPVariant *result);
typedef bool (*NPN_GetPropertyFP)(NPP npp, NPObject *obj, NPIdentifier propertyName, NPVariant *result);
typedef bool (*NPN_SetPropertyFP)(NPP npp, NPObject *obj, NPIdentifier propertyName, const NPVariant *value);
typedef bool (*NPN_RemovePropertyFP)(NPP npp, NPObject *obj, NPIdentifier propertyName);
typedef bool (*NPN_HasPropertyFP)(NPP npp, NPObject *obj, NPIdentifier propertyName);
typedef bool (*NPN_HasMethodFP)(NPP npp, NPObject *obj, NPIdentifier methodName);
typedef void (*NPN_ReleaseVariantValueFP)(NPVariant *variant);
typedef void (*NPN_SetExceptionFP)(NPObject *obj, const char *message);

// function declarations
NPError	NPN_GetURL(NPP instance, const char* url, const char* window);
NPError NPN_PostURL(NPP instance, const char* url, const char* window, uint32 len, const char* buf, NPBool file);
NPError	NPN_RequestRead(NPStream* stream, NPByteRange* rangeList);
NPError	NPN_NewStream(NPP instance, NPMIMEType type, const char* window, NPStream** stream);
int32 NPN_Write(NPP instance, NPStream* stream, int32 len, void* buffer);
NPError NPN_DestroyStream(NPP instance, NPStream* stream, NPReason reason);
void NPN_Status(NPP instance, const char* message);
const char* NPN_UserAgent(NPP instance);
void* NPN_MemAlloc(uint32 size);
void NPN_MemFree(void* ptr);
uint32 NPN_MemFlush(uint32 size);
void NPN_ReloadPlugins(NPBool reloadPages);
JRIEnv* NPN_GetJavaEnv(void);
jref NPN_GetJavaPeer(NPP instance);
NPError	NPN_GetURLNotify(NPP instance, const char* url, const char* window, void* notifyData);
NPError NPN_PostURLNotify(NPP instance, const char* url, const char* window, uint32 len, const char* buf, NPBool file, void* notifyData);
NPError	NPN_GetValue(NPP instance, NPNVariable variable, void *ret_value);
NPError NPN_SetValue(NPP instance, NPPVariable variable, void *ret_value);
void NPN_InvalidateRect(NPP instance, NPRect *rect);
void NPN_InvalidateRegion(NPP instance, NPRegion *region);
void NPN_ForceRedraw(NPP instance);
NPIdentifier NPN_GetStringIdentifier(const char* name);
void NPN_GetStringIdentifiers(const char** names, int32 nameCount, NPIdentifier* identifiers);
NPIdentifier NPN_GetIntIdentifier(int32 intid);
bool NPN_IdentifierIsString(NPIdentifier identifier);
char* NPN_UTF8FromIdentifier(NPIdentifier identifier);
int32 NPN_IntFromIdentifier(NPIdentifier identifier);
NPObject* NPN_CreateObject(NPP npp, NPClass *aClass);
NPObject* NPN_RetainObject(NPObject *obj);
void NPN_ReleaseObject(NPObject *obj);
bool NPN_Invoke(NPP npp, NPObject* obj, NPIdentifier methodName, const NPVariant *args, int32 argCount, NPVariant *result);
bool NPN_InvokeDefault(NPP npp, NPObject* obj, const NPVariant *args, int32 argCount, NPVariant *result);
bool NPN_Evaluate(NPP npp, NPObject *obj, NPString *script, NPVariant *result);
bool NPN_GetProperty(NPP npp, NPObject *obj, NPIdentifier propertyName, NPVariant *result);
bool NPN_SetProperty(NPP npp, NPObject *obj, NPIdentifier propertyName, const NPVariant *value);
bool NPN_RemoveProperty(NPP npp, NPObject *obj, NPIdentifier propertyName);
bool NPN_HasProperty(NPP npp, NPObject *obj, NPIdentifier propertyName);
bool NPN_HasMethod(NPP npp, NPObject *obj, NPIdentifier methodName);
void NPN_ReleaseVariantValue(NPVariant *variant);
void NPN_SetException(NPObject *obj, const char *message);

// table of function implemented by the browser
struct NPNetscapeFuncs {
    uint16 size;
    uint16 version;
    FUNCTION_POINTER(NPN_GetURLFP) geturl;
    FUNCTION_POINTER(NPN_PostURLFP) posturl;
    FUNCTION_POINTER(NPN_RequestReadFP) requestread;
    FUNCTION_POINTER(NPN_NewStreamFP) newstream;
    FUNCTION_POINTER(NPN_WriteFP) write;
    FUNCTION_POINTER(NPN_DestroyStreamFP) destroystream;
    FUNCTION_POINTER(NPN_StatusFP) status;
    FUNCTION_POINTER(NPN_UserAgentFP) uagent;
    FUNCTION_POINTER(NPN_MemAllocFP) memalloc;
    FUNCTION_POINTER(NPN_MemFreeFP) memfree;
    FUNCTION_POINTER(NPN_MemFlushFP) memflush;
    FUNCTION_POINTER(NPN_ReloadPluginsFP) reloadplugins;
    FUNCTION_POINTER(NPN_GetJavaEnvFP) getJavaEnv;
    FUNCTION_POINTER(NPN_GetJavaPeerFP) getJavaPeer;
    FUNCTION_POINTER(NPN_GetURLNotifyFP) geturlnotify;
    FUNCTION_POINTER(NPN_PostURLNotifyFP) posturlnotify;
    FUNCTION_POINTER(NPN_GetValueFP) getvalue;
    FUNCTION_POINTER(NPN_SetValueFP) setvalue;
    FUNCTION_POINTER(NPN_InvalidateRectFP) invalidaterect;
    FUNCTION_POINTER(NPN_InvalidateRegionFP) invalidateregion;
    FUNCTION_POINTER(NPN_ForceRedrawFP) forceredraw;
    FUNCTION_POINTER(NPN_GetStringIdentifierFP) getstringidentifier;
    FUNCTION_POINTER(NPN_GetStringIdentifiersFP) getstringidentifiers;
    FUNCTION_POINTER(NPN_GetIntIdentifierFP) getintidentifier;
    FUNCTION_POINTER(NPN_IdentifierIsStringFP) identifierisstring;
    FUNCTION_POINTER(NPN_UTF8FromIdentifierFP) utf8fromidentifier;
    FUNCTION_POINTER(NPN_IntFromIdentifierFP) intfromidentifier;
    FUNCTION_POINTER(NPN_CreateObjectFP) createobject;
    FUNCTION_POINTER(NPN_RetainObjectFP) retainobject;
    FUNCTION_POINTER(NPN_ReleaseObjectFP) releaseobject;
    FUNCTION_POINTER(NPN_InvokeFP) invoke;
    FUNCTION_POINTER(NPN_InvokeDefaultFP) invokedefault;
    FUNCTION_POINTER(NPN_EvaluateFP) evaluate;
    FUNCTION_POINTER(NPN_GetPropertyFP) getproperty;
    FUNCTION_POINTER(NPN_SetPropertyFP) setproperty;
    FUNCTION_POINTER(NPN_RemovePropertyFP) removeproperty;
    FUNCTION_POINTER(NPN_HasPropertyFP) hasproperty;
    FUNCTION_POINTER(NPN_HasMethodFP) hasmethod;
    FUNCTION_POINTER(NPN_ReleaseVariantValueFP) releasevariantvalue;
    FUNCTION_POINTER(NPN_SetExceptionFP) setexception;
};

#ifdef Q_WS_X11
}
#endif

#endif
