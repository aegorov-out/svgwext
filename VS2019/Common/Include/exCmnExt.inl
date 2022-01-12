/*	exCmnExt.inl	- common convenience extensions

	Copyright (c) Alexandre Egorov

	This software is provided 'as-is', without any warranty.
	This software can be used and modified freely for any purpose,
	including commercial applications,
	as long as its origin is not misrepresented.
 */
#pragma once
#define _CMN_EXTENSIONS_INCLUDED_


INLINE bool_t __fastcall IsAsciiDigitA(char ch) {
	return (ch >= '0') & (ch <= '9');
}
#define ISADIGIT(ch)	IsAsciiDigitA((char)(ch))

INLINE bool_t __fastcall IsAsciiDigitW(wchar_t wc) {
	return (wc >= L'0') & (wc <= L'9');
}
#define ISWDIGIT(ch)	IsAsciiDigitW((wchar_t)(ch))

INLINE bool_t __fastcall IsAsciiAlphaA(char ch) {
	ch = TOUPPER_(ch);
	return (ch >= 'A') & (ch <= 'Z');
}
#define ISAALPHA(ch)	IsAsciiAlphaA((char)(ch))

INLINE bool_t __fastcall IsAsciiAlphaW(wchar_t wc) {
	wc = TOUPPER_(wc);
	return (wc >= L'A') & (wc <= L'Z');
}
#define ISWALPHA(ch)	IsAsciiAlphaW((wchar_t)(ch))

#ifdef _UNICODE
#define IsAsciiDigit(wc)	IsAsciiDigitW(wc)
#define IsAsciiAlpha(wc)	IsAsciiAlphaW(wc)
#else
#define IsAsciiDigit(ch)	IsAsciiDigitA(ch)
#define IsAsciiAlpha(ch)	IsAsciiAlphaA(ch)
#endif	// _UNICODE
#define ISDIGIT(ch)			IsAsciiDigit(ch)
#define ISALPHA(ch)			IsAsciiAlpha(ch)


INLINE bool_t __fastcall StringIsValidA(_In_opt_ const char* s) { return (s && s[0]); }
INLINE bool_t __fastcall StringIsValidW(_In_opt_ const wchar_t* s) { return (s && s[0]); }
#ifdef __cplusplus
template<typename _CT> inline bool __fastcall StringIsValid(_In_opt_ const _CT* s) { return (s && s[0]); }
#elif defined(UNICODE)
#define StringIsValid	StringIsValidW
#else
#define StringIsValid	StringIsValidA
#endif

INLINE bool_t __fastcall StringIsEmptyA(_In_opt_ const char* s) { return !(s && s[0]); }
INLINE bool_t __fastcall StringIsEmptyW(_In_opt_ const wchar_t* s) { return !(s && s[0]); }
#ifdef __cplusplus
template<typename _CT> inline bool __fastcall StringIsEmpty(_In_opt_ const _CT* s) { return !(s && s[0]); }
#elif defined(UNICODE)
#define StringIsEmpty	StringIsEmptyW
#else
#define StringIsEmpty	StringIsEmptyA
#endif


INLINE void __fastcall uswap32(__inout unsigned __int32* pdw1, __inout unsigned __int32* pdw2)
{
	*pdw1 ^= *pdw2;
	*pdw2 ^= *pdw1;
	*pdw1 ^= *pdw2;
}
#define uswap(pi1, pi2)	uswap32((unsigned __int32*)(pi1), (unsigned __int32*)(pi2))

INLINE void __fastcall uswap64(__inout unsigned __int64* pqw1, __inout unsigned __int64* pqw2)
{
	*pqw1 ^= *pqw2;
	*pqw2 ^= *pqw1;
	*pqw1 ^= *pqw2;
}


#ifdef __cplusplus
template<typename _PT> inline _PT AddBytes(_PT ptr, ptrdiff_t cb)
{
	return (_PT)((unsigned char*)ptr + cb);
}
template<typename _PT> inline _PT AddBytesU(_PT ptr, size_t cb)
{
	return (_PT)((unsigned char*)ptr + cb);
}
#else
__inline void* AddBytes(const void* ptr, intptr_t cb)
{
	return (void*)((unsigned char*)ptr + cb);
}
__inline void* AddBytesU(const void* ptr, size_t cb)
{
	return (void*)((unsigned char*)ptr + cb);
}
#endif
#define ADD_BYTES(ptr, cb)		AddBytes(ptr, (ptrdiff_t)(cb))
#define ADD_BYTES_U(ptr, cb)	AddBytesU(ptr, (size_t)(cb))


// Memory helpers /////////////////////////////////////////////////////


#pragma intrinsic(_byteswap_ushort, _byteswap_ulong, _byteswap_uint64)
#define _bswap16(u16)	_byteswap_ushort((unsigned short)(u16))
#define _bswap32(u32)	_byteswap_ulong((unsigned long)(u32))
#define _bswap(u32)		_byteswap_ulong((unsigned long)(u32))
#define _bswap64(u64)	_byteswap_uint64((unsigned long long)(u64))
#ifdef _WIN64
#define _bswapp(ptr)	_byteswap_uint64((unsigned long long)(ptr))
#else
#define _bswapp(ptr)	_byteswap_ulong((unsigned long)(ptr))
#endif


#define _movsb(dst, src, cb)	__movsb((unsigned char*)(dst), (const unsigned char*)(src), (size_t)(cb))
#define _movsw(dst, src, cw)	__movsw((unsigned short*)(dst), (const unsigned short*)(src), (size_t)(cw))
#define _movsd(dst, src, cd)	__movsd((unsigned long*)(dst), (const unsigned long*)(src), (size_t)(cd))

