// SvgXml.cpp

#include "pch.h"
#include "Main.h"
#include <MsXml2.h>
#include <MsXml6.h>
#include <xmllite.h>
#pragma comment(lib, "msxml2.lib")
#pragma comment(lib, "msxml6.lib")

namespace RootNamespace {

///////////////////////////////////////////////////////////////////////
// Global XML factory /////////////////////////////////////////////////


static IClassFactory* g_xmlSaxFactory = nullptr;


void ReleaseXmlGlobals()
{
	__try
	{
		if (g_xmlSaxFactory)
			g_xmlSaxFactory->Release();
	}
	__finally
	{
		g_xmlSaxFactory = nullptr;
	}
}


_Success_(return == S_OK) static HRESULT __fastcall GetXmlSaxFactory_(IClassFactory** factory)
{
	if (!g_xmlSaxFactory)
	{
		HRESULT hr = ::CoGetClassObject(CLSID_SAXXMLReader60, CLSCTX_INPROC_SERVER,
				NULL, IID_IClassFactory, PPV_ARG(&g_xmlSaxFactory));
		if (S_OK != hr)
		{
			hr = ::CoGetClassObject(CLSID_SAXXMLReader30, CLSCTX_INPROC_SERVER,
					NULL, IID_IClassFactory, PPV_ARG(&g_xmlSaxFactory));
			if (S_OK != hr)
			{
				g_xmlSaxFactory = nullptr;
				return hr;
			}
		}
	}
	*factory = g_xmlSaxFactory;
	g_xmlSaxFactory->AddRef();
	return S_OK;
}


_Success_(return == S_OK) static HRESULT __fastcall CreateXmlSaxReader(ISAXXMLReader** reader)
{
	*reader = nullptr;

	EnterGlobalCS();
	IClassFactory* factory;
	HRESULT hr;
	__try
	{
		hr = GetXmlSaxFactory_(&factory);
		if (S_OK == hr)
		{
			hr = factory->CreateInstance(nullptr, IID_ISAXXMLReader, PPV_ARG(reader));
			factory->Release();
		}
	}
	__finally
	{
		LeaveGlobalCS();
	}
	return hr;
}


///////////////////////////////////////////////////////////////////////
// XML parser /////////////////////////////////////////////////////////


class SvgContentHandler : public ISAXContentHandler
{
	bool m_docStarted = false;
	bool m_parsed = false;;
	bool m_isSvg = false;
public:
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void **ppvObject) override;
	ULONG STDMETHODCALLTYPE AddRef() override { return 2; }
	ULONG STDMETHODCALLTYPE Release() override { return 1; }

	HRESULT STDMETHODCALLTYPE putDocumentLocator(ISAXLocator *pLocator) override { return S_OK;  }
	HRESULT STDMETHODCALLTYPE startDocument() override { m_docStarted = true;  return S_OK; }
	HRESULT STDMETHODCALLTYPE endDocument() override { return E_FAIL; }
	HRESULT STDMETHODCALLTYPE startPrefixMapping(const wchar_t *pwchPrefix, int cchPrefix,
		const wchar_t *pwchUri, int cchUri) override { return S_OK; }
	HRESULT STDMETHODCALLTYPE endPrefixMapping(const wchar_t *pwchPrefix, int cchPrefix) override { return S_OK; }
	HRESULT STDMETHODCALLTYPE startElement(const wchar_t *pwchNamespaceUri, int cchNamespaceUri,
		const wchar_t *pwchLocalName, int cchLocalName, const wchar_t *pwchQName, int cchQName,
		ISAXAttributes *pAttributes) override;
	HRESULT STDMETHODCALLTYPE endElement(const wchar_t *pwchNamespaceUri, int cchNamespaceUri,
		const wchar_t *pwchLocalName, int cchLocalName, const wchar_t *pwchQName, int cchQName) override { return E_FAIL; }
	HRESULT STDMETHODCALLTYPE characters(const wchar_t *pwchChars, int cchChars) override { return S_OK; }
	HRESULT STDMETHODCALLTYPE ignorableWhitespace(const wchar_t *pwchChars, int cchChars) override { return S_OK; }
	HRESULT STDMETHODCALLTYPE processingInstruction(const wchar_t *pwchTarget, int cchTarget,
		const wchar_t *pwchData, int cchData) override { return S_OK; }
	HRESULT STDMETHODCALLTYPE skippedEntity(const wchar_t *pwchName, int cchName) override { return S_OK; }

