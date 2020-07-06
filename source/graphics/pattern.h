#ifndef PATTERN_H
#define PATTERN_H

#include "affinetransform.h"
#include "canvas.h"

namespace lunasvg {

enum TileMode
{
    TileModePad,
    TileModeReflect,
    TileModeRepeat
};

class Pattern
{
public:
    Pattern();
    Pattern(const Canvas& canvas);

    void setTile(const Canvas& canvas) { m_tile = canvas; }
    void setTileMode(TileMode mode) { m_tileMode = mode; }
    void setMatrix(const AffineTransform& matrix) { m_matrix = matrix; }
    const Canvas& tile() const { return m_tile; }
    TileMode tileMode() const { return m_tileMode; }
    const AffineTransform& matrix() const { return m_matrix; }

private:
    Canvas m_tile;
    TileMode m_tileMode;
    AffineTransform m_matrix;
};

} // namespace lunasvg

#endif // PATTERN_H
