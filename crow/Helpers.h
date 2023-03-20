#pragma once

#include <string>

namespace helpers
{
	class String
	{
	public:
		static int countOccurances(char needle, std::string haystack);
	};

	class Char
	{
	public:
		static bool isNumeric(char sign);
		static int castToRealInt(char sign);
	};
}