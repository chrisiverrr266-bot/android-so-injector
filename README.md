# Android SO Injector

Android shared library (.so) injector using ptrace to inject libraries into target applications.

## Features

- **Multi-ABI Support**: arm, arm64, x86, x86_64
- **All Android Versions**: Tested on Android 5.0+
- **Ptrace Injection**: Reliable injection method using ptrace system calls
- **Package/PID Targeting**: Inject by app package name or process ID
- **Advanced Options**: memfd dlopen, hide from maps, bypass linker namespace

## Build Requirements

- Android NDK
- CMake 3.10+
- Ninja (optional but recommended)
- Root access on target device

## How to Build

### Method 1: Using build script

```bash
git clone https://github.com/chrisiverrr266-bot/android-so-injector.git
cd android-so-injector
mkdir build
chmod +x build.sh
./build.sh
```

### Method 2: Manual CMake build

```bash
git clone https://github.com/chrisiverrr266-bot/android-so-injector.git
cd android-so-injector
mkdir build
cd build
cmake .. -G Ninja -DANDROID_ABI=arm64-v8a
cmake --build .
```

## Usage

### Basic injection by package name:

```bash
adb push injector /data/local/tmp/
adb push your_lib.so /data/local/tmp/
adb shell
su
cd /data/local/tmp
chmod +x injector
./injector -pkg com.example.app -lib /data/local/tmp/your_lib.so
```

### Injection by PID:

```bash
./injector -pid 12345 -lib /data/local/tmp/your_lib.so
```

### Advanced options:

```bash
# Use memfd dlopen (bypass path restrictions)
./injector -pkg com.example.app -lib /data/local/tmp/your_lib.so -dl_memfd

# Hide from /proc/[pid]/maps
./injector -pkg com.example.app -lib /data/local/tmp/your_lib.so -hide_maps

# Monitor app launch and inject immediately
./injector -pkg com.example.app -lib /data/local/tmp/your_lib.so -watch

# Add delay before injection (microseconds)
./injector -pkg com.example.app -lib /data/local/tmp/your_lib.so -delay 500000
```

## Command Line Arguments

| Argument | Description | Required |
|----------|-------------|----------|
| `-pkg` | Target app package name | Yes (or -pid) |
| `-pid` | Target process ID | Yes (or -pkg) |
| `-lib` | Path to .so library to inject | Yes |
| `-dl_memfd` | Use memfd_create & dlopen_ext | No |
| `-hide_maps` | Hide lib from /proc/[pid]/maps | No |
| `-hide_solist` | Remove lib from linker solist | No |
| `-watch` | Monitor process launch | No |
| `-delay` | Delay in microseconds before inject | No |
| `-symbols` | Specify symbol to call in library | No |

## Creating Injectable Libraries

### Example library code:

```cpp
#include <jni.h>
#include <android/log.h>
#include <thread>

#define LOG_TAG "InjectedLib"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void my_thread_function() {
    LOGI("Injected library thread started!");
    // Your code here
}

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    // Key 1337 is passed by injector
    if (reserved != (void*)1337) {
        return JNI_VERSION_1_6;
    }
    
    LOGI("JNI_OnLoad called by injector");
    
    JNIEnv* env = nullptr;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_OK) {
        LOGI("JavaEnv obtained: %p", env);
        // Your initialization code
    }
    
    // Start your thread here, NOT in constructor
    std::thread(my_thread_function).detach();
    
    return JNI_VERSION_1_6;
}
```

## Technical Details

### Injection Method

The injector uses the ptrace system call to:
1. Attach to the target process (`PTRACE_ATTACH`)
2. Backup current register state
3. Manipulate registers to call `dlopen()` or `__loader_dlopen()`
4. Load the specified .so library into target's memory space
5. Restore original registers
6. Detach from process

### SELinux Handling

The injector automatically handles SELinux contexts to ensure injection works on enforcing mode.

### Namespace Bypass

On Android 7.0+, the injector bypasses linker namespace restrictions to allow loading libraries from any path.

## References & Credits

This project is inspired by and references:

- [AndroidPtraceInject](https://github.com/SsageParuders/AndroidPtraceInject) - Multi-ABI ptrace injection
- [AndKittyInjector](https://github.com/MJx0/AndKittyInjector) - Advanced injection features
- [TinyInjector](https://github.com/shunix/TinyInjector) - Classic injection techniques
- [droid_injectso](https://github.com/yurushao/droid_injectso) - Early Android injection

## License

MIT License - See LICENSE file for details

## Disclaimer

**Educational purposes only.** This tool is for security research and debugging. Using this tool on applications you don't own or have permission to modify may violate terms of service or laws. Use responsibly.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues.

## TODO

- [ ] Add source code for injector binary
- [ ] Add CMakeLists.txt configuration
- [ ] Add build scripts for all ABIs
- [ ] Add example injectable library
- [ ] Add unit tests
- [ ] Add SELinux policy helper
- [ ] Improve error handling and logging
