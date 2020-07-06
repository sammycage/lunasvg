#include "svgenumeration.h"

namespace lunasvg {

template<> const EnumEntryList& getEnumEntryList<Display>()
{
    static EnumEntryList entries;
    if(entries.empty())
    {
        entries.push_back(EnumEntry(DisplayInline, "inline"));
        entries.push_back(EnumEntry(DisplayBlock, "block"));
        entries.push_back(EnumEntry(DisplayNone, "none"));
    }

    return entries;
}

template<> const EnumEntryList& getEnumEntryList<Visibility>()
{
    static EnumEntryList entries;
    if(entries.empty())
    {
        entries.push_back(EnumEntry(VisibilityVisible, "visible"));
        entries.push_back(EnumEntry(VisibilityHidden, "hidden"));
        entries.push_back(EnumEntry(VisibilityCollaspe, "collaspe"));
    }

    return entries;
}

template<> const EnumEntryList& getEnumEntryList<Overflow>()
{
    static EnumEntryList entries;
    if(entries.empty())
    {
        entries.push_back(EnumEntry(OverflowVisible, "visible"));
        entries.push_back(EnumEntry(OverflowHidden, "hidden"));
        entries.push_back(EnumEntry(OverflowScroll, "scroll"));
        entries.push_back(EnumEntry(OverflowAuto, "auto"));
    }

    return entries;
}

template<> const EnumEntryList& getEnumEntryList<LineCap>()
{
    static EnumEntryList entries;
    if(entries.empty())
    {
        entries.push_back(EnumEntry(LineCapButt, "butt"));
        entries.push_back(EnumEntry(LineCapRound, "round"));
        entries.push_back(EnumEntry(LineCapSquare, "square"));
    }

    return entries;
}

template<> const EnumEntryList& getEnumEntryList<LineJoin>()
{
    static EnumEntryList entries;
    if(entries.empty())
    {
        entries.push_back(EnumEntry(LineJoinMiter, "miter"));
        entries.push_back(EnumEntry(LineJoinBevel, "bevel"));
        entries.push_back(EnumEntry(LineJoinRound, "round"));
    }

    return entries;
}

template<> const EnumEntryList& getEnumEntryList<WindRule>()
{
    static EnumEntryList entries;
    if(entries.empty())
    {
        entries.push_back(EnumEntry(WindRuleNonZero, "nonzero"));
        entries.push_back(EnumEntry(WindRuleEvenOdd, "evenodd"));
    }

    return entries;
}

template<> const EnumEntryList& getEnumEntryList<SpreadMethod>()
{
    static EnumEntryList entries;
    if(entries.empty())
    {
        entries.push_back(EnumEntry(SpreadMethodPad, "pad"));
        entries.push_back(EnumEntry(SpreadMethodReflect, "reflect"));
        entries.push_back(EnumEntry(SpreadMethodRepeat, "repeat"));
    }

    return entries;
}

template<> const EnumEntryList& getEnumEntryList<UnitType>()
{
    static EnumEntryList entries;
    if(entries.empty())
    {
        entries.push_back(EnumEntry(UnitTypeObjectBoundingBox, "objectBoundingBox"));
        entries.push_back(EnumEntry(UnitTypeUserSpaceOnUse, "userSpaceOnUse"));
    }

    return entries;
}

void SVGEnumerationBase::setValueAsString(const std::string& value)
{
    m_value = 0;
    if(value.empty())
        return;

    for(unsigned int i = 0;i < m_entries.size();i++)
    {
        if(m_entries[i].second == value)
        {
            m_value = m_entries[i].first;
            return;
        }
    }
}

std::string SVGEnumerationBase::valueAsString() const
{
    for(unsigned int i = 0;i < m_entries.size();i++)
        if(m_entries[i].first == m_value)
            return m_entries[i].second;

    return KEmptyString;
}

} //namespace lunasvg
