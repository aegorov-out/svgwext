/*	exSDKExt.inl	- common Windows SDK extensions

	Copyright (c) Alexandre Egorov

	This software is provided 'as-is', without any warranty.
	This software can be used and modified freely for any purpose,
	including commercial applications,
	as long as its origin is not misrepresented.
 */
#pragma once
#define _CMN_SDKEXT_INCLUDED_

#ifndef _CMN_WINSDK_INCLUDED_
#include <intrin.h>
#include <immintrin.h>
#include <math.h>
#endif


// Types etc. /////////////////////////////////////////////////////////


#define MAX_NOGROWHEAP_BLOCK	0x7FFF7u
#define MAX_PERFHEAP_BLOCK		0x7FFF0u


#ifdef INFOTIPSIZE
#define MAX_INFOTIP			INFOTIPSIZE
#else
#define MAX_INFOTIP			1024
#endif
#ifdef MAX_PATH
#define SAFE_PATH			(MAX_PATH + 12)		// 272
#else
#define SAFE_PATH			272
#endif	// MAX_PATH
#define MAX_LONG_PATH		(MAX_INFOTIP - 32)	// 992
#define MAX_PATH_BUFF		MAX_INFOTIP			// 1024
#define MAX_PATH_SIZE		MAX_PATH_BUFF

#define MAX_LABEL_TEXT		80
#define MAX_LABEL_TIP		MAX_LABEL_TEXT


INLINE HRESULT MeaningHRESULT(_In_ HRESULT hr)
{
	return ((S_FALSE != hr) & (HRESULT_INVALID != hr)) ? hr : E_UNEXPECTED;
}
_Check_return_ _Post_satisfies_(return != NOERROR || return == defCode)
INLINE unsigned long EnsureErrorCode(unsigned long code, unsigned long defCode DEFARG_((unsigned long)E_FAIL))
{
	return (NOERROR != code) ? code : defCode;
}


#define EXTRACT_DOSYEAR(wd)		(unsigned __int8)((unsigned __int16)(wd)>>9)
#define GET_DOSYEAR(wd)			(EXTRACT_DOSYEAR(wd)+1980)
#define GET_DOSMONTH(wd)		(unsigned __int8)(((unsigned __int16)(wd)>>5)&0xF)
#define GET_DOSDATE(wd)			(unsigned __int8)((unsigned __int16)(wd)&0x1F)

#define GET_DOSHOUR(wt)			(unsigned __int8)((unsigned __int16)(wt)>>11)
#define GET_DOSMINUTE(wt)		(unsigned __int8)(((unsigned __int16)(wt)>>5)&0x7F)
#define EXTRACT_DOSSECOND(wt)	(unsigned __int8)((unsigned __int16)(wt)&0x1F)
#define GET_DOSSECOND(wt)		(EXTRACT_DOSSECOND(wt)*2)

#define PACK_DOSDATE(y,m,d)		(((unsigned __int8)((y)-1980)<<9)|(((unsigned __int8)(m)&0xF)<<5)|((unsigned __int8)(d)&0x1F))
#define PACK_DOSTIME(h,m,s)		(((unsigned __int8)(h)<<11)|(((unsigned __int8)(m)&0x7F)<<5)|((unsigned __int8)((s)/2)&0x1F))

#define FILETIME_SECOND			((ULONGLONG)10000000)
#define FILETIME_MINUTE			(60 * FILETIME_SECOND)
#define FILETIME_HOUR			(60 * FILETIME_MINUTE)
#define FILETIME_DAY			(24 * FILETIME_HOUR)
#define FILETIME_WEEK			(7 * FILETIME_DAY)

#ifdef __cplusplus
FORCEINLINE unsigned __int64 MakeFileTime64(FILETIME ft)
{
	return CPP_COMMONS(ToUInt64<FILETIME>(ft));
}
#else
FORCEINLINE unsigned __int64 MakeFileTime64(FILETIME ft)
{
	return *(unsigned __int64*)&ft;
}
#endif
#define FILETIME_64(ft)	MakeFileTime64(ft)


