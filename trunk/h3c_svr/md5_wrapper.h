#pragma once

#include <Windows.h>

typedef struct {  
	ULONG i[2];
	ULONG buf[4];
	unsigned char in[64];
	unsigned char digest[16];
} MD5_CTX;

class md5_wrapper
{
public:
	md5_wrapper();
	~md5_wrapper();
	void Update ( unsigned char *input, unsigned int inputLen);
	void Final (unsigned char digest[16]);

private:
	MD5_CTX ctx_;
};
