//
// $Id$
//

#include "xpshell.h"
#include "acIJSContextPublisher.h"
#include "FileHelper.h"

#include <nsIXPConnect.h>
#include <nsIXPCScriptable.h>
#include <nsIInterfaceInfo.h>
#include <nsIInterfaceInfoManager.h>
#include <nsIXPCScriptable.h>
#include <nsIServiceManager.h>
#include <nsIComponentManager.h>
#include <nsIComponentRegistrar.h>
#include <nsLocalFile.h>

#include <prenv.h>
#include <jsarena.h>
#include <jsapi.h>
#include <jsprf.h>
#include <nscore.h>
#include <nsMemory.h>
#include <nsIGenericFactory.h>
#include <nsIJSRuntimeService.h>
#include <nsCOMPtr.h>
#include <nsIXPCSecurityManager.h>

#include <jsparse.h>
#include <jsscan.h>
#include <jsemit.h>
#include <jsscript.h>
#include <jsarena.h>
#include <jscntxt.h>
#include <jsdbgapi.h>

#include <nsIJSContextStack.h>

#include <errno.h>
#include <stdio.h>
#include <iostream>
#include <libgen.h>

#include <string>

JSContext * ourJSContext(0);

/***************************************************************************/

#ifdef JS_THREADSAFE
#define DoBeginRequest(cx) JS_BeginRequest((cx))
#define DoEndRequest(cx)   JS_EndRequest((cx))
#else
#define DoBeginRequest(cx) ((void)0)
#define DoEndRequest(cx)   ((void)0)
#endif

/***************************************************************************/

#define EXITCODE_RUNTIME_ERROR 3
#define EXITCODE_FILE_NOT_FOUND 4

using namespace std;

FILE *gOutFile = NULL;
FILE *gErrFile = NULL;

int gExitCode = 0;
JSBool gQuitting = JS_FALSE;
static JSBool reportWarnings = JS_TRUE;

JSContext * getJSContext()
{
    return ourJSContext;
}


JS_STATIC_DLL_CALLBACK(void)
my_ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report)
{
    int i, j, k, n;
    char *prefix = NULL, *tmp;
    const char *ctmp;

    if (!report) {
        fprintf(gErrFile, "%s\n", message);
        return;
    }

    /* Conditionally ignore reported warnings. */
    if (JSREPORT_IS_WARNING(report->flags) && !reportWarnings)
        return;

    if (report->filename)
        prefix = JS_smprintf("%s:", report->filename);
    if (report->lineno) {
        tmp = prefix;
        prefix = JS_smprintf("%s%u: ", tmp ? tmp : "", report->lineno);
        JS_free(cx, tmp);
    }
    if (JSREPORT_IS_WARNING(report->flags)) {
        tmp = prefix;
        prefix = JS_smprintf("%s%swarning: ",
                             tmp ? tmp : "",
                             JSREPORT_IS_STRICT(report->flags) ? "strict " : "");
        JS_free(cx, tmp);
    }

    /* embedded newlines -- argh! */
    while ((ctmp = strchr(message, '\n')) != 0) {
        ctmp++;
        if (prefix) fputs(prefix, gErrFile);
        fwrite(message, 1, ctmp - message, gErrFile);
        message = ctmp;
    }
    /* If there were no filename or lineno, the prefix might be empty */
    if (prefix)
        fputs(prefix, gErrFile);
    fputs(message, gErrFile);

    if (!report->linebuf) {
        fputc('\n', gErrFile);
        goto out;
    }

    fprintf(gErrFile, ":\n%s%s\n%s", prefix, report->linebuf, prefix);
    n = report->tokenptr - report->linebuf;
    for (i = j = 0; i < n; i++) {
        if (report->linebuf[i] == '\t') {
            for (k = (j + 8) & ~7; j < k; j++) {
                fputc('.', gErrFile);
            }
            continue;
        }
        fputc('.', gErrFile);
        j++;
    }
    fputs("^\n", gErrFile);
 out:
    if (!JSREPORT_IS_WARNING(report->flags))
        gExitCode = EXITCODE_RUNTIME_ERROR;
    JS_free(cx, prefix);
}

