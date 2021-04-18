/*	exDXExt.inl	- common convenience D2D and D3D extensions

	Copyright (c) Alexandre Egorov

	This software is provided 'as-is', without any warranty.
	This software can be used and modified freely for any purpose,
	including commercial applications,
	as long as its origin is not misrepresented.
 */
#pragma once
#define _CMN_DXEXT_INCLUDED_
#ifndef __dxgitype_h__
#include <dxgitype.h>
#endif
#ifndef DCOMMON_H_INCLUDED
#include <dcommon.h>
#endif


#ifndef D3DDDIERR_DEVICEREMOVED
#define D3DDDIERR_DEVICEREMOVED		((1 << 31) | (0x876 << 16) | 2160)
#endif

#define DXGI_FORMAT_DESKTOP_DEFAULT	DXGI_FORMAT_B8G8R8A8_UNORM
#define DXGI_FORMAT_DESKTOP_32BPP	DXGI_FORMAT_DESKTOP_DEFAULT
#define DXGI_FORMAT_DESKTOP			DXGI_FORMAT_DESKTOP_DEFAULT

#define FontPtSizeToDIP(pts)		((pts) * 96.f / 72)
#define FontDIPSizeToPt(pts)		((pts) * 72.f / 96)


INLINE bool_t DXFloatEq(float a, float b)
{
	return CPP_COMMONS(FloatEq(a, b, .001f));
}
INLINE bool_t DXFloatZero(float f)
{
	return (fabsf(f) <= .001f);
}
INLINE bool_t DXColorEq(float a, float b)
{
	return CPP_COMMONS(FloatEq(a, b, .0127f));
}
INLINE bool_t DXDoubleEq(double a, double b)
{
	return CPP_COMMONS(DoubleEq(a, b, .000001));
}
INLINE bool_t DXDoubleZero(double d)
{
	return (fabs(d) <= .000001);
}


CONSTEXPR INLINE D2D_SIZE_F DXSizeF_1()
{
	const UINT64 ui64 = 0x3F8000003F800000ull;
	return *(D2D_SIZE_F*)&ui64;
}
CONSTEXPR INLINE D2D_SIZE_F DXSizeF_127()
{
	const UINT64 ui64 = 0x42FE000042FE0000ull;
	return *(D2D_SIZE_F*)&ui64;
}
CONSTEXPR INLINE D2D_SIZE_F DXSizeF_255()
{
	const UINT64 ui64 = 0x437F0000437F0000ull;
	return *(D2D_SIZE_F*)&ui64;
}
CONSTEXPR INLINE D2D_SIZE_F DXSizeF_1d255()
{
	const UINT64 ui64 = 0x3B8080813B808081ull;
	return *(D2D_SIZE_F*)&ui64;
}


#ifdef __cplusplus	// ++++++++++++++++++++++++++++++++++++++++++++++++


inline void ToNonNeg(D2D_SIZE_F* psize)
{
	ToNonNeg32x2F(psize);
}
inline D2D_SIZE_F ToNonNeg(D2D_SIZE_F size)
{
	ToNonNeg32x2F(&size);
	return size;
}
inline void ToNonNeg(D2D_RECT_F* prc)
{
	ToNonNeg32x4F(prc);
}

inline INT32 DipsToPixelsT(FLOAT dips, FLOAT dpi)
{
	return FltToInt32((dips * dpi) / 96);
}
inline INT32 DipsToPixelsR(FLOAT dips, FLOAT dpi)
{
	return CPP_COMMONS(ftoi32f((dips * dpi) / 96));
}
inline INT32 DipsToPixelsC(FLOAT dips, FLOAT dpi)
{
	return FltToInt32(ceilf((dips * dpi) / 96));
}
#define DipsToPixels(dips, dpi)	DipsToPixelsC(dips, dpi)

inline FLOAT PixelsToDips(INT32 pixels, FLOAT dpi)
{
	return (pixels * 96) / dpi;
}


