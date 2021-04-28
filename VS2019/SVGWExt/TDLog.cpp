// TDLog.cpp

#include "pch.h"
#include "Main.h"
#include <stdio.h>

namespace RootNamespace {
#ifdef WCX_ENABLE_LOG
///////////////////////////////////////////////////////////////////////
// Log ////////////////////////////////////////////////////////////////

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
	if (INVALID_FILE_ATTRIBUTES == attr)
		return true;
	if (!(attr & FILE_ATTRIBUTE_DIRECTORY))
	{
		const HANDLE hf = ::CreateFileW(szFileName, GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, 0, NULL);
		if (INVALID_HANDLE_VALUE != hf)
		{
			::CloseHandle(hf);
			return true;
		}
	}
	return false;
}
#else
#define fsopenMode	L"wt"
static bool __fastcall IsLogFileValid(_In_ PCWSTR szFileName)
{
	return (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(szFileName));
}
#endif

static NOINLINE HRESULT GetLogFolder(_Outptr_ PWSTR* ppszPath)
{
	HRESULT hr;
#ifndef _DEBUG
	hr = ::SHGetKnownFolderPath(FOLDERID_Desktop, KF_FLAG_CREATE|KF_FLAG_INIT|KF_FLAG_NO_ALIAS, NULL, ppszPath);
	if (S_OK == hr)
		return hr;
#endif
	const PWSTR szBuff = (PWSTR)::CoTaskMemAlloc(MAX_PATH * sizeof(WCHAR));
	hr = E_OUTOFMEMORY;
	if (szBuff)
	{
		const UINT cch = wcGetModuleName(g_hModule, szBuff, MAX_PATH);
		if (cch)
		{
			const PWSTR szTitle = wcFindFileName(szBuff, (int)cch);
			szTitle[0] = 0;
			if (!(*ppszPath = (PWSTR)::CoTaskMemRealloc(szBuff, (szTitle - szBuff + 1) * sizeof(WCHAR))))
			{
				WARNING_SUPPRESS(6001)
				*ppszPath = szBuff;
			}
			return S_OK;
		}
		else hr = HRESULT_WIN_ERROR;
		::CoTaskMemFree(szBuff);
	}
	return hr;
}


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
		hr = GetLogFolder(&pwzFolder);
		if (S_OK != hr)
			goto Leave_;
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
	if (fprintf(g_logFile, "\r\n>>>>>>> %.2u-%.2u-%.2u %.2u:%.2u:%.2u >>>>>>>\r\n",
		(UINT)st.wYear % 100, (UINT)st.wMonth, (UINT)st.wDay,
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
			hr = HRESULT_INVALID;
			__try {
				ASSUME(FAILED(hr));
				hr = LogWriteTime_();
			}
			__finally {
				if (FAILED(hr))
				{
					::LeaveCriticalSection(&g_logCS);
					return hr;
				}
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
		if (fputs(szText, g_logFile) >= 0)
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

WCXFASTAPI wcLogWriteW(_In_opt_ PCWSTR szText)
{
	HRESULT hr = LogPreWrite(&szText, false);
	if (S_OK == hr) __try
	{
		if (fputws(szText, g_logFile) >= 0)
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
		if (vfprintf(g_logFile, szFmt, Args) >= 0)
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
			ULARGE_INTEGER uliPos;
			if (FAILED(Stream_Position(pstm, &uliPos.QuadPart)))
				uliPos.QuadPart = (UINT64)-1ll;
			const UINT64 cbSize = ToUInt64(stat.cbSize);
			SYSTEMTIME st = { 0 };
			::FileTimeToSystemTime(&stat.mtime, &st);
			hr = (fprintf(g_logFile, "'%S' (%.2f KB, %u bytes; %.2u-%.2u-%.2u %.2u:%.2u:%.2u) @ %u ",
				stat.pwcsName, (float)(cbSize / 1024.), (UINT)cbSize,
				(UINT)st.wYear % 100, (UINT)st.wMonth, (UINT)st.wDay,
				(UINT)st.wHour, (UINT)st.wMinute, (UINT)st.wSecond, uliPos.LowPart) >= 0)
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
		{
			if (vfprintf(g_logFile, szFmt, Args) >= 0)
			{
				fflush(g_logFile);
				hr = S_OK;
			}
			else hr = HRESULT_WIN_ERROR;
		}
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
			SYSTEMTIME st;
			::GetLocalTime(&st);
			fprintf(g_logFile, "\r\n<<<<<<< %.2u-%.2u-%.2u %.2u:%.2u:%.2u <<<<<<<\r\n",
					(UINT)st.wYear % 100, (UINT)st.wMonth, (UINT)st.wDay,
					(UINT)st.wHour, (UINT)st.wMinute, (UINT)st.wSecond);
			fflush(g_logFile);
			fclose(g_logFile);
		}
		::LeaveCriticalSection(&g_logCS);
		::DeleteCriticalSection(&g_logCS);
	}
}


#endif	// WCX_ENABLE_LOG
}	// namespace