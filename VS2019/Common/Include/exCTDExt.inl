/*	exCTDExt.inl	- common "C" and global namespace extensions

	Copyright (c) Alexandre Egorov

	This software is provided 'as-is', without any warranty.
	This software can be used and modified freely for any purpose,
	including commercial applications,
	as long as its origin is not misrepresented.
 */
#pragma once
#define _CMN_CTDEXT_INCLUDED_

// Definitions ////////////////////////////////////////////////////////


#ifdef _M_X64
#define _M_SSE	2
#elif defined(_M_IX86_FP)
#define _M_SSE	_M_IX86_FP
#else
#define _M_SSE	0
#endif
#if (_M_SSE >= 2)
#define _M_SSE2	2
#else
#define _M_SSE2	0
#endif


#define CPUF_SSE			0x01
#define CPUF_SSE2			(0x02 | CPUF_SSE)
#define CPUF_SSE3			(0x04 | CPUF_SSE2)
#define CPUF_SSSE3			(0x08 | CPUF_SSE3)
#define CPUF_SSE41			(0x10 | CPUF_SSSE3)
#define CPUF_SSE4			CPUF_SSE41
#define CPUF_SSE42			(0x20 | CPUF_SSE41)
#define CPUF_AVX 			(0x40 | CPUF_SSE42)
#define CPUF_AVX2			(0x80 | CPUF_AVX)
#define CPUF_AVX512F		(0x0100 | CPUF_AVX2)
#define CPUF_AVX512			CPUF_AVX512F


#ifdef M_BUILD_CPUF
#error	M_BUILD_CPUF must not be defined elsewhere
#endif
#if defined(__AVX512VL__) || defined(__AVX512F__) || defined(__AVX512DQ__) || defined(__AVX512CD__) || defined(__AVX512BW__)
#define M_BUILD_CPUF		CPUF_AVX512
#elif defined(__AVX2__)
#define M_BUILD_CPUF		CPUF_AVX2
#elif defined(__AVX__)
#define M_BUILD_CPUF		CPUF_AVX
#else
#define M_BUILD_CPUF		CPUF_SSE2
#endif
#if !defined(M_TARG_CPUF) || (M_TARG_CPUF < M_BUILD_CPUF)
#undef M_TARG_CPUF
#define M_TARG_CPUF			M_BUILD_CPUF
#endif


#define PACK_OSVERSION_(major, minor, build, sp, prodType)	(((UINT64)(UINT8)(major) << 56) | ((UINT64)(UINT8)(minor) << 48)	\
		| ((UINT64)(UINT32)(build) << 16) | ((UINT64)(UINT8)(sp) << 8) | (UINT8)(prodType))
#define PACK_OSVERSION_BUILD(major, minor, build)	PACK_OSVERSION_(major, minor, build, 0, 0)
#define PACK_OSVERSION_SP(major, minor, sp)			PACK_OSVERSION_(major, minor, 0, sp, 0)

#define GET_OSVERSION_MAJOR(v64)	(UINT8)((v64) >> 56)
#define GET_OSVERSION_MINOR(v64)	(UINT8)((v64) >>48)
#define GET_OSVERSION_BUILD(v64)	(UINT32)((v64) >> 16)
#define GET_OSVERSION_SP(v64)		(UINT8)((v64) >> 8)
#define GET_OSVERSION_PRODTYPE(v64)	(UINT8)(v64)

#define OSVERSION_WIN7_SP1		PACK_OSVERSION_SP(6, 1, 1)
#define OSVERSION_WIN8			PACK_OSVERSION_BUILD(6, 2, 0)
#define OSVERSION_WIN81			PACK_OSVERSION_BUILD(6, 3, 0)
#define OSVERSION_WINBLUE		OSVERSION_WIN81
#define OSVERSION_WIN10			PACK_OSVERSION_BUILD(10, 0, 0)
#define OSVERSION_WINTHRESHOLD	PACK_OSVERSION_BUILD(10, 0, 10240)
#define OSVERSION_WIN10_TH2		PACK_OSVERSION_BUILD(10, 0, 10586)
#define OSVERSION_WIN10_1511	OSVERSION_WIN10_TH2
#define OSVERSION_WIN10_RS1		PACK_OSVERSION_BUILD(10, 0, 14393)
#define OSVERSION_WIN10_1607	OSVERSION_WIN10_RS1
#define OSVERSION_WIN10_RS2		PACK_OSVERSION_BUILD(10, 0, 15063)
#define OSVERSION_WIN10_1703	OSVERSION_WIN10_RS2
#define OSVERSION_WIN10_RS3		PACK_OSVERSION_BUILD(10, 0, 16299)
#define OSVERSION_WIN10_1709	OSVERSION_WIN10_RS3
#define OSVERSION_WIN10_RS4		PACK_OSVERSION_BUILD(10, 0, 17134)
#define OSVERSION_WIN10_1803	OSVERSION_WIN10_RS4
#define OSVERSION_WIN10_RS5		PACK_OSVERSION_BUILD(10, 0, 17763)
#define OSVERSION_WIN10_1809	OSVERSION_WIN10_RS5
#define OSVERSION_WIN10_19H1	PACK_OSVERSION_BUILD(10, 0, 18362)
#define OSVERSION_WIN10_1903	OSVERSION_WIN10_19H1
#define OSVERSION_WIN10_19H2	PACK_OSVERSION_BUILD(10, 0, 18363)
#define OSVERSION_WIN10_VB		OSVERSION_WIN10_19H2
#define OSVERSION_WIN10_1909	OSVERSION_WIN10_19H2
#define OSVERSION_WIN10_20H1	PACK_OSVERSION_BUILD(10, 0, 19041)
#define OSVERSION_WIN10_2004	OSVERSION_WIN10_20H1
#define OSVERSION_WIN10_20H2	PACK_OSVERSION_BUILD(10, 0, 19042)


