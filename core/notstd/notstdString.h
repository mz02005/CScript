#pragma once
#include "config.h"
#include <assert.h>

// 把<string>移到这里是为了能够在fedora9上正确编译通过
// fedora上的为gcc4.3.0在编译stringP.hpp会出现莫名错误
#include <string>
#include <wchar.h>
#include <stdarg.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <wctype.h>
#include <stdio.h>
#include "notstdMemory.h"

namespace notstd {

	template <class CHARTYPE>
	struct CStringOperTrait
	{
		static int strcmp(const CHARTYPE *string1, const CHARTYPE *string2);
		static int strncmp(const CHARTYPE *string1, const CHARTYPE *string2, size_t count);
		static CHARTYPE* strcpy(CHARTYPE *strDestination, const CHARTYPE *strSource);
		static size_t strlen(const CHARTYPE *str);
		static int tolower(int c);
		static int toupper(int c);
		static const CHARTYPE* strchr(const CHARTYPE *str, int c);
		static int vsnprintf(CHARTYPE *string, size_t count,
			const CHARTYPE *format, va_list ap);
	};

	template <>
	struct CStringOperTrait<char>
	{
		static int strncmp(const char *string1, const char *string2, size_t count)
		{
			return ::strncmp(string1, string2, count);
		}

		static int strcmp(const char *string1, const char *string2)
		{
			return ::strcmp(string1, string2);
		}

		static char* strcpy(char *strDestination, const char *strSource)
		{
			return ::strcpy(strDestination, strSource);
		}

		static size_t strlen(const char *str)
		{
			return ::strlen(str);
		}

		static int tolower(int c)
		{
			return ::tolower(c);
		}

		static int toupper(int c)
		{
			return ::toupper(c);
		}

		static const char * strchr(const char *str, int c)
		{
			return ::strchr(str, c);
		}

		static int vsnprintf(char *string, size_t count,
			const char *format, va_list ap)
		{
			return ::vsnprintf(string, count, format, ap);
		}
	};

	template <>
	struct CStringOperTrait<wchar_t>
	{
		static int strncmp(const wchar_t *string1, const wchar_t *string2, size_t count)
		{
			return ::wcsncmp(string1, string2, count);
		}

		static int strcmp(const wchar_t *string1, const wchar_t *string2)
		{
			return ::wcscmp(string1, string2);
		}

		static wchar_t* strcpy(wchar_t *strDestination, const wchar_t *strSource)
		{
			return ::wcscpy(strDestination, strSource);
		}

		static size_t strlen(const wchar_t *str)
		{
			return ::wcslen(str);
		}

		static int tolower(int c)
		{
			return ::towlower((wint_t)c);
		}

		static int toupper(int c)
		{
			return ::towupper((wint_t)c);
		}

		static const wchar_t * strchr(const wchar_t *str, int c)
		{
			return ::wcschr(str, c);
		}

		static int vsnprintf(wchar_t *string, size_t count,
			const wchar_t *format, va_list ap)
		{
#ifdef WIN32
			return ::_vsnwprintf(string, count, format, ap);
#else
			string[0] = 0;
			return 0;
#endif
		}
	};

	static const int defaultStackSize = 64;

	template <class CharType>
	struct StringQData
	{
		int mLen;
		union {
			CharType mStackBuf[defaultStackSize];
			struct {
				int mCap;
				CharType *mBuf;
			};
		};
		bool mNotOnStack;

		StringQData()
			: mLen(0)
			, mNotOnStack(false)
		{
			mStackBuf[0] = 0;
		}

		~StringQData()
		{
			if (mNotOnStack)
			{
				MemFree(mBuf);
				mBuf = NULL;
				mCap = 0;
			}
			mLen = 0;
			mNotOnStack = false;
		}

		int GetCap() const {
			return (mNotOnStack ? mCap : defaultStackSize);
		}

		static bool MayPlacedInStack(int size)
		{
			return (size < defaultStackSize);
		}

