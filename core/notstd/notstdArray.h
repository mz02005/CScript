#pragma once
#include "config.h"
#include "notstdArray.h"

namespace notstd {

	template<class TYPE>
	inline void CopyElements(TYPE* pDest, const TYPE* pSrc, int nCount)
	{
		while (nCount--)
			*pDest++ = *pSrc++;
	}

	class ArrayInvalidArgExcept {};
#define NOTSTD_ArrayThrowInvalidArgException() do { throw new notstd::ArrayInvalidArgExcept; } while (0)

#define bl_array_max(a, b) ((a) > (b) ? (a) : (b))

	template<class TYPE>
	class Array
	{
	public:
		Array();
		~Array();

		int GetSize() const;
		int GetCount() const;
		bool IsEmpty() const;
		int GetUpperBound() const;
		void SetSize(int nNewSize, int nGrowBy = -1);

		void FreeExtra();
		void RemoveAll();

		const TYPE& GetAt(int nIndex) const;
		TYPE& GetAt(int nIndex);
		void SetAt(int nIndex, TYPE newElement);
		const TYPE& ElementAt(int nIndex) const;
		TYPE& ElementAt(int nIndex);

		const TYPE* GetData() const;
		TYPE* GetData();

		void SetAtGrow(int nIndex, TYPE newElement);
		int Add(TYPE newElement);
		int Append(const Array& src);
		void Copy(const Array& src);

		const TYPE& operator[](int nIndex) const;
		TYPE& operator[](int nIndex);

		void InsertAt(int nIndex, TYPE newElement, int nCount = 1);
		void RemoveAt(int nIndex, int nCount = 1);
		void InsertAt(int nStartIndex, Array* pNewArray);

	protected:
		TYPE* m_pData;
		int m_nSize;
		int m_nMaxSize;
		int m_nGrowBy;
	};

	///////////////////////////////////////////////////////////////////////////////

	template<class TYPE>
	inline int Array<TYPE>::GetSize() const
	{
		return m_nSize;
	}
	template<class TYPE>
	inline int Array<TYPE>::GetCount() const
	{
		return m_nSize;
	}
	template<class TYPE>
	inline bool Array<TYPE>::IsEmpty() const
	{
		return m_nSize == 0;
	}
	template<class TYPE>
	inline int Array<TYPE>::GetUpperBound() const
	{
		return m_nSize - 1;
	}
	template<class TYPE>
	inline void Array<TYPE>::RemoveAll()
	{
		SetSize(0, -1);
	}
	template<class TYPE>
	inline TYPE& Array<TYPE>::GetAt(int nIndex)
	{
		NOTSTD_ASSERT(nIndex >= 0 && nIndex < m_nSize);
		if (nIndex >= 0 && nIndex < m_nSize)
			return m_pData[nIndex];
		NOTSTD_ArrayThrowInvalidArgException();
	}
	template<class TYPE>
	inline const TYPE& Array<TYPE>::GetAt(int nIndex) const
	{
		NOTSTD_ASSERT(nIndex >= 0 && nIndex < m_nSize);
		if (nIndex >= 0 && nIndex < m_nSize)
			return m_pData[nIndex];
		NOTSTD_ArrayThrowInvalidArgException();
	}
	template<class TYPE>
	inline void Array<TYPE>::SetAt(int nIndex, TYPE newElement)
	{
		NOTSTD_ASSERT(nIndex >= 0 && nIndex < m_nSize);
		if (nIndex >= 0 && nIndex < m_nSize)
			m_pData[nIndex] = newElement;
		else
			NOTSTD_ArrayThrowInvalidArgException();
	}
	template<class TYPE>
	inline const TYPE& Array<TYPE>::ElementAt(int nIndex) const
	{
		NOTSTD_ASSERT(nIndex >= 0 && nIndex < m_nSize);
		if (nIndex >= 0 && nIndex < m_nSize)
			return m_pData[nIndex];
		NOTSTD_ArrayThrowInvalidArgException();
	}
	template<class TYPE>
	inline TYPE& Array<TYPE>::ElementAt(int nIndex)
	{
		NOTSTD_ASSERT(nIndex >= 0 && nIndex < m_nSize);
		if (nIndex >= 0 && nIndex < m_nSize)
			return m_pData[nIndex];
		NOTSTD_ArrayThrowInvalidArgException();
	}
	template<class TYPE>
	inline const TYPE* Array<TYPE>::GetData() const
	{
		return (const TYPE*)m_pData;
	}
	template<class TYPE>
	inline TYPE* Array<TYPE>::GetData()
	{
		return (TYPE*)m_pData;
	}
	template<class TYPE>
	inline int Array<TYPE>::Add(TYPE newElement)
	{
		int nIndex = m_nSize;
		SetAtGrow(nIndex, newElement);
		return nIndex;
	}
	template<class TYPE>
	inline const TYPE& Array<TYPE>::operator[](int nIndex) const
	{
		return GetAt(nIndex);
	}
	template<class TYPE>
	inline TYPE& Array<TYPE>::operator[](int nIndex)
	{
		return ElementAt(nIndex);
	}

