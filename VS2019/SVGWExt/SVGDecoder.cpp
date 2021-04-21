// SVGDecoder.cpp

#include "pch.h"
#include "SVGDecoder.h"

#pragma warning(disable: 6387 6388 28196)

namespace RootNamespace {

///////////////////////////////////////////////////////////////////////
// SVG decoder ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


_Success_(return == S_OK) NOALIAS
HRESULT InitLoadSvg(_In_ IStream* pstm, _COM_Outptr_result_nullonfailure_ ID2D1DeviceContext5** ppDC,
		_COM_Outptr_result_nullonfailure_ ID2D1SvgDocument** ppSvg, _Out_ D2D_SIZE_F* pSize)
{
	/*CHAR _steps[256];
	_steps[0] = 0;*/

	IStream* pstmInf;
	HRESULT hr = wcUncompressStream(pstm, FALSE, &pstmInf);
	if (S_OK == hr)
		pstm = pstmInf;
	else if (IsUnknownImage(hr))
		pstm->AddRef();
	else
		goto Error_;

	//strcpy(_steps, ">Begin");
	hr = CreateD2DC(ppDC);
	if (S_OK == hr)
	{
		//strcat(_steps, " CreateD2DC");
		hr = (*ppDC)->CreateSvgDocument(pstm, InitialSvgSize(), ppSvg);
		if (S_OK == hr)
		{
			//strcat(_steps, " CreateSvgDocument");
			hr = wcUpdateSvgSize(*ppSvg, true, pSize);
			if (S_OK == hr)
			{
				//strcat(_steps, " wcUpdateSvgSize...OK!");
				pstm->Release();
				return S_OK;
			}
			(*ppSvg)->Release();
		}
		(*ppDC)->Release();
	}
	pstm->Release();

Error_:
	//wcLogWriteLn("SVG load error");
	//wcLogFormatStat(pstm, ": 0x%.8X (%s)\r\n", hr, _steps);
	Zero8Bytes(pSize);
	*ppSvg = nullptr;
	*ppDC = nullptr;
	return hr;
}


HRESULT SvgDecoder::QueryCapability(__RPC__in_opt IStream* pIStream, __RPC__out DWORD* pdwCapability)
{
	HRESULT hr = E_INVALIDARG;
	if (pdwCapability)
	{
		*pdwCapability = 0;
		if (pIStream)
		{
			hr = wcMatchesSvgPattern(pIStream);
			if (S_OK == hr)
				*pdwCapability = WICBitmapDecoderCapabilityCanDecodeAllImages|WICBitmapDecoderCapabilityCanDecodeThumbnail;
			else if (SUCCEEDED(hr))
				hr = WINCODEC_ERR_UNKNOWNIMAGEFORMAT;
		}
	}
	return hr;
}


HRESULT SvgDecoder::Initialize(__RPC__in_opt IStream* pIStream, WICDecodeOptions cacheOptions)
{
	HRESULT hr = E_INVALIDARG;
	if (pIStream)
	{
		EnterCS();
		hr = WINCODEC_ERR_WRONGSTATE;
		__try
		{
			if (IsClear())
			{
				hr = InitLoadSvg(pIStream, &m_d2dDC, &m_svgDoc, &m_sizeDips);
				if (S_OK == hr)
					m_screenDpi = GetScreenDpi();
				else if (AnyTrue(SUCCEEDED(hr), E_FAIL == hr))
					hr = WINCODEC_ERR_UNKNOWNIMAGEFORMAT;
			}
		}
		__finally
		{
			LeaveCS();
		}	
	}
	return hr;
}


HRESULT SvgDecoder::GetContainerFormat(__RPC__out GUID* pguidContainerFormat)
{
	if (pguidContainerFormat)
	{
		CopyGUID(pguidContainerFormat, GUID_AE_ContainerFormatSvg);
		return S_OK;
	}
	return E_INVALIDARG;
}


HRESULT SvgDecoder::GetDecoderInfo(__RPC__deref_out_opt IWICBitmapDecoderInfo** ppIDecoderInfo)
{
	HRESULT hr = E_INVALIDARG;
	if (ppIDecoderInfo)
	{
		if (*ppIDecoderInfo = static_cast<IWICBitmapDecoderInfo*>(new SvgDecoderInfo()))
			return S_OK;
		hr = E_OUTOFMEMORY;
	}
	return hr;
}


HRESULT SvgDecoder::GetThumbnail(__RPC__deref_out_opt IWICBitmapSource** ppIThumbnail)
{
	return CreateThumbnail(CalcThumbSize(), D2D1::Matrix3x2F::Identity(), ppIThumbnail);
}


HRESULT SvgDecoder::QueryCopyPixelTransform_(D2D_SIZE_U reqSize, WICBitmapTransformOptions wicTransform,
		UINT angleRotated, _Inout_ D2D_MATRIX_3X2_F* pMatrix) const
{
	return S_FALSE;
}


_Success_(return == S_OK) HRESULT SvgDecoder::CreateCpuReadBitmap_(D2D_SIZE_F drawSize, D2D_SIZE_U targSize,
		_In_ const D2D1_MATRIX_3X2_F& transform, _COM_Outptr_ ID2D1Bitmap1** ppbmMap) const
{
	return CreateSvgCpuReadBitmap(m_d2dDC, m_svgDoc, drawSize, targSize, transform, ppbmMap);
}


void SvgDecoder::Clear()
{
	SafeRelease(&m_svgDoc);
	__super::Clear();
}


_Success_(return == S_OK)
WCXSTDAPI wcSvgToCpuBitmap(_In_opt_ ID2D1DeviceContext5* d2dDC, _In_ ID2D1SvgDocument* svgDoc, D2D_SIZE_F drawSize,
		D2D_SIZE_U targSize, _In_opt_ const D2D1_MATRIX_3X2_F* transform, _COM_Outptr_result_nullonfailure_ ID2D1Bitmap1** ppbm)
{
	*ppbm = nullptr;

	HRESULT hr = D2DERR_NO_HARDWARE_DEVICE;
	if (d2dDC)
	{
		ID2D1Bitmap1* pbmTarg;
		D2D1_BITMAP_PROPERTIES1 props = { { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED },
			96, 96, D2D1_BITMAP_OPTIONS_TARGET, nullptr };
		hr = d2dDC->CreateBitmap(targSize, nullptr, 0, &props, &pbmTarg);
		if (S_OK == hr)
		{
			ID2D1Image* oldTarg = nullptr;
			d2dDC->GetTarget(&oldTarg);
			d2dDC->SetTarget(pbmTarg);
			svgDoc->SetViewportSize(drawSize);
			d2dDC->BeginDraw();
			d2dDC->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			if (transform)
				d2dDC->SetTransform(transform);
			D2D1_COLOR_F clr;
			Zero16Bytes(&clr);
			d2dDC->Clear(&clr);
			d2dDC->DrawSvgDocument(svgDoc);
			hr = d2dDC->EndDraw();
			d2dDC->SetTarget(oldTarg);
			if (S_OK == hr)
			{
				ID2D1Bitmap1* pbmCpu;
				props.bitmapOptions = D2D1_BITMAP_OPTIONS_CPU_READ|D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
				hr = d2dDC->CreateBitmap(targSize, nullptr, 0, &props, &pbmCpu);
				if (S_OK == hr)
				{
					//const D2D_RECT_U rcSrc = { 0, 0, targSize.width, targSize.height };
					//const D2D_POINT_2U ptDest = { 0, 0 };
					hr = pbmCpu->CopyFromBitmap(nullptr /*&ptDest*/, pbmTarg, nullptr /*&rcSrc*/);
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


HRESULT SvgDecoderInfo::GetCLSID(__RPC__out CLSID* pclsid)
{
	if (pclsid)
	{
		CopyGUID(pclsid, CLSID_SvgDecoder);
		return S_OK;
	}
	return E_INVALIDARG;
}


HRESULT SvgDecoderInfo::GetFriendlyName(UINT cchFriendlyName,
	__RPC__inout_ecount_full_opt(cchFriendlyName) WCHAR* wzFriendlyName, __RPC__out UINT* pcchActual)
{
	WCHAR wcName[ALIGN4(_countof(SVG_DECODER_NAMEA) + 1)];
	return ReturnInfoRealString(wcName, wcAsciiToWide(SVG_DECODER_NAMEA, _countof(wcName), wcName),
			cchFriendlyName, wzFriendlyName, pcchActual);
}


HRESULT SvgDecoderInfo::GetContainerFormat(__RPC__out GUID* pguidContainerFormat)
{
	if (pguidContainerFormat)
	{
		CopyGUID(pguidContainerFormat, GUID_AE_ContainerFormatSvg);
		return S_OK;
	}
	return E_INVALIDARG;
}


HRESULT SvgDecoderInfo::CreateInstance(__RPC__deref_out_opt IWICBitmapDecoder** ppIBitmapDecoder)
{
	if (ppIBitmapDecoder)
	{
		IWICBitmapDecoder* const decoder = new SvgDecoder();
		*ppIBitmapDecoder = decoder;
		return (decoder ? S_OK : E_OUTOFMEMORY);
	}
	return E_INVALIDARG;
}


PCWSTR SvgDecoderInfo::GetMimeTypes(_Out_ PUINT pcch) const
{
	*pcch = _countof(SVG_WIC_MIME_TYPES) - 1;
	return SVG_WIC_MIME_TYPES;
}

PCWSTR SvgDecoderInfo::GetExtensions(_Out_ PUINT pcch) const
{
	*pcch = _countof(SVG_WIC_EXTENSIONS) - 1;
	return SVG_WIC_EXTENSIONS;
}


HRESULT SvgDecoderInfo::MatchesPattern(__RPC__in_opt IStream* pIStream, __RPC__out BOOL* pfMatches)
{
	if (pfMatches)
	{
		if (pIStream)
		{
			const HRESULT hr = wcMatchesSvgPattern(pIStream);
			*pfMatches = (S_OK == hr);
			return (IsUnknownImage(hr) ? S_OK : hr);
		}
		*pfMatches = FALSE;
	}
	return E_INVALIDARG;
}


WCXFASTAPI wcCreateSvgComponent(_In_ REFIID riid, _COM_Outptr_result_nullonfailure_ void** ppvObject)
{
	*ppvObject = nullptr;

	HRESULT hr = E_NOINTERFACE;
	if (IsEqualGUID2(riid, IID_IWICBitmapDecoder, IID_IUnknown))
	{
		if (*ppvObject = static_cast<IWICBitmapDecoder*>(new SvgDecoder()))
			return S_OK;
		hr = E_OUTOFMEMORY;
	}
	else if (IsEqualGUID3(riid, IID_IWICBitmapDecoderInfo, IID_IWICBitmapCodecInfo, IID_IWICComponentInfo))
	{
		if (*ppvObject = static_cast<IWICBitmapDecoderInfo*>(new SvgDecoderInfo()))
			return S_OK;
		hr = E_OUTOFMEMORY;
	}
	return hr;
}


}	// namespace