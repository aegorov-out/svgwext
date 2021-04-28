// SVGWCmn.h
#pragma once
#define _SVGWCMN_H_INCLUDED_

#ifndef WINSDK_VERSION
#define WINSDK_VERSION	0x0A000003	// NTDDI_WIN10_RS2
#endif
#ifndef D3D11_NO_HELPERS
#define D3D11_NO_HELPERS
#endif

#define RootNamespace	wcx
#ifndef CommonNamespace
#define CommonNamespace	RootNamespace
#endif
#define CMN				CommonNamespace

#include "exCmnInc.h"
#include <d3d11_3.h>
#include <dxgi1_4.h>
#include <d2d1_3.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <wincodec.h>

#define WCX_SUPPORT_SELFREG
#ifdef SVGWEXT_EXPORTS
#define WCXENTRY			EXTERN_C __declspec(dllexport)
#else
#define WCXENTRY			EXTERN_C __declspec(dllimport)
#endif
#define WCXAPI_(type)		WCXENTRY type
#define WCXAPI				WCXAPI_(HRESULT)
#define WCXSTDAPI_(type)	WCXAPI_(type) __stdcall
#define WCXSTDAPI			WCXAPI __stdcall
#define WCXFASTAPI_(type)	WCXAPI_(type) __fastcall
#define WCXFASTAPI			WCXAPI __fastcall
#define WCXCAPI_(type)		WCXAPI_(type) __cdecl
#define WCXCAPI				WCXCAPI_(HRESULT)


EXTERN_C HMODULE g_hModule;


#ifndef GUIDSTRING_MAX
#define GUIDSTRING_MAX		(1 + 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)  // "{12345678-1234-1234-1234-123456789012}"
#endif
#define MAX_GUID_STRSIZE	ALIGN4(GUIDSTRING_MAX + 1)


#define D2D_VERSION_MAJOR	1
#define D2D_VERSION_MINOR	3


#define SVG_WIC_EXTENSIONS		L".svg,.svgz"
#define SVG_MIME_TYPE			L"image/svg+xml"
#define SVG_WIC_MIME_TYPES		SVG_MIME_TYPE

#define WMFEMF_WIC_EXTENSIONS	L".emf,.wmf,.emz,.wmz"
#define WMF_MIME_TYPE			L"image/x-wmf"
#define EMF_MIME_TYPE			L"image/x-emf"
#define WMFEMF_WIC_MIME_TYPES	WMF_MIME_TYPE L"," EMF_MIME_TYPE


#ifdef __cplusplus	// ++++++++++++++++++++++++++++++++++++++++++++++++
namespace RootNamespace {	// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#include "exDXExt.inl"
#include "SVGWHelpers.inl"
#endif


#ifdef __cplusplus
constexpr UINT MAX_PICTURE_FILE_SIZE =	(1024 * 1024 * 128 - 1);	// 0x07FFFFFF
#else
#define MAX_PICTURE_FILE_SIZE			(1024 * 1024 * 128 - 1)		// 0x07FFFFFF
#endif

typedef _Ret_opt_ _Post_writable_byte_size_(cb)  __drv_allocatesMem(Mem) _Check_return_
PVOID (WINAPI* MEMALLOC_T)(_In_ SIZE_T cb);
typedef void (WINAPI* MEMFREE_T)(_Frees_ptr_opt_ PVOID pv);

typedef HRESULT (CALLBACK* STREAM_PARAM_CALLBACK)(IStream*, UINT64, void*);


#ifdef __cplusplus
///////////////////////////////////////////////////////////////////////
// C++ definitions ++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef union UWicRect
{
	WICRect Rect;
	struct {
		POINT lptLT;
		SIZE lSize;
	};
	struct {
		D2D_POINT_2U uptLT;
		D2D_SIZE_U uSize;
	};
	struct {
		UINT64 lt64;
		UINT64 size64;
	};
} * PUWicRect;


}	// namespace <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#if (RootNamespace != CommonNamespace)
using namespace CommonNamespace;
#endif
using namespace RootNamespace;
#endif	// __cplusplus ++++++++++++++++++++++++++++++++++++++++++++++++


WCXSTDAPI wcDllInstall(BOOL bInstall, _In_reads_opt_(cArgs) const PCWSTR* rgszArgs, int cArgs);

#ifdef WCX_INCLUDE_TESTS

