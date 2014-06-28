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

#include <QtGui>

#include "qtnpapi.h"

#include "qtbrowserplugin.h"
#include "qtbrowserplugin_p.h"

#ifndef WINAPI
# ifdef Q_WS_WIN
#  define WINAPI __stdcall
# else
#  define WINAPI
# endif
#endif

#ifdef Q_WS_X11
#  ifdef Bool
#    undef Bool
#  endif

/*
static void debuginfo(const QString &str)
{
    static bool inited = false;
    QFile file("/tmp/qnsdebug.txt");
    if (file.open(QFile::WriteOnly | QFile::Append)) {
        if (!inited) {
            file.write("\n\n*** New run started ***\n");
            inited = true;
        }
        file.write(qtNPFactory()->pluginName().toLatin1() + ": " + str.toLatin1() + '\n');
        file.close();
    }
}
*/

#endif

static QtNPFactory *qNP = 0;
static NPNetscapeFuncs *qNetscapeFuncs = 0;

// The single global plugin
QtNPFactory *qtNPFactory()
{
    extern QtNPFactory *qtns_instantiate();

    if (!qNP) {
        qNP = qtns_instantiate();
    }

    return qNP;
}

// NPN functions, forwarding to function pointers provided by browser
void NPN_Version(int* plugin_major, int* plugin_minor, int* netscape_major, int* netscape_minor)
{
    Q_ASSERT(qNetscapeFuncs);
    *plugin_major   = NP_VERSION_MAJOR;
    *plugin_minor   = NP_VERSION_MINOR;
    *netscape_major = qNetscapeFuncs->version  >> 8;  // Major version is in high byte
    *netscape_minor = qNetscapeFuncs->version & 0xFF; // Minor version is in low byte
}

#define NPN_Prolog(x) \
    Q_ASSERT(qNetscapeFuncs); \
    Q_ASSERT(qNetscapeFuncs->x); \


const char *NPN_UserAgent(NPP instance)
{
    NPN_Prolog(uagent);
    return FIND_FUNCTION_POINTER(NPN_UserAgentFP, qNetscapeFuncs->uagent)(instance);
}

void NPN_Status(NPP instance, const char* message)
{
    NPN_Prolog(status);
    FIND_FUNCTION_POINTER(NPN_StatusFP, qNetscapeFuncs->status)(instance, message);
}

NPError NPN_GetURL(NPP instance, const char* url, const char* window)
{
    NPN_Prolog(geturl);
    return FIND_FUNCTION_POINTER(NPN_GetURLFP, qNetscapeFuncs->geturl)(instance, url, window);
}

NPError NPN_GetURLNotify(NPP instance, const char* url, const char* window, void* notifyData)
{
    if ((qNetscapeFuncs->version & 0xFF) < NPVERS_HAS_NOTIFICATION)
        return NPERR_INCOMPATIBLE_VERSION_ERROR;

    NPN_Prolog(geturlnotify);
    return FIND_FUNCTION_POINTER(NPN_GetURLNotifyFP, qNetscapeFuncs->geturlnotify)(instance, url, window, notifyData);
}

NPError NPN_PostURLNotify(NPP instance, const char* url, const char* window, uint32 len, const char* buf, NPBool file, void* notifyData)
{
    if ((qNetscapeFuncs->version & 0xFF) < NPVERS_HAS_NOTIFICATION)
        return NPERR_INCOMPATIBLE_VERSION_ERROR;

    NPN_Prolog(posturlnotify);
    return FIND_FUNCTION_POINTER(NPN_PostURLNotifyFP, qNetscapeFuncs->posturlnotify)(instance, url, window, len, buf, file, notifyData);
}

void* NPN_MemAlloc(uint32 size)
{
    NPN_Prolog(memalloc);
    return FIND_FUNCTION_POINTER(NPN_MemAllocFP, qNetscapeFuncs->memalloc)(size);
}

void NPN_MemFree(void* ptr)
{
    NPN_Prolog(memfree);
    FIND_FUNCTION_POINTER(NPN_MemFreeFP, qNetscapeFuncs->memfree)(ptr);
}

uint32 NPN_MemFlush(uint32 size)
{
    NPN_Prolog(memflush);
    return FIND_FUNCTION_POINTER(NPN_MemFlushFP, qNetscapeFuncs->memflush)(size);
}

NPError	NPN_GetValue(NPP instance, NPNVariable variable, void *ret_value)
{
    NPN_Prolog(getvalue);
    return FIND_FUNCTION_POINTER(NPN_GetValueFP, qNetscapeFuncs->getvalue)(instance, variable, ret_value);
}

NPError NPN_SetValue(NPP instance, NPPVariable variable, void *ret_value)
{
    NPN_Prolog(setvalue);
    return FIND_FUNCTION_POINTER(NPN_SetValueFP, qNetscapeFuncs->setvalue)(instance, variable, ret_value);
}

NPIdentifier NPN_GetStringIdentifier(const char* name)
{
    NPN_Prolog(getstringidentifier);
    return FIND_FUNCTION_POINTER(NPN_GetStringIdentifierFP, qNetscapeFuncs->getstringidentifier)(name);
}

void NPN_GetStringIdentifiers(const char** names, int32 nameCount, NPIdentifier* identifiers)
{
    NPN_Prolog(getstringidentifiers);
    FIND_FUNCTION_POINTER(NPN_GetStringIdentifiersFP, qNetscapeFuncs->getstringidentifiers)(names, nameCount, identifiers);
}

NPIdentifier NPN_GetIntIdentifier(int32 intid)
{
    NPN_Prolog(getintidentifier);
    return FIND_FUNCTION_POINTER(NPN_GetIntIdentifierFP, qNetscapeFuncs->getintidentifier)(intid);
}

bool NPN_IdentifierIsString(NPIdentifier identifier)
{
    NPN_Prolog(identifierisstring);
    return FIND_FUNCTION_POINTER(NPN_IdentifierIsStringFP, qNetscapeFuncs->identifierisstring)(identifier);
}

char* NPN_UTF8FromIdentifier(NPIdentifier identifier)
{
    NPN_Prolog(utf8fromidentifier);
    return FIND_FUNCTION_POINTER(NPN_UTF8FromIdentifierFP, qNetscapeFuncs->utf8fromidentifier)(identifier);
}

int32 NPN_IntFromIdentifier(NPIdentifier identifier)
{
    NPN_Prolog(intfromidentifier);
    return FIND_FUNCTION_POINTER(NPN_IntFromIdentifierFP, qNetscapeFuncs->intfromidentifier)(identifier);
}

NPObject* NPN_CreateObject(NPP npp, NPClass *aClass)
{
    NPN_Prolog(createobject);
    return FIND_FUNCTION_POINTER(NPN_CreateObjectFP, qNetscapeFuncs->createobject)(npp, aClass);
}

