// CmnUtils.cpp

#include "pch.h"
#include "Main.h"
#include <imagehlp.h>
using namespace RootNamespace;

///////////////////////////////////////////////////////////////////////
// Constants //////////////////////////////////////////////////////////


// {AE97E52A-51BC-41FB-871A-FC9B86FB20A2}
WCXENTRY const DECLALIGN16 GUID GUID_AE_VendorID =
{ 0xae97e52a, 0x51bc, 0x41fb, { 0x87, 0x1a, 0xfc, 0x9b, 0x86, 0xfb, 0x20, 0xa2 } };


//	{28EA8895-834D-4941-8794-7EE5B5A05356}
WCXENTRY const DECLALIGN16 GUID GUID_AE_ContainerFormatSvg =
{ 0x28ea8895, 0x834d, 0x4941, { 0x87, 0x94, 0x7e, 0xe5, 0xb5, 0xa0, 0x53, 0x56 } };

// {C1D289A3-A846-47C7-9F94-E5E8F6AC4D46}
WCXENTRY const DECLALIGN16 GUID GUID_AE_ContainerFormatWmfEmf =
{ 0xc1d289a3, 0xa846, 0x47c7, { 0x9f, 0x94, 0xe5, 0xe8, 0xf6, 0xac, 0x4d, 0x46 } };

/*
//	{7B3E6D17-7B8A-4685-85AB-2BD124F2475A}
WCXENTRY const DECLALIGN16 GUID GUID_AE_ContainerFormatSvgz =
{ 0x7b3e6d17, 0x7b8a, 0x4685, { 0x85, 0xab, 0x2b, 0xd1, 0x24, 0xf2, 0x47, 0x5a } };
// {FE180EF6-23E2-412A-BE6B-3F6DF1DC4D45}
WCXENTRY const DECLALIGN16 GUID GUID_AE_ContainerFormatWmzEmz =
{ 0xfe180ef6, 0x23e2, 0x412a, { 0xbe, 0x6b, 0x3f, 0x6d, 0xf1, 0xdc, 0x4d, 0x45 } };
*/


///////////////////////////////////////////////////////////////////////
// Global items ///////////////////////////////////////////////////////


static IBindCtx* g_readBindCtx = nullptr;
static IBindCtx* g_rwriteBindCtx = nullptr;


WARNING_SUPPRESS(28196) _Check_return_ _Success_(return == S_OK)
static HRESULT GetGlobalBindCtx(_Inout_ IBindCtx** ppgbc, _In_ DWORD grfMode,
				_COM_Outptr_result_nullonfailure_ IBindCtx** ppBC)
{
	HRESULT hr;
	BIND_OPTS bopt;
	EnterGlobalCS();

	if (*ppgbc)
		goto RetOk_;

	hr = ::CreateBindCtx(0, ppgbc);
	if (S_OK != hr)
		goto Error_;

	bopt = { sizeof(BIND_OPTS), 0, grfMode, 0 };
	hr = (*ppgbc)->SetBindOptions(&bopt);
	if (SUCCEEDED(hr))
		goto RetOk_;
	(*ppgbc)->Release();
	*ppgbc = nullptr;
	goto Error_;

RetOk_:
	*ppBC = *ppgbc;
	(*ppgbc)->AddRef();
	hr = S_OK;
Leave_:
	LeaveGlobalCS();
	return hr;

Error_:
	*ppBC = nullptr;
	goto Leave_;
}

_Success_(return == S_OK)
WCXFASTAPI wcGetReadOnlyBindCtx(_COM_Outptr_result_nullonfailure_ IBindCtx** ppBC)
{
	return GetGlobalBindCtx(&g_readBindCtx, STGM_READ|STGM_SHARE_DENY_WRITE, ppBC);
}

_Success_(return == S_OK)
WCXFASTAPI wcGetReadWriteBindCtx(_COM_Outptr_result_nullonfailure_ IBindCtx** ppBC)
{
	return GetGlobalBindCtx(&g_rwriteBindCtx, STGM_READWRITE|STGM_SHARE_EXCLUSIVE, ppBC);
}


