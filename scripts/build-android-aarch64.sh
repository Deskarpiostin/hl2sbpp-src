#!/bin/sh
export PATH="$PWD/clang+llvm-11.1.0-x86_64-linux-gnu-ubuntu-16.04/bin:$PATH"
./waf configure -T release --build-game=hl2sbpp --prefix=srceng-mod-launcher/android --togles --android=aarch64,host,21 --target=../aarch64 -8 --disable-warns &&
./waf install --target=client,server,GameUI,engine,studiorender,matsys_controls --strip
