/*
	tab: 2
	版权所有mz02005@qq.com
	基本兼容mfc的双向链表类，由于std::list的遍历器在某些情况下使用不便
	所以实现了本类
*/

#ifndef _NSLIST_H
#define _NSLIST_H

#pragma once

#include "config.h"
// 用于replacement new操作符
#include <iostream>
#include <assert.h>

namespace notstd {

#pragma pack(push, 4)

	struct CPlexInfo;

	struct NOTSTD_API CPlexInfo
	{
		CPlexInfo *m_pNext;

		// 当对齐方式大于8时，数据部分会挤占CPlexInfo结构
		// 所以，预先分配4字节数据，填充至8字节大小
		uint32_t m_nReserved;

		static CPlexInfo* CreatePlex(CPlexInfo *&pHead, uint32_t nElemCount, uint32_t cbElemSize);
		void FreePlexChain();
		void* GetData();
	};

	class NOTSTD_API FixedSizeAllocator
	{
	protected:
		struct CNode
		{
			CNode *m_pNext;
		};

	private:
		CPlexInfo *m_pPlex;
		CNode *m_pFreeNodeHead;
		uint32_t m_nUnitSize;
		uint32_t m_nUnitPerBlock;

	public:
		// nUnitSize是每个用户分配单元的大小
		// 即FixedSizeAllocator::malloc函数返回的内存区域大小
		// nUnitPerBlock是每次分配器分配的内存块大小
		FixedSizeAllocator(uint32_t nUnitSize, uint32_t nUnitPerBlock);
		~FixedSizeAllocator();

		void freeall();
		void* malloc();
		void free(void *p);
	};

	// 一个双向链表模板类
	template <class T, int PreAllocCount = 4>
	class List
	{
	protected:
		struct CListNode
		{
			CListNode *m_pNext, *m_pPrev;
			T m_val;

			CListNode();
			CListNode(const T &t);
		};

	private:
		CListNode *m_pHead;
		CListNode *m_pTail;
		uint32_t m_count;
		FixedSizeAllocator *m_pAlloc;

		CListNode* NewNode();
		CListNode* NewNode(const T &t);
		void DeleteNode(CListNode *node);

	protected:
		inline CListNode* CreateNodeInternal(const T &t);
		inline CListNode* CreateNodeInternal();
		inline void DestroyNodeInternal(CListNode *pNode);

	public:
		List();
		virtual ~List();

		List(const List &other);
		List& operator = (const List &other);

		POSITION AddHead(const T &t);
		void AddHead(List *newlist);
		POSITION AddTail(const T &t);
		void AddTail(List *newlist);
		void AddTail(const List *newlist);

		const T& GetHead() const;
		T& GetHead();
		const T& GetTail() const;
		T& GetTail();

		T RemoveHead();
		T RemoveTail();
		void RemoveAll();

		POSITION GetHeadPosition() const;
		POSITION GetTailPosition() const;
		const T& GetNext(POSITION &pos) const;
		T& GetNext(POSITION &pos);
		const T& GetPrev(POSITION &pos) const;
		T& GetPrev(POSITION &pos);

		const T& GetAt(POSITION pos) const;
		T& GetAt(POSITION pos);
		void RemoveAt(POSITION position);
		void SetAt(POSITION pos, const T &t);

		POSITION InsertAfter(POSITION pos, const T& t);
		POSITION InsertBefore(POSITION pos, const T& t);

		POSITION Find(const T &t, POSITION startafter = NULL) const;
		POSITION FindIndex(uint32_t index) const;

		uint32_t GetCount() const;
		bool IsEmpty() const;
	};

	template <class T, int PreAllocCount>
	typename List<T, PreAllocCount>::CListNode* List<T, PreAllocCount>::NewNode()
	{
		void *nl = m_pAlloc->malloc();
		return new(nl)CListNode();
	}

	template <class T, int PreAllocCount>
	typename List<T, PreAllocCount>::CListNode* List<T, PreAllocCount>::NewNode(const T &t)
	{
		void *nl = m_pAlloc->malloc();
		return new(nl)CListNode(t);
	}