enum CPP_ONLY(: UINT) {
	DLLT_NULL = 0, LLT_NONE = DLLT_NULL, DLLT_THUMB_HBITMAP, DLLT_THUMB_WICSOURCE,
	DLLT_DECODER_WICFRAME, DLLT_DECODER_WICTRANSFORM, DLLT_WICDECODER,
	DLLT_PROPERTY_STORE, DLLT_UNGZIP_STREAM, DLLT_UNGZIP_FILE
};
WCXSTDAPI wcRunDllTest(_In_opt_ HWND hWnd, UINT idMsg, _In_opt_ PCWSTR szCmdLine, LPARAM lParam DEFARG_(0));

#endif	// WCX_INCLUDE_TESTS


///////////////////////////////////////////////////////////////////////
// Common globals /////////////////////////////////////////////////////


_Success_(return == S_OK)
WCXFASTAPI wcGetReadOnlyBindCtx(_COM_Outptr_result_nullonfailure_ IBindCtx** ppBC);
_Success_(return == S_OK)
WCXFASTAPI wcGetReadWriteBindCtx(_COM_Outptr_result_nullonfailure_ IBindCtx** ppBC);

WCXSTDAPI_(void) wcReleaseComShared();


///////////////////////////////////////////////////////////////////////
// Misc. //////////////////////////////////////////////////////////////


WCXFASTAPI_(D2D_SIZE_U) wcToImageSizeU(D2D_SIZE_F sizeF);

WCXSTDAPI_(UINT) wcNum32ToStr(UINT32 iVal, bool_t isSigned,
		_Out_writes_to_(cchMax, return-1) PWSTR szNum, int cchMax, UINT8 radix DEFARG_(10));
INLINE UINT wcUInt32ToDec(UINT32 iVal, _Out_writes_to_(cchMax, return-1) PWSTR szNum, int cchMax) {
	return wcNum32ToStr(iVal, false, szNum, cchMax, 10);
}
INLINE UINT wcInt32ToDec(INT32 iVal, _Out_writes_to_(cchMax, return-1) PWSTR szNum, int cchMax) {
	return wcNum32ToStr(iVal, true, szNum, cchMax, 10);
}

WCXFASTAPI_(UINT) wcAsciiToWide(_In_opt_ PCSTR szAscii, _Out_writes_to_(cchBuff, return + 1) PWSTR pwcBuff, int cchBuff);

WCXFASTAPI_(bool_t) wcAsciiIsEqual(_In_opt_ PCWSTR szWide, _In_opt_ PCSTR szAscii);
WCXCAPI_(bool_t) wcAsciiIsAnyEqual(_In_opt_ PCWSTR szwCmp, UINT cArgs, ...);
WCXFASTAPI_(int) wcCompareString(_In_NLS_string_opt_(cwch) PCWCH pwc1,
		_In_NLS_string_opt_(cwch) PCWCH pwc2, int cwch DEFARG_(-1));

WCXFASTAPI_(PDWORD32) wcSetMemory32(_Out_writes_all_(cdw) PDWORD32 dest, _In_ DWORD32 val, _In_ SIZE_T cdw);

WCXSTDAPI_(D2D_SIZE_F) wcScaleSizeF(_In_ D2D_SIZE_F size, _In_opt_ UINT maxSide);
INLINE SIZE wcScaleSizeL(_In_ SIZE size, _In_opt_ UINT maxSide)
{
	const D2D_SIZE_F sizeF = wcScaleSizeF({ (FLOAT)size.cx, (FLOAT)size.cy }, maxSide);
	return SIZE{ CPP_COMMONS(ftoi32f(sizeF.width)), CPP_COMMONS(ftoi32f(sizeF.height)) };
}

WCXFASTAPI_(UINT32) wcPackStdExtension4(_In_reads_(cch) PCWCH pwcExt, const UINT cch);

WCXFASTAPI_(BOOL) wcSwitchThread(_In_opt_ UINT msSleep DEFARG_(USER_TIMER_MINIMUM));

_Success_(return == S_OK) WCXSTDAPI
wcTryUncompressStream(_In_ IStream* pstmIn, BOOL headersOnly, _COM_Outptr_result_nullonfailure_ IStream** ppstmOut);


///////////////////////////////////////////////////////////////////////
// DX & WIC ///////////////////////////////////////////////////////////


#ifndef D3DDDIERR_DEVICEREMOVED
#define D3DDDIERR_DEVICEREMOVED		((1 << 31) | (0x876 << 16) | 2160)
#endif

#define MAX_DX_BITMAP_SIDE	D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION


#define REGPATH_WIC_DECODERS	L"CLSID\\{7ED96837-96F0-4812-B211-F13C24117ED3}\\Instance"
#define REGPATH_WIC_ENCODERS	L"CLSID\\{AC757296-3522-4E11-9862-C17BE5A1767E}\\Instance"


