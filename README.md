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

Follow the steps below to install LunaSVG using either [CMake](https://cmake.org/) or [Meson](https://mesonbuild.com/).

### Using CMake

```bash
git clone https://github.com/sammycage/lunasvg.git
cd lunasvg
cmake -B build .
cmake --build build
cmake --install build
```

After installing LunaSVG, you can include the library in your CMake projects using `find_package`:

```cmake
find_package(lunasvg REQUIRED)

# Link LunaSVG to your target
target_link_libraries(your_target_name PRIVATE lunasvg::lunasvg)
```

Alternatively, you can use CMake's `FetchContent` to include LunaSVG directly in your project without needing to install it first:

```cmake
include(FetchContent)
FetchContent_Declare(
    lunasvg
    GIT_REPOSITORY https://github.com/sammycage/lunasvg.git
    GIT_TAG master  # Specify the desired branch or tag
)
FetchContent_MakeAvailable(lunasvg)

# Link LunaSVG to your target
target_link_libraries(your_target_name PRIVATE lunasvg::lunasvg)
```

Replace `your_target_name` with the name of your executable or library target.

### Using Meson

```bash
git clone https://github.com/sammycage/lunasvg.git
cd lunasvg
meson setup build
meson compile -C build
meson install -C build
```

After installing LunaSVG, you can include the library in your Meson projects using the `dependency` function:

```meson
lunasvg_dep = dependency('lunasvg', required: true)
```

Alternatively, add `lunasvg.wrap` to your `subprojects` directory to include LunaSVG directly in your project without needing to install it first. Create a file named `lunasvg.wrap` with the following content:

```ini
[wrap-git]
url = https://github.com/sammycage/lunasvg.git
revision = head
depth = 1

[provide]
lunasvg = lunasvg_dep
```

You can retrieve the dependency from the wrap fallback with:

```meson
lunasvg_dep = dependency('lunasvg', fallback: ['lunasvg', 'lunasvg_dep'])
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
- [EKA2L1](https://github.com/EKA2L1/EKA2L1)
- [ObEngine](https://github.com/ObEngine/ObEngine)
- [OTTO](https://github.com/bitfieldaudio/OTTO)
- [EmulationStation-DE](https://gitlab.com/es-de/emulationstation-de)
- [SvgBooga](https://github.com/etodanik/SvgBooga/tree/main)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [Multi Theft Auto: San Andreas](https://github.com/multitheftauto/mtasa-blue)
- [eScada Solutions](https://www.escadasolutions.com)
- [CARLA Simulator](https://carla.org/)