#define _stosb(dst, b, cb)		__stosb((unsigned char*)(dst), (unsigned char)(b), (size_t)(cb))
#define _stosw(dst, w, cw)		__stosw((unsigned short*)(dst), (unsigned short)(w), (size_t)(cw))
#define _stosd(dst, d, cd)		__stosd((unsigned long*)(dst), (unsigned long)(d), (size_t)(cd))
#define _zerob(dst, cb)			__stosb((unsigned char*)(dst), 0, (size_t)(cb))
#define _zerow(dst, cw)			__stosw((unsigned short*)(dst), 0, (size_t)(cw))
#define _zerod(dst, cd)			__stosd((unsigned long*)(dst), 0, (size_t)(cd))

INLINE void* __fastcall dqmemcpy(void* dst, const void* src, size_t cdq)
{
	for (; cdq; cdq--)
	{
		_mm_storeu_si128((__m128i*)dst, _mm_loadu_si128((const __m128i*)src));
		src = (const void*)((const char*)src + 16);
		dst = (void*)((char*)dst + 16);
	}
	return dst;
}
#define _movsdq(dst, src, cdq)		dqmemcpy(dst, src, cdq)

INLINE void* __fastcall dqmemzero(void* dst, size_t cdq)
{
	const __m128i mz = _mm_setzero_si128();
	for (; cdq; cdq--)
	{
		_mm_storeu_si128((__m128i*)dst, mz);
		dst = (void*)((char*)dst + 16);
	}
	return dst;
}
#define zerodq(dst, cdq)			dqmemzero((void*)(dst), (void*)(cdq))


#define Zero8Bytes(pqw)				*((UNALIGNED __int64*)(pqw)) = 0ll
#define Copy8Bytes(dst, src)		*((UNALIGNED __int64*)(dst)) = *((UNALIGNED __int64*)(src))
#define Zero16Bytes(dst)			_mm_storeu_si128((__m128i*)(dst), _mm_setzero_si128())
#define Copy16Bytes(dst, src)		_mm_storeu_si128((__m128i*)(dst), _mm_loadu_si128((__m128i*)(src)))

#define ZeroMemDQ(ptr, cdq)			((1 != (cdq)) ? (void)CPP_COMMONS(dqmemzero((void*)(ptr), (size_t)(cdq))) : Zero16Bytes(ptr))
#define CopyMemDQ(dst, src, cdq)	((1 != (cdq)) ? (void)CPP_COMMONS(dqmemcpy((void*)(dst), (const void*)(src), (size_t)(cdq))) : Copy16Bytes(dst, src))


#ifdef _WIN64

#define _movsq(dst, src, cq)	__movsq((unsigned long long*)(dst), (const unsigned long long*)(src), (size_t)(cq))
#define _movsp	_movsq
#define _movspb(dst, src, cb)	__movsq((unsigned long long*)(dst), (const unsigned long long*)(src), (size_t)(cb) / 8)
#define _zeroq(dst, cq)			__stosq((unsigned long long*)(dst), 0, (size_t)(cq))
#define _zerop(dst, cp)			__stosq((unsigned long long*)(dst), 0, (size_t)(cp))
#define _zeroqb(dst, cb)		__stosq((unsigned long long*)(dst), 0, (size_t)(cb) / 8)


INLINE void __fastcall dmemcpy(void* dst, const void* src, size_t cd)
{
	if (!(((unsigned char)cd | (unsigned char)(size_t)src | (unsigned char)(size_t)dst) & 7))
		_movsq(dst, src, cd / 2);
	else
		_movsd(dst, src, cd);
}
INLINE void __fastcall dmemzero(void* dst, size_t cd)
{
	if (!(((unsigned char)cd | (unsigned char)(size_t)dst) & 7))
		_zeroq(dst, cd / 2);
	else
		_zerod(dst, cd);
}

#define ZeroMemBlock(ptr, cb)	(((char)(cb) & 15) ? (((char)(cb) & 7) ?		\
								__stosd((unsigned long*)(ptr), 0, (cb) / 4) :		\
								__stosq((unsigned long long*)(ptr), 0, (cb) / 8)) :	\
								ZeroMemDQ(ptr, cb / 16))
#define ZeroMemBlockP(ptr, cb)	ZeroMemBlock(ptr, cb)

#define CopyMemBlock(dst, src, cb)	(((char)(cb) & 15) ? (((char)(cb) & 7) ?	\
								__movsd((unsigned long*)(dst), (unsigned long*)(src), (cb) / 4) :		\
								__movsq((unsigned long long*)(dst), (unsigned long long*)(src), (cb) / 8)) :	\
								CopyMemDQ(dst, src, cb / 16))

#else	// _WIN64

#define _movsq(dst, src, cq)	__movsd((unsigned long*)(dst), (const unsigned long *)(src), (size_t)(cq) * 2)
#define _movsp	_movsd
#define _movspb(dst, src, cb)	__movsd((unsigned long*)(dst), (const unsigned long *)(src), (size_t)(cb) / 4)
#define _zeroq(dst, cq)			__stosd((unsigned long*)(dst), 0, (size_t)(cq) * 2)
#define _zerop(dst, cp)			__stosd((unsigned long*)(dst), 0, (size_t)(cp))
#define _zeroqb(dst, cb)		__stosd((unsigned long*)(dst), 0, (size_t)(cb) / 4)

#define dmemcpy(dst, src, cd)	__movsd((unsigned long*)(dst), (const unsigned long *)(src), (size_t)(cd))
#define dmemzero(dst, cd)		__stosd((unsigned long*)(dst), (unsigned long)(d), (size_t)(cd))

