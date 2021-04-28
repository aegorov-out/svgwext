// ClassFactory.cpp

#include "pch.h"
#include "Main.h"

namespace RootNamespace {

///////////////////////////////////////////////////////////////////////
// Class factory //////////////////////////////////////////////////////


class DllClassFactory final : public IClassFactory
{
	volatile ULONG m_cRef;
	const ServComp m_compType;

public:
	DllClassFactory(_In_ ServComp compType, _In_ UINT cRef = 1) : m_cRef(cRef), m_compType(compType) { DllAddRef(); }
	~DllClassFactory() { DllRelease(); }

	DECLARE_IUNKNOWN;

	HRESULT STDMETHODCALLTYPE CreateInstance(_In_opt_ IUnknown *pUnkOuter,
			_In_ REFIID riid, _COM_Outptr_ void **ppvObject) override;
	HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock) override;

	ZEROINIT_NEW_DELETE
};



HRESULT DllClassFactory::QueryInterface(REFIID riid, _COM_Outptr_ void** ppvObject)
{
	HRESULT hr = E_POINTER;
	if (ppvObject)
	{
		if (IsEqualGUID2(riid, IID_IClassFactory, IID_IUnknown))
		{
			_InterlockedIncrement(&m_cRef);
			*ppvObject = static_cast<IClassFactory*>(this);
			return S_OK;
		}
		*ppvObject = nullptr;
		hr = E_NOINTERFACE;
	}
	return hr;
}

ULONG DllClassFactory::AddRef()
{
	ADDREF_IMPL;
}

ULONG DllClassFactory::Release()
{
	RELEASE_IMPL;
}


HRESULT DllClassFactory::CreateInstance(_In_opt_ IUnknown* pUnkOuter,
		_In_ REFIID riid, _COM_Outptr_ void** ppvObject)
{
	HRESULT hr = E_POINTER;
	if (ppvObject)
	{
		*ppvObject = nullptr;
		hr = CLASS_E_NOAGGREGATION;
		if (nullptr == pUnkOuter)
		{
			switch (m_compType)
			{
			case ServComp::SvgThumbProvider:
				hr = wcCreateSvgThumbProvider(riid, ppvObject);
				break;
			case ServComp::WmfThumbProvider:
				hr = wcCreateWmfEmfThumbProvider(riid, ppvObject);
				break;
			case ServComp::SvgDecoder:
				hr = wcCreateSvgComponent(riid, ppvObject);
				break;
			case ServComp::WmfDecoder:
				hr = wcCreateWmfEmfComponent(riid, ppvObject);
				break;
			default:
				hr = CLASS_E_CLASSNOTAVAILABLE;
			}
		}
	}
	return hr;
}


HRESULT DllClassFactory::LockServer(BOOL fLock)
{
	if (fLock)
		DllAddRef();
	else if (DllGetRefCount())
		DllRelease();
	else
		return E_FAIL;
	return S_OK;
}


///////////////////////////////////////////////////////////////////////
// COM server exports /////////////////////////////////////////////////


_Check_return_ STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv)
{
	HRESULT hr = E_POINTER;
	if (ppv)
	{
		*ppv = nullptr;
		hr = VerifyVersion();
		if (SUCCEEDED(hr))
		{
			ServComp ctype;
			if (IsEqualCLSID(rclsid, CLSID_SvgThumbnailProvider))
				ctype = ServComp::SvgThumbProvider;
			else if (IsEqualCLSID(rclsid, CLSID_WmfEmfThumbnailProvider))
				ctype = ServComp::WmfThumbProvider;
			else if (IsEqualCLSID(rclsid, CLSID_SvgDecoder))
				ctype = ServComp::SvgDecoder;
			else if (IsEqualCLSID(rclsid, CLSID_WmfEmfDecoder))
				ctype = ServComp::WmfDecoder;
			else
				return CLASS_E_CLASSNOTAVAILABLE;
			hr = E_NOINTERFACE;
			if (IsEqualGUID2(riid, IID_IClassFactory, IID_IUnknown))
				hr = (*ppv = static_cast<IClassFactory*>(new DllClassFactory(ctype))) ? S_OK : E_OUTOFMEMORY;
		}
	}
	return hr;
}


__control_entrypoint(DllExport) STDAPI DllCanUnloadNow()
{
	return (DllGetRefCount() ? S_FALSE : S_OK);
}


}	// namespace