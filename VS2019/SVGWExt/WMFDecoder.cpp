// WMFDecoder.cpp

#include "pch.h"
#include "WMFDecoder.h"
//#include <gdiplusenums.h>

#pragma warning(disable: 6387 6388 28196)

namespace RootNamespace {

///////////////////////////////////////////////////////////////////////
// WMF decoder ////////////////////////////////////////////////////////

static bool g_wmfSuccess = false;
static bool g_wmfInfoFail = false;
static inline bool WmfCopyStream() { return AllTrue(!g_wmfSuccess, g_wmfInfoFail); }


WARNING_SUPPRESS(6101) _Success_(return == S_OK) NOALIAS NOINLINE
static HRESULT CreateMetafile(bool setGlobals, _In_ IStream* pstm, _In_ ID2D1Factory1* d2dFactory,
		_COM_Outptr_ ID2D1GdiMetafile1** ppMetafile, _Out_ D2D_RECT_F* prcBounds)
{
	ID2D1GdiMetafile* metafile;
	HRESULT hr = d2dFactory->CreateGdiMetafile(pstm, &metafile);
	if (S_OK == hr)
	{
		hr = metafile->QueryInterface(IID_ID2D1GdiMetafile1, PPV_ARG(ppMetafile));
		metafile->Release();
		metafile = *ppMetafile;
		if (S_OK == hr)
		{
			hr = metafile->GetBounds(prcBounds);
			if (S_OK == hr || S_OK == (hr = reinterpret_cast<ID2D1GdiMetafile1*>(metafile)->GetSourceBounds(prcBounds)))
			{
				if (IsZero(prcBounds->left))
					prcBounds->left = 0;
				if (IsZero(prcBounds->top))
					prcBounds->top = 0;
				if (AllTrue(prcBounds->right > prcBounds->left, prcBounds->bottom > prcBounds->top))
				{
					if (setGlobals)
						g_wmfSuccess = true;
					return S_OK;
				}
				hr = WINCODEC_ERR_IMAGESIZEOUTOFRANGE;
			}
			else if (AllTrue(FAILED(hr), setGlobals))
				g_wmfInfoFail = true;
			metafile->Release();
		}
	}
	return hr;
}


_Success_(return == S_OK) NOALIAS
HRESULT InitLoadWmf(_In_ IStream* pstm, _COM_Outptr_opt_result_maybenull_ IStream** ppstmTemp,
		_COM_Outptr_result_nullonfailure_ ID2D1DeviceContext5** ppDC,
		_COM_Outptr_result_nullonfailure_ ID2D1GdiMetafile1** ppMetafile,
		_Out_ D2D_POINT_2F* pOrigin, _Out_ D2D_SIZE_F* pSize)
{
	IStream* pstmMem = nullptr;
	HRESULT hr;

	if (ppstmTemp)
	{
		WARNING_SUPPRESS(6001) ASSERT(NULL == *ppstmTemp);
		hr = wcTryUncompressStream(pstm, FALSE, &pstmMem);
		if (S_OK == hr)
		{
			pstm = pstmMem;
			*ppstmTemp = pstmMem;
		}
		else
		{
			*ppstmTemp = nullptr;
			if (!IsUnknownImage(hr))
				goto Error_;
		}
	}

	hr = CreateD2DC(ppDC);
	if (S_OK == hr)
	{
		ID2D1Factory6* d2dFactory;
		hr = GetD2DFactory(&d2dFactory);
		if (S_OK == hr)
		{
			D2D_RECT_F rcBounds;
			hr = HRESULT_INVALID;
			if (!WmfCopyStream())
				hr = CreateMetafile(true, pstm, d2dFactory, ppMetafile, &rcBounds);
			if (AllTrue(S_OK != hr, ppstmTemp, !pstmMem) && AnyTrue(g_wmfInfoFail, E_FAIL == hr))
			{
				ASSUME(ppstmTemp && !pstmMem);
				hr = wcCopyTempStream(pstm, 0, &pstmMem);
				if (S_OK == hr)
				{
					hr = CreateMetafile(false, pstmMem, d2dFactory, ppMetafile, &rcBounds);
					if (S_OK == hr)
					{
						pstm = pstmMem;
						*ppstmTemp = pstmMem;
					}
				}
			}
			d2dFactory->Release();
			if (S_OK == hr)
			{
				Copy8Bytes(pOrigin, &rcBounds);
				pSize->width = rcBounds.right - rcBounds.left;
				pSize->height = rcBounds.bottom - rcBounds.top;
				return S_OK;
			}
		}
		(*ppDC)->Release();
	}

	if (pstmMem)
	{
		pstmMem->Release();
		*ppstmTemp = nullptr;
	}

Error_:
	Zero8Bytes(pSize);
	*ppMetafile = nullptr;
	*ppDC = nullptr;
	return hr;
}


HRESULT WmfDecoder::QueryCapability(__RPC__in_opt IStream* pIStream, __RPC__out DWORD* pdwCapability)
{
	HRESULT hr = E_INVALIDARG;
	if (pdwCapability)
	{
		*pdwCapability = 0;
		if (pIStream)
		{
			hr = ::wcMatchesWmfEmfPattern(pIStream);
			if (S_OK == hr)
				*pdwCapability = WICBitmapDecoderCapabilityCanDecodeAllImages|WICBitmapDecoderCapabilityCanDecodeThumbnail;
			else if (SUCCEEDED(hr))
				hr = WINCODEC_ERR_UNKNOWNIMAGEFORMAT;
		}
	}
	return hr;
}


HRESULT WmfDecoder::InitLoad(_In_ IStream* pstm)
{
	return InitLoadWmf(pstm, (pstm != m_pstmInit) ? &m_pstmInit : nullptr, &m_d2dDC, &m_metafile, &m_ptOrigin, &m_sizeDips);
}


HRESULT WmfDecoder::GetContainerFormat(__RPC__out GUID* pguidContainerFormat)
{
	if (pguidContainerFormat)
	{
		CopyGUID(pguidContainerFormat, GUID_AE_ContainerFormatWmfEmf);
		return S_OK;
	}
	return E_INVALIDARG;
}


HRESULT WmfDecoder::GetDecoderInfo(__RPC__deref_out_opt IWICBitmapDecoderInfo** ppIDecoderInfo)
{
	HRESULT hr = E_INVALIDARG;
	if (ppIDecoderInfo)
	{
		if (*ppIDecoderInfo = static_cast<IWICBitmapDecoderInfo*>(new WmfDecoderInfo()))
			return S_OK;
		hr = E_OUTOFMEMORY;
	}
	return hr;
}


HRESULT WmfDecoder::GetThumbnail(__RPC__deref_out_opt IWICBitmapSource** ppIThumbnail)
{
	D2D_SIZE_F sizeDips = CalcThumbSize();
	D2D_MATRIX_3X2_F scale;
	if (AnyTrue(sizeDips.width < m_sizeDips.width, sizeDips.height < m_sizeDips.height))
	{
		scale = D2D1::Matrix3x2F::Scale(
			{ sizeDips.width / m_sizeDips.width, sizeDips.height / m_sizeDips.height },
			{ sizeDips.width / 2, sizeDips.height / 2 });
	}
	else
	{
		scale = D2D1::Matrix3x2F::Identity();
		Copy8Bytes(&sizeDips, &m_sizeDips);
	}
	return CreateThumbnail(sizeDips, scale, ppIThumbnail);
}


HRESULT WmfDecoder::QueryCopyPixelTransform_(D2D_SIZE_U reqSize, WICBitmapTransformOptions wicTransform,
		UINT angleRotated, _Inout_ D2D_MATRIX_3X2_F* pMatrix) const
{
	if (reqSize != GetPixelSize())
	{
		const D2D_MATRIX_3X2_F scaleMatrix = D2D1::Matrix3x2F::Scale(
			{ reqSize.width / m_sizeDips.width, reqSize.height / m_sizeDips.height });
		if (angleRotated)
			*pMatrix = scaleMatrix * *pMatrix;
		else
			CopyStruct(pMatrix, &scaleMatrix);
		return S_OK;
	}
	return S_FALSE;
}

_Success_(return == S_OK) HRESULT WmfDecoder::CreateCpuReadBitmap_(D2D_SIZE_F drawSize, D2D_SIZE_U targSize,
		_In_ const D2D1_MATRIX_3X2_F& transform, _COM_Outptr_ ID2D1Bitmap1** ppbmMap) const
{
	return CreateWmfCpuReadBitmap(m_d2dDC, m_metafile, targSize, m_ptOrigin, transform, ppbmMap);
}


void WmfDecoder::Clear()
{
	Zero8Bytes(&m_ptOrigin);
	SafeRelease(&m_metafile);
	__super::Clear();
}


_Success_(return == S_OK)
WCXSTDAPI wcWmfToCpuBitmap(_In_opt_ ID2D1DeviceContext5* d2dDC, _In_ ID2D1GdiMetafile* metafile, D2D_SIZE_U size,
		_In_opt_ const D2D1_POINT_2F *targOffset, _In_opt_ const D2D1_MATRIX_3X2_F* transform,
		_COM_Outptr_result_nullonfailure_ ID2D1Bitmap1** ppbm)
{
	*ppbm = nullptr;

	HRESULT hr = D2DERR_NO_HARDWARE_DEVICE;
	if (d2dDC)
	{
		ID2D1Bitmap1* pbmTarg;
		D2D1_BITMAP_PROPERTIES1 props = { { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED },
			96, 96, D2D1_BITMAP_OPTIONS_TARGET, nullptr };
		hr = d2dDC->CreateBitmap(size, nullptr, 0, &props, &pbmTarg);
		if (S_OK == hr)
		{
			ID2D1Image* oldTarg = nullptr;
			d2dDC->GetTarget(&oldTarg);
			d2dDC->SetTarget(pbmTarg);
			d2dDC->BeginDraw();
			d2dDC->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			if (transform)
				d2dDC->SetTransform(transform);
			D2D1_COLOR_F clr;
			Zero16Bytes(&clr);
			d2dDC->Clear(&clr);
			d2dDC->DrawGdiMetafile(metafile, targOffset);
			hr = d2dDC->EndDraw();
			d2dDC->SetTarget(oldTarg);
			if (S_OK == hr)
			{
				ID2D1Bitmap1* pbmCpu;
				props.bitmapOptions = D2D1_BITMAP_OPTIONS_CPU_READ|D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
				hr = d2dDC->CreateBitmap(size, nullptr, 0, &props, &pbmCpu);
				if (S_OK == hr)
				{
					hr = pbmCpu->CopyFromBitmap(nullptr, pbmTarg, nullptr);
					if (S_OK == hr)
						*ppbm = pbmCpu;
					else
						pbmCpu->Release();
				}
			}
			pbmTarg->Release();
		}
	}
	return hr;
}


///////////////////////////////////////////////////////////////////////
// SVG decoder info ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


HRESULT WmfDecoderInfo::GetCLSID(__RPC__out CLSID* pclsid)
{
	if (pclsid)
	{
		CopyGUID(pclsid, CLSID_WmfEmfDecoder);
		return S_OK;
	}
	return E_INVALIDARG;
}


HRESULT WmfDecoderInfo::GetFriendlyName(UINT cchFriendlyName,
	__RPC__inout_ecount_full_opt(cchFriendlyName) WCHAR* wzFriendlyName, __RPC__out UINT* pcchActual)
{
	WCHAR wcName[ALIGN4(_countof(WMF_DECODER_NAMEA) + 1)];
	return ReturnInfoRealString(wcName, wcAsciiToWide(WMF_DECODER_NAMEA, wcName, _countof(wcName)),
			cchFriendlyName, wzFriendlyName, pcchActual);
}


HRESULT WmfDecoderInfo::GetContainerFormat(__RPC__out GUID* pguidContainerFormat)
{
	if (pguidContainerFormat)
	{
		CopyGUID(pguidContainerFormat, GUID_AE_ContainerFormatWmfEmf);
		return S_OK;
	}
	return E_INVALIDARG;
}


HRESULT WmfDecoderInfo::CreateInstance(__RPC__deref_out_opt IWICBitmapDecoder** ppIBitmapDecoder)
{
	if (ppIBitmapDecoder)
	{
		IWICBitmapDecoder* const decoder = new WmfDecoder();
		*ppIBitmapDecoder = decoder;
		return (decoder ? S_OK : E_OUTOFMEMORY);
	}
	return E_INVALIDARG;
}


PCWSTR WmfDecoderInfo::GetMimeTypes(_Out_ PUINT pcch) const
{
	*pcch = _countof(WMFEMF_WIC_MIME_TYPES) - 1;
	return WMFEMF_WIC_MIME_TYPES;
}

PCWSTR WmfDecoderInfo::GetExtensions(_Out_ PUINT pcch) const
{
	*pcch = _countof(WMFEMF_WIC_EXTENSIONS) - 1;
	return WMFEMF_WIC_EXTENSIONS;
}


HRESULT WmfDecoderInfo::MatchesPattern(__RPC__in_opt IStream* pIStream, __RPC__out BOOL* pfMatches)
{
	if (pfMatches)
	{
		if (pIStream)
		{
			const HRESULT hr = ::wcMatchesWmfEmfPattern(pIStream);
			*pfMatches = (S_OK == hr);
			return (IsUnknownImage(hr) ? S_OK : hr);
		}
		*pfMatches = FALSE;
	}
	return E_INVALIDARG;
}


_Success_(S_OK == return || S_FALSE == return)
static HRESULT __fastcall ReadWmfEmfPattern(_In_ IStream* pstm, _Out_opt_ UGZIPHEADER* pGzHdr)
{
	union {
		UMETAHEADER hdr;
		UGZIPHEADER gzh;
	};
	ULONG cb = 0;
	HRESULT hr = pstm->Read(&hdr, sizeof(hdr), &cb);
	if (pGzHdr)
		ZeroStruct(pGzHdr);
	else
		pstm->Revert();	// stop further uncompressing
	if (SUCCEEDED(hr))
	{
		if (sizeof(hdr) == cb)
		{
			if (0x9AC6CDD7 == hdr.WEmf.TypeSign || AllTrue(EMR_HEADER == hdr.WEmf.TypeSign, ENHMETA_SIGNATURE == hdr.WEmf.EmfSignature))
				return S_OK;
			if (0x0002 == hdr.Wmf.WmfType && AnyTrue(0x0100 == hdr.Wmf.WmfVersion, 0x0300 == hdr.Wmf.WmfVersion))
				return S_OK;
			if (AllTrue(pGzHdr, 0x088B1F == (gzh.Part32 & 0xFFFFFF)))
			{
				pGzHdr->Part32 = gzh.Part32;
				return S_FALSE;
			}
		}
		hr = WINCODEC_ERR_UNKNOWNIMAGEFORMAT;
	}
	return hr;
}

static HRESULT CALLBACK MatchesWmfEmfPattern(_In_ IStream* pstm, UINT64, _Out_opt_ void* pIsGzip)
{
	UGZIPHEADER gzHdr;
	HRESULT hr = ReadWmfEmfPattern(pstm, &gzHdr);
	bool_t isGzip = false;
	if (S_FALSE == hr)
	{
		IStream* pstmInf;
		hr = wcTryUncompressStream(pstm, TRUE, &pstmInf);
		if (S_OK == hr)
		{
			hr = ReadWmfEmfPattern(pstmInf, nullptr);
			isGzip = (S_OK == hr);
			pstmInf->Release();
		}
	}
	if (pIsGzip)
		*((bool_t*)pIsGzip) = isGzip;
	return hr;
}

_Check_return_ WCXFASTAPI wcMatchesWmfEmfPattern(_In_ IStream* pstm, _Out_opt_ bool_t* pbGzip)
{
	if (pbGzip)
		*pbGzip = false;
	return StreamSeekBack(pstm, &MatchesWmfEmfPattern, pbGzip);
}


WCXFASTAPI wcCreateWmfEmfComponent(_In_ REFIID riid, _COM_Outptr_result_nullonfailure_ void** ppvObject)
{
	*ppvObject = nullptr;

	HRESULT hr = E_NOINTERFACE;
	if (IsEqualGUID2(riid, IID_IWICBitmapDecoder, IID_IUnknown))
	{
		if (*ppvObject = static_cast<IWICBitmapDecoder*>(new WmfDecoder()))
			return S_OK;
		hr = E_OUTOFMEMORY;
	}
	else if (IsEqualGUID3(riid, IID_IWICBitmapDecoderInfo, IID_IWICBitmapCodecInfo, IID_IWICComponentInfo))
	{
		if (*ppvObject = static_cast<IWICBitmapDecoderInfo*>(new WmfDecoderInfo()))
			return S_OK;
		hr = E_OUTOFMEMORY;
	}
	return hr;
}


}	// namespace