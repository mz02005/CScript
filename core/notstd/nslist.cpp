#include "stdafx.h"
#include "nslist.h"
#include <stdlib.h>

namespace notstd {

	///////////////////////////////////////////////////////////////////////////////

	CPlexInfo* CPlexInfo::CreatePlex(CPlexInfo *&pHead, uint32_t nElemCount, uint32_t cbElemSize)
	{
		assert(nElemCount > 0 && cbElemSize > 0);
		int allocsize = sizeof(CPlexInfo) + nElemCount * cbElemSize;
		CPlexInfo *pNew = (CPlexInfo*)malloc(allocsize);
		pNew->m_nReserved = 0;
		pNew->m_pNext = pHead;
		pHead = pNew;
		return pHead;
	}

	void CPlexInfo::FreePlexChain()
	{
		CPlexInfo *pPlex = this;
		while (pPlex != NULL)
		{
			assert(pPlex->m_nReserved == 0);
			uint8_t *pData = (uint8_t*)pPlex;
			CPlexInfo *pNext = pPlex->m_pNext;
			free(pData);
			pPlex = pNext;
		}
	}

	void* CPlexInfo::GetData()
	{
		return(void*)(this + 1);
	}

	///////////////////////////////////////////////////////////////////////////////

	FixedSizeAllocator::FixedSizeAllocator(uint32_t nUnitSize, uint32_t nUnitPerBlock)
		: m_pPlex(NULL)
		, m_pFreeNodeHead(NULL)
		, m_nUnitSize(nUnitSize)
		, m_nUnitPerBlock(nUnitPerBlock)
	{
		assert(nUnitSize > 0 && m_nUnitPerBlock > 0);
	}

	FixedSizeAllocator::~FixedSizeAllocator()
	{
		freeall();
	}

	void FixedSizeAllocator::freeall()
	{
		if (m_pPlex) {
			m_pPlex->FreePlexChain();
			m_pFreeNodeHead = NULL;
			m_pPlex = NULL;
		}
	}

	void* FixedSizeAllocator::malloc()
	{
		if (!m_pFreeNodeHead)
		{
			CPlexInfo *pNewPlex = CPlexInfo::CreatePlex(m_pPlex,
				m_nUnitPerBlock, m_nUnitSize + sizeof(CNode));

			// 初始化空闲链表
			uint8_t *p = (uint8_t*)pNewPlex->GetData();
			for (uint32_t i = 0; i < m_nUnitPerBlock; i++)
			{
				CNode *pNode = (CNode*)(p + (m_nUnitSize + sizeof(CNode)) * i);
				pNode->m_pNext = m_pFreeNodeHead;
				m_pFreeNodeHead = pNode;
			}
		}

		assert(m_pFreeNodeHead);

		// 移除第一个空闲节点
		CNode *pRet = m_pFreeNodeHead;
		m_pFreeNodeHead = m_pFreeNodeHead->m_pNext;
		return(void*)(pRet + 1);
	}

	void FixedSizeAllocator::free(void *p)
	{
		if (p)
		{
			CNode *pNode = ((CNode*)p) - 1;
			pNode->m_pNext = m_pFreeNodeHead;
			m_pFreeNodeHead = pNode;
		}
	}

}
