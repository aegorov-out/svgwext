/*	exDbgExt.inl	- common convenience debug extensions

	Copyright (c) Alexandre Egorov

	This software is provided 'as-is', without any warranty.
	This software can be used and modified freely for any purpose,
	including commercial applications,
	as long as its origin is not misrepresented.
 */
#pragma once
#define _CMN_DEBUG_EXT_INCLUDED_


#ifdef _DEBUG

#include <crtdbg.h>
#define IS_DEBUG	1
#ifndef DEBUG
#define DEBUG		_DEBUG
#endif
#ifndef ENABLE_CRT_TRACE
#define ENABLE_CRT_TRACE
#endif

#define DEBUGBREAK			__debugbreak()
#define DEBUG_DUMPMEMLEAKS	_CrtDumpMemoryLeaks()
#define DEBUG_HEAPINIT		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_CHECK_ALWAYS_DF|_CRTDBG_LEAK_CHECK_DF)
#define DEBUG_INITIALIZE	DEBUG_HEAPINIT;_CrtSetReportMode(_CRT_WARN,_CRTDBG_MODE_DEBUG)

#else	// _DEBUG

#ifdef ENABLE_CRT_TRACE
#include <crtdbg.h>
#endif

#define IS_DEBUG	0
#if defined _RELEASE && !defined RELEASE
#define RELEASE
#elif defined RELEASE && !defined _RELEASE
#define _RELEASE
#endif

#define DEBUGBREAK			((void)0)
#define DEBUG_DUMPMEMLEAKS	((void)0)
#define DEBUG_HEAPINIT		((void)0)
#define DEBUG_INITIALIZE	((void)0)

#endif	// _DEBUG


#ifdef __cplusplus
inline constexpr bool IsDebug() { return IS_DEBUG; }
#else
#define IsDebug()	IS_DEBUG
#endif


#ifdef ENABLE_CRT_TRACE

#define ASSERT(expr)			_ASSERT(expr)
#define ASSUME(expr)			ASSERT(expr)
#define ASSERT_DEBUG(expr)		ASSERT(expr)
#define ASSERTC(expr)			ASSERT_DEBUG(expr)
#define ASSERT_RESULT(expr,r)	ASSERT((expr)==(r))

#define VERIFY(expr)			ASSERT(expr)
#define VERIFY_EQUALS(expr,res)	VERIFY((expr)==(res))
#define VERIFY_RESULT(expr,res)	VERIFY_EQUALS(expr,res)
#define VERIFY_OK(expr)			VERIFY_RESULT(expr,S_OK)
#define VERIFY_SUCCEEDED(hr)	VERIFY(SUCCEEDED(hr))
#define OLE_VERIFY(hr)			VERIFY_SUCCEEDED(hr)
#define DEBUG_ONLY(expr)		expr
#define RELEASE_ONLY(expr)

#define TRACELN(msg)				_CrtDbgReport(_CRT_WARN,__FILE_TITLE__,__LINE__,NULL,msg##"\n")
#define TRACE(msg,...)				_CrtDbgReport(_CRT_WARN,__FILE_TITLE__,__LINE__,NULL,msg,__VA_ARGS__)
#define TRACE0(msg)					_CrtDbgReport(_CRT_WARN,__FILE_TITLE__,__LINE__,NULL,msg)
#define TRACE1(msg,p1)				_CrtDbgReport(_CRT_WARN,__FILE_TITLE__,__LINE__,NULL,msg,p1)
#define TRACE2(msg,p1,p2)			_CrtDbgReport(_CRT_WARN,__FILE_TITLE__,__LINE__,NULL,msg,p1,p2)
#define TRACE3(msg,p1,p2,p3)		_CrtDbgReport(_CRT_WARN,__FILE_TITLE__,__LINE__,NULL,msg,p1,p2,p3)
#define TRACE4(msg,p1,p2,p3,p4)		_CrtDbgReport(_CRT_WARN,__FILE_TITLE__,__LINE__,NULL,msg,p1,p2,p3,p4)
#define TRACER0(msg)				_CrtDbgReport(_CRT_ERROR,__FILE_TITLE__,__LINE__,NULL,msg)
#define TRACER1(msg,p1)				_CrtDbgReport(_CRT_ERROR,__FILE_TITLE__,__LINE__,NULL,msg,p1)
#define TRACER2(msg,p1,p2)			_CrtDbgReport(_CRT_ERROR,__FILE_TITLE__,__LINE__,NULL,msg,p1,p2)
#define TRACER3(msg,p1,p2,p3)		_CrtDbgReport(_CRT_ERROR,__FILE_TITLE__,__LINE__,NULL,msg,p1,p2,p3)
#define TRACER4(msg,p1,p2,p3,p4)	_CrtDbgReport(_CRT_ERROR,__FILE_TITLE__,__LINE__,NULL,msg,p1,p2,p3,p4)

