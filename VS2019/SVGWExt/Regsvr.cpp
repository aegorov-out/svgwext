// Regsvr.cpp

#include "pch.h"
#include "Main.h"
#include <regstr.h>

#pragma function(memcpy, memset, wcslen)

///////////////////////////////////////////////////////////////////////
// Exported entries ///////////////////////////////////////////////////


STDAPI DllRegisterServer()
{
	return wcDllInstall(TRUE, nullptr, 0);
}

STDAPI DllUnregisterServer()
{
	return wcDllInstall(FALSE, nullptr, 0);
}


STDAPI DllInstall(BOOL bInstall, _In_opt_ PCWSTR szCmdLine)
{
#ifdef WCX_SUPPORT_SELFREG
	const PCWSTR* rgszArgs = nullptr;
	union {
		int cArgs = 0;
		HRESULT hr;
	};
	if (szCmdLine && szCmdLine[0])
	{
		::SetLastError(NOERROR);
		rgszArgs = CommandLineToArgvW(szCmdLine, &cArgs);
		if (!rgszArgs)
		{
			const DWORD lerr = ::GetLastError();
			if (NOERROR != lerr)
				return HRESULT_FROM_WIN32(lerr);
		}
	}
	hr = wcDllInstall(bInstall, rgszArgs, cArgs);
	::LocalFree(const_cast<PWSTR*>(rgszArgs));
	return hr;
#else
	return E_NOTIMPL;
#endif
}


////////////////////////////////////////////////////////////////////////
#ifdef WCX_SUPPORT_SELFREG	////////////////////////////////////////////
namespace RootNamespace {

constexpr DWORD REGSAM_WRITE = KEY_WRITE;
constexpr DWORD REGSAM_CREATE_SUBKEY = (KEY_WRITE & ~KEY_SET_VALUE);
constexpr DWORD REGSAM_SET_VALUE = (KEY_WRITE & ~KEY_CREATE_SUB_KEY);

#ifndef _DEBUG

#define REGKEY_OPTION		0
#define HK_CLASSES_ROOT		HKEY_CLASSES_ROOT
#define HK_LOCAL_MACHINE	HKEY_LOCAL_MACHINE

#define OpenRootKeys()		S_OK
#define CloseRootKeys()		((void)0)

#else	// _DEBUG

static HKEY g_hkCR = NULL, g_hkLM = NULL;

#define REGKEY_OPTION		0	//REG_OPTION_VOLATILE
#define HK_CLASSES_ROOT		g_hkCR
#define HK_LOCAL_MACHINE	g_hkLM

#define REGPATH_APP_			L"SOFTWARE\\AE_\\SVGWExt\\"
#define REGPATH_CLASSES_ROOT	REGPATH_APP_ L"HKCR"
#define REGPATH_LOCAL_MACHINE	REGPATH_APP_ L"HKLM"

static HRESULT OpenRootKeys()
{
	const auto CreateKey = [](PCWSTR szKey) -> HKEY {
		HKEY hKey;
		const LSTATUS lstat = ::RegCreateKeyExW(HKEY_CURRENT_USER, szKey, 0, nullptr,
			REGKEY_OPTION, KEY_ALL_ACCESS, nullptr, &hKey, nullptr);
		if (ERROR_SUCCESS == lstat)
			return hKey;
		::SetLastError((DWORD)lstat);
		return NULL;
	};

	g_hkCR = CreateKey(REGPATH_CLASSES_ROOT);
	if (g_hkCR)
	{
		g_hkLM = CreateKey(REGPATH_LOCAL_MACHINE);
		if (g_hkLM)
			return S_OK;
		::RegCloseKey(g_hkCR);
		g_hkCR = NULL;
	}
	return SELFREG_E_CLASS;
}

static void CloseRootKeys()
{
	SafeRegCloseKey(&g_hkLM);
	SafeRegCloseKey(&g_hkCR);
}

#endif	// _DEBUG


static NOALIAS UINT8 __fastcall ReadCompArg(_In_opt_ PCWSTR szArg)
{
	UINT8 comp = 0;
	if (szArg && szArg[0])
	{
		const PCWSTR szSwitch = wcCheckCLSwitch(szArg);
		if (szSwitch)
			szArg = szSwitch;
		if (wcAsciiIsAnyEqual(szArg, 2, "svg", "svgall"))
			comp = (UINT8)ServComp::SvgThumbProvider | (UINT8)ServComp::SvgDecoder;
		else if (wcAsciiIsAnyEqual(szArg, 2, "svgt", "svgthumb"))
			comp = (UINT8)ServComp::SvgThumbProvider;
		else if (wcAsciiIsAnyEqual(szArg, 3, "svgd", "svgdec", "svgwic"))
			comp = (UINT8)ServComp::SvgDecoder;
		else if (wcAsciiIsAnyEqual(szArg, 3, "wmf", "wmfemf", "wmfall"))
			comp = (UINT8)ServComp::WmfThumbProvider | (UINT8)ServComp::WmfDecoder;
		else if (wcAsciiIsAnyEqual(szArg, 2, "wmft", "wmfthumb"))
			comp = (UINT8)ServComp::WmfThumbProvider;
		else if (wcAsciiIsAnyEqual(szArg, 3, "wmfd", "wmfdec", "wmfwic"))
			comp = (UINT8)ServComp::WmfDecoder;
		else
			ASSUME(0 == comp);
	}
	return comp;
}


///////////////////////////////////////////////////////////////////////
// Actual registration ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


#pragma warning(disable: 26495)

class RegSvr : public Uncopyable_
{
	HKEY m_hkClass = NULL;
	HKEY m_hkSetup = NULL;
	UINT m_cchModule = 0;
	_Field_size_(m_cchModule + 1) WCHAR m_wcsModule[MAX_PATH];

