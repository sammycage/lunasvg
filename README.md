# lunasvg
lunasvg is c++ library to create, animate, manipulate and render SVG files.

# simple example
```cpp
#include <memory>
#include <cmath>

#include <lunasvg/svgdocument.h>

using namespace lunasvg;

int main()
{
    SVGDocument document;
    SVGElement* documentElement = document.rootElement();
    documentElement->setAttribute("transform", "translate(258 256) scale(0.7)");
    static const std::string colors[] = {"red", "purple", "blue", "green", "orange", "yellow"};
    double x1 = 0, y1 = 0, x2 = 0, y2 = 0, angle = 0;
    for(int i = 0;i < 360;i++)
    {
        x2 = x1 + i * std::cos(M_PI * angle / 180.0);
        y2 = y1 + i * std::sin(M_PI * angle / 180.0);
        SVGElement* line = documentElement->appendContent("<line fill='none'/>");
        line->setAttribute("x1", std::to_string(x1));
        line->setAttribute("y1", std::to_string(y1));
        line->setAttribute("x2", std::to_string(x2));
        line->setAttribute("y2", std::to_string(y2));
        line->setAttribute("stroke-width", std::to_string(i / 100 + 1));
        line->setAttribute("stroke", colors[i % 6]);

        angle -= 59;
        x1 = x2;
        y1 = y2;
    }

    std::uint32_t bitmapWidth = 512;
    std::uint32_t bitmapHeight = 512;

    std::unique_ptr<std::uint8_t[]> pixels(new std::uint8_t[bitmapWidth*bitmapHeight*4]);
    Bitmap bitmap(pixels.get(), bitmapWidth, bitmapHeight, bitmapWidth*4);

    document.render(bitmap, 96.0, 0x000000FF);
    
    // do something useful with the bitmap
    
    return 0;
}
```
