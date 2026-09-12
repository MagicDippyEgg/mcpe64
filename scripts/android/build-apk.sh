#!/usr/bin/env bash
# ============================================================
#  MCPE Android APK build (Linux)
#  Mirrors the logic of build.ps1:
#    1. ndk-build the arm64-v8a libminecraftpe.so (native lib)
#    2. aapt -> R.java, javac, d8 -> classes.dex
#    3. package APK (manifest + res + dex + .so + data assets)
#    4. zipalign + apksigner -> signed debug APK
#
#  Required env:
#    ANDROID_NDK    - path to NDK (default: /tmp/android-ndk-r14b)
#    ANDROID_HOME   - path to Android SDK (default: /tmp/androidsdk)
#    BUILD_TOOLS    - build-tools version (default: 35.0.0)
#    APK_VERSION    - used in the output name (default: git sha / 0.4.0)
# ============================================================
set -euo pipefail

repo="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
ndk="${ANDROID_NDK:-/tmp/android-ndk-r14b}"
sdk="${ANDROID_HOME:-/tmp/androidsdk}"
buildTools="${BUILD_TOOLS:-35.0.0}"
api="${ANDROID_API:-21}"

bt="$sdk/build-tools/$buildTools"
aapt="$bt/aapt"
d8="$bt/d8"
zipalign="$bt/zipalign"
apksigner="$bt/apksigner"
androidJar="$sdk/platforms/android-$api/android.jar"

apkbuild="${APKBUILD_DIR:-/tmp/apkbuild}"

# failsafe: recent NDK images may be used; we want gnustl (r14b)
echo "==> Using NDK: $ndk"
test -x "$ndk/ndk-build" || { echo "NDK not found at $ndk"; exit 1; }
for tool in "$aapt" "$d8" "$zipalign" "$apksigner"; do
  test -x "$tool" || { echo "SDK tool missing: $tool (install build-tools;$buildTools)"; exit 1; }
done
test -f "$androidJar" || { echo "android.jar missing: $androidJar (install platforms;android-$api)"; exit 1; }

libSrc="$repo/project/android/libs/arm64-v8a/libminecraftpe.so"

cleanup() { rm -rf "$apkbuild"; }
trap cleanup EXIT

rm -rf "$apkbuild"
mkdir -p "$apkbuild/gen" "$apkbuild/classes" "$apkbuild/lib/arm64-v8a"

echo "==> Cross-checking native library (build with ndk-build if absent)"
if [ ! -f "$libSrc" ]; then
  echo "WARNING: no libminecraftpe.so found; run ndk-build first."
  echo "  cd project/android && NDK_MODULE_PATH=$repo/project/lib_projects $ndk/ndk-build"
  exit 1
fi
cp "$libSrc" "$apkbuild/lib/arm64-v8a/libminecraftpe.so"
echo "  .so -> $apkbuild/lib/arm64-v8a/libminecraftpe.so"

echo "==> aapt: generating R.java"
"$aapt" package -f -M "$repo/project/android_java/AndroidManifest.xml" \
  -S "$repo/project/android_java/res" -I "$androidJar" \
  -J "$apkbuild/gen" -F "$apkbuild/_rgen.apk" >/dev/null
rm -f "$apkbuild/_rgen.apk"

echo "==> javac"
mapfile -t javaSrcs < <(find "$repo/project/android_java/src" -name "*.java")
javaSrcs+=("$apkbuild/gen/com/mojang/minecraftpe/R.java")
javac --release 8 -sourcepath "$repo/project/android_java/src" \
  -cp "$androidJar" -d "$apkbuild/classes" "${javaSrcs[@]}"

echo "==> d8 (dex)"
"$d8" --min-api "$api" --lib "$androidJar" --output "$apkbuild" \
  $(find "$apkbuild/classes" -name "*.class")

echo "==> aapt: packaging APK"
"$aapt" package -f -M "$repo/project/android_java/AndroidManifest.xml" \
  -S "$repo/project/android_java/res" -I "$androidJar" -F "$apkbuild/unsigned.apk"

echo "==> assembling dex + native lib + data assets"
python3 - "$apkbuild/unsigned.apk" "$repo" "$apkbuild" <<'PY'
import sys, zipfile, os
apk, repo, build = sys.argv[1], sys.argv[2], sys.argv[3]
with zipfile.ZipFile(apk, 'a', zipfile.ZIP_DEFLATED) as z:
    z.write(os.path.join(build, 'classes.dex'), 'classes.dex', zipfile.ZIP_DEFLATED)
    z.write(os.path.join(build, 'lib/arm64-v8a/libminecraftpe.so'),
            'lib/arm64-v8a/libminecraftpe.so', zipfile.ZIP_STORED)
    data = os.path.join(repo, 'data')
    for root, _dirs, files in os.walk(data):
        for f in files:
            full = os.path.join(root, f)
            rel = os.path.join('assets', os.path.relpath(full, data)).replace(os.sep, '/')
            z.write(full, rel, zipfile.ZIP_STORED)
print("  apk contents assembled")
PY

echo "==> zipalign"
"$zipalign" -p 4 "$apkbuild/unsigned.apk" "$apkbuild/aligned.apk"

echo "==> debug keystore"
keystore="${DEBUG_KEYSTORE:-$apkbuild/debug.keystore}"
if [ ! -f "$keystore" ]; then
  "$JAVA_HOME/bin/keytool" -genkeypair -keystore "$keystore" \
    -storepass android -keypass android \
    -alias androiddebugkey -keyalg RSA -keysize 2048 -validity 10000 \
    -dname "CN=Android Debug,O=Android,C=US" 2>/dev/null
fi

echo "==> apksigner"
"$apksigner" sign --ks "$keystore" --ks-pass pass:android \
  --key-pass pass:android --out "$apkbuild/minecraftpe-debug.apk" "$apkbuild/aligned.apk"

final="${BUILD_OUTPUT_DIR:-$repo/out}"
mkdir -p "$final"
version="${APK_VERSION:-$(git -C "$repo" rev-parse --short HEAD 2>/dev/null || echo 0.4.0)}"
cp "$apkbuild/minecraftpe-debug.apk" "$final/mcpe64-android-arm64-v8a-$version.apk"
echo "  done -> $final/mcpe64-android-arm64-v8a-$version.apk"