NPObject* NPN_RetainObject(NPObject *obj)
{
    NPN_Prolog(retainobject);
    return FIND_FUNCTION_POINTER(NPN_RetainObjectFP, qNetscapeFuncs->retainobject)(obj);
}

void NPN_ReleaseObject(NPObject *obj)
{
    NPN_Prolog(releaseobject);
    FIND_FUNCTION_POINTER(NPN_ReleaseObjectFP, qNetscapeFuncs->releaseobject)(obj);
}

// Scripting implementation (QObject calling JavaScript in browser) - we don't use those
bool NPN_Invoke(NPP npp, NPObject* obj, NPIdentifier methodName, const NPVariant *args, int32 argCount, NPVariant *result)
{
    NPN_Prolog(invoke);
    return FIND_FUNCTION_POINTER(NPN_InvokeFP, qNetscapeFuncs->invoke)(npp, obj, methodName, args, argCount, result);
}

bool NPN_InvokeDefault(NPP npp, NPObject* obj, const NPVariant *args, int32 argCount, NPVariant *result)
{
    NPN_Prolog(invokedefault);
    return FIND_FUNCTION_POINTER(NPN_InvokeDefaultFP, qNetscapeFuncs->invokedefault)(npp, obj, args, argCount, result);
}

bool NPN_Evaluate(NPP npp, NPObject *obj, NPString *script, NPVariant *result)
{
    NPN_Prolog(evaluate);
    return FIND_FUNCTION_POINTER(NPN_EvaluateFP, qNetscapeFuncs->evaluate)(npp, obj, script, result);
}

bool NPN_GetProperty(NPP npp, NPObject *obj, NPIdentifier propertyName, NPVariant *result)
{
    NPN_Prolog(getproperty);
    return FIND_FUNCTION_POINTER(NPN_GetPropertyFP, qNetscapeFuncs->getproperty)(npp, obj, propertyName, result);
}

bool NPN_SetProperty(NPP npp, NPObject *obj, NPIdentifier propertyName, const NPVariant *value)
{
    NPN_Prolog(setproperty);
    return FIND_FUNCTION_POINTER(NPN_SetPropertyFP, qNetscapeFuncs->setproperty)(npp, obj, propertyName, value);
}

bool NPN_RemoveProperty(NPP npp, NPObject *obj, NPIdentifier propertyName)
{
    NPN_Prolog(removeproperty);
    return FIND_FUNCTION_POINTER(NPN_RemovePropertyFP, qNetscapeFuncs->removeproperty)(npp, obj, propertyName);
}

bool NPN_HasProperty(NPP npp, NPObject *obj, NPIdentifier propertyName)
{
    NPN_Prolog(hasproperty);
    return FIND_FUNCTION_POINTER(NPN_HasPropertyFP, qNetscapeFuncs->hasproperty)(npp, obj, propertyName);
}

bool NPN_HasMethod(NPP npp, NPObject *obj, NPIdentifier methodName)
{
    NPN_Prolog(hasmethod);
    return FIND_FUNCTION_POINTER(NPN_HasMethodFP, qNetscapeFuncs->hasmethod)(npp, obj, methodName);
}

void NPN_ReleaseVariantValue(NPVariant *variant)
{
    NPN_Prolog(releasevariantvalue);
    FIND_FUNCTION_POINTER(NPN_ReleaseVariantValueFP, qNetscapeFuncs->releasevariantvalue)(variant);
}

void NPN_SetException(NPObject *obj, const char *message)
{
    qDebug("NPN_SetException: %s", message);
    NPN_Prolog(setexception);
    FIND_FUNCTION_POINTER(NPN_SetExceptionFP, qNetscapeFuncs->setexception)(obj, message);
}

// Scripting implementation (JavaScript calling QObject)
#define NPClass_Prolog \
    if (!npobj->_class) return false; \
    if (!npobj->_class->qtnp) return false; \
    QtNPInstance *This = npobj->_class->qtnp; \
    if (!This->qt.object) return false; \
    QObject *qobject = This->qt.object \


static NPObject *NPAllocate(NPP npp, NPClass *aClass)
{
    Q_UNUSED(npp);
    Q_UNUSED(aClass);

    Q_ASSERT(false);
    return 0;
}

static void NPDeallocate(NPObject *npobj)
{
    Q_UNUSED(npobj);

    Q_ASSERT(false);
    return;
}

static void NPInvalidate(NPObject *npobj)
{
    if (npobj)
        delete npobj->_class;
    npobj->_class = 0;
}

enum MetaOffset { MetaProperty, MetaMethod };

static int metaOffset(const QMetaObject *metaObject, MetaOffset offsetType)
{
    int classInfoIndex = metaObject->indexOfClassInfo("ToSuperClass");
    if (classInfoIndex == -1)
        return 0;
    QByteArray ToSuperClass = metaObject->classInfo(classInfoIndex).value();
    int offset = offsetType == MetaProperty ? metaObject->propertyOffset()
        : metaObject->methodOffset();

    while (ToSuperClass != metaObject->className()) {
        metaObject = metaObject->superClass();
        if (!metaObject)
            break;
        offset -= offsetType == MetaProperty ? metaObject->propertyCount()
                    : metaObject->methodCount();
    }
    return offset;
}

static int publicMethodIndex(NPObject *npobj, const QByteArray &slotName, int argCount = -1)
{
    NPClass_Prolog;
    const QMetaObject *metaObject = qobject->metaObject();
    for (int slotIndex = metaOffset(metaObject, MetaMethod); slotIndex < metaObject->methodCount(); ++slotIndex) {
        const QMetaMethod slot = qobject->metaObject()->method(slotIndex);
        if (slot.access() != QMetaMethod::Public || slot.methodType() == QMetaMethod::Signal)
            continue;
        QByteArray signature = slot.signature();
        if (signature.left(signature.indexOf('(')) == slotName) {
            if (argCount == -1 || slot.parameterTypes().count() == argCount)
                return slotIndex;
        }
    }
    return -1;
}

static bool NPClass_HasMethod(NPObject *npobj, NPIdentifier name)
{
    NPClass_Prolog;
    Q_UNUSED(qobject);
    return publicMethodIndex(npobj, NPN_UTF8FromIdentifier(name)) != -1;
}