WCXSTDAPI_(void) wcReleaseComShared()
{
	EnterGlobalCS();
	__try {
		ReleaseXmlGlobals();
		ReleaseGlobalFactories();
		if (g_rwriteBindCtx)
			g_rwriteBindCtx->Release();
		if (g_readBindCtx)
			g_readBindCtx->Release();
	}
	__finally {
		g_rwriteBindCtx = nullptr;
		g_readBindCtx = nullptr;
		LeaveGlobalCS();
	}
}


///////////////////////////////////////////////////////////////////////
// DX & WIC ///////////////////////////////////////////////////////////


_Check_return_ _Success_(return == S_OK)
WCXFASTAPI wcCreateWicFactory(_COM_Outptr_result_nullonfailure_ IWICImagingFactory2** ppWicFact)
{
	const HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory2, nullptr,
		CLSCTX_INPROC_SERVER, IID_IWICImagingFactory2, PPV_ARG(ppWicFact));
	if (S_OK != hr)
		*ppWicFact = nullptr;
	return hr;
}


static constexpr inline D2D1_DEBUG_LEVEL D2DebugLevel() {
#ifdef _DEBUG
	return (::IsDebuggerPresent() ? D2D1_DEBUG_LEVEL_ERROR : D2D1_DEBUG_LEVEL_NONE);
#else
	return D2D1_DEBUG_LEVEL_NONE;
#endif
}

_Check_return_ _Success_(return == S_OK)
WCXSTDAPI wcCreateD2DFactory(BOOL multiThreaded, _COM_Outptr_result_nullonfailure_ ID2D1Factory6** ppD2D1Fact)
{
	const D2D1_FACTORY_OPTIONS opts = { D2DebugLevel() };
	multiThreaded = (multiThreaded ? D2D1_FACTORY_TYPE_MULTI_THREADED : D2D1_FACTORY_TYPE_SINGLE_THREADED);
	HRESULT hr = ::D2D1CreateFactory((D2D1_FACTORY_TYPE)multiThreaded, IID_ID2D1Factory7, &opts, PPV_ARG(ppD2D1Fact));
	if (S_OK != hr)
	{
		hr = ::D2D1CreateFactory((D2D1_FACTORY_TYPE)multiThreaded, IID_ID2D1Factory6, &opts, PPV_ARG(ppD2D1Fact));
		if (S_OK != hr)
			*ppD2D1Fact = nullptr;
	}
	return hr;
}


_Check_return_ _Success_(return == S_OK)
WCXSTDAPI wcCreateDXDevices(BOOL multiThreaded, _COM_Outptr_result_nullonfailure_ ID3D11Device** ppD3D11Device,
		_COM_Outptr_result_nullonfailure_ IDXGIDevice1** ppDXGIDevice)
{
	const D3D_FEATURE_LEVEL rgFeatureLevels[] = {  D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
	HRESULT hr = ::D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT | (multiThreaded ? 0 : D3D11_CREATE_DEVICE_SINGLETHREADED),
		rgFeatureLevels, ARRAYSIZE(rgFeatureLevels), D3D11_SDK_VERSION, ppD3D11Device, nullptr, nullptr);
	if (S_OK == hr)
	{
		hr = (*ppD3D11Device)->QueryInterface(IID_IDXGIDevice1, PPV_ARG(ppDXGIDevice));
		if (S_OK == hr)
		{
			(*ppDXGIDevice)->SetMaximumFrameLatency(1);
			return S_OK;
		}
		(*ppD3D11Device)->Release();
	}
	*ppDXGIDevice = nullptr;
	*ppD3D11Device = nullptr;
	return hr;
}


