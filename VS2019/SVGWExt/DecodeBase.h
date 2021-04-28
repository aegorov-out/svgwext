 // DecodeBase.h
#pragma once

#include "Main.h"


#ifdef __cplusplus	///////////////////////////////////////////////////
namespace RootNamespace {	// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// "C++" declarations /////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Base classes ///////////////////////////////////////////////////////


#pragma warning (disable: 26495)


class NOVTABLE DecoderBase abstract : public IWICBitmapDecoder,
		public IWICBitmapFrameDecode, public IWICBitmapSourceTransform
{
	IMPL_UNCOPYABLE(DecoderBase)

protected:
	volatile ULONG m_cRef;
	UINT m_screenDpi;
	ID2D1DeviceContext5* m_d2dDC;
	D2D_SIZE_F m_sizeDips;
	IWICBitmap* m_wicBitmap;
	IStream* m_pstmInit;
	mutable CRITICAL_SECTION m_CS;

protected:
	DecoderBase(_In_ ULONG cRef) : m_cRef(cRef) { DllAddRef(); wcInitCommonCS(&m_CS); }
	virtual ~DecoderBase()
	{
		if (m_wicBitmap)
			m_wicBitmap->Release();
		if (m_d2dDC)
			m_d2dDC->Release();
		if (m_pstmInit)
			m_pstmInit->Release();
		::DeleteCriticalSection(&m_CS);
		DllRelease();
	}

	void EnterCS() const { ::EnterCriticalSection(&m_CS); }
	BOOL TryEnterCS() const { return ::TryEnterCriticalSection(&m_CS); }
	void LeaveCS() const { ::LeaveCriticalSection(&m_CS); }

	UINT GetMaxBitmapSide() const { return (m_d2dDC ? m_d2dDC->GetMaximumBitmapSize() : MAX_DX_BITMAP_SIDE); }
	_Success_(return == S_OK) HRESULT EnsureWicBitmap(_COM_Outptr_opt_result_nullonfailure_ IWICBitmap** ppBitmap = nullptr);
	IWICBitmap* GetWicBitmap() const { return m_wicBitmap; }
	D2D_SIZE_F GetDipSize() const { return m_sizeDips; }
	D2D_SIZE_U GetPixelSize() const;
	D2D_SIZE_F CalcThumbSize() const;

	_Success_(return == S_OK) HRESULT CreateBitmapSource(D2D_SIZE_F size,
		_In_ const D2D1_MATRIX_3X2_F& transform, _COM_Outptr_ IWICBitmap** ppIBitmap);
	_Success_(return == S_OK) HRESULT CreateCpuReadBitmap(D2D_SIZE_F drawSize, D2D_SIZE_U targSize,
		_In_ const D2D1_MATRIX_3X2_F& transform, _COM_Outptr_ ID2D1Bitmap1** ppbmMap);
	_Success_(return == S_OK) HRESULT CreateThumbnail(D2D_SIZE_F size, _In_ const D2D1_MATRIX_3X2_F& transform,
		__RPC__deref_out_opt IWICBitmapSource** ppIThumbnail);
	HRESULT __fastcall DoLoad(_In_ IStream* pstm);
	HRESULT Reload(HRESULT hrRecreate);

	virtual HRESULT __fastcall InitLoad(_In_ IStream* pstm) = 0;
	virtual HRESULT QueryCopyPixelTransform_(D2D_SIZE_U reqSize, WICBitmapTransformOptions wicTransform,
		UINT angleRotated, _Inout_ D2D_MATRIX_3X2_F* pMatrix) const = 0;
	_Success_(return == S_OK)
	virtual HRESULT CreateCpuReadBitmap_(D2D_SIZE_F drawSize, D2D_SIZE_U targSize,
		_In_ const D2D1_MATRIX_3X2_F& transform, _COM_Outptr_ ID2D1Bitmap1** ppbmMap) const = 0;
	_Success_(return == S_OK)
	virtual bool IsInit() const = 0;
	virtual bool IsClear() const= 0;
	virtual void Clear();

public:
	DECLARE_IUNKNOWN;

	// IWICBitmapDecoder
	HRESULT STDMETHODCALLTYPE Initialize(__RPC__in_opt IStream* pIStream, WICDecodeOptions cacheOptions) override;
	HRESULT STDMETHODCALLTYPE CopyPalette(__RPC__in_opt IWICPalette* pIPalette) override;
	HRESULT STDMETHODCALLTYPE GetMetadataQueryReader(__RPC__deref_out_opt IWICMetadataQueryReader** ppIMetadataQueryReader) override;
	HRESULT STDMETHODCALLTYPE GetPreview(__RPC__deref_out_opt IWICBitmapSource** ppIBitmapSource) override;
	HRESULT STDMETHODCALLTYPE GetColorContexts(UINT cCount,
		__RPC__inout_ecount_full_opt(cCount) IWICColorContext** ppIColorContexts,
		__RPC__out UINT* pcActualCount) override;
	HRESULT STDMETHODCALLTYPE GetFrameCount(__RPC__out UINT* pCount) override;
	HRESULT STDMETHODCALLTYPE GetFrame(UINT index, __RPC__deref_out_opt IWICBitmapFrameDecode** ppIBitmapFrame) override;

	// IWICBitmapFrameDecode
	HRESULT STDMETHODCALLTYPE GetSize(_Out_ PUINT puiWidth, _Out_ PUINT puiHeight) override;
	HRESULT STDMETHODCALLTYPE GetPixelFormat(__RPC__out WICPixelFormatGUID* pPixelFormat) override;
	HRESULT STDMETHODCALLTYPE GetResolution(__RPC__out double *pDpiX, __RPC__out double *pDpiY) override;
	HRESULT STDMETHODCALLTYPE IWICBitmapFrameDecode::CopyPixels(__RPC__in_opt const WICRect* prc, UINT cbStride,
		UINT cbBufferSize, __RPC__out_ecount_full(cbBufferSize) BYTE* pbBuffer) override;

	// IWICBitmapSourceTransform
	HRESULT STDMETHODCALLTYPE IWICBitmapSourceTransform::CopyPixels(__RPC__in_opt const WICRect* prc,
		UINT uiWidth, UINT uiHeight, __RPC__in_opt WICPixelFormatGUID* pguidDstFormat, WICBitmapTransformOptions dstTransform,
		UINT nStride, UINT cbBufferSize, __RPC__out_ecount_full(cbBufferSize) BYTE* pbBuffer) override;
	HRESULT STDMETHODCALLTYPE GetClosestSize(__RPC__inout UINT* puiWidth, __RPC__inout UINT* puiHeight) override;
	HRESULT STDMETHODCALLTYPE GetClosestPixelFormat(__RPC__inout WICPixelFormatGUID* pguidDstFormat) override;
	HRESULT STDMETHODCALLTYPE DoesSupportTransform(WICBitmapTransformOptions dstTransform,
			__RPC__out BOOL* pfIsSupported) override;
};


///////////////////////////////////////////////////////////////////////
// Decoder info ///////////////////////////////////////////////////////


enum class DecoderInfoFlag : ULONG {
	Animation = 0x01, Chromakey = 0x02, Lossless = 0x04, Multiframe = 0x08,
	SvgWmfDefault = Chromakey|Lossless
};


class NOVTABLE DecoderInfoBase abstract : public IWICBitmapDecoderInfo
{
	IMPL_UNCOPYABLE(DecoderInfoBase)

protected:
	volatile ULONG m_cRef;
	const ULONG m_infoFlags;