		// 直接将字符串附加过来，供内部使用的函数
		// 总是将当前字符串置为分配于堆上的
		void Attach(CharType *str, int newCap, int length)
		{
			if (mNotOnStack) {
				//delete [] mBuf;
				MemFree(mBuf);
			}
			mLen = length;
			mBuf = str;
			mCap = newCap;
			mNotOnStack = true;
		}

		// 得到内部缓冲区
		CharType* GetDirectBuffer()
		{
			return (mNotOnStack ? mBuf : mStackBuf);
		}

		const CharType* GetDirectBuffer() const
		{
			return (mNotOnStack ? mBuf : mStackBuf);
		}

		// 得到指定大小的缓冲区
		CharType* GetBuffer(int len)
		{
			if (len <= mLen)
			{
				return (mNotOnStack ? mBuf : mStackBuf);
			}

			if (mNotOnStack)
			{
				if (len < mCap)
					return mBuf;

				if (mCap < 1024)
					mCap *= 2;
				if (mCap <= len) {
					mCap = len + 1;
					mCap += 7;
					mCap /= 8;
					mCap *= 8;
				}
				CharType *p = reinterpret_cast<CharType*>(MemReallocate(mBuf, sizeof(CharType) * mCap));
				if (!p) {
					return p;
				}
				mBuf = p;
				return mBuf;
			}
			else
			{
				if (!MayPlacedInStack(len))
				{
					// 这种情况下，就需要在堆上分配内存
					int newCap = (len + 1 + 7) / 8 * 8;
					CharType *p = reinterpret_cast<CharType*>(MemAllocate(sizeof(CharType) * newCap));
					memcpy(p, mStackBuf, (mLen + 1) * sizeof(CharType));
					mNotOnStack = true;
					// uNewCap必须用临时变量，如果用mCap会覆盖掉mStackBuf的前面4字节内容
					mCap = newCap;
					mBuf = p;
					return mBuf;
				}
				return mStackBuf;
			}
		}
	};

	template <class CharType>
	class StringT
	{
	private:
		StringQData<CharType> m_strData;

	public:
		StringT()
		{
		}
		~StringT()
		{
		}

		StringT(const CharType *str, int len = -1)
		{
			if (len >= 0)
				Append(str, len);
			else
				*this = str;
		}

		StringT(const StringT &str)
		{
			*this = str;
		}

		StringT(CharType c)
		{
			*this = c;
		}

		static StringT MakeString(CharType c, int count)
		{
			StringT str;
			if (count < 0)
				count = 0;
			CharType *p = str.m_strData.GetBuffer(count);
			memset(p, c, count * sizeof(CharType));
			p[count] = 0;
			str.m_strData.mLen = count;
			return str;
		}

		void SetEmpty()
		{
			m_strData.mLen = 0;
			if (m_strData.mNotOnStack)
				m_strData.mBuf[0] = 0;
			else
				m_strData.mStackBuf[0] = 0;
		}

		StringT& operator = (const CharType *str)
		{
			SetEmpty();
			size_t size = CStringOperTrait<CharType>::strlen(str);
			return Append(str, size);
		}

		StringT& operator = (const StringT &str)
		{
			SetEmpty();
			return Append(str, str.GetLength());
		}

		StringT& operator = (CharType c)
		{
			SetEmpty();
			return Append(&c, 1);
		}

		int GetLength() const {
			return m_strData.mLen;
		}

		operator const CharType*() const {
			return m_strData.GetDirectBuffer();
		}

		const CharType* c_str() const {
			return m_strData.GetDirectBuffer();
		}

		// 获得单个字符
		CharType& operator [](int charIndex)
		{
			return m_strData.GetDirectBuffer()[charIndex];
		}

		CharType operator [](int charIndex) const
		{
			return m_strData.GetDirectBuffer()[charIndex];
		}

		CharType GetAt(int charIndex) const
		{
			if (charIndex < 0 || charIndex >= GetLength())
				throw 1;
			return (*this)[charIndex];
		}

		// Append
		StringT& operator += (const CharType *str)
		{
			return Append(str, CStringOperTrait<CharType>::strlen(str));
		}

		StringT& operator += (CharType c)
		{
			return Append(&c, 1);
		}

		StringT& operator += (const StringT &str)
		{
			return Append(str, str.GetLength());
		}

