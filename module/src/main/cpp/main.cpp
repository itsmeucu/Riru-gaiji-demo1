#include <jni.h>
#include <sys/types.h>
#include <riru.h>
#include <malloc.h>
#include <cstring>
#include <config.h>
#include <android/log.h>
#include <iostream>
#include <map>
#include "dobby/dobby.h"

#define TAG "riru-gaiji"


#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static const char* PROP_CONF_PATH = "/data/local/tmp/prop-conf.prop";
static const char* BUILD_CONF_PATH = "/data/local/tmp/build-conf.prop";

static std::map<std::string,std::string> propMap;
static std::map<std::string,std::string> buildMap;

static void forkAndSpecializePre(
        JNIEnv *env, jclass clazz, jint *uid, jint *gid, jintArray *gids, jint *runtimeFlags,
        jobjectArray *rlimits, jint *mountExternal, jstring *seInfo, jstring *niceName,
        jintArray *fdsToClose, jintArray *fdsToIgnore, jboolean *is_child_zygote,
        jstring *instructionSet, jstring *appDataDir, jboolean *isTopApp, jobjectArray *pkgDataInfoList,
        jobjectArray *whitelistedDataInfoList, jboolean *bindMountAppDataDirs, jboolean *bindMountAppStorageDirs) {
    // Called "before" com_android_internal_os_Zygote_nativeForkAndSpecialize in frameworks/base/core/jni/com_android_internal_os_Zygote.cpp
    // Parameters are pointers, you can change the value of them if you want
    // Some parameters are not exist is older Android versions, in this case, they are null or 0
        LOGE("forkAndSpecializePre");

}

static int (*origin__system_property_get)(const char* __name, char* __value);

static int my__system_property_get(const char* name, char* value){

    if(NULL == name || NULL == value){
        return origin__system_property_get(name,value);
    }

    auto ret = propMap.find(name);
    if(ret != propMap.end()){
       LOGD("find key '%s'",ret->first.c_str());

        const char* valueChs = ret->second.c_str();
        strcpy(value,valueChs);
        return strlen(valueChs);
    }


    return origin__system_property_get(name,value);
}


void initDobby(){
    void* sym = DobbySymbolResolver(NULL,"__system_property_get");
    if(NULL != sym) {
        LOGE("hook start");
        DobbyHook(sym, (void *) my__system_property_get, (void **) &origin__system_property_get);
    }
}

void readPropFile(const char* filename,std::map<std::string,std::string> &map){
    FILE *fp = fopen(filename,"r");
    if(fp == NULL){
        LOGW("open config file fail!");
        return;
    }
    char buf[256];

    while(fgets(buf,256,fp) != NULL){
        char *sep = strchr(buf,'=');
        *sep = '\0';
        std::string key(buf);
        std::string value(sep + 1);
        LOGD("put key = '%s' , value = '%s' to map",key.c_str(),value.c_str());
        map[key] = value;
    }
    fclose(fp);
}

static void setBuild(JNIEnv* env){
    jclass BuildClass = env->FindClass("android/os/Build");
    std::map<std::string,std::string>::iterator it;
    for(it = buildMap.begin();it != buildMap.end();it++){
        LOGD("read build prop -> key = %s,value = %s",it->first.c_str(),it->second.c_str());
        jstring key = env->NewStringUTF(it->first.c_str());
        jstring value = env->NewStringUTF(it->second.c_str());
        jfieldID buildField = env->GetStaticFieldID(BuildClass,it->first.c_str(),"Ljava/lang/String;");
        env->SetStaticObjectField(BuildClass,buildField,value);
    }
}

static void forkAndSpecializePost(JNIEnv *env, jclass clazz, jint res) {
    // Called "after" com_android_internal_os_Zygote_nativeForkAndSpecialize in frameworks/base/core/jni/com_android_internal_os_Zygote.cpp
    // "res" is the return value of com_android_internal_os_Zygote_nativeForkAndSpecialize
        LOGE("forkAndSpecializePost");

    if (res == 0) {
        // In app process
        readPropFile(PROP_CONF_PATH,propMap);
        readPropFile(BUILD_CONF_PATH,buildMap);
        setBuild(env);
        initDobby();

        // When unload allowed is true, the module will be unloaded (dlclose) by Riru
        // If this modules has hooks installed, DONOT set it to true, or there will be SIGSEGV
        // This value will be automatically reset to false before the "pre" function is called
        riru_set_unload_allowed(false);
    } else {
        // In zygote process
    }
}

