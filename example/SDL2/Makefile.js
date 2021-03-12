
CXX= em++

CXXFLAGS= -Wall  -ggdb

LIBS= ../buildjs/liblunasvg.a  

all: lunaSVG_SDL2.cpp 
	$(CXX) lunaSVG_SDL2.cpp -o lunaSVG_SDL2.html $(LIBS) -s USE_SDL=2  -s WASM=1 --preload-file assets --pre-js load.js
	
clean:
	rm -rf lunaSVG_SDL2.html lunaSVG_SDL2.js lunaSVG_SDL2.wasm lunaSVG_SDL2.data

run:
	emrun --serve_after_close lunaSVG_SDL2.html
