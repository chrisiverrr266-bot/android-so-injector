#include "elf_utils.h"
#include "process_utils.h"
#include <android/log.h>
#include <dlfcn.h>
#include <link.h>
#include <fstream>
#include <elf.h>

#define LOG_TAG "ElfUtils"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace ElfUtils {

uintptr_t getLocalModuleBase(const char* moduleName) {
    void* handle = dlopen(moduleName, RTLD_LAZY);
    if (!handle) {
        LOGE("Failed to dlopen %s: %s", moduleName, dlerror());
        return 0;
    }
    
    struct link_map* map;
    if (dlinfo(handle, RTLD_DI_LINKMAP, &map) != 0) {
        LOGE("Failed to get link_map for %s", moduleName);
        dlclose(handle);
        return 0;
    }
    
    uintptr_t baseAddr = (uintptr_t)map->l_addr;
    dlclose(handle);
    
    return baseAddr;
}

uintptr_t getLocalFunctionAddress(const char* moduleName, const char* funcName) {
    void* handle = dlopen(moduleName, RTLD_LAZY);
    if (!handle) {
        LOGE("Failed to dlopen %s: %s", moduleName, dlerror());
        return 0;
    }
    
    void* funcAddr = dlsym(handle, funcName);
    if (!funcAddr) {
        LOGE("Failed to find symbol %s in %s: %s", funcName, moduleName, dlerror());
        dlclose(handle);
        return 0;
    }
    
    uintptr_t addr = (uintptr_t)funcAddr;
    dlclose(handle);
    
    return addr;
}

uintptr_t getModuleBase(pid_t pid, const char* moduleName) {
    ProcessUtils::ModuleInfo* module = ProcessUtils::findModule(pid, moduleName);
    if (!module) {
        LOGE("Failed to find module %s in PID %d", moduleName, pid);
        return 0;
    }
    
    return module->baseAddress;
}

uintptr_t getFunctionOffset(const char* modulePath, const char* funcName) {
    // This is a simplified implementation
    // Full implementation would parse ELF file and find symbol offset
    
    void* handle = dlopen(modulePath, RTLD_LAZY);
    if (!handle) {
        LOGE("Failed to dlopen %s: %s", modulePath, dlerror());
        return 0;
    }
    
    void* funcAddr = dlsym(handle, funcName);
    if (!funcAddr) {
        LOGE("Failed to find symbol %s: %s", funcName, dlerror());
        dlclose(handle);
        return 0;
    }
    
    struct link_map* map;
    if (dlinfo(handle, RTLD_DI_LINKMAP, &map) != 0) {
        dlclose(handle);
        return 0;
    }
    
    uintptr_t offset = (uintptr_t)funcAddr - (uintptr_t)map->l_addr;
    dlclose(handle);
    
    return offset;
}

uintptr_t getRemoteFunctionAddress(pid_t pid, const char* moduleName, const char* funcName) {
    // Get module base in remote process
    uintptr_t remoteBase = getModuleBase(pid, moduleName);
    if (remoteBase == 0) {
        return 0;
    }
    
    // Get module base in local process
    uintptr_t localBase = getLocalModuleBase(moduleName);
    if (localBase == 0) {
        return 0;
    }
    
    // Get function address in local process
    uintptr_t localFuncAddr = getLocalFunctionAddress(moduleName, funcName);
    if (localFuncAddr == 0) {
        return 0;
    }
    
    // Calculate offset and apply to remote base
    uintptr_t offset = localFuncAddr - localBase;
    uintptr_t remoteFuncAddr = remoteBase + offset;
    
    LOGI("Function %s::%s", moduleName, funcName);
    LOGI("  Local base: 0x%lx", localBase);
    LOGI("  Remote base: 0x%lx", remoteBase);
    LOGI("  Offset: 0x%lx", offset);
    LOGI("  Remote address: 0x%lx", remoteFuncAddr);
    
    return remoteFuncAddr;
}

bool parseElfSymbols(const std::string& elfPath) {
    // Placeholder for ELF parsing implementation
    LOGI("Parsing ELF file: %s", elfPath.c_str());
    return true;
}

} // namespace ElfUtils
