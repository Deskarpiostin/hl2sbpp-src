#!/bin/bash
./waf configure -T debug --use-ccache --togles --64bits --prefix=../game --build-games=hl2sbpp --disable-warns
./waf build -p -v
./waf install