	bool IsParsed() const { return AllTrue(m_docStarted, m_parsed); }
	bool IsSvg() const { return m_isSvg; }
};


HRESULT STDMETHODCALLTYPE SvgContentHandler::QueryInterface(REFIID riid, _COM_Outptr_ void **ppvObject)
{
	HRESULT hr = E_POINTER;
	if (ppvObject)
	{
		if (IsEqualGUID2(riid, IID_ISAXContentHandler, IID_IUnknown))
		{
			*ppvObject = static_cast<ISAXContentHandler*>(this);
			return S_OK;
		}
		*ppvObject = NULL;
		hr = E_NOINTERFACE;
	}
	return hr;
}


HRESULT STDMETHODCALLTYPE SvgContentHandler::startElement(const wchar_t *pwchNamespaceUri, int cchNamespaceUri,
	const wchar_t *pwchLocalName, int cchLocalName, const wchar_t *pwchQName, int cchQName, ISAXAttributes *pAttributes)
{
	m_parsed = true;
	if (AllTrue(m_docStarted, 3 == cchLocalName, !m_isSvg) && AllTrue(L's' == TOLOWER_(pwchLocalName[0]),
		L'v' == TOLOWER_(pwchLocalName[1]), L'g' == TOLOWER_(pwchLocalName[2])))
	{
		m_isSvg = (cchNamespaceUri <= 0 || FindStringOrdinal(FIND_FROMEND, pwchNamespaceUri, cchNamespaceUri, L"/svg", 4, TRUE) > 0);
	}
	return E_FAIL;
}


///////////////////////////////////////////////////////////////////////
// SVG verifiers //////////////////////////////////////////////////////


NOALIAS static bool IsXmlError(HRESULT hr)
{
	return AllTrue(hr >= (HRESULT)MX_E_MX, hr <= (HRESULT)WR_E_INVALIDSURROGATEPAIR) ||
		AllTrue(hr >= (HRESULT)XML_E_INVALID_DECIMAL, hr <= (HRESULT)XML_E_INVALIDENCODING);
}

NOALIAS static HRESULT __fastcall CheckParseError(HRESULT hr)
{
	return (IsXmlError(hr) || AnyTrue(SUCCEEDED(hr), E_FAIL == hr)) ? WINCODEC_ERR_UNKNOWNIMAGEFORMAT : hr;
}


static HRESULT CALLBACK MatchesSvgPattern(_In_ IStream* pstm, UINT64, _Out_opt_ void* pIsGzip)
{
	union {
		IStream* pstmInf;
		ISAXXMLReader* reader;
	};
	HRESULT hr = wcTryUncompressStream(pstm, TRUE, &pstmInf);
	if (pIsGzip)
		*((bool_t*)pIsGzip) = (S_OK == hr);
	if (S_OK == hr)
		pstm = pstmInf;
	else if (IsUnknownImage(hr))
		pstm->AddRef();
	else
		return hr;
	hr = CreateXmlSaxReader(&reader);
	if (S_OK == hr)
	{
		SvgContentHandler handler;
		hr = reader->putContentHandler(&handler);
		if (S_OK == hr)
		{
			VARIANT vsrc = { VT_UNKNOWN };
			vsrc.punkVal = pstm;
			hr = reader->parse(vsrc);
			if (handler.IsParsed())
				hr = (handler.IsSvg() ? S_OK : WINCODEC_ERR_UNKNOWNIMAGEFORMAT);
			else
				hr = CheckParseError(hr);
		}
		reader->Release();
	}
	pstm->Release();
	return hr;
}