_Check_return_ WCXFASTAPI_(LONG) wcIsDXRecreateError(HRESULT hr)
{
	static const LONG32 s_rghr[] = {		
		DXGI_ERROR_DEVICE_REMOVED, DXGI_ERROR_DEVICE_RESET, DXGI_ERROR_DEVICE_HUNG,
		DXGI_ERROR_ACCESS_LOST, DXGI_ERROR_SESSION_DISCONNECTED, DXGI_ERROR_NOT_CURRENT,
		D2DERR_RECREATE_TARGET, E_ACCESSDENIED, E_OUTOFMEMORY
	};
	if (dmemchr(s_rghr, (UINT)hr, ARRAYSIZE(s_rghr)))
		return hr;
	return (D3DDDIERR_DEVICEREMOVED != hr) ? 0 : DXGI_ERROR_DEVICE_REMOVED;
}


///////////////////////////////////////////////////////////////////////
// Storage & Files ////////////////////////////////////////////////////


_Success_(return > 0)
WCXSTDAPI_(UINT) wcGetModuleName(_In_opt_ HMODULE hModule,
		_Out_writes_to_(cchMax, return+1) PWSTR szFileName, UINT cchMax)
{
	const UINT clen = GetModuleFileNameW(hModule, szFileName, cchMax);
	if (AllTrue(clen, clen < cchMax))
		return clen;
	if (cchMax > 0)
		szFileName[0] = 0;
	if (clen >= cchMax && NOERROR == GetLastError())
		SetLastError(ERROR_BUFFER_OVERFLOW);
	return 0;
}


_Check_return_ _Success_(return == S_OK)
WCXFASTAPI wcCreateStreamOnFile(_In_opt_ PCWSTR szFileName, BOOL write, _COM_Outptr_result_nullonfailure_ IStream** ppstm)
{
	HRESULT hr = S_INVALIARD;
	if (szFileName && szFileName[0])
	{
		hr = ::SHCreateStreamOnFileEx(szFileName,
			write ? (STGM_READWRITE|STGM_SHARE_EXCLUSIVE|STGM_CREATE) : (STGM_READ|STGM_SHARE_DENY_WRITE),
			FILE_ATTRIBUTE_NORMAL, write, nullptr, ppstm);
		if (S_OK == hr)
			return S_OK;
	}
	*ppstm = nullptr;
	return hr;
}

_Check_return_ _Success_(return == S_OK)
WCXSTDAPI wcCreateStreamOnItem(_In_ IShellItem* psi, BOOL write, _COM_Outptr_result_nullonfailure_ IStream** ppstm)
{
	IBindCtx* pbc;
	HRESULT hr = (write ? wcGetReadWriteBindCtx(&pbc) : wcGetReadOnlyBindCtx(&pbc));
	if (S_OK == hr)
	{
		hr = psi->BindToHandler(pbc, BHID_Stream, IID_IStream, PPV_ARG(ppstm));
		pbc->Release();
		if (S_OK == hr)
			return S_OK;
	}
	*ppstm = nullptr;
	return hr;
}


_Check_return_ _Success_(return == S_OK)
WCXFASTAPI wcCreateTempFileStream(_In_opt_ SIZE_T cbInitSize, _COM_Outptr_result_nullonfailure_ IStream** ppstm)
{
	union {
		DWORD clen;
		HRESULT hr;
		UINT64 pref64;
	};
	WCHAR wcPath[MAX_PATH - 12];
	WCHAR wcFName[MAX_PATH];

	clen = ::GetTempPathW(_countof(wcPath), wcPath);
	if (0 == clen)
		goto ErrWin_;
	if (clen >= _countof(wcPath))
	{
		hr = __HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
		goto ErrRet_;
	}
	hr = ::SHPathPrepareForWriteW(NULL, NULL, wcPath, SHPPFW_DIRCREATE);
	if (S_OK != hr)
		goto ErrRet_;
	pref64 = MAKEULONGLONGW(L'W', L'C', L'X', 0);
	if (!::GetTempFileNameW(wcPath, (PCWSTR)&pref64, 0, wcFName))
		goto ErrWin_;
	hr = ::SHCreateStreamOnFileEx(wcFName, STGM_READWRITE|STGM_SHARE_EXCLUSIVE|STGM_CREATE|STGM_DELETEONRELEASE,
			FILE_ATTRIBUTE_TEMPORARY, TRUE, nullptr, ppstm);
	if (S_OK == hr)
	{
		if (cbInitSize)
			(*ppstm)->SetSize(ToULargeInteger(cbInitSize));
		return S_OK;
	}

ErrRet_:
	*ppstm = nullptr;
	return hr;
ErrWin_:
	hr = HRESULT_WIN_ERROR;
	goto ErrRet_;
}


