// QUtils.cpp

#include "pch.h"
#include "Main.h"


namespace RootNamespace {	// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

///////////////////////////////////////////////////////////////////////
// Misc. helpers //////////////////////////////////////////////////////


WCXFASTAPI_(D2D_SIZE_U) wcToImageSizeU(D2D_SIZE_F sizeF)
{
	return { wcToImageSizeU(sizeF.width), wcToImageSizeU(sizeF.height) };
}


///////////////////////////////////////////////////////////////////////
// Strings ////////////////////////////////////////////////////////////


WCXSTDAPI_(UINT) wcNum32ToStr(UINT32 iVal, bool_t isSigned, _Out_writes_to_(cchMax, return-1) PWSTR szNum, int cchMax, UINT8 radix)
{
	if (cchMax > 0)
	{
		cchMax = (isSigned ? _ltow_s((INT32)iVal, szNum, (UINT)cchMax, (int)radix) : _ultow_s(iVal, szNum, (UINT)cchMax, (int)radix));
		if (0 == cchMax)
			return (UINT)wcslen(szNum);
		szNum[0] = 0;
	}
	::SetLastError(ERROR_INVALID_PARAMETER);
	return 0;
}


WCXFASTAPI_(UINT) wcAsciiToWide(_In_opt_ PCSTR szAscii, int cchBuff, _Out_writes_to_(cchBuff, return + 1) PWSTR pwcBuff)
{
	if (cchBuff > 0)
	{
		if (szAscii)
		{
			const int clen = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, szAscii, -1, pwcBuff, cchBuff);
			if (clen > 0)
				return (UINT)(clen - 1);
		}
		pwcBuff[0] = 0;
	}
	return 0;
}


WCXFASTAPI_(bool_t) wcAsciiIsEqual(_In_opt_ PCWSTR szWide, _In_opt_ PCSTR szAscii)
{
	if (szWide)
	{
		WCHAR wc = szWide[0];
		if (szAscii)
		{
			while (AllTrue(wc <= 127, TOUPPER_((CHAR)wc) == TOUPPER_(szAscii[0])))
			{
				szWide++;
				szAscii++;
				if (0 == wc)
					return true;
				wc = szWide[0];
			}
		}
		else if (0 == wc)
			return true;
	}
	else if (!(szAscii && szAscii[0]))
		return true;
	return false;
}


WCXCDECLAPI_(bool_t) wcAsciiIsEqualV(_In_opt_ PCWSTR szwCmp, UINT cArgs, ...)
{
	va_list vaArgs;
	va_start(vaArgs, cArgs);
	bool equal = false;
	for (; cArgs; cArgs--)
	{
		const PCSTR sza = va_arg(vaArgs, PCSTR);
		if (equal = wcAsciiIsEqual(szwCmp, sza))
			break;
	}
	va_end(vaArgs);
	return equal;
}


WCXFASTAPI_(PWSTR) wcFindFileName(_In_NLS_string_opt_(pathLen) PCWCH pathName, int pathLen)
{
	if (NLStringLen(pathName, &pathLen))
	{
		PCWSTR pend = pathName + ((UINT)pathLen - 1);
		while (pend >= pathName)
		{
			const WCHAR ch = *pend--;
			if (L'\\' != ch && L'/' != ch && L':' != ch)
				continue;
			pathName = pend + 2;
			break;
		}
	}
	return (PWSTR)pathName;
}


NOALIAS PCWCH FindInCSList(_In_reads_(cchList) PCWCH pwcList, _In_ UINT cchList,
		_In_reads_(cchVal) PCWCH pwcVal, _In_ UINT cchVal)
{
	const auto IsEqual = [pwcVal, cchVal](_In_reads_(cch) PCWCH pwcs, UINT cch) -> bool {
		return (cch == cchVal && CSTR_EQUAL == ::CompareStringOrdinal(pwcs, (int)cch, pwcVal, (int)cch, TRUE));
	};

	PCWCH next;
	UINT clen;
	while (cchVal < cchList && (next = wmemchr(pwcList, L',', cchList)))
	{
		clen = (UINT)(next - pwcList);
		if (IsEqual(pwcList, clen))
			return pwcList;
		++clen;
		cchList -= clen;
		pwcList += clen;
	}
	return (IsEqual(pwcList, cchList) ? pwcList : nullptr);
}


