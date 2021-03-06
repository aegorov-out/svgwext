// ThumbProviderBase.cpp

#include "pch.h"
#include "DecodeBase.h"

#pragma warning(disable: 6387 6388 26495 28196)


namespace RootNamespace {

///////////////////////////////////////////////////////////////////////
// Base thumbnail provider ////////////////////////////////////////////


class NOVTABLE ThumbProviderBase abstract : public IThumbnailProvider,
		public IPropertyStore, public IPropertyStoreCapabilities,
		public IInitializeWithStream, public IInitializeWithItem, public IInitializeWithFile
{
	IMPL_UNCOPYABLE(ThumbProviderBase)

protected:
	// PKEY_Kind, PKEY_Image_Dimensions, PKEY_Image_HorizontalSize, PKEY_Image_VerticalSize, PKEY_Image_BitDepth
	static constexpr auto COMMON_PROP_COUNT = 5u;
	static constexpr HRESULT E_UNKNOWN_PROPERTY = __HRESULT_FROM_WIN32(ERROR_UNKNOWN_PROPERTY);

	typedef HRESULT(__fastcall* PROPHANDLERPROC)(_In_ const ThumbProviderBase* pThis, _Inout_ PROPVARIANT* pv);

	typedef struct PropHandler {
		const PROPERTYKEY* propKey;
		PROPHANDLERPROC pfnHandler;
	} const * PCPropHandler;

	volatile ULONG m_cRef;
	const UINT m_cProps;
	_Field_size_(m_cProps) const PCPropHandler m_rgProps;
	ID2D1DeviceContext5* m_d2dDC;
	D2D_SIZE_F m_sizeDips;
	union {
		IUnknown* m_unkData;
		ID2D1SvgDocument* m_svgDoc;
		ID2D1GdiMetafile1* m_metafile;
	};
	IStream* m_pstmInit;
	volatile mutable LONG64 m_mutexLock;


	ThumbProviderBase(_In_reads_(cProps) PCPropHandler rgProps, UINT cProps, _In_ ULONG cRef = 1)
			: m_cRef(cRef), m_cProps(cProps), m_rgProps(rgProps) { DllAddRef(); }

	bool IsInit() const { return AllTrue(m_d2dDC, m_unkData); }
	UINT GetWidth() const { return wcToImageSizeU(m_sizeDips.width); }
	UINT GetHeight() const { return wcToImageSizeU(m_sizeDips.height); }

	void EnterCS() const { wcEnterInterlockedMutex(&m_mutexLock); }
	void LeaveCS() const { wcLeaveInterlockedMutex(&m_mutexLock); }

	void Release_();
	void ClearLocked();
	virtual void Clear();

	HRESULT InitLoad(_In_ IStream* pstm);
	virtual HRESULT InitLoad_(_In_ IStream* pstm) = 0;
	_Success_(return == S_OK) virtual HRESULT CreateCpuReadBitmap_(D2D_SIZE_F drawSize,
			D2D_SIZE_U targSize, _COM_Outptr_ ID2D1Bitmap1** ppbmMap) const = 0;

	static const PropHandler s_rgPropHandlers[COMMON_PROP_COUNT];

	static HRESULT __fastcall GetKindProp(_In_ const ThumbProviderBase* pThis, _Inout_ PROPVARIANT* pv);
	static HRESULT __fastcall GetHorzSizeProp(_In_ const ThumbProviderBase* pThis, _Inout_ PROPVARIANT* pv);
	static HRESULT __fastcall GetVertSizeProp(_In_ const ThumbProviderBase* pThis, _Inout_ PROPVARIANT* pv);
	static HRESULT __fastcall GetDimensionsProp(_In_ const ThumbProviderBase* pThis, _Inout_ PROPVARIANT* pv);
	static HRESULT __fastcall GetBitDepthProp(_In_ const ThumbProviderBase* pThis, _Inout_ PROPVARIANT* pv);

	HRESULT GetValue(_In_reads_(cProps) PCPropHandler rgProps, UINT cProps, REFPROPERTYKEY key, _Inout_ PROPVARIANT* pv) const;

public:
	virtual ~ThumbProviderBase() { Release_(); DllRelease(); }

	DECLARE_IUNKNOWN;

	HRESULT STDMETHODCALLTYPE GetThumbnail(UINT cx, __RPC__deref_out_opt HBITMAP* phbmp,
			__RPC__out WTS_ALPHATYPE* pdwAlpha) override;

	HRESULT STDMETHODCALLTYPE Initialize(_In_ IStream* pstream, DWORD grfMode) override;
	HRESULT STDMETHODCALLTYPE Initialize(__RPC__in_opt IShellItem *psi, DWORD grfMode) override;
	HRESULT STDMETHODCALLTYPE Initialize(__RPC__in_string LPCWSTR pszFilePath, DWORD grfMode) override;

	HRESULT STDMETHODCALLTYPE GetCount(__RPC__out DWORD* pcProps) override;
	HRESULT STDMETHODCALLTYPE GetAt(DWORD iProp, __RPC__out PROPERTYKEY* pkey) override;
	HRESULT STDMETHODCALLTYPE GetValue(__RPC__in REFPROPERTYKEY key, __RPC__out PROPVARIANT* pv) override;
	HRESULT STDMETHODCALLTYPE SetValue(__RPC__in REFPROPERTYKEY key, __RPC__in REFPROPVARIANT propvar) override;
	HRESULT STDMETHODCALLTYPE Commit();
	HRESULT STDMETHODCALLTYPE IsPropertyWritable(__RPC__in REFPROPERTYKEY key) override;
};


inline PROPERTYKEY* CopyPROPERTYKEY(PROPERTYKEY* targ, REFPROPERTYKEY src)
{
	targ->pid = src.pid;
	CopyGUID(&targ->fmtid, src.fmtid);
	return targ;
}

inline PROPERTYKEY* ClearPROPERTYKEY(PROPERTYKEY* ppk)
{
	ppk->pid = 0;
	Zero16Bytes(&ppk->fmtid);
	return ppk;
}


///////////////////////////////////////////////////////////////////////
// Implementation /////////////////////////////////////////////////////


void ThumbProviderBase::Release_() 
{
	if (m_unkData)
		m_unkData->Release();
	if (m_d2dDC)
		m_d2dDC->Release();
	if (m_pstmInit)
		m_pstmInit->Release();
}

void ThumbProviderBase::ClearLocked()
{
	EnterCS();
	Clear();
	LeaveCS();
}

void ThumbProviderBase::Clear()
{
	Release_();
	ZeroStructFromP(this, m_d2dDC);
}


// IUnknown ///////////////////////////////////////////////////////////


WARNING_SUPPRESS(6101 6388 28196)
HRESULT ThumbProviderBase::QueryInterface(REFIID riid, _COM_Outptr_ void** ppvObject)
{
	const HRESULT hr = QueryInterfaceImpl_(riid, ppvObject, 6, QI_ARG(IThumbnailProvider, this),
			QI_ARG(IPropertyStore, this), QI_ARG(IPropertyStoreCapabilities, this),
			QI_ARG(IInitializeWithStream, this), QI_ARG(IInitializeWithItem, this), QI_ARG(IInitializeWithFile, this));
	if (S_OK == hr)
		_InterlockedIncrement(&m_cRef);
	return hr;
}

ULONG ThumbProviderBase::AddRef()
{
	ADDREF_IMPL;
}

ULONG ThumbProviderBase::Release()
{
	RELEASE_IMPL;
}


HRESULT ThumbProviderBase::InitLoad(_In_ IStream* pstm)
{
	HRESULT hr = WINCODEC_ERR_WRONGSTATE;
	EnterCS();
	if (!IsInit())
		hr = InitLoad_(pstm);
	LeaveCS();
	return hr;
}


// IThumbnailProvider /////////////////////////////////////////////////


HRESULT ThumbProviderBase::GetThumbnail(UINT cx, __RPC__deref_out_opt HBITMAP* phbmp, __RPC__out WTS_ALPHATYPE* pdwAlpha)
{
	HRESULT hr = E_POINTER;
	if (phbmp)
	{
		*phbmp = nullptr;
		if (pdwAlpha)
			*pdwAlpha = WTSAT_ARGB;
		hr = WINCODEC_ERR_NOTINITIALIZED;
		EnterCS();
		if (IsInit())
		{
			const D2D_SIZE_F sizeF = wcScaleSizeF(m_sizeDips, cx);
			ASSUME(FAILED(hr));
			if (AllTrue(sizeF.width >= 1, sizeF.height >= 1))
			{
				const D2D_SIZE_U sizeU = wcToImageSizeU(sizeF);
				D2D_MATRIX_3X2_F transform = D2D1::Matrix3x2F::Identity();
				ID2D1Bitmap1* pbm;			
				hr = CreateCpuReadBitmap_(sizeF, sizeU, &pbm);
				if (S_OK == hr)
				{
					hr = wcD2DBitmapToDIB(pbm, phbmp);
					pbm->Release();
				}
				else if (wcIsDXRecreateError(hr))
				{
					Clear();
					if (m_pstmInit)
					{
						hr = Stream_SeekStart(m_pstmInit);
						if (FAILED(hr) || S_OK != (hr = InitLoad_(m_pstmInit)))
							SafeRelease(&m_pstmInit);
					}
				}
			}
		}
		LeaveCS();
	}
	return hr;
}


// IInitializeWithStream //////////////////////////////////////////////


HRESULT ThumbProviderBase::Initialize(_In_ IStream* pstream, DWORD grfMode)
{
	ClearLocked();
	return (pstream ? InitLoad(pstream) : E_INVALIDARG);
}


// IInitializeWithItem ////////////////////////////////////////////////


HRESULT ThumbProviderBase::Initialize(__RPC__in_opt IShellItem *psi, DWORD grfMode)
{
	HRESULT hr = E_INVALIDARG;
	if (psi)
	{
		IStream* pstm;
		hr = wcCreateStreamOnItem(psi, (BOOL)(grfMode & (STGM_WRITE|STGM_READWRITE)), &pstm);
		if (S_OK == hr)
		{
			hr = InitLoad(pstm);
			pstm->Release();
			return hr;
		}
	}
	ClearLocked();
	return hr;
}


// IInitializeWithFile ////////////////////////////////////////////////


HRESULT ThumbProviderBase::Initialize(__RPC__in_string LPCWSTR pszFilePath, DWORD grfMode)
{
	HRESULT hr = E_INVALIDARG;
	if (pszFilePath)
	{
		IStream* pstm;
		hr = wcCreateStreamOnFile(pszFilePath, (BOOL)(grfMode & (STGM_WRITE|STGM_READWRITE)), &pstm);
		if (S_OK == hr)
		{
			hr = InitLoad(pstm);
			pstm->Release();
			return hr;
		}
	}
	ClearLocked();
	return hr;
}


// IPropertyStore /////////////////////////////////////////////////////


const ThumbProviderBase::PropHandler ThumbProviderBase::s_rgPropHandlers[COMMON_PROP_COUNT] = {
	{ &PKEY_Kind,					&GetKindProp },
	{ &PKEY_Image_Dimensions,		&GetDimensionsProp },
	{ &PKEY_Image_HorizontalSize,	&GetHorzSizeProp },
	{ &PKEY_Image_VerticalSize,		&GetVertSizeProp },
	{ &PKEY_Image_BitDepth,			&GetBitDepthProp }
};


HRESULT ThumbProviderBase::GetCount(__RPC__out DWORD* pcProps)
{
	if (pcProps)
	{
		EnterCS();
		if (IsInit())
		{
			*pcProps = COMMON_PROP_COUNT + m_cProps;
			LeaveCS();
			return S_OK;
		}
		LeaveCS();
		*pcProps = 0;
		return WINCODEC_ERR_NOTINITIALIZED;
	}
	return E_INVALIDARG;
}

HRESULT ThumbProviderBase::GetAt(DWORD iProp, __RPC__out PROPERTYKEY* pkey)
{
	if (pkey)
	{
		if (iProp < COMMON_PROP_COUNT)
		{
			CopyPROPERTYKEY(pkey, *(s_rgPropHandlers[iProp].propKey));
			return S_OK;
		}
		iProp -= COMMON_PROP_COUNT;
		if (iProp < m_cProps)
		{
			CopyPROPERTYKEY(pkey, *(m_rgProps[iProp].propKey));
			return S_OK;
		}
		ClearPROPERTYKEY(pkey);
		return __HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
	}
	return E_INVALIDARG;
}

HRESULT ThumbProviderBase::GetValue(__RPC__in REFPROPERTYKEY key, __RPC__out PROPVARIANT* pv)
{
	HRESULT hr = E_INVALIDARG;
	if (pv)
	{
		ZeroStruct(pv);
		hr = GetValue(s_rgPropHandlers, ARRAYSIZE(s_rgPropHandlers), key, pv);
		if (E_UNKNOWN_PROPERTY == hr)
			hr = GetValue(m_rgProps, m_cProps, key, pv);
	}
	return hr;
}

HRESULT ThumbProviderBase::GetValue(_In_reads_(cProps) PCPropHandler rgProps,
		UINT cProps, REFPROPERTYKEY key, _Inout_ PROPVARIANT* pv) const
{
	for (; cProps; cProps--, rgProps++)
	{
		if (!IsEqualPropertyKey(*(rgProps->propKey), key))
			continue;
		return rgProps->pfnHandler(this, pv);
	}
	return E_UNKNOWN_PROPERTY;
}


HRESULT ThumbProviderBase::GetKindProp(_In_ const ThumbProviderBase* pThis, _Inout_ PROPVARIANT* pv)
{
	if (pv->pwszVal = (PWSTR)::CoTaskMemAlloc(sizeof(KIND_PICTURE)))
	{
		V_VT(pv) = VT_LPWSTR;
		wmemcpy(pv->pwszVal, KIND_PICTURE, wsizeof(KIND_PICTURE));
		return S_OK;
	}
	V_VT(pv) = VT_EMPTY;
	return E_OUTOFMEMORY;
}

HRESULT ThumbProviderBase::GetHorzSizeProp(_In_ const ThumbProviderBase* pThis, _Inout_ PROPVARIANT* pv)
{
	V_VT(pv) = VT_UI4;
	V_UI4(pv) = wcToImageSizeU(pThis->m_sizeDips.width);
	return S_OK;
}

HRESULT ThumbProviderBase::GetVertSizeProp(_In_ const ThumbProviderBase* pThis, _Inout_ PROPVARIANT* pv)
{
	V_VT(pv) = VT_UI4;
	V_UI4(pv) = wcToImageSizeU(pThis->m_sizeDips.height);
	return S_OK;
}

HRESULT ThumbProviderBase::GetDimensionsProp(_In_ const ThumbProviderBase* pThis, _Inout_ PROPVARIANT* pv)
{
	WCHAR wcText[40];
	const D2D_SIZE_U sizeU = wcToImageSizeU(pThis->m_sizeDips);
	UINT clen = wcUInt32ToDec(sizeU.width, wcText, _countof(wcText));
	wcText[clen] = WCHAR_NBSP;			// L' '
	wcText[clen + 1] = WCHAR_MULTIPLY;	// L'x'
	wcText[clen + 2] = WCHAR_NBSP;		// L' '
	clen += 3;
	clen += wcUInt32ToDec(sizeU.height, wcText + clen, (int)(_countof(wcText) - clen));
	const PWSTR pwsz = (PWSTR)::CoTaskMemAlloc((SIZE_T)(clen + 1) * sizeof(WCHAR));
	if (pwsz)
	{
		wmemcpy(pwsz, wcText, clen);
		pwsz[clen] = 0;
		V_VT(pv) = VT_LPWSTR;
		pv->pwszVal = pwsz;
		return S_OK;
	}
	return E_OUTOFMEMORY;
}

HRESULT ThumbProviderBase::GetBitDepthProp(_In_ const ThumbProviderBase* pThis, _Inout_ PROPVARIANT* pv)
{
	V_VT(pv) = VT_UI4;
	V_UI4(pv) = 32;
	return S_OK;
}


HRESULT ThumbProviderBase::SetValue(__RPC__in REFPROPERTYKEY key, __RPC__in REFPROPVARIANT propvar)
{
	return STG_E_ACCESSDENIED;
}

HRESULT ThumbProviderBase::Commit()
{
	return STG_E_ACCESSDENIED;
}


// IPropertyStoreCapabilities /////////////////////////////////////////


HRESULT ThumbProviderBase::IsPropertyWritable(__RPC__in REFPROPERTYKEY key)
{
	return S_FALSE;
}


///////////////////////////////////////////////////////////////////////
// SVG thumbnail provider /////////////////////////////////////////////


class SvgThumbProvider : public ThumbProviderBase
{
	HRESULT InitLoad_(_In_ IStream* pstm) override;
	_Success_(return == S_OK) HRESULT CreateCpuReadBitmap_(D2D_SIZE_F drawSize,
		D2D_SIZE_U targSize, _COM_Outptr_ ID2D1Bitmap1**ppbmMap) const override;

