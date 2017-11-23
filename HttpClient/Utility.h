#pragma once
#include <string>
#include <windows.h>

namespace utility
{

	inline std::wstring UTF8_To_UTF16(const std::string &source)
	{
		unsigned long len = ::MultiByteToWideChar(CP_UTF8, NULL, source.c_str(), -1, NULL, NULL);
		if (len == 0)
			return std::wstring();
		wchar_t *buffer = new wchar_t[len];
		::MultiByteToWideChar(CP_UTF8, NULL, source.c_str(), -1, buffer, len);

		std::wstring dest(buffer);
		delete[] buffer;
		return dest;
	}

	inline std::string UTF16_To_UTF8(const std::wstring &source)
	{
		unsigned long len = ::WideCharToMultiByte(CP_UTF8, NULL, source.c_str(), -1, NULL, NULL, NULL, NULL);
		if (len == 0)
			return std::string();
		char *buffer = new char[len];
		::WideCharToMultiByte(CP_UTF8, NULL, source.c_str(), -1, buffer, len, NULL, NULL);

		std::string dest(buffer);
		delete[] buffer;
		return dest;
	}


	inline std::wstring GBK_To_UTF16(const std::string &source)
	{
		enum { GB2312 = 936 };

		unsigned long len = ::MultiByteToWideChar(GB2312, NULL, source.c_str(), -1, NULL, NULL);
		if (len == 0)
			return std::wstring();
		wchar_t *buffer = new wchar_t[len];
		::MultiByteToWideChar(GB2312, NULL, source.c_str(), -1, buffer, len);

		std::wstring dest(buffer);
		delete[] buffer;
		return dest;
	}

	inline std::string UTF16_To_GBK(const std::wstring &source)
	{
		enum { GB2312 = 936 };

		unsigned long len = ::WideCharToMultiByte(GB2312, NULL, source.c_str(), -1, NULL, NULL, NULL, NULL);
		if (len == 0)
			return std::string();
		char *buffer = new char[len];
		::WideCharToMultiByte(GB2312, NULL, source.c_str(), -1, buffer, len, NULL, NULL);

		std::string dest(buffer);
		delete[] buffer;
		return dest;
	}

	inline std::string GBK_To_UTF8(const std::string &source)
	{
		enum { GB2312 = 936 };

		unsigned long len = ::MultiByteToWideChar(GB2312, NULL, source.c_str(), -1, NULL, NULL);
		if (len == 0)
			return std::string();
		wchar_t *wide_char_buffer = new wchar_t[len];
		::MultiByteToWideChar(GB2312, NULL, source.c_str(), -1, wide_char_buffer, len);

		len = ::WideCharToMultiByte(CP_UTF8, NULL, wide_char_buffer, -1, NULL, NULL, NULL, NULL);
		if (len == 0)
		{
			delete[] wide_char_buffer;
			return std::string();
		}
		char *multi_byte_buffer = new char[len];
		::WideCharToMultiByte(CP_UTF8, NULL, wide_char_buffer, -1, multi_byte_buffer, len, NULL, NULL);

		std::string dest(multi_byte_buffer);
		delete[] wide_char_buffer;
		delete[] multi_byte_buffer;
		return dest;
	}

	inline std::string UTF8_To_GBK(const std::string &source)
	{
		enum { GB2312 = 936 };

		unsigned long len = ::MultiByteToWideChar(CP_UTF8, NULL, source.c_str(), -1, NULL, NULL);
		if (len == 0)
			return std::string();
		wchar_t *wide_char_buffer = new wchar_t[len];
		::MultiByteToWideChar(CP_UTF8, NULL, source.c_str(), -1, wide_char_buffer, len);

		len = ::WideCharToMultiByte(GB2312, NULL, wide_char_buffer, -1, NULL, NULL, NULL, NULL);
		if (len == 0)
		{
			delete[] wide_char_buffer;
			return std::string();
		}
		char *multi_byte_buffer = new char[len];
		::WideCharToMultiByte(GB2312, NULL, wide_char_buffer, -1, multi_byte_buffer, len, NULL, NULL);

		std::string dest(multi_byte_buffer);
		delete[] wide_char_buffer;
		delete[] multi_byte_buffer;
		return dest;
	}
}