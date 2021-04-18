// Globals.cpp

#include "pch.h"
#include "Main.h"

namespace RootNamespace {

///////////////////////////////////////////////////////////////////////
// DX /////////////////////////////////////////////////////////////////


static ID2D1Factory6* g_d2dFactory = nullptr;	// should never be reloaded
static IDXGIFactory2* g_dxgiFactory = nullptr;
static ID2D1Device5* g_d2dDevice = nullptr;
static DWORD g_tLastMetricUpdate = 0;



static HRESULT __fastcall EnsureD2DFactory_()
{
	return (g_d2dFactory ? S_OK : wcCreateD2DFactory(TRUE, &g_d2dFactory));
}

_Success_(return == S_OK) HRESULT __fastcall GetD2DFactory_(_COM_Outptr_ ID2D1Factory6** ppFact)
{
	const HRESULT hr = EnsureD2DFactory_();
	if (S_OK == hr)
	{
		*ppFact = g_d2dFactory;
		g_d2dFactory->AddRef();
		return S_OK;
	}
	*ppFact = nullptr;
	return hr;
}

_Success_(return == S_OK) HRESULT __fastcall GetD2DFactory(_COM_Outptr_ ID2D1Factory6** ppFact)
{
	EnterGlobalCS();
	const HRESULT hr = GetD2DFactory_(ppFact);
	LeaveGlobalCS();
	return hr;
}


_Success_(return == S_OK)
static HRESULT CreateGdiMetafile_(_In_ IStream* pstm, _COM_Outptr_ ID2D1GdiMetafile** ppMetafile, _Out_ D2D_RECT_F* pBounds)
{
	HRESULT hr = EnsureD2DFactory_();
	if (S_OK == hr)
	{
		ID2D1GdiMetafile* pmf;
		hr = g_d2dFactory->CreateGdiMetafile(pstm, &pmf);
		if (S_OK == hr)
		{
			hr = pmf->GetBounds(pBounds);
			if (S_OK == hr)
			{
				if (pBounds->right > pBounds->left && pBounds->bottom > pBounds->top)
				{
					*ppMetafile = pmf;
					return S_OK;
				}
				hr = WINCODEC_ERR_IMAGESIZEOUTOFRANGE;
			}
			pmf->Release();
		}
	}
	Zero16Bytes(pBounds);
	*ppMetafile = nullptr;
	return hr;
}

_Success_(return == S_OK)
HRESULT CreateGdiMetafile(_In_ IStream* pstm, _COM_Outptr_ ID2D1GdiMetafile** ppMetafile, _Out_ D2D_RECT_F* pBounds)
{
	EnterGlobalCS();
	const HRESULT hr = CreateGdiMetafile_(pstm, ppMetafile, pBounds);
	LeaveGlobalCS();
	return hr;
}


static void ReleaseDX_();


static HRESULT CreateDX_()
{
	IDXGIDevice1* dxgiDevice;
	ID3D11Device* d3dDevice;
	HRESULT hr = EnsureD2DFactory_();
	if (S_OK == hr)
	{
		hr = wcCreateDXDevices(TRUE, &d3dDevice, &dxgiDevice);
		if (S_OK == hr)
		{
			IDXGIAdapter* dxgiAdapter;
			hr = dxgiDevice->GetAdapter(&dxgiAdapter);
			if (S_OK == hr)
			{
				hr = dxgiAdapter->GetParent(IID_IDXGIFactory2, PPV_ARG(&g_dxgiFactory));
				dxgiAdapter->Release();
				if (S_OK == hr)
					hr = g_d2dFactory->CreateDevice(dxgiDevice, &g_d2dDevice);
			}
			dxgiDevice->Release();
			d3dDevice->Release();
		}
		if (S_OK == hr)
			return S_OK;
		ReleaseDX_();
	}
	return hr;
}


static HRESULT RecreateDX_()
{
	ReleaseDX_();
	return CreateDX_();
}


static HRESULT PrepareDX_()
{
	if (AllTrue(g_dxgiFactory, g_d2dDevice) && g_dxgiFactory->IsCurrent())
		return S_OK;
	ReleaseDX_();
	return CreateDX_();
}


_Success_(return == S_OK) HRESULT __fastcall GetD2DDevice(_COM_Outptr_ ID2D1Device5** ppDevice)
{
	EnterGlobalCS();
	const HRESULT hr = PrepareDX_();
	*ppDevice = nullptr;
	if (S_OK == hr)
	{
		*ppDevice = g_d2dDevice;
		g_d2dDevice->AddRef();
	}
	LeaveGlobalCS();
	return hr;
}


WARNING_SUPPRESS(6388 28196) _Success_(return == S_OK)
HRESULT __fastcall CreateD2DC(_COM_Outptr_ ID2D1DeviceContext5** ppDC)
{
	*ppDC = nullptr;
	EnterGlobalCS();
	HRESULT hr = PrepareDX_();
	if (S_OK == hr)
	{
		hr = g_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS, ppDC);
		if (S_OK != hr && wcIsDXRecreateError(hr))
		{
			hr = RecreateDX_();
			if (S_OK == hr)
				hr = g_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS, ppDC);
		}
		if (S_OK == hr)
			(*ppDC)->SetDpi(96, 96);
	}
	LeaveGlobalCS();
	return hr;
}