	HRESULT GetRootChildText(_In_reads_(cchTag) PCWCH pwcTagName, UINT cchTag, _Inout_ PROPVARIANT* pv) const;

	static HRESULT __fastcall GetTitleProp(_In_ const SvgThumbProvider* pThis, _Inout_ PROPVARIANT* pv);
	static HRESULT __fastcall GetDescProp(_In_ const SvgThumbProvider* pThis, _Inout_ PROPVARIANT* pv);
	static HRESULT __fastcall GetResUnitProp(_In_ const SvgThumbProvider* pThis, _Inout_ PROPVARIANT* pv);

	static const PropHandler s_rgPropHandlers[4];

public:
	SvgThumbProvider() : ThumbProviderBase(s_rgPropHandlers, ARRAYSIZE(s_rgPropHandlers)) {}

	ZEROINIT_NEW_DELETE
};


const SvgThumbProvider::PropHandler SvgThumbProvider::s_rgPropHandlers[4] = {
	{ &PKEY_Title,					reinterpret_cast<PROPHANDLERPROC>(&GetTitleProp) },
	{ &PKEY_Comment,				reinterpret_cast<PROPHANDLERPROC>(&GetDescProp) },
	{ &PKEY_Subject,				reinterpret_cast<PROPHANDLERPROC>(&GetDescProp) },
	{ &PKEY_Image_ResolutionUnit,	reinterpret_cast<PROPHANDLERPROC>(&GetResUnitProp) }
};


HRESULT SvgThumbProvider::InitLoad_(_In_ IStream* pstm)
{
	return InitLoadSvg(pstm, (pstm != m_pstmInit) ? &m_pstmInit : nullptr, &m_d2dDC, &m_svgDoc, &m_sizeDips);
}

_Success_(return == S_OK)
HRESULT SvgThumbProvider::CreateCpuReadBitmap_(D2D_SIZE_F drawSize,
		D2D_SIZE_U targSize, _COM_Outptr_ ID2D1Bitmap1**ppbmMap) const
{
	return CreateSvgCpuReadBitmap(m_d2dDC, m_svgDoc, drawSize, targSize, D2D1::Matrix3x2F::Identity(), ppbmMap);
}


HRESULT SvgThumbProvider::GetTitleProp(_In_ const SvgThumbProvider* pThis, _Inout_ PROPVARIANT* pv)
{
	const union {
		struct {
			UINT64 name64;
			UINT32 name32;
		} DUMMYSTRUCTNAME;
		WCHAR wcsName[6];
	} uTag = { { MAKEULONGLONGW(L't', L'i', L't', L'l'), (UINT32)L'e' } };
	return pThis->GetRootChildText(uTag.wcsName, 5, pv);
}

HRESULT SvgThumbProvider::GetDescProp(_In_ const SvgThumbProvider* pThis, _Inout_ PROPVARIANT* pv)
{
	const union {
		UINT64 name64;
		WCHAR wcsName[4];
	} uTag = { MAKEULONGLONGW(L'd', L'e', L's', L'c') };
	return pThis->GetRootChildText(uTag.wcsName, 4, pv);
}

HRESULT SvgThumbProvider::GetRootChildText(_In_reads_(cchTag) PCWCH pwcTagName, UINT cchTag, _Inout_ PROPVARIANT* pv) const
{
	HRESULT hr = WINCODEC_ERR_NOTINITIALIZED;
	ASSUME(VT_EMPTY == V_VT(pv));
	if (m_svgDoc)
	{
		ID2D1SvgElement* root = nullptr;
		m_svgDoc->GetRoot(&root);
		hr = S_OK;
		if (root)
		{
			hr = wcGetSvgChildText(root, pwcTagName, cchTag, &pv->pwszVal);
			if (SUCCEEDED(hr))
			{
				V_VT(pv) = VT_LPWSTR;
				hr = S_OK;
			}
			root->Release();
		}
	}
	return hr;
}


HRESULT SvgThumbProvider::GetResUnitProp(_In_ const SvgThumbProvider* pThis, _Inout_ PROPVARIANT* pv)
{
	V_VT(pv) = VT_I4;
	V_I4(pv) = 1;	// No absolute unit of measurement
	return S_OK;
}


///////////////////////////////////////////////////////////////////////
// WMF thumbnail provider /////////////////////////////////////////////


class WmfThumbProvider : public ThumbProviderBase
{
	D2D_POINT_2F m_ptOrigin;

