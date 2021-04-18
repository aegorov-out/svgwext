// SVGWHelpers.inl
#pragma once


inline bool FloatEq(FLOAT a, FLOAT b)
{
	return CPP_COMMONS(DXFloatEq(a, b));
}
inline bool DoubleEq(DOUBLE a, DOUBLE b)
{
	return CPP_COMMONS(DXDoubleEq(a, b));
}
inline bool IsZero(FLOAT flt)
{
	return CPP_COMMONS(DXFloatZero(flt));
}
inline bool IsZero(DOUBLE dbl)
{
	return CPP_COMMONS(DXDoubleZero(dbl));
}


inline bool __fastcall IsOkOrFalse(HRESULT hr)
{
	return (S_OK == hr) | (S_FALSE == hr);
}
inline bool __fastcall IsAllocOk(HRESULT hr)
{
	return wcx::IsOkOrFalse(hr);
}
inline bool __fastcall IsAllocSuccess(HRESULT hr)
{
	return wcx::IsAllocOk(hr) | (S_EMPTY == hr);
}
inline bool __fastcall IsImplSuccess(HRESULT hr)
{
	return SUCCEEDED(hr) | (E_NOTIMPL == hr);
}
inline bool __fastcall IsValidError(HRESULT hr)
{
	return ((S_OK != hr) & (S_FALSE != hr) & (HRESULT_INVALID != hr));
}

inline bool __fastcall IsUnknownImage(HRESULT hr)
{
	return SUCCEEDED(hr) | (WINCODEC_ERR_UNKNOWNIMAGEFORMAT == hr);
}


inline UINT wcToImageSizeU(FLOAT fdim)
{
	const UINT uval = (UINT)(fdim + (1 - .001));
	return uval + (0 == uval);
}


constexpr DWORD INIT_CRITICAL_SECTION_FLAGS =
#ifdef _DEBUG
0;
#else
CRITICAL_SECTION_NO_DEBUG_INFO;
#endif

inline void wcInitCommonCS(_Out_ LPCRITICAL_SECTION pcs)
{
	if (!::InitializeCriticalSectionEx(pcs, 7200, INIT_CRITICAL_SECTION_FLAGS))
	{
		heapmin();
		::InitializeCriticalSection(pcs);
	}
}


inline PWSTR __fastcall wcCheckCLSwitch(_In_ PCWSTR szArg)
{
	switch (szArg[0])
	{
	case L'/':
	case L'+':
		return const_cast<PWSTR>(szArg + 1);
	case L'-':
		++szArg;
		return const_cast<PWSTR>(szArg + (L'-' == szArg[0]));
	}
	return nullptr;
}
