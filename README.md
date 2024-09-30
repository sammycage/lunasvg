[![Releases](https://img.shields.io/badge/Version-3.0.0-orange.svg)](https://github.com/sammycage/lunasvg/releases)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](https://github.com/sammycage/lunasvg/blob/master/LICENSE)
[![Build Status](https://github.com/sammycage/lunasvg/actions/workflows/main.yml/badge.svg)](https://github.com/sammycage/lunasvg/actions)

# LunaSVG

LunaSVG is an SVG rendering library in C++, designed to be lightweight and portable, offering efficient rendering and manipulation of Scalable Vector Graphics (SVG) files.

## Basic Usage

```cpp
#include <lunasvg.h>

using namespace lunasvg;

int main()
{
    auto document = Document::loadFromFile("tiger.svg");
    if(document == nullptr)
        return -1;
    auto bitmap = document->renderToBitmap();
    if(bitmap.isNull())
        return -1;
    bitmap.writeToPng("tiger.png");
    return 0;
}

```

![tiger.png](https://github.com/user-attachments/assets/b87bbf92-6dd1-4b29-a890-99cfffce66b8)

## Features

LunaSVG supports nearly all graphical features outlined in the SVG 1.1 and SVG 1.2 Tiny specifications. The primary exceptions are animation, filters, and scripts. As LunaSVG is designed for static rendering, animation is unlikely to be supported in the future. However, support for filters may be added. It currently handles a wide variety of elements, including:

`<a>` `<circle>` `<clipPath>` `<defs>` `<ellipse>` `<g>` `<image>` `<line>` `<linearGradient>` `<marker>` `<mask>` `<path>` `<pattern>` `<polygon>` `<polyline>` `<radialGradient>` `<rect>` `<stop>` `<style>` `<svg>` `<symbol>` `<text>` `<tspan>` `<use>`

## Installation

Follow the steps below to install LunaSVG using either [Meson](https://mesonbuild.com/) or [CMake](https://cmake.org/).

### Using Meson

```bash
git clone https://github.com/sammycage/lunasvg.git
cd lunasvg
meson setup build
meson compile -C build
meson install -C build
```

### Using CMake

```bash
git clone https://github.com/sammycage/lunasvg.git
cd lunasvg
cmake -B build .
cmake --build build
cmake --install build
```

## Demo

LunaSVG provides a command-line tool `svg2png` for converting SVG files to PNG format.

### Usage:
```bash
svg2png [filename] [resolution] [bgColor]
```

### Examples:
```bash
$ svg2png input.svg
$ svg2png input.svg 512x512
$ svg2png input.svg 512x512 0xff00ffff
```

## Projects Using LunaSVG

- [OpenSiv3D](https://github.com/Siv3D/OpenSiv3D)
- [PICsimLab](https://github.com/lcgamboa/picsimlab)
- [MoneyManagerEx](https://github.com/moneymanagerex/moneymanagerex)
- [RmlUi](https://github.com/mikke89/RmlUi)
- [ObEngine](https://github.com/ObEngine/ObEngine)
- [OTTO](https://github.com/bitfieldaudio/OTTO)
- [EmulationStation-DE](https://gitlab.com/es-de/emulationstation-de)
- [SvgBooga](https://github.com/etodanik/SvgBooga/tree/main)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [Multi Theft Auto: San Andreas](https://github.com/multitheftauto/mtasa-blue)
- [eScada Solutions](https://www.escadasolutions.com)
