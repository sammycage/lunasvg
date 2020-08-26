#include <SFML/Graphics.hpp>
#include <ctime>

#include "svgdocument.h"

using namespace lunasvg;

/** copied from https://www.tutorialspoint.com/svg/clock.htm **/

#define WIDTH 800
#define HEIGHT 600

void tick(SVGElement* hourHand, SVGElement* minuteHand, SVGElement* secondHand);

int main()
{
    SVGDocument document;

    SVGElement* rootElement = document.rootElement();
    rootElement->setAttribute("viewBox", "150, 150, 200, 200");
    rootElement->setAttribute("x", "50%");
    rootElement->setAttribute("y", "50%");
    rootElement->setAttribute("width", "512");
    rootElement->setAttribute("height", "512");

    SVGElement* clockFace = document.appendContent("<circle cx='150' cy='150' r='100'/>");
    clockFace->setAttribute("style", "fill:#FFF;stroke:#000;stroke-width:4");

    for(int x = 1;x<=12;x++)
    {
        SVGElement* hourStroke = document.appendContent("<line x1='150' y1='60' x2='150' y2='80'/>");
        hourStroke->setAttribute("style", "stroke:#000;stroke-width:4");
        hourStroke->setAttribute("transform", "rotate(" + std::to_string((360.0/12.0) * x) + ", 150, 150)");
    }

    SVGElement* hourHand = document.appendContent("<line x1='150' y1='150' x2='150' y2='110'/>");
    hourHand->setAttribute("style", "stroke:#000;stroke-width:4");

    SVGElement* minuteHand = document.appendContent("<line x1='150' y1='150' x2='150' y2='60'/>");
    minuteHand->setAttribute("style", "stroke:#000;stroke-width:3");

    SVGElement* secondHand = document.appendContent("<line x1='150' y1='150' x2='150' y2='60'/>");
    secondHand->setAttribute("style", "stroke:#000;stroke-width:1");

    SVGElement* clockCenter = document.appendContent("<circle cx='150' cy='150' r='6'/>");
    clockCenter->setAttribute("style", "fill:#000;");

    Bitmap bitmap(WIDTH, HEIGHT);
    tick(hourHand, minuteHand, secondHand);
    document.render(bitmap, 96.0, 0x46C2D4FF);

    sf::RenderWindow window;
    sf::Texture texture;
    sf::Sprite sprite;

    window.create(sf::VideoMode(bitmap.width(), bitmap.height()), "lunasvg clock");
    texture.create(bitmap.width(), bitmap.height());
    sprite.setTexture(texture);
    texture.update(bitmap.data());

    sf::Clock clock;
    float timer=0.f, delay=1.f;
    while(window.isOpen())
    {
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();
        timer += time;

        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type==sf::Event::Closed)
                window.close();
        }

        if(timer > delay)
        {
            timer = 0.f;
            tick(hourHand, minuteHand, secondHand);
            document.render(bitmap, 96.0, 0x46C2D4FF);
            texture.update(bitmap.data());
        }

        window.clear(sf::Color::White);
        window.draw(sprite);
        window.display();
    }

    return 0;
}

void tick(SVGElement* hourHand, SVGElement* minuteHand, SVGElement* secondHand)
{
    std::time_t now = std::time(nullptr);
    std::tm* localtm = std::localtime(&now);

    int hour = localtm->tm_hour;
    int minute = localtm->tm_min;
    int second = localtm->tm_sec;

    double a1 = ((360.0/12.0) * hour) + (30.0/60.0) * minute;
    double a2 = (360.0/60.0) * minute;
    double a3 = (360.0/60.0) * second;

    hourHand->setAttribute("transform", "rotate("+std::to_string(a1)+ ", 150, 150)");
    minuteHand->setAttribute("transform","rotate("+std::to_string(a2)+ ", 150, 150)");
    secondHand->setAttribute("transform","rotate("+std::to_string(a3)+ ", 150, 150)");
}
