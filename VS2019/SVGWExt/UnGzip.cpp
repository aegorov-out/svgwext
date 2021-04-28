// UnGzip.cpp

#include "pch.h"
#include "Main.h"
#include "zlib/zutil.h"

namespace RootNamespace {

///////////////////////////////////////////////////////////////////////
// Definitions & locals ///////////////////////////////////////////////


NOALIAS NOINLINE static HRESULT __fastcall ZRetToHRESULT(int zret)
{
	static const struct {
		int zcode;
		HRESULT hr;
	} s_rgCodes[] = {
		{ Z_ERRNO,			WINCODEC_ERR_WIN32ERROR },
		{ Z_DATA_ERROR,		WINCODEC_ERR_UNKNOWNIMAGEFORMAT },
		{ Z_BUF_ERROR,		E_FAIL },
		{ Z_MEM_ERROR,		E_OUTOFMEMORY },
		{ Z_NEED_DICT,		WINCODEC_ERR_BADSTREAMDATA },
		{ Z_STREAM_END,		ERROR_NO_MORE_ITEMS },
		{ Z_OK,				S_OK }
	};
	for (UINT i = 0; i < ARRAYSIZE(s_rgCodes); i++)
	{
		if (s_rgCodes[i].zcode == zret)
			return s_rgCodes[i].hr;
	}
	return WINCODEC_ERR_INTERNALERROR;
}

inline HRESULT __fastcall CheckZRet(int zret)
{
	return (Z_OK == zret) ? S_OK : ZRetToHRESULT(zret);
}


NOALIAS static HRESULT CALLBACK VerifyGzipFormat(_In_ IStream* pstm, UINT64, _Out_opt_ void* pcbOutSize)
{
	union {
		UGZIPHEADER hdr;
		UINT32 cbOut;
	};
	ULONG cb = 0;
	HRESULT hr = pstm->Read(&hdr.Part32, 4, &cb);
	if (SUCCEEDED(hr))
	{
		hr = WINCODEC_ERR_UNKNOWNIMAGEFORMAT;
		if (AllTrue(4 == cb, 0x088B1F == (hdr.Part32 & 0xFFFFFF)))
		{
			hr = S_OK;
			if (pcbOutSize)
			{
				*(PUINT32)pcbOutSize = 0;
				if ((hdr.Flags & 0x08) && SUCCEEDED(pstm->Seek(ToLargeInteger(-4), STREAM_SEEK_END, nullptr))
					&& SUCCEEDED(pstm->Read(&cbOut, 4, &cb)))
				{
					hr = WINCODEC_ERR_STREAMREAD;
					if (4 == cb)
					{
						hr = STG_E_DOCFILETOOLARGE;
						if (cbOut <= MAX_PICTURE_FILE_SIZE)
						{
							*(PUINT32)pcbOutSize = cbOut;
							hr = S_OK;
						}
					}
				}
			}
		}
	}
	return hr;
}


///////////////////////////////////////////////////////////////////////
// Basic decompressor /////////////////////////////////////////////////


class GZipInflate : public IStream
{
	ULONG m_cRef;
	const UINT32 m_cbIn;
	IStream* const m_pstmIn;

	union InfStatus {
		UINT64 quadPart;
		struct {
			UINT32 cbOut;
			HRESULT hrStat;
		};
	} volatile m_Status;

	volatile bool m_isAwaitRead;
	UINT32 m_cbOutAlloc;
	union {
		PBYTE m_pbOutData;		// valid if cbAlloc is non-zero
		IStream* m_pstmOutData;	// valid if cbAlloc is zero
	};
	union {
		UINT32 m_cbOutPos;
		IStream* m_pstmOutRead;
	};
	volatile PTP_WORK m_pwkInflate;
	BYTE m_bzIn[1024 * 8 - 32];
	BYTE m_bzOut[1024 * 12];


	static HRESULT CreateCallback(_In_ IStream* pstmIn, UINT64 cbCurPos, _Inout_ void* pSizeParam);

	GZipInflate(_In_ IStream* pstmIn, _In_opt_ UINT cbIn);
	HRESULT Init(_In_opt_ UINT cbOut, UINT8 inFlags);

