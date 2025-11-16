#!/bin/bash
scripts/build-android-armv7a.sh
scripts/build-android-aarch64.sh

scripts/build-apk.sh
scripts/build-apk.sh # another one
scripts/build-apk.sh # and another one
mv srceng-mod-launcher/build/android/mod-signed.apk hl2sbpp-release.apk
adb install -r hl2sbpp-release.apk
