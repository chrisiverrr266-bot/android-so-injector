# References and Open Source Projects

This project references and learns from several excellent open-source Android injection projects:

## Primary References

### 1. AndroidPtraceInject
- **Author**: SsageParuders
- **Repository**: https://github.com/SsageParuders/AndroidPtraceInject
- **License**: MIT
- **Stars**: 222+
- **Features**:
  - Supports all ABIs (arm, arm64, x86, x86_64)
  - Works on all Android API levels
  - Comprehensive Chinese comments
  - Command-line interface
  - SELinux handling

### 2. AndKittyInjector
- **Author**: MJx0
- **Repository**: https://github.com/MJx0/AndKittyInjector
- **License**: MIT
- **Stars**: 239+
- **Features**:
  - Tested Android 5.0 - 14
  - memfd_create & dlopen_ext support
  - Bypass linker namespace restrictions
  - Hide from /proc/[pid]/maps
  - Hide from linker solist
  - App launch monitor (-watch)
  - Emulated architecture support (libhoudini/libndk_translation)

### 3. droid_injectso
- **Author**: yurushao
- **Repository**: https://github.com/yurushao/droid_injectso
- **Features**:
  - Classic injection tool
  - ELF hook engine included
  - Tested on Android 2.2 - 4.1
  - Simple implementation, good for learning

### 4. NullTrace-Injector
- **Author**: 0NullBit0
- **Repository**: https://github.com/0NullBit0/NullTrace-Injector
- **Features**:
  - Modern implementation
  - Clean code structure

## Additional Resources

### Technical Articles

1. **Shared Library Injection in Android** by Alexander Fadeev
   - URL: https://fadeevab.com/shared-library-injection-on-android-8/
   - Covers injection on Android 8.0+

2. **Shared Library Injection** by shunix
   - URL: https://shunix.com/shared-library-injection-in-android/
   - Detailed technical explanation

3. **LD_PRELOAD for Android Applications**
   - URL: https://serializethoughts.com/2017/04/01/working-of-ld_preload-for-android-applications-and-its-anti-re-technique
   - LD_PRELOAD technique and anti-RE

### Related Projects

- **TinyInjector** - Classic injector by shunix
- **Adrill** - By mustime
- **SharkInject** - By bigGreenPeople
- **androidinject** - By mergerly
- **arminject** - ARM injection reference
- **injectvm-binderjack** - VM injection techniques
- **am_proc_start** - Process monitoring

## Injection Techniques

### 1. Ptrace Injection
- Most reliable method
- Works across all Android versions
- Requires root access
- Uses `PTRACE_ATTACH`, `PTRACE_GETREGS`, `PTRACE_SETREGS`

### 2. LD_PRELOAD
- Environment variable hijacking
- Works for newly launched processes
- Limited by Android restrictions on modern versions

### 3. PLT/GOT Hooking
- Function interception
- Used after injection
- Common in ELF hook engines

## Learning Path

1. Start with **droid_injectso** for basic concepts
2. Study **AndroidPtraceInject** for multi-ABI support
3. Review **AndKittyInjector** for advanced features
4. Read technical articles for deeper understanding

## Community

- Android Reverse Engineering communities
- XDA Developers forums
- GitHub discussions on referenced projects

---

*Last updated: January 2026*