#ifndef _M_X64
#define _InterlockedCompareExchange_np(Destination, Exchange, Comparand)	\
		_InterlockedCompareExchange(Destination, Exchange, Comparand)
#endif	// _M_X64


#ifdef __cplusplus	// ++++++++++++++++++++++++++++++++++++++++++++++++

typedef bool	bool_t;

#ifndef null
#define null	nullptr
#endif

#ifndef PROPERTY
#define PROPERTY(type, name, getter, putter)	__declspec(property(get = getter, put = putter)) type name
#endif
#ifndef PROPGET
#define PROPGET(type, name, getter)				__declspec(property(get = getter)) type name
#endif
#ifndef PROPPUT
#define PROPPUT(type, name, putter)				__declspec(property(put = putter)) type name
#endif

#else	// __cplusplus	+++++++++++++++++++++++++++++++++++++++++++++++

typedef _Bool	bool_t;
#ifndef bool
typedef _Bool	bool;
#endif
#ifndef false
#define false	0
#endif
#ifndef true
#define true	1
#endif

#ifndef null
#define null	NULL
#define nullptr	NULL
#endif

#define ToBool(bval)	(_Bool)(0 != (bval))

#endif	// __cplusplus	+++++++++++++++++++++++++++++++++++++++++++++++


// Types //////////////////////////////////////////////////////////////


#ifdef _WIN64
#define _M_SIGNBIT			0x8000000000000000ll
typedef __int32				MSHORT;
typedef unsigned __int32	MWORD;
#else	// _WIN64
#define _M_SIGNBIT		0x80000000
typedef __int16				MSHORT;
typedef unsigned __int16	MWORD;
#endif	// _WIN64
#define _SIGNBIT			_M_SIGNBIT


#ifdef __cplusplus
constexpr HRESULT HRESULT_INVALID	= (HRESULT)-1l;
constexpr HRESULT S_OUTOFMEMORY		= ERROR_OUTOFMEMORY;
constexpr HRESULT S_INVALIARD		= ERROR_INVALID_PARAMETER;
constexpr HRESULT S_EMPTY			= ERROR_EMPTY;
constexpr HRESULT S_NOT_EMPTY		= ERROR_NOT_EMPTY;
constexpr HRESULT S_UNALIGNED		= ERROR_OFFSET_ALIGNMENT_VIOLATION;
constexpr HRESULT S_CANCELLED		= ERROR_CANCELLED;
constexpr HRESULT S_TIMEOUT			= ERROR_TIMEOUT;
constexpr HRESULT S_BUSY			= ERROR_BUSY;
constexpr HRESULT S_LARGEFILE		= ERROR_FILE_TOO_LARGE;
constexpr HRESULT S_NOT_SUPPORTED	= ERROR_NOT_SUPPORTED;
constexpr HRESULT E_NOT_SUPPORTED	= __HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
#else	// __cplusplus
#define HRESULT_INVALID				((HRESULT)-1l)
#define S_OUTOFMEMORY				((HRESULT)ERROR_OUTOFMEMORY)
#define S_INVALIARD					((HRESULT)ERROR_INVALID_PARAMETER)
#define S_EMPTY						((HRESULT)ERROR_EMPTY)
#define S_NOT_EMPTY					((HRESULT)ERROR_NOT_EMPTY)
#define S_UNALIGNED					((HRESULT)ERROR_OFFSET_ALIGNMENT_VIOLATION)
#define S_CANCELLED					((HRESULT)ERROR_CANCELLED)
#define S_TIMEOUT					((HRESULT)ERROR_TIMEOUT)
#define S_BUSY						((HRESULT)ERROR_BUSY)
#define S_LARGEFILE					((HRESULT)ERROR_FILE_TOO_LARGE)
#define S_NOT_SUPPORTED				((HRESULT)ERROR_NOT_SUPPORTED)
#define E_NOT_SUPPORTED				((HRESULT)__HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
#endif	// __cplusplus