#define ZeroMemBlock(ptr, cb)	(((char)(cb) & 15) ? __stosd((unsigned long*)ptr, 0, (cb) / 4) : ZeroMemDQ(ptr, cb / 16))
#define ZeroMemBlockP(ptr, cb)	ZeroMemBlock(ptr, cb)

#define CopyMemBlock(dst, src, cb)	(((char)(cb) & 15) ? __movsd((unsigned long*)dst, (unsigned long*)src, (cb) / 4) : CopyMemDQ(dst, src, cb / 16))

#endif	//_WIN64

#ifdef UNICODE
#define _movst	_movsw
#else
#define _movst	_movsb
#endif

#ifndef _wmemcpy
#define _wmemcpy(dst, src, cw)	_movsw(dst, src, cw)
#endif
#if defined(_VC_NODEFAULTLIB) && !defined(wmemcpy)
#define wmemcpy(dst, src, cw)	_wmemcpy(dst, src, cw)
#endif


#define ZeroStruct(ptr)			ZeroMemBlock(ptr, sizeof(*ptr))
#define ZeroStructP(ptr)		ZeroMemBlockP(ptr, sizeof(*ptr))
#define CopyStruct(dst, src)	CopyMemBlock(dst, src, sizeof(*src))

#define ZeroStructFrom(obj, member)			ZeroMemBlock(&((obj)->member), SIZEOF_OBJECT_FROM(obj, member))
#define ZeroStructFromP(obj, member)		ZeroMemBlockP(&((obj)->member), SIZEOF_OBJECT_FROM(obj, member))
#define ZeroStructAfter(obj, member)		ZeroMemBlock(((volatile char*)&((obj)->member)) + sizeof((obj)->member), SIZEOF_OBJECT_AFTER(obj, member))
// Includes both members
#define ZeroStructRange(obj, mfirst, mlast)	ZeroMemBlock(&((obj)->mfirst), SIZEOF_OBJECT_RANGE(obj, mfirst, mlast))
// Excludes the last member
#define ZeroStructRangeTo(obj, mfirst, mlast)	ZeroMemBlock(&((obj)->mfirst), SIZEOF_OBJECT_RANGE_TO(obj, mfirst, mlast))


#ifdef __cplusplus
template <class CT>
inline CT* __fastcall xmemchr(const CT* px, CT x, size_t cx)
{
	for (; cx; --cx)
	{
		if (*((CT*)px) != x)
		{
			px = ((CT*)px) + 1;
			continue;
		}
		return const_cast<CT*>(px);
	}
	return nullptr;
}
inline void* __fastcall dmemchr(const void* pd, unsigned __int32 d, size_t cd)
{
	return reinterpret_cast<void*>(CPP_COMMONS(xmemchr<unsigned __int32>(reinterpret_cast<const unsigned __int32*>(pd), d, cd)));
}
#else
__inline void* __fastcall dmemchr(const void* pd, unsigned __int32 d, size_t cd)
{
	for (; cd; --cd)
	{
		if (*((unsigned __int32*)pd) != d)
		{
			pd = ((unsigned __int32*)pd) + 1;
			continue;
		}
		return (void*)pd;
	}
	return NULL;
}
#endif


#ifdef _malloc_z
#define malloc_z(cb)	_malloc_z((size_t)(cb))
#else
#define malloc_z(cb)	calloc((size_t)(cb), 1)
#endif
#ifndef heapmin
#define heapmin()		(void)_heapmin()
#endif

#ifndef onstack_
#define onstack_(name)	(name*)_alloca(sizeof(name))
#endif


///////////////////////////////////////////////////////////////////////


INLINE int IMin(int a, int b)
{
	return MIN_(a, b);
}
INLINE int IMax(int a, int b)
{
	return MAX_(a, b);
}
INLINE int IMinMax(int val, int lo, int hi)
{
	return MAX_(MIN_(val, hi), lo);
}

INLINE unsigned UMin(unsigned a, unsigned b)
{
	return MIN_(a, b);
}
INLINE unsigned UMax(unsigned a, unsigned b)
{
	return MAX_(a, b);
}
INLINE unsigned UMinMax(unsigned val, unsigned lo, unsigned hi)
{
	return MAX_(MIN_(val, hi), lo);
}

INLINE unsigned __int64 UMin64(unsigned __int64 a, unsigned __int64 b)
{
	return MIN_(a, b);
}
INLINE unsigned __int64 UMax64(unsigned __int64 a, unsigned __int64 b)
{
	return MAX_(a, b);
}
INLINE unsigned __int64 UMinMax64(unsigned __int64 val, unsigned __int64 lo, unsigned __int64 hi)
{
	return MAX_(MIN_(val, hi), lo);
}

INLINE float FMin(float a, float b)
{
	return MIN_(a, b);
}
INLINE float FMax(float a, float b)
{
	return MAX_(a, b);
}
INLINE float FMinMax(float val, float lo, float hi)
{
	return MAX_(MIN_(val, hi), lo);
}

INLINE double DMin(double a, double b)
{
	return MIN_(a, b);
}
INLINE double DMax(double a, double b)
{
	return MAX_(a, b);
}
INLINE double DMinMax(double val, double lo, double hi)
{
	return MAX_(MIN_(val, hi), lo);
}

#ifdef __cplusplus

template <typename _T>
INLINE _T Min(_T a, _T b)
{
	return MIN_(a, b);
}
template <typename _T>
INLINE _T Max(_T a, _T b)
{
	return MAX_(a, b);
}
#ifdef min
#undef min
#endif
#define min(a,b)	Min(a,b)
#ifdef max
#undef max
#endif
#define max(a,b)	Max(a,b)