		StringT& AppendFormat(const CharType *strFormat, ...)
		{
			StringT strTemporary;
			va_list argList;
			va_start(argList, strFormat);
			strTemporary.FormatV(strFormat, argList);
			va_end(argList);
			return Append(strTemporary);
		}

		StringT& Append(const CharType *str, int length)
		{
			int nNeedLen;
			CharType *pTemp;

			if (length <= 0)
				return *this;

			nNeedLen = GetLength() + length;
			pTemp = m_strData.GetBuffer(nNeedLen);
			memcpy(pTemp + GetLength(), str, length * sizeof(CharType));
			m_strData.mLen += length;
			pTemp[m_strData.mLen] = 0;
			return *this;
		}

		StringT& Append(const StringT &str)
		{
			return Append(str, str.GetLength());
		}

		StringT& PreAppend(const CharType *str, int length)
		{
			if (length <= 0)
				return *this;

			int nNewLen = length + GetLength();

			if (!m_strData.mNotOnStack && StringQData<CharType>::MayPlacedInStack(nNewLen))
			{
				memmove(m_strData.mStackBuf + length, m_strData.mStackBuf, (m_strData.mLen + 1) * sizeof(CharType));
				memcpy(m_strData.mStackBuf, str, length * sizeof(CharType));
				m_strData.mLen = nNewLen;
				return *this;
			}

			// 如果本来就是基于堆的，但是容量已经足够了，则直接移动，否则需要重新分配内存
			if (m_strData.mNotOnStack && m_strData.mCap > nNewLen)
			{
				memmove(m_strData.mBuf + length, m_strData.mBuf, (m_strData.mLen + 1) * sizeof(CharType));
				memcpy(m_strData.mBuf, str, length * sizeof(CharType));
				m_strData.mLen = nNewLen;
				return *this;
			}

			// 计算新的容量
			int mCapCacl = (nNewLen + 1 + 7) / 8 * 8;
			if (mCapCacl < 1024)
				mCapCacl *= 2;
			CharType *p = reinterpret_cast<CharType*>(MemAllocate(sizeof(CharType) * mCapCacl));
			memcpy(p + length, m_strData.GetDirectBuffer(), (m_strData.mLen + 1) * sizeof(CharType));
			memcpy(p, str, length * sizeof(CharType));
			m_strData.Attach(p, mCapCacl, nNewLen);

			return *this;
		}

		StringT& PreAppend(const StringT &str)
		{
			PreAppend(str, str.GetLength());
			return *this;
		}

		// Format
		void FormatV(const CharType *lpszFormat, va_list args)
		{
			int n, nSize = m_strData.GetCap() - 1;
			CharType *pTemp = m_strData.GetDirectBuffer();

			n = CStringOperTrait<CharType>::vsnprintf(pTemp, nSize,
				lpszFormat, args);

#ifdef WIN32
			while (n == -1)
			{
				if (nSize > 1024)
					nSize += 1024;
				else
					nSize *= 2;

				pTemp = m_strData.GetBuffer(nSize);

				n = CStringOperTrait<CharType>::vsnprintf(pTemp, nSize - 1,
					lpszFormat, args);
			}
#else
			if (n < 0)
				return;

			if (n >= nSize)
			{
				nSize = n * 2;
				pTemp = m_strData.GetBuffer(nSize);
				n = CStringOperTrait<CharType>::vsnprintf(pTemp, nSize,
					lpszFormat, args);
			}
#endif

			m_strData.mLen = n;
			pTemp[n] = 0;
		}

		StringT& Format(const CharType *strFormat, ...)
		{
			if (strFormat == NULL)
				return *this;
			va_list argList;
			va_start(argList, strFormat);
			FormatV(strFormat, argList);
			va_end(argList);
			return *this;
		}

		static StringT SFormat(const CharType *strFormat, ...)
		{
			StringT ret;
			if (strFormat == nullptr)
				return ret;
			va_list argList;
			va_start(argList, strFormat);
			ret.FormatV(strFormat, argList);
			va_end(argList);
			return ret;
		}