JSStackFrame * getStackFrame(int i, JSContext *cx) {
    JSStackFrame* fp;
    JSStackFrame* iter = nsnull;
    int num = 0;

    while(nsnull != (fp = JS_FrameIterator(cx, &iter)))
    {
        if (num == i) {
            return fp;
        }
        ++num;
    }
    return 0;
}

bool
getFileLine(JSContext *cx, uintN argc, jsval *argv, 
        const char * & filename, int & lineno) 
{
    uint16 n = 1;
    if (argc > 0 && JSVAL_IS_INT(argv[0])) {
        JS_ValueToUint16(cx, argv[0], &n);
    }

    JSStackFrame * fp = getStackFrame(n,cx);

    if (fp) {
        if(!JS_IsNativeFrame(cx, fp)) {
            JSScript* script = JS_GetFrameScript(cx, fp);
            jsbytecode* pc = JS_GetFramePC(cx, fp);
            if(script && pc)
            {
                filename = JS_GetScriptFilename(cx, script);
                lineno =  (PRInt32) JS_PCToLineNumber(cx, script, pc);
                return true;
            }
        }
    }
    return false;
}

JS_STATIC_DLL_CALLBACK(JSBool)
Print(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    uintN i, n;
    JSString *str;

    for (i = n = 0; i < argc; i++) {
        str = JS_ValueToString(cx, argv[i]);
        if (!str)
            return JS_FALSE;
        fprintf(gOutFile, "%s%s", i ? " " : "", JS_GetStringBytes(str));
    }
    n++;
    if (n)
        fputc('\n', gOutFile);
    return JS_TRUE;
}

JS_STATIC_DLL_CALLBACK(JSBool)
Dump(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSString *str;
    if (!argc)
        return JS_TRUE;

    str = JS_ValueToString(cx, argv[0]);
    if (!str)
        return JS_FALSE;

    char *bytes = JS_GetStringBytes(str);
    bytes = strdup(bytes);

    fputs(bytes, gOutFile);
    nsMemory::Free(bytes);
    return JS_TRUE;
}


string findIncludeFile(const char * pFilename,
        JSContext *cx, JSObject *obj, uintN argc, jsval *argv)
{
    string IncludePath;
    if (IncludePath=="") {
        char * pIncludePath = getenv("AVG_INCLUDE_PATH");
        if (pIncludePath) {
            IncludePath = pIncludePath;
            IncludePath += ";";
        } else {
            cerr << "Warning: AVG_INCLUDE_PATH not set." << endl;
        }
    }
    
    const char * pCurFilename;
    int lineno;
    bool bOk = getFileLine(cx, argc, argv, pCurFilename, lineno); 
    if (bOk) {
        IncludePath += getPath(pCurFilename);
    }
    string FoundFilename = findFile(pFilename, IncludePath);
    if (FoundFilename == "") {
        cerr << "xpshell Warning: could not find include file " << pFilename 
            << endl << "in " << IncludePath << endl;
    }
    return FoundFilename; 
}

JS_STATIC_DLL_CALLBACK(JSBool)
Use(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    uintN i;
    JSString *str;
    const char *filename;
    JSScript *script;
    JSBool ok;
    jsval result;

    for (i = 0; i < argc; i++) {
        str = JS_ValueToString(cx, argv[i]);
        if (!str)
            return JS_FALSE;
        argv[i] = STRING_TO_JSVAL(str);
        filename = JS_GetStringBytes(str);
        string sFilename = findIncludeFile(filename, cx, obj, argc, argv);
        script = JS_CompileFile(cx, obj, sFilename.c_str());
        if (!script)
            ok = JS_FALSE;
        else {
            ok = JS_ExecuteScript(cx, obj, script, &result);
            JS_DestroyScript(cx, script);
        }
        if (!ok)
            return JS_FALSE;
    }
    return JS_TRUE;
}

JS_STATIC_DLL_CALLBACK(JSBool)
Version(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    if (argc > 0 && JSVAL_IS_INT(argv[0]))
        *rval = INT_TO_JSVAL(JS_SetVersion(cx, JSVersion(JSVAL_TO_INT(argv[0]))));
    else
        *rval = INT_TO_JSVAL(JS_GetVersion(cx));
    return JS_TRUE;
}

