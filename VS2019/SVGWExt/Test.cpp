// Test.cpp

#include "pch.h"
#include "SVGDecoder.h"
#include "WMFDecoder.h"

#define INTERNAL_TESTS

namespace RootNamespace {

#ifdef WCX_INCLUDE_TESTS

#include <propvarutil.h>
#pragma comment(lib, "propsys.lib")

///////////////////////////////////////////////////////////////////////
// Tests //////////////////////////////////////////////////////////////


static HRESULT TestUngzipStream(_In_ PCWSTR szFileName, bool isSvg, IStream** ppstm)
{
	IStream* pstmIn;
	union {
		HRESULT hr;
		int zcode;
	};
	hr = wcCreateStreamOnFile(szFileName, FALSE, &pstmIn);
	if (S_OK == hr)
	{
		hr = wcTryUncompressStream(pstmIn, FALSE, ppstm);
		pstmIn->Release();
	}
	return hr;
}

static HRESULT TestUngzipToFile(_In_ PCWSTR szFileName, bool isSvg, _In_ PCWSTR szFileOut)
{
	IStream* pstmIn;
	HRESULT hr = TestUngzipStream(szFileName, isSvg, &pstmIn);
	if (S_OK == hr)
	{
		union {
			LARGE_INTEGER liPos;
			ULARGE_INTEGER uliSize;
		};
		hr = Stream_SeekStart(pstmIn);
		if (SUCCEEDED(hr))
		{
			IStream* pstmOut;
			hr = wcCreateStreamOnFile(szFileOut, TRUE, &pstmOut);
			if (S_OK == hr)
			{
				liPos.QuadPart = -1ll;
				hr = pstmIn->CopyTo(pstmOut, uliSize, nullptr, nullptr);
				pstmOut->Release();
				if (SUCCEEDED(hr))
				{
					hr = pstmIn->Commit(STGC_DEFAULT);
					if (IsImplSuccess(hr))
						hr = S_OK;
				}
			}
		}
		pstmIn->Release();
	}
	return hr;
}


static HRESULT TestPropStore(_In_ IShellItem2* psi, bool isSvg, _Outptr_opt_result_maybenull_ PWSTR* ppOut)
{
	if (ppOut)
		*ppOut = nullptr;

	IPropertyStore* props;
	HRESULT hr = psi->BindToHandler(NULL, BHID_PropertyStore, IID_IPropertyStore, PPV_ARG(&props));
	if (S_OK == hr)
	{
		DWORD count = 0, i;
		hr = props->GetCount(&count);
		TRACEW2(L">>>> %u properties found (0x%X)...\n", count, hr);
		if (SUCCEEDED(hr))
		{
			PROPERTYKEY key;
			PROPVARIANT val;
			PWSTR szVal;
			for (i = 0; i < count; i++)
			{
				hr = props->GetAt(i, &key);
				if (S_OK == hr)
				{
					ZeroStruct(&val);
					hr = props->GetValue(key, &val);
					if (SUCCEEDED(hr))
					{
						if (VT_EMPTY != val.vt && VT_NULL != val.vt)
						{
							hr = ::PropVariantToStringAlloc(val, &szVal);
							if (S_OK == hr)
							{
								TRACEW2(L"%u)\t'%ls'\n", i + 1, szVal);
								::CoTaskMemFree(szVal);
							}
							::PropVariantClear(&val);
						}
						else
						{
							TRACEW1(L"%u)\t[Empty]\n", i + 1);
							hr = S_OK;
						}
					}
				}
				if (S_OK != hr)
					TRACEW2(L"%u)\t[E0x%X]\n", i + 1, hr);
			}
			TRACEW1(L"<<<< end of properties (0x%X)...\n", hr);
		}
		props->Release();
	}
	else if (E_NOTIMPL == hr)
		hr = __HRESULT_FROM_WIN32(ERROR_CALL_NOT_IMPLEMENTED);
	return hr;
}

static HRESULT TestPropStore(_In_ PCWSTR szFileName, bool isSvg, _Outptr_opt_result_maybenull_ PWSTR* ppOut)
{
	IShellItem2* psi;
	HRESULT hr = ::SHCreateItemFromParsingName(szFileName, nullptr, IID_IShellItem2, PPV_ARG(&psi));
	if (S_OK == hr)
	{
		hr = TestPropStore(psi, isSvg, ppOut);
		psi->Release();
	}
	else if (ppOut)
		*ppOut = nullptr;
	return hr;
}

static HRESULT TestThumbBitmap(_In_ PCWSTR szFileName, bool isSvg, HBITMAP* phBitmap, WTS_ALPHATYPE* pwAlpha)
{
	HRESULT hr = E_INVALIDARG;
	if (!IS_INTRESOURCE(phBitmap))
	{
		IThumbnailProvider* thumbs;

	#ifdef INTERNAL_TESTS
		IInitializeWithFile* init;
		hr = (isSvg ? wcCreateSvgThumbProvider(IID_IInitializeWithFile, PPV_ARG(&init))
				: wcCreateWmfEmfThumbProvider(IID_IInitializeWithFile, PPV_ARG(&init)));
		if (S_OK == hr)
		{
			hr = init->Initialize(szFileName, STGM_READ);
			if (S_OK == hr)
			{
				hr = init->QueryInterface(IID_IThumbnailProvider, PPV_ARG(&thumbs));
				if (S_OK == hr)
				{
					
					hr = thumbs->GetThumbnail(256, phBitmap, pwAlpha);
					thumbs->Release();
				}
			}
			init->Release();
		}
	#else	// INTERNAL_TESTS
		IShellItem2* psi;
		hr = ::SHCreateItemFromParsingName(szFileName, nullptr, IID_IShellItem2, PPV_ARG(&psi));
		if (S_OK == hr)
		{
			//BHID_PropertyStore
			hr = psi->BindToHandler(NULL, BHID_ThumbnailHandler, IID_IThumbnailProvider, PPV_ARG(&thumbs));
			if (S_OK == hr)
			{
				hr = thumbs->GetThumbnail(96, phBitmap, pwAlpha);
				thumbs->Release();
			}
			else if (E_NOTIMPL == hr)
				hr = __HRESULT_FROM_WIN32(ERROR_CALL_NOT_IMPLEMENTED);
			psi->Release();
		}
	#endif	// INTERNAL_TESTS
		if (S_OK != hr)
		{
			*pwAlpha = WTSAT_UNKNOWN;
			*phBitmap = NULL;
		}
	}
	return hr;
}


static HRESULT TestThumbBitmap(_In_ PCWSTR szFileName, bool isSvg, HBITMAP* phBitmap)
{
	WTS_ALPHATYPE wAlpha;
	return TestThumbBitmap(szFileName, isSvg, phBitmap, &wAlpha);
}


static HRESULT TestThumbWicSource(_In_ PCWSTR szFileName, bool isSvg, IWICBitmapSource** ppWicSource)
{
	HRESULT hr = E_INVALIDARG;
	if (!IS_INTRESOURCE(ppWicSource))
	{
		WTS_ALPHATYPE wAlpha;
		HBITMAP hbm;
		hr = TestThumbBitmap(szFileName, isSvg, &hbm, &wAlpha);
		if (S_OK == hr)
		{
			IWICImagingFactory2* wicFact;
			hr = GetWicFactory(&wicFact);
			if (S_OK == hr)
			{
				hr = wicFact->CreateBitmapFromHBITMAP(hbm, NULL,
					(WTSAT_RGB != wAlpha) ? WICBitmapUseAlpha : WICBitmapIgnoreAlpha,
					(IWICBitmap**)ppWicSource);
				wicFact->Release();
			}
			::DeleteObject(hbm);
		}
		if (S_OK != hr)
			*ppWicSource = nullptr;
	}
	return hr;
}


static HRESULT TestWicDecoder(_In_ PCWSTR szFileName, bool isSvg, IWICBitmapDecoder** ppDecoder)
{
	HRESULT hr = E_INVALIDARG;
	if (!IS_INTRESOURCE(ppDecoder))
	{
		hr = (isSvg ? wcCreateSvgComponent(IID_IWICBitmapDecoder, PPV_ARG(ppDecoder))
				: wcCreateWmfEmfComponent(IID_IWICBitmapDecoder, PPV_ARG(ppDecoder)));
		if (S_OK == hr)
		{
			IStream* pstm;
			hr = wcCreateStreamOnFile(szFileName, FALSE, &pstm);
			if (S_OK == hr)
			{
				hr = (*ppDecoder)->Initialize(pstm, WICDecodeMetadataCacheOnDemand);
				pstm->Release();
				if (S_OK == hr)
					return S_OK;
			}
			(*ppDecoder)->Release();
		}
		*ppDecoder = nullptr;
	}
	return hr;
}


static HRESULT TestDecoderWicFrame(_In_ PCWSTR szFileName, bool isSvg, IWICBitmapFrameDecode** ppFrame)
{
	HRESULT hr = E_INVALIDARG;
	if (!IS_INTRESOURCE(ppFrame))
	{
		IWICBitmapDecoder* decoder;
		hr = TestWicDecoder(szFileName, isSvg, &decoder);
		if (S_OK == hr)
		{
			hr = decoder->GetFrame(0, ppFrame);
			decoder->Release();
			if (S_OK == hr)
				return S_OK;
		}
		*ppFrame = nullptr;
	}
	return hr;
}


static HRESULT TestSourceTransform(_In_ IWICBitmapSource* source, UINT maxSide,
		WICBitmapTransformOptions topts, IWICBitmapSource** output)
{
	IWICBitmapSourceTransform* transform;
	HRESULT hr = source->QueryInterface(IID_IWICBitmapSourceTransform, PPV_ARG(&transform));
	if (S_OK == hr)
	{
		union {
			BOOL bval;
			SIZE sizeL;
			D2D_SIZE_U sizeU;
		};
		hr = transform->DoesSupportTransform(topts, &bval);
		if (S_OK == hr)
		{
			hr = WINCODEC_ERR_UNSUPPORTEDOPERATION;
			if (bval)
			{
				hr = source->GetSize(&sizeU.width, &sizeU.height);
				if (S_OK == hr)
				{
					if (0 == maxSide)
					{
						maxSide = max(sizeU.width, sizeU.height);
						if (maxSide > 512)
							maxSide /= 2;
						else
							maxSide *= 3;
					}
					sizeL = wcScaleSizeL(sizeL, maxSide);
					hr = transform->GetClosestSize(&sizeU.width, &sizeU.height);
					if (SUCCEEDED(hr))
					{
						WICPixelFormatGUID fmtId = GUID_WICPixelFormat32bppBGRA;
						hr = transform->GetClosestPixelFormat(&fmtId);
						if (SUCCEEDED(hr))
						{
							hr = WINCODEC_ERR_UNSUPPORTEDOPERATION;
							if (GUID_WICPixelFormat32bppBGRA == fmtId || GUID_WICPixelFormat32bppPBGRA == fmtId
								|| GUID_WICPixelFormat32bppBGR == fmtId)
							{
								const bool rotated = AnyTrue(WICBitmapTransformRotate90 == topts, WICBitmapTransformRotate270 == topts);
								const D2D_SIZE_U sizeOut = { rotated ? sizeU.height : sizeU.width, rotated ? sizeU.width : sizeU.height };
								const UINT cbStride = sizeOut.width * 4;
								const UINT cbImage = cbStride * sizeOut.height;
								const PBYTE pImage = (PBYTE)malloc(cbImage);
								hr = E_OUTOFMEMORY;
								if (pImage)
								{
									hr = transform->CopyPixels(nullptr, sizeU.width, sizeU.height,
													&fmtId, topts, cbStride, cbImage, pImage);
									if (S_OK == hr)
									{
										IWICImagingFactory2* wicFact;
										hr = GetWicFactory(&wicFact);
										if (S_OK == hr)
										{
											hr = wicFact->CreateBitmapFromMemory(sizeOut.width, sizeOut.height,
													fmtId, cbStride, cbImage, pImage, (IWICBitmap**)output);
											wicFact->Release();
										}
									}
									free(pImage);
								}
							}
						}
					}
				}
			}
		}
		transform->Release();
	}
	return hr;
}


static HRESULT TestDecoderWicTransform(_In_ PCWSTR szFileName, bool isSvg, IWICBitmapSource** ppSource)
{
	HRESULT hr = E_INVALIDARG;
	if (!IS_INTRESOURCE(ppSource))
	{
		IWICBitmapFrameDecode* frame;
		hr = TestDecoderWicFrame(szFileName, isSvg, &frame);
		if (S_OK == hr)
		{
			hr = TestSourceTransform(frame, 0, WICBitmapTransformRotate90, ppSource);
			frame->Release();
			if (S_OK == hr)
				return S_OK;
		}
		*ppSource = nullptr;
	}
	return hr;
}



static HRESULT MiscTest(_In_opt_ PCWSTR szParam)
{
	return E_NOTIMPL;
}


#endif	// WCX_INCLUDE_TESTS

}	// namespace

