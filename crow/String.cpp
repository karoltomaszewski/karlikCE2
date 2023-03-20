#include "Helpers.h"

int helpers::String::countOccurances(char needle, std::string haystack)
{
	return std::count(haystack.begin(), haystack.end(), needle);
}