	enum : int {
		Value_DWORD = 0, Value_GUID = -1, Value_Ascii = -2
	};

	typedef struct UValData
	{
		PCSTR szName;
		union {
			PCWSTR szWideVal;
			PCSTR szAsciiVal;
			const GUID* pGuid;
			DWORD32 dwValue;
		};
		int cchValue;

		UValData(_In_opt_ PCSTR szValName, _In_reads_(cchValue + 1) PCWSTR szValue, UINT cchValue)
			: szName(szValName), szWideVal(szValue), cchValue((int)cchValue) {}
		UValData(_In_opt_ PCSTR szValName, _In_ PCSTR szValue)
			: szName(szValName), szAsciiVal(szValue), cchValue(Value_Ascii) {}
		UValData(_In_opt_ PCSTR szValName, _In_ REFGUID rGuid)
			: szName(szValName), pGuid(&rGuid), cchValue(Value_GUID) {}
		UValData(_In_opt_ PCSTR szValName, _In_ DWORD32 dword)
			: szName(szValName), dwValue(dword), cchValue(Value_DWORD) {}
		UValData() {}

		void SetValue(_In_opt_ PCSTR szValName, _In_reads_(cchValue + 1) PCWSTR szValue, UINT cchValue) {
			this->szName = szValName;
			this->szWideVal = szValue;
			this->cchValue = (int)cchValue;
		}
		void SetValue(_In_opt_ PCSTR szValName, _In_ PCSTR szValue) {
			this->szName = szValName;
			this->szAsciiVal = szValue;
			this->cchValue = Value_Ascii;
		}
		void SetValue(_In_opt_ PCSTR szValName, _In_ REFGUID rGuid) {
			this->szName = szValName;
			this->pGuid = &rGuid;
			this->cchValue = Value_GUID;
		}
		void SetValue(_In_opt_ PCSTR szValName, _In_ DWORD32 dwVal) {
			this->szName = szValName;
			this->dwValue = dwVal;
			this->cchValue = Value_DWORD;
		}
	} const* PCUValData;


	_Success_(return == S_OK)
	HRESULT RegisterServer(_In_ REFCLSID rClsid, _In_ PCSTR szThreadModel, _In_ PCSTR szName, bool isDecoder = false);
	void CloseClassKey();

	HRESULT RegisterSvgThumbs();
	HRESULT RegisterWmfThumbs();
	HRESULT RegisterThumbExt(_In_ PCWSTR szDotExt, _In_reads_(cchClsid + 1) PCWSTR szClsid, UINT cchClsid, bool option = false);
	HRESULT WriteThumbExt(_In_ HKEY hkRoot, _In_ PCWSTR szDotExt,
		_In_reads_(cchClsid + 1) PCWSTR szClsid, UINT cchClsid, bool option = false) const;

	HRESULT RegisterSvgDecoder();
	HRESULT RegisterWmfDecoder();
	HRESULT WriteWmfEmfPatterns() const;
	static NOALIAS HRESULT WritePattern(_In_ HKEY hKey, _In_ PCWSTR szIndex, DWORD32 Length,
		_In_reads_(Length) PCVOID Pattern, _In_reads_(Length) PCVOID Mask, DWORD32 Position = 0);
	void RegisterSvgTypes();
	void RegisterWmfTypes();
	HRESULT RegisterPictureType(_In_ PCWSTR szDotExt,
		_In_reads_(cchContType + 1) PCWSTR szContentType, UINT cchContType, UINT iconId);
	HRESULT WritePictureType(_In_ PCWSTR szDotExt, _In_reads_(cchContType + 1) PCWSTR szContentType, UINT cchContType, bool option = false);
	_Success_(return == S_OK) HRESULT OpenSetupKey(_In_ PCWSTR szSubkey, _Out_ PHKEY phKey, _Out_opt_ bool* wasCreated = nullptr);
	bool ValueContainsModule(_In_ HKEY hKey, _In_opt_ PCWSTR szSubkey, _In_opt_ PCWSTR szValName) const;
	bool ValueContainsModule(_In_ HKEY hKey, _In_opt_ PCWSTR szSubkey, _In_opt_ PCWSTR szValName, _In_ PCWSTR szModuleName) const;

	HRESULT SetDecoderInfoValues(_Inout_updates_(cSpecValues + CODEC_COMMON_ENTRIES) UValData* rgValues, UINT cSpecValues);


	static constexpr auto CODEC_COMMON_ENTRIES = 5u;	// VendorGUID, Author, SupportAnimation, SupportChromaKey, SupportMultiframe

	static NOALIAS _Translates_Win32_to_HRESULT_(lstat) HRESULT __fastcall ToHRESULT(LSTATUS lstat);

