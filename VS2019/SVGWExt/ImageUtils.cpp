// DXUtils.cpp

#include "pch.h"
#include "Main.h"

namespace RootNamespace {

///////////////////////////////////////////////////////////////////////
// Validation etc. ////////////////////////////////////////////////////


WCXSTDAPI_(D2D_SIZE_F) wcScaleSizeF(_In_ D2D_SIZE_F size, _In_opt_ UINT maxSide)
{
	if (AnyTrue(size.width > maxSide, size.height > maxSide) && AllTrue(maxSide, size.width, size.height))
	{
		if ((UINT)fabsf(size.width - size.height) > 1)
		{
			const float ratio = size.width / size.height;
			if (ratio > 1)
			{
				size.width = (FLOAT)maxSide;
				size.height = ceilf(size.width / ratio);
			}
			else
			{
				size.height = (FLOAT)maxSide;
				size.width = ceilf(size.height * ratio);
			}
		}
		else size.height = size.width = (FLOAT)maxSide;
	}
	else
	{
		size.width = ceilf(size.width);
		size.height = ceilf(size.height);
	}
	return size;
}


NOALIAS bool __fastcall IsRectValid(_In_ const WICRect& rc, UINT maxWidth, UINT maxHeight)
{
	return (AllTrue((UINT)(rc.X + rc.Width) <= maxWidth, (UINT)(rc.Y + rc.Height) <= maxHeight)
			&& AllTrue(rc.X >= 0, rc.Y >= 0, rc.Width > 0, rc.Height > 0));
}


///////////////////////////////////////////////////////////////////////
// Misc. helpers //////////////////////////////////////////////////////


_Check_return_ NOALIAS
HRESULT StreamSeekBack(_In_ IStream* pstm, _In_ STREAM_PARAM_CALLBACK pfnCallback, _Inout_opt_ void* cbkParam)
{
	union {
		ULARGE_INTEGER curPos = { 0 };
		LARGE_INTEGER setPos;
	};
	HRESULT hr = pstm->Seek({ 0 }, STREAM_SEEK_CUR, &curPos);
	if (SUCCEEDED(hr))
	{
		hr = pfnCallback(pstm, curPos.QuadPart, cbkParam);
		const HRESULT hrTmp = pstm->Seek(setPos, STREAM_SEEK_SET, nullptr);
		if (AllTrue(FAILED(hrTmp), SUCCEEDED(hr)))
			hr = hrTmp;
	}
	return hr;
}


///////////////////////////////////////////////////////////////////////
// D2D & GDI //////////////////////////////////////////////////////////


_Success_(return != 0) WCXFASTAPI_(HBITMAP) wcCreateDIB32(SIZE size, _Outptr_opt_ PDWORD32* ppBits)
{
	PDWORD32 bits = nullptr;
	const UINT square = uabs(size.cx) * uabs(size.cy);
	union {
		HBITMAP hbm;
		BITMAPINFO bi;
	};
	ZeroStruct(&bi);
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = size.cx;
	bi.bmiHeader.biHeight = size.cy;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
#if (0 != BI_RGB)
	bi.bmiHeader.biCompression = BI_RGB;
#endif
	bi.bmiHeader.biSizeImage = square * 4;
	hbm = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, (void**)&bits, NULL, 0);
	if (hbm)
		wcSetMemory32(bits, 0, square);
	if (ppBits)
		*ppBits = bits;
	return hbm;
}


