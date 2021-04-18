/*	exCmnInc.h	- main include header for common extensions

	Copyright (c) Alexandre Egorov

	This software is provided 'as-is', without any warranty.
	This software can be used and modified freely for any purpose,
	including commercial applications,
	as long as its origin is not misrepresented.
 */
#pragma once
#define _COMMONS_INCLUDED_

#if defined(_AFX)
#error MFC is not suppoted! _AFX must not be #defined!
#endif

#ifndef _WINDOWS_
#include "exSDKInc.h"
#endif
#include "exRCExt.h"
#include <assert.h>
#ifdef _DEBUG
#ifndef ENABLE_CRT_TRACE
#define ENABLE_CRT_TRACE
#endif
#endif
#ifdef ENABLE_CRT_TRACE
#include <crtdbg.h>
#endif


#ifdef _NODEFAULTLIB
#ifndef _VC_NODEFAULTLIB
#define _VC_NODEFAULTLIB	1
#pragma comment(linker, "/NODEFAULTLIB")
#endif
#elif defined(_VC_NODEFAULTLIB)
#define _NODEFAULTLIB		_VC_NODEFAULTLIB
#endif

#if defined(_VC_NODEFAULTLIB) || !defined(_DEBUG) || defined(ENABLE_INTRINSICS)
#pragma intrinsic(memcpy, memmove, memset, memcmp, memchr, strlen, wcslen, strcmp, wcscmp, strcpy, wcscpy,	\
	abs, labs, llabs, _abs64, fabs, floor, ceil, div, ldiv, lldiv, pow, sqrt, log, log10,	\
	_byteswap_uint64, _byteswap_ulong, _byteswap_ushort, _rotr64, _rotl64, _rotr, _rotl, _rotr16, _rotl16, _rotr8, _rotl8,	\
	atan, atan2, __emul, __emulu)
#ifdef _WIN64
#pragma intrinsic(floorf, ceilf, logf, log10f, atan2f)
#endif
#endif

#pragma warning(disable: 4197 6255 6286 26812)


///////////////////////////////////////////////////////////////////////


#ifdef __cplusplus	// ++++++++++++++++++++++++++++++++++++++++++++++++

#if defined(ROOT_NAMESPACE)
#ifndef RootNamespace
#define RootNamespace	ROOT_NAMESPACE
#endif
#elif defined(RootNamespace)
#define ROOT_NAMESPACE	RootNamespace
#endif
#if defined(RootNamespace)
#define BEGIN_ROOT_NAMESPACE	namespace RootNamespace {
#define END_ROOT_NAMESPACE		}	// end RootNamespace
#define CPP_ROOTNS(e)			RootNamespace##::##e
#else
#define BEGIN_ROOT_NAMESPACE
#define END_ROOT_NAMESPACE
#define CPP_ROOTNS(e)			::##e
#endif

#if defined(COMMON_NAMESPACE)
#ifndef CommonNamespace
#define CommonNamespace	COMMON_NAMESPACE
#endif
#elif defined(CommonNamespace)
#define COMMON_NAMESPACE	CommonNamespace
#elif defined(RootNamespace)
#define COMMON_NAMESPACE	RootNamespace
#define CommonNamespace		RootNamespace
#endif
#if defined(CommonNamespace)
#define BEGIN_COMMON_NAMESPACE	namespace CommonNamespace {
#define END_COMMON_NAMESPACE	}	// end CommonNamespace
#define CPP_COMMONS(e)			CommonNamespace##::##e
#else
#define BEGIN_COMMON_NAMESPACE
#define END_COMMON_NAMESPACE
#define CPP_COMMONS(e)			::##e
#endif

#else	// __cplusplus	+++++++++++++++++++++++++++++++++++++++++++++++++

#define CPP_ROOTNS(e)			e
#define CPP_COMMONS(e)			e
#define BEGIN_ROOT_NAMESPACE
#define END_ROOT_NAMESPACE
#define BEGIN_COMMON_NAMESPACE
#define END_COMMON_NAMESPACE

#endif	// __cplusplus	+++++++++++++++++++++++++++++++++++++++++++++++++


#ifndef NAKED
#define NAKED		__declspec(naked)
#endif

#ifndef SELECTANY
#define SELECTANY	__declspec(selectany)
#endif
#ifndef DECLGLOBAL
#define DECLGLOBAL	__declspec(selectany)
#endif

#ifndef CONSTEXPR
#ifdef __cplusplus
#define CONSTEXPR	constexpr
#else
#define CONSTEXPR
#endif
#endif

#ifndef VECTORCALL
#define VECTORCALL	__vectorcall
#endif
#ifndef VECTORAPI_
#define VECTORAPI_(ret_type)	ret_type _VECTORCALL
#endif
#ifndef STDVECTORAPI_
#define STDVECTORAPI_(ret_type)	EXTERN_C VECTORAPI_(ret_type)
#endif
#ifndef STDVECTORAPI
#define STDVECTORAPI	STDVECTORAPI_(HRESULT)
#endif


