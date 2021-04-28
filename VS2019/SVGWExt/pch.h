// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#pragma once
#ifndef PCH_H
#define PCH_H

#if defined(_DEBUG) && !defined(WCX_INCLUDE_TESTS)
#define WCX_INCLUDE_TESTS
#endif

#ifndef WCXENTRY
#define WCXENTRY		EXTERN_C __declspec(dllexport)
#endif

#include <SVGWCmn.h>
#include <olectl.h>
#include <thumbcache.h>
#include <propkey.h>

#endif //PCH_H