	///////////////////////////////////////////////////////////////////////////////

	template<class TYPE>
	Array<TYPE>::Array()
	{
		m_pData = NULL;
		m_nSize = m_nMaxSize = m_nGrowBy = 0;
	}

	template<class TYPE>
	Array<TYPE>::~Array()
	{
		if (m_pData != NULL)
		{
			for (int i = 0; i < m_nSize; i++)
				(m_pData + i)->~TYPE();
			MemFree(m_pData);
		}
	}

	template<class TYPE>
	void Array<TYPE>::SetSize(int nNewSize, int nGrowBy)
	{
		NOTSTD_ASSERT(nNewSize >= 0);

		if (nNewSize < 0)
			NOTSTD_ArrayThrowInvalidArgException();

		if (nGrowBy >= 0)
			m_nGrowBy = nGrowBy;

		if (nNewSize == 0)
		{
			if (m_pData != NULL)
			{
				for (int i = 0; i < m_nSize; i++)
					(m_pData + i)->~TYPE();
				MemFree(m_pData);
				m_pData = NULL;
			}
			m_nSize = m_nMaxSize = 0;
		}
		else if (m_pData == NULL)
		{
#ifdef SIZE_T_MAX
			NOTSTD_ASSERT(nNewSize <= SIZE_T_MAX / sizeof(TYPE));
#endif
			size_t nAllocSize = bl_array_max(nNewSize, m_nGrowBy);
			m_pData = reinterpret_cast<TYPE*>(MemAllocate((size_t)nAllocSize * sizeof(TYPE)));
			memset((void*)m_pData, 0, (size_t)nAllocSize * sizeof(TYPE));
			for (int i = 0; i < nNewSize; i++)
#pragma push_macro("new")
#undef new
				::new((void*)(m_pData + i)) TYPE;
#pragma pop_macro("new")
			m_nSize = nNewSize;
			m_nMaxSize = nAllocSize;
		}
		else if (nNewSize <= m_nMaxSize)
		{
			if (nNewSize > m_nSize)
			{
				memset((void*)(m_pData + m_nSize), 0, (size_t)(nNewSize - m_nSize) * sizeof(TYPE));
				for (int i = 0; i < nNewSize - m_nSize; i++)
#pragma push_macro("new")
#undef new
					::new((void*)(m_pData + m_nSize + i)) TYPE;
#pragma pop_macro("new")
			}
			else if (m_nSize > nNewSize)
			{
				for (int i = 0; i < m_nSize - nNewSize; i++)
					(m_pData + nNewSize + i)->~TYPE();
			}
			m_nSize = nNewSize;
		}
		else
		{
			nGrowBy = m_nGrowBy;
			if (nGrowBy == 0)
			{
				nGrowBy = m_nSize / 8;
				nGrowBy = (nGrowBy < 4) ? 4 : ((nGrowBy > 1024) ? 1024 : nGrowBy);
			}
			int nNewMax;
			if (nNewSize < m_nMaxSize + nGrowBy)
				nNewMax = m_nMaxSize + nGrowBy;
			else
				nNewMax = nNewSize;

			NOTSTD_ASSERT(nNewMax >= m_nMaxSize);

			if (nNewMax < m_nMaxSize)
				NOTSTD_ArrayThrowInvalidArgException();

#ifdef SIZE_T_MAX
			NOTSTD_ASSERT(nNewMax <= SIZE_T_MAX / sizeof(TYPE));
#endif
			TYPE *pNewData = reinterpret_cast<TYPE*>(MemAllocate((size_t)nNewMax * sizeof(TYPE)));

			memcpy(pNewData, m_pData, (size_t)m_nSize * sizeof(TYPE));

			NOTSTD_ASSERT(nNewSize > m_nSize);
			memset((void*)(pNewData + m_nSize), 0, (size_t)(nNewSize - m_nSize) * sizeof(TYPE));
			for (int i = 0; i < nNewSize - m_nSize; i++)
#pragma push_macro("new")
#undef new
				::new((void*)(pNewData + m_nSize + i)) TYPE;
#pragma pop_macro("new")

			MemFree(m_pData);
			m_pData = pNewData;
			m_nSize = nNewSize;
			m_nMaxSize = nNewMax;
		}
	}

