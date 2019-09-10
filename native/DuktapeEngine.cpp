#include <jni.h>
#include "com_pwootage_oc_js_duktape_DuktapeEngine.h"
#include "com_pwootage_oc_js_duktape_DuktapeStatic.h"
#include "DuktapeEngineNative.h"
#include "JSValue.hpp"
#include <string>

using namespace std;

void InitializeDuktape(JNIEnv *env, jobject obj);
DukTapeEngineNative *getDuktapeFromJava(JNIEnv *env, jobject obj);
void setDuktapeToJava(JNIEnv *env, jobject obj, DukTapeEngineNative *data);

jobject DukTapeEngineClass = nullptr;
jfieldID DukTapeEngineNativeFID = nullptr;

JNIEXPORT void JNICALL
Java_com_pwootage_oc_js_duktape_DuktapeStatic_native_1init(JNIEnv *env, jobject self) {
  InitializeDuktape(env, self);
}

JNIEXPORT void JNICALL
Java_com_pwootage_oc_js_duktape_DuktapeEngine_native_1start(JNIEnv *env, jobject self) {
  DukTapeEngineNative *engine = getDuktapeFromJava(env, self);
  if (engine == nullptr) {
    printf("Native start\n");
    fflush(stdout);

    engine = new DukTapeEngineNative();
    setDuktapeToJava(env, self, engine);
  }
}


JNIEXPORT void JNICALL
Java_com_pwootage_oc_js_duktape_DuktapeEngine_native_1destroy(JNIEnv *env, jobject self) {
  DukTapeEngineNative *engine = getDuktapeFromJava(env, self);
  if (engine != nullptr) {
    printf("Native destroy\n");
    fflush(stdout);

    delete engine;
    setDuktapeToJava(env, self, nullptr);
  }
}

JNIEXPORT jobject JNICALL
Java_com_pwootage_oc_js_duktape_DuktapeEngine_native_1next(JNIEnv *env, jobject self, jobject next) {
  DukTapeEngineNative *engine = getDuktapeFromJava(env, self);
  if (engine != nullptr) {
    auto value = JSValue::fromJVM(env, next);

//    DukTapeEngineNative::debug_print(u"Recieved next: " + nextVal);
    future<JSValuePtr> resFuture = engine->next(value);
    auto status = resFuture.wait_for(chrono::seconds(1));
    if (status == future_status::ready) {
      JSValuePtr res = resFuture.get();
//      DukTapeEngineNative::debug_print(u"Recieved next result: " + res);
      return res->toJVM(env);
    } else {
      return env->NewStringUTF(R"({"type":"unresponsive"})");
    }
  } else {
    return env->NewStringUTF(R"({"type":"no_engine"})");
  }
}


void InitializeDuktape(JNIEnv *env, jobject obj) {
  JSValue::jvmInit(env);

  jclass local = env->FindClass("com/pwootage/oc/js/duktape/DuktapeEngine");
  // this global ref is never freed (naturally)
  DukTapeEngineClass = env->NewGlobalRef(local);
  // local should be auto-cleaned up
  DukTapeEngineNativeFID = env->GetFieldID(local, "duktapeEngineNative", "J");
}

DukTapeEngineNative *getDuktapeFromJava(JNIEnv *env, jobject obj) {
  return reinterpret_cast<DukTapeEngineNative *>(env->GetLongField(obj, DukTapeEngineNativeFID));
}

void setDuktapeToJava(JNIEnv *env, jobject obj, DukTapeEngineNative *data) {
  env->SetLongField(obj, DukTapeEngineNativeFID, reinterpret_cast<jlong>(data));
}