		// 截取
		StringT Mid(int first, int count) const
		{
			StringT str;
			if (first < 0)
				first = 0;
			if (first >= GetLength())
				first = GetLength();
			if (first + count > GetLength())
				count = GetLength() - first;
			if (count < 0)
				count = 0;
			CharType *p = str.m_strData.GetBuffer(count);
			memcpy(p, c_str() + first, (count)* sizeof(CharType));
			p[count] = 0;
			str.m_strData.mLen = count;
			return str;
		}

		StringT Mid(int first) const
		{
			// 长度交由上一个版本的Mid函数来截取和判断
			return Mid(first, GetLength() - first);
		}

		StringT& SetLength(int length)
		{
			if (length < 0)
				length = 0;
			CharType *p = m_strData.GetBuffer(length);
			p[length] = 0;
			m_strData.mLen = length;
			return *this;
		}

		StringT& Truncate(int length)
		{
			return SetLength(length);
		}

		StringT Right(int length) const
		{
			if (length < 0 || length > GetLength())
				return *this;

			return Mid(GetLength() - length);
		}

		StringT Left(int length) const
		{
			if (length < 0 || length > GetLength())
				return *this;

			return Mid(0, length);
		}

		StringT& MakeLower()
		{
			CharType *p = m_strData.GetDirectBuffer();
			const CharType *pEnd = p + GetLength();
			for (; p != pEnd; p++)
			{
				*p = CStringOperTrait<CharType>::tolower(*p);
			}
			return *this;
		}

		StringT& MakeUpper()
		{
			CharType *p = m_strData.GetDirectBuffer();
			const CharType *pEnd = p + GetLength();
			for (; p != pEnd; p++)
			{
				*p = CStringOperTrait<CharType>::toupper(*p);
			}
			return *this;
		}

		StringT& TrimLeft(CharType cTarget)
		{
			StringT str;
			CharType *p = m_strData.GetDirectBuffer();
			CharType *pEnd = p + GetLength();
			for (; p != pEnd && *p == cTarget; p++)
			{
			}
			int uNewLen = pEnd - p;
			memmove(m_strData.GetDirectBuffer(), p, (uNewLen + 1) * sizeof(CharType));
			m_strData.mLen = uNewLen;
			return *this;
		}

		StringT& TrimLeft(const CharType* pszTargets)
		{
			CharType *p = m_strData.GetDirectBuffer(), *pEnd = p + GetLength();
			for (; p != pEnd && CStringOperTrait<CharType>::strchr(pszTargets, *p); p++)
			{
			}
			if (p != m_strData.GetDirectBuffer())
			{
				StringT str(p);
				*this = str;
			}
			return *this;
		}

		StringT& TrimRight(CharType cTarget)
		{
			if (GetLength() > 0)
			{
				CharType *pEnd = m_strData.GetDirectBuffer();
				CharType *p = pEnd + GetLength() - 1;
				for (; p >= pEnd && *p == cTarget; p--)
				{
				}
				*(p + 1) = 0;
				m_strData.mLen = p - m_strData.GetDirectBuffer() + 1;
			}
			return *this;
		}

		StringT& TrimRight(const CharType* pszTargets)
		{
			if (GetLength() > 0)
			{
				CharType *pEnd = m_strData.GetDirectBuffer();
				CharType *p = pEnd + GetLength() - 1;
				for (; p >= pEnd && CStringOperTrait<CharType>::strchr(pszTargets, *p); p--)
				{
				}
				*(p + 1) = 0;
				m_strData.mLen = p - m_strData.GetDirectBuffer() + 1;
			}
			return *this;
		}

		StringT& Trim(CharType cTarget)
		{
			return TrimRight(cTarget).TrimLeft(cTarget);
		}

		StringT& Trim(const CharType *pszTarget)
		{
			return TrimRight(pszTarget).TrimLeft(pszTarget);
		}

