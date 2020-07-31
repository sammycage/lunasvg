#ifndef FLATTENINGPATHITERATOR_H
#define FLATTENINGPATHITERATOR_H

#include "pathiterator.h"

namespace lunasvg {

class FlatteningPathIterator
{
public:
   FlatteningPathIterator(const Path& path, unsigned int limit);

   SegType currentSegment(double* coords) const;
   bool isDone() const;
   void next();

private:
   void flattenCurveQuadTo(std::vector<double>& coords) const;
   void flattenCurveCubicTo(std::vector<double>& coords) const;

private:
   PathIterator m_pathIterator;
   std::vector<double> m_pointCoords;
   SegType m_segType;
   unsigned int m_recursionLimit;
   unsigned int m_coordIndex;
   Point m_lastPoint;
};

} // namespace lunasvg

#endif // FLATTENINGPATHITERATOR_H
