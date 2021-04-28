/*	CmnTpl.inl	- common convenience templates

	Copyright (c) Alexandre Egorov

	This software is provided 'as-is', without any warranty.
	This software can be used and modified freely for any purpose,
	including commercial applications,
	as long as its origin is not misrepresented.
 */
#pragma once
#define _CMN_TEMPLATES_INCLUDED_
#ifdef __cplusplus


///////////////////////////////////////////////////////////////////////
// Function templates /////////////////////////////////////////////////


template <class _B>
_Check_return_ _Post_satisfies_(return == false || bval != 0) FORCEINLINE bool ToBool(const _B bval)
{
	return ((_B)0 != bval);
}
_Check_return_ _Post_satisfies_(return == false || bval != false)
FORCEINLINE bool ToBool(bool bval)
{
	return bval;
}

template<class T1, class T2>
_Check_return_ _Post_satisfies_(return == false || (a != 0 && b != 0)) bool AllTrue(const T1 a, const T2 b)
{
	return (ToBool(a) & ToBool(b));
}

template<class T1, class T2, class T3>
_Check_return_ _Post_satisfies_(return == false || (a != 0 && b != 0 && c != 0))
bool AllTrue(const T1 a, const T2 b, const T3 c)
{
	return (ToBool(a) & ToBool(b) & ToBool(c));
}

template<class T1, class T2, class T3, class T4>
_Check_return_ _Post_satisfies_(return == false || (a != 0 && b != 0 && c != 0 && d != 0))
bool AllTrue(const T1 a, const T2 b, T3 c, const T4 d)
{
	return (ToBool(a) & ToBool(b) & ToBool(c) & ToBool(d));
}

template<class T1, class T2, class T3, class T4, class T5>
_Check_return_ _Post_satisfies_(return == false || (a != 0 && b != 0 && c != 0 && d != 0 && e != 0))
bool AllTrue(const T1 a, const T2 b, const T3 c,const  T4 d, const T5 e)
{
	return (ToBool(a) & ToBool(b) & ToBool(c) & ToBool(d) & ToBool(e));
}

template<class T1, class T2>
_Check_return_ _Check_return_ _Post_satisfies_(return == false || a != 0 || b != 0)
bool AnyTrue(const T1 a, const T2 b)
{
	return (ToBool(a) | ToBool(b));
}

template<class T1, class T2, class T3>
_Check_return_ _Check_return_ _Post_satisfies_(return == false || a != 0 || b != 0 || c != 0)
bool AnyTrue(const T1 a, const T2 b, const T3 c)
{
	return (ToBool(a) | ToBool(b) | ToBool(c));
}

template<class T1, class T2, class T3, class T4>
_Check_return_ _Post_satisfies_(return == false || a != 0 || b != 0 || c != 0 || d != 0)
bool AnyTrue(const T1 a, const T2 b, const T3 c, const T4 d)
{
	return (ToBool(a) | ToBool(b) | ToBool(c) | ToBool(d));
}

template<class T1, class T2, class T3, class T4, class T5>
_Check_return_ _Post_satisfies_(return == false || a != 0 || b != 0 || c != 0 || d != 0 || e != 0)
bool AnyTrue(const T1 a, const T2 b, const T3 c, const T4 d, const T5 e)
{
	return (ToBool(a) | ToBool(b) | ToBool(c) | ToBool(d) | ToBool(e));
}


template <typename _N> _N SetBit(_N var, _N mask, bool set)
{
	return (var & ~mask) | ((_N)(-(int)(signed char)set) & mask);
}


template <class _T> void Swap(_T& a, _T& b)
{
	const _T t = a;
	a = b;
	b = t;
}
template <class _T> void NumSwap(_T& a, _T& b)
{
	NUMSWAP(a, b);
}


template <class _T> unsigned __int32 ToUInt32(const _T& x)
{
	STATIC_ASSERT(sizeof(_T) >= sizeof(unsigned __int32));
	return *reinterpret_cast<const unsigned __int32*>(&x);
}
template <class _T> unsigned __int64 ToUInt64(const _T& x)
{
	STATIC_ASSERT(sizeof(_T) >= sizeof(unsigned __int64));
	return *reinterpret_cast<const unsigned __int64*>(&x);
}


