#include "ptrace_utils.h"
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <android/log.h>
#include <unistd.h>

#define LOG_TAG "PtraceUtils"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace PtraceUtils {

bool attach(pid_t pid) {
    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
        LOGE("PTRACE_ATTACH failed for PID %d: %s", pid, strerror(errno));
        return false;
    }
    
    int status;
    if (waitpid(pid, &status, WUNTRACED) != pid) {
        LOGE("waitpid failed for PID %d: %s", pid, strerror(errno));
        return false;
    }
    
    LOGI("Successfully attached to PID %d", pid);
    return true;
}

bool detach(pid_t pid) {
    if (ptrace(PTRACE_DETACH, pid, NULL, NULL) == -1) {
        LOGE("PTRACE_DETACH failed for PID %d: %s", pid, strerror(errno));
        return false;
    }
    
    LOGI("Successfully detached from PID %d", pid);
    return true;
}

bool getRegs(pid_t pid, struct user_regs_struct* regs) {
#ifdef __aarch64__
    struct iovec io;
    io.iov_base = regs;
    io.iov_len = sizeof(*regs);
    if (ptrace(PTRACE_GETREGSET, pid, (void*)NT_PRSTATUS, &io) == -1) {
#else
    if (ptrace(PTRACE_GETREGS, pid, NULL, regs) == -1) {
#endif
        LOGE("PTRACE_GETREGS failed: %s", strerror(errno));
        return false;
    }
    return true;
}

bool setRegs(pid_t pid, const struct user_regs_struct* regs) {
#ifdef __aarch64__
    struct iovec io;
    io.iov_base = (void*)regs;
    io.iov_len = sizeof(*regs);
    if (ptrace(PTRACE_SETREGSET, pid, (void*)NT_PRSTATUS, &io) == -1) {
#else
    if (ptrace(PTRACE_SETREGS, pid, NULL, regs) == -1) {
#endif
        LOGE("PTRACE_SETREGS failed: %s", strerror(errno));
        return false;
    }
    return true;
}

bool readMemory(pid_t pid, uintptr_t addr, void* buffer, size_t size) {
    size_t count = 0;
    long* buf = (long*)buffer;
    
    while (count < size) {
        long word = ptrace(PTRACE_PEEKDATA, pid, (void*)(addr + count), NULL);
        if (word == -1 && errno != 0) {
            LOGE("PTRACE_PEEKDATA failed at 0x%lx: %s", addr + count, strerror(errno));
            return false;
        }
        buf[count / sizeof(long)] = word;
        count += sizeof(long);
    }
    
    return true;
}

bool writeMemory(pid_t pid, uintptr_t addr, const void* buffer, size_t size) {
    size_t count = 0;
    const long* buf = (const long*)buffer;
    
    while (count < size) {
        if (ptrace(PTRACE_POKEDATA, pid, (void*)(addr + count), buf[count / sizeof(long)]) == -1) {
            LOGE("PTRACE_POKEDATA failed at 0x%lx: %s", addr + count, strerror(errno));
            return false;
        }
        count += sizeof(long);
    }
    
    return true;
}

bool continueExecution(pid_t pid) {
    if (ptrace(PTRACE_CONT, pid, NULL, NULL) == -1) {
        LOGE("PTRACE_CONT failed: %s", strerror(errno));
        return false;
    }
    return true;
}

bool waitForSignal(pid_t pid) {
    int status;
    if (waitpid(pid, &status, WUNTRACED) != pid) {
        LOGE("waitpid failed: %s", strerror(errno));
        return false;
    }
    return true;
}

uintptr_t callFunction(pid_t pid, uintptr_t funcAddr, const uintptr_t* args, int argCount) {
    struct user_regs_struct originalRegs, newRegs;
    
    // Save original registers
    if (!getRegs(pid, &originalRegs)) {
        return 0;
    }
    
    memcpy(&newRegs, &originalRegs, sizeof(newRegs));
    
    // Set up function call based on architecture
#if defined(__aarch64__)
    // ARM64: x0-x7 for arguments, pc for function address
    for (int i = 0; i < argCount && i < 8; i++) {
        newRegs.regs[i] = args[i];
    }
    newRegs.pc = funcAddr;
    newRegs.regs[30] = 0; // LR = 0 to cause crash on return
#elif defined(__arm__)
    // ARM32: r0-r3 for arguments
    for (int i = 0; i < argCount && i < 4; i++) {
        newRegs.uregs[i] = args[i];
    }
    newRegs.ARM_pc = funcAddr;
    newRegs.ARM_lr = 0;
#elif defined(__i386__)
    // x86: arguments on stack
    newRegs.eip = funcAddr;
    // Stack setup would go here
#elif defined(__x86_64__)
    // x86_64: rdi, rsi, rdx, rcx, r8, r9 for arguments
    if (argCount > 0) newRegs.rdi = args[0];
    if (argCount > 1) newRegs.rsi = args[1];
    if (argCount > 2) newRegs.rdx = args[2];
    if (argCount > 3) newRegs.rcx = args[3];
    if (argCount > 4) newRegs.r8 = args[4];
    if (argCount > 5) newRegs.r9 = args[5];
    newRegs.rip = funcAddr;
#endif
    
    // Set new registers
    if (!setRegs(pid, &newRegs)) {
        return 0;
    }
    
    // Continue and wait for crash (return to address 0)
    if (!continueExecution(pid)) {
        return 0;
    }
    
    waitForSignal(pid);
    
    // Get return value
    struct user_regs_struct returnRegs;
    if (!getRegs(pid, &returnRegs)) {
        return 0;
    }
    
    uintptr_t retValue;
#if defined(__aarch64__)
    retValue = returnRegs.regs[0];
#elif defined(__arm__)
    retValue = returnRegs.ARM_r0;
#elif defined(__i386__)
    retValue = returnRegs.eax;
#elif defined(__x86_64__)
    retValue = returnRegs.rax;
#endif
    
    // Restore original registers
    setRegs(pid, &originalRegs);
    
    return retValue;
}

} // namespace PtraceUtils