static bool NPClass_Invoke(NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32 argCount, NPVariant *result)
{
    NPClass_Prolog;
    const QByteArray slotName = NPN_UTF8FromIdentifier(name);
    int slotIndex = publicMethodIndex(npobj, slotName, static_cast<int>(argCount));
    if (slotIndex == -1) {
        NPN_SetException(npobj, QByteArray("No method '" + slotName + "' with " + QByteArray::number(argCount) + " parameters").constData());
        return false;
    }

    const QMetaMethod slot = qobject->metaObject()->method(slotIndex);
    QList<QByteArray> parameterTypes = slot.parameterTypes();
    if (parameterTypes.count() != static_cast<int>(argCount)) {
        NPN_SetException(npobj, QByteArray("Wrong parameter count for method " + slotName).constData());
        return false;
    }

    QVariant returnVariant(QVariant::nameToType(slot.typeName()), (void*)0);
    QVector<QVariant> variants(parameterTypes.count()); // keep data alive
    QVector<const void*> metacallArgs(parameterTypes.count() + 1); // arguments for qt_metacall
    metacallArgs[0] = returnVariant.data(); // args[0] == return value

    for (int p = 0; p < parameterTypes.count(); ++p) {
        QVariant::Type type = QVariant::nameToType(parameterTypes.at(p));
        if (type == QVariant::Invalid && parameterTypes.at(p) != "QVariant") {
            NPN_SetException(npobj, QString("Parameter %1 in method '%2' has invalid type")
                .arg(p).arg(QString::fromUtf8(slotName)).toAscii().constData());
            return false;
        }
        QVariant qvar = args[p];
        if (type != QVariant::Invalid && !qvar.convert(type)) {
            NPN_SetException(npobj, QString("Parameter %1 to method '%2' needs to be convertable to '%3'")
                .arg(p).arg(QString::fromUtf8(slotName)).arg(QString::fromAscii(parameterTypes.at(p))).toAscii().constData());
            return false;
        }

        variants[p] = qvar;
        if (type == QVariant::Invalid)
            metacallArgs[p + 1] = &variants.at(p);
        else
            metacallArgs[p + 1] = variants.at(p).constData(); // must not detach!
    }

    qobject->qt_metacall(QMetaObject::InvokeMetaMethod, slotIndex, const_cast<void**>(metacallArgs.data()));
    if (returnVariant.isValid() && result)
        *result = NPVariant::fromQVariant(This, returnVariant);

    return true;
}

static bool NPClass_InvokeDefault(NPObject * /*npobj*/, const NPVariant * /*args*/, uint32 /*argCount*/, NPVariant * /*result*/)
{
    return false;
}

static bool NPClass_HasProperty(NPObject *npobj, NPIdentifier name)
{
    NPClass_Prolog;
    const QByteArray qname = NPN_UTF8FromIdentifier(name);
    const QMetaObject *metaObject = qobject->metaObject();
    int propertyIndex = metaObject->indexOfProperty(qname);
    if (propertyIndex == -1 || propertyIndex < metaOffset(metaObject, MetaProperty))
        return false;
    QMetaProperty property = qobject->metaObject()->property(propertyIndex);
    if (!property.isScriptable())
        return false;

    return true;
}

static bool NPClass_GetProperty(NPObject *npobj, NPIdentifier name, NPVariant *result)
{
    NPClass_Prolog;
    const QByteArray qname = NPN_UTF8FromIdentifier(name);
    QVariant qvar = qobject->property(qname);
    if (!qvar.isValid()) {
        NPN_SetException(npobj, QByteArray("Failed to get value for property " + qname).constData());
        return false;
    }
    *result = NPVariant::fromQVariant(This, qvar);
    return true;
}

static bool NPClass_SetProperty(NPObject *npobj, NPIdentifier name, const NPVariant *result)
{
    NPClass_Prolog;
    const QByteArray qname = NPN_UTF8FromIdentifier(name);
    QVariant qvar = *result;
    return qobject->setProperty(qname, qvar);
}

static bool NPClass_RemoveProperty(NPObject * /*npobj*/, NPIdentifier /*name*/)
{
    return false;
}

NPClass::NPClass(QtNPInstance *This)
{
    structVersion = NP_CLASS_STRUCT_VERSION;
    allocate = 0;
    deallocate = 0;
    invalidate = NPInvalidate;
    hasMethod = NPClass_HasMethod;
    invoke = NPClass_Invoke;
    invokeDefault = NPClass_InvokeDefault;
    hasProperty = NPClass_HasProperty;
    getProperty = NPClass_GetProperty;
    setProperty = NPClass_SetProperty;
    removeProperty = NPClass_RemoveProperty;
    qtnp = This;
    delete_qtnp = false;
}

NPClass::~NPClass()
{
    if (delete_qtnp)
        delete qtnp;
}

// Type conversions
NPString NPString::fromQString(const QString &qstr)
{
    NPString npstring;
    const QByteArray qutf8 = qstr.toUtf8();

    npstring.utf8length = qutf8.length();
    npstring.utf8characters = (char*)NPN_MemAlloc(npstring.utf8length);
    memcpy((char*)npstring.utf8characters, qutf8.constData(), npstring.utf8length);

    return npstring;
}

NPString::operator QString() const
{
    return QString::fromUtf8(utf8characters, utf8length);
}

NPVariant NPVariant::fromQVariant(QtNPInstance *This, const QVariant &qvariant)
{
    Q_ASSERT(This);
    NPVariant npvar;
    npvar.type = Null;

    QVariant qvar(qvariant);
    switch(qvariant.type()) {
    case QVariant::Bool:
        npvar.value.boolValue = qvar.toBool();
        npvar.type = Boolean;
        break;
    case QVariant::Int:
        npvar.value.intValue = qvar.toInt();
        npvar.type = Int32;
        break;
    case QVariant::Double:
        npvar.value.doubleValue = qvar.toDouble();
        npvar.type = Double;
        break;
    case QVariant::UserType:
        {
            QByteArray userType = qvariant.typeName();
            if (userType.endsWith('*')) {
                QtNPInstance *that = new QtNPInstance;
                that->npp = This->npp;
                that->qt.object = *(QObject**)qvariant.constData();
                NPClass *npclass = new NPClass(that);
                npclass->delete_qtnp = true;
                npvar.value.objectValue = NPN_CreateObject(This->npp, npclass);
                npvar.type = Object;
            }
        }
        break;
    default: // including QVariant::String
        if (!qvar.convert(QVariant::String))
            break;
        npvar.type = String;
        npvar.value.stringValue = NPString::fromQString(qvar.toString());
        break;
    }

    return npvar;
}

NPVariant::operator QVariant() const
{
    switch(type) {
    case Void:
    case Null:
        return QVariant();
    case Object:
        {
            if (!value.objectValue || !value.objectValue->_class)
                break;
            NPClass *aClass = value.objectValue->_class;
            // not one of ours?
            if (aClass->invoke != NPClass_Invoke)
                break;
            // or just empty for some reason
            QObject *qobject = aClass->qtnp->qt.object;
            if (!qobject)
                break;
            QByteArray typeName = qobject->metaObject()->className();
            int userType = QMetaType::type(typeName + "*");
            if (userType == QVariant::Invalid)
                break;
            QVariant result(userType, &aClass->qtnp->qt.object);
            // sanity check
            Q_ASSERT(*(QObject**)result.constData() == aClass->qtnp->qt.object);
            return result;
        }
    case Boolean:
        return value.boolValue;
    case Int32:
        return value.intValue;
    case Double:
        return value.doubleValue;
    case String:
        return QString(value.stringValue);
    default:
        break;
    }
    return QVariant();
}