// {AE97E52A-51BC-41FB-871A-FC9B86FB20A2}
WCXENTRY const DECLALIGN16 GUID GUID_AE_VendorID;

//	{28EA8895-834D-4941-8794-7EE5B5A05356}
WCXENTRY const DECLALIGN16 GUID GUID_AE_ContainerFormatSvg;
//	{C1D289A3-A846-47C7-9F94-E5E8F6AC4D46}
WCXENTRY const DECLALIGN16 GUID GUID_AE_ContainerFormatWmfEmf;


#ifdef __cplusplus
inline constexpr
#else
__inline
#endif
D2D_SIZE_F InitialSvgSize()
{
	const UINT64 ui64 = 0x437F0000437F0000ull;
	return *(D2D_SIZE_F*)&ui64;
}


_Check_return_ _Success_(return == S_OK)
WCXFASTAPI wcCreateWicFactory(_COM_Outptr_result_nullonfailure_ IWICImagingFactory2** ppWicFact);

_Check_return_ _Success_(return == S_OK)
WCXSTDAPI wcCreateD2DFactory(BOOL multiThreaded, _COM_Outptr_result_nullonfailure_ ID2D1Factory6** ppD2D1Fact);

_Check_return_ _Success_(return == S_OK)
WCXSTDAPI wcCreateDXDevices(BOOL multiThreaded, _COM_Outptr_result_nullonfailure_ ID3D11Device** ppD3D11Device,
			_COM_Outptr_result_nullonfailure_ IDXGIDevice1** ppDXGIDevice);

_Check_return_ WCXFASTAPI_(LONG) wcIsDXRecreateError(HRESULT hr);


_Check_return_ WCXFASTAPI wcMatchesSvgPattern(_In_ IStream* pstm, _Out_opt_ bool_t* pbGzip DEFARG_(nullptr));
//_Check_return_ WCXFASTAPI wcUrlMatchesSvgPattern(_In_ PCWSTR szUrl);
_Check_return_ WCXSTDAPI wcUpdateSvgSize(_In_ ID2D1SvgDocument* svgDoc, bool_t removeSize, _Out_ D2D_SIZE_F* pSize);

_Check_return_ WCXFASTAPI wcMatchesWmfEmfPattern(_In_ IStream* pstm, _Out_opt_ bool_t* pbGzip DEFARG_(nullptr));

_Check_return_ WCXFASTAPI wcMatchesGzipPattern(_In_ IStream* pstm, _Out_opt_ PUINT32 pcbOutSize DEFARG_(nullptr));


WCXFASTAPI wcCreateSvgThumbProvider(_In_ REFIID riid, _COM_Outptr_result_nullonfailure_ void** ppvObject);
WCXFASTAPI wcCreateWmfEmfThumbProvider(_In_ REFIID riid, _COM_Outptr_result_nullonfailure_ void** ppvObject);
WCXFASTAPI wcCreateSvgComponent(_In_ REFIID riid, _COM_Outptr_result_nullonfailure_ void** ppvObject);
WCXFASTAPI wcCreateWmfEmfComponent(_In_ REFIID riid, _COM_Outptr_result_nullonfailure_ void** ppvObject);


_Success_(return != 0) WCXFASTAPI_(HBITMAP) wcCreateDIB32(SIZE size, _Outptr_opt_ PDWORD32* ppBits);
_Success_(return == S_OK) WCXFASTAPI wcD2DBitmapToDIB(_In_ ID2D1Bitmap1* d2dBm, _Out_ HBITMAP* phbmOut);

_Success_(return == S_OK)
WCXSTDAPI wcSvgToCpuBitmap(_In_opt_ ID2D1DeviceContext5* d2dDC, _In_ ID2D1SvgDocument* svgDoc, D2D_SIZE_F drawSize,
		D2D_SIZE_U targSize, _In_opt_ const D2D1_MATRIX_3X2_F* transform,
		_COM_Outptr_result_nullonfailure_ ID2D1Bitmap1** ppbm);
_Success_(return == S_OK)
WCXSTDAPI wcWmfToCpuBitmap(_In_opt_ ID2D1DeviceContext5* d2dDC, _In_ ID2D1GdiMetafile* metafile, D2D_SIZE_U size,
		_In_opt_ const D2D1_POINT_2F *targOffset, _In_opt_ const D2D1_MATRIX_3X2_F* transform,
		_COM_Outptr_result_nullonfailure_ ID2D1Bitmap1**ppbm);

