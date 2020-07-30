## lunasvg
lunasvg is c++ library to create, animate, manipulate and render SVG files.

## example
```cpp
...
SVGDocument document;
document.loadFromFile("tiger.svg");
Bitmap bitmap = document.renderToFit();
...

```

## build
Install [cmake](https://cmake.org/download/) if not already installed.
Install [cairo](https://www.cairographics.org/download/) if not already installed.

Create a build directory.
```
mkdir build
```
Run cmake command inside build directory.
```
cd build
cmake ..

```
Run make to build lunasvg.

```
make -j 2
```
To install lunasvg library.

```
make install
```

## demo
While building lunasvg library it generates a simple SVG to PNG converter which can be used to convert SVG file to PNG file.

Run Demo.
```
svg2png [filename] [resolution] [bgColor]
```
