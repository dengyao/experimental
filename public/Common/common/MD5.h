#ifndef __MD5_H__
#define __MD5_H__

#include <string>

class MD5
{
public:
	static std::string Encrypt(const std::string &input);
};

#endif