#ifndef INLINE
#ifdef __cplusplus
#define INLINE			inline
#else
#define INLINE			__inline
#endif	// __cplusplus
#endif
#ifndef _INLINE
#define _INLINE			INLINE
#endif

#ifndef FORCEINLINE
#define FORCEINLINE		__forceinline
#endif

#ifndef NOINLINE
#define NOINLINE		__declspec(noinline)
#endif

#ifndef NOVTABLE
#define NOVTABLE		__declspec(novtable)
#endif
	
#ifndef NOALIAS
#define NOALIAS			__declspec(noalias)
#endif

#ifndef RESTRICT
#define RESTRICT		__declspec(restrict)
#endif

#ifndef ALLOCATOR
#define ALLOCATOR		__declspec(allocator)
#endif


#define CPU_CACHE_LINE_SIZE	64
#define MM_MEMORY_ALIGN		CPU_CACHE_LINE_SIZE

#define ALIGNOF(x)		alignof(x)
#define ALIGNAS(n)		alignas(n)
#define DECLALIGN(n)	__declspec(align(n))
#define DECLALIGN4		DECLALIGN(4)
#define DECLALIGN8		DECLALIGN(8)
#define DECLALIGN16		DECLALIGN(16)
#define DECLALIGN32		DECLALIGN(32)
#define DECLALIGN64		DECLALIGN(64)
#define DECLALIGN128	DECLALIGN(128)
#define DECLALIGN256	DECLALIGN(256)
#define DECLALIGN_CACHE	DECLALIGN(CPU_CACHE_LINE_SIZE)


#define PUSH_MACRO(name)	__pragma(push_macro(name))
#define POP_MACRO(name)		__pragma(pop_macro(name))

#define PUSH_SDKVER	PUSH_MACRO("_WIN32_IE");PUSH_MACRO("_WIN32_WINNT");PUSH_MACRO("NTDDI_VERSION")
#define POP_SDKVER	POP_MACRO("NTDDI_VERSION");POP_MACRO("_WIN32_WINNT");POP_MACRO("_WIN32_IE")


#define __stringize(s)	#s
#define _stringize_x(x)	__stringize(x)
#define _stringize_n(n)	__stringize(n)
#define _preproc_message(text)	__pragma(message(__FILE__ "(" _stringize_n(__LINE__) ") : " #text))
#define MESSAGE(text)	_preproc_message(text)


#ifdef _preproc_message
#define __print_todo_message(text)	_preproc_message(text)
#else
#define __print_todo_message(text)	__pragma(message(text))
#endif
#ifndef _TODO
#define _TODO(text)		__print_todo_message(text)
#endif
#ifndef TODO
#define TODO(text)		__print_todo_message(text)
#endif
#ifndef FIXME
#ifdef ENABLE_CRT_TRACE
#define FIXME(text)		__print_todo_message(text)
#else
#define FIXME(text)		static_assert(0,"FIXME in build: '" #text## "'")
#endif
#endif


// Use to convert literal definitions to Unicode:
#define _L(s)			L ## s
#define _A2W(s)			_L(s)
#ifndef A2W
#define A2W(s)			_L(s)
#endif

#define __WFILE__		_A2W(__FILE__)
#ifdef UNICODE
#define __TFILE__		__WFILE__
#else
#define __TFILE__		__FILE__
#endif
#define __WFUNCTION__	_A2W(__FUNCTION__)
#ifdef UNICODE
#define __TFUNCTION__	__WFUNCTION__
#else
#define __TFUNCTION__	__FUNCTION__
#endif


#ifdef __cplusplus

#define BYREF(ref)				(ref)

#define recast(type,obj)		reinterpret_cast<type>(obj)
#define RECAST(type,obj)		(reinterpret_cast<type>(obj))
#define statcast(type,obj)		static_cast<type>(obj)
#define STATCAST(type,obj)		(static_cast<type>(obj))
#define constcast(type,obj)		const_cast<type>(obj)
#define CONSTCAST(type,obj)		(const_cast<type>(obj))
#define PPV_ARG(ppv)			reinterpret_cast<void**>(ppv)
#define C_ONLY(e)
#define CPP_ONLY(e)				e
#define CPP_GLOBAL(e)			::##e

#define MUTABLE					mutable

#else	// #ifdef __cplusplus

#define BYREF(ref)				&(ref)

#define recast(type,obj)		(type)(obj)
#define RECAST(type,obj)		((type)(obj))
#define statcast(type,obj)		(type)(obj)
#define STATCAST(type,obj)		((type)(obj))
#define constcast(type,obj)		(type)(obj)
#define CONSTCAST(type,obj)		((type)(obj))
#define PPV_ARG(ppv)			(void**)(ppv)
#define C_ONLY(e)				e
#define CPP_ONLY(e)
#define CPP_GLOBAL(e)			e

#define MUTABLE

#endif	// #ifdef __cplusplus


///////////////////////////// Platforms ///////////////////////////////////////////////////////


#if defined _M_X64
#ifndef _WIN64
#define _WIN64	1
#endif
#elif defined(_WIN64) || defined(_M_AMD64)
#define _M_X64	100
#endif