JS_STATIC_DLL_CALLBACK(JSBool)
BuildDate(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    fprintf(gOutFile, "built on %s at %s\n", __DATE__, __TIME__);
    return JS_TRUE;
}

JS_STATIC_DLL_CALLBACK(JSBool)
Quit(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
#ifdef LIVECONNECT
    JSJ_SimpleShutdown();
#endif

    gExitCode = 0;
    JS_ConvertArguments(cx, argc, argv,"/ i", &gExitCode);

    gQuitting = JS_TRUE;
//    exit(0);
    return JS_FALSE;
}

JS_STATIC_DLL_CALLBACK(JSBool)
DumpXPC(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    int32 depth = 2;

    if (argc > 0) {
        if (!JS_ValueToInt32(cx, argv[0], &depth))
            return JS_FALSE;
    }

    nsCOMPtr<nsIXPConnect> xpc = do_GetService(nsIXPConnect::GetCID());
    if(xpc)
        xpc->DebugDump((int16)depth);
    return JS_TRUE;
}

#ifdef GC_MARK_DEBUG
extern "C" JS_FRIEND_DATA(FILE *) js_DumpGCHeap;
#endif

JS_STATIC_DLL_CALLBACK(JSBool)
GC(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSRuntime *rt;
    uint32 preBytes;
    rt = cx->runtime;
    preBytes = rt->gcBytes;
#ifdef GC_MARK_DEBUG
    if (argc && JSVAL_IS_STRING(argv[0])) {
        char *name = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
        FILE *file = fopen(name, "w");
        if (!file) {
            fprintf(gErrFile, "gc: can't open %s: %s\n", strerror(errno));
            return JS_FALSE;
        }
        js_DumpGCHeap = file;
    } else {
        js_DumpGCHeap = stdout;
    }
#endif
    JS_GC(cx);
#ifdef GC_MARK_DEBUG
    if (js_DumpGCHeap != stdout)
        fclose(js_DumpGCHeap);
    js_DumpGCHeap = NULL;
#endif

#ifdef JS_GCMETER
    js_DumpGCStats(rt, stdout);
#endif
    return JS_TRUE;
}

JS_STATIC_DLL_CALLBACK(JSBool)
Clear(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    if (argc > 0 && !JSVAL_IS_PRIMITIVE(argv[0])) {
        JS_ClearScope(cx, JSVAL_TO_OBJECT(argv[0]));
    } else {
        JS_ReportError(cx, "'clear' requires an object");
        return JS_FALSE;
    }
    return JS_TRUE;
}

static JSFunctionSpec glob_functions[] = {
    {"print",           Print,          0},
    {"use",             Use,            1},
    {"quit",            Quit,           0},
    {"version",         Version,        1},
    {"build",           BuildDate,      0},
    {"dumpXPC",         DumpXPC,        1},
    {"dump",            Dump,           1},
    {"gc",              GC,             0},
    {"clear",           Clear,          1},
    {0}
};

static JSClass global_class = {
    "global", 0,
    JS_PropertyStub,  JS_PropertyStub,  JS_PropertyStub,  JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,   JS_FinalizeStub
};

static void
Process(JSContext *cx, JSObject *obj, const char *filename)
{
    JSScript *script;
    jsval result;
    DoBeginRequest(cx);
    script = JS_CompileFile(cx, obj, filename);
    if (script) {
	    (void)JS_ExecuteScript(cx, obj, script, &result);
	    JS_DestroyScript(cx, script);
    }
    DoEndRequest(cx);
}

static int
usage(void)
{
    fprintf(gErrFile, "%s\n", JS_GetImplementationVersion());
    fprintf(gErrFile, "usage: xpshell scriptfile\n");
    return 2;
}