INLINE _Check_return_ DWORD ValidTickCount()
{
	WARNING_SUPPRESS(28159);
	const DWORD t = CPP_GLOBAL(GetTickCount());
	return t + (0 == t);
}


#if !defined(NOKEYSTATES) && !defined(MK_MENU)
#define MK_MENU		0x0200	// ALT key
#endif


#ifdef _WIN64
typedef POINTL		POINTM;
typedef RECT		RECTM;
#else // _WIN64
typedef POINTS		POINTM;
typedef SMALL_RECT	RECTM;
#endif // _WIN64
typedef POINTM*			PPOINTM;
typedef const POINTM*	PCPOINTM;
typedef RECTM*			PRECTM;
typedef const RECTM*	PCRECTM;


#ifdef DUMMYSTRUCTNAME

typedef union RECTF128 {
	struct {
		float left;
		float top;
		float right;
		float bottom;
	} DUMMYSTRUCTNAME;
	__m128 m128;

#ifdef __cplusplus

	operator __m128& () { return m128; }
	operator __m128 () const { return m128; }
	RECTF128& operator =(const RECTF128& rcf) { m128 = rcf.m128; return *this; }
	RECTF128& operator =(const RECT& rc) {
		left = (float)rc.left; top = (float)rc.top;
		right = (float)rc.left; bottom = (float)rc.bottom;
		return *this; }
	RECTF128& operator =(__m128 val) { m128 = val; return *this; }

	bool IsEmpty() const { return (left >= right || top >= bottom); }
	RECTF128& SetEmpty() { m128 = _mm_setzero_ps(); return *this; }

#endif // __cplusplus
} RECTF, *PRECTF;


typedef union RECTI128 {
	struct {
		__int32 left;
		__int32 top;
		__int32 right;
		__int32 bottom;
	} DUMMYSTRUCTNAME;
	__m128i m128;

#ifdef __cplusplus
	operator __m128i& () { return m128; }
	operator __m128i () const { return m128; }
	RECTI128& operator =(const RECTI128& rcf) { m128 = rcf.m128; return *this; }
	RECTI128& operator =(const RECT& rc) { m128 = *(UNALIGNED const __m128i*)&rc; return *this; }
	RECTI128& operator =(__m128i val) { m128 = val; return *this; }

	bool IsEmpty() const { return (left >= right || top >= bottom); }
	RECTI128& SetEmpty() { m128 = _mm_setzero_si128(); return *this; }
#endif // __cplusplus
} RECTI, *PRECTI;

#endif	// DUMMYSTRUCTNAME


#ifdef CALLBACK
typedef BOOL(CALLBACK* PARAM_CALLBACK_PROC)(void*);
#endif


// Misc. //////////////////////////////////////////////////////////////


INLINE ULARGE_INTEGER ToULargeInteger(ULONGLONG quadPart)
{
	return *((PULARGE_INTEGER)&quadPart);
}
INLINE LARGE_INTEGER ToLargeInteger(LONGLONG quadPart)
{
	return *((PLARGE_INTEGER)&quadPart);
}


INLINE unsigned DIBStride(unsigned long width, unsigned bpp)
{
	return ((width * bpp + 31u) & ~31u) >> 3u;
}
#ifdef __cplusplus
inline unsigned DIBStride(unsigned width, unsigned bpp)
{
	return ((width * bpp + 31u) & ~31u) >> 3u;
}
inline unsigned DIBStride(int width, unsigned bpp)
{
	return (((unsigned)labs(width) * bpp + 31u) & ~31u) >> 3u;
}
inline unsigned DIBStride(long width, unsigned bpp)
{
	return (((unsigned)labs(width) * bpp + 31u) & ~31u) >> 3u;
}
#endif	// __cplusplus
#define DIB_STRIDE(width, bpp)			DIBStride(width, (unsigned)(bpp))
#define DIB_WIDTHBYTES(width, bpp)		DIBStride(width, (unsigned)(bpp))