		// Find
		int Find(const CharType *strSub, int start = 0) const
		{
			int r = -1;

			if (start < 0)
				return r;

			int nPatternLen = (int)CStringOperTrait<CharType>::strlen(strSub);
			int nStrLen = (int)GetLength();

			if (nStrLen < nPatternLen
				|| nPatternLen <= 0)
				return r;

			if (nPatternLen >= 4)
			{
				// 使用KMP

				int nIndex, nPatternIndex, nSourceIndex;

				// 舍入到4的倍数
				int nAllocSize = (nPatternLen + 3) / 4 * 4;
				int *pNextTable = reinterpret_cast<int*>(MemAllocate(sizeof(int) * nAllocSize));//new int[nAllocSize];

				pNextTable[0] = -1;
				for (int i = 1; i < nPatternLen; i++)
				{
					nIndex = pNextTable[i - 1];
					while (nIndex >= 0 && strSub[nIndex + 1] != strSub[i])
						nIndex = pNextTable[nIndex];
					if (strSub[nIndex + 1] == strSub[i])
						pNextTable[i] = nIndex + 1;
					else
						pNextTable[i] = -1;
				}

				nPatternIndex = 0;
				nSourceIndex = start;
				while (nPatternIndex < nPatternLen && nSourceIndex < nStrLen)
				{
					if ((*this)[nSourceIndex] == strSub[nPatternIndex])
					{
						nPatternIndex++;
						nSourceIndex++;
					}
					else if (nPatternIndex == 0)
						nSourceIndex++;
					else
						nPatternIndex = pNextTable[nPatternIndex - 1] + 1;
				}
				if (nPatternIndex == nPatternLen)
					r = nSourceIndex - nPatternIndex;

				// 如下两行可以省略
				//else
				//	r = -1;

				//delete [] pNextTable;
				MemFree(pNextTable);
			}
			else
			{
				// 使用简单的算法
				for (int i = start; i <= nStrLen - nPatternLen; i++)
				{
					if (memcmp(strSub, m_strData.GetDirectBuffer() + i, nPatternLen * sizeof(CharType)) == 0)
						return i;
				}
			}

			return r;
		}

		int Find(CharType ch, int start = 0) const
		{
			int nLength = GetLength(), nRet = -1, temp = start;
			if (start < 0 || start >= nLength)
				return nRet;

			const CharType *pTemp = m_strData.GetDirectBuffer();
			for (; temp < nLength && pTemp[temp] != ch; temp++)
			{
			}
			if (temp == nLength)
				return nRet;

			return temp;
		}

		int FindOneOf(CharType ch, int start = 0) const
		{
			return Find(ch, start);
		}

		int FindOneOf(const CharType *strSub, int start = 0) const
		{
			int len, i;
			CharType c;
			const CharType *p;

			len = GetLength();
			for (i = start; i < len; i++)
			{
				c = (*this)[i];
				for (p = strSub; *p != 0; p++)
				{
					if (*p == c)
						return i;
				}
			}

			return -1;
		}

		int ReverseFind(CharType ch) const
		{
			int f = -1;

			if (!GetLength())
				return f;

			for (f = GetLength() - 1; f >= 0; f--)
			{
				if ((*this)[f] == ch)
					return f;
			}

			return f;
		}

		int ReverseFind(const CharType *strSub) const
		{
			const CharType *p, *s;
			int len = (int)CStringOperTrait<CharType>::strlen(strSub);

			if (len > GetLength())
				return -1;

			s = m_strData.GetDirectBuffer();
			p = s + GetLength() - len;

			for (; p >= s &&
				memcmp(p, strSub, len * sizeof(CharType)) != 0; p--)
			{
			}

			if (p >= s)
				return (int)(p - s);

			return -1;
		}

		// Replace
		int Replace(CharType cOld, CharType cNew)
		{
			int count = 0;
			int i;
			for (i = 0; i < GetLength(); i++)
			{
				CharType &c = (*this)[i];
				if (c == cOld)
				{
					c = cNew;
					count++;
				}
			}
			return count;
		}

		int Replace(const CharType *pszOld, const CharType *pszNew)
		{
			int f = Find(pszOld);
			int r = 0;
			if (f >= 0)
			{
				int nLast = 0;
				notstd::StringT<CharType> str;
				int nOldLen = (int)CStringOperTrait<CharType>::strlen(pszOld);

				do {
					r += nOldLen;
					str += Mid(nLast, f - nLast);
					str += pszNew;
					nLast = f + nOldLen;
					f = Find(pszOld, nLast);
				} while (f >= 0);
				str += Mid(nLast);
				*this = str;
			}
			return r;
		}

