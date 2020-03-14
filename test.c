#include<openssl/bn.h>
#include<stdio.h>
#include<openssl/rsa.h>
#include<openssl/pem.h>
// #include<openssl/applink.h>
#include<openssl/evp.h>
int main(void){
 // 	BIGNUM  static_bin,*dynamic_bin;
	// BN_init(&static_bn);
	// dynamic_bin=BN_new();
	// BN_free(dynamic_bin);
	// BN_free(&static_bn);
	// printf("is it ok");
	// return 0;
	BIGNUM  *dynamic_bin;
	BN_init(dynamic_bin);
	// dynamic_bin=BN_new();
	BN_free(dynamic_bin);
	// BN_free(&static_bn);
	printf("is it ok");
	return 0;
}