template <typename _T>
INLINE _T Min3(_T a, _T b, _T c)
{
	return MIN_(MIN_(a, b), c);
}
template <typename _T>
INLINE _T Max3(_T a, _T b, _T c)
{
	return MAX_(MAX_(a, b), c);
}
#define min3_(a, b, c)	Min3(a, b, c)
#define max3_(a, b, c)	Max3(a, b, c)

template <typename _T>
INLINE _T MinMax(_T val, _T lo, _T hi)
{
	return MAX_(MIN_(val, hi), lo);
}

#else	// __cplusplus

#define min3_(a, b, c)	min(min(a, b), c)
#define max3_(a, b, c)	max(max(a, b), c)

#endif	// __cplusplus


INLINE __int64 ftoi64(double x)
{
#ifdef _M_IX86
	__int64 i;
	__asm {
		fld	x
		fistp i
	}
	return i;
#else
	return _mm_cvtsd_si64(_mm_set_sd(x));
#endif
}
#define roundi64(d)	ftoi64((double)(d))
#define roundu64(d)	(unsigned __int64)ftoi64((double)(d))


INLINE __int32 ftoi32(double x)
{
	return _mm_cvtsd_si32(_mm_set_sd(x));
}
#define ftoui32(d)	(unsigned)ftoi32(d)

INLINE __int32 ftoi32f(float x)
{
	return _mm_cvt_ss2si(_mm_set_ss(x));
}
#define ftoui32f(d)	(unsigned)ftoi32f(d)
#define roundi32(x)	ftoi32f((float)(x))
#define roundu32(x)	(unsigned)ftoi32f((float)(x))
#define roundif(x)	roundi32(x)
#define rounduf(x)	roundu32(x)

#define rndeq64(dbl, i64)	(ftoi64((double)(dbl)) == (__int64)(i64))
#define rndeq32(flt, i32)	(ftoi32f((float)(flt)) == (__int32)(i32))
#ifdef _VC_NODEFAULTLIB
#ifndef round
#define round(d)	mm_round(d)
#endif
#ifndef roundf
#define roundf(f)	mm_roundf(f)
#endif
#endif

#ifdef _M_IX86
#define DblToInt32(x)	_mm_cvttsd_si32(_mm_set_sd(x))
#define FltToInt32(x)	_mm_cvtt_ss2si(_mm_set_ss(x))
#else
#define DblToInt32(x)	((__int32)(x))
#define FltToInt32(x)	((__int32)(x))
#endif


INLINE unsigned __int16 UInt32To16US(unsigned __int32 n)
{
	return (unsigned __int16)(n | ((n <= 0xFFFF) - 1));
}

INLINE unsigned __int32 ToNonNeg32(__int32 num)
{
	return (unsigned __int32)(num & ((__int32)(num < 0) - 1));
}
INLINE unsigned __int64 ToNonNeg64(__int64 num)
{
	return (unsigned __int64)(num & ((__int64)(num < 0) - 1ll));
}

INLINE void mmToNonNeg32x2(_Inout_updates_(2) __int32* rgi)
{
	const __m128i xval = _mm_loadl_epi64((__m128i*)rgi);
	_mm_storel_epi64((__m128i*)rgi, _mm_and_si128(xval, _mm_cmpgt_epi32(xval, _mm_setzero_si128())));
}
#define ToNonNeg32x2(rgi)	CPP_COMMONS(mmToNonNeg32x2((__int32*)(rgi)))

INLINE void mmToNonNeg32x4(_Inout_updates_(4) __int32* rgi)
{
	const __m128i xval = _mm_loadu_si128((__m128i*)rgi);
	_mm_storeu_si128((__m128i*)rgi, _mm_and_si128(xval, _mm_cmpgt_epi32(xval, _mm_setzero_si128())));
}
#define ToNonNeg32x4(rgi)	CPP_COMMONS(mmToNonNeg32x4((__int32*)(rgi)))

INLINE void mmToNonNeg32x2f(_Inout_updates_(2) float* rgf)
{
	const __m128 xnil = _mm_setzero_ps();
	const __m128 xval = _mm_loadl_pi(xnil, (__m64 const*)rgf);
	_mm_storel_pi((__m64*)rgf, _mm_and_ps(xval, _mm_cmpgt_ps(xval, xnil)));
}
#define ToNonNeg32x2F(rgf)	CPP_COMMONS(mmToNonNeg32x2f((float*)(rgf)))

INLINE void mmToNonNeg32x4f(_Inout_updates_(4) float* rgf)
{
	const __m128 xval = _mm_loadu_ps(rgf);
	_mm_storeu_ps(rgf, _mm_and_ps(xval, _mm_cmpgt_ps(xval, _mm_setzero_ps())));
}
#define ToNonNeg32x4F(rgf)	CPP_COMMONS(mmToNonNeg32x4f((float*)(rgf)))

INLINE float mmToNonNeg32f(_In_ float flt)
{
	const __m128 xval = _mm_load_ss(&flt);
	_mm_cvtss_f32(_mm_and_ps(xval, _mm_cmpgt_ps(xval, _mm_setzero_ps())));
}
#define ToNonNegF(flt)	CPP_COMMONS(mmToNonNeg32f((float)(flt)))

