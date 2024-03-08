#pragma once

#include <afxstr.h>

#include <string>
#include <format>

template<>
struct std::formatter<CString, char> : public std::formatter<std::string_view, char>
{
	template<class FmtContext>
	FmtContext::iterator format(const CString& s, FmtContext& ctx) const
	{
		return std::formatter<std::string_view, char>::format(s.GetString(), ctx);
	}
};