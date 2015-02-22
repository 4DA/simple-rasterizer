SRCFILES = rasterizer.cpp
HDRFILES = rasterizer.h
SDLCFLAGS = $(shell sdl2-config --cflags)
SDLLIBS = $(shell sdl2-config --libs) -lSDL2_image
CFLAGS = $(SDLCFLAGS) $(SDLLIBS) -g -DDEBUG -std=c++11

rasterizer: 	$(SRCFILES) $(HDRFILES)
		g++ $(SRCFILES) $(CFLAGS) -o $(@) 