template<class T> _Check_return_
int IndexOfX(_In_reads_(cItems) const T* rgItems, int cItems, T item, int start = 0)
{
	if (start >= 0)
	{
		rgItems += start;
		for (; start < cItems; start++)
		{
			if (*rgItems != item)
			{
				rgItems++;
				continue;
			}
			return start;
		}
	}
	return -1;
}


template <class T>
void ShellSort(_Inout_updates_all_(count) T* items, size_t count)
{
    size_t h, i, j;
    for (h = count; h /= 2;)
	{
        for (i = h; i < count; i++)
		{
            const T tmp = items[i];
            for (j = i; j >= h && tmp < items[j - h]; j -= h)
                items[j] = items[j - h];
            items[j] = tmp;
        }
    }
}

template <class T>
void ShellSort(_Inout_updates_all_(count) T* items, size_t count,
	int (__cdecl* compare)(T a, T b, intptr_t param), intptr_t param)
{
    size_t h, i, j;
    for (h = count; h /= 2;)
	{
        for (i = h; i < count; i++)
		{
            const T tmp = items[i];
            for (j = i; j >= h && compare(tmp, items[j - h], param) < 0; j -= h)
                items[j] = items[j - h];
            items[j] = tmp;
        }
    }
}

template <class T>
void ShellSort(_Inout_updates_all_(count) T* items, size_t count, int (__fastcall* compare)(T a, T b))
{
    size_t h, i, j;
    for (h = count; h /= 2;)
	{
        for (i = h; i < count; i++)
		{
            const T tmp = items[i];
            for (j = i; j >= h && compare(tmp, items[j - h]) < 0; j -= h)
                items[j] = items[j - h];
            items[j] = tmp;
        }
    }
}

template <class T>
void ShellSort(_Inout_updates_all_(count) T* items, size_t count,
	int (__cdecl* compare)(const T& a, const T& b, intptr_t param), intptr_t param)
{
    size_t h, i, j;
    for (h = count; h /= 2;)
	{
        for (i = h; i < count; i++)
		{
            const T tmp = items[i];
            for (j = i; j >= h && compare(tmp, items[j - h], param) < 0; j -= h)
                items[j] = items[j - h];
            items[j] = tmp;
        }
    }
}

template <class T>
void ShellSort(_Inout_updates_all_(count) T* items, size_t count, int (__fastcall* compare)(const T& a, const T& b))
{
    size_t h, i, j;
    for (h = count; h /= 2;)
	{
        for (i = h; i < count; i++)
		{
            const T tmp = items[i];
            for (j = i; j >= h && compare(tmp, items[j - h]) < 0; j -= h)
                items[j] = items[j - h];
            items[j] = tmp;
        }
    }
}


// Interlocked ////////////////////////////////////////////////////////


template<typename T> T InterlockedExchangeX(volatile T* targ, T val)
{
	switch (sizeof(T))
	{
	case 1:
		return (T)_InterlockedExchange8((volatile char*)targ, (char)val);
	case 2:
		return (T)_InterlockedExchange16((volatile short*)targ, (short)val);
	case 4:
		return (T)_InterlockedExchange((volatile long*)targ, (long)val);
	case 8:
		return (T)_InterlockedExchange64((volatile long long*)targ, (long)val);
	default:
#ifdef _ASSERTE
		_ASSERTE(!"Invalid parameter size");
#endif
		{
			const T _old = *targ;
			*targ = val;
			return _old;
		}
	}
}

template<typename T> T InterlockedCompareExchangeX(volatile T* dest, T exchange, T comparand)
{
	switch (sizeof(T))
	{
	case 1:
		return (T)_InterlockedCompareExchange8((volatile char*)dest, (char)exchange, (char)comparand);
	case 2:
		return (T)_InterlockedCompareExchange16((volatile short*)dest, (short)exchange, (short)comparand);
	case 4:
		return (T)_InterlockedCompareExchange((volatile long*)dest, (long)exchange, (long)comparand);
	case 8:
		return (T)_InterlockedCompareExchange64((volatile long long*) dest, (long long)exchange, (long)comparand);
	default:
#ifdef _ASSERT
		_ASSERT(!"Invalid parameter size");
#endif
		{
			const T _old = *dest;
			if (_old == comparand)
				*dest = exchange;
			return _old;
		}
	}
}