_Success_(return == S_OK) WCXFASTAPI wcD2DBitmapToDIB(_In_ ID2D1Bitmap1* d2dBm, _Out_ HBITMAP* phbmOut)
{
	*phbmOut = NULL;
	D2D_SIZE_U size = d2dBm->GetPixelSize();
	PDWORD32 pdwBits;
	const HBITMAP hbm = wcCreateDIB32({ (LONG)size.width, (LONG)size.height }, &pdwBits);
	if (hbm)
	{
		D2D1_MAPPED_RECT map;
		HRESULT hr = d2dBm->Map(D2D1_MAP_OPTIONS_READ, &map);
		if (S_OK == hr)
		{
			pdwBits += (size.height - 1) * size.width;
			*phbmOut = hbm;
			const int oldCSR = mm_set_rounding_mode(_MM_ROUND_NEAREST);
			for (; size.height; size.height--)
			{
				PremulArgbToArgb((PDWORD32)pdwBits, (const DWORD32*)map.bits, size.width);
				map.bits += map.pitch;
				pdwBits -= size.width;
			}
			_mm_setcsr(oldCSR);
			d2dBm->Unmap();
		}
		return hr;
	}
	return HRESULT_WIN_ERROR;
}


///////////////////////////////////////////////////////////////////////
// SVG document ///////////////////////////////////////////////////////

#define MAX_SVGTAG_SIZE	48


_Success_(return == S_OK)
HRESULT __stdcall QuerySvgElemText(_In_ ID2D1SvgElement* pelem, bool queryChild, _Outptr_result_nullonfailure_ PWSTR* ppszText)
{
	UINT clen = pelem->GetTextValueLength();
	HRESULT hr = S_FALSE;
	if (clen)
	{
		clen = ALIGN4(clen + 2);
		const PWSTR wcsText = (PWSTR)::CoTaskMemAlloc((SIZE_T)clen * sizeof(WCHAR));
		hr = E_OUTOFMEMORY;
		if (wcsText)
		{
			hr = pelem->GetTextValue(wcsText, clen);
			if (S_OK == hr)
			{
				*ppszText = wcsText;
				return S_OK;
			}
			::CoTaskMemFree(wcsText);
		}
	}
	else if (queryChild && pelem->HasChildren())
	{
		pelem->GetFirstChild(&pelem);
		if (pelem)
		{
			hr = QuerySvgElemText(pelem, false, ppszText);
			pelem->Release();
			if (S_OK == hr)
				return S_OK;
		}
	}
	*ppszText = nullptr;
	return hr;
}


_Success_(return == S_OK)
WCXSTDAPI wcGetSvgElemText(_In_ ID2D1SvgElement* pelem, _Outptr_result_nullonfailure_ PWSTR* ppszText)
{
	return QuerySvgElemText(pelem, true, ppszText);
}


WCXSTDAPI_(ID2D1SvgElement*) wcFindSvgElemChild(_In_ ID2D1SvgElement* pelem, _In_reads_(cchTag) PCWCH pwcTagName, UINT cchTag)
{
	ID2D1SvgElement* child = nullptr;
	if (pelem->HasChildren())
	{
		ID2D1SvgElement* nextChild;
		union {
			HRESULT hr;
			WCHAR wcName[MAX_SVGTAG_SIZE];
		};
		if (cchTag < _countof(wcName))
		{
			pelem->GetFirstChild(&child);
			while (child)
			{
				if (child->GetTagNameLength() == cchTag && S_OK == child->GetTagName(wcName, _countof(wcName))
					&& 0 == wmemcmp(wcName, pwcTagName, cchTag))
				{
					break;
				}
				nextChild = nullptr;
				hr = pelem->GetNextChild(child, &nextChild);
				child->Release();
				child = nullptr;
				if (S_OK != hr)
					break;
				child = nextChild;
			}
		}
	}
	return child;
}


_Success_(return == S_OK)
WCXSTDAPI wcGetSvgChildText(_In_ ID2D1SvgElement* pelem, _In_reads_(cchTag) PCWCH pwcTagName,
		UINT cchTag, _Outptr_result_nullonfailure_ PWSTR* ppszText)
{
	ID2D1SvgElement* const child = ::wcFindSvgElemChild(pelem, pwcTagName, cchTag);
	if (child)
	{
		const HRESULT hr = ::wcGetSvgElemText(child, ppszText);
		child->Release();
		return hr;
	}
	*ppszText = nullptr;
	return HRESULT_WIN_ERROR;
}


}	// namespace