// WMFDecoder.h
#pragma once

#include "DecodeBase.h"

namespace RootNamespace {

#pragma warning (disable: 26495)


///////////////////////////////////////////////////////////////////////
// WMF decode /////////////////////////////////////////////////////////


class WmfDecoder : public DecoderBase
{
	ID2D1GdiMetafile1* m_metafile;
	D2D_POINT_2F m_ptOrigin;

	bool IsInit() const override { return AllTrue(m_d2dDC, m_metafile); }
	bool IsClear() const override { return AllTrue(!m_d2dDC, !m_metafile); }

	HRESULT __fastcall InitLoad(_In_ IStream* pstm) override;
	HRESULT QueryCopyPixelTransform_(D2D_SIZE_U targSize, WICBitmapTransformOptions wicTransform,
		UINT angleRotated, _Inout_ D2D_MATRIX_3X2_F* pMatrix) const override;
	_Success_(return == S_OK) HRESULT CreateCpuReadBitmap_(D2D_SIZE_F drawSize, D2D_SIZE_U targSize,
		_In_ const D2D1_MATRIX_3X2_F& transform, _COM_Outptr_ ID2D1Bitmap1** ppbmMap) const override;

	void Clear() override;

public:
	WmfDecoder() : DecoderBase(1) {}
	virtual ~WmfDecoder() { if (m_metafile) m_metafile->Release(); }

	_Success_(return == S_OK) HRESULT CreateCpuReadBitmapWT(D2D_SIZE_U targSize, bool scale,
		WICBitmapTransformOptions transform, _COM_Outptr_ ID2D1Bitmap1** ppbmMap);

	HRESULT STDMETHODCALLTYPE QueryCapability(__RPC__in_opt IStream* pIStream, __RPC__out DWORD* pdwCapability) override;
	HRESULT STDMETHODCALLTYPE GetContainerFormat(__RPC__out GUID* pguidContainerFormat) override;
	HRESULT STDMETHODCALLTYPE GetDecoderInfo(__RPC__deref_out_opt IWICBitmapDecoderInfo** ppIDecoderInfo) override;

	HRESULT STDMETHODCALLTYPE GetThumbnail(__RPC__deref_out_opt IWICBitmapSource** ppIThumbnail) override;

	ZEROINIT_NEW_DELETE
};



class WmfDecoderInfo : public DecoderInfoBase
{
private:
	PCWSTR __fastcall GetMimeTypes(_Out_ PUINT pcch) const override;
	PCWSTR __fastcall GetExtensions(_Out_ PUINT pcch) const override;

public:
	WmfDecoderInfo() : DecoderInfoBase(1, (ULONG)DecoderInfoFlag::SvgWmfDefault) {}

	HRESULT STDMETHODCALLTYPE GetCLSID(__RPC__out CLSID *pclsid) override;
	HRESULT STDMETHODCALLTYPE GetFriendlyName(UINT cchFriendlyName,
		__RPC__inout_ecount_full_opt(cchFriendlyName) WCHAR* wzFriendlyName, __RPC__out UINT* pcchActual) override;
	HRESULT STDMETHODCALLTYPE GetContainerFormat(__RPC__out GUID* pguidContainerFormat) override;
	HRESULT STDMETHODCALLTYPE MatchesPattern(__RPC__in_opt IStream* pIStream, __RPC__out BOOL* pfMatches) override;
	HRESULT STDMETHODCALLTYPE CreateInstance(__RPC__deref_out_opt IWICBitmapDecoder** ppIBitmapDecoder) override;
};


#pragma warning (default: 26495)

}	// namespace
