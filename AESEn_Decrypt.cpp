#include "stdafx.h"
#include "AESEn_Decrypt.h"
#include "aes.h"
#include "base64.h"
#include "modes.h"

//统一的aes加解密算法
static byte AES_KEY[] = "1234567890000000";
static byte AES_VI[] = "1234567890000000";
using namespace CryptoPP;

std::string AES_EnDeCrypt::Encode(const char* src, const int &length)
{
	if (src == NULL || length == 0)
	{
		return "";
	}
	std::string strencodeed = aesencode(src,length,AES_KEY, AES_VI);
	return base64encode(strencodeed);
}

std::string AES_EnDeCrypt::Decode(const char* src, const int &length)
{
	if (src == NULL || length == 0)
	{
		return "";
	}
	std::string strdecodeed = base64decode(src);
	return aesdecode(strdecodeed.c_str(), strdecodeed.length(), AES_KEY, AES_VI);
}

std::string AES_EnDeCrypt::base64encode(const std::string& src)
{
	//byte decoded[] = { 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00 };
	std::string encoded;

	try
	{
		StringSource ss(src, true,
			new Base64Encoder(
				new StringSink(encoded), false
			) // Base64Encoder
		); // StringSource

		return encoded;
	}
	catch(...)
	{ }

	return encoded;
}

std::string AES_EnDeCrypt::base64decode(const std::string& src)
{
	std::string decoded;

	try
	{
		StringSource ss(src, true,
			new Base64Decoder(
				new StringSink(decoded)
			) // Base64Decoder
		); // StringSource

		return decoded;
	}
	catch (...)
	{

	}
	return decoded;

}

std::string AES_EnDeCrypt::aesencode(const char* src, const int &length, const byte* key, const byte* iv)
{
	std::string cipherText;

	//  
	try
	{
		AES::Encryption aesEncryption(key, 16);
		CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);
		StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink(cipherText));
		stfEncryptor.Put(reinterpret_cast<const unsigned char*>(src), length);
		stfEncryptor.MessageEnd();

		return cipherText;
	}
	catch (...)
	{
	}
	return cipherText;
}

std::string AES_EnDeCrypt::aesdecode(const char* src, const int &length, const byte* key, const byte* iv)
{
	std::string recover;
	//CBC_Mode<AES>::Decryption aesDecryptor(key, key_length, iv);
	try
	{
		AES::Decryption aesDecryption(key,16);
		CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);
		StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(recover));
		stfDecryptor.Put(reinterpret_cast<const unsigned char*>(src), length);
		stfDecryptor.MessageEnd();

		return recover;
	}
	catch (...)
	{
	}
	return recover;
}