	_Success_(return == S_OK) HRESULT OpenKey(_In_ HKEY hkRoot, _In_ PCWSTR szSubkey,
		REGSAM regSam, _Out_ PHKEY phKey, _Out_opt_ bool* wasCreated = nullptr) const;
	static NOALIAS _Success_(return == S_OK) HRESULT CreateKey(_In_ HKEY hkRoot, _In_ PCWSTR szSubkey,
		REGSAM regSam, _Out_ PHKEY phKey, _Out_opt_ bool* wasCreated = nullptr);
	static NOALIAS HRESULT DeleteKey(_In_ HKEY hkRoot, _In_opt_ PCWSTR szSubkey);
	static NOALIAS BOOL StringValueExists(_In_ HKEY hKey, _In_opt_ PCWSTR szSubkey, _In_opt_ PCWSTR szName);
	static NOALIAS HRESULT SetKeyValue(_In_ HKEY hKey, _In_opt_ PCWSTR szSubKey, _In_opt_ PCWSTR szValueName,
		_In_reads_(cchVal + 1) PCWSTR szValue, UINT cchVal, bool option = false);
	static NOALIAS HRESULT SetValue(_In_ HKEY hKey, _In_opt_ PCSTR szName,
		DWORD regType, _In_reads_bytes_(cbSize) PCVOID pValue, UINT cbSize);
	static NOALIAS HRESULT SetValue(_In_ HKEY hKey, _In_opt_ PCWSTR szName,
		_In_reads_(cchVal + 1) PCWSTR szValue, UINT cchVal);
	static NOALIAS HRESULT SetValue(_In_ HKEY hKey, _In_opt_ PCSTR szName,
		_In_reads_(cchVal + 1) PCWSTR szValue, UINT cchVal);
	static NOALIAS HRESULT SetValue(_In_ HKEY hKey, _In_opt_ PCSTR szName, _In_ PCSTR szValue);
	static NOALIAS HRESULT SetValue(_In_ HKEY hKey, const UValData& Value);
	static NOALIAS HRESULT SetValues(_In_ HKEY hKey, _In_reads_(cValues) PCUValData rgValues, UINT cValues);

public:
	~RegSvr()
	{
		if (m_hkSetup)
			::RegCloseKey(m_hkSetup);
		if (m_hkClass)
			::RegCloseKey(m_hkClass);
	}

	BOOL IsInstall() const { return (BOOL)m_cchModule; }
	bool  IsUninstall() const { return !m_cchModule; }

