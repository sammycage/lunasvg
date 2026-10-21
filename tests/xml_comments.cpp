// SPDX-License-Identifier: MIT
#include "lunasvg.h"

#include <iostream>
#include <string>

static std::string textContent(const lunasvg::Element& element)
{
    std::string content;
    for(const auto& child : element.children()) {
        if(child.isTextNode())
            content += child.toTextNode().data();
        else if(child.isElement())
            content += textContent(child.toElement());
    }
    return content;
}

int main()
{
    const struct {
        const char* name;
        const char* content;
        const char* expected;
    } cases[] = {
        {"leading", "<!--hidden-->visible", "visible"},
        {"between", "before<!--hidden-->after", "beforeafter"},
        {"only", "<!--hidden-->", ""},
        {"adjacent", "a<!----><!--hidden-->b", "ab"},
        {"nested", "a<tspan><!--hidden-->b</tspan><!--hidden-->c", "abc"},
        {"cdata", "<![CDATA[<!--visible-->]]><!--hidden-->tail", "<!--visible-->tail"},
        {"entities", "a&amp;b<!--&amp;hidden-->c", "a&bc"},
        {"plain", "visible", "visible"},
        {"cdata_only", "<![CDATA[<!--visible-->]]>", "<!--visible-->"}
    };
    int failures = 0;
    for(const auto& test : cases) {
        const auto input = std::string("<svg xmlns=\"http://www.w3.org/2000/svg\"><text id=\"target\">")
            + test.content + "</text></svg>";
        const auto document = lunasvg::Document::loadFromData(input);
        if(!document) {
            std::cerr << test.name << ": document did not parse\n";
            ++failures;
            continue;
        }
        const auto target = document->getElementById("target");
        if(target.isNull()) {
            std::cerr << test.name << ": target element missing\n";
            ++failures;
            continue;
        }
        const auto actual = textContent(target);
        if(actual != test.expected) {
            std::cerr << test.name << ": expected [" << test.expected << "], got [" << actual << "]\n";
            ++failures;
        }
    }
    return failures == 0 ? 0 : 1;
}
