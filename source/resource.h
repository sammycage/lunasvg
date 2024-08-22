#ifndef LUNASVG_RESOURCE_H
#define LUNASVG_RESOURCE_H

#include <string>

namespace lunasvg {

class Bitmap;

Bitmap loadImageResource(const std::string& href);

} // namespace lunasvg

#endif // LUNASVG_RESOURCE_H
