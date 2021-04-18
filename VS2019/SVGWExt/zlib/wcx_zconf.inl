// Zconf.inl
#pragma once

#ifndef NTDDI_VERSION
#define NTDDI_VERSION	0x0A000003	// NTDDI_WIN10_RS2
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT	0x0A00		// _WIN32_WINNT_WIN10
#endif
#define _WIN32_IE		0x0A00		// _WIN32_IE_IE100

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>

#define ZLIB_WINAPI
#define ZLIB_CONST	const
#define NO_DEFLATE
#define NO_GZCOMPRESS
#define BUILDFIXED
#ifdef _DEBUG
#define ZLIB_DEBUG
#endif

#pragma warning(disable: 4267 4996 6297 26451 28278)