#ifdef _M_AMD64
#define PLATFORM_NAMEA	"x64"
#define PLATFORM_NAMEW	L"x64"
#elif defined _M_IX86
#define PLATFORM_NAMEA	"x86"
#define PLATFORM_NAMEW	L"x86"
#elif defined _M_ARM64
#define PLATFORM_NAMEA	"arm64"
#define PLATFORM_NAMEW	L"arm64"
#elif defined _M_ARM
#define PLATFORM_NAMEA	"arm"
#define PLATFORM_NAMEW	L"arm"
#else
__pragma(message("Unknown target platform"))
#define PLATFORM_NAMEA	"win32"
#define PLATFORM_NAMEW	L"win32"
#endif

#ifdef UNICODE
#define PLATFORM_NAME	PLATFORM_NAMEW
#else
#define PLATFORM_NAME	PLATFORM_NAMEA
#endif


///////////////////////////////////////////////////////////////////////


#define WARNING_SUPPRESS(x)	__pragma(warning(suppress: x##))
#define WARNING_DISABLE(x)	__pragma(warning(disable: x##))
#define WARNING_DEFAULT(x)	__pragma(warning(default: x##))
#define SUPPRESS_WARNING(x)	WARNING_SUPPRESS(x)

#define PRAGMA_INTRINSIC(names)	__pragma(intrinsic(names))
#define PRAGMA_FUNCTION(names)	__pragma(function(names))


#ifndef STATIC_ASSERT
#define STATIC_ASSERT(e)	_STATIC_ASSERT(e)
#endif

#define DLLEXPORT		EXTERN_C __declspec(dllexport)
#define DLLIMPORT		EXTERN_C __declspec(dllimport)


#define UUIDOF(t)	__uuidof(t)


// This is for use with GetProcAddress()
#ifdef UNICODE
#define _AW_SUFFIX	"W"
#else
#define _AW_SUFFIX	"A"
#endif


#ifndef _In_NLS_string_opt_
#define _In_NLS_string_opt_(size)	_SAL_L_Source_(_In_NLS_string_, (size),  _When_((size) < 0,  _In_opt_z_)	\
									_When_((size) >= 0, _In_reads_opt_(size)))
#endif
#ifndef _In_opt_NLS_string_
#define _In_opt_NLS_string_(size)	_In_NLS_string_opt_(size)
#endif

#ifndef _COM_Outptr_result_nullonfailure_
#define _COM_Outptr_result_nullonfailure_		_COM_Outptr_ _On_failure_(_Deref_post_null_)
#endif
#ifndef _COM_Outptr_opt_result_nullonfailure_
#define _COM_Outptr_opt_result_nullonfailure_	_COM_Outptr_opt_ _On_failure_(_Deref_post_null_)
#endif

#ifndef _Out_post_maybenull_
#define _Out_post_maybenull_		_Out_ _Post_maybenull_
#define _Out_post_maybenull_opt_	_Out_opt_ _Post_maybenull_
#define _Outptr_post_maybenull_		_Outptr_ _Post_maybenull_
#define _Outptr_post_maybenull_opt_	_Outptr_opt_ _Post_maybenull_
#endif

#define _Return_maybenull_	_Check_return_ _Ret_maybenull_
#define _Ret_error_code_	_Success_(return == 0)
#define _Ret_error_			_Ret_error_code_
#define _RetError_			_Ret_error_code_
#define _Ret_valid_count_	_Success_(return > 0)
#define _RetCount_			_Ret_valid_count_
#define _Ret_bool_			_Success_(return != 0)
#define _RetBool_			_Ret_bool_


#ifdef __fallthrough
#define __through		__pragma(warning(suppress: 6319));__fallthrough
#define _THROUGH		__through
#ifndef _fallthrough
#define _fallthrough	__through
#endif
#endif


///////////////////////////////////////////////////////////////////////


#ifndef offsetof
#ifdef __cplusplus
#ifdef _WIN64
#define offsetof(s, m)	(size_t)((ptrdiff_t)&reinterpret_cast<const volatile char&>((((s*)0)->m)))
#else	// #ifdef _WIN64
#define offsetof(s, m)	(size_t)&reinterpret_cast<const volatile char&>((((s*)0)->m))
#endif	// #ifdef _WIN64
#else  // #ifdef __cplusplus
#ifdef _WIN64
#define offsetof(s, m)	(size_t)((ptrdiff_t)&(((s*)0)->m))
#else	// #ifdef _WIN64
#define offsetof(s, m)	(size_t)&(((s*)0)->m)
#endif	// #ifdef _WIN64
#endif  // #ifdef __cplusplus
#endif	// #ifndef offsetof