	HRESULT DllInstall(UINT8 compMask, BOOL bInstall);
};

#pragma warning(default: 26495)

///////////////////////////////////////////////////////////////////////
// Individual component regisration ///////////////////////////////////


HRESULT RegSvr::DllInstall(UINT8 compMask, BOOL bInstall)
{
	bool anyOk = false;
	HRESULT hr = S_FALSE;

	*(PDWORD)m_wcsModule = 0;
	if (bInstall)
	{
		m_cchModule = wcGetModuleName(g_hModule, m_wcsModule, _countof(m_wcsModule));
		hr = SELFREG_E_CLASS;
		if (!m_cchModule)
			goto Exit_;
	}
	else ASSUME(0 == m_cchModule);

	if (compMask & (UINT8)ServComp::SvgDecoder)
	{
		hr = RegisterSvgDecoder();
		anyOk |= (S_OK == hr);
		if (FAILED(hr))
			goto Finish_;
	}
	if (compMask & (UINT8)ServComp::WmfDecoder)
	{
		hr = RegisterWmfDecoder();
		anyOk |= (S_OK == hr);
		if (FAILED(hr))
			goto Finish_;
	}
	if (compMask & (UINT8)ServComp::SvgThumbProvider)
	{
		hr = RegisterSvgThumbs();
		anyOk |= (S_OK == hr);
		if (FAILED(hr))
			goto Finish_;
	}
	if (compMask & (UINT8)ServComp::WmfThumbProvider)
	{
		hr = RegisterWmfThumbs();
		anyOk |= (S_OK == hr);
		if (FAILED(hr))
			goto Finish_;
	}

	if (ServComp_Svg == (compMask & ServComp_Svg))
	{
		RegisterSvgTypes();
		anyOk = true;
	}
	if (ServComp_Wmf == (compMask & ServComp_Wmf))
	{
		RegisterWmfTypes();
		anyOk = true;
	}

Finish_:
	if (anyOk)
		::SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
Exit_:
	return hr;
}


_Success_(return == S_OK)
HRESULT RegSvr::RegisterServer(_In_ REFCLSID rClsid, _In_ PCSTR szThreadModel, _In_ PCSTR szName, bool isDecoder)
{
	CloseClassKey();

	HRESULT hr = SELFREG_E_CLASS;
	UINT cchKey;
	HKEY hkCls, hKey;
	union {
		UINT64 key64[2];
		WCHAR wcKey[ALIGN4(8 + MAX_GUID_STRSIZE)];
	};

	key64[0] = MAKEULONGLONGW(L'C', L'L', L'S', L'I');
	key64[1] = MAKEDWORD(L'D', L'\\');
	PWSTR pszClsid = wcKey + 6;

	cchKey = (UINT)::StringFromGUID2(rClsid, pszClsid, _countof(wcKey) - 6);
	ASSUME(SELFREG_E_CLASS == hr);
	if ((int)cchKey > 0)
	{
		if (IsInstall())	// register server
		{
			hr = CreateKey(HK_CLASSES_ROOT, wcKey, REGSAM_WRITE, &hkCls);
			if (S_OK == hr)
			{
				m_hkClass = hkCls;
				SetValue(hkCls, nullptr, szName);
				hr = CreateKey(hkCls, L"InprocServer32", REGSAM_SET_VALUE, &hKey);
				if (S_OK == hr)
				{
					UValData rgValues[] = {
						UValData(nullptr, m_wcsModule, m_cchModule),
						UValData("ThreadingModel", szThreadModel)
					};
					hr = SetValues(hKey, rgValues, ARRAYSIZE(rgValues));
					::RegCloseKey(hKey);
					if (SUCCEEDED(hr))
					{
						hr = S_OK;
						if (isDecoder)
						{
							HKEY hkSubkey;
							hr = CreateKey(HK_CLASSES_ROOT, REGPATH_WIC_DECODERS, REGSAM_CREATE_SUBKEY, &hkSubkey);
							if (S_OK == hr)
							{
								hr = CreateKey(hkSubkey, pszClsid, REGSAM_SET_VALUE, &hKey);
								if (S_OK == hr)
								{
									rgValues[0].SetValue("CLSID", rClsid);
									rgValues[1].SetValue("FriendlyName", szName);
									hr = SetValues(hKey, rgValues, ARRAYSIZE(rgValues));
									::RegCloseKey(hKey);
									if (SUCCEEDED(hr))
										hr = S_OK;
									else
										::RegDeleteKeyW(hkSubkey, pszClsid);
								}
								::RegCloseKey(hkSubkey);

								if (S_OK == hr)
								{
									key64[0] = MAKEULONGLONGW(L'F', L'o', L'r', L'm');
									key64[1] = MAKEULONGLONGW(L'a', L't', L's', L'\\');
									pszClsid = wcKey + 8;
									if (::StringFromGUID2(GUID_WICPixelFormat32bppBGRA, pszClsid, _countof(wcKey) - 8) > 0)
									{
										if (S_OK == CreateKey(hkCls, wcKey, REGSAM_SET_VALUE, &hKey))
											::RegCloseKey(hKey);
									}
									if (::StringFromGUID2(GUID_WICPixelFormat32bppPBGRA, pszClsid, _countof(wcKey) - 8) > 0)
									{
										if (S_OK == CreateKey(hkCls, wcKey, REGSAM_SET_VALUE, &hKey))
											::RegCloseKey(hKey);
									}
								}
							}
						}
						return hr;
					}
				}
				::RegCloseKey(hkCls);
			}
		}
		else if (isDecoder && ERROR_SUCCESS == ::RegOpenKeyExW(HK_CLASSES_ROOT,
			REGPATH_WIC_DECODERS, REGKEY_OPTION, REGSAM_CREATE_SUBKEY, &hKey))

		{
			::RegDeleteTreeW(hKey, pszClsid);
			::RegCloseKey(hKey);
		}
		hr = DeleteKey(HK_CLASSES_ROOT, wcKey);
	}
	return hr;
}


void RegSvr::CloseClassKey()
{
	SafeRegCloseKey(&m_hkClass);
}


// Thumbnail providers ////////////////////////////////////////////////


HRESULT RegSvr::RegisterSvgThumbs()
{
	HRESULT hr = RegisterServer(CLSID_SvgThumbnailProvider, "Apartment", "SVG thumbnail provider");
	if (SUCCEEDED(hr))
	{
		WCHAR wcClsid[MAX_GUID_STRSIZE];
		int cch = StringFromGUID2(CLSID_SvgThumbnailProvider, wcClsid, _countof(wcClsid));
		hr = SELFREG_E_CLASS;
		if (cch > 0)
		{
			--cch;
			hr = RegisterThumbExt(L".svg", wcClsid, (UINT)cch);
			if (SUCCEEDED(hr))
			{
				hr = RegisterThumbExt(L".svgz", wcClsid, (UINT)cch);
				if (SUCCEEDED(hr))
					hr = S_OK;
			}
		}
	}
	return hr;
}


HRESULT RegSvr::RegisterWmfThumbs()
{
	HRESULT hr = RegisterServer(CLSID_WmfEmfThumbnailProvider, "Apartment", "WMF/EMF thumbnail provider");
	if (SUCCEEDED(hr))
	{
		WCHAR wcClsid[MAX_GUID_STRSIZE];
		int cch = StringFromGUID2(CLSID_WmfEmfThumbnailProvider, wcClsid, _countof(wcClsid));
		hr = SELFREG_E_CLASS;
		if (cch > 0)
		{
			--cch;
			hr = RegisterThumbExt(L".wmf", wcClsid, (UINT)cch);
			if (SUCCEEDED(hr))
			{
				RegisterThumbExt(L".wmz", wcClsid, (UINT)cch, true);
				hr = RegisterThumbExt(L".emf", wcClsid, (UINT)cch);
				if (SUCCEEDED(hr))
				{
					RegisterThumbExt(L".emz", wcClsid, (UINT)cch, true);
					hr = S_OK;
				}
			}
		}
	}
	return hr;
}


HRESULT RegSvr::RegisterThumbExt(_In_ PCWSTR szDotExt, _In_reads_(cchClsid + 1) PCWSTR szClsid, UINT cchClsid, bool option)
{
	HRESULT hr = WriteThumbExt(HK_CLASSES_ROOT, szDotExt, szClsid, cchClsid, option);
	if (SUCCEEDED(hr))
	{
		HKEY hKey;
		bool keyCreated;
		hr = OpenSetupKey(L"PhotoPropertyHandler", &hKey, &keyCreated);
		if (S_OK == hr)
		{
			if (keyCreated)
				option = false;
			if (IsInstall())
				hr = SetKeyValue(hKey, szDotExt, nullptr, szClsid, cchClsid, option);
			else
				hr = DeleteKey(hKey, szDotExt);
			::RegCloseKey(hKey);

			if (SUCCEEDED(hr) && S_OK == OpenKey(HK_CLASSES_ROOT, L"SystemFileAssociations", REGSAM_CREATE_SUBKEY, &hKey))
			{
				WriteThumbExt(hKey, szDotExt, szClsid, cchClsid, option);
				::RegCloseKey(hKey);
			}
		}
	}
	return hr;
}

HRESULT RegSvr::WriteThumbExt(_In_ HKEY hkRoot, _In_ PCWSTR szDotExt,
		_In_reads_(cchClsid + 1) PCWSTR szClsid, UINT cchClsid, bool option) const
{
	HRESULT hr;
	if (IsInstall())
	{
		bool keyCreated;
		hr = CreateKey(hkRoot, szDotExt, REGSAM_WRITE, &hkRoot, &keyCreated);
		if (S_OK == hr)
		{
			if (keyCreated)
				option = false;
			hr = SetKeyValue(hkRoot, L"ShellEx\\{E357FCCD-A995-4576-B01F-234630154E96}", nullptr, szClsid, cchClsid, option);
			::RegCloseKey(hkRoot);
		}
	}
	else
	{
		hr = OpenKey(hkRoot, szDotExt, REGSAM_WRITE, &hkRoot);
		if (S_OK == hr)
		{
			hr = DeleteKey(hkRoot, L"ShellEx\\{E357FCCD-A995-4576-B01F-234630154E96}");
			if (SUCCEEDED(hr))	// try to delete an empty ShellEx key
				::RegDeleteKeyW(hkRoot, L"ShellEx");
			::RegCloseKey(hkRoot);
		}
	}
	return hr;
}


// Decoders ///////////////////////////////////////////////////////////


HRESULT RegSvr::RegisterSvgDecoder()
{
	HRESULT hr = RegisterServer(CLSID_SvgDecoder, "Both", SVG_DECODER_NAMEA, true);
	if (SUCCEEDED(hr))
	{
		hr = S_OK;
		if (IsInstall())
		{
			UValData Values[4 + CODEC_COMMON_ENTRIES] = {
			   UValData("FriendlyName", SVG_DECODER_NAMEA),
			   UValData("ContainerFormat", GUID_AE_ContainerFormatSvg),
			   UValData("MimeTypes", SVG_WIC_MIME_TYPES, CSLEN_(SVG_WIC_MIME_TYPES)),
			   UValData("FileExtensions", SVG_WIC_EXTENSIONS, CSLEN_(SVG_WIC_EXTENSIONS))
			};
			hr = SetDecoderInfoValues(Values, 4);
			if (SUCCEEDED(hr))
				hr = S_OK;
		}
	}
	return hr;
}


HRESULT RegSvr::RegisterWmfDecoder()
{
	HRESULT hr = RegisterServer(CLSID_WmfEmfDecoder, "Both", WMF_DECODER_NAMEA, true);
	if (SUCCEEDED(hr))
	{
		hr = S_OK;
		if (IsInstall())
		{
			UValData Values[4 + CODEC_COMMON_ENTRIES] = {
				UValData("FriendlyName", WMF_DECODER_NAMEA),
				UValData("ContainerFormat", GUID_AE_ContainerFormatWmfEmf),
				UValData("MimeTypes", WMFEMF_WIC_MIME_TYPES, CSLEN_(WMFEMF_WIC_MIME_TYPES)),
				UValData("FileExtensions", WMFEMF_WIC_EXTENSIONS, CSLEN_(WMFEMF_WIC_EXTENSIONS))
			};
			hr = SetDecoderInfoValues(Values, 4);
			if (SUCCEEDED(hr))
			{
				hr = WriteWmfEmfPatterns();
				if (SUCCEEDED(hr))
					hr = S_OK;
			}
		}
	}
	return hr;
}


HRESULT RegSvr::SetDecoderInfoValues(_Inout_updates_(cSpecValues + CODEC_COMMON_ENTRIES) UValData* rgValues, UINT cSpecValues)
{
	UValData* const rgCmnValues = rgValues + cSpecValues;
	rgCmnValues[0].SetValue("Author", APP_AUTHOR_NAME, CSLEN_(APP_AUTHOR_NAME));
	rgCmnValues[1].SetValue("Vendor", GUID_AE_VendorID);
	rgCmnValues[2].SetValue("SupportAnimation", 0u);
	rgCmnValues[3].SetValue("SupportChromaKey", 1u);
	rgCmnValues[4].SetValue("SupportMultiframe", 0u);
	STATIC_ASSERT(5 == CODEC_COMMON_ENTRIES);
	return SetValues(m_hkClass, rgValues, cSpecValues + CODEC_COMMON_ENTRIES);
}


HRESULT RegSvr::WriteWmfEmfPatterns() const
{
	HKEY hKey;
	HRESULT hr = CreateKey(m_hkClass, L"Patterns", REGSAM_CREATE_SUBKEY, &hKey);
	if (S_OK == hr)
	{
		union {
			DWORD32 dwIndex = L'0';
			WCHAR wcIndex[2];
		};
		UMETAHEADER pat, msk;

		pat.WEmf.TypeSign = 0x9AC6CDD7;
		msk.WEmf.TypeSign = (decltype(msk.WEmf.TypeSign))-1;
		hr = WritePattern(hKey, wcIndex, sizeof(pat.WEmf.TypeSign), &pat.WEmf.TypeSign, &msk.WEmf.TypeSign);
		if (SUCCEEDED(hr))
		{
			dwIndex++;
			ZeroStruct(&pat);
			ZeroStruct(&msk);
			pat.WEmf.TypeSign = EMR_HEADER;
			msk.WEmf.TypeSign = (decltype(msk.WEmf.TypeSign))-1;
			pat.WEmf.EmfSignature = ENHMETA_SIGNATURE;
			msk.WEmf.EmfSignature = (decltype(msk.WEmf.TypeSign))-1;
			hr = WritePattern(hKey, wcIndex, sizeof(pat.WEmf), &pat.WEmf, &msk.WEmf);
			if (SUCCEEDED(hr))
			{
				dwIndex++;
				ZeroStruct(&pat);
				ZeroStruct(&msk);
				pat.Wmf.WmfType = 0x0002;
				msk.Wmf.WmfType = (decltype(msk.Wmf.WmfType))-1;
				pat.Wmf.WmfVersion = 0x0300;
				msk.Wmf.WmfVersion = (decltype(msk.Wmf.WmfVersion))-1;
				if (SUCCEEDED(WritePattern(hKey, wcIndex, sizeof(pat.Wmf), &pat.Wmf, &msk.Wmf)))
				{
					dwIndex++;
					pat.Wmf.WmfVersion = 0x0100;
					WritePattern(hKey, wcIndex, sizeof(pat.Wmf), &pat.Wmf, &msk.Wmf);
				}
				hr = S_OK;
			}
		}
		::RegCloseKey(hKey);
	}
	return hr;
}

HRESULT RegSvr::WritePattern(_In_ HKEY hKey, _In_ PCWSTR szIndex, DWORD32 Length,
	_In_reads_(Length) PCVOID Pattern, _In_reads_(Length) PCVOID Mask, DWORD32 Position)
{
	HRESULT hr = CreateKey(hKey, szIndex, REGSAM_SET_VALUE, &hKey);
	if (S_OK == hr)
	{
		hr = SetValue(hKey, "Position", REG_DWORD, &Position, 4);
		if (SUCCEEDED(hr))
		{
			hr = SetValue(hKey, "Length", REG_DWORD, &Length, 4);
			if (SUCCEEDED(hr))
			{
				hr = SetValue(hKey, "Pattern", REG_BINARY, Pattern, Length);
				if (SUCCEEDED(hr))
				{
					hr = SetValue(hKey, "Mask", REG_BINARY, Mask, Length);
					if (SUCCEEDED(hr))
						hr = S_OK;
				}
			}
		}
		::RegCloseKey(hKey);
	}
	return hr;
}


// Type extensions ////////////////////////////////////////////////////


void RegSvr::RegisterSvgTypes()
{
	RegisterPictureType(L".svg", SVG_MIME_TYPE, CSLEN_(SVG_MIME_TYPE), IDI_SVG_FILE);
	RegisterPictureType(L".svgz", SVG_MIME_TYPE, CSLEN_(SVG_MIME_TYPE), IDI_SVGZ_FILE);
}


void RegSvr::RegisterWmfTypes()
{
	if (IsInstall())
	{
		if (SUCCEEDED(WritePictureType(L".emf", EMF_MIME_TYPE, CSLEN_(EMF_MIME_TYPE))))
		{
			WritePictureType(L".emz", EMF_MIME_TYPE, CSLEN_(EMF_MIME_TYPE), true);
			if (SUCCEEDED(WritePictureType(L".wmf", WMF_MIME_TYPE, CSLEN_(WMF_MIME_TYPE))))
				WritePictureType(L".wmz", WMF_MIME_TYPE, CSLEN_(WMF_MIME_TYPE), true);
		}
	}
}


HRESULT RegSvr::RegisterPictureType(_In_ PCWSTR szDotExt,
	_In_reads_(cchContType + 1) PCWSTR szContentType, UINT cchContType, UINT iconId)
{
	HRESULT hr = (IsInstall() ? WritePictureType(szDotExt, szContentType, cchContType) : S_FALSE);
	HKEY hkExt;
	if (SUCCEEDED(hr) && S_OK == OpenKey(HK_CLASSES_ROOT, szDotExt, REGSAM_CREATE_SUBKEY, &hkExt))
	{
		WCHAR wcVal[MAX_PATH + 12];
		if (IsInstall())
		{
			HKEY hkDefIcon;
			if (m_cchModule && m_cchModule < _countof(wcVal) - 8)
			{
				wmemcpy(wcVal, m_wcsModule, m_cchModule);
				wcVal[m_cchModule] = L',';
				const UINT cchPre = (m_cchModule + 1);
				const UINT cchId = wcInt32ToDec(-(INT32)iconId, wcVal + cchPre, _countof(wcVal) - cchPre);
				if (cchId && (!StringValueExists(hkExt, L"DefaultIcon", nullptr)
					|| ValueContainsModule(hkExt, L"DefaultIcon", nullptr)) &&
					S_OK == CreateKey(hkExt, L"DefaultIcon", REGSAM_SET_VALUE, &hkDefIcon))
				{
					SetValue(hkDefIcon, (PCWSTR)nullptr, wcVal, cchPre + cchId);
					::RegCloseKey(hkDefIcon);
				}
			}
		}
		else if (wcGetModuleName(g_hModule, wcVal, _countof(wcVal)) &&
			ValueContainsModule(hkExt, L"DefaultIcon", nullptr, wcVal))
		{
			hr = (ERROR_SUCCESS == ::RegDeleteKeyW(hkExt, L"DefaultIcon")) ? S_OK : S_FALSE;
		}
		::RegCloseKey(hkExt);
	}
	return hr;
}

HRESULT RegSvr::WritePictureType(_In_ PCWSTR szDotExt,
	_In_reads_(cchContType + 1) PCWSTR szContentType, UINT cchContType, bool option)
{
	ASSERT(IsInstall());

	HKEY hkKindMap;
	bool created;
	HRESULT hr = OpenSetupKey(L"Explorer\\KindMap", &hkKindMap, &created);
	if (S_OK == hr)
	{
		hr = SetKeyValue(hkKindMap, nullptr, szDotExt, L"picture", CSLEN_(L"picture"), option);
		::RegCloseKey(hkKindMap);
		if (S_OK == hr)
			hr = SetKeyValue(HK_CLASSES_ROOT, szDotExt, L"ContentType", szContentType, cchContType, option);
	}
	return hr;
}


_Success_(return == S_OK) HRESULT RegSvr::OpenSetupKey(_In_ PCWSTR szSubkey, _Out_ PHKEY phKey, _Out_opt_ bool* wasCreated)
{
	if (!m_hkSetup)
	{
		const HRESULT hr = OpenKey(HK_LOCAL_MACHINE, REGSTR_PATH_SETUP, REGSAM_CREATE_SUBKEY, &m_hkSetup);
		if (S_OK != hr)
		{
			if (wasCreated)
				*wasCreated = false;
			return hr;
		}
	}
	return OpenKey(m_hkSetup, szSubkey, REGSAM_WRITE, phKey, wasCreated);
}


bool RegSvr::ValueContainsModule(_In_ HKEY hKey, _In_opt_ PCWSTR szSubkey, _In_opt_ PCWSTR szValName) const
{
	return (m_cchModule && ValueContainsModule(hKey, szSubkey, szValName, wcFindFileName(m_wcsModule, (int)m_cchModule)));
}

NOINLINE bool RegSvr::ValueContainsModule(_In_ HKEY hKey, _In_opt_ PCWSTR szSubkey,
	_In_opt_ PCWSTR szValName, _In_ PCWSTR szModuleName) const
{
	WCHAR wcVal[SAFE_PATH];
	DWORD cbVal = sizeof(wcVal);
	if (ERROR_SUCCESS == ::RegGetValueW(hKey, szSubkey, szValName, RRF_RT_REG_SZ|RRF_RT_REG_EXPAND_SZ, nullptr, wcVal, &cbVal))
	{
		wcVal[_countof(wcVal) - 1] = 0;
		if (::FindStringOrdinal(FIND_FROMSTART, wcVal, -1, szModuleName, -1, TRUE) >= 0)
			return true;
	}
	return false;
}


///////////////////////////////////////////////////////////////////////
// Registry helpers ///////////////////////////////////////////////////


NOALIAS _Translates_Win32_to_HRESULT_(lstat) HRESULT RegSvr::ToHRESULT(LSTATUS lstat)
{
	return HRESULT_FROM_WIN32((ULONG)lstat);
}


_Success_(return == S_OK) HRESULT RegSvr::OpenKey(_In_ HKEY hkRoot, _In_ PCWSTR szSubkey,
		REGSAM regSam, _Out_ PHKEY phKey, _Out_opt_ bool* wasCreated) const
{
	if (IsInstall())
		return CreateKey(hkRoot, szSubkey, regSam, phKey, wasCreated);

	if (wasCreated)
		*wasCreated = false;
	const LSTATUS lstat = ::RegOpenKeyExW(hkRoot, szSubkey, REGKEY_OPTION, regSam, phKey);
	if (AllTrue(ERROR_FILE_NOT_FOUND != lstat, ERROR_PATH_NOT_FOUND != lstat, ERROR_KEY_DELETED != lstat))
		return ToHRESULT(lstat);
	return S_FALSE;
}


NOALIAS _Success_(return == S_OK) HRESULT RegSvr::CreateKey(_In_ HKEY hkRoot, _In_ PCWSTR szSubkey,
		REGSAM regSam, _Out_ PHKEY phKey, _Out_opt_ bool* wasCreated)
{
	DWORD cdisp = 0;
	const LSTATUS lstat = ::RegCreateKeyExW(hkRoot, szSubkey, 0, nullptr, REGKEY_OPTION, regSam, nullptr, phKey, &cdisp);
	if (wasCreated)
		*wasCreated = (REG_CREATED_NEW_KEY == cdisp);
	if (ERROR_SUCCESS == lstat)
		return S_OK;
	*phKey = NULL;
	return ToHRESULT(lstat);
}


NOALIAS HRESULT RegSvr::DeleteKey(_In_ HKEY hkRoot, _In_opt_ PCWSTR szSubkey)
{
	const LSTATUS lstat = ::RegDeleteTreeW(hkRoot, szSubkey);
	if (AllTrue(ERROR_FILE_NOT_FOUND != lstat, ERROR_PATH_NOT_FOUND != lstat, ERROR_KEY_DELETED != lstat))
		return ToHRESULT(lstat);
	return S_FALSE;
}


NOALIAS BOOL RegSvr::StringValueExists(_In_ HKEY hKey, _In_opt_ PCWSTR szSubkey, _In_opt_ PCWSTR szName)
{
	DWORD cb = 0;
	return (ERROR_SUCCESS == ::RegGetValueW(hKey, szSubkey, szName, RRF_RT_REG_SZ|RRF_RT_REG_EXPAND_SZ,
		nullptr, nullptr, &cb) && AllTrue(!(cb & 1), cb >= 4));
}


NOALIAS HRESULT RegSvr::SetKeyValue(_In_ HKEY hKey, _In_opt_ PCWSTR szSubKey, _In_opt_ PCWSTR szValueName,
		_In_reads_(cchVal + 1) PCWSTR szValue, UINT cchVal, bool option)
{
#if (REGKEY_OPTION != 0)
	HRESULT hr;
	if (szSubKey)
	{
		bool keyCreated;
		hr = CreateKey(hKey, szSubKey, REGSAM_SET_VALUE, &hKey, &keyCreated);
		if (S_OK == hr)
		{
			hr = ERROR_ALREADY_EXISTS;
			if (keyCreated || !StringValueExists(hKey, szSubKey, szValueName))
				hr = SetValue(hKey, szValueName, szValue, cchVal);
			::RegCloseKey(hKey);
		}
	}
	else hr = SetValue(hKey, szValueName, szValue, cchVal);
	return hr;
#else
	return (option && StringValueExists(hKey, szSubKey, szValueName)) ? (HRESULT)ERROR_ALREADY_EXISTS :
		((szSubKey && szSubKey[0]) ? ToHRESULT(::RegSetKeyValueW(hKey, szSubKey, szValueName, REG_SZ,
			(PCBYTE)szValue, (cchVal + 1) * sizeof(WCHAR))) : SetValue(hKey, szValueName, szValue, cchVal));
#endif
}


NOALIAS HRESULT RegSvr::SetValue(_In_ HKEY hKey, _In_opt_ PCSTR szName,
		DWORD regType, _In_reads_bytes_(cbSize) PCVOID pValue, UINT cbSize)
{
	PWSTR pwcName = nullptr;
	if (szName && szName[0])
	{
		constexpr size_t WNAME_BUFF_SIZE = 64;
		ASSERT(strlen(szName) < WNAME_BUFF_SIZE);
		pwcName = (PWSTR)_alloca(WNAME_BUFF_SIZE * sizeof(WCHAR));
		if (!wcAsciiToWide(szName, (int)WNAME_BUFF_SIZE, pwcName))
			return HRESULT_WIN_ERROR;
	}
	return ToHRESULT(::RegSetValueExW(hKey, pwcName, 0, regType, (PCBYTE)pValue, cbSize));
}

NOALIAS HRESULT RegSvr::SetValue(_In_ HKEY hKey, _In_opt_ PCWSTR szName,
		_In_reads_(cchVal + 1) PCWSTR szValue, UINT cchVal)
{
	return ToHRESULT(::RegSetValueExW(hKey, szName, 0, REG_SZ, (PCBYTE)szValue, (cchVal + 1) * sizeof(WCHAR)));
}

NOALIAS HRESULT RegSvr::SetValue(_In_ HKEY hKey, _In_opt_ PCSTR szName,
		_In_reads_(cchVal + 1) PCWSTR szValue, UINT cchVal)
{
	return SetValue(hKey, szName, REG_SZ, szValue, (cchVal + 1) * sizeof(WCHAR));
}

NOALIAS HRESULT RegSvr::SetValue(_In_ HKEY hKey, _In_opt_ PCSTR szName, _In_ PCSTR szValue)
{
	WCHAR wcVal[80];
	ASSERT(strlen_s(szValue) < _countof(wcVal));
	const UINT cchVal = wcAsciiToWide(szValue, _countof(wcVal), wcVal);
	if (cchVal)
		return SetValue(hKey, szName, REG_SZ, wcVal, (cchVal + 1) * sizeof(WCHAR));
	return HRESULT_WIN_ERROR;
}

NOALIAS HRESULT RegSvr::SetValue(_In_ HKEY hKey, const UValData& Value)
{
	PCWSTR szValue = Value.szWideVal;
	UINT cchValue = (UINT)(Value.cchValue + 1) * sizeof(WCHAR);
	if (Value_DWORD == Value.cchValue)
		return SetValue(hKey, Value.szName, REG_DWORD, &Value.dwValue, sizeof(Value.dwValue));
	if (Value.cchValue < 0)
	{
		constexpr int WCHAR_BUFF_SIZE = 80;
		szValue = (PCWSTR)_alloca((size_t)WCHAR_BUFF_SIZE * sizeof(WCHAR));
		if (Value_GUID == Value.cchValue)
			cchValue = (UINT)(StringFromGUID2(*(Value.pGuid), const_cast<PWSTR>(szValue), WCHAR_BUFF_SIZE) - 1);
		else
			cchValue = wcAsciiToWide(Value.szAsciiVal, WCHAR_BUFF_SIZE, const_cast<PWSTR>(szValue));
		if ((int)cchValue <= 0)
			return SELFREG_E_CLASS;
	}
	return SetValue(hKey, Value.szName, szValue, cchValue);
}

NOALIAS HRESULT RegSvr::SetValues(_In_ HKEY hKey, _In_reads_(cValues) PCUValData rgValues, UINT cValues)
{
	HRESULT hr = S_FALSE;
	if (hKey)
	{
		for (; cValues; cValues--, rgValues++)
		{
			hr = SetValue(hKey, *rgValues);
			if (FAILED(hr))
				break;
			hr = S_OK;
		}
	}
	return hr;
}


}	// namespace
#endif	// WCX_SUPPORT_SELFREG ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Component regisration exports //////////////////////////////////////