	template <class T, int PreAllocCount>
	void List<T, PreAllocCount>::DeleteNode(CListNode *node)
	{
		node->~CListNode();
		m_pAlloc->free(node);
	}

	template <class T, int PreAllocCount>
	List<T, PreAllocCount>::CListNode::CListNode()
		: m_pNext(NULL)
		, m_pPrev(NULL)
	{
	}

	template <class T, int PreAllocCount>
	List<T, PreAllocCount>::CListNode::CListNode(const T &t)
		: m_pNext(NULL)
		, m_pPrev(NULL)
		, m_val(t)
	{
	}

	template <class T, int PreAllocCount>
	typename List<T, PreAllocCount>::CListNode* List<T, PreAllocCount>::CreateNodeInternal(const T &t)
	{
		CListNode *pRet = NewNode(t);
		return pRet;
	}

	template <class T, int PreAllocCount>
	typename List<T, PreAllocCount>::CListNode* List<T, PreAllocCount>::CreateNodeInternal()
	{
		CListNode *pRet = NewNode();
		return pRet;
	}

	template <class T, int PreAllocCount>
	void List<T, PreAllocCount>::DestroyNodeInternal(CListNode *pNode)
	{
		DeleteNode(pNode);
	}

	template <class T, int PreAllocCount>
	List<T, PreAllocCount>::List()
	{
		m_pAlloc = reinterpret_cast<FixedSizeAllocator*>(malloc(sizeof(FixedSizeAllocator)));
		new(m_pAlloc)FixedSizeAllocator(sizeof(CListNode), PreAllocCount);
		m_pHead = CreateNodeInternal();
		m_pTail = CreateNodeInternal();
		m_pHead->m_pNext = m_pHead->m_pPrev = m_pTail;
		m_pTail->m_pNext = m_pTail->m_pPrev = m_pHead;
		m_count = 0;
	}

	template <class T, int PreAllocCount>
	List<T, PreAllocCount>::~List()
	{
		RemoveAll();
		DeleteNode(m_pHead); m_pHead = NULL;
		DeleteNode(m_pTail); m_pTail = NULL;
		m_pAlloc->~FixedSizeAllocator();
		free(m_pAlloc);
		m_pAlloc = NULL;
	}

	template <class T, int PreAllocCount>
	List<T, PreAllocCount>::List(const List &other)
	{
		*this = other;
	}

	template <class T, int PreAllocCount>
	List<T, PreAllocCount>& List<T, PreAllocCount>::operator = (const List &other)
	{
		m_pAlloc = reinterpret_cast<FixedSizeAllocator*>(malloc(sizeof(FixedSizeAllocator)));
		new(m_pAlloc)FixedSizeAllocator(sizeof(CListNode), PreAllocCount);
		m_pHead = CreateNodeInternal();
		m_pTail = CreateNodeInternal();
		m_pHead->m_pNext = m_pHead->m_pPrev = m_pTail;
		m_pTail->m_pNext = m_pTail->m_pPrev = m_pHead;
		m_count = 0;

		AddTail(&other);
		return *this;
	}

	template <class T, int PreAllocCount>
	POSITION List<T, PreAllocCount>::AddHead(const T &t)
	{
		CListNode *pNode = CreateNodeInternal(t);
		pNode->m_pNext = m_pHead->m_pNext;
		pNode->m_pPrev = m_pHead;
		m_pHead->m_pNext->m_pPrev = pNode;
		m_pHead->m_pNext = pNode;
		m_count++;

		return(POSITION)pNode;
	}

	template <class T, int PreAllocCount>
	void List<T, PreAllocCount>::AddHead(List *newlist)
	{
		POSITION pos = newlist->GetTailPosition();
		while (pos)
		{
			AddHead(newlist->GetPrev(pos));
		}
	}

	template <class T, int PreAllocCount>
	POSITION List<T, PreAllocCount>::AddTail(const T &t)
	{
		CListNode *pNode = CreateNodeInternal(t);
		pNode->m_pNext = m_pTail;
		pNode->m_pPrev = m_pTail->m_pPrev;
		m_pTail->m_pPrev->m_pNext = pNode;
		m_pTail->m_pPrev = pNode;
		m_count++;
		return(POSITION)pNode;
	}

