#!/bin/bash
./waf configure -T release --64bits --prefix=../game --build-games=hl2sbpp --disable-warns
./waf build -p -v
./waf install