WCXSTDAPI wcDllInstall(BOOL bInstall, _In_reads_opt_(cArgs) const PCWSTR* rgszArgs, int cArgs)
{
#ifdef WCX_SUPPORT_SELFREG
	UINT8 compMask = 0;	// SvgThumbProvider|WmfThumbProvider|SvgDecoder|WmfDecoder

	if (AllTrue(rgszArgs, cArgs > 0))
	{
		ASSUME(rgszArgs);
		for (; cArgs; cArgs--)
		{
			const PCWSTR szArg = rgszArgs[0];
			rgszArgs++;
			const UINT8 cmp = ReadCompArg(szArg);
			if (cmp)
			{
				compMask |= cmp;
				continue;
			}
			if (szArg && L'U' == TOUPPER_(szArg[0]) && 0 == szArg[1])
			{
				bInstall = FALSE;
				continue;
			}
			return E_INVALIDARG;
		}
	}
	else if (cArgs < 0)
		return E_INVALIDARG;

	HRESULT hr;
	if (bInstall)
	{
		hr = VerifyVersion();
		if (FAILED(hr))
			return hr;
	}
	hr = OpenRootKeys();
	if (SUCCEEDED(hr))
	{
		RegSvr RegSvr;
		hr = RegSvr.DllInstall(compMask ? compMask : ServComp_All, bInstall);
		CloseRootKeys();
	}
	return hr;
#else
	return E_NOTIMPL;
#endif
}