	void __fastcall CloseInfWork(HRESULT hrStat);
	static void NTAPI InfWorkProc(_Inout_ PTP_CALLBACK_INSTANCE Instance, _Inout_ PVOID Context, _Inout_ PTP_WORK Work);
	HRESULT Inflate(_In_opt_ PTP_WORK pwkAsync);
	HRESULT WriteOut(UINT cbWrite, UINT cbTotal);

	UINT __fastcall AddOutputSize(UINT cb);
	HRESULT __fastcall SetStatus(HRESULT hr);
	HRESULT SetZStatus(INT32 zstat) { return SetStatus(CheckZRet(zstat)); }

	HRESULT __fastcall EnsureReadSize(_Inout_ PULONG pcbRead);
	HRESULT EnsureReadSize(SIZE_T cbPos, _Inout_ PULONG pcbRead);

	IMPL_UNCOPYABLE(GZipInflate)

public:
	static constexpr HRESULT S_COMPLETE = ERROR_HANDLE_EOF;
	static constexpr HRESULT S_OUTOFBUF = ERROR_OUTOFMEMORY;

	static bool IsWorkComplete(HRESULT hr) { return AnyTrue(S_COMPLETE == hr, S_OUTOFBUF == hr); }

	enum : UINT8 {
		INF_HEADONLY = 0x01, INF_SLOW = 0x02
	};

	~GZipInflate();

	_Success_(return == S_OK) static HRESULT Create(_In_ IStream* pstmIn, BOOL headOnly,
			_COM_Outptr_result_nullonfailure_ IStream** ppstm);

	DECLARE_IUNKNOWN;

	// ISequentialStream
	HRESULT STDMETHODCALLTYPE Read(_Out_writes_bytes_to_(cb, *pcbRead) void* pv,
			_In_ ULONG cb, _Out_opt_  ULONG* pcbRead) override;
	HRESULT STDMETHODCALLTYPE Write(_In_reads_bytes_(cb) const void* pv,
			_In_ ULONG cb, _Out_opt_ ULONG* pcbWritten) override;
	// IStream
	HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, _Out_opt_ ULARGE_INTEGER* plibNewPosition) override;
	HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize) override;
	HRESULT STDMETHODCALLTYPE CopyTo(_In_ IStream* pstm, ULARGE_INTEGER cb,
			_Out_opt_ ULARGE_INTEGER* pcbRead, _Out_opt_ ULARGE_INTEGER* pcbWritten) override;
	HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags) override;
	HRESULT STDMETHODCALLTYPE Revert(void) override;
	HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override;
	HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override;
	HRESULT STDMETHODCALLTYPE Stat(__RPC__out STATSTG* pstatstg, DWORD grfStatFlag) override;
	HRESULT STDMETHODCALLTYPE Clone(__RPC__deref_out_opt IStream** ppstm) override;
};


// Implementation /////////////////////////////////////////////////////


_Success_(return == S_OK) HRESULT GZipInflate::Create(_In_ IStream* pstmIn, BOOL headOnly,
		_COM_Outptr_result_nullonfailure_ IStream** ppstmOut)
{
	ULARGE_INTEGER ullSizes;
	HRESULT hr = Stream_Size(pstmIn, &ullSizes.QuadPart);
	if (SUCCEEDED(hr))
	{
		DWORD ticks = ::GetTickCount();
		hr = StreamSeekBack(pstmIn, &CreateCallback, &ullSizes);
		if (S_OK == hr)
		{
			ticks = ::GetTickCount() - ticks;
			GZipInflate* const pinf = new GZipInflate(pstmIn, ullSizes.LowPart);
			hr = E_OUTOFMEMORY;
			if (pinf)
			{
				hr = pinf->Init(ullSizes.HighPart, (headOnly ? INF_HEADONLY : 0) | ((ticks > 1) ? INF_SLOW : 0));
				if (S_OK == hr)
				{
					*ppstmOut = pinf;
					return S_OK;
				}
				delete pinf;
			}
		}
	}
	*ppstmOut = nullptr;
	return hr;
}