_Check_return_ WCXFASTAPI wcMatchesSvgPattern(_In_ IStream* pstm, _Out_opt_ bool_t* pbGzip)
{
	if (pbGzip)
		*pbGzip = false;
	return StreamSeekBack(pstm, &MatchesSvgPattern, pbGzip);
}


_Check_return_ WCXFASTAPI wcUrlMatchesSvgPattern(_In_ PCWSTR szUrl)
{
	ISAXXMLReader* reader;
	HRESULT hr = CreateXmlSaxReader(&reader);
	if (S_OK == hr)
	{
		SvgContentHandler handler;
		hr = reader->putContentHandler(&handler);
		if (S_OK == hr)
		{
			hr = reader->parseURL(szUrl);
			if (handler.IsParsed())
				hr = (handler.IsSvg() ? S_OK : WINCODEC_ERR_UNKNOWNIMAGEFORMAT);
			else
				hr = CheckParseError(hr);
		}
		reader->Release();
	}
	return hr;
}


_Check_return_ WCXSTDAPI wcUpdateSvgSize(_In_ ID2D1SvgDocument* svgDoc, bool_t removeSize, _Out_ D2D_SIZE_F* pSize)
{
	HRESULT hr = E_INVALIDARG;
	if (pSize)
	{
		if (svgDoc)
		{
			bool noViewBox = false;
			ID2D1SvgElement* svgRoot;
			D2D1_SVG_VIEWBOX viewBox;
			D2D1_SVG_LENGTH width, height;

			svgDoc->GetRoot(&svgRoot);

			if (S_OK != svgRoot->GetAttributeValue(L"viewBox", D2D1_SVG_ATTRIBUTE_POD_TYPE_VIEWBOX, &viewBox, sizeof(viewBox)))
			{
				noViewBox = true;
				ZeroStruct(&viewBox);
			}
			if (S_OK != svgRoot->GetAttributeValue(L"width", &width))
				width.value = viewBox.width;
			else if (D2D1_SVG_LENGTH_UNITS_PERCENTAGE == width.units)
				width.value *= viewBox.width / 100;

			if (S_OK != svgRoot->GetAttributeValue(L"height", &height))
				height.value = viewBox.height;
			else if (D2D1_SVG_LENGTH_UNITS_PERCENTAGE == height.units)
				height.value *= viewBox.height / 100;

			if (removeSize)
			{
				if (noViewBox)
				{
					D2D1_SVG_LENGTH x, y;
					ASSUME(0 == viewBox.x && 0 == viewBox.y);
					if (S_OK == svgRoot->GetAttributeValue(L"x", &x))
					{
						viewBox.x = x.value;
						if (D2D1_SVG_LENGTH_UNITS_PERCENTAGE == x.units)
							viewBox.x /= 100;
					}
					if (S_OK == svgRoot->GetAttributeValue(L"y", &y))
					{
						viewBox.y = y.value;
						if (D2D1_SVG_LENGTH_UNITS_PERCENTAGE == y.units)
							viewBox.y /= 100;
					}
					viewBox.width = width.value;
					viewBox.height = height.value;
					svgRoot->SetAttributeValue(L"viewBox", D2D1_SVG_ATTRIBUTE_POD_TYPE_VIEWBOX, &viewBox, sizeof(viewBox));
				}
				svgRoot->RemoveAttribute(L"x");
				svgRoot->RemoveAttribute(L"y");
				svgRoot->RemoveAttribute(L"width");
				svgRoot->RemoveAttribute(L"height");
			}
			svgRoot->Release();

			if (AllTrue(width.value > 0, height.value > 0))
			{
				pSize->width = width.value;
				pSize->height = height.value;
				hr = svgDoc->SetViewportSize(*pSize);
				if (SUCCEEDED(hr))
					return S_OK;
			}

			hr = WINCODEC_ERR_IMAGESIZEOUTOFRANGE;
		}
		Zero8Bytes(pSize);
	}
	return hr;
}


}	// namespace