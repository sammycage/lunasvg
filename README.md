<div align="center">
    <span align="center">
        <img height="128" class="center" src="https://github.com/sammycage/lunasvg/blob/master/luna.png" alt="Icon">
    </span>
    <h1 align="center">LunaSVG</h1>
    <h3 align="center">LunaSVG is a standalone SVG rendering library in C++</h3>
</div>
<br>
<p align="center">
    <img height="480" src="https://dev.w3.org/SVG/tools/svgweb/samples/svg-files/cartman.svg">
    <br>
    <a href="https://github.com/sammycage/lunasvg/releases">
        <img src="https://img.shields.io/badge/Version-2.1.5-orange.svg">
    </a>
    <a href="https://github.com/sammycage/lunasvg/blob/master/LICENSE">
        <img src="https://img.shields.io/badge/License-MIT-blue.svg">
    </a>
    <a href="https://github.com/sammycage/lunasvg/actions">
        <img src="https://github.com/sammycage/lunasvg/actions/workflows/ci.yml/badge.svg">
    </a>
    <br>
    <a href="https://github.com/sammycage/lunasvg/issues/new">Report a problem!</a>
</p>

## Example

```cpp
#include <lunasvg/document.h>

using namespace lunasvg;

int main()
{
    auto document = Document::loadFromFile("tiger.svg");
    auto bitmap = document->renderToBitmap();

    // do something useful with the bitmap here.

    return 0;
}

```

## Features

- Basic Shapes
- Document Structures
- Coordinate Systems, Transformations and Units
- SolidColors
- Gradients
- Patterns
- Masks
- ClipPaths
- Markers
- StyleSheet

## TODO

- Texts
- Filters
- Images

## Build

```
git clone https://github.com/sammycage/lunasvg.git
cd lunasvg
mkdir build
cd build
cmake ..
make -j 2
```

To install lunasvg library.

```
make install
```

## Demo

While building lunasvg example it generates a simple SVG to PNG converter which can be used to convert SVG file to PNG file.

Run Demo.
```
svg2png [filename] [resolution] [bgColor]
```

## Support

If you like the work lunasvg is doing please consider a small donation:

<a href="https://www.buymeacoffee.com/sammycage"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=&slug=sammycage&button_colour=FFDD00&font_colour=000000&font_family=Arial&outline_colour=000000&coffee_colour=ffffff"></a>
