#include "pattern.h"

namespace lunasvg {

Pattern::Pattern()
{
}

Pattern::Pattern(const Canvas& canvas)
    : m_tile(canvas),
      m_tileMode(TileModeRepeat)
{
}

} // namespace lunasvg
