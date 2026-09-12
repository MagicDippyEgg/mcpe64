# removed most of original readme

## How to Build
### Android
Download r14b Android NDK and run `build.ps1`:
```
# Full build (NDK + Java + APK + install)
C:\apkbuild\build.ps1

# Skip NDK recompile (Java/assets changed only)
C:\apkbuild\build.ps1 -NoJava

# Skip Java recompile (C++ changed only)  
C:\apkbuild\build.ps1 -NoCpp

# Only repackage + install (no recompile at all)
C:\apkbuild\build.ps1 -NoBuild
```
