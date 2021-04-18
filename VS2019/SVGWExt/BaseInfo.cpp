// BaseInfo.cpp

#include "pch.h"
#include "DecodeBase.h"

#pragma warning(disable: 6387 6388 28196)

namespace RootNamespace {

///////////////////////////////////////////////////////////////////////
// IWICBitmapDecoderInfo //////////////////////////////////////////////


WARNING_SUPPRESS(6101 6388 28196)
HRESULT DecoderInfoBase::QueryInterface(REFIID riid, _COM_Outptr_ void** ppvObject)
{
	HRESULT hr = E_POINTER;
	if (ppvObject)
	{
		if (IsEqualGUID4(riid, IID_IWICBitmapDecoderInfo, IID_IWICBitmapCodecInfo, IID_IWICComponentInfo, IID_IUnknown))
		{
			_InterlockedIncrement(&m_cRef);
			*ppvObject = static_cast<IWICBitmapDecoderInfo*>(this);
			return S_OK;
		}
		*ppvObject = nullptr;
		hr = E_NOINTERFACE;
	}
	return hr;
}

ULONG DecoderInfoBase::AddRef()
{
	ADDREF_IMPL;
}

ULONG DecoderInfoBase::Release()
{
	RELEASE_IMPL;
}


HRESULT DecoderInfoBase::GetComponentType(__RPC__out WICComponentType* pType)
{
	if (pType)
	{
		*pType = WICDecoder;
		return S_OK;
	}
	return E_INVALIDARG;
}


HRESULT DecoderInfoBase::GetSigningStatus(__RPC__out DWORD* pStatus)
{
	if (pStatus)
	{
		*pStatus = WICComponentUnsigned;
		return S_OK;
	}
	return E_INVALIDARG;
}


HRESULT DecoderInfoBase::GetAuthor(UINT cchAuthor,
	__RPC__inout_ecount_full_opt(cchAuthor) WCHAR* wzAuthor, __RPC__out UINT* pcchActual)
{
	return ReturnInfoRealString(APP_AUTHOR_NAME, CSLEN_(APP_AUTHOR_NAME), cchAuthor, wzAuthor, pcchActual);
}


HRESULT DecoderInfoBase::GetVendorGUID(__RPC__out GUID* pguidVendor)
{
	if (pguidVendor)
	{
		CopyGUID(pguidVendor, GUID_AE_VendorID);
		return S_OK;
	}
	return E_INVALIDARG;
}


HRESULT DecoderInfoBase::GetVersion(UINT cchVersion,
	__RPC__inout_ecount_full_opt(cchVersion) WCHAR* wzVersion, __RPC__out UINT* pcchActual)
{
	return ReturnInfoVersion(g_AppVersion.Version64, cchVersion, wzVersion, pcchActual);
}

HRESULT DecoderInfoBase::GetSpecVersion(UINT cchSpecVersion,
	__RPC__inout_ecount_full_opt(cchSpecVersion) WCHAR* wzSpecVersion, __RPC__out UINT* pcchActual)
{
	return ReturnInfoVersion(MAKEDLLVERULL(D2D_VERSION_MAJOR, D2D_VERSION_MINOR, 0, 0),
			cchSpecVersion, wzSpecVersion, pcchActual);
}


HRESULT DecoderInfoBase::GetPixelFormats(UINT cFormats,
	__RPC__inout_ecount_full_opt(cFormats) GUID* pguidPixelFormats, __RPC__out UINT* pcActual)
{
	if (pguidPixelFormats && cFormats)
	{
		if (cFormats >= 2)
		{
			CopyGUID(pguidPixelFormats, GUID_WICPixelFormat32bppPBGRA);
			CopyGUID(pguidPixelFormats + 1, GUID_WICPixelFormat32bppBGRA);
			if (pcActual)
				*pcActual = 2;
			return S_OK;
		}
		return __HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
	}
	else if (pcActual && !pguidPixelFormats && !cFormats)
	{
		*pcActual = 2;
		return S_OK;
	}
	return E_INVALIDARG;
}


HRESULT DecoderInfoBase::GetColorManagementVersion(UINT cchColorManagementVersion,
	__RPC__inout_ecount_full_opt(cchColorManagementVersion) WCHAR* wzColorManagementVersion, __RPC__out UINT* pcchActual)
{
	return ReturnInfoEmptyString(cchColorManagementVersion, wzColorManagementVersion, pcchActual);
}


HRESULT DecoderInfoBase::GetDeviceManufacturer(UINT cchDeviceManufacturer,
	__RPC__inout_ecount_full_opt(cchDeviceManufacturer) WCHAR* wzDeviceManufacturer, __RPC__out UINT* pcchActual)
{
	return ReturnInfoEmptyString(cchDeviceManufacturer, wzDeviceManufacturer, pcchActual);
}


HRESULT DecoderInfoBase::GetDeviceModels(UINT cchDeviceModels,
	__RPC__inout_ecount_full_opt(cchDeviceModels) WCHAR* wzDeviceModels, __RPC__out UINT* pcchActual)
{
	return ReturnInfoEmptyString(cchDeviceModels, wzDeviceModels, pcchActual);
}



HRESULT DecoderInfoBase::GetMimeTypes(UINT cchMimeTypes,
	__RPC__inout_ecount_full_opt(cchMimeTypes) WCHAR* wzMimeTypes, __RPC__out UINT* pcchActual)
{
	UINT cchList;
	const PCWSTR szList = GetMimeTypes(&cchList);
	return ReturnInfoRealString(szList, cchList, cchMimeTypes, wzMimeTypes, pcchActual);
}


HRESULT DecoderInfoBase::GetFileExtensions(UINT cchFileExtensions,
	__RPC__inout_ecount_full_opt(cchFileExtensions) WCHAR* wzFileExtensions, __RPC__out UINT* pcchActual)
{
	UINT cchList;
	const PCWSTR szList = GetExtensions(&cchList);
	return ReturnInfoRealString(szList, cchList, cchFileExtensions, wzFileExtensions, pcchActual);
}


HRESULT DecoderInfoBase::MatchesMimeType(__RPC__in LPCWSTR wzMimeType, __RPC__out BOOL* pfMatches)
{
	UINT cchList;
	const PCWSTR szList = GetMimeTypes(&cchList);
	return CSListContains(szList, cchList, wzMimeType, pfMatches);
}


HRESULT DecoderInfoBase::DoesSupportAnimation(__RPC__out BOOL* pfSupportAnimation)
{
	if (pfSupportAnimation)
	{
		*pfSupportAnimation = (m_infoFlags & (ULONG)DecoderInfoFlag::Animation) != 0;
		return S_OK;
	}
	return E_INVALIDARG;
}


HRESULT DecoderInfoBase::DoesSupportChromakey(__RPC__out BOOL* pfSupportChromakey)
{
	if (pfSupportChromakey)
	{
		*pfSupportChromakey = (m_infoFlags & (ULONG)DecoderInfoFlag::Chromakey) != 0;
		return S_OK;
	}
	return E_INVALIDARG;
}


HRESULT DecoderInfoBase::DoesSupportLossless(__RPC__out BOOL* pfSupportLossless)
{
	if (pfSupportLossless)
	{
		*pfSupportLossless = (m_infoFlags & (ULONG)DecoderInfoFlag::Lossless) != 0;
		return S_OK;
	}
	return E_INVALIDARG;
}


HRESULT DecoderInfoBase::DoesSupportMultiframe(__RPC__out BOOL* pfSupportMultiframe)
{
	if (pfSupportMultiframe)
	{
		*pfSupportMultiframe = (m_infoFlags & (ULONG)DecoderInfoFlag::Multiframe) != 0;
		return S_OK;
	}
	return E_INVALIDARG;
}


HRESULT DecoderInfoBase::GetPatterns(UINT cbSizePatterns,
	_Out_writes_bytes_to_opt_(cbSizePatterns, *pcbPatternsActual) WICBitmapPattern* pPatterns,
	_Out_opt_  UINT* pcPatterns, _Out_  UINT* pcbPatternsActual)
{
	if (pPatterns && cbSizePatterns)
	{
		if (pcbPatternsActual)
			*pcbPatternsActual = 0;
		if (pcPatterns)
			*pcPatterns = 0;
		return S_OK;
	}
	else if (pcbPatternsActual && !pPatterns)
	{
		*pcbPatternsActual = 0;
		if (pcPatterns)
			*pcPatterns = 0;
		return S_OK;
	}
	return E_INVALIDARG;
}


}	// namespace