static int
processArguments(JSContext * theContext, JSObject * theObject, char **argv, int argc) {
    if (argc < 2) {
        usage();
        return -1;
    }

    JS_ToggleOptions(theContext, JSOPTION_STRICT);
    std::string myFilename = argv[1];

    jsval * myVector = 0;
    unsigned myScriptArgCount = argc - 1;
/*
    if (myScriptArgCount > 0) {
        myVector = static_cast<jsval*>(JS_malloc(theContext, myScriptArgCount * sizeof(jsval)));
        if ( ! myVector) {
            return 1;
        }

        jsval * myIt = myVector;
        for (unsigned i = 2; i < argc; ++i) {
            JSString * myString = JS_NewStringCopyZ(theContext, argv[i]);
            if ( ! myString) {
                return 1;
            }
            *myIt++ = STRING_TO_JSVAL(myString);
        }
    }
    JSObject * myArgsObject = JS_NewArrayObject(theContext, myScriptArgCount, myVector);
    if (myVector) {
        JS_free(theContext, myVector);
    }
    if ( ! myArgsObject) {
        return 1;
    }

    if ( ! JS_DefineProperty(theContext, theObject, "arguments",
        OBJECT_TO_JSVAL(myArgsObject), 0, 0, 0))
    {
        return 1;
    }
*/
    Process(theContext, theObject, myFilename.c_str());
    return gExitCode;
}


/***************************************************************************/

class FullTrustSecMan : public nsIXPCSecurityManager
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIXPCSECURITYMANAGER
  FullTrustSecMan();
};

NS_IMPL_ISUPPORTS1(FullTrustSecMan, nsIXPCSecurityManager);

FullTrustSecMan::FullTrustSecMan()
{
}

NS_IMETHODIMP
FullTrustSecMan::CanCreateWrapper(JSContext * aJSContext, const nsIID & aIID, nsISupports *aObj, nsIClassInfo *aClassInfo, void * *aPolicy)
{
    return NS_OK;
}

NS_IMETHODIMP
FullTrustSecMan::CanCreateInstance(JSContext * aJSContext, const nsCID & aCID)
{
    return NS_OK;
}

NS_IMETHODIMP
FullTrustSecMan::CanGetService(JSContext * aJSContext, const nsCID & aCID)
{
    return NS_OK;
}

/* void CanAccess (in PRUint32 aAction, in nsIXPCNativeCallContext aCallContext, in JSContextPtr aJSContext, in JSObjectPtr aJSObject, in nsISupports aObj, in nsIClassInfo aClassInfo, in JSVal aName, inout voidPtr aPolicy); */
NS_IMETHODIMP
FullTrustSecMan::CanAccess(PRUint32 aAction, nsIXPCNativeCallContext *aCallContext, JSContext * aJSContext, JSObject * aJSObject, nsISupports *aObj, nsIClassInfo *aClassInfo, jsval aName, void * *aPolicy)
{
    return NS_OK;
}

/***************************************************************************/

nsILocalFile *
getComponentsDir() {
    const char * theXPComponentsDir = PR_GetEnv("XP_COMPONENTS_DIR");
    if (theXPComponentsDir) {
        cout << "Componentsdir environmentvar XP_COMPONENTS_DIR: " << theXPComponentsDir << endl;
    } else {
        cout << "XP_COMPONENTS_DIR not set" << endl;
    }
    nsILocalFile * theComponentsDir;
    if (theXPComponentsDir) {
        nsresult rc = NS_NewNativeLocalFile(nsCString(theXPComponentsDir), false, &theComponentsDir);
        if (NS_SUCCEEDED(rc)) {
            return theComponentsDir;
        } else {
            cerr << "Warning: Could not open components directory." << endl;
            return 0;
        }
    } else {
        return 0;
    }
}

void
registerComponents(nsCOMPtr<nsIServiceManager> servMan) {
    nsCOMPtr<nsIComponentRegistrar> registrar = do_QueryInterface(servMan);

    NS_ASSERTION(registrar, "Null nsIComponentRegistrar");
    if (registrar) {
        nsILocalFile * theComponentsDir = getComponentsDir();
        if (theComponentsDir) {
            registrar->AutoRegister(theComponentsDir);
            theComponentsDir->Release();
        } else {
            registrar->AutoRegister(nsnull);
        }
    }
}