#define STRUCT_MEMBER_POINTER_(structname, member)	(const volatile char*)(&((structname*)0)->member)
#define STRUCT_MEMBER_UPTR_(structname, member)		(size_t)(&((structname*)0)->member)
#ifndef STRUCT_MEMBER_OFFSET
#define STRUCT_MEMBER_OFFSET(structname, member)	(STRUCT_MEMBER_UPTR_(structname, member) - (size_t)((structname*)0))
#endif
#ifndef STRUCT_OFFSET_TO
#define STRUCT_OFFSET_TO(structname, member)		STRUCT_MEMBER_OFFSET(structname, member)
#endif
// Excludes the member
#define SIZEOF_STRUCT_WITHOUT(structname, member)	STRUCT_MEMBER_OFFSET(structname, member)
// Includes the member
#define SIZEOF_STRUCT_WITH(structname, member)		(STRUCT_MEMBER_OFFSET(structname, member) + sizeof(((structname*)0)->member))
// Includes the member
#define SIZEOF_STRUCT_FROM(structname, member)		((sizeof(structname)) - STRUCT_MEMBER_OFFSET(structname, member))
// Excludes the member
#define SIZEOF_STRUCT_AFTER(structname, member)		((sizeof(structname)) - SIZEOF_STRUCT_WITH(structname, member))
// Includes both members
#define SIZEOF_STRUCT_RANGE(stname, mfirst, mlast)	(SIZEOF_STRUCT_WITH(stname, mlast) - STRUCT_MEMBER_OFFSET(stname, mfirst))
// Excludes the last member
#define SIZEOF_STRUCT_RANGE_TO(stname, mfirst, mlast)	(SIZEOF_STRUCT_WITHOUT(stname, mlast) - STRUCT_MEMBER_OFFSET(stname, mfirst))

#define OBJECT_MEMBER_POINTER_(pobj, member)		(const volatile char*)(&((structname*)0)->member)
#define OBJECT_MEMBER_UPTR_(pobj, member)			(size_t)(&(pobj)->member)
#ifndef OBJECT_MEMBER_OFFSET
#define OBJECT_MEMBER_OFFSET(pobj, member)			(OBJECT_MEMBER_UPTR_(pobj, member) - (size_t)(pobj))
#endif
#ifndef OBJECT_OFFSET_TO
#define OBJECT_OFFSET_TO(pobj, member)				OBJECT_MEMBER_OFFSET(pobj, member)
#endif
// Excludes the member
#define SIZEOF_OBJECT_WITHOUT(pobj, member)			OBJECT_MEMBER_OFFSET(pobj, member)
// Includes the member
#define SIZEOF_OBJECT_WITH(pobj, member)			(OBJECT_MEMBER_OFFSET(pobj, member) + sizeof((pobj)->member))
// Includes the member
#define SIZEOF_OBJECT_FROM(pobj, member)			((sizeof(*(pobj))) - OBJECT_MEMBER_OFFSET(pobj, member))
// Excludes the member
#define SIZEOF_OBJECT_AFTER(pobj, member)			((sizeof(*(pobj))) - SIZEOF_OBJECT_WITH(pobj, member))
// Includes both members
#define SIZEOF_OBJECT_RANGE(pobj, mfirst, mlast)	(SIZEOF_OBJECT_WITH(pobj, mlast) - OBJECT_MEMBER_OFFSET(pobj, mfirst))
// Excludes the last member
#define SIZEOF_OBJECT_RANGE_TO(pobj, mfirst, mlast)	(SIZEOF_OBJECT_WITHOUT(pobj, mlast) - OBJECT_MEMBER_OFFSET(pobj, mfirst))


#define IS_ALIGNED(arg, align)		(((DWORD_PTR)(arg)&((align)-1))==0)
#define ALIGNED_PTR(ptr, align)		IS_ALIGNED(ptr, align)
#define ASSERT_ALIGNED(arg, align)	ASSERT(IS_ALIGNED(arg,align))
#define ASSUME_ALIGNED(arg, align)	ASSUME(IS_ALIGNED(arg,align))
#ifndef IS_2_POW_N
#define IS_2_POW_N(X)	(((X)&((X)-1))==0)
#endif

#define ALIGN(data,n)		(((data)+(n)-1) & ~((n)-1))
#define ALIGN_SIZE(data,n)	(((size_t)(data)+(n)-1) & ~((n)-1))
#define ALIGN_PTR(data,n)	ALIGN_SIZE(data,n)
#define ALIGN2(data)		ALIGN(data,2)
#define ALIGN4(data)		ALIGN(data,4)
#define ALIGN8(data)		ALIGN(data,8)
#define ALIGN16(data)		ALIGN(data,16)
#define ALIGN32(data)		ALIGN(data,32)
#define ALIGN64(data)		ALIGN(data,64)
#define ALIGN128(data)		ALIGN(data,128)
#define ALIGN2_PTR(data)	ALIGN_PTR(data,2)
#define ALIGN4_PTR(data)	ALIGN_PTR(data,4)
#define ALIGN8_PTR(data)	ALIGN_PTR(data,8)
#define ALIGN16_PTR(data)	ALIGN_PTR(data,16)
#define ALIGN32_PTR(data)	ALIGN_PTR(data,32)
#define ALIGN64_PTR(data)	ALIGN_PTR(data,64)
#define ALIGN_CACHE(data)	ALIGN(data,CPU_CACHE_LINE_SIZE)