	DecoderInfoBase(_In_ ULONG cRef, _In_opt_ ULONG infoFlags = (ULONG)DecoderInfoFlag::SvgWmfDefault)
		: m_cRef(cRef), m_infoFlags(infoFlags) { DllAddRef(); }
	virtual ~DecoderInfoBase() { DllRelease(); }

	virtual PCWSTR __fastcall GetMimeTypes(_Out_ PUINT pcch) const = 0;
	virtual PCWSTR __fastcall GetExtensions(_Out_ PUINT pcch) const = 0;

public:
	DECLARE_IUNKNOWN;

	HRESULT STDMETHODCALLTYPE GetComponentType(__RPC__out WICComponentType* pType) override;
	HRESULT STDMETHODCALLTYPE GetSigningStatus(__RPC__out DWORD* pStatus) override;
	HRESULT STDMETHODCALLTYPE GetAuthor(UINT cchAuthor,
		__RPC__inout_ecount_full_opt(cchAuthor) WCHAR* wzAuthor, __RPC__out UINT* pcchActual) override;
	HRESULT STDMETHODCALLTYPE GetVendorGUID(__RPC__out GUID* pguidVendor) override;
	HRESULT STDMETHODCALLTYPE GetVersion(UINT cchVersion,
		__RPC__inout_ecount_full_opt(cchVersion) WCHAR* wzVersion, __RPC__out UINT* pcchActual) override;
	HRESULT STDMETHODCALLTYPE GetSpecVersion(UINT cchSpecVersion,
		__RPC__inout_ecount_full_opt(cchSpecVersion) WCHAR* wzSpecVersion, __RPC__out UINT* pcchActual) override;
	/*HRESULT STDMETHODCALLTYPE GetFriendlyName(UINT cchFriendlyName,
		__RPC__inout_ecount_full_opt(cchFriendlyName) WCHAR* wzFriendlyName, __RPC__out UINT* pcchActual) override;*/