HRESULT GZipInflate::CreateCallback(_In_ IStream* pstmIn, UINT64 cbCurPos, _Inout_ void* const pSizeParam)
{
	ULARGE_INTEGER& Sizes = *(PULARGE_INTEGER)pSizeParam;
	if (Sizes.QuadPart)
	{
		if (Sizes.QuadPart <= cbCurPos)
			return WINCODEC_ERR_STREAMREAD;
		Sizes.QuadPart -= cbCurPos;
		if (Sizes.QuadPart > MAX_PICTURE_FILE_SIZE)
			return STG_E_DOCFILETOOLARGE;
	}
	const HRESULT hr = VerifyGzipFormat(pstmIn, cbCurPos, &Sizes.HighPart);
	if (S_OK == hr)
		return S_OK;
	Sizes.HighPart = 0;
	return hr;
}


WARNING_SUPPRESS(26495)
GZipInflate::GZipInflate(_In_ IStream* pstmIn, _In_opt_ UINT cbIn)
			: m_cRef(1), m_cbIn(cbIn), m_pstmIn(pstmIn)
{
	pstmIn->AddRef();
	ZeroStructRangeTo(this, m_Status, m_bzIn);
}

GZipInflate::~GZipInflate()
{
	CloseInfWork(ERROR_BROKEN_PIPE);	// code can be any non-zero value
	m_pstmIn->Release();
	
	if (m_cbOutAlloc)
	{
		free(m_pbOutData);
	}
	else if (m_pstmOutData)
	{
		m_pstmOutData->Release();
		if (m_pstmOutRead)
			m_pstmOutRead->Release();
	}
}

void GZipInflate::CloseInfWork(HRESULT hrStat)
{
	if (S_OK != hrStat)
		SetStatus(hrStat);
	const PTP_WORK pwk = (PTP_WORK)InterlockedExchangePointer((volatile PVOID*)&m_pwkInflate, NULL);
	if (pwk)
	{
		::WaitForThreadpoolWorkCallbacks(pwk, TRUE);
		::CloseThreadpoolWork(pwk);
	}
}


HRESULT GZipInflate::Init(_In_opt_ UINT cbOut, UINT8 inFlags)
{
	ASSERT(0 == m_cbOutAlloc && nullptr == m_pstmOutData);

	if (AllTrue(inFlags & INF_HEADONLY, AnyTrue(0 == cbOut, cbOut >= 4096)))
		cbOut = sizeof(m_bzOut);
	if (cbOut)
	{
		cbOut = ALIGN16(cbOut);
		if (cbOut > HEAP_MEMORY_THRESHOLD)
			heapmin();
		m_pbOutData = (PBYTE)malloc(cbOut);
		if (m_pbOutData)
		{
			m_cbOutAlloc = cbOut;
			goto Inf_;
		}
		heapmin();
	}

	m_cbOutAlloc = 0;
	const HGLOBAL hmem = ::GlobalAlloc(GMEM_MOVEABLE, ALIGN16(cbOut ? cbOut : sizeof(m_bzOut)));
	HRESULT hr = E_OUTOFMEMORY;
	if (hmem)
	{
		hr = ::CreateStreamOnHGlobal(hmem, TRUE, &m_pstmOutData);
		if (S_OK == hr)
		{
			hr = m_pstmOutData->Clone(&m_pstmOutRead);
			if (S_OK == hr)
				goto Inf_;
			m_pstmOutData->Release();
		}
		m_pstmOutData = nullptr;
	}
	return hr;

Inf_:
	if (AllTrue(inFlags & INF_SLOW, m_cbIn > 4095) || AllTrue(0 == m_cbIn, 0 == m_cbOutAlloc)
		|| AnyTrue(m_cbIn > sizeof(m_bzIn), cbOut > sizeof(m_bzOut)))
	{
		m_pwkInflate = ::CreateThreadpoolWork(&InfWorkProc, this, nullptr);
		if (m_pwkInflate)
		{
			::SubmitThreadpoolWork(m_pwkInflate);
			wcSwitchThread(0);
			return S_OK;
		}
	}
	hr = Inflate(NULL);
	return (IsWorkComplete(hr) ? S_OK : hr);
}