STDAPI RunDllInstall(_In_opt_ HWND hWnd, UINT idMsg, _In_opt_ PCSTR szCmdLine, LPARAM lParam)
{
#ifdef WCX_SUPPORT_SELFREG
	PWSTR szwCmd = nullptr;
	PWSTR* prgArgs = nullptr;
	PCWSTR* rgszArgs = nullptr;
	int cArgs = 0;
	BOOL bInstall = TRUE;
	union {
		HRESULT hr;
		UINT clen;
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
			if ((int)clen <= 0 || !(prgArgs = ::CommandLineToArgvW(szwCmd, &cArgs)))
			{
				hr = HRESULT_WIN_ERROR;
				free(szwCmd);
				return hr;
			}
			rgszArgs = const_cast<PCWSTR*>(prgArgs);
			if (cArgs > 0)
			{
				const PCWSTR szSwitch = wcCheckCLSwitch(rgszArgs[0]);
				if (szSwitch && wcAsciiIsAnyEqual(szSwitch, 3, "u", "x", "uninstall"))
				{
					rgszArgs++;
					cArgs--;
					bInstall = FALSE;
				}
			}
		}
	}
	hr = wcDllInstall(bInstall, rgszArgs, cArgs);
	::LocalFree(prgArgs);
	free(szwCmd);
	return hr;
#else
	return E_NOTIMPL;
#endif
}