inline RECT& RectFToRect(_In_ const D2D_RECT_F* rcf, _Out_ PRECT rcl)
{
	_mm_storeu_si128((__m128i*)rcl, _mm_cvtps_epi32(_mm_loadu_ps((const float*)rcf)));
	return *rcl;
}
inline D2D_RECT_F& RectToRectF(_In_ const RECT* rcl, _Out_ D2D_RECT_F* rcf)
{
	_mm_storeu_ps((float*)rcf, _mm_cvtepi32_ps(_mm_loadu_si128((const __m128i*)rcl)));
	return *rcf;
}

inline bool IsSizeEmpty(_In_ D2D_SIZE_U size)
{
	return (!size.width) | (!size.height);
}
inline bool IsSizeEmpty(_In_ D2D_SIZE_F size)
{
	return (size.width <= 0) | (size.height <= 0);
}

inline UINT64 SizeArea(_In_ D2D_SIZE_U size)
{
	return Int32x32To64(size.width, size.height);
}
inline DOUBLE SizeArea(_In_ D2D_SIZE_F size)
{
	return (DOUBLE)size.width * size.height;
}

inline bool IsRectEmpty(_In_ const D2D_RECT_U& rc)
{
	return CPP_COMMONS(IsRectEmpty<D2D_RECT_U>(rc));
}
inline bool IsRectEmpty(_In_opt_ const D2D_RECT_U* prc)
{
	return (!prc || CPP_COMMONS(IsRectEmpty(*prc)));
}
inline bool IsRectEmpty(_In_ const D2D_RECT_F& rc)
{
	return CPP_COMMONS(IsRectEmpty<D2D_RECT_F>(rc));
}
inline bool IsRectEmpty(_In_opt_ const D2D_RECT_F* prc)
{
	return (!prc || CPP_COMMONS(IsRectEmpty(*prc)));
}

inline void EmptyRect(_Out_ D2D_RECT_F* prc)
{
	_mm_storeu_ps((float*)prc, _mm_setzero_ps());
}
inline void EmptyRect(_Out_ D2D_RECT_U* prc)
{
	Zero16Bytes(prc);
}

inline D2D_RECT_F& CopyRect(_Out_ D2D_RECT_F* dest, _In_ const D2D_RECT_F& src)
{
	_mm_storeu_ps((float*)dest, _mm_loadu_ps((const float*)&src));
	return *dest;
}

inline D2D_RECT_U& CopyRect(_Out_ D2D_RECT_U* dest, _In_ const D2D_RECT_U& src)
{
	Copy16Bytes(dest, &src);
	return *dest;
}

inline INT32 RectWidth(_In_ const D2D_RECT_U& rc)
{
	return (INT32)(rc.right - rc.left);
}
inline FLOAT RectWidth(_In_ const D2D_RECT_F& rc)
{
	return (rc.right - rc.left);
}
inline UINT32 RectWidthU(_In_ const D2D_RECT_U& rc)
{
	return CPP_COMMONS(ToNonNeg32(rc.right - rc.left));
}
inline FLOAT RectWidthU(_In_ const D2D_RECT_F& rc)
{
	return ToNonNegF(rc.right - rc.left);
}
inline INT32 RectHeight(_In_ const D2D_RECT_U& rc)
{
	return (INT32)(rc.bottom - rc.top);
}
inline FLOAT RectHeight(_In_ const D2D_RECT_F& rc)
{
	return (rc.bottom - rc.top);
}
inline UINT32 RectHeightU(_In_ const D2D_RECT_U& rc)
{
	return CPP_COMMONS(ToNonNeg32(rc.bottom - rc.top));
}
inline FLOAT RectHeightU(_In_ const D2D_RECT_F& rc)
{
	return ToNonNegF(rc.bottom - rc.top);
}
inline SIZE RectSize(_In_ const D2D_RECT_U& rc)
{
	return SIZE{ CPP_COMMONS(RectWidth(rc)), CPP_COMMONS(RectHeight(rc)) };
}
inline D2D_SIZE_U RectSizeU(_In_ const D2D_RECT_U& rc)
{
	return D2D_SIZE_U{ CPP_COMMONS(RectWidthU(rc)), CPP_COMMONS(RectHeightU(rc)) };
}
inline D2D_SIZE_U RectSizeU(_In_ const RECT& rc)
{
	return D2D_SIZE_U{ CPP_COMMONS(RectWidthU(rc)), CPP_COMMONS(RectHeightU(rc)) };
}
inline D2D_SIZE_F RectSize(_In_ const D2D_RECT_F& rc)
{
	return D2D_SIZE_F{ CPP_COMMONS(RectWidth(rc)), CPP_COMMONS(RectHeight(rc)) };
}
inline D2D_SIZE_F RectSizeU(_In_ const D2D_RECT_F& rc)
{
	return D2D_SIZE_F{ CPP_COMMONS(RectWidthU(rc)), CPP_COMMONS(RectHeightU(rc)) };
}

