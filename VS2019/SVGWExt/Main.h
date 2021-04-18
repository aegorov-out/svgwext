 // Main.h
#pragma once

#ifndef PCH_H
#include "pch.h"
#endif
#include "resource.h"

#ifdef _DEBUG
#ifndef WCX_SUPPORT_SELFREG
#define WCX_SUPPORT_SELFREG
#endif	// WCX_SUPPORT_SELFREG
#ifndef WCX_INCLUDE_TESTS
#define WCX_INCLUDE_TESTS
#endif	// WCX_INCLUDE_TESTS
#endif	// _DEBUG

#pragma warning(disable: 26451 28159)


///////////////////////////////////////////////////////////////////////
// Global definitions /////////////////////////////////////////////////


#define APPVER_MAJOR		1
#define APPVER_MINOR		0
#define APPVER_BUILD		__IYEAR__
#define APPVER_SPQFE		__IMONTHDAY__

#define APP_AUTHOR_NAME		L"Alexandre Egorov"


#ifdef __cplusplus	////////////////////////////////////////////////////////////////////////////
namespace RootNamespace {	// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
///////////////////////////// "C++" declarations ///////////////////////////////////////////////


extern volatile ULONG g_dllRefCount;
extern DWORD g_AppBuildDate;
extern MVERSION64 g_AppVersion;


// {0FF41E38-A793-45FF-9F38-5278DC444D57}
extern const CLSID DECLALIGN16 CLSID_SvgDecoder;
// {9DDB0D52-BDEE-4451-903D-27B29FA407B0}
extern const CLSID DECLALIGN16 CLSID_WmfEmfDecoder;
// {A3699533-14C8-4D18-80A4-3484B11EDC85}
extern const CLSID DECLALIGN16 CLSID_SvgThumbnailProvider;
// {ACDB5DAE-C8D0-4443-A86D-CE94E679670C}
extern const CLSID DECLALIGN16 CLSID_WmfEmfThumbnailProvider;

#define SVG_DECODER_NAMEA		"SVG decoder"
#define WMF_DECODER_NAMEA		"WMF/EMF decoder"


enum class ServComp : UINT8 {
	None = 0, SvgThumbProvider = 0x01, WmfThumbProvider = 0x02, SvgDecoder = 0x10, WmfDecoder = 0x20
};
constexpr UINT8 ServComp_Svg = (UINT8)ServComp::SvgDecoder | (UINT8)ServComp::SvgThumbProvider;
constexpr UINT8 ServComp_Wmf = (UINT8)ServComp::WmfDecoder | (UINT8)ServComp::WmfThumbProvider;
constexpr UINT8 ServComp_All = ServComp_Svg | ServComp_Wmf;


NOALIAS BOOL TryEnterGlobalCS();
NOALIAS void EnterGlobalCS();
NOALIAS void LeaveGlobalCS();

class GlobalCS
{
public:
	GlobalCS() { EnterGlobalCS(); }
	~GlobalCS() { LeaveGlobalCS(); }
};
#define GLOBAL_LOCK	GlobalCS _Global_CS_


#ifdef _WIN64
constexpr BOOLEAN IsOnWow64() { return FALSE; }
#else
_Check_return_ NOALIAS BOOLEAN IsOnWow64();
#endif


_Check_return_ NOALIAS HRESULT VerifyVersion();


_Check_return_ FORCEINLINE UINT32 DllGetRefCount()
{
	return g_dllRefCount;
}
FORCEINLINE UINT32 DllAddRef()
{
	return _InterlockedIncrement(&g_dllRefCount);
}
FORCEINLINE UINT32 DllRelease()
{
	return _InterlockedDecrement(&g_dllRefCount);
}


///////////////////////////////////////////////////////////////////////
// Initialization and resources ///////////////////////////////////////


HRESULT __fastcall HandleDXResult(HRESULT hr);


_Success_(return == S_OK) HRESULT __fastcall GetD2DFactory(_COM_Outptr_ ID2D1Factory6**ppFact);
_Success_(return == S_OK) HRESULT __fastcall GetD2DDevice(_COM_Outptr_ ID2D1Device5**ppDevice);

_Success_(return == S_OK)
HRESULT CreateGdiMetafile(_In_ IStream* pstm, _COM_Outptr_ ID2D1GdiMetafile** ppMetafile, _Out_ D2D_RECT_F* pBounds);

_Success_(return == S_OK)
HRESULT __fastcall CreateD2DC(_COM_Outptr_ ID2D1DeviceContext5** ppDC);


_Success_(return == S_OK) HRESULT __fastcall GetWicFactory(_COM_Outptr_ IWICImagingFactory2** ppFact);

_Success_(return == S_OK)
HRESULT CreateWicTarget(_In_ D2D_SIZE_U size, _COM_Outptr_ ID2D1RenderTarget** ppTarg, _COM_Outptr_ IWICBitmap** ppBm);


void ReleaseGlobalFactories();
void ReleaseXmlGlobals();


///////////////////////////////////////////////////////////////////////
// Image utilities ////////////////////////////////////////////////////


union UGZIPHEADER {
	struct {
		UINT8 ID1F;
		UINT8 ID8B;
		UINT8 Compression;
		UINT8 Flags;
	};
	UINT32 Part32;
};


union UMETAHEADER {
	struct {
		UINT16 WmfType;			// 0x0002
		UINT16 WmfHeaderSize;
		UINT16 WmfVersion;		// 0x0100 or 0300
	} Wmf;
	struct {
		UINT32 TypeSign;		// 0x9AC6CDD7 or EMR_HEADER
		UINT32 EmfHeaderSize;
		UINT32 EmfBounds[4];
		UINT32 EmfFrame[4];
		UINT32 EmfSignature;	// ENHMETA_SIGNATURE if TypeSign(version) is EMR_HEADER
	} WEmf;
};


_Success_(return == S_OK) NOALIAS
HRESULT InitLoadSvg(_In_ IStream* pstm, _COM_Outptr_result_nullonfailure_ ID2D1DeviceContext5** ppDC,
		_COM_Outptr_result_nullonfailure_ ID2D1SvgDocument** ppSvg, _Out_ D2D_SIZE_F* pSize);

_Success_(return == S_OK) NOALIAS inline
HRESULT CreateSvgCpuReadBitmap(_In_ ID2D1DeviceContext5* d2dDC, _In_ ID2D1SvgDocument* svgDoc, D2D_SIZE_F drawSize,
		D2D_SIZE_U targSize, _In_ const D2D1_MATRIX_3X2_F& transform, _COM_Outptr_result_nullonfailure_ ID2D1Bitmap1** ppbm)
{
	return wcSvgToCpuBitmap(d2dDC, svgDoc, drawSize, targSize, &transform, ppbm);
}

_Success_(return == S_OK) NOALIAS
HRESULT InitLoadWmf(_In_ IStream* pstm, _COM_Outptr_result_nullonfailure_ ID2D1DeviceContext5** ppDC,
		_COM_Outptr_result_nullonfailure_ ID2D1GdiMetafile** ppMetafile, _Out_ D2D_POINT_2F* pOrigin, _Out_ D2D_SIZE_F* pSize);

_Success_(return == S_OK) NOALIAS inline
HRESULT CreateWmfCpuReadBitmap(_In_ ID2D1DeviceContext5* d2dDC, _In_ ID2D1GdiMetafile* metafile, D2D_SIZE_U size,
		_In_opt_ const D2D1_POINT_2F& ptOrigin, _In_ const D2D1_MATRIX_3X2_F& transform,
		_COM_Outptr_result_nullonfailure_ ID2D1Bitmap1** ppbm)
{
	const D2D1_POINT_2F ptOffset = { -(ptOrigin.x), -(ptOrigin.y) };
	return ::wcWmfToCpuBitmap(d2dDC, metafile, size, &ptOffset, &transform, ppbm);
}


NOALIAS void CopyImageBits32(_Out_writes_bytes_(cbDstStride * size.height) PBYTE pDst, UINT cbDstStride,
		_In_reads_bytes_(cbSrcStride * size.height) PCBYTE pSrc, UINT cbSrcStride, D2D_SIZE_U size,
		WICBitmapTransformOptions transform = WICBitmapTransformRotate0);

NOALIAS PDWORD32 __fastcall PremulArgbToArgb(_Out_writes_all_(cdwCount) PDWORD32 argbDest,
		_In_reads_(cdwCount) const DWORD32* pargbSrc, UINT cdwCount);


///////////////////////////////////////////////////////////////////////
// Misc. helpers //////////////////////////////////////////////////////


D2D_POINT_2F GetDesktopDpi();
UINT GetScreenDpi();
NOALIAS BOOL IsCurrentDXAdapter();


NOALIAS bool __fastcall IsEqualGUID2(REFGUID guidCmp, REFGUID guid1, REFGUID guid2);
NOALIAS bool __fastcall IsEqualGUID3(REFGUID guidCmp, REFGUID guid1, REFGUID guid2, REFGUID guid3);
NOALIAS bool __fastcall IsEqualGUID4(REFGUID guidCmp, REFGUID guid1, REFGUID guid2, REFGUID guid3, REFGUID guid4);
NOALIAS bool __fastcall IsEqualGUID5(REFGUID guidCmp, REFGUID guid1,
		REFGUID guid2, REFGUID guid3, REFGUID guid4, REFGUID guid5);

NOALIAS HRESULT __cdecl QueryInterfaceImpl_(REFIID riid, _COM_Outptr_ void** ppvObject, UINT8 count, ...);
#define QI_ARG(IFace, pThis)	IID_##IFace, static_cast<IFace*>(pThis)

NOALIAS bool __fastcall IsRectValid(_In_ const WICRect& rc, UINT maxWidth, UINT maxHeight);


NOALIAS PCWCH FindInCSList(_In_reads_(cchList) PCWCH pwcList, _In_ UINT cchList,
		_In_reads_(cchVal) PCWCH pwcVal, _In_ UINT cchVal);
NOALIAS HRESULT CSListContains(_In_reads_(cchList) PCWCH pwcList, _In_ UINT cchList,
		_In_opt_ PCWSTR szVal, _Out_opt_ PBOOL pbContains);

NOALIAS HRESULT STDMETHODCALLTYPE ReturnInfoEmptyString(UINT cchBuff,
		__RPC__inout_ecount_full_opt(cchBuff) WCHAR* wcBuff, __RPC__out UINT* pcchActual);
NOALIAS HRESULT ReturnInfoRealString(_In_reads_(cchValue) PCWCH wcsValue, UINT cchValue,
		UINT cchBuff, __RPC__inout_ecount_full_opt(cchBuff) WCHAR* wcBuff, __RPC__out UINT* pcchActual);
NOALIAS HRESULT ReturnInfoVersion(_In_ UINT64 version64, UINT cchBuff,
		__RPC__inout_ecount_full_opt(cchBuff) WCHAR* wcBuff, __RPC__out UINT* pcchActual);


_Check_return_ NOALIAS HRESULT StreamSeekBack(_In_ IStream* pstm,
		_In_ STREAM_PARAM_CALLBACK pfnCallback, _Inout_opt_ void* cbkParam = nullptr);


#define DECLARE_IUNKNOWN	\
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void** ppvObject) override;	\
	ULONG STDMETHODCALLTYPE AddRef() override;	\
	ULONG STDMETHODCALLTYPE Release() override

#define ADDREF_IMPL		return _InterlockedIncrement(&m_cRef)
#define RELEASE_IMPL	\
	const ULONG cref = _InterlockedDecrement(&m_cRef);	\
	if (0 == cref) delete this;	\
	return cref


#pragma warning (default: 26495)

}	// namespace esmag	<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#endif	// #ifdef __cplusplus ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++