	template <class T, int PreAllocCount>
	void List<T, PreAllocCount>::AddTail(List *newlist)
	{
		POSITION pos = newlist->GetHeadPosition();
		while (pos)
		{
			AddTail(newlist->GetNext(pos));
		}
	}

	template <class T, int PreAllocCount>
	void List<T, PreAllocCount>::AddTail(const List *newlist)
	{
		POSITION pos = newlist->GetHeadPosition();
		while (pos)
		{
			AddTail(newlist->GetNext(pos));
		}
	}

	template <class T, int PreAllocCount>
	const T& List<T, PreAllocCount>::GetHead() const
	{
		assert(!IsEmpty());
		return m_pHead->m_pNext->m_val;
	}

	template <class T, int PreAllocCount>
	T& List<T, PreAllocCount>::GetHead()
	{
		assert(!IsEmpty());
		return m_pHead->m_pNext->m_val;
	}

	template <class T, int PreAllocCount>
	const T& List<T, PreAllocCount>::GetTail() const
	{
		assert(!IsEmpty());
		return m_pTail->m_pPrev->m_val;
	}

	template <class T, int PreAllocCount>
	T& List<T, PreAllocCount>::GetTail()
	{
		assert(!IsEmpty());
		return m_pTail->m_pPrev->m_val;
	}

	template <class T, int PreAllocCount>
	T List<T, PreAllocCount>::RemoveHead()
	{
		assert(!IsEmpty());
		T r = m_pHead->m_pNext->m_val;
		CListNode *del = m_pHead->m_pNext;
		m_pHead->m_pNext = del->m_pNext;
		del->m_pNext->m_pPrev = m_pHead;
		m_count--;
		DestroyNodeInternal(del);
		return r;
	}

	template <class T, int PreAllocCount>
	T List<T, PreAllocCount>::RemoveTail()
	{
		assert(!IsEmpty());
		T r = m_pTail->m_pPrev->m_val;
		CListNode *del = m_pTail->m_pPrev;
		m_pTail->m_pPrev = del->m_pPrev;
		m_pTail->m_pPrev->m_pNext = m_pTail;
		m_count--;
		DestroyNodeInternal(del);
		return r;
	}

	template <class T, int PreAllocCount>
	void List<T, PreAllocCount>::RemoveAll()
	{
		CListNode *node = m_pHead->m_pNext;
		while (node != m_pTail)
		{
			CListNode *del = node;
			node = node->m_pNext;
			DestroyNodeInternal(del);
		}
		m_pHead->m_pNext = m_pTail;
		m_pHead->m_pPrev = m_pTail;
		m_pTail->m_pNext = m_pHead;
		m_pTail->m_pPrev = m_pHead;
		m_count = 0;
	}

	template <class T, int PreAllocCount>
	POSITION List<T, PreAllocCount>::GetHeadPosition() const
	{
		if (IsEmpty())
			return NULL;
		return(POSITION)m_pHead->m_pNext;
	}

	template <class T, int PreAllocCount>
	POSITION List<T, PreAllocCount>::GetTailPosition() const
	{
		if (IsEmpty())
			return NULL;
		return(POSITION)m_pTail->m_pPrev;
	}

	template <class T, int PreAllocCount>
	const T& List<T, PreAllocCount>::GetNext(POSITION &pos) const
	{
		assert(pos);
		CListNode *pNode = (CListNode*)pos;
		pos = POSITION(pNode->m_pNext != m_pTail ? pNode->m_pNext : NULL);
		return pNode->m_val;
	}

	template <class T, int PreAllocCount>
	T& List<T, PreAllocCount>::GetNext(POSITION &pos)
	{
		assert(pos);
		CListNode *pNode = (CListNode*)pos;
		pos = pNode->m_pNext != m_pTail ? (POSITION)pNode->m_pNext : NULL;
		return pNode->m_val;
	}

