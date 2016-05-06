#include "stdafx.h"
#include "nslist.h"
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////

CPlexInfo* CPlexInfo::CreatePlex(CPlexInfo *&pHead, UINT nElemCount, UINT cbElemSize)
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
		BYTE *pData = (BYTE*)pPlex;
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

FixedSizeAllocator::FixedSizeAllocator(UINT nUnitSize, UINT nUnitPerBlock)
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
	if(m_pPlex) {
		m_pPlex->FreePlexChain();
		m_pFreeNodeHead = NULL;
		m_pPlex = NULL;
	}
}

void* FixedSizeAllocator::malloc()
{
	if(!m_pFreeNodeHead)
	{
		CPlexInfo *pNewPlex = CPlexInfo::CreatePlex(m_pPlex, 
			m_nUnitPerBlock, m_nUnitSize + sizeof(CNode));

		// 初始化空闲链表
		BYTE *p = (BYTE*)pNewPlex->GetData();
		for (UINT i = 0; i < m_nUnitPerBlock; i ++)
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
	return(void*)(pRet+1);
}

void FixedSizeAllocator::free(void *p)
{
	if(p)
	{
		CNode *pNode = ((CNode*)p) - 1;
		pNode->m_pNext = m_pFreeNodeHead;
		m_pFreeNodeHead = pNode;
	}
}
