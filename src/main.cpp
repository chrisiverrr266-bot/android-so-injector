#include "injector.h"
#include <iostream>
#include <cstring>
#include <android/log.h>

#define LOG_TAG "Injector"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

void printUsage(const char* programName) {
    std::cout << "Android SO Injector - Usage:\n";
    std::cout << "\nBasic Usage:\n";
    std::cout << "  " << programName << " -pkg <package_name> -lib <library_path>\n";
    std::cout << "  " << programName << " -pid <process_id> -lib <library_path>\n";
    std::cout << "\nArguments:\n";
    std::cout << "  -pkg <package>      Target application package name\n";
    std::cout << "  -pid <pid>          Target process ID\n";
    std::cout << "  -lib <path>         Path to .so library to inject (required)\n";
    std::cout << "  -dl_memfd           Use memfd_create & dlopen_ext\n";
    std::cout << "  -hide_maps          Hide library from /proc/[pid]/maps\n";
    std::cout << "  -hide_solist        Remove library from linker solist\n";
    std::cout << "  -watch              Monitor and inject on app launch\n";
    std::cout << "  -delay <us>         Delay in microseconds before injection\n";
    std::cout << "  -symbols <name>     Symbol name to call in library\n";
    std::cout << "  -h, --help          Show this help message\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << programName << " -pkg com.example.app -lib /data/local/tmp/hook.so\n";
    std::cout << "  " << programName << " -pid 12345 -lib /data/local/tmp/hook.so -dl_memfd\n";
    std::cout << "  " << programName << " -pkg com.game -lib /data/local/tmp/cheat.so -watch\n";
}

int main(int argc, char* argv[]) {
    LOGI("Android SO Injector started");
    
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    Injector::InjectionConfig config;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        }
        else if (strcmp(argv[i], "-pkg") == 0 && i + 1 < argc) {
            config.packageName = argv[++i];
        }
        else if (strcmp(argv[i], "-pid") == 0 && i + 1 < argc) {
            config.pid = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-lib") == 0 && i + 1 < argc) {
            config.libraryPath = argv[++i];
        }
        else if (strcmp(argv[i], "-dl_memfd") == 0) {
            config.useMemfd = true;
        }
        else if (strcmp(argv[i], "-hide_maps") == 0) {
            config.hideMaps = true;
        }
        else if (strcmp(argv[i], "-hide_solist") == 0) {
            config.hideSolist = true;
        }
        else if (strcmp(argv[i], "-watch") == 0) {
            config.watchLaunch = true;
        }
        else if (strcmp(argv[i], "-delay") == 0 && i + 1 < argc) {
            config.delayUs = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-symbols") == 0 && i + 1 < argc) {
            config.symbolName = argv[++i];
        }
    }
    
    // Validate arguments
    if (config.libraryPath.empty()) {
        LOGE("Error: Library path (-lib) is required");
        printUsage(argv[0]);
        return 1;
    }
    
    if (config.packageName.empty() && config.pid == 0) {
        LOGE("Error: Either package name (-pkg) or PID (-pid) is required");
        printUsage(argv[0]);
        return 1;
    }
    
    // Check if running as root
    if (getuid() != 0) {
        LOGE("Error: This tool requires root privileges");
        std::cerr << "Error: This tool requires root privileges\n";
        return 1;
    }
    
    // Display configuration
    LOGI("Configuration:");
    if (!config.packageName.empty()) {
        LOGI("  Package: %s", config.packageName.c_str());
    }
    if (config.pid != 0) {
        LOGI("  PID: %d", config.pid);
    }
    LOGI("  Library: %s", config.libraryPath.c_str());
    if (config.useMemfd) LOGI("  Use memfd: enabled");
    if (config.hideMaps) LOGI("  Hide maps: enabled");
    if (config.hideSolist) LOGI("  Hide solist: enabled");
    if (config.watchLaunch) LOGI("  Watch launch: enabled");
    if (config.delayUs > 0) LOGI("  Delay: %u us", config.delayUs);
    
    // Perform injection
    Injector::LibraryInjector injector;
    
    if (injector.inject(config)) {
        LOGI("Injection successful!");
        std::cout << "Injection successful!\n";
        return 0;
    } else {
        LOGE("Injection failed!");
        std::cerr << "Injection failed!\n";
        return 1;
    }
}