INLINE double mmToNonNeg64f(_In_ double dbl)
{
	const __m128d xval = _mm_load_sd(&dbl);
	_mm_cvtsd_f64(_mm_and_pd(xval, _mm_cmpgt_pd(xval, _mm_setzero_pd())));
}
#define ToNonNegD(dbl)	CPP_COMMONS(mmToNonNeg64f((double)(dbl)))


#ifdef __cplusplus	// +++++++++++++++++++++++++++++++++++++++++++++++++

inline unsigned __int32 ToNonNeg(_In_ __int32 num)
{
	return CPP_COMMONS(ToNonNeg32(num));
}
inline unsigned __int64 ToNonNeg(_In_ __int64 num)
{
	return CPP_COMMONS(ToNonNeg64(num));
}
inline float ToNonNeg(_In_ float num)
{
	return ToNonNegF(num);
}
inline double ToNonNeg(_In_ double num)
{
	return ToNonNegD(num);
}

#endif	// __cplusplus +++++++++++++++++++++++++++++++++++++++++++++++++


INLINE unsigned __int32 UAlign32(unsigned __int32 num, unsigned __int32 align)
{
	align += (0 == align);
	num += align - 1;
	num += (0 == num);
	return num - num % align;
}
INLINE unsigned __int64 UAlign64(unsigned __int64 num, unsigned __int64 align)
{
	align += (0 == align);
	num += align - 1;
	num += (0 == num);
	return num - num % align;
}
#ifdef _WIN64
#define UAlignPtr(num, align) CPP_COMMONS(UAlign64((unsigned __int64)(num), (unsigned __int64)(align)))
#else
#define UAlignPtr(num, align) CPP_COMMONS(UAlign32((unsigned __int32)(num), (unsigned __int32)(align)))
#endif

INLINE float FAlign(float num, float align)
{
	return align ? (align * ceilf(num / align)) : num;
}
INLINE double DAlign(double num, double align)
{
	return align ? (align * ceil(num / align)) : num;
}

#ifdef __cplusplus	// +++++++++++++++++++++++++++++++++++++++++++++++++

inline unsigned __int32 Align(unsigned __int32 num, unsigned __int32 align)
{
	return CPP_COMMONS(UAlign32(num, align));
}
inline unsigned __int64 Align(unsigned __int64 num, unsigned __int64 align)
{
	return CPP_COMMONS(UAlign64(num, align));
}
inline float Align(float num, float align)
{
	return CPP_COMMONS(FAlign(num, align));
}
inline double Align(double num, double align)
{
	return CPP_COMMONS(DAlign(num, align));
}

#endif	// __cplusplus +++++++++++++++++++++++++++++++++++++++++++++++++


// Truncate a 32-bit float to 1.0:
INLINE float ftrunc_1(float f)
{
	f -= 1;
	*(unsigned __int32 *)&f &= ((f >= 0) - 1u);
	return f + 1;
}


#define MAX_DECIMAL_DIGITS(n)	((UINT)(n)<10?1:((UINT)(n)<100?2:((UINT)(n)<1000?3:((UINT)(n)<10000?4:((UINT)(n)<100000?5:((UINT)(n)<1000000?6:((UINT)(n)<10000000?7:8)))))))

INLINE unsigned MaxDecimalDigits(unsigned n)
{
	return (n >= 10) ? ((unsigned)log10((double)n) + 1) : 1;
}
INLINE unsigned MaxDecimalDigits64(unsigned __int64 n)
{
	return (n >= 10) ? ((unsigned)log10((double)n) + 1) : 1;
}
#define MaxDigits(n)	MaxDecimalDigits((unsigned)(n))
#define MaxDigits64(n)	MaxDecimalDigits64((unsigned __int64)(n))


INLINE double PointsAngle(double x1, double y1, double x2, double y2)
{
	return atan2(y2 - y1, x2 - x1);
}
INLINE float PointsAngleF(float x1, float y1, float x2, float y2)
{
	return atan2f(y2 - y1, x2 - x1);
}
INLINE double PointsAngleI(__int64 x1, __int64 y1, __int64 x2, __int64 y2)
{
	return atan2((double)(y2 - y1), (double)(x2 - x1));
}
INLINE float PointsAngleIF(int x1, int y1, int x2, int y2)
{
	return atan2f((float)(y2 - y1), (float)(x2 - x1));
}
#ifdef __cplusplus
inline float PointsAngle(float x1, float y1, float x2, float y2)
{
	return CPP_COMMONS(PointsAngleF(x1, y1, x2, y2));
}
inline double PointsAngle(__int64 x1, __int64 y1, __int64 x2, __int64 y2)
{
	return CPP_COMMONS(PointsAngleI(x1, y1, x2, y2));
}
inline double PointsAngle(POINT pt1, POINT pt2)
{
	return CPP_COMMONS(PointsAngleI(pt1.x, pt1.y, pt2.x, pt2.y));
}
template <class T>
inline double PointsAngle(T x1, T y1, T x2, T y2)
{
	return atan2(reinterpret_cast<double>(y2 - y1), reinterpret_cast<double>(x2 - x1));
}
template <class Tpt>
inline double PointsAngle(Tpt pt1, Tpt pt2)
{
	return CPP_COMMONS(PointsAngle(pt1.x, pt1.y, pt2.x, pt2.y));
}
#endif	// __cplusplus

