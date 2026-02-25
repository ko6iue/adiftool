all: html/assets/adiftool.js adiftool test-maidenhead

# INVOKE_RUN=0 causes emscripten to not run main when starting
# All emscripten methods are in emscripten.c
# adiftool.js and adiftool.wasm are output to html/assets directory
html/assets/adiftool.js: adiftool.c maidenhead.c adif.c emscripten.c kml.c geojson.c cmdline.c
	emcc -Wall $^ -o $@ -sEXPORTED_RUNTIME_METHODS=[ccall,HEAPU8] \
		-sEXPORTED_FUNCTIONS=[_main,_malloc,_free] -sALLOW_MEMORY_GROWTH=1 \
		-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE='$$stringToNewUTF8' -lm -s INVOKE_RUN=0

adiftool: adiftool.c maidenhead.c adif.c kml.c geojson.c cmdline.c
	gcc -Wall -W -O3 -o $@ $^ -lm

test-maidenhead: test-maidenhead.c maidenhead.c 
	gcc -Wall -W -O3 -o $@ $^ -lm

test: test-maidenhead
	./test-maidenhead

pretty: pretty-c pretty-js

# Berkeley style, spaces not tabs
pretty-c: adiftool.c adif.c adif.h emscripten.c maidenhead.c maidenhead.h test-maidenhead.c kml.c geojson.c
	indent -orig -nut $^

pretty-js: html/assets/filehelper.js html/*.html html/assets/*.js
	prettier --write $^

clean:
	rm adiftool *.c~ *.h~ html/assets/*.wasm html/assets/adiftool.js 2>/dev/null || /usr/bin/env true
