// Test.cpp

#include "pch.h"
#include "SVGDecoder.h"
#include "WMFDecoder.h"

namespace RootNamespace {

#ifdef WCX_ENABLE_LOG
///////////////////////////////////////////////////////////////////////
// Log ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#include <stdio.h>

#define LOG_SHORT_FILE_NAME	L"SVGWExt.log"
#define LOG_FILE_NAME		LOG_SHORT_FILE_NAME

static FILE* g_logFile = nullptr;
static CRITICAL_SECTION g_logCS;


bool IsDllLogging()
{
	return ToBool(g_logFile);
}


#ifdef WCX_LOG_APPEND
#define fsopenMode	L"at"
static bool __fastcall IsLogFileValid(_In_ PCWSTR szFileName)
{
	const DWORD attr = ::GetFileAttributesW(szFileName);
	return AnyTrue(INVALID_FILE_ATTRIBUTES == attr, !(attr & FILE_ATTRIBUTE_DIRECTORY));
}
#else
#define fsopenMode	L"wt"
static bool __fastcall IsLogFileValid(_In_ PCWSTR szFileName)
{
	return (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(szFileName));
}
#endif

NOINLINE HRESULT __fastcall wcLogCreate(_In_opt_ PCWSTR szFileName)
{
	HRESULT hr = S_OK;
	UINT clen;
	union {
		WCHAR wczFName[SAFE_PATH];
		CHAR czModule[sizeof(wczFName)];
		SYSTEMTIME st;
	};

	wcInitCommonCS(&g_logCS);
	::EnterCriticalSection(&g_logCS);

	if (!(szFileName && szFileName[0]))
	{
		PWSTR pwzFolder;
		hr = ::SHGetKnownFolderPath(FOLDERID_Desktop, KF_FLAG_CREATE|KF_FLAG_INIT|KF_FLAG_NO_ALIAS, NULL, &pwzFolder);
		if (S_OK != hr)
			goto Leave_;
		//WCX_LOG_APPEND
		if (!::PathCombineW(wczFName, pwzFolder, LOG_FILE_NAME) || !IsLogFileValid(wczFName))
		{
			if (!::PathMakeUniqueName(wczFName, _countof(wczFName), LOG_SHORT_FILE_NAME, LOG_FILE_NAME, pwzFolder))
			{
				hr = HRESULT_WIN_ERROR;
				ASSUME(FAILED(hr));
			}
		}
		::CoTaskMemFree(pwzFolder);
		if (S_OK != hr)
			goto Leave_;
		szFileName = wczFName;
	}
	else
	{
		hr = ::SHPathPrepareForWriteW(NULL, nullptr, szFileName, SHPPFW_DIRCREATE|SHPPFW_IGNOREFILENAME);
		if (FAILED(hr))
			goto Leave_;
	}

	g_logFile = _wfsopen(szFileName, fsopenMode, SH_DENYWR);
	if (!g_logFile)
	{
		hr = HRESULT_WIN_ERROR;
		goto Leave_;
	}
	fputs("\r\n", g_logFile);
	clen = ::GetModuleFileNameA(g_hModule, czModule, _countof(czModule));
	if (clen && clen < _countof(czModule))
		fwrite(czModule, sizeof(czModule[0]), clen, g_logFile);
	::GetLocalTime(&st);
	if (fprintf(g_logFile, "\r\n------- %.2u:%.2u:%.2u ---------------\r\n",
		(UINT)st.wHour, (UINT)st.wMinute, (UINT)st.wSecond) > 0)
	{
		fflush(g_logFile);
		hr = S_OK;
	}

Leave_:
	::LeaveCriticalSection(&g_logCS);
	return hr;
}

WCXFASTAPI wcLogOpen(_In_opt_ PCWSTR szFileName)
{
	if (g_logFile)
		return S_FALSE;

	const BOOL inCS = TryEnterGlobalCS();
	const HRESULT hr = wcLogCreate(szFileName);
	if (inCS)
		LeaveGlobalCS();
	return hr;
}


static HRESULT __fastcall LogWriteTime_(_In_ const SYSTEMTIME& st)
{
	return (fprintf(g_logFile, "[%.2u:%.2u:%.2u]  ", (UINT)st.wHour, (UINT)st.wMinute, (UINT)st.wSecond) > 0)
			? S_OK : HRESULT_WIN_ERROR;
}

static NOINLINE HRESULT LogWriteTime_()
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	return LogWriteTime_(st);
}