		// GetBuffer是兼容MFC库中，很重要的一个函数
		// 返回以0结尾的字符串缓冲
		// 如果nMinBufferLength大于当前缓冲区的长度，本函数会销毁当前缓冲
		// 并重新分配足够的空间，原来位置上的内容保持不变
		// 在调用GetBuffer之后，必须使用ReleaseBuffer函数来释放资源
		// 这意味着GetBuffer返回的缓冲区在调用ReleaseBuffer后就不合法了
		// 缓冲区内容和相关资源在对象析构时会释放掉
		// 如果在ReleaseBuffer时，nMinBufferLength为-1，最终缓冲区字符串当度
		// 通过调用strlen来获得，如果指定了长度，则最终字符串的长度就是指定的长度
		CharType* GetBuffer(int minBufferLength)
		{
			return m_strData.GetBuffer(minBufferLength);
		}

		CharType* GetBuffer()
		{
			return m_strData.GetDirectBuffer();
		}

		void ReleaseBuffer(int newLength = -1)
		{
			// 这步必须放在下面的判断前面
			// 因为判断后面有个GetBuffer在需要迁移数据时，根据长度来决定移动数据的长度
			m_strData.mLen = (int)CStringOperTrait<CharType>::strlen(m_strData.GetDirectBuffer());
			if (newLength < 0)
			{
				m_strData.GetDirectBuffer()[m_strData.mLen] = 0;
				return;
			}

			CharType *p = m_strData.GetBuffer(newLength);
			p[newLength] = 0;
			m_strData.mLen = newLength;

			return;
		}

		// 处理记号
		StringT Tokenize(const CharType *pszTokens, int &start)
		{
			if (start < 0 || start > GetLength())
				throw StringT<char>("Invalid param");
			const CharType *pCur = &(*this)[start],
				*pEnd = &(*this)[0] + GetLength();
			while (pCur != pEnd)
			{
				if (CStringOperTrait<CharType>::strchr(pszTokens, *pCur))
				{
					int nBegin = start, nLength = (pCur - &(*this)[0]) - start;
					start = (pCur - m_strData.GetDirectBuffer()) + 1;
					return Mid(nBegin, nLength);
				}
				pCur++;
			}
			int startPos = start;
			start = -1;
			if (start < GetLength()) {
				return Mid(startPos);
			}
			return StringT();
		}

		// 各种友元
		friend StringT operator + (const StringT &str1, const StringT &str2)
		{
			StringT str = str1;
			str += str2;
			return str;
		}

		friend StringT operator + (const StringT &str1, const CharType *str2)
		{
			StringT str = str1;
			str += str2;
			return str;
		}

		friend StringT operator + (const CharType *str1, const StringT &str2)
		{
			StringT str = str1;
			str += str2;
			return str;
		}

		friend StringT operator + (const StringT &str, CharType c)
		{
			StringT r = str;
			r += c;
			return r;
		}

		friend bool operator == (const StringT &str1, const StringT &str2)
		{
			int len1 = str1.GetLength();
			int len2 = str2.GetLength();
			return
				(len1 == len2) ?
				memcmp(str1, str2, len1 * sizeof(CharType)) == 0 :
				false;
		}

		friend bool operator == (const StringT &str1, const CharType *str2)
		{
			int len1 = str1.GetLength();
			int len2 = CStringOperTrait<CharType>::strlen(str2);
			return (len1 == len2) ?
				memcmp(str1, str2, len1 * sizeof(CharType)) == 0 :
				false;
		}

		friend bool operator != (const StringT &str1, const StringT &str2)
		{
			int len1 = str1.GetLength();
			int len2 = str2.GetLength();
			return (len1 != len2) ?
				true :
				memcmp(str1, str2, len1 * sizeof(CharType)) != 0;
		}

