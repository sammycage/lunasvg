#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstring>

#include "svgdocument.h"

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

    SVGDocument document;
    if(!document.loadFromFile(fileName)) return help();

    std::string baseName = fileName.substr(fileName.find_last_of("/\\") + 1);
    baseName.append(".png");

    sf::RenderWindow window;
    sf::Image image;
    sf::Texture texture;
    sf::Sprite sprite;

    Bitmap bitmap = document.renderToBitmap(width, height, 96.0, bgColor);
    image.create(bitmap.width(), bitmap.height(), bitmap.data());
    image.saveToFile(baseName);

    std::cout << "Generated PNG file : "<< baseName << std::endl;

    window.create(sf::VideoMode(bitmap.width(), bitmap.height()), "svg2png");
    texture.loadFromImage(image);
    sprite.setTexture(texture);

    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type==sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::White);
        window.draw(sprite);
        window.display();
    }

    return 0;
}
