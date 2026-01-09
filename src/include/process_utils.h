#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include <string>
#include <vector>
#include <sys/types.h>

namespace ProcessUtils {

struct ModuleInfo {
    std::string name;
    uintptr_t baseAddress;
    uintptr_t endAddress;
    std::string path;
};

pid_t findProcessByName(const std::string& processName);
std::vector<pid_t> findAllProcessesByName(const std::string& processName);
pid_t findProcessByPackage(const std::string& packageName);

bool getProcessModules(pid_t pid, std::vector<ModuleInfo>& modules);
ModuleInfo* findModule(pid_t pid, const std::string& moduleName);

bool isProcessRunning(pid_t pid);
std::string getProcessName(pid_t pid);

bool setSelinuxContext(const std::string& context);

} // namespace ProcessUtils

#endif // PROCESS_UTILS_H