	HRESULT InitLoad_(_In_ IStream* pstm) override;
	_Success_(return == S_OK) HRESULT CreateCpuReadBitmap_(D2D_SIZE_F drawSize,
			D2D_SIZE_U targSize, _COM_Outptr_ ID2D1Bitmap1** ppbmMap) const override;
	void Clear() override;

	HRESULT GetResolution(bool vertical, _Inout_ PROPVARIANT* pv) const;

	static HRESULT __fastcall GetHorzDpiProp(_In_ const WmfThumbProvider* pThis, _Inout_ PROPVARIANT* pv);
	static HRESULT __fastcall GetVertDpiProp(_In_ const WmfThumbProvider* pThis, _Inout_ PROPVARIANT* pv);

	static const PropHandler s_rgPropHandlers[2];

public:
	WmfThumbProvider() : ThumbProviderBase(s_rgPropHandlers, ARRAYSIZE(s_rgPropHandlers)) {}

	ZEROINIT_NEW_DELETE
};


const WmfThumbProvider::PropHandler WmfThumbProvider::s_rgPropHandlers[2] = {
	{ &PKEY_Image_HorizontalResolution,	reinterpret_cast<PROPHANDLERPROC>(&GetHorzDpiProp) },
	{ &PKEY_Image_VerticalResolution,	reinterpret_cast<PROPHANDLERPROC>(&GetVertDpiProp) }
};


HRESULT WmfThumbProvider::InitLoad_(_In_ IStream* pstm)
{
	return InitLoadWmf(pstm, (pstm != m_pstmInit) ? &m_pstmInit : nullptr, &m_d2dDC, &m_metafile, &m_ptOrigin, &m_sizeDips);
}

_Success_(return == S_OK)
HRESULT WmfThumbProvider::CreateCpuReadBitmap_(D2D_SIZE_F drawSize,
		D2D_SIZE_U targSize, _COM_Outptr_ ID2D1Bitmap1**ppbmMap) const
{
	D2D_MATRIX_3X2_F transform;
	if (AnyTrue(drawSize.width < m_sizeDips.width, drawSize.height < m_sizeDips.height))
		transform = D2D1::Matrix3x2F::Scale({ drawSize.width / m_sizeDips.width, drawSize.height / m_sizeDips.height });
	else
		transform = D2D1::Matrix3x2F::Identity();

	return CreateWmfCpuReadBitmap(m_d2dDC, m_metafile, targSize, m_ptOrigin, transform, ppbmMap);
}

void WmfThumbProvider::Clear()
{
	Zero8Bytes(&m_ptOrigin);
	__super::Clear();
}


HRESULT WmfThumbProvider::GetHorzDpiProp(_In_ const WmfThumbProvider* pThis, _Inout_ PROPVARIANT* pv)
{
	return pThis->GetResolution(false, pv);
}

HRESULT WmfThumbProvider::GetVertDpiProp(_In_ const WmfThumbProvider* pThis, _Inout_ PROPVARIANT* pv)
{
	return pThis->GetResolution(true, pv);
}

HRESULT WmfThumbProvider::GetResolution(bool vertical, _Inout_ PROPVARIANT* pv) const
{
	HRESULT hr = WINCODEC_ERR_NOTINITIALIZED;
	ASSUME(VT_EMPTY == V_VT(pv));
	if (m_metafile)
	{
		FLOAT dpiX = 0, dpiY = 0;
		hr = m_metafile->GetDpi(&dpiX, &dpiY);
		if (SUCCEEDED(hr))
		{
			V_VT(pv) = VT_R8;
			V_R8(pv) = (vertical ? dpiY : dpiX);
			hr = S_OK;
		}
	}
	return hr;
}


///////////////////////////////////////////////////////////////////////
// Globals ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


template <class TP>
HRESULT __fastcall CreateXThumbProvider(_In_ REFIID riid, _COM_Outptr_result_nullonfailure_ void** ppvObject)
{
	*ppvObject = nullptr;

	HRESULT hr = E_NOINTERFACE;
	if (IsEqualGUID2(riid, IID_IThumbnailProvider, IID_IUnknown))
	{
		if (*ppvObject = static_cast<IThumbnailProvider*>(new TP()))
			return S_OK;
		hr = E_OUTOFMEMORY;
	}
	else if (IsEqualIID(riid, IID_IInitializeWithStream))
	{
		if (*ppvObject = static_cast<IInitializeWithStream*>(new TP()))
			return S_OK;
		hr = E_OUTOFMEMORY;
	}
	else if (IsEqualIID(riid, IID_IInitializeWithItem))
	{
		if (*ppvObject = static_cast<IInitializeWithItem*>(new TP()))
			return S_OK;
		hr = E_OUTOFMEMORY;
	}
	else if (IsEqualIID(riid, IID_IInitializeWithFile))
	{
		if (*ppvObject = static_cast<IInitializeWithFile*>(new TP()))
			return S_OK;
		hr = E_OUTOFMEMORY;
	}
	else if (IsEqualIID(riid, IID_IPropertyStore))
	{
		if (*ppvObject = static_cast<IPropertyStore*>(new TP()))
			return S_OK;
		hr = E_OUTOFMEMORY;
	}
	else if (IsEqualIID(riid, IID_IPropertyStoreCapabilities))
	{
		if (*ppvObject = static_cast<IPropertyStoreCapabilities*>(new TP()))
			return S_OK;
		hr = E_OUTOFMEMORY;
	}
	return hr;
}


WCXFASTAPI wcCreateSvgThumbProvider(_In_ REFIID riid, _COM_Outptr_result_nullonfailure_ void** ppvObject)
{
	return CreateXThumbProvider<SvgThumbProvider>(riid, ppvObject);
}

WCXFASTAPI wcCreateWmfEmfThumbProvider(_In_ REFIID riid, _COM_Outptr_result_nullonfailure_ void** ppvObject)
{
	return CreateXThumbProvider<WmfThumbProvider>(riid, ppvObject);
}


}	// namespace