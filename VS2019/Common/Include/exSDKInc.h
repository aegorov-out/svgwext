/*	exSDKInc.inl	- includes common Windows SDK headers

	Copyright (c) Alexandre Egorov

	This software is provided 'as-is', without any warranty.
	This software can be used and modified freely for any purpose,
	including commercial applications,
	as long as its origin is not misrepresented.
 */
#pragma once
#define _CMN_WINSDK_INCLUDED_


#if defined(WINSDK_21H2) || (WINSDK_VERSION >= 0x0A00000C)
// Build 19044
#define NTDDI_VERSION	0x0A00000C
#define _WIN32_WINNT	0x0A00
#define _WIN32_IE		0x0A00
#elif defined(WINSDK_CO) || defined(WINSDK_21H1) || (WINSDK_VERSION >= 0x0A00000B)
// Build 19043
#define NTDDI_VERSION	0x0A00000B
#define _WIN32_WINNT	0x0A00
#define _WIN32_IE		0x0A00
#elif defined(WINSDK_FE) || defined(WINSDK_20H2) || (WINSDK_VERSION >= 0x0A00000A)
// Build 19042
#define NTDDI_VERSION	0x0A00000A
#define _WIN32_WINNT	0x0A00
#define _WIN32_IE		0x0A00
#elif defined(WINSDK_MN) || defined(WINSDK_20H1) || defined(WINSDK_2004) || (WINSDK_VERSION >= 0x0A000009)
// Build 19041
#define NTDDI_VERSION	0x0A000009
#define _WIN32_WINNT	0x0A00
#define _WIN32_IE		0x0A00
#elif defined(WINSDK_VB) || defined(WINSDK_19H2) || defined(WINSDK_1909) || (WINSDK_VERSION >= 0x0A000008)
// Build 18363
#define NTDDI_VERSION	0x0A000008
#define _WIN32_WINNT	0x0A00
#define _WIN32_IE		0x0A00
#elif defined(WINSDK_19H1) || defined(WINSDK_1903) || (WINSDK_VERSION == 0x0A000007)
// Build 18362
#define NTDDI_VERSION	0x0A000007
#define _WIN32_WINNT	0x0A00
#define _WIN32_IE		0x0A00
#elif defined(WINSDK_10RS5) || defined(WINSDK_1809) || (WINSDK_VERSION == 0x0A000006)
// Build 17763
#define NTDDI_VERSION	0x0A000006
#define _WIN32_WINNT	0x0A00
#define _WIN32_IE		0x0A00
#elif defined(WINSDK_10RS4) || defined(WINSDK_1803) || (WINSDK_VERSION == 0x0A000005)
// Build 17134
#define NTDDI_VERSION	0x0A000005
#define _WIN32_WINNT	0x0A00
#define _WIN32_IE		0x0A00
#elif defined(WINSDK_10RS3) || defined(WINSDK_1709) || (WINSDK_VERSION == 0x0A000004)
// Build 16299
#define NTDDI_VERSION	0x0A000004
#define _WIN32_WINNT	0x0A00
#define _WIN32_IE		0x0A00
#elif defined(WINSDK_10RS2) || defined(WINSDK_1703) || (WINSDK_VERSION == 0x0A000003)
// Build 15063
#define NTDDI_VERSION	0x0A000003
#define _WIN32_WINNT	0x0A00
#define _WIN32_IE		0x0A00
#elif defined(WINSDK_10RS1) || defined(WINSDK_1607) || (WINSDK_VERSION == 0x0A000002)
// Build 14393
#define NTDDI_VERSION	0x0A000002
#define _WIN32_WINNT	0x0A00
#define _WIN32_IE		0x0A00
#elif defined(WINSDK_10TH2) || defined(WINSDK_1511) || (WINSDK_VERSION == 0x0A000001)
// Build 10586
#define NTDDI_VERSION	0x0A000001
#define _WIN32_WINNT	0x0A00
#define _WIN32_IE		0x0A00
#elif defined(WINSDK_WIN10) || defined(WINSDK_1507) || (WINSDK_VERSION == 0x0A000000)
// Build 10240
#define NTDDI_VERSION	0x0A000000
#define _WIN32_WINNT	0x0A00
#define _WIN32_IE		0x0A00
#elif defined(WINSDK_WINBLUE) || defined(WINSDK_WIN81) || (WINSDK_VERSION == 0x06030000)
// Version 6.3 Build >= 9600
#define NTDDI_VERSION	0x06030000
#define _WIN32_WINNT	0x0603
#define _WIN32_IE		0x0A00
#elif defined(WINSDK_WIN8) || (WINSDK_VERSION == 0x06020000)
// Version 6.2 Build >= 9200
#define NTDDI_VERSION	0x06020000
#define _WIN32_WINNT	0x0602
#define _WIN32_IE		0x0A00
#elif defined(WINSDK_WIN7) || (defined(WINSDK_VERSION) && (WINSDK_VERSION <= 0x06010000))
// Version 6.1 Build >= 7100
#define NTDDI_VERSION	0x06010000
#define _WIN32_WINNT	0x0601
#ifndef _WIN32_IE
#define _WIN32_IE		0x0800
#endif
#elif !defined(NTDDI_VERSION) && !defined(_WIN32_WINNT)
// default to Win10 RS2 (1703)
#define NTDDI_VERSION	0x0A000003
#define _WIN32_WINNT	0x0A00
#ifndef _WIN32_IE
#define _WIN32_IE		0x0A00
#endif
#endif

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NOMCX
#define NOIME
#define NOCOMM
#define NOKANJI
#define NOHELP
#define _CRT_SECURE_NO_WARNINGS
#define _SYS_GUID_OPERATORS_
#define _NO_SYS_GUID_OPERATOR_EQ_
#define __INLINE_ISEQUAL_GUID

#pragma warning(disable: 26454 28252 28253)
#include <windows.h>
#include <intrin.h>
#include <immintrin.h>
#include <intsafe.h>
#include <combaseapi.h>
#ifndef _CRT_FUNCTIONS_REQUIRED
#define _CRT_FUNCTIONS_REQUIRED		1
#endif
#include <wchar.h>
#include <math.h>
#pragma warning(disable: 4005 4668)
#include <stdint.h>
#pragma warning(default: 4005 4668)
#pragma warning(default: 26454 28252 28253)

#if !defined(WINSDK_VERSION) && defined(NTDDI_VERSION)
#define WINSDK_VERSION	NTDDI_VERSION
#endif
