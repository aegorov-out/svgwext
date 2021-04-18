//	exRCExt.h	- resource compiler extra types
//	Copyright (c) Alexandre Egorov
#pragma once
#define _CMN_RCEXT_INCLUDED_


#if defined(APSTUDIO_INVOKED) || defined(APSTUDIO_READONLY_SYMBOLS)

#define RT_JPEG	74	// 0x004A
#define RT_PNG	80	// 0x0050
#define RT_XML	88	// 0x0058

#elif defined(MAKEINTRESOURCE) && !defined(_RT_X_DEFINED_)
#define _RT_X_DEFINED_

#define RT_JPEG	MAKEINTRESOURCE('J')	// 74	0x004A
#define RT_PNG	MAKEINTRESOURCE('P')	// 80	0x0050
#define RT_XML	MAKEINTRESOURCE('X')	// 88	0x0058

#endif	// defined(MAKEINTRESOURCE) && !defined(_RT_X_DEFINED_)