using namespace RootNamespace;


#ifdef WCX_INCLUDE_TESTS
WCXSTDAPI wcRunDllTest(_In_opt_ HWND hWnd, UINT idMsg, _In_opt_ PCWSTR szCmdLine, LPARAM lParam)
{
	HRESULT hr = E_INVALIDARG;
	bool isSvg = false;
	if (szCmdLine)
	{
		PCWSTR szExt = ::PathFindExtensionW(szCmdLine);
		if (szExt && szExt[0])
		{
			szExt += (L'.' == szExt[0]);
			isSvg = (0 == _wcsicmp(szExt, L"svg") || 0 == _wcsicmp(szExt, L"svgz") || 0 == _wcsicmp(szExt, L"xml"));

			switch (idMsg)
			{
			case DLLT_THUMB_HBITMAP:
				hr = TestThumbBitmap(szCmdLine, isSvg, (HBITMAP*)lParam);
				break;
			case DLLT_THUMB_WICSOURCE:
				hr = TestThumbWicSource(szCmdLine, isSvg, (IWICBitmapSource**)lParam);
				break;
			case DLLT_DECODER_WICFRAME:
				hr = TestDecoderWicFrame(szCmdLine, isSvg, (IWICBitmapFrameDecode**)lParam);
				break;
			case DLLT_DECODER_WICTRANSFORM:
				hr = TestDecoderWicTransform(szCmdLine, isSvg, (IWICBitmapSource**)lParam);
				break;
			case DLLT_WICDECODER:
				hr = TestWicDecoder(szCmdLine, isSvg, (IWICBitmapDecoder**)lParam);
				break;
			case DLLT_PROPERTY_STORE:
				hr = TestPropStore(szCmdLine, isSvg, (PWSTR*)lParam);
				break;
			case DLLT_UNGZIP_STREAM:
				hr = TestUngzipStream(szCmdLine, isSvg, (IStream**)lParam);
				break;
			case DLLT_UNGZIP_FILE:
				hr = (IS_INTRESOURCE(lParam) ? E_INVALIDARG : TestUngzipToFile(szCmdLine, isSvg, (PCWSTR)lParam));
				break;
			default:
				hr = E_NOTIMPL;
				break;
			}
		}
	}
	return hr;

}
#endif	// WCX_INCLUDE_TESTS


STDAPI RunDllTest(_In_opt_ HWND hWnd, UINT idMsg, _In_opt_ PCSTR szCmdLine, LPARAM lParam)
{
#ifdef WCX_INCLUDE_TESTS
	PWSTR szwCmd = nullptr;
	union {
		UINT clen;
		HRESULT hr;
	};
	if (szCmdLine)
	{
		clen = (UINT)strlen(szCmdLine);
		if (clen)
		{
			clen = ALIGN16(clen + 7);
			szwCmd = (PWSTR)malloc(clen * sizeof(WCHAR));
			if (!szwCmd)
				return E_OUTOFMEMORY;
			clen = (UINT)MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, szCmdLine, -1, szwCmd, (int)clen);
			if ((int)clen <= 0)
			{
				hr = HRESULT_WIN_ERROR;
				free(szwCmd);
				return hr;
			}
		}
	}
	hr = wcRunDllTest(hWnd, idMsg, szwCmd, lParam);
	free(szwCmd);
	return hr;
#else
	return E_NOTIMPL;
#endif
}