typedef enum _MATCH_TYPE CPP_ONLY(: int) {
	MATCH_INVALID	= -1,
	MATCH_NONE		= 0,
	MATCH_PARTIAL	= 1,
	MATCH_FULL		= 2
} MATCH_TYPE;


typedef union UNION_SPLIT64 {
	unsigned __int64 UInt64;
	__int64 Int64;
	unsigned __int32 UInt32[2];
	__int32 Int32[2];
	unsigned long ULong[2];
	long Long[2];
	unsigned __int16 UInt16[4];
	__int16 Int16[4];
	wchar_t Wchar[4];
	unsigned short UShort[4];
	short Short[4];
	unsigned __int8 UInt8[8];
	__int8 Int8[8];
	char Char[8];
	unsigned char Byte[8];
} SPLIT64, * PSPLIT64;


typedef union MODULE_VERSION64 {
	unsigned long long Version64;	// MAKEDLLVERULL(major, minor, build, qfe)
	struct {
		unsigned short SPQFE;		// DLLVER_QFE_MASK
		unsigned short Build;		// DLLVER_BUILD_MASK
		unsigned short Minor;		// DLLVER_MINOR_MASK
		unsigned short Major;		// DLLVER_MAJOR_MASK
	} C_ONLY(Part);
} MVERSION64, * PMVERSION64;


typedef _Return_type_success_(return==S_OK) HRESULT	OKRESULT;
typedef _Return_type_success_(return>=0) LONG		ELENRET;
typedef _Return_type_success_(return>0) UINT		ULENRET;

typedef const void*				PCVOID;
typedef const unsigned char*	PCBYTE;
typedef const unsigned char*	PCUINT8;
typedef const signed __int8*	PCINT8;
typedef const unsigned __int16*	PCUINT16;
typedef const signed __int16*	PCINT16;
typedef const unsigned short*	PCWORD;
typedef const signed short*		PCSHORT;
typedef const unsigned long*	PCDWORD;
typedef const signed long*		PCLONG;
typedef const unsigned __int32*	PCUINT32;
typedef const unsigned __int32*	PCULONG32;
typedef const unsigned __int32*	PCDWORD32;
typedef const signed __int32*	PCINT32;
typedef const unsigned __int32*	PCUINT;
typedef const signed __int32*	PCINT;
typedef const unsigned __int64*	PCULONGLONG;
typedef const signed __int64*	PCLONGLONG;
typedef const unsigned __int64*	PCUINT64;
typedef const signed __int64*	PCINT64;


typedef enum SEVERITY_STATUS_LEVEL CPP_ONLY(: unsigned __int8) {
	STL_NONE = 0, Status_None = STL_NONE, STL_DEBUG = STL_NONE, Status_Debug = STL_DEBUG,
	STL_TRACE = 0x01, Status_Trace = STL_TRACE,
	STL_NEUTRAL = 0x02, Status_Neutral = STL_NEUTRAL,
	STL_INFORMATION = 0x04, Status_Information = STL_INFORMATION, STL_INFO = STL_INFORMATION, Status_Info = Status_Information,
	STL_SUCCESS = 0x10, Status_Success = STL_SUCCESS,
	STL_WARNING = 0x20, Status_Warning = STL_WARNING,
	STL_ERROR = 0x40, Status_Error = STL_ERROR,
	STL_CRITICAL = 0x80, Status_Critical = STL_CRITICAL
} STATUS_LEVEL, StatusLevel, ERROR_LEVEL, ErrorLevel;


enum WM_ESU_ID CPP_ONLY(: unsigned __int32) {
	WMID_APP_RESERVED_FIRST = WM_APP + 0x0026,
	WM_APP_COMMAND, WM_APP_BADCHECKSUM,
	WMID_APP_RESERVED_LAST = WMID_APP_RESERVED_FIRST + 20,
	WMID_APP_USER_FIRST
};


typedef enum MEMORY_TYPE CPP_ONLY(:UINT8) {
	MTYPE_NONE = 0, MTYPE_UNKNOWN = MTYPE_NONE, MTYPE_HEAP = 0x01, MTYPE_VIRTUAL = 0x02
} MEMTYPE;


// Functions //////////////////////////////////////////////////////////