WARNING_SUPPRESS(6387 28196) _Check_return_ _Success_(return == S_OK)
WCXFASTAPI wcCreateTempStream(_In_opt_ SIZE_T cbInitSize, _COM_Outptr_result_nullonfailure_ IStream** ppstm)
{
	if (AllTrue(cbInitSize, cbInitSize <= HEAP_MEMORY_THRESHOLD))
	{
		const HGLOBAL hmem = ::GlobalAlloc(GMEM_MOVEABLE, cbInitSize);
		if (hmem)
		{
			if (S_OK == ::CreateStreamOnHGlobal(hmem, TRUE, ppstm))
				return S_OK;
			::GlobalFree(hmem);
		}
	}
	return wcCreateTempFileStream(cbInitSize, ppstm);
}


_Check_return_ _Success_(return == S_OK)
WCXFASTAPI wcCopyTempStream(_In_ IStream* pstmIn, _In_opt_ UINT64 cbCopy, _COM_Outptr_result_nullonfailure_ IStream** ppstmOut)
{
	union {
		STATSTG stat;
		ULARGE_INTEGER pos;
	};
	stat.cbSize.QuadPart = 0;
	HRESULT hr = pstmIn->Stat(&stat, STATFLAG_NONAME);
	if (SUCCEEDED(hr))
	{
		if (AnyTrue(0 == cbCopy, cbCopy > stat.cbSize.QuadPart))
			cbCopy = stat.cbSize.QuadPart;
		pos.QuadPart = 0;
		hr = pstmIn->Seek({ 0 }, STREAM_SEEK_CUR, &pos);
		if (SUCCEEDED(hr))
		{
			hr = ERROR_CANNOT_COPY;
			if (pos.QuadPart < cbCopy)
			{
				IStream* pout;
				cbCopy -= pos.QuadPart;
				hr = wcCreateTempStream((SIZE_T)cbCopy, &pout);
				if (S_OK == hr)
				{
					hr = pstmIn->CopyTo(pout, stat.cbSize, nullptr, nullptr);
					if (E_NOTIMPL == hr)
					{
						const UINT cbBuf = (UINT)MIN_(cbCopy, 1024 * 16);
						void* const pbuf = malloc(cbBuf);
						hr = E_OUTOFMEMORY;
						if (pbuf)
						{
							ULONG cbRead = 0, cbWrite;
							do {
								cbCopy -= cbRead;
								hr = pstmIn->Read(pbuf, cbBuf, &cbRead);
								if (FAILED(hr))
									break;
								hr = WINCODEC_ERR_STREAMREAD;
								if (0 == cbRead)
									break;
								WARNING_SUPPRESS(6385)
								hr = pout->Write(pbuf, cbRead, &cbWrite);
								if (FAILED(hr))
									break;
								hr = WINCODEC_ERR_STREAMWRITE;
								if (cbWrite != cbRead)
									break;
								hr = S_OK;
							} while (cbRead < cbCopy);
							free(pbuf);
						}
					}
					if (S_OK == hr)
					{
						hr = Stream_SeekStart(pout);
						if (SUCCEEDED(hr))
						{
							*ppstmOut = pout;
							return S_OK;
						}
					}
					pout->Release();
				}
			}
		}
	}
	*ppstmOut = nullptr;
	return hr;
}


