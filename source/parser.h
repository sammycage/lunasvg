#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <map>
#include <memory>

#include "property.h"
#include "element.h"

namespace lunasvg {

class SVGElement;
class StyledElement;

enum LengthNegativeValuesMode
{
    AllowNegativeLengths,
    ForbidNegativeLengths
};

enum class TransformType
{
    Matrix,
    Rotate,
    Scale,
    SkewX,
    SkewY,
    Translate
};

class Parser
{
public:
    static Length parseLength(const std::string& string, LengthNegativeValuesMode mode, const Length& defaultValue);
    static LengthList parseLengthList(const std::string& string, LengthNegativeValuesMode mode);
    static double parseNumber(const std::string& string, double defaultValue);
    static double parseNumberPercentage(const std::string& string, double defaultValue);
    static PointList parsePointList(const std::string& string);
    static Transform parseTransform(const std::string& string);
    static Path parsePath(const std::string& string);
    static std::string parseUrl(const std::string& string);
    static std::string parseHref(const std::string& string);
    static Rect parseViewBox(const std::string& string);
    static PreserveAspectRatio parsePreserveAspectRatio(const std::string& string);
    static Angle parseAngle(const std::string& string);
    static MarkerUnits parseMarkerUnits(const std::string& string);
    static SpreadMethod parseSpreadMethod(const std::string& string);
    static Units parseUnits(const std::string& string, Units defaultValue);
    static Color parseColor(const std::string& string, const StyledElement* element, const Color& defaultValue);
    static Paint parsePaint(const std::string& string, const StyledElement* element, const Color& defaultValue);
    static WindRule parseWindRule(const std::string& string);
    static LineCap parseLineCap(const std::string& string);
    static LineJoin parseLineJoin(const std::string& string);
    static Display parseDisplay(const std::string& string);
    static Visibility parseVisibility(const std::string& string);

private:
    static bool parseLength(const char*& ptr, const char* end, double& value, LengthUnits& units, LengthNegativeValuesMode mode);
    static bool parseNumberList(const char*& ptr, const char* end, double* values, int count);
    static bool parseArcFlag(const char*& ptr, const char* end, bool& flag);
    static bool parseColorComponent(const char*& ptr, const char* end, double& value);
    static bool parseTransform(const char*& ptr, const char* end, TransformType& type, double* values, int& count);
};

struct AttributeSelector
{
    enum class MatchType
    {
        None,
        Equal,
        Includes,
        DashMatch,
        StartsWith,
        EndsWith,
        Contains
    };

    PropertyId id{PropertyId::Unknown};
    std::string value;
    MatchType matchType{MatchType::None};
};

struct SimpleSelector
{
    enum class Combinator
    {
        Descendant,
        Child,
        DirectAdjacent,
        InDirectAdjacent
    };

    ElementId id{ElementId::Star};
    std::vector<AttributeSelector> attributeSelectors;
    Combinator combinator{Combinator::Descendant};
};

struct Selector
{
    std::vector<SimpleSelector> simpleSelectors;
    int specificity{0};
};

struct Rule
{
    std::vector<Selector> selectors;
    PropertyMap declarations;
};

class RuleMatchContext
{
public:
    RuleMatchContext(const std::vector<Rule>& rules);

    std::multimap<int, const PropertyMap*> match(const Element* element) const;

private:
    bool attributeSelectorMatch(const AttributeSelector& selector, const Element* element) const;
    bool simpleSelectorMatch(const SimpleSelector& selector, const Element* element) const;
    bool selectorMatch(const Selector& selector, const Element* element) const;

private:
    const std::vector<Rule>& m_rules;
};

class CSSParser
{
public:
    CSSParser() = default;

    bool parseMore(const std::string& value);

    const std::vector<Rule>& rules() const { return m_rules; }

private:
    bool parseRule(const char*& ptr, const char* end, Rule& rule) const;
    bool parseSelector(const char*& ptr, const char* end, Selector& selector) const;
    bool parseSimpleSelector(const char*& ptr, const char* end, SimpleSelector& simpleSelector) const;
    bool parseDeclarations(const char*& ptr, const char* end, PropertyMap& declarations) const;

private:
    std::vector<Rule> m_rules;
};

class LayoutRoot;

class ParseDocument
{
public:
    ParseDocument();
    ~ParseDocument();

    bool parse(const char* data, std::size_t size);

    SVGElement* rootElement() const { return m_rootElement.get(); }
    Element* getElementById(const std::string& id) const;
    std::unique_ptr<LayoutRoot> layout() const;

private:
    std::unique_ptr<SVGElement> m_rootElement;
    std::map<std::string, Element*> m_idCache;
};

} // namespace lunasvg

#endif // PARSER_H