void GZipInflate::InfWorkProc(_Inout_ PTP_CALLBACK_INSTANCE Instance, _Inout_ PVOID Context, _Inout_ PTP_WORK Work)
{
	GZipInflate* const pThis = (GZipInflate*)Context;
	ASSERT(Work == pThis->m_pwkInflate);
	if (pThis->m_cbOutAlloc > 0xFFFF || AllTrue(0 == pThis->m_cbOutAlloc, pThis->m_cbIn > 0xFFFF))
		::CallbackMayRunLong(Instance);
	pThis->Inflate(Work);
}

HRESULT GZipInflate::Inflate(_In_opt_ PTP_WORK pwkAsync)
{
	ASSERT(NULL == m_pwkInflate || pwkAsync == m_pwkInflate);
	ASSERT(0 == m_Status.cbOut && (0 == m_cbOutPos || 0 == m_cbOutAlloc));
	m_Status.cbOut = 0;

	HRESULT hr = m_Status.hrStat;
	if (S_OK == hr)
	{
		z_stream zStm;
		ZeroStruct(&zStm);
		hr = CheckZRet(inflateInit2(&zStm, DEF_WBITS + 16));
		if (S_OK == hr)
		{
			DWORD cbIn;
			while (AllTrue(S_OK == hr, pwkAsync == m_pwkInflate))
			{
				cbIn = 0;
				hr = m_pstmIn->Read(m_bzIn, sizeof(m_bzIn), &cbIn);
				if (AnyTrue(FAILED(hr), 0 == cbIn))
					break;
				hr = m_Status.hrStat;
				if (S_OK != hr)
					break;
				ASSUME(cbIn && cbIn <= sizeof(m_bzIn));

				zStm.avail_in = cbIn;
				zStm.next_in = m_bzIn;
				do {
					zStm.avail_out = sizeof(m_bzOut);
					zStm.next_out = m_bzOut;
					const auto zret = ::inflate(&zStm, m_isAwaitRead ? Z_SYNC_FLUSH : Z_NO_FLUSH);
					const UINT cbOut = sizeof(m_bzOut) - zStm.avail_out;
					ASSUME(S_OK == hr);
					if (Z_OK == zret)
					{
						hr = WriteOut(cbOut, zStm.total_out);
						if (AnyTrue(S_OK == hr, S_OUTOFBUF == hr))
							hr = m_Status.hrStat;
					}
					else if (Z_STREAM_END == zret)
					{
						hr = WriteOut(cbOut, zStm.total_out);
						if (IsAllocSuccess(hr))
							hr = S_COMPLETE;
						ASSUME(S_OK != hr);
						break;
					}
					else
					{
						hr = ZRetToHRESULT(zret);
						ASSUME(S_OK != hr);
						break;
					}
				} while (AllTrue(S_OK == hr, 0 == zStm.avail_out));
			}
			if (AllTrue(S_OK == hr, m_Status.cbOut))
				hr = S_COMPLETE;
			::inflateEnd(&zStm);
		}
	}
	CloseInfWork((S_OK != hr) ? hr : S_COMPLETE);
	//TRACE("Inflate() end: 0x%X (%u)\n", hr, hr);
	return hr;
}


HRESULT GZipInflate::WriteOut(UINT cbWrite, UINT cbTotal)
{
	ASSERT(m_Status.cbOut + cbWrite == cbTotal);

	HRESULT hr = m_Status.hrStat;
	if (S_OK == hr)
	{
		if (m_cbOutAlloc)
		{
			const UINT cbOut = m_Status.cbOut;
			const UINT cbReq = cbOut + cbWrite;
			ASSUME(S_OK == hr);
			if (m_cbOutAlloc < cbReq)
			{
				cbWrite -= (cbReq - m_cbOutAlloc);
				hr = S_OUTOFMEMORY;
				if ((INT)cbWrite < 0)
					return E_UNEXPECTED;
			}
			CopyMemory(m_pbOutData + cbOut, m_bzOut, cbWrite);
		}
		else
		{
			DWORD cbDone = 0;
			hr = m_pstmOutData->Write(m_bzOut, cbWrite, &cbDone);
			if (SUCCEEDED(hr))
			{
				hr = S_OK;
				if (cbWrite != cbDone)
				{
					cbWrite = cbDone;
					if (0 == cbDone)
						return WINCODEC_ERR_STREAMWRITE;
					hr = S_OUTOFBUF;
				}

			}
		}
		AddOutputSize(cbWrite);
	}
	return hr;
}