	//HRESULT STDMETHODCALLTYPE GetContainerFormat(__RPC__out GUID* pguidContainerFormat) override;
	HRESULT STDMETHODCALLTYPE GetPixelFormats(UINT cFormats,
		__RPC__inout_ecount_full_opt(cFormats) GUID* pguidPixelFormats, __RPC__out UINT* pcActual) override;
	HRESULT STDMETHODCALLTYPE GetColorManagementVersion(UINT cchColorManagementVersion,
		__RPC__inout_ecount_full_opt(cchColorManagementVersion) WCHAR* wzColorManagementVersion,
		__RPC__out UINT* pcchActual) override;
	HRESULT STDMETHODCALLTYPE GetDeviceManufacturer(UINT cchDeviceManufacturer,
		__RPC__inout_ecount_full_opt(cchDeviceManufacturer) WCHAR* wzDeviceManufacturer,
		__RPC__out UINT* pcchActual) override;
	HRESULT STDMETHODCALLTYPE GetDeviceModels(UINT cchDeviceModels,
		__RPC__inout_ecount_full_opt(cchDeviceModels) WCHAR* wzDeviceModels,
		__RPC__out UINT* pcchActual) override;
	HRESULT STDMETHODCALLTYPE GetMimeTypes(UINT cchMimeTypes,
		__RPC__inout_ecount_full_opt(cchMimeTypes) WCHAR* wzMimeTypes, __RPC__out UINT* pcchActual) override;
	HRESULT STDMETHODCALLTYPE GetFileExtensions(UINT cchFileExtensions,
		__RPC__inout_ecount_full_opt(cchFileExtensions) WCHAR* wzFileExtensions, __RPC__out UINT* pcchActual) override;
	HRESULT STDMETHODCALLTYPE MatchesMimeType(__RPC__in LPCWSTR wzMimeType, __RPC__out BOOL* pfMatches) override;
	HRESULT STDMETHODCALLTYPE DoesSupportAnimation(__RPC__out BOOL* pfSupportAnimation) override;
	HRESULT STDMETHODCALLTYPE DoesSupportChromakey(__RPC__out BOOL* pfSupportChromakey) override;
	HRESULT STDMETHODCALLTYPE DoesSupportLossless(__RPC__out BOOL* pfSupportLossless) override;
	HRESULT STDMETHODCALLTYPE DoesSupportMultiframe(__RPC__out BOOL* pfSupportMultiframe) override;	

	HRESULT STDMETHODCALLTYPE GetPatterns(UINT cbSizePatterns,
            _Out_writes_bytes_to_opt_(cbSizePatterns, *pcbPatternsActual) WICBitmapPattern* pPatterns,
            _Out_opt_  UINT *pcPatterns,  _Out_  UINT *pcbPatternsActual) override;
};


#pragma warning (default: 26495)

}	// namespace esmag	<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#endif	// #ifdef __cplusplus ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++