#include <iostream>
#include <cstring>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "document.h"

using namespace lunasvg;

int help()
{
    std::cout<<"Usage: \n   svg2png [filename] [resolution] [bgColor]\n\nExamples: \n    $ svg2png input.svg\n    $ svg2png input.svg 512x512\n    $ svg2png input.svg 512x512 ff00ffff\n\n";
    return 1;
}

int setup(int argc, char** argv, std::string& fileName, std::uint32_t* width, std::uint32_t* height, std::uint32_t* bgColor)
{
    char* path = nullptr;

    if(argc > 1) path = argv[1];
    if(argc > 2)
    {
        char* x = strstr(argv[2], "x");
        if(x)
        {
            char buf[20];
            snprintf(buf, x - argv[2] + 1, "%s", argv[2]);
            *width = atoi(buf);
            snprintf(buf, sizeof(buf), "%s", x + 1);
            *height = atoi(buf);
        }
    }

    if(argc > 3) *bgColor = strtol(argv[3], nullptr, 16);

    if(!path) return help();

    fileName = std::string(path);

    std::string ext = ".svg";
    if(fileName.size() <= ext.size() || fileName.substr(fileName.size()- ext.size()) != ext)
        return help();

    return 0;
}

int main(int argc, char** argv)
{
    std::uint32_t width = 0, height = 0;
    std::string fileName;
    std::uint32_t bgColor = 0x00000000;

    if(setup(argc, argv, fileName, &width, &height, &bgColor)) return 1;

    auto document = Document::loadFromFile(fileName);
    if(document == nullptr)
        return help();

    auto bitmap = document->renderToBitmap(width, height, bgColor);

    std::string baseName = fileName.substr(fileName.find_last_of("/\\") + 1);
    baseName.append(".png");

    stbi_write_png(baseName.c_str(), bitmap.width(), bitmap.height(), 4, bitmap.data(), 0);

    std::cout << "Generated PNG file : "<< baseName << std::endl;

    return 0;
}
