#include "process_utils.h"
#include <android/log.h>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>

#define LOG_TAG "ProcessUtils"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace ProcessUtils {

pid_t findProcessByName(const std::string& processName) {
    DIR* dir = opendir("/proc");
    if (!dir) {
        LOGE("Failed to open /proc");
        return -1;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type != DT_DIR) continue;
        
        pid_t pid = atoi(entry->d_name);
        if (pid <= 0) continue;
        
        std::string cmdlinePath = "/proc/" + std::string(entry->d_name) + "/cmdline";
        std::ifstream cmdlineFile(cmdlinePath);
        if (!cmdlineFile.is_open()) continue;
        
        std::string cmdline;
        std::getline(cmdlineFile, cmdline, '\0');
        cmdlineFile.close();
        
        if (cmdline.find(processName) != std::string::npos) {
            closedir(dir);
            return pid;
        }
    }
    
    closedir(dir);
    return -1;
}

std::vector<pid_t> findAllProcessesByName(const std::string& processName) {
    std::vector<pid_t> pids;
    
    DIR* dir = opendir("/proc");
    if (!dir) {
        LOGE("Failed to open /proc");
        return pids;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type != DT_DIR) continue;
        
        pid_t pid = atoi(entry->d_name);
        if (pid <= 0) continue;
        
        std::string cmdlinePath = "/proc/" + std::string(entry->d_name) + "/cmdline";
        std::ifstream cmdlineFile(cmdlinePath);
        if (!cmdlineFile.is_open()) continue;
        
        std::string cmdline;
        std::getline(cmdlineFile, cmdline, '\0');
        cmdlineFile.close();
        
        if (cmdline.find(processName) != std::string::npos) {
            pids.push_back(pid);
        }
    }
    
    closedir(dir);
    return pids;
}

pid_t findProcessByPackage(const std::string& packageName) {
    // On Android, package name = process name for main process
    return findProcessByName(packageName);
}

bool getProcessModules(pid_t pid, std::vector<ModuleInfo>& modules) {
    modules.clear();
    
    std::string mapsPath = "/proc/" + std::to_string(pid) + "/maps";
    std::ifstream mapsFile(mapsPath);
    if (!mapsFile.is_open()) {
        LOGE("Failed to open %s", mapsPath.c_str());
        return false;
    }
    
    std::string line;
    while (std::getline(mapsFile, line)) {
        // Parse line: address perms offset dev inode pathname
        std::istringstream iss(line);
        std::string addrRange, perms, offset, dev, inode, pathname;
        
        iss >> addrRange >> perms >> offset >> dev >> inode;
        std::getline(iss, pathname);
        
        // Skip if no pathname or not executable
        if (pathname.empty() || perms.find('x') == std::string::npos) {
            continue;
        }
        
        // Trim whitespace from pathname
        size_t start = pathname.find_first_not_of(" \t");
        if (start != std::string::npos) {
            pathname = pathname.substr(start);
        }
        
        // Parse address range
        size_t dashPos = addrRange.find('-');
        if (dashPos == std::string::npos) continue;
        
        uintptr_t baseAddr = std::stoull(addrRange.substr(0, dashPos), nullptr, 16);
        uintptr_t endAddr = std::stoull(addrRange.substr(dashPos + 1), nullptr, 16);
        
        // Extract module name from path
        size_t lastSlash = pathname.find_last_of('/');
        std::string moduleName = (lastSlash != std::string::npos) ? 
                                 pathname.substr(lastSlash + 1) : pathname;
        
        // Check if module already exists (only keep first entry)
        bool exists = false;
        for (const auto& mod : modules) {
            if (mod.name == moduleName) {
                exists = true;
                break;
            }
        }
        
        if (!exists) {
            ModuleInfo info;
            info.name = moduleName;
            info.baseAddress = baseAddr;
            info.endAddress = endAddr;
            info.path = pathname;
            modules.push_back(info);
        }
    }
    
    mapsFile.close();
    return true;
}

ModuleInfo* findModule(pid_t pid, const std::string& moduleName) {
    static std::vector<ModuleInfo> modules;
    
    if (!getProcessModules(pid, modules)) {
        return nullptr;
    }
    
    for (auto& mod : modules) {
        if (mod.name.find(moduleName) != std::string::npos) {
            return &mod;
        }
    }
    
    return nullptr;
}

bool isProcessRunning(pid_t pid) {
    std::string procPath = "/proc/" + std::to_string(pid);
    struct stat st;
    return (stat(procPath.c_str(), &st) == 0);
}

std::string getProcessName(pid_t pid) {
    std::string cmdlinePath = "/proc/" + std::to_string(pid) + "/cmdline";
    std::ifstream cmdlineFile(cmdlinePath);
    if (!cmdlineFile.is_open()) {
        return "";
    }
    
    std::string cmdline;
    std::getline(cmdlineFile, cmdline, '\0');
    cmdlineFile.close();
    
    return cmdline;
}

bool setSelinuxContext(const std::string& context) {
    // Attempt to set SELinux context
    // This is a simplified implementation
    LOGI("Setting SELinux context to: %s", context.c_str());
    
    std::string contextPath = "/proc/self/attr/current";
    std::ofstream contextFile(contextPath);
    if (!contextFile.is_open()) {
        LOGE("Failed to open %s", contextPath.c_str());
        return false;
    }
    
    contextFile << context;
    contextFile.close();
    
    return true;
}

} // namespace ProcessUtils