UINT GZipInflate::AddOutputSize(UINT cb)
{
	UINT32 retval = m_Status.cbOut;
	if (cb)
	{
		retval = InterlockedAdd((volatile LONG*)&m_Status.cbOut, (LONG)cb);
		if (m_pwkInflate)
			::WakeByAddressSingle((PVOID)&m_Status);
	}
	return retval;
}

HRESULT GZipInflate::SetStatus(HRESULT hr)
{
	const HRESULT hrOld = (INT32)InterlockedCompareExchange(&m_Status.hrStat, hr, S_OK);
	if (wcx::AllTrue(hrOld != hr, m_pwkInflate))
		::WakeByAddressSingle((PVOID)&m_Status);
	return hrOld;
}


HRESULT GZipInflate::EnsureReadSize(_Inout_ PULONG pcbRead)
{
	ULONGLONG qwPos = m_cbOutPos;
	if (0 == m_cbOutAlloc)
	{
		const HRESULT hr = Stream_Position(m_pstmOutRead, &qwPos);
		if (FAILED(hr))
		{
			*pcbRead = 0;
			return hr;
		}
		if (qwPos > MAX_PICTURE_FILE_SIZE)
		{
			*pcbRead = 0;
			return S_FALSE;
		}
	}
	return EnsureReadSize((SIZE_T)qwPos, pcbRead);
}

HRESULT GZipInflate::EnsureReadSize(SIZE_T cbPos, _Inout_ PULONG pcbRead)
{
	if (*pcbRead > MAX_PICTURE_FILE_SIZE)
		*pcbRead = MAX_PICTURE_FILE_SIZE;
	const SIZE_T cbMax = cbPos + *pcbRead;
	InfStatus Stat = { m_Status.quadPart };
	if (Stat.cbOut >= cbMax)
		return S_OK;

	while (wcx::AllTrue(m_pwkInflate, S_OK == Stat.hrStat))
	{
		m_isAwaitRead = true;
		const BOOL sucs = ::WaitOnAddress(&m_Status, &Stat, sizeof(InfStatus), 127);
		m_isAwaitRead = false;
		Stat.quadPart = m_Status.quadPart;
		if (Stat.cbOut >= cbMax)
			return S_OK;
	}

	const int cbAvail = (int)(Stat.cbOut - cbPos);
	if (cbAvail > 0)
	{
		if (*pcbRead > (UINT)cbAvail)
			*pcbRead = (UINT)cbAvail;
		return S_OK;
	}
	*pcbRead = 0;
	return (SUCCEEDED(Stat.hrStat) ? S_FALSE : Stat.hrStat);
}


// IUnknown -----------------------------------------------------------


HRESULT GZipInflate::QueryInterface(REFIID riid, _COM_Outptr_ void** ppvObject)
{
	HRESULT hr = E_POINTER;
	if (ppvObject)
	{
		if (IsEqualGUID3(riid, IID_IStream, IID_ISequentialStream, IID_IUnknown))
		{
			_InterlockedIncrement(&m_cRef);
			*ppvObject = static_cast<IStream*>(this);
			return S_OK;
		}
		*ppvObject = nullptr;
		hr = E_NOINTERFACE;
	}
	return hr;
}

ULONG GZipInflate::AddRef()
{
	ADDREF_IMPL;
}

ULONG GZipInflate::Release()
{
	RELEASE_IMPL;
}


// ISequentialStream --------------------------------------------------


HRESULT GZipInflate::Read(_Out_writes_bytes_to_(cb, *pcbRead) void* pv, _In_ ULONG cb, _Out_opt_ ULONG* pcbRead)
{
	HRESULT hr = EnsureReadSize(&cb);
	if (S_OK == hr)
	{
		if (0 == m_cbOutAlloc)
			return m_pstmOutRead->Read(pv, cb, pcbRead);
		CopyMemory(pv, m_pbOutData + m_cbOutPos, cb);
		m_cbOutPos += cb;
	}
	if (pcbRead)
		*pcbRead = cb;
	return hr;
}