WCXSTDAPI_(ID2D1SvgElement*) wcFindSvgElemChild(_In_ ID2D1SvgElement* pelem, _In_reads_(cchTag) PCWCH pwcTagName, UINT cchTag);
_Success_(return == S_OK)
WCXSTDAPI wcGetSvgElemText(_In_ ID2D1SvgElement* pelem, _Outptr_result_nullonfailure_ PWSTR* ppszText);
_Success_(return == S_OK)
WCXSTDAPI wcGetSvgChildText(_In_ ID2D1SvgElement* pelem, _In_reads_(cchTag) PCWCH pwcTagName,
		UINT cchTag, _Outptr_result_nullonfailure_ PWSTR* ppszText);
_Success_(return == S_OK)
INLINE HRESULT wcGetSvgRootTitle(_In_ ID2D1SvgElement* proot, _Outptr_result_nullonfailure_ PWSTR* ppszText)
{
	const union {
		struct {
			UINT64 name64;
			UINT32 name32;
		} DUMMYSTRUCTNAME;
		WCHAR wcsName[6];
	} uTag = { { MAKEULONGLONGW(L't', L'i', L't', L'l'), (UINT32)L'e' } };
	return wcGetSvgChildText(proot, uTag.wcsName, 5, ppszText);
}
_Success_(return == S_OK)
INLINE HRESULT wcGetSvgRootDesc(_In_ ID2D1SvgElement* proot, _Outptr_result_nullonfailure_ PWSTR* ppszText)
{
	const union {
		UINT64 name64;
		WCHAR wcsName[4];
	} uTag = { MAKEULONGLONGW(L'd', L'e', L's', L'c') };
	return wcGetSvgChildText(proot, uTag.wcsName, 4, ppszText);
}


///////////////////////////////////////////////////////////////////////
// Paths & files //////////////////////////////////////////////////////


WCXFASTAPI_(PWSTR) wcFindFileName(_In_NLS_string_opt_(pathLen) PCWCH pathName, int pathLen DEFARG_(-1));

_Success_(return > 0)
WCXSTDAPI_(UINT) wcGetModuleName(_In_opt_ HMODULE hModule,
	_Out_writes_to_(cchMax, return+1) PWSTR szFileName, UINT cchMax);

_Check_return_ _Success_(return == S_OK)
WCXFASTAPI wcCreateStreamOnFile(_In_opt_ PCWSTR szFileName, BOOL write, _COM_Outptr_result_nullonfailure_ IStream** ppstm);

_Check_return_ _Success_(return == S_OK)
WCXSTDAPI wcCreateStreamOnItem(_In_ IShellItem* psi, BOOL write, _COM_Outptr_result_nullonfailure_ IStream** ppstm);

_Check_return_ _Success_(return == S_OK)
WCXFASTAPI wcCreateTempFileStream(_In_opt_ SIZE_T cbInitSize, _COM_Outptr_result_nullonfailure_ IStream** ppstm);
_Check_return_ _Success_(return == S_OK)
WCXFASTAPI wcCreateTempStream(_In_opt_ SIZE_T cbInitSize, _COM_Outptr_result_nullonfailure_ IStream** ppstm);
_Check_return_ _Success_(return == S_OK)
WCXFASTAPI wcCopyTempStream(_In_ IStream* pstmIn, _In_opt_ UINT64 cbCopy, _COM_Outptr_result_nullonfailure_ IStream** ppstmOut);


///////////////////////////////////////////////////////////////////////
// Resources //////////////////////////////////////////////////////////


_Success_(return != 0) WCXSTDAPI_(PCVOID)
wcGetModuleResource(_In_opt_ HMODULE hModule, _In_ PCWSTR resType, _In_ UINT wID, _Out_opt_ PDWORD pcbSize);
INLINE PCVOID wcGetResource(_In_ PCWSTR resType, _In_ UINT wID, _Out_opt_ PDWORD pcbSize)
{
	return wcGetModuleResource(g_hModule, resType, wID, pcbSize);
}

_Success_(return != NULL) WCXSTDAPI_(const VS_FIXEDFILEINFO*)
wcGetModuleVersionResource(_In_ HMODULE hModule);
INLINE const VS_FIXEDFILEINFO* GetVersionResource()
{
	return wcGetModuleVersionResource(g_hModule);
}


_Success_(return != 0)
WCXSTDAPI_(UINT64) wcVerifyModuleVersionResource(_In_ HMODULE hModule, _In_ UINT64 appBuildDateVer);
INLINE UINT64 wcVerifyVersionResource(_In_ UINT64 appBuildDateVer)
{
	return wcVerifyModuleVersionResource(g_hModule, appBuildDateVer);
}
#define QueryVersionResource(major, minor)	wcVerifyVersionResource(MAKEDATEVERSION(major, minor))

