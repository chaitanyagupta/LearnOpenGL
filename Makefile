CFLAGS = -Iinclude -Lbuild

build/hello-world: src/hello-world.c build/libglad.dylib
	cc $(CFLAGS) `pkg-config --cflags --libs glfw3` -lglad -o $@ $<

build/libglad.dylib: src/glad.c
	cc -c $(CFLAGS) -o $@ $<

.PHONY: run
run: build/hello-world
	$<

$(shell mkdir -p build)
