/*

	SVG2ICO: https://github.com/g40

	Copyright (c) 2024, Jerry Evans

	A how-to for using various Windows APIs to
	create a multiple resolution icon file from a
	single SVG image rendered via LunaSVG.

	BSD 2-clause license applies.
*/


#include <windows.h>
#include "svg2ico.h"
#include <sstream>

//-----------------------------------------------------------------------------
// debug settings, assuming CMake build directory is in root
// properties >> debugging
// command arguments: picasso.svg picasso.ico 256x256 128x128 72x72
// working directory ..\examples\win32\svg2ico
int main(int argc, char** argv)
{
	int ret = -1;
	// 
	std::vector<std::string> args;
	for (int i = 1; argv[i] != nullptr; i++)
	{
		args.push_back(argv[i]);
	}
	//
	if (args.size() < 2)
	{
		std::cout << "Usage: \n"
					"   svg2ico [.SVG input filename] [.ICO output file] { width*height }+ { background BGRA value }\n"
					"Examples: \n"
					"svg2ico tiger.svg tiger.ico\n"
					"svg2ico tiger.svg tiger.ico 256x256 128x128 64x32\n"
					"svg2ico tiger.svg tiger.ico 256x256 128x128 64x32 0x00000000\n";
		return ret;
	}

	try
	{
		//
		std::string ipname = args[0];
		std::string opname = args[1];
		// default to 128x128
		std::vector<std::pair<int, int>> dimensions = { {128,128} };
		// transparent black background
		std::uint32_t bgColor = 0x00000000;
		if (args.size() >= 3)
		{
			for (size_t s = 2; s < args.size(); s++)
			{
				std::string arg = args[s];
				if (arg.find("0x") == 0)
				{
					// parse out background color.
					std::stringstream ss;
					ss << std::hex << arg;
					ss >> std::hex >> bgColor;
				}
				else
				{
					// parse out WxH
					int width = 0;
					int height = 0;
					std::stringstream ss;
					ss << arg;
					ss >> width;
					if(ss.fail() || ss.get() != 'x')
						throw std::runtime_error("Expecting 'x' in dimension");
					ss >> height;
					if (width <= 0 || width > 256)
						throw std::runtime_error("width must be non-zero and 256 or less");
					if (height <= 0 || height > 256)
						throw std::runtime_error("height must be non-zero and 256 or less");
					// already got this default dimension
					if (!(width == 128 && height == 128))
						dimensions.push_back(std::make_pair(width,height));
				}
			}
		}
		// render SVG, create ICO.
		if (ico::MakeIcon(ipname,opname,dimensions,bgColor))
		{
			std::cout << "Created icon file: " << opname << std::endl;
			ret = 0;
		}
	}
	catch(std::exception& ex)
	{
		std::cout << "Error: " << ex.what() << std::endl;
	}
	return ret;
}