INLINE bool_t mm_isequal_128(_In_reads_bytes_(16) const void *p1, _In_reads_bytes_(16) const void *p2)
{
	return !((__int16)_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_si128((__m128i*)p1), _mm_loadu_si128((__m128i*)p2))) + (__int16)1);
}
INLINE bool_t IsMem16Equal(_In_reads_bytes_(16) const void* p1, _In_reads_bytes_(16) const void* p2)
{
#if defined(_WIN64) || (_M_SSE < 2)
	return (*(UNALIGNED unsigned __int64*)p1 == *(UNALIGNED unsigned __int64*)p2) &
		(*((UNALIGNED unsigned __int64*)p1 + 1) == *((UNALIGNED unsigned __int64*)p2 + 1));
#else
	return mm_isequal_128(p1, p2);
#endif
}
#define Is16BytesEqual(p1, p2)	IsMem16Equal(p1, p2)

#define IsMem8Equal(p1, p2)		(*((UNALIGNED unsigned __int64*)(p1)) == *((UNALIGNED unsigned __int64*)(p2)))
#define Is8BytesEqual(p1, p2)	(*((UNALIGNED unsigned __int64*)(p1)) == *((UNALIGNED unsigned __int64*)(p2)))

INLINE bool_t IsMem8Zero(_In_reads_bytes_(16) const void *p)
{
	return 0 == *((UNALIGNED unsigned __int64*)p);
}
#define Is8BytesZero(p)		IsMem8Zero(p)

INLINE bool_t IsMem16Zero(_In_reads_bytes_(16) const void *p)
{
#if defined(_WIN64) || (_M_SSE < 2)
	return !((*(UNALIGNED unsigned __int64*)p) | *((UNALIGNED unsigned __int64*)p + 1));
#else
	return 0 == (__int16)_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_si128((__m128i*)p), _mm_setzero_si128())) + 1;
#endif
}
#define Is16BytesZero(p)	IsMem16Zero(p)


#ifdef __cplusplus	// ++++++++++++++++++++++++++++++++++++++++++++++++

template <class TPtr> TPtr reallocp(_Inout_ TPtr* ppmem, _In_ size_t cb)
{
	TPtr const tmp = static_cast<TPtr>(realloc(*ppmem, cb));
	if (tmp) *ppmem = tmp;
	return tmp;
}

inline bool InlineIsEqualGUID(const GUID& rguid1, const GUID& rguid2)
{
	return IsMem16Equal(&rguid1, &rguid2);
}

#else	// __cplusplus ++++++++++++++++++++++++++++++++++++++++++++++++

__inline void* reallocp(_Inout_ void** ppmem, _In_ size_t cb)
{
	void* const tmp = realloc(*ppmem, cb);
	if (tmp) *ppmem = tmp;
	return tmp;
}

__inline bool_t InlineIsEqualGUID(const GUID* pguid1, const GUID* pguid2)
{
	return IsMem16Equal(pguid1, pguid2);
}

#endif	// __cplusplus ++++++++++++++++++++++++++++++++++++++++++++++++


#define mm_alloc(cb)	_aligned_malloc((size_t)(cb), MM_MEMORY_ALIGN)
#define mm_free(pv)		_aligned_free(pv)


_Check_return_ _Post_satisfies_(psz || return == 0)
INLINE size_t __fastcall wcslen_s(_In_opt_z_ PCWSTR psz)
{
	return (psz ? wcslen(psz) : 0);
}
_Check_return_ _Post_satisfies_(psz || return == 0)
INLINE size_t __fastcall strlen_s(_In_opt_z_ PCSTR psz)
{
	return (psz ? strlen(psz) : 0);
}

_Check_return_ _Post_satisfies_(return >= 0 && return == *pcch && (psz || return == 0))
INLINE unsigned __fastcall NLStringLen(_In_NLS_string_opt_(*pcch) PCWCH psz, _Inout_ int* pcch)
{
	return (unsigned)((*pcch < 0) ? (*pcch = (int)CPP_GLOBAL(wcslen_s(psz))) : *pcch);
}


INLINE char* __fastcall InlineFindFileNameA(_In_NLS_string_opt_(pathLen) const char* pathName, int pathLen)
{
	if (pathName)
	{
		const char* pend = pathName + ((pathLen >= 0) ? (size_t)pathLen : strlen(pathName));
		--pend;
		while (pend >= pathName)
		{
			const char ch = *pend--;
			if (('\\' != ch) & ('/' != ch) & (':' != ch))
				continue;
			pathName = pend + 2;
			break;
		}
	}
	return (char*)pathName;
}
INLINE wchar_t* __fastcall InlineFindFileNameW(_In_NLS_string_opt_(pathLen) const wchar_t* pathName, int pathLen)
{
	if (pathName)
	{
		const wchar_t* pend = pathName + ((pathLen >= 0) ? (size_t)pathLen : wcslen(pathName));
		--pend;
		while (pend >= pathName)
		{
			const wchar_t ch = *pend--;
			if ((L'\\' != ch) & (L'/' != ch) & (L':' != ch))
				continue;
			pathName = pend + 2;
			break;
		}
	}
	return (wchar_t*)pathName;
}