#ifndef RGBA
#define RGBA(r,g,b,a)	(RGB(r,g,b) | ((unsigned __int32)(unsigned char)(a) << 24))
#endif
#ifndef GetAValue
#define GetAValue(rgb)	((unsigned char)((unsigned __int32)(rgb) >> 24))
#endif



INLINE __m128i VECTORCALL mm_rgb_blend_si128(_In_ __m128i src32, _In_ __m128i dst32,
		_In_ __m128i epi16alpha, _In_ __m128i _zero, _In_ __m128i _pi16_1, _In_ __m128i _pi16_255)
{
	src32 = _mm_unpacklo_epi8(src32, _zero);
	dst32 = _mm_unpacklo_epi8(dst32, _zero);
	return _mm_packus_epi16(_mm_insert_epi16(_mm_srli_epi16(_mm_adds_epu16(_mm_mullo_epi16( _mm_add_epi16(epi16alpha, _pi16_1), src32),
			_mm_mullo_epi16(_mm_xor_si128(_pi16_255, epi16alpha), dst32)), 8), _mm_extract_epi16(dst32, 3), 3), dst32);
}

INLINE COLORREF ColorBlend50(COLORREF cr1, COLORREF cr2)
{
	return (COLORREF)_mm_cvtsi128_si32(_mm_avg_epu8(_mm_cvtsi32_si128((INT32)cr1), _mm_cvtsi32_si128((INT32)cr2)));
}


#ifdef _PROPVARIANTINIT_DEFINED_

#ifdef __cplusplus
inline bool PropVariantIsEmpty(const PROPVARIANT& pv)
{
	return ((unsigned short)pv.vt <= VT_NULL);	// VT_EMPTY or VT_NULL
}
#else
__inline BOOLEAN PropVariantIsEmpty(const PROPVARIANT* pv)
{
	return ((unsigned short)pv->vt <= VT_NULL);	// VT_EMPTY or VT_NULL
}
#endif
#define PropVarIsEmpty(pv)	PropVariantIsEmpty(pv)

#ifdef PropVariantInit
#undef PropVariantInit
#endif
#define PropVariantInit(pv)	ZeroStruct(pv)

#endif	// _PROPVARIANTINIT_DEFINED_

#define PropVarInit(pv)		ZeroStruct(pv)
#define VariantInit(pv)		ZeroStruct(pv)


// Safe release ///////////////////////////////////////////////////////


INLINE void SafeStrFree(wchar_t** pps)
{
	wchar_t* const ps = *pps;
	*pps = NULL;
	free(ps);
}
INLINE void SafeTaskStrFree(wchar_t** pps)
{
	wchar_t* const ps = *pps;
	*pps = NULL;
	CPP_GLOBAL(CoTaskMemFree(ps));
}
INLINE void SafeLocalStrFree(wchar_t** pps)
{
	wchar_t* const ps = *pps;
	*pps = NULL;
	CPP_GLOBAL(LocalFree(ps));
}

INLINE void SafeCloseHandle(HANDLE* ph)
{
	const HANDLE h = *ph;
	*ph = NULL;
	if (h) CPP_GLOBAL(CloseHandle(h));
}
INLINE void SafeCloseFileHandle(HANDLE* ph)
{
	const HANDLE h = *ph;
	*ph = NULL;
	if (h && INVALID_HANDLE_VALUE != h)
		CPP_GLOBAL(CloseHandle(*ph));
}

INLINE void SafeRegCloseKey(PHKEY phk)
{
	const HKEY hk = *phk;
	*phk = NULL;
	if (hk) CPP_GLOBAL(RegCloseKey(hk));
}

INLINE void SafeDeleteObject(_Inout_ HGDIOBJ* phGdiObj)
{
	const HGDIOBJ hobj = *phGdiObj;
	*phGdiObj = NULL;
	if (hobj) CPP_GLOBAL(DeleteObject(hobj));
}