#define TRACEW(msg,...)				_CrtDbgReportW(_CRT_WARN,__WFILE_TITLE__,__LINE__,NULL,msg,__VA_ARGS__)
#define TRACEW0(msg)				_CrtDbgReportW(_CRT_WARN,__WFILE_TITLE__,__LINE__,NULL,msg)
#define TRACEW1(msg,p1)				_CrtDbgReportW(_CRT_WARN,__WFILE_TITLE__,__LINE__,NULL,msg,p1)
#define TRACEW2(msg,p1,p2)			_CrtDbgReportW(_CRT_WARN,__WFILE_TITLE__,__LINE__,NULL,msg,p1,p2)
#define TRACEW3(msg,p1,p2,p3)		_CrtDbgReportW(_CRT_WARN,__WFILE_TITLE__,__LINE__,NULL,msg,p1,p2,p3)
#define TRACEW4(msg,p1,p2,p3,p4)	_CrtDbgReportW(_CRT_WARN,__WFILE_TITLE__,__LINE__,NULL,msg,p1,p2,p3,p4)
#define TRACERW0(msg)				_CrtDbgReportW(_CRT_ERROR,__WFILE_TITLE__,__LINE__,NULL,msg)
#define TRACERW1(msg,p1)			_CrtDbgReportW(_CRT_ERROR,__WFILE_TITLE__,__LINE__,NULL,msg,p1)
#define TRACERW2(msg,p1,p2)			_CrtDbgReportW(_CRT_ERROR,__WFILE_TITLE__,__LINE__,NULL,msg,p1,p2)
#define TRACERW3(msg,p1,p2,p3)		_CrtDbgReportW(_CRT_ERROR,__WFILE_TITLE__,__LINE__,NULL,msg,p1,p2,p3)
#define TRACERW4(msg,p1,p2,p3,p4)	_CrtDbgReportW(_CRT_ERROR,__WFILE_TITLE__,__LINE__,NULL,msg,p1,p2,p3,p4)

#define TRACELSEQ0(lntext)		{ static UINT _s_tracencnt = 0; TRACE1("%u.\t"##lntext##"\n", ++_s_tracencnt); }
#define TRACELSEQ(lntext,...)	{ static UINT _s_tracencnt = 0; TRACE("%u.\t"##lntext##"\n", ++_s_tracencnt,__VA_ARGS__); }

#define TRACEVAL(val)			TRACEW2(L"[" __WFUNCTION__ L"] : '"#val"' = %i (0x%X)\n", (int)(val), (unsigned)(val))
#define PRINTVAL(val)			printf("%s (%u) : [" __FUNCTION__ "] : '"#val"' = %i (0x%X)\n", __FILE_TITLE__, __LINE__, (int)(val), (unsigned)(val))
#define TRACEPOINT(pt)			TRACEW2(L"[" __WFUNCTION__ L"] : '"#pt"' = %i, %i\n", (int)(pt).x, (int)(pt).y)
#define TRACESIZE(size)			TRACEW2(L"[" __WFUNCTION__ L"] : '"#size"' = %i x %i\n", (int)(size).cx, (int)(size).cy)
#define TRACESIZEU(size)		TRACEW2(L"[" __WFUNCTION__ L"] : '"#size"' = %u x %u\n", (unsigned)(size).width, (unsigned)(size).height)
#define TRACERECT(rc)			TRACEW(L"[" __WFUNCTION__ L"] : '"#rc"' = %i, %i, %i, %i (%i x %i)\n",	\
								(int)(rc).left, (int)(rc).top, (int)(rc).right, (int)(rc).bottom,	\
								(int)((rc).right - (rc).left), (int)((rc).bottom - (rc).top))

#else	// ENABLE_CRT_TRACE
// release:

#define ASSERT(expr)
#define ASSUME(expr)			__assume(expr)
#define ASSERT_DEBUG(expr)		expr
#define ASSERTC(expr)			ASSERT_DEBUG(expr)
#define ASSERT_RESULT(expr,r)	expr

#define VERIFY(expr)			((void)(expr))
#define VERIFY_RESULT(expr,r)	((void)(expr))
#define VERIFY_OK(expr)			((void)(expr))
#define VERIFY_SUCCEEDED(hr)	((void)(hr))
#define DEBUG_ONLY(expr)
#define RELEASE_ONLY(expr)		expr

#define TRACELN(msg)				((void)0)
#define TRACE(msg,...)				((void)0)
#define TRACE0(msg)					((void)0)
#define TRACE1(msg,p1)				((void)0)
#define TRACE2(msg,p1,p2)			((void)0)
#define TRACE3(msg,p1,p2,p3)		((void)0)
#define TRACE4(msg,p1,p2,p3,p4)		((void)0)
#define TRACER0(msg)				((void)0)
#define TRACER1(msg,p1)				((void)0)
#define TRACER2(msg,p1,p2)			((void)0)
#define TRACER3(msg,p1,p2,p3)		((void)0)
#define TRACER4(msg,p1,p2,p3,p4)	((void)0)

#define TRACEW(msg,...)				((void)0)
#define TRACEW0(msg)				((void)0)
#define TRACEW1(msg,p1)				((void)0)
#define TRACEW2(msg,p1,p2)			((void)0)
#define TRACEW3(msg,p1,p2,p3)		((void)0)
#define TRACEW4(msg,p1,p2,p3,p4)	((void)0)
#define TRACERW0(msg)				((void)0)
#define TRACERW1(msg,p1)			((void)0)
#define TRACERW2(msg,p1,p2)			((void)0)
#define TRACERW3(msg,p1,p2,p3)		((void)0)
#define TRACERW4(msg,p1,p2,p3,p4)	((void)0)

#define TRACELSEQ0(lntext)			((void)0)
#define TRACELSEQ(lntext,...)		((void)0)

#define TRACEVAL(e)					((void)0)
#define PRINTVAL(e)					((void)0)
#define TRACEPOINT(pt)				((void)0)
#define TRACESIZE(size)				((void)0)
#define TRACESIZEU(size)			((void)0)
#define TRACERECT(rc)				((void)0)

#endif	// ENABLE_CRT_TRACE

#define TRACEPT(pt)				TRACEPOINT(pt)
