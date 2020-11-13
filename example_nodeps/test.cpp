
#include <ctime>
#include <string>
#include <iostream>
#include <fstream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "svgdocument.h"
using namespace lunasvg;

const char* strokedasharray[][2] =
{
    { "nodash", "" }, 
    { "dash_10", R"(stroke-dasharray="10")" },
    { "dash_10_2", R"(stroke-dasharray="10 2")" },
    { "dash_10_2_5", R"(stroke-dasharray="10 2 5")" },
    { "dash_10_2_5_8", R"(stroke-dasharray="10 2 5 8")" }
};

const char* strokelinecap[]
{
    R"("")",
    R"(stroke-linecap="butt")",
    R"(stroke-linecap="square")",
    R"(stroke-linecap="round")"
};

const char* tests[] =
{
// line with dashoffset
R"svg(
<svg xmlns="http://www.w3.org/2000/svg" baseProfile="tiny" width="300" height="100" viewBox="0 0 300 100">
    <line x1="0" y1="10" x2="300" y2="10" stroke-width="3" stroke="red" %strokedasharray%/> 
    <line x1="0" y1="30" x2="300" y2="30" stroke-width="3" stroke="green" %strokedasharray% stroke-dashoffset="10"/> 
    <line x1="0" y1="50" x2="300" y2="50" stroke-width="3" stroke="blue" %strokedasharray% stroke-dashoffset="20"/>
    <line x1="0" y1="70" x2="300" y2="70" stroke-width="3" stroke="yellow" %strokedasharray% stroke-dashoffset="30"/>
    <line x1="0" y1="90" x2="300" y2="90" stroke-width="3" stroke="brown" %strokedasharray% stroke-dashoffset="40"/>
</svg>
)svg"

, // line
R"svg(
<svg width="120" height="120" xmlns="http://www.w3.org/2000/svg">
 <rect x="10" y="10" width="100" height="100" stroke="red" fill="purple"
       fill-opacity="0.5" stroke-opacity="0.8" stroke-width="3" %strokedasharray%/>
</svg>
)svg"

, // arc
R"svg(
<svg width="320" height="320" xmlns="http://www.w3.org/2000/svg">
  <path d="M 10 315
           L 110 215
           A 30 50 0 0 1 162.55 162.45
           L 172.55 152.45
           A 30 50 -45 0 1 215.1 109.9
           L 315 10" stroke="red" fill="green" stroke-width="3" fill-opacity="0.5" %strokedasharray%/>
</svg>
)svg"

, // bezier curve

R"svg(
<svg width="190" height="160" xmlns="http://www.w3.org/2000/svg">
  <path d="M 10 10 C 20 20, 40 20, 50 10" stroke-width="3" stroke="red" fill="transparent" %strokedasharray%/>
  <path d="M 70 10 C 70 20, 110 20, 110 10" stroke-width="3" stroke="red" fill="transparent" %strokedasharray%/>
  <path d="M 130 10 C 120 20, 180 20, 170 10" stroke-width="3" stroke="red" fill="transparent" %strokedasharray%/>
  <path d="M 10 60 C 20 80, 40 80, 50 60" stroke-width="3" stroke="red" fill="transparent" %strokedasharray%/>
  <path d="M 70 60 C 70 80, 110 80, 110 60" stroke-width="3" stroke="red" fill="transparent" %strokedasharray%/>
  <path d="M 130 60 C 120 80, 180 80, 170 60" stroke-width="3" stroke="red" fill="transparent" %strokedasharray%/>
  <path d="M 10 110 C 20 140, 40 140, 50 110" stroke-width="3" stroke="red" fill="transparent" %strokedasharray%/>
  <path d="M 70 110 C 70 140, 110 140, 110 110" stroke-width="3" stroke="red" fill="transparent" %strokedasharray%/>
  <path d="M 130 110 C 120 140, 180 140, 170 110" stroke-width="3" stroke="red" fill="transparent" %strokedasharray%/>
</svg>
)svg"

, // quadratic curve

R"svg(
<svg width="190" height="160" xmlns="http://www.w3.org/2000/svg">
  <path d="M 10 80 Q 95 10 180 80" stroke-width="3" stroke="red" fill="transparent" %strokedasharray%/>
</svg>
)svg"

};

std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }

    return str;
}

bool save_on_disk(const char* filename, std::string pSvg)
{
    FILE* file = fopen(filename, "w");
    if
        (file)
    {
        fputs(pSvg.c_str(), file);
        fclose(file);
        return true;
    }

    return false;
}


bool run_test(const std::string& svgNotResolved, int test_id, int strokedasharray_id)
{
    const char* strokedasharray_token_name = strokedasharray[strokedasharray_id][0];
    const char* strokedasharray_token_str = strokedasharray[strokedasharray_id][1];


    std::string lResolvedStr = replaceAll(svgNotResolved, "%strokedasharray%", strokedasharray_token_str);

    SVGDocument lDoc;
    bool test_success = lDoc.loadFromData(lResolvedStr);
    if (test_success)
    {
        Bitmap lBitmap = lDoc.renderToBitmap();
        test_success = test_success && lBitmap.width() != 0;

        char tmp_filename[1024];
        sprintf(tmp_filename, "test_%03d_%s.svg", test_id, strokedasharray_token_name);
        save_on_disk(tmp_filename, lResolvedStr);

        sprintf(tmp_filename, "test_%03d_%s.png", test_id, strokedasharray_token_name);
        test_success = test_success && stbi_write_png(tmp_filename, lBitmap.width(), lBitmap.height(), 4, lBitmap.data(), 0);

    }

    return test_success;
}

void run_tests()
{
    const int test_count = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < test_count; ++i)
    {        
        // original (must resolved token)
        std::string lSvgStr(tests[i]);

        const int strokedasharray_count = sizeof(strokedasharray) / sizeof(strokedasharray[0]);
        for (int stroke_id = 0; stroke_id < strokedasharray_count; ++stroke_id)
        {
            bool test_success = run_test(lSvgStr, i, stroke_id);
            if (!test_success)
            {
                printf("[ERROR] test_%03d\n", i);
            }
        }        
    }
}

int main()
{
    return run_tests();


    /*
    SVGDocument lDoc;
    bool lFontSuccess = lDoc.loadFontFromFile("D:\\tmp\\Roboto\\Roboto-Regular.ttf");

    std::string lFilename("d:\\_svg\\test_0312.svg");
    //std::string lFilename("d:\\_svg\\test_0314.svg");
    //std::string lFilename("d:\\_svg\\stroke-dasharray.svg");

    if (lDoc.loadFromFile(lFilename))
    {           
        Bitmap lBitmap = lDoc.renderToBitmap();
        int result = stbi_write_png("d:\\_svg\\out.png", lBitmap.width(), lBitmap.height(), 4, lBitmap.data(), 0);
        printf("result %d\n", result);
    }
    */

    
    return 0;
}