#define BYTE0(l)			((unsigned char)(l))
#define BYTE1(l)			((unsigned char)((l) >> 8))
#define BYTE2(l)			((unsigned char)((l) >> 16))
#define BYTE3(l)			((unsigned char)((l) >> 24))
#define CHAR0(l)			((signed char)BYTE0(l))
#define CHAR1(l)			((signed char)BYTE1(l))
#define CHAR2(l)			((signed char)BYTE2(l))
#define CHAR3(l)			((signed char)BYTE3(l))

#define LOSHORT(l)			((signed short)(l))
#define HISHORT(l)			((signed short)((__int32)(l) >> 16))
#define LOLONG(ll)			((__int32)(ll))
#define HILONG(ll)			((__int32)((__int64)(ll)>>32))
#define LOULONG(ll)			((unsigned __int32)(ll))
#define HIULONG(ll)			((unsigned __int32)((unsigned __int64)(ll)>>32))
#ifndef LODWORD
#define LODWORD(ll)			LOULONG(ll)
#endif
#ifndef HIDWORD
#define HIDWORD(ll)			HIULONG(ll)
#endif

#define MAKEINT32(l,h)		((__int32)(unsigned __int16)(l) | ((__int32)(unsigned __int16)(h) << 16))
#define MAKEUINT32(l,h)		((unsigned __int32)(unsigned __int16)(l) | ((unsigned __int32)(unsigned __int16)(h) << 16))
#define MAKEULONG(l,h)		((unsigned long)(unsigned __int16)(l) | ((unsigned long)(unsigned __int16)(h) << 16))
#define MAKEDWORD(l,h)		MAKEULONG(l, h)
#define MAKEINT64(l, h)		((__int64)(((unsigned __int32)(l))|((unsigned __int64)((unsigned __int32)(h)))<<32))
#define MAKELONG64(l, h)	MAKEINT64(l, h)
#define MAKELONGLONG(l, h)	MAKEINT64(l, h)
#define MAKEUINT64(l, h)	((unsigned __int64)(((unsigned __int32)(l))|((unsigned __int64)((unsigned __int32)(h)))<<32))
#define MAKEULONG64(l, h)	MAKEUINT64(l, h)
#define MAKEULONGLONG(l, h)	MAKEUINT64(l, h)

#define MAKELONGB(b1,b2,b3,b4)					MAKELONG(MAKEWORD(b1,b2),MAKEWORD(b3,b4))
#define MAKELONGLONGB(b1,b2,b3,b4,b5,b6,b7,b8)	MAKELONGLONG(MAKELONGB(b1,b2,b3,b4),MAKELONGB(b5,b6,b7,b8))
#define MAKELONGLONGW(w1,w2,w3,w4)				MAKELONGLONG(MAKEULONG(w1,w2),MAKELONG(w3,w4))
#define MAKEULONGB(b1,b2,b3,b4)					MAKEULONG(MAKEWORD(b1,b2),MAKEWORD(b3,b4))
#define MAKEULONGLONGB(b1,b2,b3,b4,b5,b6,b7,b8)	MAKEULONGLONG(MAKEULONGB(b1,b2,b3,b4),MAKEULONGB(b5,b6,b7,b8))
#define MAKEULONGLONGW(w1,w2,w3,w4)				MAKEULONGLONG(MAKEULONG(w1,w2),MAKEULONG(w3,w4))
#define MAKEULONG4(b1,b2,b3,b4)					MAKEULONGB(b1,b2,b3,b4)
#define MAKEULONGLONG8(b1,b2,b3,b4,b5,b6,b7,b8)	MAKEULONGLONGB(b1,b2,b3,b4,b5,b6,b7,b8)
#define MAKEULONGLONG4(w1,w2,w3,w4)				MAKEULONGLONGW(w1,w2,w3,w4)


#define MIN_(a, b)		(((a)<=(b))?(a):(b))
#define MAX_(a, b)		(((a)>=(b))?(a):(b))
#define ABS_(i)			(((i) >= 0) ? (i) : -(i))
#define ulabs(i)		(unsigned)labs((long)(i))
#define uabs(i)			ulabs(i)
#define ulabs64(ll)		(unsigned long long)llabs((long long)(ll))
#define uabs64(ll)		ulabs64(ll)
#define abd(a,b)		(unsigned)labs((long)((a)-(b)))		// absolute difference


#define BIN___(x)						\
	(((x / 01ul) % 010)*(2>>1) +		\
	((x / 010ul) % 010)*(2<<0) +		\
	((x / 0100ul) % 010)*(2<<1) +		\
	((x / 01000ul) % 010)*(2<<2) +		\
	((x / 010000ul) % 010)*(2<<3) +		\
	((x / 0100000ul) % 010)*(2<<4) +	\
	((x / 01000000ul) % 010)*(2<<5) +	\
	((x / 010000000ul) % 010)*(2<<6))

#define BIN8(x) BIN___(0##x)
#define BIN16(x1,x2) \
    ((BIN8(x1)<<8)+BIN8(x2))
#define BIN24(x1,x2,x3) \
    ((BIN8(x1)<<16)+(BIN8(x2)<<8)+BIN8(x3))
