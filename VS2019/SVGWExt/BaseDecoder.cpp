// BaseDecoder.cpp

#include "pch.h"
#include "DecodeBase.h"

#pragma warning(disable: 6387 6388 28196)

namespace RootNamespace {

///////////////////////////////////////////////////////////////////////
// IWICBitmapDecoder //////////////////////////////////////////////////


WARNING_SUPPRESS(6101 6388 28196)
HRESULT DecoderBase::QueryInterface(REFIID riid, _COM_Outptr_ void** ppvObject)
{
	const HRESULT hr = QueryInterfaceImpl_(riid, ppvObject, 3, QI_ARG(IWICBitmapDecoder, this),
				QI_ARG(IWICBitmapFrameDecode, this), QI_ARG(IWICBitmapSourceTransform, this));
	if (S_OK == hr)
		_InterlockedIncrement(&m_cRef);
	return hr;
}

ULONG DecoderBase::AddRef()
{
	ADDREF_IMPL;
}

ULONG DecoderBase::Release()
{
	RELEASE_IMPL;
}



HRESULT DecoderBase::Initialize(__RPC__in_opt IStream* pIStream, WICDecodeOptions)
{
	HRESULT hr = E_INVALIDARG;
	if (pIStream)
	{
		EnterCS();
		hr = WINCODEC_ERR_WRONGSTATE;
		if (IsClear()) __try
		{
			hr = DoLoad(pIStream);
		}
		__finally
		{
			LeaveCS();
		}
	}
	return hr;
}


HRESULT DecoderBase::GetMetadataQueryReader(__RPC__deref_out_opt IWICMetadataQueryReader** ppIMetadataQueryReader)
{
	if (ppIMetadataQueryReader)
		*ppIMetadataQueryReader = nullptr;
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}


HRESULT DecoderBase::GetPreview(__RPC__deref_out_opt IWICBitmapSource** ppIBitmapSource)
{
	if (ppIBitmapSource)
		*ppIBitmapSource = nullptr;
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}


HRESULT DecoderBase::GetColorContexts(UINT cCount,
	__RPC__inout_ecount_full_opt(cCount) IWICColorContext** ppIColorContexts,
	__RPC__out UINT* pcActualCount)
{
	if (pcActualCount)
		*pcActualCount = 0;
	if (ppIColorContexts)
		*ppIColorContexts = nullptr;
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}


HRESULT DecoderBase::GetFrameCount(__RPC__out UINT* pCount)
{
	if (pCount)
	{
		if (IsInit())
		{
			*pCount = 1;
			return S_OK;
		}
		*pCount = 0;
		return WINCODEC_ERR_NOTINITIALIZED;
	}
	return E_INVALIDARG;
}


HRESULT DecoderBase::GetFrame(UINT index, __RPC__deref_out_opt IWICBitmapFrameDecode** ppIBitmapFrame)
{
	HRESULT hr = E_INVALIDARG;
	if (ppIBitmapFrame)
	{
		*ppIBitmapFrame = nullptr;
		hr = WINCODEC_ERR_FRAMEMISSING;
		if (0 == index)
		{
			*ppIBitmapFrame = static_cast<IWICBitmapFrameDecode*>(this);
			_InterlockedIncrement(&m_cRef);
			hr = S_OK;
		}
	}
	return hr;
}


WARNING_SUPPRESS(28183) _Success_(return == S_OK)
HRESULT DecoderBase::EnsureWicBitmap(_COM_Outptr_opt_result_nullonfailure_ IWICBitmap** ppBitmap)
{
	HRESULT hr = S_OK;
	if (!m_wicBitmap)
	{
		EnterCS();
		__try
		{
			ASSUME(S_OK == hr);
			if (!m_wicBitmap)
				hr = CreateBitmapSource(m_sizeDips, D2D1::Matrix3x2F::Identity(), &m_wicBitmap);
		}
		__finally
		{
			LeaveCS();
		}
	}
	if (ppBitmap)
	{
		*ppBitmap = m_wicBitmap;
		if (m_wicBitmap)
		{
			m_wicBitmap->AddRef();
			ASSUME(S_OK == hr);
		}
	}
	return hr;
}


D2D_SIZE_U DecoderBase::GetPixelSize() const
{
	D2D_SIZE_U size;
	if (!m_wicBitmap || S_OK != m_wicBitmap->GetSize(&size.width, &size.height))
		size = wcToImageSizeU(m_sizeDips);
	return size;
}


D2D_SIZE_F DecoderBase::CalcThumbSize() const
{
	UINT side = 96;
	if (m_screenDpi >= 144)
	{
		side = 176;
		if (m_screenDpi >= 192)
			side = 256;
	}
	return wcScaleSizeF(m_sizeDips, side);
}


/*
_Success_(return == S_OK)
HRESULT DecoderBase::CreateBitmapSource(_In_opt_ UINT maxSide, _COM_Outptr_ IWICBitmap** ppIBitmap) const
{
	return CreateBitmapSource(ScaleSize(m_sizeDips, maxSide), ppIBitmap);
}
*/

WARNING_SUPPRESS(6101)
_Success_(return == S_OK) HRESULT DecoderBase::CreateBitmapSource(D2D_SIZE_F size,
		_In_ const D2D1_MATRIX_3X2_F& transform, _COM_Outptr_ IWICBitmap** ppIBitmap)
{
	IWICImagingFactory2* wicFact;
	HRESULT hr = GetWicFactory(&wicFact);
	if (S_OK == hr)
	{
		ID2D1Bitmap1* pbmMap;
		const D2D_SIZE_U targSize = wcToImageSizeU(size);
		hr = CreateCpuReadBitmap(size, targSize, transform, &pbmMap);
		if (S_OK == hr)
		{
			D2D1_MAPPED_RECT mapRect;
			hr = pbmMap->Map(D2D1_MAP_OPTIONS_READ, &mapRect);
			if (S_OK == hr)
			{
				hr = wicFact->CreateBitmapFromMemory(targSize.width, targSize.height, GUID_WICPixelFormat32bppPBGRA,
							mapRect.pitch, mapRect.pitch * targSize.height, mapRect.bits, ppIBitmap);
				pbmMap->Unmap();
			}
			pbmMap->Release();
		}
		wicFact->Release();
	}
	return hr;
}

/*
_Success_(return == S_OK) HRESULT DecoderBase::CreateBitmapSource(D2D_SIZE_F size, _COM_Outptr_ IWICBitmap** ppIBitmap)
{
	return CreateBitmapSource(size, D2D1::Matrix3x2F::Identity(), ppIBitmap);
}
*/

_Success_(return == S_OK) HRESULT DecoderBase::CreateCpuReadBitmap(D2D_SIZE_F drawSize, D2D_SIZE_U targSize,
		_In_ const D2D1_MATRIX_3X2_F& transform, _COM_Outptr_ ID2D1Bitmap1** ppbmMap)
{
	EnterCS();
	HRESULT hr;
	__try
	{
		hr = CreateCpuReadBitmap_(drawSize, targSize, transform, ppbmMap);
		if (S_OK != hr && wcIsDXRecreateError(hr))
		{
			hr = Reload(hr);
			if (S_OK == hr)
				hr = CreateCpuReadBitmap_(drawSize, targSize, transform, ppbmMap);
		}
	}
	__finally
	{
		LeaveCS();
	}
	return hr;
}


_Success_(return == S_OK)
HRESULT DecoderBase::CreateThumbnail(D2D_SIZE_F size, _In_ const D2D1_MATRIX_3X2_F& transform,
		__RPC__deref_out_opt IWICBitmapSource** ppIThumbnail)
{
	HRESULT hr = E_INVALIDARG;
	if (ppIThumbnail)
	{
		hr = WINCODEC_ERR_NOTINITIALIZED;
		if (IsInit())
		{
			hr = CreateBitmapSource(size, transform, RECAST(IWICBitmap**, ppIThumbnail));
			if (S_OK == hr)
				return S_OK;
		}
		*ppIThumbnail = nullptr;
	}
	return hr;
}


HRESULT DecoderBase::DoLoad(_In_ IStream* pstm)
{
	ASSERT(IsClear());
	HRESULT hr = InitLoad(pstm);
	if (S_OK == hr)
		m_screenDpi = GetScreenDpi();
	else if (AnyTrue(SUCCEEDED(hr), E_FAIL == hr))
		hr = WINCODEC_ERR_UNKNOWNIMAGEFORMAT;
	return hr;
}

HRESULT DecoderBase::Reload(HRESULT hrRecreate)
{
	Clear();
	if (m_pstmInit)
	{
		hrRecreate = Stream_SeekStart(m_pstmInit);
		if (SUCCEEDED(hrRecreate))
		{
			hrRecreate = DoLoad(m_pstmInit);
			if (S_OK == hrRecreate)
				return S_OK;
		}
		SafeRelease(&m_pstmInit);
	}
	return hrRecreate;
}

void DecoderBase::Clear()
{
	Zero8Bytes(&m_sizeDips);
	SafeRelease(&m_wicBitmap);
	SafeRelease(&m_d2dDC);
	SafeRelease(&m_pstmInit);
}


///////////////////////////////////////////////////////////////////////
// IWICBitmapFrameDecode //////////////////////////////////////////////


HRESULT DecoderBase::GetSize(_Out_ PUINT puiWidth, _Out_ PUINT puiHeight)
{
	if (m_wicBitmap)
		return m_wicBitmap->GetSize(puiWidth, puiHeight);

	HRESULT hr = E_INVALIDARG;
	if (puiHeight)
	{
		*puiHeight = wcToImageSizeU(m_sizeDips.height);
		hr = S_OK;
	}
	if (puiWidth)
	{
		*puiWidth = wcToImageSizeU(m_sizeDips.width);
		hr = S_OK;
	}
	return hr;
}

HRESULT DecoderBase::GetPixelFormat(__RPC__out WICPixelFormatGUID* pPixelFormat)
{
	if (m_wicBitmap)
		return m_wicBitmap->GetPixelFormat(pPixelFormat);

	if (pPixelFormat)
	{
		CopyGUID(pPixelFormat, GUID_WICPixelFormat32bppPBGRA);
		return S_OK;
	}
	return E_INVALIDARG;
}

HRESULT DecoderBase::GetResolution(__RPC__out double *pDpiX, __RPC__out double *pDpiY)
{
	if (m_wicBitmap)
		return m_wicBitmap->GetResolution(pDpiX, pDpiY);

	HRESULT hr = E_INVALIDARG;
	if (pDpiY)
	{
		*pDpiY = 96;
		hr = S_OK;
	}
	if (pDpiX)
	{
		*pDpiX = 96;
		hr = S_OK;
	}
	return hr;
}

HRESULT DecoderBase::CopyPalette(__RPC__in_opt IWICPalette *pIPalette)
{
	if (m_wicBitmap)
		return m_wicBitmap->CopyPalette(pIPalette);
	return WINCODEC_ERR_PALETTEUNAVAILABLE;
}

HRESULT DecoderBase::CopyPixels(__RPC__in_opt const WICRect* prc, UINT cbStride,
		UINT cbBufferSize, __RPC__out_ecount_full(cbBufferSize) BYTE* pbBuffer)
{
	const HRESULT hr = EnsureWicBitmap();
	return (S_OK == hr) ? m_wicBitmap->CopyPixels(prc, cbStride, cbBufferSize, pbBuffer) : hr;
}


///////////////////////////////////////////////////////////////////////
// IWICBitmapSourceTransform //////////////////////////////////////////


HRESULT DecoderBase::CopyPixels(__RPC__in_opt const WICRect* prc,
		UINT uiWidth, UINT uiHeight, __RPC__in_opt WICPixelFormatGUID* pguidDstFormat, WICBitmapTransformOptions dstTransform,
		UINT nStride, UINT cbBufferSize, __RPC__out_ecount_full(cbBufferSize) BYTE* pbBuffer)
{
	HRESULT hr = E_INVALIDARG;
	if (pbBuffer && (!prc || IsRectValid(*prc, uiWidth, uiHeight)))
	{
		hr = HRESULT_INVALID;
		if (AnyTrue(WICBitmapTransformRotate0 == dstTransform, WICBitmapTransformRotate180 == dstTransform,
			WICBitmapTransformFlipHorizontal == dstTransform, WICBitmapTransformFlipVertical == dstTransform)
			&& AllTrue(nStride >= uiWidth * 4, cbBufferSize >= nStride * uiHeight))
		{
			UINT wicStride;
			UWicRect rcBm;
			rcBm.lt64 = 0;
			rcBm.uSize = GetPixelSize();
			ASSUME(HRESULT_INVALID == hr);
			if (AllTrue(rcBm.uSize.width == uiWidth, rcBm.uSize.height == uiHeight))
			{
				hr = EnsureWicBitmap();
				if (S_OK == hr)
				{
					IWICBitmapLock* lock = nullptr;
					WICInProcPointer wicBytes;
					EnterCS();
					__try
					{
						hr = m_wicBitmap->Lock(prc ? prc : &rcBm.Rect, WICBitmapLockRead, &lock);
						if (S_OK == hr)
						{
							hr = lock->GetDataPointer(&wicStride, &wicBytes);
							if (S_OK == hr)
							{
								hr = HRESULT_INVALID;
								if (wicStride >= uiWidth * uiHeight * 4)
								{
									hr = lock->GetStride(&wicStride);
									if (S_OK == hr)
									{
										CopyImageBits32(pbBuffer, nStride, wicBytes, wicStride, rcBm.uSize, dstTransform);
										ASSUME(S_OK == hr);
									}
								}
							}
						}
					}
					__finally
					{
						if (lock)
							lock->Release();
						LeaveCS();
					}
				}
			}
		}

		if (HRESULT_INVALID == hr)
		{
			D2D_SIZE_U targSize = { uiWidth, uiHeight };
			const D2D_SIZE_F drawSize = { (FLOAT)uiWidth, (FLOAT)uiHeight };
			D2D_MATRIX_3X2_F matrix = D2D1::Matrix3x2F::Identity();
			D2D_POINT_2F ptRotate = { drawSize.width / 2, drawSize.height / 2 };
			UINT angle = 0;
			switch (dstTransform)
			{
			case WICBitmapTransformRotate90:
				Swap(targSize.width, targSize.height);
				ptRotate.x = ptRotate.y;
				angle = 90;
				break;
			case WICBitmapTransformRotate180:
				angle = 180;
				break;
			case WICBitmapTransformRotate270:
				Swap(targSize.width, targSize.height);
				ptRotate.y = ptRotate.x;
				angle = 270;
				break;
			}

			hr = __HRESULT_FROM_WIN32(ERROR_INVALID_USER_BUFFER);	// WINCODEC_ERR_INSUFFICIENTBUFFER
			if (AllTrue(nStride >= targSize.width * 4, cbBufferSize >= nStride * targSize.height))
			{
				if (angle)
				{
					::D2D1MakeRotateMatrix((FLOAT)angle, ptRotate, &matrix);
					dstTransform = WICBitmapTransformRotate0;
				}
				hr = QueryCopyPixelTransform_({ uiWidth, uiHeight }, dstTransform, angle, &matrix);
				if (SUCCEEDED(hr))
				{
					ID2D1Bitmap1* pbm;
					hr = CreateCpuReadBitmap(drawSize, targSize, matrix, &pbm);
					if (S_OK == hr)
					{
						D2D1_MAPPED_RECT map;
						hr = pbm->Map(D2D1_MAP_OPTIONS_READ, &map);
						if (S_OK == hr)
						{
							CopyImageBits32(pbBuffer, nStride, map.bits, map.pitch, targSize, dstTransform);
							pbm->Unmap();
							ASSUME(S_OK == hr);
						}
						pbm->Release();
					}
				}
			}
		}
	}
	return hr;
}


HRESULT DecoderBase::GetClosestSize(__RPC__inout UINT* puiWidth, __RPC__inout UINT* puiHeight)
{ 
	if (AllTrue(puiWidth, puiHeight))
	{
		const UINT maxSide = GetMaxBitmapSide();
		const D2D_SIZE_U size = GetPixelSize();
		if (0 == *puiWidth)
			*puiWidth = size.width;
		else if (*puiWidth > maxSide)
			*puiWidth = maxSide;
		if (0 == *puiHeight)
			*puiHeight = size.height;
		else if (*puiHeight > maxSide)
			*puiHeight = maxSide;
		return S_OK;
	}
	return E_INVALIDARG;
}

HRESULT DecoderBase::GetClosestPixelFormat(__RPC__inout WICPixelFormatGUID* pguidDstFormat)
{
	if (pguidDstFormat)
	{
		CopyGUID(pguidDstFormat, GUID_WICPixelFormat32bppPBGRA);
		return S_OK;
	}
	return E_INVALIDARG;
}

HRESULT DecoderBase::DoesSupportTransform(WICBitmapTransformOptions dstTransform, __RPC__out BOOL* pfIsSupported)
{
	if (pfIsSupported)
	{
		switch (dstTransform)
		{
		case WICBitmapTransformRotate0:
		case WICBitmapTransformRotate90:
		case WICBitmapTransformRotate180:
		case WICBitmapTransformRotate270:
		case WICBitmapTransformFlipHorizontal:
		case WICBitmapTransformFlipVertical:
			*pfIsSupported = TRUE;
			return S_OK;
		}
		*pfIsSupported = FALSE;
	}
	return E_INVALIDARG;
}


}	// namespace