INLINE float RadToDegF(float rad)
{
	return rad * (180 / MATH_PI_F);
}
INLINE float DegToRadF(float deg)
{
	return deg * (MATH_PI_F / 180);
}
#ifdef __cplusplus
inline constexpr float RadToDeg(float rad)
{
	return rad * (180 / MATH_PI_F);
}
inline constexpr double RadToDeg(double rad)
{
	return rad * (180 / MATH_PI);
}
inline constexpr float DegToRad(float deg)
{
	return deg * (MATH_PI_F / 180);
}
inline constexpr double DegToRad(double deg)
{
	return deg * (MATH_PI / 180);
}
#else	// __cplusplus
__inline double RadToDeg(double rad)
{
	return rad * (180 / MATH_PI);
}
__inline double DegToRad(double deg)
{
	return deg * (MATH_PI / 180);
}
#endif	// __cplusplus


INLINE bool_t FloatEq(float a, float b, float precision)
{
	ASSERT(precision >= 0);
	return fabsf(a - b) <= precision;
}
INLINE bool_t DoubleEq(double a, double b, double precision)
{
	ASSERT(precision >= 0);
	return fabs(a - b) <= precision;
}


_Check_return_
INLINE unsigned long SetDefaultMaskBit(unsigned long value,
		unsigned long mask, long unsigned deftBit)
{
	return (value | (deftBit & (!!(value & mask) - 1)));
}
_Check_return_
INLINE unsigned long long SetDefaultMaskBit64(unsigned long long value,
		unsigned long long mask, long long unsigned deftBit)
{
	return (value | (deftBit & (!!(value & mask) - 1ull)));
}


// XMM extensions /////////////////////////////////////////////////////

#define mm_sll_ps(ps, bytes)	_mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(ps), (int)(unsigned)(bytes)))
#define mm_srl_ps(ps, bytes)	_mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(ps), (int)(unsigned)(bytes)))
#define mm_sll_pd(pd, bytes)	_mm_castsi128_pd(_mm_slli_si128(_mm_castpd_si128(pd), (int)(unsigned)(bytes)))
#define mm_srl_pd(pd, bytes)	_mm_castsi128_pd(_mm_srli_si128(_mm_castpd_si128(pd), (int)(unsigned)(bytes)))


INLINE void mm_setzero_pi(__m128i* p128)
{
	*p128 = _mm_xor_si128(*p128, *p128);
}
INLINE void mm_setzero_ps(__m128* p128)
{
	*p128 = _mm_xor_ps(*p128, *p128);
}
INLINE void mm_setzero_pd(__m128d* p128)
{
	*p128 = _mm_xor_pd(*p128, *p128);
}

/**
* _MM_ROUND_NEAREST, _MM_ROUND_DOWN, _MM_ROUND_UP, _MM_ROUND_TOWARD_ZERO
* restore the old mode with _mm_setcsroldCSR)
*/
INLINE int mm_set_rounding_mode(int mode)
{
	const int oldCSR = _mm_getcsr();
	_mm_setcsr((oldCSR & ~_MM_ROUND_MASK) | mode);
	return oldCSR;
}
#define SetXmmRounding(mmRound)		mm_set_rounding_mode(mmRound)
#define RestoreXmmRounding(oldCSR)	_mm_setcsr(oldCSR)


INLINE double mm_round(double x)
{
#ifdef _M_IX86
	__int64 i;
	__asm {
		fld	x
		fistp i
		fild i
	}
#else
	return (double)_mm_cvtsd_si64(_mm_set_sd(x));
#endif
}

INLINE float mm_roundf(float x)
{
	return (float)_mm_cvt_ss2si(_mm_set_ss(x));
}


#define sqrti(n)	(unsigned)sqrt((double)(unsigned)(n))
#define sqrtu(n)	sqrti(n)
#define sqrti64(n)	(unsigned __int64)sqrt((double)(unsigned __int64)(n))
#define sqrtu64(n)	sqrti64(n)

_Check_return_ INLINE float mm_sqrtf(float _X)
{
	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(_X)));
}
#ifdef sqrtf
#undef sqrtf
#endif
#define sqrtf(x)	mm_sqrtf((float)(x))

_Check_return_ INLINE double mm_sqrt(double _X)
{
	const __m128d m128 = _mm_set_sd(_X);
	return _mm_cvtsd_f64(_mm_sqrt_sd(m128, m128));
}
#define sqrtd(x)	mm_sqrt((double)(x))
#ifdef sqrtl
#undef sqrtl
#endif
#define sqrtl(x)	mm_sqrt(x)

_Check_return_ INLINE float mm_rsqrtf(float _X)
{
	return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(_X)));
}
#define rsqrtf(x)	mm_rsqrtf((float)(x))

_Check_return_ INLINE double mm_tan(_In_ double _X)
{
	return _mm_cvtsd_f64(_mm_tan_pd(_mm_set_sd(_X)));
}
_Check_return_ INLINE float mm_tanf(_In_ float _X)
{
	return _mm_cvtss_f32(_mm_tan_ps(_mm_set_ss(_X)));
}

_Check_return_ INLINE double mm_tanh(_In_ double _X)
{
	return _mm_cvtsd_f64(_mm_tanh_pd(_mm_set_sd(_X)));
}
_Check_return_ INLINE float mm_tanhf(_In_ float _X)
{
	return _mm_cvtss_f32(_mm_tanh_ps(_mm_set_ss(_X)));
}

_Check_return_ INLINE double mm_atan(_In_ double _X)
{
	return _mm_cvtsd_f64(_mm_atan_pd(_mm_set_sd(_X)));
}
_Check_return_ INLINE float mm_atanf(_In_ float _X)
{
	return _mm_cvtss_f32(_mm_atan_ps(_mm_set_ss(_X)));
}