		friend bool operator != (const StringT &str1, const CharType *str2)
		{
			return !(str1 == str2);
		}

		friend bool operator != (const CharType *str1,
			const StringT &str2)
		{
			return !(StringT(str1) == str2);
		}

		friend bool operator < (const StringT &str1, const StringT &str2)
		{
			int len1 = str1.GetLength();
			int len2 = str2.GetLength();
			int compsize = len1 < len2 ? len1 : len2;
			int r = memcmp(str1, str2, compsize * sizeof(CharType));
			if (r < 0)
				return true;
			if (r == 0) {
				return len1 < len2;
			}
			return false;
		}

		friend bool operator < (const StringT &str1, const CharType *str2)
		{
			int len1 = str1.GetLength();
			int len2 = CStringOperTrait<CharType>::strlen(str2);
			int compsize = len1 < len2 ? len1 : len2;
			int r = memcmp(str1, str2, compsize * sizeof(CharType));
			if (r < 0)
				return true;
			if (r == 0)
				return len1 < len2;
			return false;
		}

		friend bool operator < (const CharType *str1, const StringT &str2)
		{
			int len1 = CStringOperTrait<CharType>::strlen(str1);
			int len2 = str2.GetLength();
			int compsize = len1 < len2 ? len1 : len2;
			int r = memcmp(str1, str2, compsize * sizeof(CharType));
			if (r < 0)
				return true;
			if (r == 0)
				return len1 < len2;
			return false;
		}

		friend bool operator >(const StringT &str1, const StringT &str2)
		{
			int len1 = str1.GetLength();
			int len2 = str2.GetLength();
			int compsize = len1 < len2 ? len1 : len2;
			int r = memcmp(str1, str2, compsize * sizeof(CharType));
			if (r > 0)
				return true;
			if (r == 0)
				return len1 > len2;
			return false;
		}

		friend bool operator > (const StringT &str1,
			const CharType *str2)
		{
			int len1 = str1.GetLength();
			int len2 = CStringOperTrait<CharType>::strlen(str2);
			int compsize = len1 < len2 ? len1 : len2;
			int r = memcmp(str1, str2, compsize * sizeof(CharType));
			if (r > 0)
				return true;
			if (r == 0)
				return len1 > len2;
			return false;
		}

		friend bool operator > (const CharType *str1,
			const StringT &str2)
		{
			int len1 = CStringOperTrait<CharType>::strlen(str1);
			int len2 = str2.GetLength();
			int compsize = len1 < len2 ? len1 : len2;
			int r = memcmp(str1, str2, compsize * sizeof(CharType));
			if (r > 0)
				return true;
			if (r == 0)
				return len1 > len2;
			return false;
		}

		friend bool operator <= (const StringT &str1,
			const StringT &str2)
		{
			return !(str1 > str2);
		}

		friend bool operator <= (const StringT &str1,
			const CharType *str2)
		{
			return !(str1 > str2);
		}

		friend bool operator <= (const CharType *str1,
			const StringT &str2)
		{
			return !(str1 > str2);
		}

		friend bool operator >= (const StringT &str1,
			const StringT &str2)
		{
			return !(str1 < str2);
		}

		friend bool operator >= (const StringT &str1,
			const CharType *str2)
		{
			return !(str1 < str2);
		}

		friend bool operator >= (const CharType *str1,
			const StringT &str2)
		{
			return !(str1 < str2);
		}
	};

#if defined(_UNICODE) || defined(UNICODE)
	typedef StringT<wchar_t> CString;
#else
	typedef StringT<char> CString;
#endif

#ifdef PLATFORM_WINDOWS
	template struct NOTSTD_API StringQData<wchar_t>;
	template struct NOTSTD_API StringQData<char>;
	template class NOTSTD_API StringT<wchar_t>;
	template class NOTSTD_API StringT<char>;
#endif

	typedef StringT<wchar_t> CStringW;
	typedef StringT<char> CStringA;
}

namespace std {
	template<>
	struct hash<notstd::CStringA>
	{
		size_t operator()(const notstd::CStringA &str) const {
			return std::hash<std::string>()(str.c_str());
		}
	};
}