D2D_POINT_2F GetDesktopDpi()
{
	D2D_POINT_2F dpi = { 96, 96 };
	if (S_OK == EnsureD2DFactory_())
	{
		const DWORD ticks = ::GetTickCount();
		if (ticks - g_tLastMetricUpdate > 600)
		{
			g_tLastMetricUpdate = ticks;
			g_d2dFactory->ReloadSystemMetrics();
		}
		WARNING_SUPPRESS(4996) g_d2dFactory->GetDesktopDpi(&dpi.x, &dpi.y);
	}
	return dpi;
}

UINT GetScreenDpi()
{
	const D2D_POINT_2F dpiF = GetDesktopDpi();
	const UINT dpi = ftoui32f((dpiF.x + dpiF.y) / 2);
	return (dpi ? dpi : 96);
}


NOALIAS BOOL IsCurrentDXAdapter()
{
	return (g_dxgiFactory ? g_dxgiFactory->IsCurrent() : FALSE);
}


HRESULT __fastcall HandleDXResult(HRESULT hr)
{
	HRESULT hrDX;
	if (S_OK == hr || 0 == (hrDX = (HRESULT)wcIsDXRecreateError(hr)))
		return hr;
	ReleaseDX_();
	return hrDX;
}


static void ReleaseDX_()
{
	SafeRelease(&g_d2dDevice);
	SafeRelease(&g_dxgiFactory);
}


///////////////////////////////////////////////////////////////////////
// WIC ////////////////////////////////////////////////////////////////


static IWICImagingFactory2* g_wicFactory = nullptr;


static HRESULT __fastcall EnsureWicFactory_()
{
	return (g_wicFactory ? S_OK : wcCreateWicFactory(&g_wicFactory));
}

_Success_(return == S_OK) HRESULT __fastcall GetWicFactory_(_COM_Outptr_ IWICImagingFactory2** ppFact)
{
	const HRESULT hr = EnsureWicFactory_();
	if (S_OK == hr)
	{
		*ppFact = g_wicFactory;
		g_wicFactory->AddRef();
		return S_OK;
	}
	*ppFact = nullptr;
	return hr;
}

_Success_(return == S_OK) HRESULT __fastcall GetWicFactory(_COM_Outptr_ IWICImagingFactory2** ppFact)
{
	EnterGlobalCS();
	const HRESULT hr = GetWicFactory_(ppFact);
	LeaveGlobalCS();
	return hr;
}


_Success_(return == S_OK) HRESULT CreateWicTarget_(_In_ D2D_SIZE_U size,
		_COM_Outptr_ ID2D1RenderTarget** ppTarg, _COM_Outptr_ IWICBitmap** ppBm)
{
	HRESULT hr = EnsureWicFactory_();
	if (S_OK == hr)
	{
		hr = PrepareDX_();
		if (S_OK == hr)
		{
			IWICBitmap* pbm;
			hr = g_wicFactory->CreateBitmap(size.width, size.height, GUID_WICPixelFormat32bppBGRA, WICBitmapCacheOnLoad, &pbm);
			if (S_OK == hr)
			{
				ID2D1RenderTarget* prt;
				const D2D1_RENDER_TARGET_PROPERTIES props = {
					D2D1_RENDER_TARGET_TYPE_HARDWARE, { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_STRAIGHT },
					96, 96, D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT
				};
				hr = g_d2dFactory->CreateWicBitmapRenderTarget(pbm, &props, &prt);
				if (S_OK == hr)
				{
					*ppBm = pbm;
					*ppTarg = prt;
					prt->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
					return S_OK;
				}
				pbm->Release();
			}
		}
	}
	*ppBm = nullptr;
	*ppTarg = nullptr;
	return hr;
}


_Success_(return == S_OK)
HRESULT CreateWicTarget(_In_ D2D_SIZE_U size, _COM_Outptr_ ID2D1RenderTarget** ppTarg, _COM_Outptr_ IWICBitmap** ppBm)
{
	EnterGlobalCS();
	const HRESULT hr = CreateWicTarget_(size, ppTarg, ppBm);
	LeaveGlobalCS();
	return hr;
}


static void ReleaseWic_()
{
	SafeRelease(&g_wicFactory);
}


///////////////////////////////////////////////////////////////////////
// Common globals /////////////////////////////////////////////////////


void ReleaseGlobalFactories()
{
	ReleaseWic_();
	ReleaseDX_();
	SafeRelease(&g_d2dFactory);
}


}	// namespace