// Helper class for handling incoming data
class QtNPStream
{
public:
    QtNPStream(NPP instance, NPStream *st);
    virtual ~QtNPStream()
    {
    }

    QString url() const;
    bool finish(QtNPBindable *bindable);

    QByteArray buffer;
    QFile file;
    QString mime;

    NPError reason;

    NPP npp;
    NPStream* stream;

protected:
    qint64 readData(char *, qint64);
    qint64 writeData(const char *, qint64);
};

QtNPStream::QtNPStream(NPP instance, NPStream *st)
    : reason(NPRES_DONE), npp(instance), stream(st)
{
}

/*!
    Returns the URL from which the stream was created, or the empty string
    for write-only streams.
*/
QString QtNPStream::url() const
{
    if (!stream)
        return QString();
    return QString::fromLocal8Bit(stream->url);
}

class ErrorBuffer : public QBuffer
{
    friend class QtNPStream;
};

bool QtNPStream::finish(QtNPBindable *bindable)
{
    if (!bindable)
        return false;

    bool res = false;
    if (bindable) {
        switch(reason) {
        case NPRES_DONE:
            // no data at all? url is probably local file (Opera)
            if (buffer.isEmpty() && file.fileName().isEmpty()) {
                QUrl u = QUrl::fromEncoded(stream->url);
                QString lfn = u.toLocalFile();
                if (lfn.startsWith("//localhost/"))
                    lfn = lfn.mid(12);
                file.setFileName(lfn);
            }

            if (file.exists()) {
                file.setObjectName(url());
                res = bindable->readData(&file, mime);
            } else {
                QBuffer io(&buffer);
                io.setObjectName(url());
                res = bindable->readData(&io, mime);
            }
            break;
        case NPRES_USER_BREAK:
            {
                ErrorBuffer empty;
                empty.setObjectName(url());
                empty.setErrorString("User cancelled operation."),
                res = bindable->readData(&empty, mime);
            }
            break;
        case NPRES_NETWORK_ERR:
            {
                ErrorBuffer empty;
                empty.setObjectName(url());
                empty.setErrorString("Network error during download."),
                res = bindable->readData(&empty, mime);
            }
            break;
        default:
            break;
        }
    }
    stream->pdata = 0;
    delete this;
    return res;
}

// Helper class for forwarding signal emissions to the respective JavaScript
class QtSignalForwarder : public QObject
{
public:
    QtSignalForwarder(QtNPInstance *that)
        : This(that), domNode(0)
    {
    }

    ~QtSignalForwarder()
    {
        if (domNode)
            NPN_ReleaseObject(domNode);
    }

    int qt_metacall(QMetaObject::Call call, int index, void **args);

private:
    QtNPInstance *This;
    NPObject *domNode;
};

int QtSignalForwarder::qt_metacall(QMetaObject::Call call, int index, void **args)
{
    // no support for QObject method/properties etc!
    if (!This || !This->npp || call != QMetaObject::InvokeMetaMethod
        || !This->qt.object)
        return index;

    switch (index) {
    case -1:
        {
            QString msg = *(QString*)args[1];
            NPN_Status(This->npp, msg.toLocal8Bit().constData());
        }
        break;
    default:
        {
            QObject *qobject = This->qt.object;
            if (!domNode)
                NPN_GetValue(This->npp, NPNVPluginElementNPObject, &domNode);
            if (!domNode)
                break;
            const QMetaObject *metaObject = qobject->metaObject();
            if (index < metaOffset(metaObject, MetaMethod))
                break;

            const QMetaMethod method = metaObject->method(index);
            Q_ASSERT(method.methodType() == QMetaMethod::Signal);

            QByteArray signalSignature = method.signature();
            QByteArray scriptFunction = signalSignature.left(signalSignature.indexOf('('));
            NPIdentifier id = NPN_GetStringIdentifier(scriptFunction.constData());
            if (NPN_HasMethod(This->npp, domNode, id)) {
                QList<QByteArray> parameterTypes = method.parameterTypes();
                QVector<NPVariant> parameters;
                NPVariant result;
                bool error = false;
                for (int p = 0; p < parameterTypes.count(); ++p) {
                    QVariant::Type type = QVariant::nameToType(parameterTypes.at(p));
                    if (type == QVariant::Invalid) {
                        NPN_SetException(domNode, QByteArray("Unsupported parameter type in ") + scriptFunction);
                        error = true;
                        break;
                    }
                    QVariant qvar(type, args[p + 1]);
                    NPVariant npvar = NPVariant::fromQVariant(This, qvar);
                    if (npvar.type == NPVariant::Null || npvar.type == NPVariant::Void) {
                        NPN_SetException(domNode, QByteArray("Unsupported parameter value in ") + scriptFunction);
                        error =true;
                        break;
                    }
                    parameters += npvar;
                }
                if (error)
                    break;

                NPError nperror = NPN_Invoke(This->npp, domNode, id, parameters.constData(), parameters.count(), &result);
                if (nperror != NPERR_NO_ERROR && false) { // disabled, as NPN_Invoke seems to always return GENERICERROR
                    NPN_SetException(domNode, QByteArray("Error invoking event handler ") + scriptFunction);
                }
                // ### TODO: update return value (args[0]) (out-parameters not supported anyway)
                NPN_ReleaseVariantValue(&result);
            }
        }
        break;
    }

    return index;
}


// Plugin functions
extern "C" NPError
NPP_GetValue(NPP instance, NPPVariable variable, void *value)
{
    if (!instance || !instance->pdata)
	return NPERR_INVALID_INSTANCE_ERROR;

    QtNPInstance* This = (QtNPInstance*) instance->pdata;

    switch (variable) {
    case NPPVpluginNameString:
        {
            static QByteArray name = qtNPFactory()->pluginName().toLocal8Bit();
            *(const char**)value = name.constData();
        }
        break;
    case NPPVpluginDescriptionString:
        {
            static QByteArray description = qtNPFactory()->pluginDescription().toLocal8Bit();
            *(const char**)value = description.constData();
        }
        break;

#ifdef Q_WS_X11
    case NPPVpluginNeedsXEmbed:
        *(int*)value = true; // PRBool = int
        break;
#endif

    case NPPVpluginScriptableNPObject:
        {
            NPObject *object = NPN_CreateObject(instance, new NPClass(This));
            *(NPObject**)value = object;
        }
        break;
    case NPPVformValue:
        {
            QObject *object = This->qt.object;
            const QMetaObject *metaObject = object->metaObject();
            int defaultIndex = metaObject->indexOfClassInfo("DefaultProperty");
            if (defaultIndex == -1)
                return NPERR_GENERIC_ERROR;
            QByteArray defaultProperty = metaObject->classInfo(defaultIndex).value();
            if (defaultProperty.isEmpty())
                return NPERR_GENERIC_ERROR;
            QVariant defaultValue = object->property(defaultProperty);
            if (!defaultValue.isValid())
                return NPERR_GENERIC_ERROR;
            defaultProperty = defaultValue.toString().toUtf8();
            int size = defaultProperty.size();
            char *utf8 = (char*)NPN_MemAlloc(size + 1);
            memcpy(utf8, defaultProperty.constData(), size);
            utf8[size] = 0; // null-terminator
            *(void**)value = utf8;
        }
        break;
    default:
        return NPERR_GENERIC_ERROR;
    }

    return NPERR_NO_ERROR;
}

