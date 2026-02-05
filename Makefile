all: adi2kml.js adi2kml test-maidenhead

# INVOKE_RUN=0 causes emscripten to not run main when starting
# All enscripten methods are in enscripten.c
adi2kml.js: adi2kml.c maidenhead.c adif.c enscripten.c
	emcc -Wall $^ -o $@ -sEXPORTED_RUNTIME_METHODS=[ccall,HEAPU8] \
		-sEXPORTED_FUNCTIONS=[_main,_malloc,_free] -sALLOW_MEMORY_GROWTH=1 \
		-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE='$$stringToNewUTF8' -lm -s INVOKE_RUN=0

adi2kml: adi2kml.c maidenhead.c adif.c
	gcc -Wall -O3 -I . -o $@ $^ -lm

test-maidenhead: test-maidenhead.c maidenhead.c
	gcc -Wall -O3 -I . -o $@ $^ -lm

deploy: adi2kml.js
	rm -rf site/*
	mkdir -p site/assets 2>/dev/null || /usr/bin/env true
	cp index.html site
	cp filehelper.js site/assets
	cp adi2kml.js site/assets
	cp adi2kml.wasm site/assets

test: test-maidenhead
	./test-maidenhead

pretty: pretty-c pretty-js

pretty-c: adi2kml.c adif.c adif.h enscripten.c maidenhead.c maidenhead.h test-maidenhead.c
	indent -orig $^

pretty-js: filehelper.js index.html
	prettier --write $^


clean:
	rm adi2kml *.c~ *.h~ adi2kml.js *.wasm 2>/dev/null || /usr/bin/env true
