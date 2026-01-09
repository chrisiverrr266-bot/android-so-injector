#ifndef ELF_UTILS_H
#define ELF_UTILS_H

#include <string>
#include <cstdint>
#include <sys/types.h>

namespace ElfUtils {

uintptr_t getLocalFunctionAddress(const char* moduleName, const char* funcName);
uintptr_t getRemoteFunctionAddress(pid_t pid, const char* moduleName, const char* funcName);

uintptr_t getModuleBase(pid_t pid, const char* moduleName);
uintptr_t getLocalModuleBase(const char* moduleName);

uintptr_t getFunctionOffset(const char* modulePath, const char* funcName);

bool parseElfSymbols(const std::string& elfPath);

} // namespace ElfUtils

#endif // ELF_UTILS_H
