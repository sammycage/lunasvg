#ifndef RGB_H
#define RGB_H

namespace lunasvg {

class Rgb
{
public:
    Rgb();
    Rgb(unsigned int value);
    Rgb(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 255);

    bool operator==(const Rgb& rgb) const;
    bool operator!=(const Rgb& rgb) const;
    unsigned int value() const;

public:
   unsigned char r;
   unsigned char g;
   unsigned char b;
   unsigned char a;
};

#define KRgbBlack Rgb(0, 0, 0)
#define KRgbWhite Rgb(255, 255, 255)
#define KRgbRed Rgb(255, 0, 0)
#define KRgbGreen Rgb(0, 255, 0)
#define KRgbBlue Rgb(0, 0, 255)
#define KRgbYellow Rgb(255, 255, 0)
#define KRgbTransparent Rgb(0, 0, 0, 0)

} // namespace lunasvg

#endif // RGB_H
