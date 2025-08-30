#!/bin/bash
scripts/build-android-armv7a.sh
scripts/build-android-aarch64.sh

# pls do this manually
#scripts/build-apk.sh
#mv srceng-mod-launcher/build/android/mod-signed.apk hl2sbpp-release.apk
#adb install -r hl2sbpp-release.apk