// SVGDecoder.h
#pragma once

#include "DecodeBase.h"

namespace RootNamespace {

#pragma warning (disable: 26495)


///////////////////////////////////////////////////////////////////////
// SVG decode /////////////////////////////////////////////////////////


class SvgDecoder : public DecoderBase
{
	ID2D1SvgDocument* m_svgDoc;

	bool IsInit() const override { return AllTrue(m_d2dDC, m_svgDoc); }
	bool IsClear() const { return AllTrue(!m_d2dDC, !m_svgDoc); }

	HRESULT QueryCopyPixelTransform_(D2D_SIZE_U targSize, WICBitmapTransformOptions wicTransform,
		UINT angleRotated, _Inout_ D2D_MATRIX_3X2_F* pMatrix) const override;
	_Success_(return == S_OK) HRESULT CreateCpuReadBitmap_(D2D_SIZE_F drawSize, D2D_SIZE_U targSize,
		_In_ const D2D1_MATRIX_3X2_F& transform, _COM_Outptr_ ID2D1Bitmap1** ppbmMap) const override;

	void Clear() override;

public:
	SvgDecoder() : DecoderBase(1) {}
	virtual ~SvgDecoder() { if (m_svgDoc) m_svgDoc->Release(); }
	void ZeroInit() { ZeroStructFrom(this, m_cRef); }

	_Success_(return == S_OK) HRESULT CreateCpuReadBitmapWT(D2D_SIZE_U targSize,
		WICBitmapTransformOptions transform, _COM_Outptr_ ID2D1Bitmap1** ppbmMap);

	HRESULT STDMETHODCALLTYPE QueryCapability(__RPC__in_opt IStream* pIStream, __RPC__out DWORD* pdwCapability) override;
	HRESULT STDMETHODCALLTYPE Initialize(__RPC__in_opt IStream* pIStream, WICDecodeOptions cacheOptions) override;
	HRESULT STDMETHODCALLTYPE GetContainerFormat(__RPC__out GUID* pguidContainerFormat) override;
	HRESULT STDMETHODCALLTYPE GetDecoderInfo(__RPC__deref_out_opt IWICBitmapDecoderInfo** ppIDecoderInfo) override;

	HRESULT STDMETHODCALLTYPE GetThumbnail(__RPC__deref_out_opt IWICBitmapSource** ppIThumbnail) override;

	ZEROINIT_NEW_DELETE
};



class SvgDecoderInfo : public DecoderInfoBase
{
private:
	PCWSTR __fastcall GetMimeTypes(_Out_ PUINT pcch) const override;
	PCWSTR __fastcall GetExtensions(_Out_ PUINT pcch) const override;

public:
	SvgDecoderInfo() : DecoderInfoBase(1, (ULONG)DecoderInfoFlag::SvgWmfDefault) {}

	HRESULT STDMETHODCALLTYPE GetCLSID(__RPC__out CLSID *pclsid) override;
	HRESULT STDMETHODCALLTYPE GetFriendlyName(UINT cchFriendlyName,
		__RPC__inout_ecount_full_opt(cchFriendlyName) WCHAR* wzFriendlyName, __RPC__out UINT* pcchActual) override;
	HRESULT STDMETHODCALLTYPE GetContainerFormat(__RPC__out GUID* pguidContainerFormat) override;
	HRESULT STDMETHODCALLTYPE MatchesPattern(__RPC__in_opt IStream* pIStream, __RPC__out BOOL* pfMatches) override;
	HRESULT STDMETHODCALLTYPE CreateInstance(__RPC__deref_out_opt IWICBitmapDecoder** ppIBitmapDecoder) override;
};


#pragma warning (default: 26495)

}	// namespace