	template <class T, int PreAllocCount>
	const T& List<T, PreAllocCount>::GetPrev(POSITION &pos) const
	{
		assert(pos);
		CListNode *pNode = (CListNode*)pos;
		pos = pNode->m_pPrev != m_pHead ? pNode->m_pPrev : NULL;
		return pNode->m_val;
	}

	template <class T, int PreAllocCount>
	T& List<T, PreAllocCount>::GetPrev(POSITION &pos)
	{
		assert(pos);
		CListNode *pNode = (CListNode*)pos;
		pos = pNode->m_pPrev != m_pHead ? reinterpret_cast<POSITION&>(pNode->m_pPrev) : NULL;
		return pNode->m_val;
	}

	template <class T, int PreAllocCount>
	const T& List<T, PreAllocCount>::GetAt(POSITION pos) const
	{
		assert(pos);
		CListNode *pNode = (CListNode*)pos;
		return pNode->m_val;
	}

	template <class T, int PreAllocCount>
	T& List<T, PreAllocCount>::GetAt(POSITION pos)
	{
		assert(pos);
		CListNode *pNode = (CListNode*)pos;
		return pNode->m_val;
	}

	template <class T, int PreAllocCount>
	void List<T, PreAllocCount>::RemoveAt(POSITION position)
	{
		assert(position);
		CListNode *pNode = (CListNode*)position;
		pNode->m_pPrev->m_pNext = pNode->m_pNext;
		pNode->m_pNext->m_pPrev = pNode->m_pPrev;
		DestroyNodeInternal(pNode);
		m_count--;
	}

	template <class T, int PreAllocCount>
	void List<T, PreAllocCount>::SetAt(POSITION pos, const T &t)
	{
		assert(pos);
		CListNode *pNode = (CListNode*)pos;
		pNode->m_val = t;
	}

	template <class T, int PreAllocCount>
	POSITION List<T, PreAllocCount>::InsertAfter(POSITION pos, const T& t)
	{
		if (!pos)
			return AddTail(t);

		CListNode *pNode = (CListNode*)pos;
		CListNode *pNewNode = CreateNodeInternal(t);
		pNewNode->m_pNext = pNode->m_pNext;
		pNewNode->m_pPrev = pNode;
		pNode->m_pNext->m_pPrev = pNewNode;
		pNode->m_pNext = pNewNode;
		m_count++;
		return(POSITION)pNewNode;
	}

	template <class T, int PreAllocCount>
	POSITION List<T, PreAllocCount>::InsertBefore(POSITION pos, const T& t)
	{
		if (!pos)
			return AddHead(t);

		CListNode *pNode = (CListNode*)pos;
		CListNode *pNewNode = CreateNodeInternal(t);
		pNewNode->m_pNext = pNode;
		pNewNode->m_pPrev = pNode->m_pPrev;
		pNode->m_pPrev->m_pNext = pNewNode;
		pNode->m_pPrev = pNewNode;
		m_count++;
		return(POSITION)pNewNode;
	}

	template <class T, int PreAllocCount>
	POSITION List<T, PreAllocCount>::Find(const T &t, POSITION startafter) const
	{
		CListNode *pNode;
		if (startafter) {
			pNode = (CListNode*)startafter;
		}
		else
		{
			//pNode =(CListNode*)GetHeadPosition();
			pNode = m_pHead->m_pNext;
		}
		while (pNode != m_pTail && pNode->m_val != t)
			pNode = pNode->m_pNext;
		return(pNode != m_pTail ? (POSITION)pNode : NULL);
	}

	template <class T, int PreAllocCount>
	POSITION List<T, PreAllocCount>::FindIndex(uint32_t index) const
	{
		if (index >= m_count || index < 0)
			return NULL;

		CListNode* pNode = m_pHead->m_pNext;
		while (index--)
		{
			pNode = pNode->m_pNext;
		}
		return(POSITION)pNode;
	}

	template <class T, int PreAllocCount>
	uint32_t List<T, PreAllocCount>::GetCount() const
	{
		return m_count;
	}

	template <class T, int PreAllocCount>
	bool List<T, PreAllocCount>::IsEmpty() const
	{
		return(GetCount() <= 0);
	}

#pragma pack(pop)
}

#endif
