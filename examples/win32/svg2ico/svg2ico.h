/*

	Windows + LunaSVG specific. 

	Create a 32BPP transparent icon file from an SVG image
		
*/

#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <lunasvg.h>

//-----------------------------------------------------------------------------
// required for AlphaBlend
#pragma comment(lib, "Msimg32.lib")


#ifndef DBMSG
#define DBMSG(arg)
#endif

namespace ico
{
	//-----------------------------------------------------------------------------
	// helper for streaming arbitrary structures etc to a buffer
	// streamer s;
	// s << int(4);
	// s << float(4.321);
	class streamer
	{
		//
		std::vector<char> m_buffer;
		//
		size_t write(const char* ps, size_t size)
		{
			m_buffer.insert(m_buffer.end(), ps, ps + size);
			return size;
		}
	public:
		//
		streamer() {}

		// variable length
		// use std::make_pair<>(ps,size) to create *arg*
		template <typename T, typename S>
		streamer& operator<<(const std::pair<T*, S>& arg)
		{
			size_t size = arg.second;
			const char* ps = reinterpret_cast<const char*>(arg.first);
			write(ps,size);
			return (*this);
		}

		// fixed size 'thing', scalar or struct
		template <typename T>
		streamer& operator<<(const T& arg)
		{
			size_t size = sizeof(arg);
			const char* ps = reinterpret_cast<const char*>(&arg);
			write(ps, size);
			return (*this);
		}

		// for writing, return a reference or raw pointer/size
		const std::vector<char>& ref() const { return m_buffer; }
		const char* data() const { return m_buffer.data(); }
		int size() const { return static_cast<int>(m_buffer.size()); }

		// e.g. ICONHEADER* pih = streamer.get<ICONHEADER>(0);
		template <typename T>
		T* get(size_t offset) { 
			T* ret = ((offset + sizeof(T)) <= m_buffer.size() ? 
				(T*) (m_buffer.data() + offset) : 
				nullptr); 
			return ret;
		}

		// const version
		template <typename T>
		const T* get(size_t offset) const { 
			T* ret = ((offset + sizeof(T)) <= m_buffer.size() ?
				(T*)(m_buffer.data() + offset) :
				nullptr);
			return ret;
		}

		// return scalar or struct
		template <typename T>
		T value(size_t offset) {
			T* ret = (T*)get<T*>(offset);
			return *ret;
		}

	};

	//-----------------------------------------------------------------------------
	// make sure we clean up. not foolproof but helps somewhat
	class DCHandle
	{
		HDC m_hDC = NULL;
		DCHandle(const DCHandle&) = delete;
		DCHandle& operator=(const DCHandle&) = delete;
	public:
		//
		DCHandle(HDC arg = NULL)
		{
			m_hDC = CreateCompatibleDC(arg);
		}
		//
		~DCHandle()
		{
			if (m_hDC)
			{
				DeleteDC(m_hDC);
				m_hDC = NULL;
			}
		}
		// intentional
		HDC Handle() { return m_hDC; }
	};

	//-----------------------------------------------------------------------------
	// Helpers to release GDI object handle
	template <typename T>
	class GDIHandle
	{
		T m_handle = NULL;
		GDIHandle() = delete;
		GDIHandle& operator=(const GDIHandle&) = delete;
	public:
		// not safe. should throw if already assigned.
		GDIHandle(T handle) : m_handle(handle) {}
		//
		~GDIHandle()
		{
			if (m_handle)
			{
				DeleteObject(m_handle);
				m_handle = NULL;
			}
		}
		// intentional
		T Handle() { return m_handle; }
	};

	//-----------------------------------------------------------------------------
	// for wretched variable length structures
	template <typename T>
	class Buffer
	{
		std::vector<uint8_t> m_data;
	public:
		Buffer(size_t size) : m_data(size, 0) {}
		size_t size() const { return m_data.size(); }
		T* data() { return reinterpret_cast<T*>(m_data.data()); }
		const T* data() const { return reinterpret_cast<const T*>(m_data.data()); }
	};
}