template <class _TC>
static HRESULT __fastcall LogPreWrite_(_Inout_ const _TC** pszText, bool line)
{
	HRESULT hr = wcLogOpen(nullptr);
	if (SUCCEEDED(hr))
	{
		_TC ch = (*pszText)[0];
		if ('!' == ch)
		{
			(*pszText)++;
			ch = (*pszText)[0];
			line = false;
		}
		else if ('@' == ch)
			line = true;
		
		::EnterCriticalSection(&g_logCS);
		if ('@' == ch)
		{
			(*pszText)++;
			ch = (*pszText)[0];
		}
		if (line)
		{
			hr = LogWriteTime_();
			if (FAILED(hr))
			{
				::LeaveCriticalSection(&g_logCS);
				return hr;
			}
		}
		hr = S_OK;
		if (0 == ch)
		{
			::LeaveCriticalSection(&g_logCS);
			hr = S_FALSE;
		}
	}
	WARNING_SUPPRESS(26115)
	return hr;
}

static HRESULT __fastcall LogPreWrite(_Inout_ PCSTR* pszText, bool line)
{
	return LogPreWrite_(pszText, line);
}

static HRESULT __fastcall LogPreWrite(_Inout_ PCWSTR* pszText, bool line)
{
	return LogPreWrite_(pszText, line);
}


WCXFASTAPI wcLogWrite(_In_opt_ PCSTR szText)
{
	HRESULT hr = LogPreWrite(&szText, false);
	if (S_OK == hr) __try
	{
		if (fputs(szText, g_logFile) < 0)
			hr = HRESULT_WIN_ERROR;
	}
	__finally {
		::LeaveCriticalSection(&g_logCS);
	}
	return hr;
}

WCXFASTAPI wcLogWriteW(_In_opt_ PCWSTR szText)
{
	HRESULT hr = LogPreWrite(&szText, false);
	if (S_OK == hr) __try
	{
		if (fputws(szText, g_logFile) < 0)
			hr = HRESULT_WIN_ERROR;
	}
	__finally {
		::LeaveCriticalSection(&g_logCS);
	}
	return hr;
}


WCXFASTAPI wcLogWriteLn(_In_opt_ PCSTR szText)
{
	HRESULT hr = LogPreWrite(&szText, true);
	if (S_OK == hr) __try
	{
		if (fputs(szText, g_logFile) >= 0 && fputs("\r\n", g_logFile) >= 0)
		{
			fflush(g_logFile);
			ASSUME(S_OK == hr);
		}
		else hr = HRESULT_WIN_ERROR;
	}
	__finally {
		::LeaveCriticalSection(&g_logCS);
	}
	return hr;
}

WCXFASTAPI wcLogWriteLnW(_In_opt_ PCWSTR szText)
{
	HRESULT hr = LogPreWrite(&szText, true);
	if (S_OK == hr) __try
	{
		if (fputws(szText, g_logFile) >= 0 && fputs("\r\n", g_logFile) >= 0)
		{
			fflush(g_logFile);
			ASSUME(S_OK == hr);
		}
		else hr = HRESULT_WIN_ERROR;
	}
	__finally {
		::LeaveCriticalSection(&g_logCS);
	}
	return hr;
}


WCXCAPI wcLogFormat(_In_opt_ PCSTR szFmt, ...)
{
	va_list Args;
	va_start(Args, szFmt);
	const HRESULT hr = wcLogFormatV(szFmt, Args);
	va_end(Args);
	return hr;
}

WCXFASTAPI wcLogFormatV(_In_opt_ PCSTR szFmt, va_list Args)
{
	HRESULT hr = LogPreWrite(&szFmt, false);
	if (S_OK == hr) __try
	{
		if (vfprintf(g_logFile, szFmt, Args) < 0)
			hr = HRESULT_WIN_ERROR;
	}
	__finally {
		::LeaveCriticalSection(&g_logCS);
	}
	return hr;
}


WCXCAPI wcLogFormatLn(_In_opt_ PCSTR szFmt, ...)
{
	va_list Args;
	va_start(Args, szFmt);
	const HRESULT hr = wcLogFormatLnV(szFmt, Args);
	va_end(Args);
	return hr;
}

WCXFASTAPI wcLogFormatLnV(_In_opt_ PCSTR szFmt, va_list Args)
{
	HRESULT hr = LogPreWrite(&szFmt, true);
	if (S_OK == hr) __try
	{
		if (vfprintf(g_logFile, szFmt, Args) >= 0 && fputs("\r\n", g_logFile) >= 0)
		{
			fflush(g_logFile);
			ASSUME(S_OK == hr);
		}
		else hr = HRESULT_WIN_ERROR;
	}
	__finally {
		::LeaveCriticalSection(&g_logCS);
	}
	return hr;
}