_Check_return_ INLINE double mm_atan2(_In_ double _Y, _In_ double _X)
{
	return _mm_cvtsd_f64(_mm_atan2_pd(_mm_set_sd(_Y), _mm_set_sd(_X)));
}
_Check_return_ INLINE float mm_atan2f(_In_ float _Y, _In_ float _X)
{
	return _mm_cvtss_f32(_mm_atan2_ps(_mm_set_ss(_Y), _mm_set_ss(_X)));
}

_Check_return_ INLINE double mm_atanh(_In_ double _X)
{
	return _mm_cvtsd_f64(_mm_atanh_pd(_mm_set_sd(_X)));
}
_Check_return_ INLINE float mm_atanhf(_In_ float _X)
{
	return _mm_cvtss_f32(_mm_atanh_ps(_mm_set_ss(_X)));
}

#ifdef __cplusplus	// +++++++++++++++++++++++++++++++++++++++++++++++++

_Check_return_ inline float mm_rsqrt(float x)
{
	return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
}
_Check_return_ inline float mm_tan(_In_ float _X)
{
	return _mm_cvtss_f32(_mm_tan_ps(_mm_set_ss(_X)));
}
_Check_return_ inline float mm_tanh(_In_ float _X)
{
	return _mm_cvtss_f32(_mm_tanh_ps(_mm_set_ss(_X)));
}
_Check_return_ inline float mm_atan(_In_ float _X)
{
	return _mm_cvtss_f32(_mm_atan_ps(_mm_set_ss(_X)));
}
_Check_return_ inline float mm_atan2(_In_ float _Y, _In_ float _X)
{
	return _mm_cvtss_f32(_mm_atan2_ps(_mm_set_ss(_Y), _mm_set_ss(_X)));
}
_Check_return_ inline float mm_atanh(_In_ float _X)
{
	return _mm_cvtss_f32(_mm_atanh_ps(_mm_set_ss(_X)));
}

#endif	// __cplusplus +++++++++++++++++++++++++++++++++++++++++++++++++


#define _MM_SHUFFLER(fp0, fp1, fp2, fp3)		_MM_SHUFFLE(fp3, fp2, fp1, fp0)
#define _MM_SHUFFLE_MASK(fp0, fp1, fp2, fp3)	_MM_SHUFFLER(fp0, fp1, fp2, fp3)

INLINE __m128i mm_setall_si128_(__m128i _any)
{
	return _mm_cmpeq_epi32(_any, _any);
}
INLINE __m128i mm_setall_si128()
{
	return CPP_COMMONS(mm_setall_si128_(_mm_setzero_si128()));
}
INLINE __m128 mm_setall_ps_(__m128 _any)
{
	return _mm_cmpeq_ps(_any, _any);
}
INLINE __m128 mm_setall_ps()
{
	return CPP_COMMONS(mm_setall_ps_(_mm_setzero_ps()));
}
INLINE __m128d mm_setall_pd_(__m128d _any)
{
	return _mm_cmpeq_pd(_any, _any);
}
INLINE __m128d mm_setall_pd()
{
	return CPP_COMMONS(mm_setall_pd_(_mm_setzero_pd()));
}

INLINE __m128i mm_set1_epi32(__int32 i32)
{
	return _mm_shuffle_epi32(_mm_cvtsi32_si128(i32), 0);
}
#define mm_set1_epi64(i64)	_mm_set1_epi64x(i64)

INLINE __m128i mm_setlo_epi64(__int64 i64)
{
#ifdef _M_X64
	return _mm_cvtsi64x_si128(i64);
#else
	return mm_set1_epi64(i64);
#endif
}
INLINE __m128i mm_setlo1_epi32(__int32 i32)
{
	return _mm_shuffle_epi32(_mm_cvtsi32_si128(i32), 0x50);
}
INLINE __m128i mm_setlo1_epi16(__int16 i16)
{
	return CPP_COMMONS(mm_setlo1_epi32((__int32)((unsigned __int32)(i16) | ((unsigned __int32)(i16) << 16))));
}

INLINE __m128i mm_epi32_31bit()
{
  return _mm_srli_epi32(CPP_COMMONS(mm_setall_si128()), 1);
}
INLINE __m128 mm_ps_31bit()
{
  return _mm_castsi128_ps(CPP_COMMONS(mm_epi32_31bit()));
}
#define mm_ps_absmsk	mm_ps_31bit
INLINE __m128i mm_epi64_63bit()
{
  return _mm_srli_epi64(mm_set1_epi64(-1ll), 1);
}
INLINE __m128d mm_pd_63bit()
{
  return _mm_castsi128_pd(CPP_COMMONS(mm_epi64_63bit()));
}
#define mm_pd_absmsk	mm_pd_63bit

INLINE __m128i mm128_abs_epi8(__m128i pi8)
{
	const __m128i msk = _mm_cmplt_epi8(pi8, _mm_setzero_si128());
	return _mm_add_epi8(_mm_xor_si128(pi8, msk), _mm_srli_si128(msk, 7));

}
INLINE __m128i mm128_abs_epi16(__m128i pi16)
{
	const __m128i msk = _mm_cmplt_epi16(pi16, _mm_setzero_si128());
	return _mm_add_epi16(_mm_xor_si128(pi16, msk), _mm_srli_epi16(msk, 15));

}
INLINE __m128i mm128_abs_epi32(__m128i pi32)
{
	const __m128i msk = _mm_cmplt_epi32(pi32, _mm_setzero_si128());
	return _mm_add_epi32(_mm_xor_si128(pi32, msk), _mm_srli_epi32(msk, 31));
}
#if (_M_TARG_ARCH < M_ARCH_SSSE3)
#define _mm_abs_epi8(pi8)	_mm128_abs_epi8(pi8)
#define _mm_abs_epi16(pi16)	_mm128_abs_epi16(pi16)
#define _mm_abs_epi32(pi32)	_mm128_abs_epi32(pi32)
#endif

