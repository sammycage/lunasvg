#include "rgb.h"

namespace lunasvg
{

Rgb::Rgb() :
    r(0),
    g(0),
    b(0),
    a(255)
{
}

Rgb::Rgb(unsigned int value) :
    r((value&0xff000000)>>24),
    g((value&0x00ff0000)>>16),
    b((value&0x0000ff00)>>8 ),
    a((value&0x000000ff)>>0 )
{
}

Rgb::Rgb(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha) :
    r(red),
    g(green),
    b(blue),
    a(alpha)
{
}

unsigned int Rgb::value() const
{
    return (r<<24) | (g<<16) | (b<<8) | a;
}

bool Rgb::operator==(const Rgb& rgb) const
{
    return (rgb.r == r)
            && (rgb.g == g)
            && (rgb.b == b)
            && (rgb.a == a);
}

bool Rgb::operator!=(const Rgb& rgb) const
{
    return !operator==(rgb);
}

} // namespace lunasvg
