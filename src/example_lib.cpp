#include <jni.h>
#include <android/log.h>
#include <thread>
#include <unistd.h>
#include <dlfcn.h>

#define LOG_TAG "ExampleLib"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Global JavaVM pointer
static JavaVM* g_jvm = nullptr;

// Example hook function
void exampleHookFunction() {
    LOGI("Example hook function called!");
    
    // Your custom code here
    // This could modify game behavior, intercept function calls, etc.
}

// Background thread function
void backgroundThread() {
    LOGI("Background thread started!");
    
    // Wait for app to fully initialize
    sleep(2);
    
    if (g_jvm) {
        JNIEnv* env = nullptr;
        bool attached = false;
        
        // Attach to current thread if needed
        int status = g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);
        if (status == JNI_EDETACHED) {
            LOGI("Attaching thread to JVM...");
            if (g_jvm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
                attached = true;
                LOGI("Thread attached successfully");
            } else {
                LOGE("Failed to attach thread");
                return;
            }
        } else if (status == JNI_OK) {
            LOGI("Thread already attached");
        } else {
            LOGE("Failed to get JNI environment");
            return;
        }
        
        if (env) {
            // Example: Find and call Java methods
            LOGI("JNIEnv obtained: %p", env);
            
            // Example: Find a class
            // jclass clazz = env->FindClass("com/example/MainActivity");
            // if (clazz) {
            //     LOGI("Found MainActivity class");
            //     // Call methods, modify fields, etc.
            // }
            
            // Your injection logic here
            exampleHookFunction();
        }
        
        // Detach if we attached
        if (attached) {
            g_jvm->DetachCurrentThread();
            LOGI("Thread detached");
        }
    }
    
    // Main loop - keep library loaded
    while (true) {
        sleep(10);
        // Your periodic tasks here
    }
}

// Constructor - called when library is loaded
__attribute__((constructor))
void onLibraryLoad() {
    LOGI("===========================================");
    LOGI("Example Library Constructor Called!");
    LOGI("===========================================");
}

// JNI_OnLoad - called by Android runtime
extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("===========================================");
    LOGI("JNI_OnLoad Called!");
    LOGI("JavaVM: %p, Reserved: %p", vm, reserved);
    LOGI("===========================================");
    
    // Check if called by our injector (key = 1337)
    if (reserved != (void*)1337) {
        LOGI("Not called by injector, loading normally");
    } else {
        LOGI("Called by injector - custom initialization");
    }
    
    // Save JavaVM for later use
    g_jvm = vm;
    
    // Get JNI environment
    JNIEnv* env = nullptr;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_OK) {
        LOGI("JNIEnv obtained successfully: %p", env);
        
        // Your initialization code here
        
    } else {
        LOGE("Failed to get JNIEnv");
    }
    
    // Start background thread
    // IMPORTANT: Start thread here, NOT in constructor!
    // Constructor may be called before JVM is ready
    std::thread(backgroundThread).detach();
    LOGI("Background thread started");
    
    return JNI_VERSION_1_6;
}

// JNI_OnUnload - called when library is unloaded
extern "C" JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved) {
    LOGI("JNI_OnUnload Called!");
    g_jvm = nullptr;
}
