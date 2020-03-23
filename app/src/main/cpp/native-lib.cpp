#include <jni.h>
#include <string>

#include "util.h"
#include "cpp_master_node.h"

//JNIEnv *print_env;
JavaVM *jvm;
jobject print_ob;

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_serverside_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_serverside_MainActivity_cppMainFunc(JNIEnv *env, jobject thiz) {
    env->GetJavaVM(&jvm);
    print_ob = env->NewGlobalRef(thiz);
//    print_env = env;
//    print_ob = thiz;

    mainfunc();

}