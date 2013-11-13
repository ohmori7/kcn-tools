
#ifndef EL__ECMASCRIPT_SPIDERMONKEY_UTIL_H
#define EL__ECMASCRIPT_SPIDERMONKEY_UTIL_H

/* For wild SpiderMonkey installations. */
#ifdef CONFIG_BEOS
#define XP_BEOS
#elif CONFIG_OS2
#define XP_OS2
#elif CONFIG_RISCOS
#error Out of luck, buddy!
#elif CONFIG_UNIX
#define XP_UNIX
#elif CONFIG_WIN32
#define XP_WIN
#endif

#include <jsapi.h>
#include "util/memory.h"
#include "util/string.h"

static void string_to_jsval(JSContext *ctx, jsval *vp, unsigned char *string);
static void astring_to_jsval(JSContext *ctx, jsval *vp, unsigned char *string);
static void int_to_jsval(JSContext *ctx, jsval *vp, int number);
static void object_to_jsval(JSContext *ctx, jsval *vp, JSObject *object);
static void boolean_to_jsval(JSContext *ctx, jsval *vp, int boolean);
static void undef_to_jsval(JSContext *ctx, jsval *vp);

static int jsval_to_boolean(JSContext *ctx, jsval *vp);
static unsigned char *jsval_to_string(JSContext *ctx, jsval *vp);



/** Inline functions */

static inline void
string_to_jsval(JSContext *ctx, jsval *vp, unsigned char *string)
{
	if (!string) {
		*vp = JSVAL_NULL;
	} else {
		*vp = STRING_TO_JSVAL(JS_NewStringCopyZ(ctx, string));
	}
}

static inline void
astring_to_jsval(JSContext *ctx, jsval *vp, unsigned char *string)
{
	string_to_jsval(ctx, vp, string);
	mem_free_if(string);
}

static inline void
int_to_jsval(JSContext *ctx, jsval *vp, int number)
{
	*vp = INT_TO_JSVAL(number);
}

static inline void
object_to_jsval(JSContext *ctx, jsval *vp, JSObject *object)
{
	*vp = OBJECT_TO_JSVAL(object);
}

static inline void
boolean_to_jsval(JSContext *ctx, jsval *vp, int boolean)
{
	*vp = BOOLEAN_TO_JSVAL(boolean);
}

static inline void
undef_to_jsval(JSContext *ctx, jsval *vp)
{
	*vp = JSVAL_NULL;
}


static inline int
jsval_to_boolean(JSContext *ctx, jsval *vp)
{
	jsval val;

	if (JS_ConvertValue(ctx, *vp, JSTYPE_BOOLEAN, &val) == JS_FALSE) {
		return JS_FALSE;
	}

	return JSVAL_TO_BOOLEAN(val);
}

static inline unsigned char *
jsval_to_string(JSContext *ctx, jsval *vp)
{
	jsval val;

	if (JS_ConvertValue(ctx, *vp, JSTYPE_STRING, &val) == JS_FALSE) {
		return "";
	}

	return empty_string_or_(JS_GetStringBytes(JS_ValueToString(ctx, val)));
}

/** An ELinks-specific replacement for JSFunctionSpec.
 *
 * Bug 1016: In SpiderMonkey 1.7 bundled with XULRunner 1.8, jsapi.h
 * defines JSFunctionSpec in different ways depending on whether
 * MOZILLA_1_8_BRANCH is defined, and there is no obvious way for
 * ELinks to check whether MOZILLA_1_8_BRANCH was defined when the
 * library was built.  Avoid the unstable JSFunctionSpec definitions
 * and use this ELinks-specific structure instead.  */
typedef struct spidermonkeyFunctionSpec {
	const char *name;
	JSNative call;
	uint8 nargs;
	/* ELinks does not use "flags" and "extra" so omit them here.  */
} spidermonkeyFunctionSpec;

JSBool spidermonkey_DefineFunctions(JSContext *cx, JSObject *obj,
				    const spidermonkeyFunctionSpec *fs);
JSObject *spidermonkey_InitClass(JSContext *cx, JSObject *obj,
				 JSObject *parent_proto, JSClass *clasp,
				 JSNative constructor, uintN nargs,
				 JSPropertySpec *ps,
				 const spidermonkeyFunctionSpec *fs,
				 JSPropertySpec *static_ps,
				 const spidermonkeyFunctionSpec *static_fs);

#endif