INLINE ULONG SafeAddRef(_In_opt_ IUnknown* punk)
{
	return (punk ?
#if defined(__cplusplus) && !defined(CINTERFACE)
		punk->AddRef()
#else
		punk->lpVtbl->AddRef(punk)
#endif
		: 0);
}
INLINE ULONG GetUnkRefCount(_In_opt_ IUnknown* punk)
{
	if (punk)
	{
#if defined(__cplusplus) && !defined(CINTERFACE)
		punk->AddRef();
		return punk->Release();
#else
		punk->lpVtbl->AddRef(punk);
		return punk->lpVtbl->Release(punk);
#endif
	}
	return 0;
}


#ifdef __cplusplus	// ++++++++++++++++++++++++++++++++++++++++++++++++


template <class T> inline void SafeTaskMemFree(T** ppmem)
{
	T* const pm = *ppmem;
	*ppmem = nullptr;
	::CoTaskMemFree(pm);
}
template <class T> inline void SafeLocalFree(T** ppmem)
{
	T* const pm = *ppmem;
	*ppmem = nullptr;
	::LocalFree(pm);
}

template <class T> inline void SafeDeleteObject(_Inout_ T* phGdiObj)
{
	const HGDIOBJ hobj = static_cast<HGDIOBJ>(*phGdiObj);
	*phGdiObj = nullptr;
	if (hobj) ::DeleteObject(hobj);
}

template <class T> inline void SafeRelease(T** ppunk)
{
	T* const punk = *ppunk;
	*ppunk = nullptr;
	if (punk) punk->Release();
}
template <class V, class T> V* SafeAssign(V* val, T** targ)
{
	if (*targ)
		(*targ)->Release();
	*targ = val;
	return val;
}


#endif	// __cplusplus	+++++++++++++++++++++++++++++++++++++++++++++++


// GUIDs //////////////////////////////////////////////////////////////


#define DEFINE_IID_OF(iface)		const IID DECLALIGN16 IID_##iface = __uuidof(iface)

#define GUIDFromString(psz, pguid)	CPP_GLOBAL(IIDFromString(psz, pguid))
#define StringToGUID(psz, pguid)	CPP_GLOBAL(IIDFromString(psz, pguid))
_Success_(return > 0)
INLINE UINT StringFromGUID(_In_ REFGUID rguid, _Out_writes_to_(cchMax, return-1) PWSTR szGuid, _In_ int cchMax)
{
	const int cch = CPP_GLOBAL(StringFromGUID2(rguid, szGuid, cchMax));
	return (cch > 0) ? (UINT)(cch - 1) : 0;
}

#define SetGUIDNull(pguid)			Zero16Bytes(pguid)

#ifdef __cplusplus	// ++++++++++++++++++++++++++++++++++++++++++++++++


#ifndef _SYS_GUID_OPERATOR_EQ_
#define _SYS_GUID_OPERATOR_EQ_

inline bool operator==(const GUID& guidOne, const GUID& guidOther)
{
    return ::InlineIsEqualGUID(guidOne, guidOther);
}

inline bool operator!=(const GUID& guidOne, const GUID& guidOther)
{
    return !::InlineIsEqualGUID(guidOne, guidOther);
}

#endif  // _SYS_GUID_OPERATOR_EQ_

inline void CopyGUID(GUID* dest, const GUID& src)
{
	Copy16Bytes(dest, &src);
}
inline void CopyGUID(GUID* dest, const GUID* src)
{
	Copy16Bytes(dest, src);
}

#else	// __cplusplus	+++++++++++++++++++++++++++++++++++++++++++++++

__inline void CopyGUID(GUID* dest, const GUID* src)
{
	Copy16Bytes(dest, src);
}

#endif	// __cplusplus	+++++++++++++++++++++++++++++++++++++++++++++++


#ifdef IsEqualGUID
#undef IsEqualGUID
#endif
#define IsEqualGUID(guid1, guid2)		CPP_GLOBAL(InlineIsEqualGUID(guid1, guid2))

#ifdef IsEqualIID
#undef IsEqualIID
#endif
#define IsEqualIID(iid1, iid2)			CPP_GLOBAL(InlineIsEqualGUID(iid1, iid2))

