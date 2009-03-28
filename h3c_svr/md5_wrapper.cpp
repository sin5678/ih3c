
#include "md5_wrapper.h"

extern "C"
{
	void DECLSPEC_IMPORT WINAPI MD5Init(
		MD5_CTX* context
		);


	void DECLSPEC_IMPORT WINAPI MD5Update(
		MD5_CTX* context,
		unsigned char* input,
		unsigned int inlen
		);


	void DECLSPEC_IMPORT WINAPI MD5Final(
		MD5_CTX* context
		);
};


#pragma comment (lib, "cryptdll.lib")

md5_wrapper::md5_wrapper()
{
	::MD5Init(&ctx_);
}

md5_wrapper::~md5_wrapper()
{

}

void md5_wrapper::Update( unsigned char *input, unsigned int inputLen )
{
	::MD5Update(&ctx_, input, inputLen);
}

void md5_wrapper::Final( unsigned char digest[16] )
{
	::MD5Final(&ctx_);
	memcpy (digest, ctx_.digest, sizeof (ctx_.digest));
}