extern "C" NPError
NPP_SetValue(NPP instance, NPPVariable variable, void *value)
{
    Q_UNUSED(variable);
    Q_UNUSED(value);

    if (!instance || !instance->pdata)
	return NPERR_INVALID_INSTANCE_ERROR;

    /*
    switch(variable) {
    default:
        return NPERR_GENERIC_ERROR;
    }
    */
    return NPERR_NO_ERROR;
}

extern "C" int16 NPP_Event(NPP instance, NPEvent* event)
{
    if (!instance || !instance->pdata)
	return NPERR_INVALID_INSTANCE_ERROR;

    QtNPInstance* This = (QtNPInstance*) instance->pdata;
    extern bool qtns_event(QtNPInstance *, NPEvent *);
    return qtns_event(This, event) ? 1 : 0;
}

#ifdef Q_WS_X11
// Instance state information about the plugin.
extern "C" char*
NP_GetMIMEDescription(void)
{
    static QByteArray mime = qtNPFactory()->mimeTypes().join(";").toLocal8Bit();
    return (char*)mime.constData();
}

extern "C" NPError
NP_GetValue(void*, NPPVariable aVariable, void *aValue)
{
    NPError err = NPERR_NO_ERROR;

    static QByteArray name = qtNPFactory()->pluginName().toLocal8Bit();
    static QByteArray descr = qtNPFactory()->pluginDescription().toLocal8Bit();

    switch (aVariable) {
    case NPPVpluginNameString:
        *static_cast<const char **> (aValue) = name.constData();
        break;
    case NPPVpluginDescriptionString:
        *static_cast<const char **>(aValue) = descr.constData();
        break;
    case NPPVpluginNeedsXEmbed:
        *static_cast<int*>(aValue) = true;
        break;
    case NPPVpluginTimerInterval:
    case NPPVpluginKeepLibraryInMemory:
    default:
        err = NPERR_INVALID_PARAM;
        break;
    }
    return err;
}
#endif

/*
** NPP_New is called when your plugin is instantiated (i.e. when an EMBED
** tag appears on a page).
*/
extern "C" NPError
NPP_New(NPMIMEType pluginType,
    NPP instance,
    uint16 mode,
    int16 argc,
    char* argn[],
    char* argv[],
    NPSavedData* /*saved*/)
{
    if (!instance)
	return NPERR_INVALID_INSTANCE_ERROR;

    QtNPInstance* This = new QtNPInstance;
    if (!This)
	return NPERR_OUT_OF_MEMORY_ERROR;

    instance->pdata = This;
    This->filter = 0;
    This->bindable = 0;
    This->npp = instance;
    This->fMode = mode; // NP_EMBED, NP_FULL, or NP_BACKGROUND (see npapi.h)
    This->window = 0;
    This->qt.object = 0;
#ifdef Q_WS_MAC
    This->rootWidget = 0;
#endif
    This->pendingStream = 0; // stream might be created before instance
    This->mimetype = QString::fromLatin1(pluginType);
    This->notificationSeqNum = 0;

    for (int i = 0; i < argc; i++) {
        QByteArray name = QByteArray(argn[i]).toLower();
        if (name == "id")
            This->htmlID = argv[i];
        This->parameters[name] = QVariant(argv[i]);
    }

    return NPERR_NO_ERROR;
}

extern "C" NPError
NPP_Destroy(NPP instance, NPSavedData** /*save*/)
{
    if (!instance || !instance->pdata)
	return NPERR_INVALID_INSTANCE_ERROR;

    QtNPInstance* This = (QtNPInstance*) instance->pdata;

#ifdef Q_WS_X11
    //This->widget->destroy(false, false); // X has destroyed all windows
#endif
    delete This->qt.object;
    This->qt.object = 0;
    delete This->filter;
    This->filter = 0;
    extern void qtns_destroy(QtNPInstance *This);
    qtns_destroy(This);
    delete This;
    instance->pdata = 0;

    return NPERR_NO_ERROR;
}

static QtNPInstance *next_pi = 0; // helper to connect to QtNPBindable

extern "C" NPError
NPP_SetWindow(NPP instance, NPWindow* window)
{
    if (!instance)
	return NPERR_INVALID_INSTANCE_ERROR;

    QtNPInstance* This = (QtNPInstance*) instance->pdata;
    extern void qtns_setGeometry(QtNPInstance*, const QRect &, const QRect &);

    const QRect clipRect(window->clipRect.left, window->clipRect.top,
                         window->clipRect.right - window->clipRect.left,
                         window->clipRect.bottom - window->clipRect.top);
    if (window)
        This->geometry = QRect(window->x, window->y, window->width, window->height);

    // take a shortcut if all that was changed is the geometry
    if (qobject_cast<QWidget*>(This->qt.object) && window && This->window == (QtNPInstance::Widget)window->window) {
        qtns_setGeometry(This, This->geometry, clipRect);
	return NPERR_NO_ERROR;
    }

	delete This->qt.object;
	This->qt.object = 0;
	extern void qtns_destroy(QtNPInstance *This);
	qtns_destroy(This);

    if (!window) {
        This->window = 0;
	return NPERR_NO_ERROR;
    }

    This->window = (QtNPInstance::Widget)window->window;
#ifdef Q_WS_X11
    //This->display = ((NPSetWindowCallbackStruct *)window->ws_info)->display;
#endif

    extern void qtns_initialize(QtNPInstance*);
    qtns_initialize(This);

    next_pi = This;
    This->qt.object = qtNPFactory()->createObject(This->mimetype);
    next_pi = 0;

    if (!This->qt.object)
        return NPERR_NO_ERROR;

    if (!This->htmlID.isEmpty())
        This->qt.object->setObjectName(QLatin1String(This->htmlID));

    This->filter = new QtSignalForwarder(This);
    QStatusBar *statusbar = qFindChild<QStatusBar*>(This->qt.object);
    if (statusbar) {
        int statusSignal = statusbar->metaObject()->indexOfSignal("messageChanged(QString)");
        if (statusSignal != -1) {
            QMetaObject::connect(statusbar, statusSignal, This->filter, -1);
            statusbar->hide();
        }
    }

    const QMetaObject *mo = This->qt.object->metaObject();
    for (int p = 0; p < mo->propertyCount(); ++p) {
        const QMetaProperty property = mo->property(p);
        QByteArray name(property.name());
        QVariant value = This->parameters.value(name.toLower());
        if (value.isValid())
            property.write(This->qt.object, value);
    }
    for (int methodIndex = 0; methodIndex < mo->methodCount(); ++methodIndex) {
        const QMetaMethod method = mo->method(methodIndex);
        if (method.methodType() == QMetaMethod::Signal)
            QMetaObject::connect(This->qt.object, methodIndex, This->filter, methodIndex);
    }

    if (This->pendingStream) {
        This->pendingStream->finish(This->bindable);
        This->pendingStream = 0;
    }

    if (!qobject_cast<QWidget*>(This->qt.object))
	return NPERR_NO_ERROR;

    extern void qtns_embed(QtNPInstance*);
    qtns_embed(This);

    QEvent e(QEvent::EmbeddingControl);
    QApplication::sendEvent(This->qt.widget, &e);

    if (!This->qt.widget->testAttribute(Qt::WA_PaintOnScreen))
        This->qt.widget->setAutoFillBackground(true);
    This->qt.widget->raise();
    qtns_setGeometry(This, This->geometry, clipRect);
    This->qt.widget->show();

    return NPERR_NO_ERROR;
}