inline D2D_RECT_F& OffsetRect(_Inout_ D2D_RECT_F* prc, FLOAT dx, FLOAT dy)
{
	prc->left += dx;
	prc->right += dx;
	prc->top += dy;
	prc->bottom += dx;
	return *prc;
}

inline D2D_RECT_F& InflateRect(_Inout_ D2D_RECT_F* prc, FLOAT dx, FLOAT dy)
{
	prc->left -= dx;
	prc->top -= dx;
	prc->right += dy;
	prc->bottom += dy;
	return *prc;
}
inline D2D_RECT_F& InflateRect(_Inout_ D2D_RECT_F* prc, FLOAT d)
{
	return CPP_COMMONS(InflateRect(prc, d, d));
}

inline D2D_RECT_F ShrinkRect(_In_ const D2D_RECT_F& rc, FLOAT d)
{
	return D2D_RECT_F{ rc.left + d, rc.top + d, rc.right - d, rc.bottom - d };
}


inline D2D_SIZE_U ToSizeU(UINT64 size)
{
	return *reinterpret_cast<D2D_SIZE_U*>(&size);
}
inline D2D_SIZE_U ToSizeU(SIZE size)
{
	ToNonNeg32x2(&size);
	return *reinterpret_cast<D2D_SIZE_U*>(&size);
}
inline SIZE ToSize(D2D_SIZE_U size)
{
	return *reinterpret_cast<PSIZE>(&size);
}

inline D2D_POINT_2U ToPointU(UINT64 pt)
{
	return *reinterpret_cast<D2D_POINT_2U*>(&pt);
}
inline D2D_POINT_2U ToPointU(POINT pt)
{
	ToNonNeg32x2(&pt);
	return *reinterpret_cast<D2D_POINT_2U*>(&pt);
}
inline D2D_POINT_2U ToPointU(POINTL pt)
{
	ToNonNeg32x2(&pt);
	return *reinterpret_cast<D2D_POINT_2U*>(&pt);
}
inline POINT ToPoint(D2D_POINT_2U pt)
{
	return *reinterpret_cast<PPOINT>(&pt);
}

inline POINT ToPoint(POINTS pts)
{
	return { pts.x, pts.y };
}

inline UINT32 ToUInt32(D2D_SIZE_U size)
{
	return MAKEULONG(CPP_COMMONS(UInt32To16US(size.width)), CPP_COMMONS(UInt32To16US(size.height)));
}
inline UINT32 ToUInt32(D2D_POINT_2U pt)
{
	return MAKEULONG(CPP_COMMONS(UInt32To16US(pt.x)), CPP_COMMONS(UInt32To16US(pt.y)));
}
inline UINT64 ToUInt64(D2D_SIZE_U size)
{
	return *reinterpret_cast<PUINT64>(&size);
}
inline UINT64 ToUInt64(D2D_POINT_2U pt)
{
	return *reinterpret_cast<PUINT64>(&pt);
}
inline UINT64 ToUInt64(const D2D_RECT_U& rc)
{
	return MAKEULONGLONGW(CPP_COMMONS(UInt32To16US(rc.left)), CPP_COMMONS(UInt32To16US(rc.top)),
			CPP_COMMONS(UInt32To16US(rc.right)), CPP_COMMONS(UInt32To16US(rc.bottom)));
}