NOALIAS HRESULT CSListContains(_In_reads_(cchList) PCWCH pwcList, _In_ UINT cchList,
		_In_opt_ PCWSTR szVal, _Out_opt_ PBOOL pbContains)
{
	if (pbContains)
	{
		const UINT cchVal = (UINT)wcslen_s(szVal);
		if (cchVal)
		{
			*pbContains = (NULL != FindInCSList(pwcList, cchList, szVal, cchVal));
			return S_OK;
		}
		*pbContains = FALSE;
	}
	return E_INVALIDARG;
}


NOALIAS HRESULT STDMETHODCALLTYPE ReturnInfoEmptyString(UINT cchBuff,
		__RPC__inout_ecount_full_opt(cchBuff) WCHAR* wcBuff, __RPC__out UINT* pcchActual)
{
	if (wcBuff && cchBuff)
	{
		if (cchBuff > 0)
		{
			wcBuff[0] = 0;
			if (pcchActual)
				*pcchActual = 0;
			return S_OK;
		}
		return __HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
	}
	else if (pcchActual && !wcBuff && !cchBuff)
	{
		*pcchActual = 0;
		return S_OK;
	}
	return E_INVALIDARG;
}


NOALIAS HRESULT ReturnInfoRealString(_In_reads_(cchValue) PCWCH wcsValue, UINT cchValue,
		UINT cchBuff, __RPC__inout_ecount_full_opt(cchBuff) WCHAR* wcBuff, __RPC__out UINT* pcchActual)
{
	if (wcBuff && cchBuff)
	{
		if (cchBuff > cchValue)
		{
			wmemcpy(wcBuff, wcsValue, cchValue);
			wcBuff[cchValue] = 0;
			if (pcchActual)
				*pcchActual = cchValue;
			return S_OK;
		}
		return __HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
	}
	else if (pcchActual && !wcBuff && !cchBuff)
	{
		*pcchActual = cchValue;
		return S_OK;
	}
	return E_INVALIDARG;
}


NOALIAS HRESULT ReturnInfoVersion(_In_ UINT64 version64, UINT cchBuff,
		__RPC__inout_ecount_full_opt(cchBuff) WCHAR* wcBuff, __RPC__out UINT* pcchActual)
{
	WCHAR wcVer[32];
	const UINT cchVer = wsFormatVersion(g_AppVersion.Version64, wcVer, _countof(wcVer));
	if (cchVer)
		return ReturnInfoRealString(wcVer, cchVer, cchBuff, wcBuff, pcchActual);
	if (pcchActual)
		*pcchActual = 0;
	return E_UNEXPECTED;
}


///////////////////////////////////////////////////////////////////////
// Memory etc. ////////////////////////////////////////////////////////


NOALIAS bool __fastcall IsEqualGUID2(REFGUID guidCmp, REFGUID guid1, REFGUID guid2)
{
	return IsEqualGUID(guidCmp, guid1) || IsEqualGUID(guidCmp, guid2);
}

NOALIAS bool __fastcall IsEqualGUID3(REFGUID guidCmp, REFGUID guid1, REFGUID guid2, REFGUID guid3)
{
	return IsEqualGUID(guidCmp, guid1) || IsEqualGUID(guidCmp, guid2) || IsEqualGUID(guidCmp, guid3);
}

NOALIAS bool __fastcall IsEqualGUID4(REFGUID guidCmp, REFGUID guid1, REFGUID guid2, REFGUID guid3, REFGUID guid4)
{
	return IsEqualGUID(guidCmp, guid1) || IsEqualGUID(guidCmp, guid2)
		|| IsEqualGUID(guidCmp, guid3) || IsEqualGUID(guidCmp, guid4);
}

NOALIAS bool __fastcall IsEqualGUID5(REFGUID guidCmp, REFGUID guid1,
		REFGUID guid2, REFGUID guid3, REFGUID guid4, REFGUID guid5)
{
	return IsEqualGUID(guidCmp, guid1) || IsEqualGUID(guidCmp, guid2)
		|| IsEqualGUID(guidCmp, guid3) || IsEqualGUID(guidCmp, guid4) || IsEqualGUID(guidCmp, guid5);
}