int
main(int argc, char **argv)
{
    JSRuntime *rt;
    JSObject *glob;
    int result;
    nsresult rv;

    gErrFile = stderr;
    gOutFile = stdout;
    {
        nsCOMPtr<nsIServiceManager> servMan;
        rv = NS_InitXPCOM2(getter_AddRefs(servMan), getComponentsDir(), nsnull);
        if (NS_FAILED(rv)) {
            printf("NS_InitXPCOM failed!\n");
            return 1;
        }
        registerComponents(servMan);

        nsCOMPtr<nsIJSRuntimeService> rtsvc = do_GetService("@mozilla.org/js/xpc/RuntimeService;1");
        // get the JSRuntime from the runtime svc
        if (!rtsvc) {
            printf("failed to get nsJSRuntimeService!\n");
            return 1;
        }

        if (NS_FAILED(rtsvc->GetRuntime(&rt)) || !rt) {
            printf("failed to get JSRuntime from nsJSRuntimeService!\n");
            return 1;
        }

        ourJSContext = JS_NewContext(rt, 8192);
        if (!ourJSContext) {
            printf("JS_NewContext failed!\n");
            return 1;
        }

        // publish our JavaScript context
        nsresult myErr;
        nsCOMPtr<acIJSContextPublisher> myJSContextPublisher =
            do_CreateInstance("@artcom.com/jscontextpublisher;1", &myErr);
        if (NS_FAILED(myErr)) {
            cerr << "Warning: could not publish js context." << endl;
        } else {
            myJSContextPublisher->SetContext((PRInt32)ourJSContext);
        }

        JS_SetErrorReporter(ourJSContext, my_ErrorReporter);

        nsCOMPtr<nsIXPConnect> xpc = do_GetService(nsIXPConnect::GetCID());
        if (!xpc) {
            printf("failed to get nsXPConnect service!\n");
            return 1;
        }

        // Since the caps security system might set a default security manager
        // we will be sure that the secman on this context gives full trust.
        // That way we can avoid getting principals from the caps security manager
        // just to shut it up. Also, note that even though our secman will allow
        // anything, we set the flags to '0' so it ought never get called anyway.
        nsCOMPtr<nsIXPCSecurityManager> secman =
            NS_STATIC_CAST(nsIXPCSecurityManager*, new FullTrustSecMan());
        xpc->SetSecurityManagerForJSContext(ourJSContext, secman, 0);

        // xpc->SetCollectGarbageOnMainThreadOnly(PR_TRUE);
        // xpc->SetDeferReleasesUntilAfterGarbageCollection(PR_TRUE);

        nsCOMPtr<nsIJSContextStack> cxstack = do_GetService("@mozilla.org/js/xpc/ContextStack;1");
        if (!cxstack) {
            printf("failed to get the nsThreadJSContextStack service!\n");
            return 1;
        }

        if(NS_FAILED(cxstack->Push(ourJSContext))) {
            printf("failed to push the current JSContext on the nsThreadJSContextStack!\n");
            return 1;
        }

        glob = JS_NewObject(ourJSContext, &global_class, NULL, NULL);
        if (!glob)
            return 1;
        if (!JS_InitStandardClasses(ourJSContext, glob))
            return 1;
        if (!JS_DefineFunctions(ourJSContext, glob, glob_functions))
            return 1;
        if (NS_FAILED(xpc->InitClasses(ourJSContext, glob)))
            return 1;

//        argc--;
//        argv++;
        result = processArguments(ourJSContext, glob, argv, argc);

        JS_ClearScope(ourJSContext, glob);
        JS_GC(ourJSContext);
        JSContext *oldcx;
        cxstack->Pop(&oldcx);
        NS_ASSERTION(oldcx == ourJSContext, "JS thread context push/pop mismatch");
        cxstack = nsnull;
        JS_GC(ourJSContext);
        JS_DestroyContext(ourJSContext);
        xpc->SyncJSContexts();

    } // this scopes the nsCOMPtrs
    // no nsCOMPtrs are allowed to be alive when you call NS_ShutdownXPCOM
    rv = NS_ShutdownXPCOM( NULL );
    NS_ASSERTION(NS_SUCCEEDED(rv), "NS_ShutdownXPCOM failed");

    return result;
}

/***************************************************************************/