INLINE __m128 mm_abs_ps(__m128 m128)
{
	return _mm_and_ps(CPP_COMMONS(mm_ps_absmsk()), m128);
}
INLINE __m128d mm_abs_pd(__m128d m128)
{
	return _mm_and_pd(CPP_COMMONS(mm_pd_absmsk()), m128);
}


INLINE __int32 mm_cvtepi32_pi8(_In_ __m128i epi32)
{
	return _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packs_epi32(epi32, epi32), epi32));
}
INLINE __int32 mm_cvtps_pi8(_In_ __m128 ps)
{
	return CPP_COMMONS(mm_cvtepi32_pi8(_mm_cvtps_epi32(ps)));
}

INLINE __m128i mm_swap02_epi32(__m128i epi32)
{
	return _mm_shuffle_epi32(epi32, _MM_SHUFFLER(2, 1, 0, 3));
}
INLINE __m128 mm_swap02_ps(__m128 epi32)
{
	return _mm_castsi128_ps(CPP_COMMONS(mm_swap02_epi32(_mm_castps_si128(epi32))));
}


INLINE __m128 mm_ps_1()
{
	return _mm_castsi128_ps(CPP_COMMONS(mm_set1_epi32(0x3F800000)));
}
INLINE __m128 mm_ps_72()
{
	return _mm_castsi128_ps(CPP_COMMONS(mm_set1_epi32(0x42900000)));
}
INLINE __m128 mm_ps_96()
{
	return _mm_castsi128_ps(CPP_COMMONS(mm_set1_epi32(0x42C00000)));
}
INLINE __m128 mm_ps_127()
{
	return _mm_castsi128_ps(CPP_COMMONS(mm_set1_epi32(0x42FE0000)));
}
INLINE __m128 mm_ps_255()
{
	return _mm_castsi128_ps(CPP_COMMONS(mm_set1_epi32(0x437F0000)));
}
INLINE __m128 mm_ps_1d255()
{
	return _mm_castsi128_ps(CPP_COMMONS(mm_set1_epi32(0x3B808081)));
}
INLINE __m128d mm_ps_1d255d()
{
	return _mm_castsi128_pd(mm_set1_epi64(0x3F70101010101010ll));
}


#ifndef _M_X64
INLINE int _rdrand64_step(unsigned __int64* prand)
{
	return (_rdrand32_step((unsigned __int32*)prand) ? _rdrand32_step((unsigned __int32*)prand + 1) : 0);
}
#endif


INLINE int ComparePointers(const void* a, const void* b)
{
	return (a != b) ? (((((void*)0 != b) & (a > b)) | ((void*)0 == a)) ? 1 : -1) : 0;
}


#ifdef __cplusplus	// ++++++++++++++++++++++++++++++++++++++++++++++++
// XMM extensions /////////////////////////////////////////////////////


inline bool operator ==(const __m128i a, const __m128i b)
{
	return !((__int16)_mm_movemask_epi8(_mm_cmpeq_epi8(a, b)) + 1);
}
inline bool operator !=(const __m128i a, const __m128i b)
{
	return !!((__int16)_mm_movemask_epi8(_mm_cmpeq_epi8(a, b)) + 1);
}

inline __m128i mm_cvtpu8_epi32(_In_ __m128i ui32, _In_ __m128i _zero)
{
	return _mm_unpacklo_epi16(_mm_unpacklo_epi8(ui32, _zero), _zero);
}
inline __m128i mm_cvtpu8_epi32(_In_ __m128i ui32)
{
	return CPP_COMMONS(mm_cvtpu8_epi32(ui32, _mm_setzero_si128()));
}
inline __m128i mm_cvtpu8_epi32(_In_ unsigned __int32 ui)
{
	return CPP_COMMONS(mm_cvtpu8_epi32(_mm_cvtsi32_si128(ui)));
}
inline __m128i mm_cvtpu8_epi32(_In_ const void* pui)
{
	return CPP_COMMONS(mm_cvtpu8_epi32(_mm_loadu_si32(pui)));
}

inline __m128 mm_cvtpu8_ps(_In_ __m128i ui32, _In_ __m128i _zero)
{
	return _mm_cvtepi32_ps(CPP_COMMONS(mm_cvtpu8_epi32(ui32, _zero)));
}
inline __m128 mm_cvtpu8_ps(_In_ __m128i ui32)
{
	return CPP_COMMONS(mm_cvtpu8_ps(ui32, _mm_setzero_si128()));
}
inline __m128 mm_cvtpu8_ps(_In_ unsigned __int32 ui)
{
	return CPP_COMMONS(mm_cvtpu8_ps(_mm_cvtsi32_si128(ui)));
}
inline __m128 mm_cvtpu8_ps(_In_ const void* pui)
{
	return CPP_COMMONS(mm_cvtpu8_ps(_mm_loadu_si32(pui)));
}


// Safe free //////////////////////////////////////////////////////////


template <class T> inline void SafeFree(T** ppmem)
{
	T* const pm = *ppmem;
	*ppmem = nullptr;
	free(pm);
}
template <class T> inline void SafeAlignedFree(T** ppmem)
{
	T* const pm = *ppmem;
	*ppmem = nullptr;
	_aligned_free(pm);
}
template <class T> inline void SafeDelete(T** ppobj)
{
	T* const po = *ppobj;
	*ppobj = nullptr;
	if (po) delete po;
}


#endif	// __cplusplus ++++++++++++++++++++++++++++++++++++++++++++++++

