#ifndef PTRACE_UTILS_H
#define PTRACE_UTILS_H

#include <sys/types.h>
#include <sys/user.h>
#include <cstdint>

namespace PtraceUtils {

bool attach(pid_t pid);
bool detach(pid_t pid);

bool getRegs(pid_t pid, struct user_regs_struct* regs);
bool setRegs(pid_t pid, const struct user_regs_struct* regs);

bool readMemory(pid_t pid, uintptr_t addr, void* buffer, size_t size);
bool writeMemory(pid_t pid, uintptr_t addr, const void* buffer, size_t size);

bool continueExecution(pid_t pid);
bool waitForSignal(pid_t pid);

uintptr_t callFunction(pid_t pid, uintptr_t funcAddr, const uintptr_t* args, int argCount);

} // namespace PtraceUtils

#endif // PTRACE_UTILS_H
