@echo off
waf configure -T release --prefix=../game --build-games=hl2sbpp --disable-warns && ^
waf build install -p -v --no-msvc-lazy -j4

pause