HRESULT GZipInflate::Write(_In_reads_bytes_(cb) const void* pv, _In_ ULONG cb, _Out_opt_ ULONG* pcbWritten)
{
	if (pcbWritten)
		*pcbWritten = 0;
	return STG_E_ACCESSDENIED;
}


// IStream ------------------------------------------------------------


HRESULT GZipInflate::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, _Out_opt_ ULARGE_INTEGER* plibNewPosition)
{

	if (0 == m_cbOutAlloc)
		return m_pstmOutRead->Seek(dlibMove, dwOrigin, plibNewPosition);
	
	HRESULT hr;
	switch (dwOrigin)
	{
	case STREAM_SEEK_SET:
		break;
	case STREAM_SEEK_CUR:
		dlibMove.QuadPart += m_cbOutPos;
		break;
	case STREAM_SEEK_END:
		dlibMove.QuadPart += m_cbOutAlloc;
		break;
	default:
		hr = E_INVALIDARG;
		goto Final_;
	}
	if ((UINT64)dlibMove.QuadPart <= MAX_PICTURE_FILE_SIZE)
	{
		m_cbOutPos = dlibMove.LowPart;
		hr = S_OK;
	}
	else if (AllTrue(dlibMove.QuadPart < 0, STREAM_SEEK_SET != dwOrigin))
		hr = __HRESULT_FROM_WIN32(ERROR_NEGATIVE_SEEK);
	else
		hr = STG_E_SEEKERROR;
Final_:
	if (plibNewPosition)
		plibNewPosition->QuadPart = m_cbOutPos;
	return hr;
}

HRESULT GZipInflate::SetSize(ULARGE_INTEGER libNewSize)
{
	return (libNewSize.QuadPart ? E_ACCESSDENIED : Revert());
}

HRESULT GZipInflate::CopyTo(_In_ IStream* pstm, ULARGE_INTEGER cb,
		_Out_opt_ ULARGE_INTEGER* pcbRead, _Out_opt_ ULARGE_INTEGER* pcbWritten)
{
	if (cb.HighPart)
		cb.QuadPart = 0x7FFFFFFFu;
	HRESULT hr = EnsureReadSize(&cb.LowPart);
	ULONG cbDone = 0;
	if (S_OK == hr)
	{
		if (0 == m_cbOutAlloc)
			return m_pstmOutRead->CopyTo(pstm, cb, pcbRead, pcbWritten);

		hr = pstm->Write(m_pbOutData + m_cbOutPos, cb.LowPart, &cbDone);
		if (SUCCEEDED(hr))
			m_cbOutPos += cbDone;
	}
	if (pcbWritten)
		pcbWritten->QuadPart = cbDone;
	if (pcbRead)
		pcbRead->QuadPart = cbDone;
	return hr;
}

HRESULT GZipInflate::Commit(DWORD grfCommitFlags)
{
	return (STGC_DEFAULT == grfCommitFlags) ? S_FALSE : E_INVALIDARG;
}

HRESULT GZipInflate::Revert() 
{
	SetStatus(S_CANCELLED);
	return m_Status.hrStat;
}

HRESULT GZipInflate::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	return E_NOTIMPL;
}

HRESULT GZipInflate::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	return E_NOTIMPL;
}

HRESULT GZipInflate::Stat(__RPC__out STATSTG* pstatstg, DWORD grfStatFlag)
{
	return E_NOTIMPL;
}

HRESULT GZipInflate::Clone(__RPC__deref_out_opt IStream** ppstm)
{
	if (ppstm)
		*ppstm = nullptr;
	return E_NOTIMPL;
}


///////////////////////////////////////////////////////////////////////
// Exports ////////////////////////////////////////////////////////////


_Check_return_ WCXFASTAPI wcMatchesGzipPattern(_In_ IStream* pstm, _Out_opt_ PUINT32 pcbOutSize)
{
	if (pcbOutSize)
		*pcbOutSize = 0;
	return StreamSeekBack(pstm, &VerifyGzipFormat, pcbOutSize);
}

_Success_(return == S_OK) WCXSTDAPI
wcTryUncompressStream(_In_ IStream* pstmIn, BOOL headersOnly, _COM_Outptr_result_nullonfailure_ IStream** ppstmOut)
{
	return GZipInflate::Create(pstmIn, headersOnly, ppstmOut);
}


}	// namespace