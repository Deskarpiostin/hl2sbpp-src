#!/bin/bash
./waf configure -T release --use-ccache --togles --64bits --prefix=../game --build-games=hl2sbpp --disable-warns
./waf build install -p -vv
