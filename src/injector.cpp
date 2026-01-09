#include "injector.h"
#include "ptrace_utils.h"
#include "process_utils.h"
#include "elf_utils.h"
#include <android/log.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

#define LOG_TAG "LibraryInjector"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace Injector {

LibraryInjector::LibraryInjector() {
    LOGI("LibraryInjector initialized");
}

LibraryInjector::~LibraryInjector() {
    LOGI("LibraryInjector destroyed");
}

bool LibraryInjector::inject(const InjectionConfig& config) {
    // Set SELinux context if needed
    ProcessUtils::setSelinuxContext("u:r:su:s0");
    
    if (config.watchLaunch && !config.packageName.empty()) {
        return watchAndInject(config.packageName, config.libraryPath, config);
    }
    
    if (!config.packageName.empty()) {
        return injectByPackage(config.packageName, config.libraryPath, config);
    }
    
    if (config.pid > 0) {
        return injectByPid(config.pid, config.libraryPath, config);
    }
    
    LOGE("Invalid configuration");
    return false;
}

bool LibraryInjector::injectByPackage(const std::string& package, const std::string& libPath, const InjectionConfig& config) {
    LOGI("Finding process for package: %s", package.c_str());
    
    pid_t pid = ProcessUtils::findProcessByPackage(package);
    if (pid <= 0) {
        LOGE("Failed to find process for package: %s", package.c_str());
        return false;
    }
    
    LOGI("Found PID: %d", pid);
    return injectByPid(pid, libPath, config);
}

bool LibraryInjector::injectByPid(pid_t pid, const std::string& libPath, const InjectionConfig& config) {
    LOGI("Starting injection into PID: %d", pid);
    LOGI("Library path: %s", libPath.c_str());
    
    // Verify process is running
    if (!ProcessUtils::isProcessRunning(pid)) {
        LOGE("Process %d is not running", pid);
        return false;
    }
    
    // Add delay if specified
    if (config.delayUs > 0) {
        LOGI("Waiting %u microseconds before injection", config.delayUs);
        usleep(config.delayUs);
    }
    
    // Attach to process
    if (!PtraceUtils::attach(pid)) {
        LOGE("Failed to attach to process %d", pid);
        return false;
    }
    
    LOGI("Attached to process successfully");
    
    // Get dlopen function address
    const char* linkerName = sizeof(void*) == 8 ? "linker64" : "linker";
    uintptr_t dlopenAddr = ElfUtils::getRemoteFunctionAddress(pid, linkerName, "dlopen");
    
    if (dlopenAddr == 0) {
        // Try alternative function name
        dlopenAddr = ElfUtils::getRemoteFunctionAddress(pid, linkerName, "__loader_dlopen");
    }
    
    if (dlopenAddr == 0) {
        LOGE("Failed to find dlopen function");
        PtraceUtils::detach(pid);
        return false;
    }
    
    LOGI("Found dlopen at: 0x%lx", dlopenAddr);
    
    // Allocate memory in remote process for library path
    // Write library path to remote process memory
    // Call dlopen with RTLD_NOW | RTLD_GLOBAL
    // This is simplified - full implementation would include:
    // - Memory allocation in remote process
    // - Writing library path
    // - Calling dlopen via ptrace
    // - Handling return value
    // - Calling JNI_OnLoad if present
    
    const int RTLD_NOW = 2;
    const int RTLD_GLOBAL = 0x00100;
    
    // Prepare arguments for dlopen(libPath, RTLD_NOW | RTLD_GLOBAL)
    uintptr_t args[2];
    // args[0] would be remote address of library path string
    // args[1] would be flags
    args[1] = RTLD_NOW | RTLD_GLOBAL;
    
    // Call dlopen (simplified)
    uintptr_t handle = PtraceUtils::callFunction(pid, dlopenAddr, args, 2);
    
    if (handle == 0) {
        LOGE("dlopen failed");
        PtraceUtils::detach(pid);
        return false;
    }
    
    LOGI("Library loaded successfully, handle: 0x%lx", handle);
    
    // Detach from process
    if (!PtraceUtils::detach(pid)) {
        LOGE("Warning: Failed to detach cleanly");
    }
    
    LOGI("Injection completed successfully");
    return true;
}

bool LibraryInjector::watchAndInject(const std::string& package, const std::string& libPath, const InjectionConfig& config) {
    LOGI("Starting watch mode for package: %s", package.c_str());
    
    // Monitor process creation and inject when app launches
    // This is simplified - full implementation would:
    // - Monitor /proc for new process creation
    // - Check if new process matches package name
    // - Inject immediately when detected
    
    while (true) {
        pid_t pid = ProcessUtils::findProcessByPackage(package);
        if (pid > 0) {
            LOGI("Process detected, PID: %d", pid);
            usleep(500000); // Wait 500ms for app to initialize
            return injectByPid(pid, libPath, config);
        }
        usleep(1000000); // Check every second
    }
    
    return false;
}

} // namespace Injector