#define BIN32(x1,x2,x3,x4) \
    ((BIN8(x1)<<24)+(BIN8(x2)<<16)+(BIN8(x3)<<8)+BIN8(x4))
#define BIN64(x1,x2,x3,x4,x5,x6,x7,x8) \
    ((__int64(BIN32(x1,x2,x3,x4)) << 32) + __int64(BIN32(x5,x6,x7,x8))FILETIME)


#define SET_ONLY_BITS(n, mask)	(0==((n)&~(mask)))
 

#define bitsizeof(t)	(sizeof(t)*8)
#define BITSIZE(t)		bitsizeof(t)
#define wsizeof(t)		(sizeof(t)/sizeof(wchar_t))
#define tsizeof(t)		(sizeof(t)/sizeof(TCHAR))
#define TSIZEOF(t)		tsizeof(t)

#ifndef _ARRAYSIZE
#define _ARRAYSIZE(A)	_countof(A)
#endif	// #ifndef _ARRAYSIZE
#ifndef ARRAYSIZE
#define ARRAYSIZE(A)	_countof(A)
#else
#ifdef __cplusplus
#undef ARRAYSIZE
#define ARRAYSIZE(A)	_ARRAYSIZE(A)
#endif
#endif	// #ifndef ARRAYSIZE

#define TYPESIZE(A)		sizeof(A[0])


#define KILO	1024U
#define MEGA	1048576U	// (1024*KILO)
#define GIGA	1073741824U	// (1024*MEGA)
#define FLOPPY	1457664U


#define IS_ODD(n)		((n)&1)
#define IS_EVEN(n)		!((n)&1)

#define IN_RANGE(n,low,up)	((n)>=(low)&&(n)<=(up))

#define SUBSAT(val,sub)		(((int)(val)>(int)(sub))?((int)(val)-(int)(sub)):0)
#define SUBUSAT(val,sub)	(((unsigned)(val)>(unsigned)(sub))?((unsigned)(val)-(unsigned)(sub)):0)
#define SUBSATW(val,sub)	(((short)(val)>(short)(sub))?((short)(val)-(short)(sub)):0)
#define SUBUSATW(val,sub)	(((unsigned short)(val)>(unsigned short)(sub))?((unsigned short)(val)-(unsigned short)(sub)):0)
#define SUBSATB(val,sub)	(((char)(val)>(char)(sub))?((char)(val)-(char)(sub)):0)
#define SUBUSATB(val,sub)	(((unsigned char)(val)>(unsigned char)(sub))?((unsigned char)(val)-(unsigned char)(sub)):0)
#define SUBZSAT(a,b)		(((a)-(b))&(((a)<(b))-1))

#define SUBUINT64(d,s)		(*((const unsigned __int64*)&(d)) - *((const unsigned __int64*)&(s)))
#define SUBINT64(d,s)		(*((const __int64*)&(d)) -  *((const __int64*)&(s)))
#define SUBFTIME(ftd,fts)	SUBUINT64(ftd,fts)
#define SUBFTIMEMS(ftd,fts)	(SUBUINT64(ftd,fts)/10000)


#ifdef _WIN64
#define Int32x32ToSize(a,b)		Int32x32To64(a,b)
#define UInt32x32ToSize(a,b)	UInt32x32To64(a,b)
#else
#define Int32x32ToSize(a,b)		((__int32)((__int32)(a)*(__int32)(b)))
#define UInt32x32ToSize(a,b)	((unsigned __int32)((unsigned __int32)(a)*(unsigned __int32)(b)))
#endif


#ifndef CP_UTF16
#define CP_UTF16			1200
#endif
#ifndef CP_UTF16BE
#define CP_UTF16BE			1201
#endif
#ifndef CP_UTF32
#define CP_UTF32			12000
#endif
#ifndef CP_UTF32BE
#define CP_UTF32BE			12001
#endif

#define UTF16_BOM_WORD		0xFEFF
#define UTF16BE_BOM_WORD	0xFFFE
#define UTF32_BOM_DWORD		0x0000FEFF
#define UTF32BE_BOM_DWORD	0xFFFE0000
#define UTF8_BOM_W1			0xBBEF
#define UTF8_BOM_W2MSK		0x00BF
#define UTF8_BOM_3MSK		MAKEDWORD(UTF8_BOM_W1, UTF8_BOM_W2MSK)
#define UTF8_BOM_C1			0xEF
#define UTF8_BOM_C2			0xBB
#define UTF8_BOM_C3			0xBF


#define CHAR_BULLET			'\x95'
#define CHAR_NDASH			'\x96'
#define CHAR_MDASH			'\x97'
#define CHAR_TRADEMARK		'\x99'
#define CHAR_NBSP			'\xA0'
#define CHAR_COPYRIGHT		'\xA9'
#define CHAR_REGISTERED		'\xAE'
#define CHAR_DEGREE			'\xB0'
#define CHAR_PLUSMINUS		'\xB1'
#define CHAR_MULTIPLY		'\xD7'
#define CHAR_AE_UPPER		'\xC6'
#define CHAR_AE_LOWER		'\xE2'

