//
// Created by pwootage on 11/14/17.
//

#ifndef OCJS_V8ENGINENATIVE_H
#define OCJS_V8ENGINENATIVE_H
#define V8_DEPRECATION_WARNINGS
#define V8_IMMINENT_DEPRECATION_WARNINGS
#include <jni.h>
#include <memory>
#include <functional>
#include "include/v8.h"
#include "include/libplatform/libplatform.h"

/**
 * The native equivalent of V8Engine, where all the V8-related classes are stored.
 *
 * We could store these as a bunch of 'long' in the Java class, but that also leads to issues
 * */
class V8EngineNative {
public:
    using JNIPtr = std::unique_ptr<JNIEnv, std::function<void(JNIEnv *)>>;

    V8EngineNative(JNIEnv *env, jobject obj);
    ~V8EngineNative();

    static void Initialize(JNIEnv *env, jclass clazz);
    static V8EngineNative *getFromJava(JNIEnv *env, jobject obj);
    static void setToJava(JNIEnv *env, jobject obj, V8EngineNative *data);

    v8::Isolate *getIsolate();
    v8::Local<v8::String> compileAndExecute(jstring src, jstring filename);

    v8::Global<v8::Context> contextRef;
    v8::Global<v8::Context> kernelContext;
    v8::Global<v8::Context> userContext;
private:
    JavaVM *javaVM;
    jobject globalObjRef;

    v8::Isolate::CreateParams create_params;
    v8::Isolate *isolate;

    JNIPtr getEnv();
    v8::Local<v8::Object> convertException(v8::Local<v8::Context> context, v8::TryCatch &tryCatch);

    static void __call(const v8::FunctionCallbackInfo<v8::Value>& info);
};


#endif //OCJS_V8ENGINENATIVE_H