static HRESULT __fastcall LogWriteStreamInfo_(_In_opt_ IStream* pstm)
{
	HRESULT hr = S_FALSE;
	if (pstm)
	{
		STATSTG stat = { nullptr };
		hr = pstm->Stat(&stat, STATFLAG_DEFAULT);
		if (SUCCEEDED(hr))
		{
			const UINT64 cbSize = ToUInt64(stat.cbSize);
			SYSTEMTIME st = { 0 };
			::FileTimeToSystemTime(&stat.mtime, &st);
			hr = (fprintf(g_logFile, "'%S' (%.2f KB, %u bytes; %.4u-%.2u-%.2u %.2u:%.2u:%.2u) ",
				stat.pwcsName, (float)(cbSize / 1024.), (UINT)cbSize,
				(UINT)st.wYear, (UINT)st.wMonth, (UINT)st.wDay,
				(UINT)st.wHour, (UINT)st.wMinute, (UINT)st.wSecond) >= 0)
				? S_OK : HRESULT_WIN_ERROR;
			::CoTaskMemFree(stat.pwcsName);
		}
	}
	return hr;
}

WCXCAPI wcLogFormatStat(_In_opt_ IStream* pstm, _In_opt_ PCSTR szFmt, ...)
{
	va_list Args;
	va_start(Args, szFmt);
	HRESULT hr = LogPreWrite(&szFmt, false);
	if (S_OK == hr) __try
	{
		hr = LogWriteStreamInfo_(pstm);
		if (SUCCEEDED(hr))
			hr = (vfprintf(g_logFile, szFmt, Args) >= 0) ? S_OK : HRESULT_WIN_ERROR;
	}
	__finally {
		::LeaveCriticalSection(&g_logCS);
		va_end(Args);
	}
	return hr;
}

void wcLogClose()
{
	if (g_logFile)
	{
		::EnterCriticalSection(&g_logCS);
		if (g_logFile)
		{
			fflush(g_logFile);
			fclose(g_logFile);
		}
		::LeaveCriticalSection(&g_logCS);
		::DeleteCriticalSection(&g_logCS);
	}
}


#endif	// WCX_ENABLE_LOG


#ifdef WCX_INCLUDE_TESTS
///////////////////////////////////////////////////////////////////////
// Tests //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


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
		hr = wcUncompressStream(pstmIn, FALSE, ppstm);
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
		hr = pstmIn->Seek({ 0 }, STREAM_SEEK_SET, nullptr);
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


static HRESULT TestThumbBitmap(_In_ PCWSTR szFileName, bool isSvg, HBITMAP* phBitmap, WTS_ALPHATYPE* pwAlpha)
{
	HRESULT hr = E_INVALIDARG;
	if (!IS_INTRESOURCE(phBitmap))
	{
		IInitializeWithFile* init;
		hr = (isSvg ? wcCreateSvgThumbProvider(IID_IInitializeWithFile, PPV_ARG(&init))
				: wcCreateWmfEmfThumbProvider(IID_IInitializeWithFile, PPV_ARG(&init)));
		if (S_OK == hr)
		{
			hr = init->Initialize(szFileName, STGM_READ);
			if (S_OK == hr)
			{
				IThumbnailProvider* thumbs;
				hr = init->QueryInterface(IID_IThumbnailProvider, PPV_ARG(&thumbs));
				if (S_OK == hr)
				{
					hr = thumbs->GetThumbnail(256, phBitmap, pwAlpha);
					thumbs->Release();
				}
			}
			init->Release();
		}
		if (S_OK != hr)
			*phBitmap = NULL;
	}
	return hr;
}


static HRESULT TestThumbBitmap(_In_ PCWSTR szFileName, bool isSvg, HBITMAP* phBitmap)
{
	HRESULT hr = E_INVALIDARG;
	if (!IS_INTRESOURCE(phBitmap))
	{
		IInitializeWithFile* init;
		hr = (isSvg ? wcCreateSvgThumbProvider(IID_IInitializeWithFile, PPV_ARG(&init))
				: wcCreateWmfEmfThumbProvider(IID_IInitializeWithFile, PPV_ARG(&init)));
		if (S_OK == hr)
		{
			hr = init->Initialize(szFileName, STGM_READ);
			if (S_OK == hr)
			{
				IThumbnailProvider* thumbs;
				hr = init->QueryInterface(IID_IThumbnailProvider, PPV_ARG(&thumbs));
				if (S_OK == hr)
				{
					WTS_ALPHATYPE wAlpha;
					hr = thumbs->GetThumbnail(256, phBitmap, &wAlpha);
					thumbs->Release();
				}
			}
			init->Release();
		}
		if (S_OK != hr)
			*phBitmap = NULL;
	}
	return hr;
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
			isSvg = (0 == lstrcmpiW(szExt, L"svg") || 0 == lstrcmpiW(szExt, L"svgz") || 0 == lstrcmpiW(szExt, L"xml"));

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
			case DLLT_UNGZIP_STREAM:
				hr = TestUngzipStream(szCmdLine, isSvg, (IStream**)lParam);
				break;
			case DLLT_UNGZIP_FILE:
				hr = (IS_INTRESOURCE(lParam) ? E_INVALIDARG : TestUngzipToFile(szCmdLine, isSvg, (PCWSTR)lParam));
				break;
			case DLLT_NULL:
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