#ifdef IsEqualCLSID
#undef IsEqualCLSID
#endif
#define IsEqualCLSID(clsid1, clsid2)	CPP_GLOBAL(InlineIsEqualGUID(clsid1, clsid2))

#define CopyGuid(dst, src)	CopyGUID(dst, src)

INLINE bool_t __fastcall IsNullGUID(_In_opt_ const GUID* pGuid)
{
	return (!pGuid || Is16BytesZero(pGuid));
}
#ifdef __cplusplus
inline bool __fastcall IsNullGUID(_In_ const GUID& rGuid)
{
	return Is16BytesZero(&rGuid);
}
#endif


#ifdef __cplusplus	// ++++++++++++++++++++++++++++++++++++++++++++++++

// Type converters & operators ////////////////////////////////////////

inline __m128 mm_cvtrgba_clrf(COLORREF cr)
{
	return _mm_mul_ps(CPP_COMMONS(mm_cvtpu8_ps(_mm_cvtsi32_si128(cr))), CPP_COMMONS(mm_ps_1d255()));
}
#define COLORREFtoM128(cr)	mm_cvtrgba_clrf((unsigned __int32)(cr))

inline __m128 mm_cvtbgra_clrf(DWORD bgra)
{
	return _mm_mul_ps(_mm_cvtepi32_ps(mm_swap02_epi32(CPP_COMMONS(mm_cvtpu8_epi32(bgra)))), CPP_COMMONS(mm_ps_1d255()));
}

inline COLORREF VECTORCALL mm_cvtclrf_rgba(__m128 clrf)
{
	return mm_cvtps_pi8(_mm_mul_ps(clrf, CPP_COMMONS(mm_ps_255())));
}
#define M128toCOLORREF(clrf)	mm_cvtclrf_rgba(clrf)


inline UINT64 ToUInt64(POINT pt)
{
	return *reinterpret_cast<PUINT64>(&pt);
}
inline UINT64 ToUInt64(POINTL pt)
{
	return *reinterpret_cast<PUINT64>(&pt);
}
inline UINT64 ToUInt64(SIZE size)
{
	return *reinterpret_cast<PUINT64>(&size);
}
inline UINT64 __fastcall ToUInt64(const RECT& rc)
{
	return MAKEULONGLONGW(UInt32To16US(rc.left), UInt32To16US(rc.top), UInt32To16US(rc.right), UInt32To16US(rc.bottom));
}
inline UINT64 __fastcall ToUInt64(const RECTL& rc)
{
	return MAKEULONGLONGW(UInt32To16US(rc.left), UInt32To16US(rc.top), UInt32To16US(rc.right), UInt32To16US(rc.bottom));
}

inline void __fastcall ToNonNeg(_Inout_ PPOINT ppt)
{
	ToNonNeg32x2(ppt);
}
inline POINT __fastcall ToNonNeg(_In_ POINT pt)
{
	ToNonNeg32x2(&pt);
	return pt;
}
inline void __fastcall ToNonNeg(_Inout_ PPOINTL ppt)
{
	ToNonNeg32x2(ppt);
}
inline POINTL __fastcall ToNonNeg(_In_ POINTL pt)
{
	ToNonNeg32x2(&pt);
	return pt;
}

inline void __fastcall ToNonNeg(_Inout_ PSIZE psize)
{
	ToNonNeg32x2(psize);
}
inline SIZE __fastcall ToNonNeg(_In_ SIZE size)
{
	ToNonNeg32x2(&size);
	return size;
}

inline void __fastcall ToNonNeg(_Inout_ PRECT prc)
{
	ToNonNeg32x4(prc);
}
inline void __fastcall ToNonNeg(_Inout_ PRECTL prc)
{
	ToNonNeg32x4(prc);
}


inline bool IsSizeEmpty(_In_ SIZE size)
{
	return (size.cx <= 0) | (size.cy <= 0);
}