#define WCHAR_BULLET		_A2W(CHAR_BULLET)
#define WCHAR_NDASH			_A2W(CHAR_NDASH)
#define WCHAR_MDASH			_A2W(CHAR_MDASH)
#define WCHAR_TRADEMARK		_A2W(CHAR_TRADEMARK)
#define WCHAR_NBSP			_A2W(CHAR_NBSP)
#define WCHAR_COPYRIGHT		_A2W(CHAR_COPYRIGHT)
#define WCHAR_REGISTERED	_A2W(CHAR_REGISTERED)
#define WCHAR_DEGREE		_A2W(CHAR_DEGREE)
#define WCHAR_PLUSMINUS		_A2W(CHAR_PLUSMINUS)
#define WCHAR_MULTIPLY		_A2W(CHAR_MULTIPLY)
#define WCHAR_AE_UPPER		_A2W(CHAR_AE_UPPER)
#define WCHAR_AE_LOWER		_A2W(CHAR_AE_LOWER)
#define WCHAR_ENQUAD		L'\x2000'
#define WCHAR_EMQUAD		L'\x2001'
#define WCHAR_ENSPACE		L'\x2002'
#define WCHAR_EMSPACE		L'\x2003'
#define WCHAR_FIGURESPACE	L'\x2007'	// non-breaking
#define WCHAR_THINSPACE		L'\x2009'
#define WCHAR_HAIRSPACE		L'\x200A'
#define WCHAR_ELLIPSIS		L'\x2026'
#define WCHAR_LEFTARROW		L'\x2190'
#define WCHAR_UPARROW		L'\x2191'
#define WCHAR_RIGHTARROW	L'\x2192'
#define WCHAR_DOWDARROW		L'\x2193'


#define LOWERCBIT			0x20
#define AUPPER(ch)			((ch) & (~LOWERCBIT))
#define TOUPPER_(ch)		AUPPER(ch)
#define ALOWER(ch)			((ch) | LOWERCBIT)
#define TOLOWER_(ch)		ALOWER(ch)


#define LITERALSLEN(s)		(ARRAYSIZE(s)-1)
#define CSLEN_(s)			LITERALSLEN(s)
#define cslen_(s)			LITERALSLEN(s)

#define NUMSWAP(a, b)		a+=b;b=a-b;a-=b


#define MATH_PI			3.14159265358979323846264338328
#define MATH_PI_F		3.14159265358979324f
#define MATH_PI_D2		1.57079632679489661923132169164		// _MATH_PI / 2
#define MATH_PI_D2_F	1.57079632679489662f				// _MATH_PI_F / 2
#define GOLDEN_RATIO	1.61803398875
#define GOLDEN_RATIO_F	1.61803398875f


#ifdef __cplusplus	// ++++++++++++++++++++++++++++++++++++++++++++++++

constexpr char operator "" _c(unsigned long long ull)
{
	return static_cast<char>(ull);
};
constexpr unsigned char operator "" _uc(unsigned long long ull)
{
	return static_cast<unsigned char>(ull);
};

constexpr intptr_t operator "" _ip(unsigned long long ull)
{
	return static_cast<intptr_t>(ull);
};
constexpr uintptr_t operator "" _uip(unsigned long long ull)
{
	return static_cast<uintptr_t>(ull);
};

constexpr double operator "" _d(unsigned long long flt)
{
	return static_cast<double>(flt);
};
constexpr float operator "" _f(unsigned long long flt)
{
	return static_cast<float>(flt);
};

#define DEFARG_(x)	= ##x


#else	// __cplusplus ++++++++++++++++++++++++++++++++++++++++++++++++

#define DEFARG_(x)

#endif	// __cplusplus ++++++++++++++++++++++++++++++++++++++++++++++++

// Message compiler ///////////////////////////////////////////////////


typedef enum CPP_ONLY(: unsigned __int32) {
	MC_SEVERITY_SUCCESS = 0,
	MC_SEVERITY_INFORMATIONAL,
	MC_SEVERITY_WARNING,
	MC_SEVERITY_ERROR
} MC_SEVERITY_CODE;

#define MC_MAKE_SEVERITY(sev)	(((sev)&0x3)<<30)
#define MC_GET_SEVERITY(dw)		((unsigned __int32)(dw)>>30)

#define MC_FACILITY_SYSTEM		0x0FF
#define MC_FACILITY_APPLICATION	0xFFF

#ifndef FACILITY_APPLICATION
#define FACILITY_APPLICATION	MC_FACILITY_APPLICATION
#endif

#define MC_MAKE_FACILITY(fac)	(((unsigned __int16)(fac)&0xFFF)<<16)
#define MC_GET_FACILITY(dw)		(((unsigned __int32)(dw)>>16)&0xFFF)

#define MC_MAKE_CODE(id)		(unsigned __int16)(w)
#define MC_GET_CODE(dw)			(unsigned __int16)(dw)

#define MC_MAKE_MSGID(sev,fac,code)	(MC_MAKE_SEVERITY(sev)|MC_MAKE_FACILITY(fac)|MC_MAKE_CODE(code))


///////////////////////////////////////////////////////////////////////