extern "C" NPError
NPP_NewStream(NPP instance,
	  NPMIMEType type,
	  NPStream *stream,
	  NPBool /*seekable*/,
	  uint16 *stype)
{
    if (!instance)
	return NPERR_INVALID_INSTANCE_ERROR;

    QtNPInstance* This = (QtNPInstance*) instance->pdata;
    if (!This)
        return NPERR_NO_ERROR;

    QtNPStream *qstream = new QtNPStream(instance, stream);
    qstream->mime = QString::fromLocal8Bit(type);
    stream->pdata = qstream;

    // Workaround bug in Firefox/Gecko/Mozilla; observed in version 3.0.5 on Windows:
    //   On page reload, it does not call StreamAsFile() even when stype is AsFileOnly
    if (QByteArray(NPN_UserAgent(instance)).contains("Mozilla"))
        *stype = NP_NORMAL;
    else
        *stype = NP_ASFILEONLY;

    return NPERR_NO_ERROR;
}

extern "C" int32
NPP_WriteReady(NPP, NPStream *stream)
{
    if (stream->pdata)
        return 0x0FFFFFFF;
    return 0;
}

// Both Netscape and FireFox call this for OnDemand streams as well...
extern "C" int32
NPP_Write(NPP instance, NPStream *stream, int32 /*offset*/, int32 len, void *buffer)
{
    if (!instance || !stream || !stream->pdata)
        return NPERR_INVALID_INSTANCE_ERROR;

    // this should not be called, as we always demand a download
    QtNPStream *qstream = (QtNPStream*)stream->pdata;
    QByteArray data((const char*)buffer, len); // make deep copy
    qstream->buffer += data;

    return len;
}

// Opera calls this for OnDemand streams without calling NPP_Write first
extern "C" NPError
NPP_DestroyStream(NPP instance, NPStream *stream, NPError reason)
{
    if (!instance || !instance->pdata || !stream || !stream->pdata)
	return NPERR_INVALID_INSTANCE_ERROR;

    QtNPInstance *This = (QtNPInstance*)instance->pdata;
    QtNPStream *qstream = (QtNPStream*)stream->pdata;
    qstream->reason = reason;

    if (!This->qt.object) { // not yet initialized
        This->pendingStream = qstream;
        return NPERR_NO_ERROR;
    }

    This->pendingStream = 0;
    qstream->finish(This->bindable);

    return NPERR_NO_ERROR;
}

extern "C" void
NPP_StreamAsFile(NPP instance, NPStream *stream, const char* fname)
{
    if (!instance || !stream || !stream->pdata)
        return;

    QString path = QString::fromLocal8Bit(fname);
#ifdef Q_WS_MAC
    path = "/" + path.section(':', 1).replace(':', '/');
#endif

    QtNPStream *qstream = (QtNPStream*)stream->pdata;
    qstream->file.setFileName(path);
}

extern "C" void
NPP_URLNotify(NPP instance,
	      const char* url,
	      NPReason reason,
	      void* notifyData)
{
    if (!instance)
        return;
    QtNPInstance* This = (QtNPInstance*) instance->pdata;
    if (!This->bindable)
        return;

    QtNPBindable::Reason r;
    switch (reason) {
    case NPRES_DONE:
	r = QtNPBindable::ReasonDone;
	break;
    case NPRES_USER_BREAK:
	r = QtNPBindable::ReasonBreak;
	break;
    case NPRES_NETWORK_ERR:
	r = QtNPBindable::ReasonError;
	break;
    default:
	r = QtNPBindable::ReasonUnknown;
	break;
    }

    qint32 id = static_cast<qint32>(reinterpret_cast<size_t>(notifyData));
    if (id < 0)  // Sanity check
        id = 0;

    This->bindable->transferComplete(QString::fromLocal8Bit(url), id, r);
}

extern "C" void
NPP_Print(NPP instance, NPPrint* printInfo)
{
    if(!printInfo || !instance)
	return;

    QtNPInstance* This = (QtNPInstance*) instance->pdata;
    if (!This->bindable)
        return;

/*
    if (printInfo->mode == NP_FULL) {
	printInfo->print.fullPrint.pluginPrinted = This->bindable->printFullPage();
    } else if (printInfo->mode == NP_EMBED) {
        extern void qtns_print(QtNPInstance*, NPPrint*);
        qtns_print(This, printInfo);
    }
*/
}

// Plug-in entrypoints - these are called by the browser

// Fills in functiontable used by browser to call entry points in plugin.
extern "C" NPError WINAPI NP_GetEntryPoints(NPPluginFuncs* pFuncs)
{
    if(!pFuncs)
        return NPERR_INVALID_FUNCTABLE_ERROR;
    if(!pFuncs->size)
        pFuncs->size = sizeof(NPPluginFuncs);
    else if (pFuncs->size < sizeof(NPPluginFuncs))
        return NPERR_INVALID_FUNCTABLE_ERROR;

    pFuncs->version       = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
    pFuncs->newp          = MAKE_FUNCTION_POINTER(NPP_New);
    pFuncs->destroy       = MAKE_FUNCTION_POINTER(NPP_Destroy);
    pFuncs->setwindow     = MAKE_FUNCTION_POINTER(NPP_SetWindow);
    pFuncs->newstream     = MAKE_FUNCTION_POINTER(NPP_NewStream);
    pFuncs->destroystream = MAKE_FUNCTION_POINTER(NPP_DestroyStream);
    pFuncs->asfile        = MAKE_FUNCTION_POINTER(NPP_StreamAsFile);
    pFuncs->writeready    = MAKE_FUNCTION_POINTER(NPP_WriteReady);
    pFuncs->write         = MAKE_FUNCTION_POINTER(NPP_Write);
    pFuncs->print         = MAKE_FUNCTION_POINTER(NPP_Print);
    pFuncs->event         = MAKE_FUNCTION_POINTER(NPP_Event);
    pFuncs->urlnotify     = MAKE_FUNCTION_POINTER(NPP_URLNotify);
    pFuncs->javaClass     = 0;
    pFuncs->getvalue      = MAKE_FUNCTION_POINTER(NPP_GetValue);
    pFuncs->setvalue      = MAKE_FUNCTION_POINTER(NPP_SetValue);
    return NPERR_NO_ERROR;
}