_Success_(return.QuadPart != 0)
WCXFASTAPI_(ULARGE_INTEGER) wcGetModuleChecksums(_In_opt_ HMODULE hModule);


_Success_(return > 0)
WCXSTDAPI_(UINT) wsFormatVersion(_In_ UINT64 ver64, _Out_writes_to_(cchMax, return+1) PWSTR szBuff, _In_ int cchMax);


///////////////////////////////////////////////////////////////////////
// Logging ////////////////////////////////////////////////////////////


#ifdef WCX_ENABLE_LOG

WCXFASTAPI wcLogOpen(_In_opt_ PCWSTR szFileName);

WCXFASTAPI wcLogWrite(_In_opt_ PCSTR szText);
WCXFASTAPI wcLogWriteW(_In_opt_ PCWSTR szText);
WCXFASTAPI wcLogWriteLn(_In_opt_ PCSTR szText);
WCXFASTAPI wcLogWriteLnW(_In_opt_ PCWSTR szText);
WCXCAPI wcLogFormat(_In_opt_ PCSTR szFmt, ...);
WCXFASTAPI wcLogFormatV(_In_opt_ PCSTR szFmt, va_list Args);
WCXCAPI wcLogFormatLn(_In_opt_ PCSTR szFmt, ...);
WCXFASTAPI wcLogFormatLnV(_In_opt_ PCSTR szFmt, va_list Args);
WCXCAPI wcLogFormatStat(_In_opt_ IStream* pstm, _In_opt_ PCSTR szFmt, ...);

#else	// WCX_ENABLE_LOG

CONSTEXPR HRESULT wcLogOpen(_In_opt_ PCWSTR szFileName) { return E_NOTIMPL; }

CONSTEXPR HRESULT wcLogWrite(_In_opt_ PCSTR szText) { return E_NOTIMPL; }
CONSTEXPR HRESULT wcLogWriteW(_In_opt_ PCWSTR szText) { return E_NOTIMPL; }
CONSTEXPR HRESULT wcLogWriteLn(_In_opt_ PCSTR szText) { return E_NOTIMPL; }
CONSTEXPR HRESULT wcLogWriteLnW(_In_opt_ PCWSTR szText) { return E_NOTIMPL; }
CONSTEXPR HRESULT wcLogFormat(_In_opt_ PCSTR szFmt, ...) { return E_NOTIMPL; }
CONSTEXPR HRESULT wcLogFormatV(_In_opt_ PCSTR szFmt, va_list Args) { return E_NOTIMPL; }
CONSTEXPR HRESULT wcLogFormatLn(_In_opt_ PCSTR szFmt, ...) { return E_NOTIMPL; }
CONSTEXPR HRESULT wcLogFormatLnV(_In_opt_ PCSTR szFmt, va_list Args) { return E_NOTIMPL; }
CONSTEXPR HRESULT wcLogFormatStat(_In_opt_ IStream* pstm, _In_opt_ PCSTR szFmt, ...) { return E_NOTIMPL; }

#endif	// WCX_ENABLE_LOG


///////////////////////////////////////////////////////////////////////
// Error handling /////////////////////////////////////////////////////


_Check_return_ _Post_satisfies_(return != 0 || return == defCode)
WCXFASTAPI_(LSTATUS) wcLastErrorStatus(_In_ LSTATUS defCode = E_FAIL);
#define LSTATUS_WIN_ERROR			wcLastErrorStatus(E_FAIL)
#define LSTATUS_WIN_ERROR_(defCode)	wcLastErrorStatus((LSTATUS)(defCode))

_Check_return_ _Post_satisfies_(return < 0 || return == defCode) _Translates_Win32_to_HRESULT_(code)
WCXFASTAPI wcEnsureFailHRESULT(LONG code, HRESULT defCode = E_FAIL);
#define FAILED_HRESULT(code)	wcEnsureFailHRESULT((LONG)(code), E_FAIL)

_Check_return_ _Post_satisfies_(return < 0 || return == defCode)
WCXFASTAPI wcLastErrorHRESULT(_In_ HRESULT defCode = E_FAIL);
#define HRESULT_WIN_ERROR				wcLastErrorHRESULT(E_FAIL)
#define HRESULT_WIN_ERROR_(defCode)		wcLastErrorHRESULT((HRESULT)(defCode))

_Post_satisfies_(return != 0 || return == defCode)
WCXFASTAPI_(DWORD) wcEnsureLastError(_In_ DWORD defCode = (DWORD)E_FAIL);