///////////////////////////////////////////////////////////////////////
// Resources //////////////////////////////////////////////////////////


_Success_(return != 0) WCXSTDAPI_(PCVOID)
wcGetModuleResource(_In_opt_ HMODULE hModule, _In_ PCWSTR resType, _In_ UINT wID, _Out_opt_ PDWORD pcbSize)
{
	const HRSRC hrs = ::FindResourceW(hModule, MAKEINTRESOURCEW(wID), resType);
	if (hrs && (!pcbSize || (*pcbSize = SizeofResource(hModule, hrs))))
	{
		const HGLOBAL hg = ::LoadResource(hModule, hrs);
		if (hg) return ::LockResource(hg);
	}
	return nullptr;
}


typedef struct VS_VERSIONINFO {
	WORD wLength;
	WORD wValueLength;
	WORD wType;
	WCHAR szKey[17];	// L"VS_VERSION_INFO"
	VS_FIXEDFILEINFO Value;
} const * PCVS_VERSIONINFO;

_Success_(return != NULL)
WCXSTDAPI_(const VS_FIXEDFILEINFO*) wcGetModuleVersionResource(_In_ HMODULE hModule)
{
	DWORD cb;
	const PCVS_VERSIONINFO pvi = (PCVS_VERSIONINFO)wcGetModuleResource(hModule, RT_VERSION, VS_VERSION_INFO, &cb);
	if (pvi)
	{
		if (cb >= sizeof(*pvi) && AllTrue(0xFEEF04BD == pvi->Value.dwSignature,
			pvi->wValueLength >= sizeof(VS_FIXEDFILEINFO), pvi->wLength > pvi->wValueLength))
		{
			return &(pvi->Value);
		}
		SetLastError(ERROR_VERSION_PARSE_ERROR);
	}
	return nullptr;
}


