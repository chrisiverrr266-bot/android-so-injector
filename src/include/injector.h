#ifndef INJECTOR_H
#define INJECTOR_H

#include <string>
#include <cstdint>
#include <sys/types.h>

namespace Injector {

struct InjectionConfig {
    std::string packageName;
    pid_t pid;
    std::string libraryPath;
    bool useMemfd;
    bool hideMaps;
    bool hideSolist;
    bool watchLaunch;
    uint32_t delayUs;
    std::string symbolName;
    
    InjectionConfig() : pid(0), useMemfd(false), hideMaps(false),
                        hideSolist(false), watchLaunch(false), delayUs(0) {}
};

class LibraryInjector {
public:
    LibraryInjector();
    ~LibraryInjector();
    
    bool inject(const InjectionConfig& config);
    
private:
    bool injectByPid(pid_t pid, const std::string& libPath, const InjectionConfig& config);
    bool injectByPackage(const std::string& package, const std::string& libPath, const InjectionConfig& config);
    bool watchAndInject(const std::string& package, const std::string& libPath, const InjectionConfig& config);
    
    pid_t findProcessByPackage(const std::string& package);
    uintptr_t getRemoteFunctionAddress(pid_t pid, const char* moduleName, const char* funcName);
    bool callRemoteFunction(pid_t pid, uintptr_t funcAddr, uintptr_t* params, int paramCount, uintptr_t* retValue);
};

} // namespace Injector

#endif // INJECTOR_H
