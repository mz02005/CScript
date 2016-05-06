#pragma once

#if !defined(NOTSTD_STRINGHELPER_H)
#define NOTSTD_STRINGHELPER_H

#include "Config.h"
#include <d2d1.h>
#include <assert.h>
#if !defined(PLATFORM_WINDOWS)
#include <stdarg.h>
#include <wchar.h>
#endif

class StringHelper
{
public:
	typedef std::vector<std::string> StringArray;

	struct ReadLineContext
	{
		char mBuf[4096];
		const char *mCur, *mEnd;

		ReadLineContext()
			: mCur(mBuf)
			, mEnd(mCur)
		{
		}
	};

	// 读取一行，撇掉每行末尾的回车换行符，file以二进制读方式打开的
	static bool ReadLine(FILE *file, std::string &line, ReadLineContext *context)
	{
		if (!context) {
			ReadLineContext rlc;
			if (!ReadLine(file, line, &rlc))
				return false;
			if (rlc.mCur != rlc.mEnd)
				fseek(file, -static_cast<long>(rlc.mEnd - rlc.mCur), SEEK_CUR);
			return true;
		}

		line.clear();

		const char *&p = context->mCur, *&e = context->mEnd;
		while (1)
		{
			for (; p != e;)
			{
				if (*p != '\r' && *p != '\n')
					line += *p++;
				else
				{
					while (++p != e && *p == '\n')
					{
					}

					return true;
				}
			}
			size_t readed = fread(context->mBuf, 1, 4096, file);
			p = context->mBuf;
			if (readed)
				e = context->mBuf + readed;
			else
			{
				return !line.empty();
			}
		}

		return true;
	}

	static std::string Right(const std::string &s, std::size_t count)
	{
		if (count >= s.size())
			return s;

		return s.substr(s.size() - count);
	}

	static std::string Left(const std::string &s, std::size_t count)
	{
		if (count >= s.size())
			return s;

		return s.substr(0, count);
	}

	static std::string Mid(const std::string &s, std::size_t start, std::size_t count = std::string::npos)
	{
		if (start >= s.size())
			return "";
		std::size_t s1 = s.size() - start;
		std::size_t theSize = s1 > count ? count : s1;
		return s.substr(start, theSize);
	}
	
	static StringArray StringHelper::SplitString(const std::string &str, const std::string &splitChars)
	{
		StringArray r;
	
		if (!str.size())
			return r;
	
		r.push_back("");
		for (const std::string::value_type *p = str.c_str(); *p != '\0'; )
		{
			if (splitChars.find(*p) != std::string::npos)
			{
				while (*++p != '\0' && splitChars.find(*p) != std::string::npos)
				{}
				if (*p != '\0') {
					r.push_back("");
					r.back() += *p++;
				}
			}
			else
			{
				r.back() += *p;
				p ++;
			}
		}
		return r;
	}

	static void StringHelper::Trim(std::string &str, const std::string::value_type c)
	{
		str.erase(0, str.find_first_not_of(c));
		str.erase(str.find_last_not_of(c) + 1);
	}

	static void StringHelper::Trim(std::string &str, const std::string &splitChars)
	{
		str.erase(0, str.find_first_not_of(splitChars));
		str.erase(str.find_last_not_of(splitChars) + 1);
	}

	static std::string& StringHelper::FormatV(std::string &str, const char *szFormat, va_list valist)
	{
		str.clear();

		std::size_t size = 64;
		str.resize(size);
		int n = vsnprintf(&str[0], size - 1, szFormat, valist);
		do {
#ifdef PLATFORM_WINDOWS
			while (n == -1)
			{
				if (size > 1024)
					size += 1024;
				else
					size *= 2;
				str.resize(size);
				n = vsnprintf(&str[0], size - 1, szFormat, valist);
			}
#else
			if (n < 0) {
				str.resize(0);
				return str;
			}
			if (n >= size)
			{
				str.resize(n + 1);
				n = vsnprintf(&str[0], n, szFormat, valist);
			}
#endif
			str.resize(n);
		} while (0);
		return str;
	}

	static std::wstring& StringHelper::Format(std::wstring &str, const wchar_t *szFormat, ...)
	{
		va_list argList;
		va_start(argList, szFormat);

		str.clear();

		int size = 64;
		wchar_t *buffer = reinterpret_cast<wchar_t*>(malloc(size * sizeof(wchar_t)));
		if (!buffer)
			return str;

		int n = _vsnwprintf(buffer, size - 1, szFormat, argList);
		do {
#if defined(PLATFORM_WINDOWS)
			while (n == -1)
			{
				if (size > 1024)
					size += 1024;
				else
					size *= 2;
				buffer = reinterpret_cast<wchar_t*>(realloc(buffer, size * sizeof(wchar_t)));
				if (!buffer)
					return str;
				n = _vsnwprintf(buffer, size - 1, szFormat, argList);
			}
#else
			if (n < 0)
				return str;
			if (n >= size)
			{
				size = n * 2;
				buffer = reinterpret_cast<char*>(realloc(size * sizeof(wchar_t)));
				n = vsnwprintf(buffer, size - 1, szFormat, argList);
			}
#endif
		} while (0);
		va_end(argList);
		if (buffer)
		{
			buffer[n] = 0;
			str = buffer;
			free(buffer);
		}
		return str;
	}