enum NPNToolkitType
{
    NPNVGtk12 = 1,
    NPNVGtk2
};

#ifndef Q_WS_X11
extern "C" NPError WINAPI NP_Initialize(NPNetscapeFuncs* pFuncs)
{
    if(!pFuncs)
        return NPERR_INVALID_FUNCTABLE_ERROR;

    qNetscapeFuncs = pFuncs;
    int navMajorVers = qNetscapeFuncs->version >> 8;

    // if the plugin's major version is lower than the Navigator's,
    // then they are incompatible, and should return an error
    if(navMajorVers > NP_VERSION_MAJOR)
        return NPERR_INCOMPATIBLE_VERSION_ERROR;

    return NPERR_NO_ERROR;
}
#else
extern "C" NPError WINAPI NP_Initialize(NPNetscapeFuncs* nFuncs, NPPluginFuncs* pFuncs)
{
    if(!nFuncs)
        return NPERR_INVALID_FUNCTABLE_ERROR;

    qNetscapeFuncs = nFuncs;
    int navMajorVers = qNetscapeFuncs->version >> 8;

    // if the plugin's major version is lower than the Navigator's,
    // then they are incompatible, and should return an error
    if(navMajorVers > NP_VERSION_MAJOR)
        return NPERR_INCOMPATIBLE_VERSION_ERROR;

    // check if the Browser supports the XEmbed protocol
    int supportsXEmbed = 0;
    NPError err = NPN_GetValue(0, NPNVSupportsXEmbedBool, (void *)&supportsXEmbed);
    if (err != NPERR_NO_ERROR ||!supportsXEmbed)
        return NPERR_INCOMPATIBLE_VERSION_ERROR;

    return NP_GetEntryPoints(pFuncs);
}
#endif

extern "C" NPError WINAPI NP_Shutdown()
{
    delete qNP;
    qNP = 0;

    extern void qtns_shutdown();
    qtns_shutdown();

    qNetscapeFuncs = 0;
    return NPERR_NO_ERROR;
}


/*!
    \class QtNPBindable qtnetscape.h
    \brief The QtNPBindable class provides an interface between a widget and the web browser.

    Inherit your plugin widget class from both QWidget (or QObject) and QtNPBindable
    to be able to call the functions of this class, and to reimplement the virtual
    functions. The \l{moc}{meta-object compiler} requires you to inherit from the
    QObject subclass first.

    \code
    class PluginWidget : public QWidget, public QtNPBindable
    {
        Q_OBJECT
    public:
        PluginWidget(QWidget *parent = 0)
        {
        }

        //...
    };
    \endcode
*/

/*!
    \enum QtNPBindable::DisplayMode

    \brief This enum specifies the different display modes of a plugin

    \value Embedded The plugin widget is embedded in a web page, usually
    with the <EMBED> or the <OBJECT> tag.
    \value Fullpage The plugin widget is the primary content of the web browser, which
    is usually the case when the web browser displays a file the plugin supports.
*/

/*!
    \enum QtNPBindable::Reason

    \brief This enum specifies how an URL operation was completed

    \value ReasonDone
    \value ReasonBreak
    \value ReasonError
    \value ReasonUnknown
*/

/*!
    Constructs a QtNPBindable object.

    This can only happen when the plugin object is created.
*/
QtNPBindable::QtNPBindable()
: pi(next_pi)
{
    if (pi)
        pi->bindable = this;
    next_pi = 0;
}

/*!
    Destroys the object.

    This can only happen when the plugin object is destroyed.
*/
QtNPBindable::~QtNPBindable()
{
}

/*!
    Returns the parameters passed to the plugin instance.

    The framework sets the properties of the plugin to the corresponding
    parameters when the plugin object has been created, but you can
    use this function to process additional parameters.

    Note that the SGML specification does not permit multiple
    arguments with the same name.
*/
QMap<QByteArray, QVariant> QtNPBindable::parameters() const
{
    if (!pi)
        return QMap<QByteArray, QVariant>();
    return pi->parameters;
}

/*!
    Returns the user agent (browser name) containing this plugin.

    This is a wrapper around NPN_UserAgent.

    \sa getBrowserVersion()
*/
QString QtNPBindable::userAgent() const
{
    if (!pi)
        return QString();
    return QString::fromLocal8Bit(NPN_UserAgent(pi->npp));
}

/*!
    Extracts the version of the plugin API used by this plugin into \a major
    and \a minor.

    See http://devedge-temp.mozilla.org/library/manuals/2002/plugin/1.0/
    for an explanation of those values.

    \sa getBrowserVersion() userAgent()
*/
void QtNPBindable::getNppVersion(int *major, int *minor) const
{
    int dummy = 0;
    if (pi)
        NPN_Version(major, minor, &dummy, &dummy);
}

/*!
    Extracts the version of the browser into \a major and \a minor.

    See http://devedge-temp.mozilla.org/library/manuals/2002/plugin/1.0/
    for an explanation of those values.

    \sa getNppVersion() userAgent()
*/
void QtNPBindable::getBrowserVersion(int *major, int *minor) const
{
    int dummy = 0;
    if (pi)
        NPN_Version(&dummy, &dummy, major, minor);
}

/*!
    Returns the display mode of the plugin.
*/
QtNPBindable::DisplayMode QtNPBindable::displayMode() const
{
    if (!pi)
        return Embedded;
    return (QtNPBindable::DisplayMode)pi->fMode;
}

/*!
    Returns the mime type this plugin has been instantiated for.
*/
QString QtNPBindable::mimeType() const
{
    if (!pi)
        return QString();
    return pi->mimetype;
}

/*!
    Returns the browser's plugin instance associated with this plugin object.
    The instance is required to call functions in the Netscape Plugin API,
    i.e. NPN_GetJavaPeer().

    The instance returned is only valid as long as this object is.

    See http://devedge-temp.mozilla.org/library/manuals/2002/plugin/1.0/
    for documentation of the \c NPP type.
*/
NPP QtNPBindable::instance() const
{
    if (!pi)
        return 0;
    return pi->npp;
}

/*!
    Reimplement this function to read data from \a source provided with
    mime type \a format. The data is the one specified in the \c src or
    \c data attribute of the \c{<EMBED>} or \c{<OBJECT>} tag of in
    HTML page. This function is called once for every stream the browser
    creates for the plugin.

    Return true to indicate successfull processing of the data, otherwise
    return false. The default implementation does nothing and returns false.
*/