inline bool operator !=(const D2D_SIZE_U a, const D2D_SIZE_U b)
{
	return *(PUINT64)&a != *(PUINT64)&b;
}
inline bool operator <(const D2D_SIZE_U a, const D2D_SIZE_U b)
{
	return (a.width < b.width) | (a.height < b.height);
}
inline bool operator <=(const D2D_SIZE_U a, const D2D_SIZE_U b)
{
	return (a.width <= b.width) | (a.height <= b.height);
}
inline bool operator >(const D2D_SIZE_U a, const D2D_SIZE_U b)
{
	return ((a.width >= b.width) & (a.height > b.height)) | ((a.width > b.width) & (a.height >= b.height));
}
inline bool operator >=(const D2D_SIZE_U a, const D2D_SIZE_U b)
{
	return (a.width >= b.width) & (a.height >= b.height);
}

inline bool operator !=(const D2D_RECT_U& a, const D2D_RECT_U& b)
{
	return !IsMem16Equal(&a, &b);
}

#if !defined(_D2D1_HELPER_H_) || defined(D2D_USE_C_DEFINITIONS)
inline bool operator ==(const D2D_SIZE_U a, const D2D_SIZE_U b)
{
	return *(PUINT64)&a == *(PUINT64)&b;
}
inline bool operator ==(const D2D_RECT_U& a, const D2D_RECT_U& b)
{
	return IsMem16Equal(&a, &b);
}
#endif


inline bool operator ==(const D2D_SIZE_F a, const D2D_SIZE_F b)
{
	return (CPP_COMMONS(DXFloatEq(a.width, b.width)) & CPP_COMMONS(DXFloatEq(a.height, b.height)));
}
inline bool operator !=(const D2D_SIZE_F a, const D2D_SIZE_F b)
{
	return !(CPP_COMMONS(DXFloatEq(a.width, b.width)) & CPP_COMMONS(DXFloatEq(a.height, b.height)));
}

inline bool operator ==(const D2D_POINT_2F a, const D2D_POINT_2F b)
{
	return (CPP_COMMONS(DXFloatEq(a.x, b.x)) & CPP_COMMONS(DXFloatEq(a.y, b.y)));
}
inline bool operator !=(const D2D_POINT_2F a, const D2D_POINT_2F b)
{
	return !(CPP_COMMONS(DXFloatEq(a.x, b.x)) & CPP_COMMONS(DXFloatEq(a.y, b.y)));
}

inline bool operator ==(const D2D_POINT_2U a, const D2D_POINT_2U b)
{
	return *(PUINT64)&a == *(PUINT64)&b;
}
inline bool operator !=(const D2D_POINT_2U a, const D2D_POINT_2U b)
{
	return *(PUINT64)&a != *(PUINT64)&b;
}

inline bool operator ==(const D2D_POINT_2U a, const POINT b)
{
	return *(PUINT64)&a == *(PUINT64)&b;
}
inline bool operator !=(const D2D_POINT_2U a, const POINT b)
{
	return (UINT64*)&a != (UINT64*)&b;
}
inline bool operator ==(const POINT a, const D2D_POINT_2U b)
{
	return (UINT64*)&a == (UINT64*)&b;
}
inline bool operator !=(const POINT a, const D2D_POINT_2U b)
{
	return *(PUINT64)&a != *(PUINT64)&b;
}