static void specializeAppProcessPre(
        JNIEnv *env, jclass clazz, jint *uid, jint *gid, jintArray *gids, jint *runtimeFlags,
        jobjectArray *rlimits, jint *mountExternal, jstring *seInfo, jstring *niceName,
        jboolean *startChildZygote, jstring *instructionSet, jstring *appDataDir,
        jboolean *isTopApp, jobjectArray *pkgDataInfoList, jobjectArray *whitelistedDataInfoList,
        jboolean *bindMountAppDataDirs, jboolean *bindMountAppStorageDirs) {
    // Called "before" com_android_internal_os_Zygote_nativeSpecializeAppProcess in frameworks/base/core/jni/com_android_internal_os_Zygote.cpp
    // Parameters are pointers, you can change the value of them if you want
    // Some parameters are not exist is older Android versions, in this case, they are null or 0
}

static void specializeAppProcessPost(
        JNIEnv *env, jclass clazz) {
    // Called "after" com_android_internal_os_Zygote_nativeSpecializeAppProcess in frameworks/base/core/jni/com_android_internal_os_Zygote.cpp
    LOGE("specializeAppProcessPost");

    // When unload allowed is true, the module will be unloaded (dlclose) by Riru
    // If this modules has hooks installed, DONOT set it to true, or there will be SIGSEGV
    // This value will be automatically reset to false before the "pre" function is called
    riru_set_unload_allowed(false);
}

static void forkSystemServerPre(
        JNIEnv *env, jclass clazz, uid_t *uid, gid_t *gid, jintArray *gids, jint *runtimeFlags,
        jobjectArray *rlimits, jlong *permittedCapabilities, jlong *effectiveCapabilities) {
    // Called "before" com_android_internal_os_Zygote_forkSystemServer in frameworks/base/core/jni/com_android_internal_os_Zygote.cpp
    // Parameters are pointers, you can change the value of them if you want
    // Some parameters are not exist is older Android versions, in this case, they are null or 0
}

static void forkSystemServerPost(JNIEnv *env, jclass clazz, jint res) {
    // Called "after" com_android_internal_os_Zygote_forkSystemServer in frameworks/base/core/jni/com_android_internal_os_Zygote.cpp

    if (res == 0) {
        // In system server process
    } else {
        // In zygote process
    }
}

static void onModuleLoaded() {
    // Called when this library is loaded and "hidden" by Riru (see Riru's hide.cpp)

    // If you want to use threads, start them here rather than the constructors
    // __attribute__((constructor)) or constructors of static variables,
    // or the "hide" will cause SIGSEGV
}

extern "C" {

int riru_api_version;
const char *riru_magisk_module_path = nullptr;
int *riru_allow_unload = nullptr;

static auto module = RiruVersionedModuleInfo{
        .moduleApiVersion = riru::moduleApiVersion,
        .moduleInfo= RiruModuleInfo{
                .supportHide = true,
                .version = riru::moduleVersionCode,
                .versionName = riru::moduleVersionName,
                .onModuleLoaded = onModuleLoaded,
                .forkAndSpecializePre = forkAndSpecializePre,
                .forkAndSpecializePost = forkAndSpecializePost,
                .forkSystemServerPre = forkSystemServerPre,
                .forkSystemServerPost = forkSystemServerPost,
                .specializeAppProcessPre = specializeAppProcessPre,
                .specializeAppProcessPost = specializeAppProcessPost
        }
};

RiruVersionedModuleInfo *init(Riru *riru) {
    auto core_max_api_version = riru->riruApiVersion;
    riru_api_version = core_max_api_version <= riru::moduleApiVersion ? core_max_api_version : riru::moduleApiVersion;
    module.moduleApiVersion = riru_api_version;

    riru_magisk_module_path = strdup(riru->magiskModulePath);
    if (riru_api_version >= 25) {
        riru_allow_unload = riru->allowUnload;
    }
    return &module;
}
}