	template<class TYPE>
	int Array<TYPE>::Append(const Array& src)
	{
		NOTSTD_ASSERT(this != &src);

		if (this == &src)
			NOTSTD_ArrayThrowInvalidArgException();

		int nOldSize = m_nSize;
		SetSize(m_nSize + src.m_nSize);
		CopyElements<TYPE>(m_pData + nOldSize, src.m_pData, src.m_nSize);
		return nOldSize;
	}

	template<class TYPE>
	void Array<TYPE>::Copy(const Array& src)
	{
		NOTSTD_ASSERT(this != &src);

		if (this != &src)
		{
			SetSize(src.m_nSize);
			CopyElements<TYPE>(m_pData, src.m_pData, src.m_nSize);
		}
	}

	template<class TYPE>
	void Array<TYPE>::FreeExtra()
	{
		if (m_nSize != m_nMaxSize)
		{
#ifdef SIZE_T_MAX
			NOTSTD_ASSERT(m_nSize <= SIZE_T_MAX / sizeof(TYPE));
#endif
			TYPE* pNewData = NULL;
			if (m_nSize != 0)
			{
				pNewData = reinterpret_cast<TYPE*>(MemAllocate(m_nSize * sizeof(TYPE)));
				memcpy(pNewData, m_pData, m_nSize * sizeof(TYPE));
			}

			MemFree(m_pData);
			m_pData = pNewData;
			m_nMaxSize = m_nSize;
		}
	}

	template<class TYPE>
	void Array<TYPE>::SetAtGrow(int nIndex, TYPE newElement)
	{
		NOTSTD_ASSERT(nIndex >= 0);

		if (nIndex < 0)
			NOTSTD_ArrayThrowInvalidArgException();

		if (nIndex >= m_nSize)
			SetSize(nIndex + 1, -1);
		m_pData[nIndex] = newElement;
	}

	template<class TYPE>
	void Array<TYPE>::InsertAt(int nIndex, TYPE newElement, int nCount /*=1*/)
	{
		NOTSTD_ASSERT(nIndex >= 0);    // will expand to meet need
		NOTSTD_ASSERT(nCount > 0);     // zero or negative size not allowed

		if (nIndex < 0 || nCount <= 0)
			NOTSTD_ArrayThrowInvalidArgException();

		if (nIndex >= m_nSize)
		{
			SetSize(nIndex + nCount, -1);
		}
		else
		{
			int nOldSize = m_nSize;
			SetSize(m_nSize + nCount, -1);
			for (int i = 0; i < nCount; i++)
				(m_pData + nOldSize + i)->~TYPE();
			memmove(m_pData + nIndex + nCount, m_pData + nIndex,
				(nOldSize - nIndex) * sizeof(TYPE));

			memset((void*)(m_pData + nIndex), 0, (size_t)nCount * sizeof(TYPE));
			for (int i = 0; i < nCount; i++)
#pragma push_macro("new")
#undef new
				::new((void*)(m_pData + nIndex + i)) TYPE;
#pragma pop_macro("new")
		}

		NOTSTD_ASSERT(nIndex + nCount <= m_nSize);
		while (nCount--)
			m_pData[nIndex++] = newElement;
	}

	template<class TYPE>
	void Array<TYPE>::RemoveAt(int nIndex, int nCount)
	{
		NOTSTD_ASSERT(nIndex >= 0);
		NOTSTD_ASSERT(nCount >= 0);
		int nUpperBound = nIndex + nCount;
		NOTSTD_ASSERT(nUpperBound <= m_nSize && nUpperBound >= nIndex && nUpperBound >= nCount);

		if (nIndex < 0 || nCount < 0 || (nUpperBound > m_nSize) || (nUpperBound < nIndex) || (nUpperBound < nCount))
			NOTSTD_ArrayThrowInvalidArgException();

		int nMoveCount = m_nSize - (nUpperBound);
		for (int i = 0; i < nCount; i++)
			(m_pData + nIndex + i)->~TYPE();
		if (nMoveCount)
		{
			memmove(m_pData + nIndex, m_pData + nUpperBound,
				(size_t)nMoveCount * sizeof(TYPE));
		}
		m_nSize -= nCount;
	}

	template<class TYPE>
	void Array<TYPE>::InsertAt(int nStartIndex, Array* pNewArray)
	{
		NOTSTD_ASSERT(pNewArray != NULL);
		NOTSTD_ASSERT(nStartIndex >= 0);

		if (pNewArray == NULL || nStartIndex < 0)
			NOTSTD_ArrayThrowInvalidArgException();

		if (pNewArray->GetSize() > 0)
		{
			InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
			for (int i = 0; i < pNewArray->GetSize(); i++)
				SetAt(nStartIndex + i, pNewArray->GetAt(i));
		}
	}

}
