#pragma once

#include <string>
typedef unsigned char byte;		// put in global namespace to avoid ambiguity with other byte typedefs
class AES_EnDeCrypt
{
public:
	static std::string Encode(const char* ciper,const int &length);  //aes加密后base64转码
	static std::string Decode(const char* ciper, const int &length);  //base64解码后aes解密
	static std::string base64encode(const std::string& src);
	static std::string base64decode(const std::string& src);
	static std::string aesencode(const char* src, const int &length,  const byte* key, const byte* iv);
	static std::string aesdecode(const char* src, const int &length,  const byte* key, const byte* iv);
	
};