template <typename _TRect>
inline bool IsRectEmpty(_In_ const _TRect& rc)
{
	return (rc.right <= rc.left) | (rc.bottom <= rc.top);
}
inline bool IsRectEmpty(_In_ const RECT& rc)
{
	return CPP_COMMONS(IsRectEmpty<RECT>(rc));
}
inline bool IsRectEmpty(_In_ const RECTL& rc)
{
	return CPP_COMMONS(IsRectEmpty<RECTL>(rc));
}
inline void EmptyRect(_Out_ RECT* prc)
{
	Zero16Bytes(prc);
}
inline void EmptyRect(_Out_ RECTL* prc)
{
	Zero16Bytes(prc);
}
#ifndef SetRectEmpty
#define SetRectEmpty(prc)	EmptyRect(prc)
#endif

inline void CopyRect(_Out_ PRECT prcDst, _In_ const RECT& rcSrc)
{
	Copy16Bytes(prcDst, &rcSrc);
}

inline LONG RectWidth(_In_ const RECT& rc)
{
	return (rc.right - rc.left);
}
inline LONG RectWidth(_In_ const RECTL& rc)
{
	return (rc.right - rc.left);
}
inline ULONG RectWidthU(_In_ const RECT& rc)
{
	return ToNonNeg32(rc.right - rc.left);
}
inline ULONG RectWidthU(_In_ const RECTL& rc)
{
	return ToNonNeg32(rc.right - rc.left);
}
inline LONG RectHeight(_In_ const RECT& rc)
{
	return (rc.bottom - rc.top);
}
inline LONG RectHeight(_In_ const RECTL& rc)
{
	return (rc.bottom - rc.top);
}
inline ULONG RectHeightU(_In_ const RECT& rc)
{
	return ToNonNeg32(rc.bottom - rc.top);
}
inline ULONG RectHeightU(_In_ const RECTL& rc)
{
	return ToNonNeg32(rc.bottom - rc.top);
}
inline SIZE RectSize(_In_ const RECT& rc)
{
	return SIZE{ CPP_COMMONS(RectWidth(rc)), CPP_COMMONS(RectHeight(rc)) };
}

template <typename _TRect, typename _TPt>
inline bool PtInRect(_In_ const _TRect& rc, _In_ _TPt pt)
{
	return (pt.x >= rc.left) & (pt.x < rc.right) & (pt.y >= rc.top) & (pt.y < rc.bottom);
}
template <typename _TBox, typename _TPt>
inline bool PtInBox(_In_ const _TBox& box, _In_ _TPt pt)
{
	return (pt.x >= box.left) & (pt.x < box.left + box.width) & (pt.y >= box.top) & (pt.y < box.top + box.height);
}

template <class _TRect>
inline bool IsRectEnclosed(_In_ const _TRect& rcInner, _In_ const _TRect& rcOuter)
{
	return (rcInner.left >= rcOuter.left) & (rcInner.top >= rcOuter.top) &
		(rcInner.right <= rcOuter.right) & (rcInner.bottom <= rcOuter.bottom);
}
template <class _TRect>
inline void __fastcall EncloseRect(_Inout_ _TRect* prcInner, _In_ const _TRect& rcOuter)
{
	if (prcInner->left < rcOuter.left)
		prcInner->left = rcOuter.left;
	if (prcInner->top < rcOuter.top)
		prcInner->top = rcOuter.top;
	if (prcInner->right > rcOuter.right)
		prcInner->right = rcOuter.right;
	if (prcInner->bottom > rcOuter.bottom)
		prcInner->bottom = rcOuter.bottom;
}

template <class TR1, class TR2>
inline bool __fastcall RectOverlap2(_In_ const TR1& rc1, _In_ const TR2& rc2)
{
	return ((rc1.left < rc2.right) & (rc1.right > rc2.left)
		& (rc1.top < rc2.bottom) & (rc1.bottom > rc2.top));
}
template <class TRect>
inline bool __fastcall RectOverlap(_In_ const TRect& rc1, _In_ const TRect& rc2)
{
	return RectOverlap2<TRect, TRect>(rc1, rc2);
}
template <class TRect>
inline bool __fastcall RectIntersect(_In_ const TRect* prc1, _In_ const TRect* prc2)
{
	return RectOverlap(static_cast<const RECT*>(prc1), static_cast<const RECT*>(prc2));
}

