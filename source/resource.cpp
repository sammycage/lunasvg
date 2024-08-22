#include "resource.h"
#include "lunasvg.h"

#include <plutovg.h>

namespace lunasvg {

Bitmap loadImageResource(const std::string& href)
{
    if(href.compare(0, 5, "data:") == 0) {
        std::string_view input(href);
        auto index = input.find(',', 5);
        if(index == std::string_view::npos)
            return Bitmap();
        input.remove_prefix(index + 1);
        return plutovg_surface_load_from_image_base64(input.data(), input.length());
    }

    return plutovg_surface_load_from_image_file(href.data());
}

} // namespace lunasvg
