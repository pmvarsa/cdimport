
#include <sstream>

#include "utility.h"

std::string Utility::readableLength(int l)
{
	int s = l;
	int h = 0;
	if(l >= 3600) {
		h = l / 3600;
		s = l % 3600;
	}
	int m = s / 60;
	s = s % 60;

	std::stringstream ss;
	if((h == 1 and m > 39) or h > 1) {
		ss << h << "h" << m << "m" << s << "s";
	} else {
		ss << (m + h * 60) << "m" << s << "s";
	}
	return ss.str();
}