bool QtNPBindable::readData(QIODevice *source, const QString &format)
{
    Q_UNUSED(source);
    Q_UNUSED(format);
    return false;
}

/*!
    Requests that the \a url be retrieved and sent to the named \a window (or
    a new window if \a window is empty), and returns the ID of the request that is
    delivered to transferComplete() when the get-operation has finished. Returns 0 when
    the browser or the system doesn't support notification, or -1 when an error occured.

    \code
    void MyPlugin::aboutQtSoftware()
    {
        openUrl("http://qt.nokia.com");
    }
    \endcode

    See Netscape's JavaScript documentation for an explanation of window names.

    \sa transferComplete() uploadData() uploadFile()
*/
int QtNPBindable::openUrl(const QString &url, const QString &window)
{
    if (!pi)
        return -1;
    QString wnd = window;
    if (wnd.isEmpty())
        wnd = "_blank";

    qint32 id = pi->getNotificationSeqNum();
    NPError err = NPN_GetURLNotify(pi->npp, url.toLocal8Bit().constData(), wnd.toLocal8Bit().constData(), reinterpret_cast<void*>(id));
    if (err != NPERR_NO_ERROR)
        id = -1;

    if (err == NPERR_INCOMPATIBLE_VERSION_ERROR) {
        err = NPN_GetURL(pi->npp, url.toLocal8Bit().constData(), wnd.toLocal8Bit().constData());
        if (NPERR_NO_ERROR == err)
            id = 0;
        else
            id = -1;
    }
    return id;
}

/*!
    Posts \a data to \a url, and displays the result in \a window. Returns the ID of the request
    that is delivered to transferComplete() when the post-operation has finished. Returns 0 when
    the browser or the system doesn't support notification, or -1 when an error occured.

    \code
    void MyPlugin::sendMail()
    {
        uploadData("mailto:fred@somewhere.com", QString(), "There is a new file for you!");
    }
    \endcode

    See Netscape's JavaScript documentation for an explanation of window names.

    \sa transferComplete() openUrl() uploadFile()
*/
int QtNPBindable::uploadData(const QString &url, const QString &window, const QByteArray &data)
{
    if (!pi)
        return -1;

    int id = pi->getNotificationSeqNum();
    if (NPERR_NO_ERROR != NPN_PostURLNotify(pi->npp, url.toLocal8Bit().constData(), window.isEmpty() ? 0 : window.toLocal8Bit().constData(), data.size(), data.constData(), false, reinterpret_cast<void*>(id)))
        id = -1;

    return id;
}

/*!
    Posts \a filename to \a url, and displays the result in \a window. Returns the ID of
    the request that is delivered to transferComplete() when the post-operation has finished.
    Returns 0 when the browser or the system doesn't support notification, or -1 when an
    error occured.

    \code
    void MyPlugin::uploadFile()
    {
        uploadFile("ftp://ftp.somewhere.com/incoming", "response", "c:\\temp\\file.txt");
    }
    \endcode

    See Netscape's JavaScript documentation for an explanation of window names.

    \sa transferComplete() uploadData() openUrl()
*/

int QtNPBindable::uploadFile(const QString &url, const QString &window, const QString &filename)
{
    if (!pi)
        return -1;

    QByteArray data = filename.toLocal8Bit();
    int id = pi->getNotificationSeqNum();
    if (NPERR_NO_ERROR != NPN_PostURLNotify(pi->npp, url.toLocal8Bit().constData(), window.isEmpty() ? 0 : window.toLocal8Bit().constData(), data.size(), data.constData(), true, reinterpret_cast<void*>(id)))
        id = -1;

    return id;
}

/*!
    Called as a result of a call to openUrl, uploadData or uploadFile.
    \a url corresponds to the respective parameter, and \a id to value returned
    by the call. \a reason indicates how the transfer was completed.
*/
void QtNPBindable::transferComplete(const QString &url, int id, Reason reason)
{
    Q_UNUSED(url)
    Q_UNUSED(id)
    Q_UNUSED(reason)
}


/******************************************************************************
 * The plugin itself - only one ever exists, created by QtNPFactory::create()
 *****************************************************************************/


/*!
    \class QtNPFactory qtbrowserplugin.h
    \brief The QtNPFactory class provides the factory for plugin objects.

    Implement this factory once in your plugin project to provide information
    about the plugin and to create the plugin objects. Subclass QtNPFactory and
    implement the pure virtual functions, and export the factory using the
    \c QTNPFACTORY_EXPORT() macro.

    If you use the Q_CLASSINFO macro in your object classes you can use the
    \c QTNPFACTORY_BEGIN(), \c QTNPCLASS() and \c QTNPFACTORY_END() macros to
    generate a factory implementation:

    \code
    class Widget : public QWidget
    {
        Q_OBJECT
        Q_CLASSINFO("MIME", "application/x-graphable:g1n:Graphable data")
    public:
        ...
    };

    QTNPFACTORY_BEGIN("Plugin name", "Plugin description")
        QTNPCLASS(WidgetClass)
    QTNPFACTORY_END()
    \endcode

    The classes exposed must provide a constructor.

    If Qt is linked to the plugin as a dynamic library, only one instance of
    QApplication will exist \e{across all plugins that have been made with Qt}.
    So, your plugin should tread lightly on global settings. Do not, for example,
    use QApplication::setFont() - that will change the font in every widget of
    every Qt-based plugin currently loaded!
*/

/*!
    Creates a QtNPFactory.
*/
QtNPFactory::QtNPFactory()
{
}

/*!
    Destroys the QtNPFactory.

    This is called by the plugin binding code just before the plugin is
    about to be unloaded from memory. If createObject() has been called,
    a QApplication will still exist at this time, but will be deleted
    shortly after, just before the plugin is deleted.
*/
QtNPFactory::~QtNPFactory()
{
}


/*!
    \fn QStringList QtNPFactory::mimeTypes() const

    Reimplement this function to return the MIME types of the data formats
    supported by your plugin. The format of each string is
    mime:extension(s):description:

    \code
    QStringList mimeTypes() const
    {
        QStringList list;
        list << "image/x-png:png:PNG Image"
             << "image/png:png:PNG Image"
             << "image/jpeg:jpg,jpeg:JPEG Image";
	return list;
    }
    \endcode
*/

/*!
    \fn QObject *QtNPFactory::createObject(const QString &type)

    Reimplement this function to return the QObject or QWidget subclass
    supporting the mime type \a type, or 0 if the factory doesn't support
    the type requested.

    \a type will be in the same form as the leftmost (mime) part of
    the string(s) returned by mimeTypes(), e.g. "image/png".
*/

/*!
    \fn QString QtNPFactory::pluginName() const

    Reimplement this function to return the name of the plugin.
*/

/*!
    \fn QString QtNPFactory::pluginDescription() const

    Reimplement this function to return the description of the plugin.
*/