_Success_(return > 0)
WCXSTDAPI_(UINT) wsFormatVersion(_In_ UINT64 ver64, _Out_writes_to_(cchMax, return+1) PWSTR szBuff, _In_ int cchMax)
{
	if (cchMax > 0)
	{
		PWSTR szNext = szBuff;
		DECLALIGN16 WCHAR wcNum[12];
		if (cchMax <= 7)
			goto Error_;
		for (INT8 i = 3; i >= 0; i--)
		{
			const UINT clen = wcUInt32ToDec((ULONG)(UINT16)(ver64 >> ((UINT8)i * 16)), wcNum, cchMax);
			if (clen >= (UINT)cchMax)
				goto Error_;
			wmemcpy(szNext, wcNum, (UINT)clen);
			szNext += clen;
			cchMax -= clen;
			if (i)
			{
				if (cchMax < 3)
					goto Error_;
				szNext[0] = L'.';
				szNext++;
				cchMax--;
			}
		}
		szNext[0] = 0;
		return (UINT)(szNext - szBuff);

	Error_:
		::SetLastError(ERROR_BUFFER_OVERFLOW);
		szBuff[0] = 0;
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////
// Module verification ////////////////////////////////////////////////


_Success_(return != 0)
WCXSTDAPI_(UINT64) wcVerifyModuleVersionResource(_In_ HMODULE hModule, _In_ UINT64 appBuildDateVer)
{
	const VS_FIXEDFILEINFO* pffi = wcGetModuleVersionResource(hModule);
	const DWORD appVersionMS = HIDWORD(appBuildDateVer);
	const DWORD appBuildDate = LODWORD(appBuildDateVer);
	TRACE("\tToday's VERSION: %hu.%hu.%hu.%hu\n",
		HIWORD(appVersionMS), LOWORD(appVersionMS), HIWORD(appBuildDate), LOWORD(appBuildDate));
	if (pffi)
	{
		const FILETIME fileTime = { pffi->dwFileDateLS, pffi->dwFileDateMS };
		SYSTEMTIME st;
		if (!FileTimeToSystemTime(&fileTime, &st) || _PACK_IDATE(st.wYear, st.wMonth, st.wDay) <= appBuildDate)
		{
			const WORD buildYear = _IDATE_YEAR(appBuildDate);
			const WORD buildMonth = _IDATE_MONTH(appBuildDate);
			if (appVersionMS == pffi->dwFileVersionMS && appVersionMS == pffi->dwProductVersionMS &&
				_IDATE_YEAR(pffi->dwFileVersionLS) == buildYear &&
				_IDATE_YEAR(pffi->dwProductVersionLS) == buildYear &&
				_IDATE_MONTH(pffi->dwFileVersionLS) == buildMonth &&
				_IDATE_MONTH(pffi->dwProductVersionLS) == buildMonth &&
				pffi->dwFileVersionLS <= appBuildDate && pffi->dwProductVersionLS <= appBuildDate)
			{
				return MAKEULONGLONG(pffi->dwProductVersionLS, appVersionMS);
			}
		}
		SetLastError(ERROR_VERSION_PARSE_ERROR);
	}
	return 0;
}


typedef DWORD (WINAPI* MAPFILEANDCHECKSUMA)(_In_ PCSTR Filename, _Out_ PDWORD HeaderSum, _Out_ PDWORD CheckSum);

_Success_(return.QuadPart != 0)
WCXFASTAPI_(ULARGE_INTEGER) wcGetModuleChecksums(_In_opt_ HMODULE hModule)
{
	ULARGE_INTEGER sums = { 0 };
	CHAR szFName[MAX_PATH];
	const UINT cch = GetModuleFileNameA(hModule, szFName, _countof(szFName));
	if (AllTrue(cch, cch < _countof(szFName)))
	{
		const HMODULE hLib = LoadLibraryW(L"imagehlp.dll");
		if (hLib)
		{
			const MAPFILEANDCHECKSUMA pfn = (MAPFILEANDCHECKSUMA)GetProcAddress(hLib, "MapFileAndCheckSumA");
			if (pfn && CHECKSUM_SUCCESS != pfn(szFName, &sums.LowPart, &sums.HighPart))
				sums.QuadPart = 0;
			FreeLibrary(hLib);
		}
	}
	return sums;
}


///////////////////////////////////////////////////////////////////////
// Misc. //////////////////////////////////////////////////////////////


WCXFASTAPI_(BOOL) wcSwitchThread(_In_opt_ UINT msSleep)
{
	const BOOL retval = SwitchToThread();
	if (!retval)
		Sleep(msSleep);
	return retval;
}


///////////////////////////////////////////////////////////////////////
// Error handling /////////////////////////////////////////////////////


_Check_return_ _Post_satisfies_(return < 0 || return == defCode) _Translates_Win32_to_HRESULT_(code)
WARNING_SUPPRESS(28196)
WCXFASTAPI wcEnsureFailHRESULT(LONG code, HRESULT defCode)
{
	return (SUCCEEDED(code) ? ((S_OK != code && S_FALSE != code) ? __HRESULT_FROM_WIN32(code) : defCode) : (HRESULT)code);
}

_Check_return_ _Post_satisfies_(return < 0 || return == defCode)
WCXFASTAPI wcLastErrorHRESULT(_In_ HRESULT defCode)
{
	defCode = wcLastErrorStatus(defCode);
	return (defCode <= 0) ? defCode : __HRESULT_FROM_WIN32(defCode);
}

_Check_return_ _Post_satisfies_(return != 0 || return == defCode)
WCXFASTAPI_(LSTATUS) wcLastErrorStatus(_In_ LSTATUS defCode)
{
	return (LSTATUS)CPP_COMMONS(EnsureErrorCode(GetLastError(), (DWORD)defCode));
}

_Post_satisfies_(return != 0 || return == defCode)
WCXFASTAPI_(DWORD) wcEnsureLastError(_In_ DWORD defCode)
{
	DWORD lerr = GetLastError();
	if (NOERROR == lerr)
	{
		lerr = defCode;
		SetLastError(defCode);
	}
	return lerr;
}