#pragma push_macro("INTERLOCKED_IMPLT_CALL_")
#define INTERLOCKED_IMPLT_CALL_(func, type)	static_cast<TVal>(Interlocked##func(	\
		reinterpret_cast<volatile type*>(address), static_cast<type>(value)))
#pragma push_macro("INTERLOCKED_IMPLT_CALL2_")
#define INTERLOCKED_IMPLT_CALL2_(func, type)	static_cast<TVal>(Interlocked##func(	\
		reinterpret_cast<volatile type*>(address), static_cast<type>(value), static_cast<type>(comp)))

#pragma push_macro("INTERLOCKED_IMPLT_SWITCH_32_")
#define INTERLOCKED_IMPLT_SWITCH_32_(func)	switch (sizeof(TVal)) {	\
	case sizeof(LONG): return INTERLOCKED_IMPLT_CALL_(func, LONG);	\
	case sizeof(LONG64): return INTERLOCKED_IMPLT_CALL_(func##64, LONG64); }	\
	ASSERT(!"Invalid argument size in "__FUNCTION__); return static_cast<TVal>(0)
#pragma push_macro("INTERLOCKED_IMPLT_SWITCH_16_")
#define INTERLOCKED_IMPLT_SWITCH_16_(func)	switch (sizeof(TVal)) {	\
	case sizeof(SHORT): return INTERLOCKED_IMPLT_CALL_(func##16, SHORT);	\
	case sizeof(LONG): return INTERLOCKED_IMPLT_CALL_(func, LONG);	\
	case sizeof(LONG64): return INTERLOCKED_IMPLT_CALL_(func##64, LONG64); }	\
	ASSERT(!"Invalid argument size in "__FUNCTION__); return static_cast<TVal>(0)
#pragma push_macro("INTERLOCKED_IMPLT_SWITCH_8_")
#define INTERLOCKED_IMPLT_SWITCH_8_(func)	switch (sizeof(TVal)) {	\
	case sizeof(CHAR): return INTERLOCKED_IMPLT_CALL_(func##8, CHAR);	\
	case sizeof(SHORT): return INTERLOCKED_IMPLT_CALL_(func##16, SHORT);	\
	case sizeof(LONG): return INTERLOCKED_IMPLT_CALL_(func, LONG);	\
	case sizeof(LONG64): return INTERLOCKED_IMPLT_CALL_(func##64, LONG64); }	\
	ASSERT(!"Invalid argument size in "__FUNCTION__); return static_cast<TVal>(0)


template <class TVal> TVal AtomicAdd32(volatile TVal* address, TVal value)
{
	STATIC_ASSERT(sizeof(value) == sizeof(LONG));
	return INTERLOCKED_IMPLT_CALL_(ExchangeAdd, LONG);
}
template <class TVal> TVal AtomicAdd64(volatile TVal* address, TVal value)
{
	STATIC_ASSERT(sizeof(value) == sizeof(LONG64));
	return INTERLOCKED_IMPLT_CALL_(ExchangeAdd64, LONG64);
}
template <class TVal> TVal AtomicAdd(volatile TVal* address, TVal value)
{
	INTERLOCKED_IMPLT_SWITCH_32_(ExchangeAdd);
}

template <class TVal> TVal AtomicAnd8(volatile TVal* address, TVal value)
{
	STATIC_ASSERT(sizeof(value) == sizeof(CHAR));
	return INTERLOCKED_IMPLT_CALL_(And8, CHAR);
}
template <class TVal> TVal AtomicAnd16(volatile TVal* address, TVal value)
{
	STATIC_ASSERT(sizeof(value) == sizeof(SHORT));
	return INTERLOCKED_IMPLT_CALL_(And16, SHORT);
}
template <class TVal> TVal AtomicAnd32(volatile TVal* address, TVal value)
{
	STATIC_ASSERT(sizeof(value) == sizeof(LONG));
	return INTERLOCKED_IMPLT_CALL_(And, LONG);
}
template <class TVal> TVal AtomicAnd64(volatile TVal* address, TVal value)
{
	STATIC_ASSERT(sizeof(value) == sizeof(LONG64));
	return INTERLOCKED_IMPLT_CALL_(And64, LONG64);
}
template <class TVal> TVal AtomicAnd(volatile TVal* address, TVal value)
{
	INTERLOCKED_IMPLT_SWITCH_8_(And);
}

template <class TVal> TVal AtomicOr8(volatile TVal* address, TVal value)
{
	STATIC_ASSERT(sizeof(value) == sizeof(CHAR));
	return INTERLOCKED_IMPLT_CALL_(Or8, CHAR);
}
template <class TVal> TVal AtomicOr16(volatile TVal* address, TVal value)
{
	STATIC_ASSERT(sizeof(value) == sizeof(SHORT));
	return INTERLOCKED_IMPLT_CALL_(Or16, SHORT);
}
template <class TVal> TVal AtomicOr32(volatile TVal* address, TVal value)
{
	STATIC_ASSERT(sizeof(value) == sizeof(LONG));
	return INTERLOCKED_IMPLT_CALL_(Or, LONG);
}
template <class TVal> TVal AtomicOr64(volatile TVal* address, TVal value)
{
	STATIC_ASSERT(sizeof(value) == sizeof(LONG64));
	return INTERLOCKED_IMPLT_CALL_(Or64, LONG64);
}
template <class TVal> TVal AtomicOr(volatile TVal* address, TVal value)
{
	INTERLOCKED_IMPLT_SWITCH_8_(Or);
}

template <class TVal> TVal AtomicXor8(volatile TVal* address, TVal value)
{
	STATIC_ASSERT(sizeof(value) == sizeof(CHAR));
	return INTERLOCKED_IMPLT_CALL_(Xor8, CHAR);
}
template <class TVal> TVal AtomicXor16(volatile TVal* address, TVal value)
{
	STATIC_ASSERT(sizeof(value) == sizeof(SHORT));
	return INTERLOCKED_IMPLT_CALL_(Xor16, SHORT);
}
template <class TVal> TVal AtomicXor32(volatile TVal* address, TVal value)
{
	STATIC_ASSERT(sizeof(value) == sizeof(LONG));
	return INTERLOCKED_IMPLT_CALL_(Xor, LONG);
}
template <class TVal> TVal AtomicXor64(volatile TVal* address, TVal value)
{
	STATIC_ASSERT(sizeof(value) == sizeof(LONG64));
	return INTERLOCKED_IMPLT_CALL_(Xor64, LONG64);
}
template <class TVal> TVal AtomicXor(volatile TVal* address, TVal value)
{
	INTERLOCKED_IMPLT_SWITCH_8_(Xor);
}

template <class TVal> TVal AtomicSet8(volatile TVal* address, TVal value)
{
	STATIC_ASSERT(sizeof(value) == sizeof(CHAR));
	return INTERLOCKED_IMPLT_CALL_(Exchange8, CHAR);
}
template <class TVal> TVal AtomicSet16(volatile TVal* address, TVal value)
{
	STATIC_ASSERT(sizeof(value) == sizeof(SHORT));
	return INTERLOCKED_IMPLT_CALL_(Exchange16, SHORT);
}
template <class TVal> TVal AtomicSet32(volatile TVal* address, TVal value)
{
	STATIC_ASSERT(sizeof(value) == sizeof(LONG));
	return INTERLOCKED_IMPLT_CALL_(Exchange, LONG);
}
template <class TVal> TVal AtomicSet64(volatile TVal* address, TVal value)
{
	STATIC_ASSERT(sizeof(value) == sizeof(LONG64));
	return INTERLOCKED_IMPLT_CALL_(Exchange64, LONG64);
}
template <class TVal> TVal AtomicSet(volatile TVal* address, TVal value)
{
	INTERLOCKED_IMPLT_SWITCH_8_(Exchange);
}

template <class TVal> TVal AtomicCompareSet8(volatile TVal* address, TVal value, TVal comp)
{
	STATIC_ASSERT(sizeof(value) == sizeof(CHAR));
	return INTERLOCKED_IMPLT_CALL2_(CompareExchange8, CHAR);
}
template <class TVal> TVal AtomicCompareSet16(volatile TVal* address, TVal value, TVal comp)
{
	STATIC_ASSERT(sizeof(value) == sizeof(SHORT));
	return INTERLOCKED_IMPLT_CALL2_(CompareExchange16, SHORT);
}
template <class TVal> TVal AtomicCompareSet32(volatile TVal* address, TVal value, TVal comp)
{
	STATIC_ASSERT(sizeof(value) == sizeof(LONG));
	return INTERLOCKED_IMPLT_CALL2_(CompareExchange, LONG);
}
template <class TVal> TVal AtomicCompareSet64(volatile TVal* address, TVal value, TVal comp)
{
	STATIC_ASSERT(sizeof(value) == sizeof(LONG64));
	return INTERLOCKED_IMPLT_CALL2_(CompareExchange64, LONG64);
}
template <class TVal> TVal AtomicCompareSet(volatile TVal* address, TVal value, TVal comp)
{
	switch (sizeof(TVal))
	{
	case sizeof(CHAR):
		return INTERLOCKED_IMPLT_CALL2_(CompareExchange8, CHAR);
	case sizeof(SHORT):
		return INTERLOCKED_IMPLT_CALL2_(CompareExchange16, SHORT);
	case sizeof(LONG):
		return INTERLOCKED_IMPLT_CALL2_(CompareExchange, LONG);
	case sizeof(LONG64):
		return INTERLOCKED_IMPLT_CALL2_(CompareExchange64, LONG64);
	}
	ASSERT(!"Invalid argument size in "__FUNCTION__);
	return static_cast<TVal>(0);
}

#pragma pop_macro("INTERLOCKED_IMPLT_SWITCH_8_")
#pragma pop_macro("INTERLOCKED_IMPLT_SWITCH_16_")
#pragma pop_macro("INTERLOCKED_IMPLT_SWITCH_32_")
#pragma pop_macro("INTERLOCKED_IMPLT_CALL2_")
#pragma pop_macro("INTERLOCKED_IMPLT_CALL_")


///////////////////////////////////////////////////////////////////////
// Classes ////////////////////////////////////////////////////////////


#define ZEROINIT_NEW_DELETE	\
	static void* operator new(size_t cb) { return calloc(cb, 1); }	\
	static void* operator new[](size_t cb) { return calloc(cb, 1); }\
	static void operator delete(void* p) { free(p); }	\
	static void operator delete[](void* p) { free(p); }

#define IMPL_UNCOPYABLE(cname)	\
	cname(const cname&);	\
	cname(const cname&&);	\
	cname& operator=(const cname&);	\
	cname& operator=(const cname&&);	\
	const cname& operator=(const cname&) const;	\
	const cname& operator=(const cname&&) const;


///////////////////////////////////////////////////////////////////////
// Generic templates //////////////////////////////////////////////////


typedef class Uncopyable_ abstract
{
protected:
	Uncopyable_() {}
private:
	IMPL_UNCOPYABLE(Uncopyable_)
} noncopyable_;


///////////////////////////////////////////////////////////////////////
// Mixed memory buffer ////////////////////////////////////////////////


constexpr UINT HEAP_MEMORY_THRESHOLD = (1024 * 1024 * 4 - 1);	// 0x003FFFFF

template <typename TElem>
class TMMemBuffer : public Uncopyable_
{
public:
	TMMemBuffer() : _Buffer(nullptr), _Size(0) {}
	TMMemBuffer(size_t cb) { Alloc_(cb); }
	~TMMemBuffer() { Free_(); }

	operator TElem* () const { return _Buffer; }
	operator void* () const { return _Buffer; }
	operator bool() const { return (nullptr != _Buffer); }
	bool operator !() const { return !_Buffer; }

	TElem* Get() const { return _Buffer; }
	unsigned char* GetBytes() const { return static_cast<unsigned char*>(_Buffer); }
	size_t GetSize() const { return (_Size & ~SIGN_BIT); }
	unsigned GetSize32() const { return (unsigned)(_Size & ~SIGN_BIT); }
	bool IsVirtualAlloc() const { return (_Size < 0); }
	bool IsHeapAlloc() const { return (_Size > 0); }

	HRESULT EnsureAlloc(size_t cb) {
		return (GetSize() < cb) ? Alloc(cb) : S_FALSE;
	}
	HRESULT Alloc(size_t cb) {
		Free_();
		return Alloc_(cb);
	}

	NOINLINE void Free() {
		Free_();
		_Buffer = nullptr;
		_Size = 0;
	}

protected:
	static constexpr intptr_t SIGN_BIT = (intptr_t)((intptr_t)1 << (sizeof(intptr_t) * 8 - 1));

	TElem* _Buffer;
	intptr_t _Size;

	NOINLINE HRESULT Alloc_(size_t cb) {
		if (cb >= HEAP_MEMORY_THRESHOLD)
		{
			cb = ALIGN_SIZE(cb, 4096);
			if ((intptr_t)cb < 0)
				return MEM_E_INVALID_SIZE;
			_Buffer = static_cast<TElem*>(::VirtualAlloc(nullptr, cb, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE));
			cb |= (size_t)SIGN_BIT;
		}
		else
		{
			cb = ALIGN_SIZE(cb, 16);
			_Buffer = static_cast<TElem*>(malloc(cb));
		}
		if (_Buffer)
		{
			_Size = (intptr_t)cb;
			return S_OK;
		}
		_Size = 0;
		return E_OUTOFMEMORY;
	}
	NOINLINE void Free_() {
		if (!(_Size & SIGN_BIT))
			::VirtualFree(_Buffer, 0, MEM_RELEASE);
		else
			free(_Buffer);
	}
};


///////////////////////////////////////////////////////////////////////
// Dynamic array //////////////////////////////////////////////////////

template <typename _T, UINT _TH>
class TDynArray : public Uncopyable_
{
protected:
	_T* _Array = nullptr;
	UINT _Length = 0;
	UINT _Capacity = 0;

	void _ElemCopy(_T* dst, const _T* src, UINT count)
	{
		if (!(sizeof(_T) & 7))
			_movsq(dst, src, (count * sizeof(_T)) / 8);
		else if (!(sizeof(_T) & 3))
			_movsd(dst, src, (count * sizeof(_T)) / 4);
		else if (!(sizeof(_T) & 1))
			_movsw(dst, src, (count * sizeof(_T)) / 2);
		else
			_movsb(dst, src, count * sizeof(_T));
	}

public:
	_T& operator [](UINT index) { return _Array[index]; }
	_T operator [](UINT index) const { return _Array[index]; }
	bool operator !() const { return !_Length; }
	operator bool() const { return (0 != _Length); }
	operator const _T* () const { return _Array; }
	operator _T* () { return _Array; }

	_T* Data() const { return _Array; }
	UINT Length() const { return _Length; }
	UINT Capacity() const { return _Capacity; }

	NOINLINE void Free()
	{
		_Length = 0;
		_Capacity = 0;
		if (_Array)
		{
			free(_Array);
			_Array = nullptr;
		}
	}

	NOINLINE void Clear()
	{
		if (_Capacity > _TH)
		{
			_T* const tmp = (_T*)realloc(_Array, _TH * sizeof(_T));
			if (tmp)
			{
				_Array = tmp;
				_Capacity = _TH;
			}
		}
		_Length = 0;
		_Capacity = 0;
	}

	HRESULT Push(const _T& elem, bool unique = true)
	{
		if (unique)
			Remove(elem);
		return Append(elem);
	}
	NOINLINE HRESULT Append(const _T& elem, UINT maxCount, bool unique = true)
	{
		if (unique)
			Remove(elem);
		if (maxCount)
		{
			while (_Length >= maxCount)
				RemoveAt(0);
		}
		return Append(elem);
	}
	NOINLINE HRESULT Append(const _T& elem)
	{
		const HRESULT hr = EnsureCapacity(_Length + 1);
		if (SUCCEEDED(hr))
		{
			_Array[_Length] = elem;
			_Length++;
		}
		return hr;
	}

	NOINLINE HRESULT EnsureCapacity(UINT elemCount)
	{
		if (_Capacity >= elemCount)
			return S_FALSE;

		elemCount = (elemCount + _TH - 1) & ~(_TH - 1);
		if (_Array)
		{
			_T* const tmp = (_T*)realloc(_Array, elemCount * sizeof(_T));
			if (tmp)
			{
				_Array = tmp;
				_Capacity = elemCount;
				return S_OK;
			}
		}
		else if (_Array = (_T*)malloc(elemCount * sizeof(_T)))
		{
			_Capacity = elemCount;
			return S_OK;
		}
		return E_OUTOFMEMORY;
	}

	NOINLINE int IndexOf(const _T& elem, UINT start = 0) const
	{
		return ::IndexOfX<_T>(_Array, (int)_Length, elem, (int)start);
	}
	NOINLINE int LastIndexOf(const _T& elem, int start = -1) const
	{
		if (start >= (int)_Length)
			start = _Length - 1;
		for (; start >= 0; start--)
		{
			if (_Array[start] != elem)
				continue;
			return start;
		}
		return -1;
	}

	bool IsLast(const _T& elem) const
	{
		return (_Length && elem == _Array[_Length - 1]);
	}

	NOINLINE UINT Remove(const _T& elem)
	{
		UINT count = 0;
		int ind = 0;
		while ((ind = IndexOf(elem, (UINT)ind)) >= 0)
		{
			RemoveAt(ind);
			count++;
		}
		return count;
	}

	UINT Pop()
	{
		return RemoveAt(-1);
	}

	NOINLINE UINT RemoveAt(int index = -1)
	{
		if (index < 0 || (UINT)index >= _Length)
			index = (int)_Length - 1;
		if (index >= 0)
		{
			_Length--;
			_ElemCopy(_Array + (UINT)index, _Array + (UINT)index + 1, _Length - (UINT)index);
		}
		return _Length;
	}
};


///////////////////////////////////////////////////////////////////////
// Thread pool works //////////////////////////////////////////////////

template <UINT _WC> class TTPWorks
{
protected:
	PTP_WORK m_rgTpWorks[_WC];

private:
	NOINLINE int _IndexOf(PTP_WORK work, UINT count) const {
		return ::IndexOfX<PTP_WORK>(m_rgTpWorks, _countof(m_rgTpWorks), work, 0);
	}

public:
	operator const PTP_WORK*() const { return m_rgTpWorks; }
	PTP_WORK operator [](UINT index) const
	{
		return (index < _countof(m_rgTpWorks)) ? m_rgTpWorks[index] : nullptr;
	}
	int IndexOf(PTP_WORK work) const { return _IndexOf(work, _countof(m_rgTpWorks)); }

	_Success_(return >= 0 || (return < 0 && allowSync))	//  returns the new work's index
		LONG AddWork(_In_ PTP_WORK_CALLBACK pfnWC, _In_opt_ PVOID param, bool allowSync = true, bool useCS = true)
	{
		return StartThreadWork(pfnWC, param, m_rgTpWorks, _countof(m_rgTpWorks), allowSync, useCS);
	}
	void Wait(PTP_WORK work) { Close(work, FALSE); }
	void Wait(int index) { Close(index, FALSE); }
	void WaitAll() { CloseAll(FALSE); }
	void Close(PTP_WORK work, int cancel = TRUE, bool useCS = true)
	{
		CloseThreadWork(m_rgTpWorks, _countof(m_rgTpWorks), work, cancel, useCS);
	}
	void Close(int index, int cancel = TRUE, bool useCS = true)
	{
		CloseThreadWorkAt(m_rgTpWorks, _countof(m_rgTpWorks), index, cancel, useCS);
	}
	void CloseAll(BOOL cancel = TRUE)
	{
		::CloseThreadpoolWorks(m_rgTpWorks, _countof(m_rgTpWorks), cancel);
	}

	~TTPWorks() { CloseAll(TRUE); }
};


///////////////////////////////////////////////////////////////////////
// COM wrapper ////////////////////////////////////////////////////////

template <typename TImpl, REFIID TIID, typename _TStat = volatile HRESULT>
class NOVTABLE TUnknownBase : public TImpl
{
protected:
	volatile ULONG32 m_cRef = 0;
	_TStat m_Status = 0;

public:
	NOINLINE
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void** ppvObject) override
	{
		ASSERT(16 == sizeof(GUID));
		if (InlineIsEqualGUID(riid, TIID) || InlineIsEqualGUID(riid, IID_IUnknown))
		{
			InterlockedIncrement(&m_cRef);
			*ppvObject = this;
			return S_OK;
		}
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef() override
	{
		return (ULONG)InterlockedIncrement(&m_cRef);
	}

	NOINLINE
	virtual ULONG STDMETHODCALLTYPE Release() override
	{
		ULONG cref = InterlockedDecrement(&m_cRef);
		ASSERT((LONG)cref >= 0);
		if ((LONG)cref <= 0)
			delete this;
		return (ULONG)((cref >= 0) ? cref : 0);
	}
};


#endif	// __cplusplus

