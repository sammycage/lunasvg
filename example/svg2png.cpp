#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <cmath>

#ifndef _WIN32
#include<libgen.h>
#else
#include <windows.h>
#include <stdlib.h>
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "svgdocument.h"

using namespace lunasvg;

int help()
{
    std::cout<<"Usage: \n   svg2png [filename] [resolution] [bgColor]\n\nExamples: \n    $ svg2png input.svg\n    $ svg2png input.svg 512x512\n    $ svg2png input.svg 512x512 ff00ffff\n\n";
    return 1;
}

int setup(int argc, char **argv, std::string& fileName, int* width, int* height, std::uint32_t* bgColor)
{
    char *path{nullptr};

    if(argc > 1) path = argv[1];
    if(argc > 2)
    {
        char tmp[20];
        char *x = strstr(argv[2], "x");
        if(x)
        {
            snprintf(tmp, x - argv[2] + 1, "%s", argv[2]);
            *width = atoi(tmp);
            snprintf(tmp, sizeof(tmp), "%s", x + 1);
            *height = atoi(tmp);
        }
    }

    if(argc > 3) *bgColor = strtol(argv[3], nullptr, 16);

    if(!path) return help();

    std::array<char, 5000> memory;

#ifdef _WIN32
    path = _fullpath(memory.data(), path, memory.size());
#else
    path = realpath(path, memory.data());
#endif
    if(!path) return help();

    fileName = std::string(path);

    std::string ext = ".svg";
    if(fileName.size() <= ext.size() || fileName.substr(fileName.size()- ext.size()) != ext)
        return help();

    return 0;
}

int main(int argc, char **argv)
{
    int w = -1, h = -1;
    std::string fileName;
    std::uint32_t bgColor = 0x00000000;

    if(setup(argc, argv, fileName, &w, &h, &bgColor)) return 1;

    SVGDocument document;
    if(!document.loadFromFile(fileName)) return help();

    if(w <= 0 || h <= 0)
    {
        double documentWidth = document.documentWidth();
        double documentHeight = document.documentHeight();

        w = int(std::ceil(std::max(documentWidth, 1.0)));
        h = int(std::ceil(std::max(documentHeight, 1.0)));
    }

    std::unique_ptr<std::uint8_t[]> pixels(new std::uint8_t[w*h*4]);
    Bitmap bitmap(pixels.get(), w, h, w*4);

    document.render(bitmap, 96.0, bgColor);

    std::string baseName = fileName.substr(fileName.find_last_of("/\\") + 1);
    baseName.append(".png");

    stbi_write_png(baseName.c_str(), w, h, 4, pixels.get(), 0);

    std::cout << "Generated PNG file : "<< baseName << std::endl;

    return 0;
}
