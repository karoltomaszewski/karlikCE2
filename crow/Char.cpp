#include "Helpers.h"

bool helpers::Char::isNumeric(char sign)
{
	return (int)sign > 47 && (int)sign < 58;
}

int helpers::Char::castToRealInt(char sign)
{
	return (int)sign - 48;
}