inline bool operator ==(const D2D_POINT_2U a, const POINTL b)
{
	return *(PUINT64)&a == *(PUINT64)&b;
}
inline bool operator !=(const D2D_POINT_2U a, const POINTL b)
{
	return *(PUINT64)&a != *(PUINT64)&b;
}
inline bool operator ==(const POINTL a, const D2D_POINT_2U b)
{
	return *(PUINT64)&a == *(PUINT64)&b;
}
inline bool operator !=(const POINTL a, const D2D_POINT_2U b)
{
	return *(PUINT64)&a != *(PUINT64)&b;
}

inline bool operator ==(const RECT& a, const D2D_RECT_U& b)
{
	return ::IsMem16Equal(&a, &b);
}
inline bool operator !=(const RECT& a, const D2D_RECT_U& b)
{
	return !::IsMem16Equal(&a, &b);
}
inline bool operator ==(const D2D_RECT_U& a, const RECT& b)
{
	return ::IsMem16Equal(&a, &b);
}
inline bool operator !=(const D2D_RECT_U& a, const RECT& b)
{
	return !::IsMem16Equal(&a, &b);
}

inline bool operator ==(const RECTL& a, const D2D_RECT_U& b)
{
	return ::IsMem16Equal(&a, &b);
}
inline bool operator !=(const RECTL& a, const D2D_RECT_U& b)
{
	return !::IsMem16Equal(&a, &b);
}
inline bool operator ==(const D2D_RECT_U& a, const RECTL& b)
{
	return ::IsMem16Equal(&a, &b);
}
inline bool operator !=(const D2D_RECT_U& a, const RECTL& b)
{
	return !::IsMem16Equal(&a, &b);
}


inline bool operator ==(_In_ const D2D1_PIXEL_FORMAT& pf1, _In_ const D2D1_PIXEL_FORMAT& pf2)
{
	STATIC_ASSERT(sizeof(D2D1_PIXEL_FORMAT) == sizeof(UINT64));
	return *((PUINT64)&pf1) == *((PUINT64)&pf2);
}
inline bool operator !=(_In_ const D2D1_PIXEL_FORMAT& pf1, _In_ const D2D1_PIXEL_FORMAT& pf2)
{
	STATIC_ASSERT(sizeof(D2D1_PIXEL_FORMAT) == sizeof(UINT64));
	return *((PUINT64)&pf1) != *((PUINT64)&pf2);
}


// Colors /////////////////////////////////////////////////////////////


inline void __fastcall BgraToColorF(_In_ DWORD argb, _Out_ D3DCOLORVALUE* colorF)
{
	_mm_storeu_ps(reinterpret_cast<float*>(colorF), CPP_COMMONS(mm_cvtbgra_clrf(argb)));
}
FORCEINLINE D3DCOLORVALUE BgraToColorF(_In_ DWORD argb)
{
	D3DCOLORVALUE colorF;
	CPP_COMMONS(BgraToColorF(argb, &colorF));
	return colorF;
}

inline void __fastcall RgbToColorF(_In_ COLORREF cr, _Out_ D3DCOLORVALUE* colorF)
{
	_mm_storeu_ps(reinterpret_cast<float*>(colorF), CPP_COMMONS(mm_cvtrgba_clrf(cr | 0xFF000000)));
}
FORCEINLINE D3DCOLORVALUE RgbToColorF(_In_ COLORREF cr)
{
	D3DCOLORVALUE colorF;
	CPP_COMMONS(RgbToColorF(cr, &colorF));
	return colorF;
}
inline bool __fastcall TryRgbToColorF(_In_ COLORREF cr, _Out_ D3DCOLORVALUE* colorF)
{
	_mm_storeu_ps(reinterpret_cast<float*>(colorF), CPP_COMMONS(mm_cvtrgba_clrf(cr | 0xFF000000)));
	return (CLR_INVALID != cr);
}


#endif	// __cplusplus	+++++++++++++++++++++++++++++++++++++++++++++++




