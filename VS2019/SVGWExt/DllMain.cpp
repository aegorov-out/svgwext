// Main.cpp

#include "pch.h"
#include "Main.h"


extern "C" HMODULE g_hModule = NULL;


namespace RootNamespace {

// Globals ////////////////////////////////////////////////////////////

enum : UINT8 {
	INITF_CSINIT = 0x01, INITINF_WOW64 = 0x10, INITINF_WINVER_OK = 0x40, INITINF_MODULE_OK = 0x80
};

static volatile UINT8 g_initFlags = 0;
volatile ULONG g_dllRefCount = 0;
DWORD g_AppBuildDate = 0;
MVERSION64 g_AppVersion = { 0 };

static DECLALIGN16 CRITICAL_SECTION g_CS;


// {0FF41E38-A793-45FF-9F38-5278DC444D57}
const CLSID DECLALIGN16 CLSID_SvgDecoder =
{ 0xff41e38, 0xa793, 0x45ff, { 0x9f, 0x38, 0x52, 0x78, 0xdc, 0x44, 0x4d, 0x57 } };
// {9DDB0D52-BDEE-4451-903D-27B29FA407B0}
const CLSID DECLALIGN16 CLSID_WmfEmfDecoder =
{ 0x9ddb0d52, 0xbdee, 0x4451, { 0x90, 0x3d, 0x27, 0xb2, 0x9f, 0xa4, 0x7, 0xb0 } };
// {A3699533-14C8-4D18-80A4-3484B11EDC85}
const CLSID DECLALIGN16 CLSID_SvgThumbnailProvider =
{ 0xa3699533, 0x14c8, 0x4d18, { 0x80, 0xa4, 0x34, 0x84, 0xb1, 0x1e, 0xdc, 0x85 } };
// {ACDB5DAE-C8D0-4443-A86D-CE94E679670C}
const CLSID DECLALIGN16 CLSID_WmfEmfThumbnailProvider =
{ 0xacdb5dae, 0xc8d0, 0x4443, { 0xa8, 0x6d, 0xce, 0x94, 0xe6, 0x79, 0x67, 0xc } };


}	// namespace
///////////////////////////////////////////////////////////////////////
// Entry point ////////////////////////////////////////////////////////
using namespace RootNamespace;


extern "C" BOOL WINAPI DllMain(_In_ HINSTANCE hinstDll, DWORD fdwReason, LPVOID staticLoad)
{
	if (DLL_PROCESS_ATTACH == fdwReason)
	{
		g_hModule = hinstDll;
	#ifndef _DEBUG
		DisableThreadLibraryCalls(hinstDll);
	#endif
		DEBUG_INITIALIZE;
	#ifndef _WIN64
		BOOL bVal = FALSE;
		if (::IsWow64Process(GetCurrentProcess(), &bVal))
			g_initFlags |= INITINF_WOW64;
	#endif
		wcInitCommonCS(&g_CS);
		g_initFlags |= INITF_CSINIT;
	#if defined(WCX_ENABLE_LOG) && !defined(_DEBUG)
		if (!staticLoad)
			wcLogCreate();
	#endif
	}
	else if (DLL_PROCESS_DETACH == fdwReason)
	{
		if (g_initFlags & INITF_CSINIT)
		{
			if (!staticLoad)
				wcReleaseComShared();
			//FreeThisModuleName();
			g_initFlags &= ~INITF_CSINIT;
			DeleteCriticalSection(&g_CS);
		}
	#ifdef WCX_ENABLE_LOG
		wcLogClose();
	#endif
	}
	return TRUE;
}


STDAPI DllGetVersion(_Inout_ DLLVERSIONINFO* pdvi)
{
	if (IS_INTRESOURCE(pdvi) || pdvi->cbSize < sizeof(DLLVERSIONINFO))
		return E_INVALIDARG;
	pdvi->dwMajorVersion = g_AppVersion.Major;
	pdvi->dwMinorVersion = g_AppVersion.Minor;
	pdvi->dwBuildNumber  = g_AppVersion.Build;
	pdvi->dwPlatformID = DLLVER_PLATFORM_NT;
	if (sizeof(DLLVERSIONINFO) == pdvi->cbSize)
		return S_OK;
	ZeroMemory(reinterpret_cast<PBYTE>(pdvi) + sizeof(pdvi->cbSize), pdvi->cbSize - sizeof(pdvi->cbSize));
	pdvi->cbSize = sizeof(DLLVERSIONINFO);
	return S_FALSE;
}


namespace RootNamespace {	// ++++++++++++++++++++++++++++++++++++++++


static NOINLINE HRESULT VerifyWinVersion()
{
	if (g_initFlags & INITINF_WINVER_OK)
		return S_OK;

	OSVERSIONINFOEXW vi;
	vi.dwOSVersionInfoSize = sizeof(vi);
	vi.dwMajorVersion = 10;
	vi.dwMinorVersion = 0;
	vi.dwBuildNumber = 15063;
	if (::VerifyVersionInfoW(&vi, VER_MAJORVERSION|VER_BUILDNUMBER,
		::VerSetConditionMask(::VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL),
			VER_BUILDNUMBER, VER_GREATER_EQUAL)))
	{
		g_initFlags |= INITINF_WINVER_OK;
		return S_OK;
	}
	return __HRESULT_FROM_WIN32(ERROR_OLD_WIN_VERSION);
}

static HRESULT CheckBuildIntegrity()
{
	if (!(g_initFlags & INITINF_MODULE_OK))
	{
		g_AppBuildDate = __IDATE__;
		g_AppVersion.Version64 = QueryVersionResource(APPVER_MAJOR, APPVER_MINOR);
		if (0 == g_AppVersion.Version64)
			return __HRESULT_FROM_WIN32(ERROR_VERSION_PARSE_ERROR);

		const ULARGE_INTEGER sums = wcGetModuleChecksums(g_hModule);
		if (AllTrue(sums.LowPart, sums.HighPart, sums.LowPart != sums.HighPart))
			return __HRESULT_FROM_WIN32(ERROR_DATA_CHECKSUM_ERROR);

		g_initFlags |= INITINF_MODULE_OK;
	}
	return S_OK;
	
}

_Check_return_ HRESULT VerifyVersion()
{
	HRESULT hr = VerifyWinVersion();
	if (SUCCEEDED(hr))
	{
		hr = CheckBuildIntegrity();
		if (SUCCEEDED(hr))
			return S_OK;
	}
	return hr;
}


#ifndef _WIN64
_Check_return_ NOALIAS BOOLEAN IsOnWow64()
{
	return (g_initFlags & INITINF_WOW64);
}
#endif


///////////////////////////////////////////////////////////////////////
// Global resources ///////////////////////////////////////////////////


NOALIAS BOOL TryEnterGlobalCS()
{
	return (g_initFlags & INITF_CSINIT) ? TryEnterCriticalSection(&g_CS) : FALSE;
}

NOALIAS void EnterGlobalCS()
{
	if (g_initFlags & INITF_CSINIT)
		EnterCriticalSection(&g_CS);
}

NOALIAS void LeaveGlobalCS()
{
	if (g_initFlags & INITF_CSINIT)
		LeaveCriticalSection(&g_CS);
}


}	// namespace