NOALIAS HRESULT __cdecl QueryInterfaceImpl_(REFIID riid, _COM_Outptr_ void** ppvObject, UINT8 count, ...)
{
	HRESULT hr = E_POINTER;
	if (ppvObject)
	{
		*ppvObject = nullptr;
		hr = E_NOINTERFACE;

		UINT8 ind = 0;
		va_list vArgs;
		va_start(vArgs, count);
		const __m128i qiid = _mm_loadu_si128((const __m128i*)&riid);
		for (; count; count--, ind++)
		{
			const __m128i objid = _mm_loadu_si128((const __m128i*)va_arg(vArgs, const IID*));
			IUnknown* const pobj = va_arg(vArgs, IUnknown*);
			if (qiid != objid && (ind || _mm_loadu_si128((const __m128i*)&IID_IUnknown) != objid))
				continue;
			*ppvObject = pobj;
			hr = S_OK;
			break;
		}
		va_end(vArgs);
	}
	return hr;
}


WCXFASTAPI_(PDWORD32) wcSetMemory32(_Out_writes_all_(cdw) PDWORD32 dest, _In_ DWORD32 val, _In_ SIZE_T cdw)
{
	if (cdw > 15)
	{
		const __m128i mval = mm_set1_epi32((int)val);
		if (!((BYTE)(DWORD_PTR)dest & 15))
		{
			for (; cdw > 3; cdw -= 4)
			{
				_mm_stream_si128((__m128i*)dest, mval);
				dest += 4;
			}
		}
		else
		{
			for (; cdw > 3; cdw -= 4)
			{
				_mm_storeu_si128((__m128i*)dest, mval);
				dest += 4;
			}
		}
	}
	for (; cdw; cdw--)
	{
		*dest = val;
		dest++;
	}
	return dest;
}


NOALIAS PDWORD32 __fastcall PremulArgbToArgb(_Out_writes_all_(cdwCount) PDWORD32 argbDest,
		_In_reads_(cdwCount) const DWORD32* pargbSrc, UINT cdwCount)
{
	const __m128 ps255 = mm_ps_1d255();
	for (; cdwCount; cdwCount--)
	{
		const DWORD32 argb = *pargbSrc;
		pargbSrc++;
		const __m128 rcp = _mm_rcp_ps(_mm_mul_ps(_mm_cvtepi32_ps(_mm_set1_epi32((__int32)(DWORD32)(UINT8)(argb >> 24))), ps255));
		const __m128 clrf = mm_cvtpu8_ps(argb);
		const __m128i clr = _mm_cvtps_epi32(_mm_mul_ps(clrf, rcp));
		*argbDest = ((DWORD32)mm_cvtepi32_pi8(clr) & 0xFFFFFF) | (argb & 0xFF000000);
		argbDest++;
	}
	return argbDest;
}


///////////////////////////////////////////////////////////////////////
// Image bits /////////////////////////////////////////////////////////


typedef void* (__cdecl* MEMCPY_T)(_Out_writes_bytes_all_(size) void* dst,
		_In_reads_bytes_(size) const void* src, _In_ size_t size);

static NOALIAS void* __cdecl revcpy32(_Out_writes_bytes_all_(size) void* dst,
		_In_reads_bytes_(size) const void* src, _In_ size_t size)
{
	size /= 4;
	register PCUINT32 pbs = (PCUINT32)src;
	register PUINT32 pbd = (PUINT32)dst + size - 1;
	for (; size; size--)
	{
		*pbd = *pbs;
		pbs++;
		pbd--;
	}
	return dst;
}

NOALIAS void CopyImageBits32(_Out_writes_bytes_(cbDstStride * size.height) PBYTE pDst, UINT cbDstStride,
		_In_reads_bytes_(cbSrcStride * size.height) PCBYTE pSrc, UINT cbSrcStride, D2D_SIZE_U size,
		WICBitmapTransformOptions transform)
{
	MEMCPY_T pfncpy = &memcpy;
	INT_PTR cbSrcLine = (INT_PTR)cbSrcStride;
	INT_PTR cbDstLine = (INT_PTR)cbDstStride;
	switch (transform)
	{
	case WICBitmapTransformRotate180:
		pfncpy = &revcpy32;
		__fallthrough;
	case WICBitmapTransformFlipVertical:
		pDst += (size_t)cbDstLine * (size.height - 1);
		cbDstLine = -cbDstLine;
		break;
	case WICBitmapTransformFlipHorizontal:
		pfncpy = &revcpy32;
		break;
	}
	const size_t cbLine = size.width * 4;
	for (; size.height; size.height--)
	{
		pfncpy(pDst, pSrc, cbLine);
		pSrc += cbSrcLine;
		pDst += cbDstLine;
	}
}


}	// namespace <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