template<class TRect>
inline bool __fastcall IntersectRect_(_Out_ TRect* dest, _In_ const TRect& rc1, _In_ const TRect& rc2)
{
	dest->left = Max(rc1.left, rc2.left);
	dest->right = Min(rc1.right, rc2.right);
	dest->top = Max(rc1.top, rc2.top);
	dest->bottom = Min(rc1.bottom, rc2.bottom);
	if ((dest->right > dest->left) & (dest->bottom > dest->top))
		return true;
	if (sizeof(*dest) != 16)
	{
		dest->left = 0;
		dest->top = 0;
		dest->right = 0;
		dest->bottom = 0;
	}
	else Zero16Bytes(dest);
	return false;
 }


#if !defined(__d3d11_h__) || defined(D3D11_NO_HELPERS)
inline bool operator ==(const RECT& a, const RECT& b)
{
	return IsMem16Equal(&a, &b);
}
inline bool operator !=(const RECT& a, const RECT& b)
{
	return !IsMem16Equal(&a, &b);
}
#endif

inline bool operator ==(const RECTL& a, const RECTL& b)
{
	return IsMem16Equal(&a, &b);
}
inline bool operator !=(const RECTL& a, const RECTL& b)
{
	return !IsMem16Equal(&a, &b);
}

inline bool operator ==(const POINT a, const POINT b)
{
	return *(PUINT64)&a == *(PUINT64)&b;
}
inline bool operator !=(const POINT a, const POINT b)
{
	return *(PUINT64)&a != *(PUINT64)&b;
}
inline bool operator ==(const POINTL a, const POINTL b)
{
	return *(PUINT64)&a == *(PUINT64)&b;
}
inline bool operator !=(const POINTL a, const POINTL b)
{
	return *(PUINT64)&a != *(PUINT64)&b;
}

inline bool operator ==(const SIZE a, const SIZE b)
{
	return *(PUINT64)&a == *(PUINT64)&b;
}
inline bool operator !=(const SIZE a, const SIZE b)
{
	return *(PUINT64)&a != *(PUINT64)&b;
}
inline bool operator <(const SIZE a, const SIZE b)
{
	return (a.cx < b.cx) | (a.cy < b.cy);
}
inline bool operator <=(const SIZE a, const SIZE b)
{
	return (a.cx <= b.cx) | (a.cy <= b.cy);
}
inline bool operator >(const SIZE a, const SIZE b)
{
	return ((a.cx >= b.cx) & (a.cy > b.cy)) | ((a.cx > b.cx) & (a.cy >= b.cy));
}
inline bool operator >=(const SIZE a, const SIZE b)
{
	return (a.cx >= b.cx) & (a.cy >= b.cy);
}


// Helper wrappers ////////////////////////////////////////////////////


inline HRESULT Stream_Size(_In_ IStream* pstm, _Out_ PULONGLONG pcb)
{
	STATSTG stat;
	stat.cbSize.QuadPart = 0;
	const HRESULT hr = pstm->Stat(&stat, STATFLAG_NONAME);
	*pcb = stat.cbSize.QuadPart;
	return hr;
}
inline UINT64 Stream_Size(_In_ IStream* pstm)
{
	STATSTG stat;
	stat.cbSize.QuadPart = 0;
	return (S_OK == pstm->Stat(&stat, STATFLAG_NONAME)) ? stat.cbSize.QuadPart : 0;
}

inline HRESULT Stream_Position(_In_ IStream* pstm, _Out_ PULONGLONG pbpos)
{
	*pbpos = 0;
	return pstm->Seek({ 0 }, STREAM_SEEK_CUR, (PULARGE_INTEGER)pbpos);
}

inline HRESULT Stream_SeekStart(_In_ IStream* pstm)
{
	return pstm->Seek({ 0 }, STREAM_SEEK_SET, nullptr);
}


#endif	// __cplusplus	+++++++++++++++++++++++++++++++++++++++++++++++