	static std::string& StringHelper::Format(std::string &str, const char *szFormat, ...)
	{
		va_list argList;
		va_start(argList,szFormat);

		str.clear();

		int size = 64;
		char *buffer = reinterpret_cast<char*>(malloc(size));
		if (!buffer)
			return str;

		int n = vsnprintf(buffer, size - 1, szFormat, argList);
		do {
#if defined(PLATFORM_WINDOWS)
			while (n == -1)
			{
				if (size > 1024)
					size += 1024;
				else
					size *= 2;
				buffer = reinterpret_cast<char*>(realloc(buffer, size));
				if (!buffer)
					return str;
				n = vsnprintf(buffer, size - 1, szFormat, argList);
			}
#else
			if (n < 0)
				return str;
			if (n >= size)
			{
				size = n * 2;
				buffer = reinterpret_cast<char*>(realloc(size));
				n = vsnprintf(buffer, size - 1, szFormat, argList);
			}
#endif
		} while (0);
		va_end(argList);
		if (buffer)
		{
			buffer[n] = 0;
			str = buffer;
			free(buffer);
		}
		return str;
	}

	static std::string& StringHelper::AppendFormat(std::string &str, const char *szFormat, ...)
	{
		std::string r;
		va_list argList;
		va_start(argList, szFormat);
		str += FormatV(r, szFormat, argList);
		va_end(argList);
		return str;
	}

	// 返回替换的个数
	static int Replace(std::string &s, const std::string &pattern, const std::string &newpat, bool replaceAll = true, std::string::size_type *startPos = NULL)
	{
		int r = 0;

		std::string::size_type pattenSize = pattern.size();
		std::string::size_type newpatSize = newpat.size();
		
		std::string::size_type p = startPos ? *startPos : 0, f;
		while ((f = s.find(pattern, p)) != s.npos)
		{
			r++;
			s.replace(f, pattenSize, newpat);
			p = f + newpatSize;

			if (startPos)
				*startPos = f;

			if (!replaceAll)
				break;
		}

		return r;
	}

	static int Replace(std::wstring &s, const std::wstring &pattern, const std::wstring &newpat, bool replaceAll = true, std::string::size_type *startPos = NULL)
	{
		int r = 0;

		std::wstring::size_type pattenSize = pattern.size();
		std::wstring::size_type newpatSize = newpat.size();

		std::wstring::size_type p = startPos ? *startPos : 0, f;
		while ((f = s.find(pattern, p)) != s.npos)
		{
			r++;
			s.replace(f, pattenSize, newpat);
			p = f + newpatSize;

			if (startPos)
				*startPos = f;

			if (!replaceAll)
				break;
		}

		return r;
	}
};

class ToStringHelper {
public:
	static std::string ToString(uint32_t v)
	{
		std::string r;
		StringHelper::Format(r, "%u", v);
		return r;
	}

	static std::string ToString(uint64_t v)
	{
		std::string r;
		StringHelper::Format(r, "%llu", v);
		return r;
	}

	static std::string ToString(int32_t v)
	{
		std::string r;
		StringHelper::Format(r, "%d", v);
		return r;
	}

	static std::string ToString(int64_t v)
	{
		std::string r;
		StringHelper::Format(r, "%lld", v);
		return r;
	}

	static std::string ToString(float v)
	{
		std::string r;
		StringHelper::Format(r, "%.6f", v);
		return r;
	}

	static std::string ToString(double v)
	{
		std::string r;
		StringHelper::Format(r, "%.6f", v);
		return r;
	}

	static std::string ToString(const D2D1_POINT_2F &v)
	{
		std::string r;
		return StringHelper::Format(r, "%f,%f", v.x, v.y);
	}

	static std::string ToString(const D2D1_COLOR_F &v)
	{
		std::string r;
		return StringHelper::Format(r, "%f,%f,%f,%f", v.r, v.g, v.b, v.a);
	}

	static std::string ToString(const D2D1_RECT_F &v)
	{
		std::string s;
		return StringHelper::Format(s, "%f,%f,%f,%f", v.left, v.top, v.right, v.bottom);
	}

	static std::string ToString(const std::string &v)
	{
		return v;
	}

	static std::string ToString(const DWORD &v)
	{
		std::string r;
		return StringHelper::Format(r, "%lu", v);
	}

	static std::string ToString(const POINT &v)
	{
		std::string r;
		return StringHelper::Format(r, "%d,%d", v.x, v.y);
	}
};

#define TS(v) (ToStringHelper::ToString(v))

#endif