//-----------------------------------------------------------------------------
namespace ico
{

// https://docs.fileformat.com/image/ico/
struct ICONHEADER
{
	// always 0
	uint16_t reserved = 0;
	// 1 == .ico, 2 == .cur
	uint16_t type = 1;
	// number of images in the file
	uint16_t count = 1;
};

//-----------------------------------------------------------------------------
// https://www.daubnet.com/en/file-format-ico
//
struct ICONDIRENTRY
{
	uint8_t bWidth = 0;
	uint8_t bHeight = 0;
	uint8_t bColorCount = 0; // 0 if no palette
	uint8_t bReserved = 0; // should be 0
	uint16_t wPlanes = 1; // 0 or 1
	uint16_t wBitCount = 32;
	uint32_t nDataLength = 0; // length in alignedSize
	uint32_t nOffset = 0xBEEF; // offset from beginning of file to Nth BITMAPINFOHEADER
}; 

//-----------------------------------------------------------------------------
static void
dump(int index, const ICONDIRENTRY& dirEntry)
{
#if _DEBUG
	//
	DBMSG("dirEntry: " << index
		<< " w:" << (dirEntry.bWidth ? dirEntry.bWidth : 256)
		<< " h:" << (dirEntry.bHeight ? dirEntry.bHeight : 256)
		<< " colors:" << dirEntry.bColorCount
		<< " res:" << dirEntry.bReserved
		<< " planes:" << dirEntry.wPlanes
		<< " bpp:" << dirEntry.wBitCount
		<< " len:" << dirEntry.nDataLength
		<< " off:" << dirEntry.nOffset);
#endif
}

//-----------------------------------------------------------------------------
static 
void
dump(BITMAP bitmap)
{
#if _DEBUG
	//
	DBMSG("bmType: " << bitmap.bmType 
		<< " bmWidth: " << bitmap.bmWidth
		<< " bmHeight: " << bitmap.bmHeight
		<< " bmWidthBytes: " << bitmap.bmWidthBytes
		<< " bmPlanes: " << bitmap.bmPlanes
		<< " bmBitsPixel: " << bitmap.bmBitsPixel
		<< " bmBits: 0x" << u::to_hex<>((uint64_t)bitmap.bmBits));
#endif
}

//-----------------------------------------------------------------------------
// 32bpp only.
// 
// credit (!)
// https://stackoverflow.com/questions/2289894/how-can-i-save-hicon-to-an-ico-file
//
//-----------------------------------------------------------------------------
static
UINT
GetAlignedSize(BITMAP* pBitmap)
{
	int nWidthBytes = pBitmap->bmWidthBytes;
	// bitmap scanlines MUST be a multiple of 4 alignedSize when stored
	// inside a bitmap resource, so round up if necessary
	if (nWidthBytes & 3)
	{
		nWidthBytes = (nWidthBytes + 4) & ~3;
	}
	//
	// DBMSG((nWidthBytes & 3 ? "Padded:" : "Aligned:") << pBitmap->bmWidthBytes << " => "  << nWidthBytes);
	//
	return nWidthBytes * pBitmap->bmHeight;
}

//-----------------------------------------------------------------------------
// write the bitmap image data to the streamer. align and orient correctly.
static
bool
WriteIconImageData(streamer& img_buffer, HBITMAP hBitmap)
{
	BITMAP bmp;
	if (GetObject(hBitmap, sizeof(BITMAP), &bmp) == 0)
		return false;
	//
	int alignedSize = GetAlignedSize(&bmp);
	if (alignedSize == 0)
		return false;
	std::shared_ptr<BYTE> iconData = std::shared_ptr<BYTE>(new BYTE[alignedSize]);
	if (!iconData.get())
		return false;
	BYTE* pIconData = iconData.get();
	if (GetBitmapBits(hBitmap, alignedSize, pIconData) == 0)
		return false;
	// on-disk is top down and 32bit aligned
	uint8_t padding[4]{ 0 };
	int mod = (bmp.bmWidthBytes % 4);
	//
	for (int i = bmp.bmHeight - 1; i >= 0; i--)
	{
		// write 1 line at a time
		img_buffer << std::make_pair<>(pIconData + (i * bmp.bmWidthBytes), bmp.bmWidthBytes);
		// extend to a 32bit boundary if necessary
		if (mod)
		{
			img_buffer << std::make_pair<>(padding, 4 - mod);
		}
	}
	//
	return true;
}

//-----------------------------------------------------------------------------
// 32bpp pixel data, ((w * h) * 32) / 8
// 1 bpp mask data, ((w * h) * 1) / 8
// width should be aligned to 32bit boundary
static
ICONDIRENTRY
WriteIconImage(streamer& img_buffer, HBITMAP hColor, HBITMAP hMask, size_t count)
{
	//
	int fixedOffset = (int)(sizeof(ICONHEADER) + (count * sizeof(ICONDIRENTRY)));

	// calculate how much space the bitmaps take
	BITMAP bmpColor;
	GetObject(hColor, sizeof(BITMAP), &bmpColor);
	UINT alignedSize = GetAlignedSize(&bmpColor);
	dump(bmpColor);

	BITMAP bmpMask;
	GetObject(hMask, sizeof(BITMAP), &bmpMask);
	alignedSize += GetAlignedSize(&bmpMask);
	dump(bmpMask);

	// write the ICONIMAGE to disk (first the BITMAPINFOHEADER)
	BITMAPINFOHEADER biHeader{ 0 };
	// Fill in only those fields that are necessary
	biHeader.biSize = sizeof(biHeader);
	biHeader.biWidth = bmpColor.bmWidth;
	biHeader.biHeight = bmpColor.bmHeight * 2; // height of color+mono
	biHeader.biPlanes = bmpColor.bmPlanes;	// should be 1
	biHeader.biBitCount = bmpColor.bmBitsPixel;
	biHeader.biSizeImage = alignedSize;

	//
	ICONDIRENTRY dirEntry{ 0 };
	// N.B. 256x256 == 0x0
	dirEntry.bWidth = (uint8_t)bmpColor.bmWidth;
	dirEntry.bHeight = (uint8_t)bmpColor.bmHeight;
	// should always be 32
	dirEntry.wBitCount = bmpColor.bmBitsPixel;
	// from https://devblogs.microsoft.com/oldnewthing/20101018-00/?p=12513
	// dirEntry.bColorCount = 1 << (dirEntry.wBitCount * dirEntry.wPlanes);
	// If wBitCount * wPlanes is greater than or equal to 8, then bColorCount is zero.
	dirEntry.bColorCount = 0;
	// yes, this includes the header itself.
	// https://devblogs.microsoft.com/oldnewthing/20101019-00/?p=12503
	// in order as on disk. 
	dirEntry.nDataLength = sizeof(BITMAPINFOHEADER) + alignedSize;
	// set up the offset to the image data
	dirEntry.nOffset = fixedOffset + img_buffer.size();

	// write the BITMAPINFOHEADER
	img_buffer << biHeader;
	// write the image and mask pixel data
	WriteIconImageData(img_buffer, hColor);
	WriteIconImageData(img_buffer, hMask);

	//
	return dirEntry;
}

//-----------------------------------------------------------------------------
// process each icon and serialize to the output buffer
static
bool IconsToBuffer(const std::vector<HICON>& icons, streamer& op_buffer)
{
	//
	int index = 1;
	// header: ICO file with 1 image
	ICONHEADER icoHeader;
	icoHeader.count = (uint16_t)icons.size();
	op_buffer << icoHeader;
	// intermediates to simplify directory construction
	streamer dir_buffer;
	streamer img_buffer;
	// 
	for (auto& hIcon : icons)
	{
		// for RAII
		DCHandle dc;
		// Get information about icon:
		ICONINFO iconInfo;
		GetIconInfo(hIcon, &iconInfo);
		// get the bitmap handles.
		GDIHandle<HBITMAP> hbmColor(iconInfo.hbmColor);
		GDIHandle<HBITMAP> hbmMask(iconInfo.hbmMask);
		// writes header, color pixels, mask pixels.
		ICONDIRENTRY dirEntry = WriteIconImage(img_buffer, hbmColor.Handle(), hbmMask.Handle(), icons.size());
		//
		// write the directory entry
		dir_buffer << dirEntry;
		//
		index++;
	}
	// buffer the directory
	op_buffer << std::make_pair<>(dir_buffer.data(), dir_buffer.size());
	// buffer image headers and data
	op_buffer << std::make_pair<>(img_buffer.data(), img_buffer.size());
	
#if _DEBUG
	{
		// sanity check what was buffered. walk the directory
		ICONHEADER* pIH = op_buffer.get<ICONHEADER>(0);
		ICONDIRENTRY* pIDE = op_buffer.get<ICONDIRENTRY>(sizeof(ICONHEADER));
		for (int i = 0; i < pIH->count; i++)
		{
			ICONDIRENTRY dirEntry = pIDE[i];
			//
			dump(i, dirEntry);
		}
	}
#endif
	//
	return true;
}

//-----------------------------------------------------------------------------
// Create an HBITMAP with a DIBSECTION onto which we can copy pixel data
// version 1 gives caller option to do copying ...
static HBITMAP build(HDC hDC, int width, int height, int bpp, /* _RETURN _WRITEABLE */ uint8_t** pixels)
{
	HBITMAP hBitmap = 0;
	BITMAPINFO bmpInfo = { 0 };
	// bpp is 1-4, as in alignedSize per pixel.
	bmpInfo.bmiHeader.biBitCount = (WORD)bpp * 8;
	bmpInfo.bmiHeader.biWidth = width;
	// top down ...!
	bmpInfo.bmiHeader.biHeight = -height;
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biCompression = BI_RGB;

	// Pointer to access the pixels of bitmap
	hBitmap = CreateDIBSection(hDC, &bmpInfo, DIB_RGB_COLORS, (void**)pixels, 0, 0);
	//
	return hBitmap;
}

//-----------------------------------------------------------------------------
// HICON must be cleaned up with ::DestroyIcon(hIcon)
static
HICON IconFromBitmap(lunasvg::Bitmap& bitmap)
{
	//
	const int bpp = 4;
	//
	DCHandle hDC;
	//
	DCHandle memDC(hDC.Handle());

	// this is the DIB section data
	uint8_t* pixels = nullptr;
	HBITMAP hBitmap = build(memDC.Handle(), bitmap.width(), bitmap.height(), bpp, &pixels);
	// copy our ARGB data into the bitmap data
	memcpy(pixels, bitmap.data(), bitmap.width() * bitmap.height() * bpp);

	//
	HGDIOBJ oldBitmap = SelectObject(memDC.Handle(), hBitmap);

	// alpha-blend so we get true transparency
	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = 255;
	bf.AlphaFormat = AC_SRC_ALPHA;
	::AlphaBlend(hDC.Handle(), 0, 0, bitmap.width(), bitmap.height(), 
			memDC.Handle(), 0, 0, bitmap.width(), bitmap.height(), bf);
	
	// 	
	SelectObject(memDC.Handle(), oldBitmap);
	
	// make icon at runtime
	ICONINFO ii{ 0 };
	ii.hbmColor = hBitmap;
	ii.hbmMask = CreateCompatibleBitmap(hDC.Handle(), bitmap.width(), bitmap.height());
	HICON hIcon = CreateIconIndirect(&ii);
	DeleteObject(ii.hbmMask);
	DeleteObject(hBitmap);
	//
	return hIcon;
}

//-----------------------------------------------------------------------------
// create a file from previously serialized data
static
bool
create_file(const std::vector<char>& data, const std::string& opname)
{
	std::ofstream opfile{ opname, std::ios::binary | std::ios::trunc };
	if (!opfile.good())
		return false;
	opfile.write(data.data(), data.size());
	return true;
}

//-----------------------------------------------------------------------------
// create an icon with 1+ entries from an SVG file
// 256x256 maximum supported size. 32bpp only
static
bool
MakeIcon(
	// .svg filename
	const std::string& ipname,
	// .ico filename
	const std::string& opname,
	// add size pairs to suit
	const std::vector<std::pair<int, int>> dimensions = { {128,128} },
	// transparent background
	std::uint32_t bgColor = 0x00000000)
{
	// for serializing
	ico::streamer st;

	// load SVG
	std::unique_ptr<lunasvg::Document> document = lunasvg::Document::loadFromFile(ipname);
	if (!document)
		return false;

	{
		// this is a bit round-about because we could synthesize the icon data
		// directly from the BGRA lunasvg::Bitmap. 
		std::vector<HICON> icons;
		for (auto dimension : dimensions)
		{
			// transparent black background
			lunasvg::Bitmap bitmap = document->renderToBitmap(dimension.first, dimension.second, bgColor);
			if (!bitmap.valid())
				return false;
			// create an ICON handle
			HICON icon = IconFromBitmap(bitmap);
			if (!icon)
				return false;
			icons.push_back(icon);
		}
		// and then de-construct for serializing
		if (!IconsToBuffer(icons, st))
			return false;
		// clean up
		for (auto icon : icons)
		{
			DestroyIcon(icon);
		}
	}

	// write the serialized icon data
	bool ok = create_file(st.ref(), opname);
	//
	return ok;
}
}