#pragma pack(push,2)
typedef union __ex_code_location_stamp64 {
	struct {
		unsigned __int16 line;
		unsigned __int8 file[6];
	};
	unsigned __int64 qpart;
} _excode_location;
#pragma pack(pop)

#define __FILELOFS__(i)	(_countof(__FILE__) - 1 - (i))
#define __FILELEN__		(__FILE__[__FILELOFS__(1)] ? __FILELOFS__(0) :	\
		(__FILE__[__FILELOFS__(2)] ? __FILELOFS__(1) :	\
		(__FILE__[__FILELOFS__(3)] ? __FILELOFS__(2) : __FILELOFS__(3))))

#define __FILELCIND__	(('.' == __FILE__[__FILELEN__ - 2]) ? (__FILELEN__ - 3)	\
						: (('.' == __FILE__[__FILELEN__ - 4]) ? (__FILELEN__ - 5)	\
						: (__FILELEN__ - 1)))
#define __FILELC__(i)	(__FILE__[__FILELCIND__ - (i)])

#define _DECLARE_CODESTAMP(name)	const _excode_location name = {	\
							{ (unsigned __int16)__LINE__,	\
							{ __FILELC__(5), __FILELC__(4), __FILELC__(3),	\
							__FILELC__(2), __FILELC__(1), __FILELC__(0)} } }


#define _PACK_IDATE(y, m, d)	((((unsigned long)(unsigned short)(y)) << 16)|((unsigned short)(unsigned char)(m) << 8)|(unsigned char)(d))
#define _IDATE_YEAR(dw)			(unsigned short)((dw) >> 16)
#define _IDATE_MONTH(dw)		(unsigned char)((dw) >> 8)
#define _IDATE_DAY(dw)			(unsigned char)(dw)
#define _IDATE_MONTHDAY(dw)		(unsigned short)(dw)

#define __IYEAR__	((((__DATE__[7]-'0') * 10 + (__DATE__[8]-'0')) * 10 \
					+ (__DATE__[9]-'0')) * 10 + (__DATE__[10]-'0'))
#define __IMONTH__	(__DATE__[2]=='n' ? (__DATE__[1]=='a' ? 1 : 6) \
					: __DATE__[2]=='b' ? 2 \
					: __DATE__[2]=='r' ? (__DATE__[1]=='a' ? 3 : 4) \
					: __DATE__[2]=='y' ? 5 \
					: __DATE__[2]=='l' ? 7 \
					: __DATE__[2]=='g' ? 8 \
					: __DATE__[2]=='p' ? 9 \
					: __DATE__[2]=='t' ? 10 \
					: __DATE__[2]=='v' ? 11 : 12)
#define __IDAY__		((__DATE__[4]==' ' ? 0 : __DATE__[4]-'0') * 10 + (__DATE__[5]-'0'))
#define __IMONTHDAY__	MAKEWORD(__IDAY__, __IMONTH__)
#define __IDATE__		_PACK_IDATE(__IYEAR__, __IMONTH__, __IDAY__)

#define __IHOUR__		((__TIME__[0]-'0')*10 + (__TIME__[1]-'0'))
#define __IMINUTE__		((__TIME__[3]-'0')*10 + (__TIME__[4]-'0'))
#define __ISECOND__		((__TIME__[6]-'0')*10 + (__TIME__[7]-'0'))
#define __ITIME__		((__IHOUR__ << 16)|(__IMINUTE__ << 8)|__ISECOND__)


#define MAKEDATEVERSION(major, minor)	MAKEULONGLONG(__IDATE__, MAKEULONG(minor, major))


#define __FILE_TITLE__		::InlineFindFileNameA(__FILE__, LITERALSLEN(__FILE__))
#define __WFILE_TITLE__		::InlineFindFileNameW(__WFILE__, LITERALSLEN(__WFILE__))
#ifdef _UNICODE
#define __TFILE_TITLE__		__WFILE_TITLE__
#else
#define __TFILE_TITLE__		__FILE_TITLE__
#endif


#include "exCTDExt.inl"

BEGIN_COMMON_NAMESPACE	///////////////////////////////////////////////

#ifndef NO_COMMON_EXTENSIONS
#include "exDbgExt.inl"
#include "exCmnExt.inl"
#ifdef __cplusplus
#include "exCmnTpl.inl"
#endif
#include "exSDKExt.inl"
#endif	// NO_COMMON_EXTENSIONS

END_COMMON_NAMESPACE	///////////////////////////////////////////////


#ifdef __cplusplus	// ++++++++++++++++++++++++++++++++++++++++++++++++

#define SETVBIT_(var, mask, set)	((var) = SetBit<decltype(var)>(var, (decltype(var))(mask), ToBool(set)))

#else	// __cplusplus ++++++++++++++++++++++++++++++++++++++++++++++++

#define SETVBIT_(var, mask, set)	((var) = ((set) ? ((var)|=(mask)) : ((var)&=~(mask))))

#endif	// __cplusplus ++++++++++++++++++++++++++++++++++++++++++++++++

#define SETFLAG_(var, flag, set)	SETVBIT_(var, flag, set)