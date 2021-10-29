

#ifdef MINGW64

#include <stdio.h>
#define Fota_printf printf
#else
#include "asr_crane.h"
#define Fota_printf uart_printf
#include "mbedtls/md.h"
#include "mbedtls/md5.h"
#include "mbedtls/compat-1.3.h" //mbedtls
#endif



#include "fota_typedef.h"
#include "fota_stdlib.h"



#ifndef MD5_DIGESTSIZE
#define MD5_DIGESTSIZE 16
#endif

#ifndef MD5_BLOCKSIZE
#define MD5_BLOCKSIZE 64
#endif
void MD_printf(const char* fmt, ...) 
{

}

#define MD 5
#if MD == 5
#include "fota_hmd5.h"
#endif

#define MDInit MD5Init_adups
#define MDUpdate MD5Update_adups
#define MDFinal MD5Final_adups

/* Constants for MD5Transform routine. */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static void MD5Transform PROTO_LIST ((UINT4 [4], unsigned char [64]));
static void Encode PROTO_LIST
  ((unsigned char *, UINT4 *, unsigned int));
static void Decode PROTO_LIST
  ((UINT4 *, unsigned char *, unsigned int));
static void MD5_memcpy PROTO_LIST ((POINTER, POINTER, unsigned int));
static void MD5_memset PROTO_LIST ((POINTER, int, unsigned int));

static unsigned char PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* F, G, H and I are basic MD5 functions. */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits. */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* 
    FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
   Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
 (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
 (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
 (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
 (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
 (a) = ROTATE_LEFT ((a), (s)); \
 (a) += (b); \
  }




/* MD5 initialization. Begins an MD5 operation, writing a new context. */
void MD5Init_adups (FOTA_MD5_CTX *context)                                        /* context */
{
  context->count[0] = context->count[1] = 0;
  /* Load magic initialization constants.
*/
  context->state[0] = 0x67452301;
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;
}

/* MD5 block update operation. Continues an MD5 message-digest
  operation, processing another message block, and updating the
  context.
 */
void MD5Update_adups (FOTA_MD5_CTX *context, unsigned char *input, unsigned int inputLen)
{
  unsigned int i, index, partLen;

  /* Compute number of bytes mod 64 */
  index = (unsigned int)((context->count[0] >> 3) & 0x3F);

  /* Update number of bits */
  if ((context->count[0] += ((UINT4)inputLen << 3))
   < ((UINT4)inputLen << 3))
 context->count[1]++;
  context->count[1] += ((UINT4)inputLen >> 29);

  partLen = 64 - index;

  /* Transform as many times as possible.
*/
  if (inputLen >= partLen) {
 MD5_memcpy
   ((POINTER)&context->buffer[index], (POINTER)input, partLen);
 MD5Transform (context->state, context->buffer);

 for (i = partLen; i + 63 < inputLen; i += 64)
   MD5Transform (context->state, &input[i]);

 index = 0;
  }
  else
 i = 0;

  /* Buffer remaining input */
  MD5_memcpy
 ((POINTER)&context->buffer[index], (POINTER)&input[i],
  inputLen-i);
}

/* MD5 finalization. Ends an MD5 message-digest operation, writing the
  the message digest and zeroizing the context.
 */
void MD5Final_adups (unsigned char *digest, FOTA_MD5_CTX *context)
{
  unsigned char bits[8];
  unsigned int index, padLen;

  /* Save number of bits */
  Encode (bits, context->count, 8);

  /* Pad out to 56 mod 64.
*/
  index = (unsigned int)((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  MD5Update_adups (context, PADDING, padLen);

  /* Append length (before padding) */
  MD5Update_adups (context, bits, 8);

  /* Store state in digest */
  Encode (digest, context->state, 16);

  /* Zeroize sensitive information.
*/
  MD5_memset ((POINTER)context, 0, sizeof (*context));
}

/* MD5 basic transformation. Transforms state based on block.
 */
static void MD5Transform (UINT4 state[4], unsigned char block[64])
{
  UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

  Decode (x, block, 64);

  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

 /* Round 2 */
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

  /* Round 4 */
  II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

  /* Zeroize sensitive information.
   */
  MD5_memset ((POINTER)x, 0, sizeof (x));
}

/* Encodes input (UINT4) into output (unsigned char). Assumes len is
  a multiple of 4.
 */
static void Encode (unsigned char *output, UINT4 *input, unsigned int len)
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4) {
 output[j] = (unsigned char)(input[i] & 0xff);
 output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
 output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
 output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
  }
}

/* Decodes input (unsigned char) into output (UINT4). Assumes len is
  a multiple of 4.
 */
static void Decode (UINT4 *output, unsigned char *input, unsigned int len)
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
 output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
   (((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
}

/* Note: Replace "for loop" with standard memcpy if possible.
 */

static void MD5_memcpy (POINTER output, POINTER input, unsigned int len)
{
  unsigned int i;

  for (i = 0; i < len; i++)
    output[i] = input[i];
}

/* Note: Replace "for loop" with standard memset if possible.
 */
static void MD5_memset (POINTER output, int value, unsigned int len)
{
  unsigned int i;

  for (i = 0; i < len; i++)
 ((char *)output)[i] = (char)value;
}




//////////////////////////////////////////////

/* Length of test block, number of test blocks.
 */
#define TEST_BLOCK_LEN 1000
#define TEST_BLOCK_COUNT 1000

static void MDPrint PROTO_LIST ((unsigned char [16]));




/* Prints a message digest in hexadecimal.
 */
static void MDPrint (unsigned char * digest)
{

  unsigned int i;
  MD_printf("MDPrint\r\n");
  for (i = 0; i < 16; i++)
  MD_printf("%d\r\n", digest[i]);
}
static void truncate
(
	char* d1, /* data to be truncated */
	char* d2, /* truncated data */
	int len /* length in bytes to keep */
)
{
	int i ;
	for (i = 0 ; i < len ; i++) 
		d2[i] = d1[i];
}

/* Function to compute the digest */
static void fota_hmac_md5
(
	char* k, /* secret key */
	int lk, /* length of the key in bytes */
	char* d, /* data */
	int ld, /* length of data in bytes */
	char* out, /* output buffer, at least "t" bytes */
	int t
)
{
	FOTA_MD5_CTX ictx, octx ;
	char imd[MD5_DIGESTSIZE], omd[MD5_DIGESTSIZE] ;
	char key[MD5_DIGESTSIZE] ;
	char buf[MD5_BLOCKSIZE] ;
	int i ;

	if (lk > MD5_BLOCKSIZE) {

	FOTA_MD5_CTX tctx ;

	MDInit(&tctx) ;
	MDUpdate(&tctx, (unsigned char *)k, lk) ;
	MDFinal((unsigned char *)key, &tctx) ;

	k = key ;
	lk = MD5_DIGESTSIZE ;
}

/**** Inner Digest ****/

	MDInit(&ictx) ;

	/* Pad the key for inner digest */
	for (i = 0 ; i < lk ; ++i) buf[i] = k[i] ^ 0x36 ;
	for (i = lk ; i < MD5_BLOCKSIZE ; ++i) buf[i] = 0x36 ;

	MDUpdate(&ictx, (unsigned char *)buf, MD5_BLOCKSIZE) ;
	MDUpdate(&ictx, (unsigned char *)d, ld) ;

	MDFinal((unsigned char *)imd, &ictx) ;

/**** Outter Digest ****/

	MDInit(&octx) ;

/* Pad the key for outter digest */

	for (i = 0 ; i < lk ; ++i) buf[i] = k[i] ^ 0x5C ;
	for (i = lk ; i < MD5_BLOCKSIZE ; ++i) buf[i] = 0x5C ;

	MDUpdate(&octx, (unsigned char *)buf, MD5_BLOCKSIZE) ;
	MDUpdate(&octx, (unsigned char *)imd, MD5_DIGESTSIZE) ;

	MDFinal((unsigned char *)omd, &octx) ;

/* truncate and print the results */
	t = t > MD5_DIGESTSIZE ? MD5_DIGESTSIZE : t ;
	truncate(omd, out, t) ;
	MDPrint((unsigned char *)out);
	Fota_printf("\n");
	//pr_md(stdout, out, t) ;

}



static fota_char fota_hmd5_value[FOTA_HMD5_LEN]={0};
int fota_hmd5_calc(char *key, int keylen, char *data, int datalen)
{
	//char out[32];

	fota_memset(fota_hmd5_value, 0, FOTA_HMD5_LEN);

	fota_hmac_md5(key,keylen,data,datalen,fota_hmd5_value,FOTA_HMD5_LEN);
	return 0;
}


char * fota_hmd5_pid_psec_mid(char *deviceId, char *deviceSecret,char *mid)
{
	char k[128],d[128],out[16],tmp_str[10];
	int lk=0,ld=0;	
	int index;

	Fota_printf("%s-01:mid=%s,pid=%s,psec=%s\r\n",__func__, mid,deviceId,deviceSecret);
	fota_memset(fota_hmd5_value, 0, FOTA_HMD5_LEN);
	fota_memset(k, 0, 128);
	fota_memset(d, 0, 128);


	fota_strcpy(d, deviceId);//what do ya want for nothing?
	fota_strcpy(d+fota_strlen(d), deviceSecret);//what do ya want for nothing?
	fota_strcpy(k, mid);//Jefe

	lk=fota_strlen(k);
	ld=fota_strlen(d);
	fota_hmac_md5(k,lk,d,ld,out,16);
	for(index=0;index<16;index++)
	{
    Fota_printf("int:%02x",out[index]);
		fota_sprintf(tmp_str,"%02x",out[index]);
    Fota_printf("str:%s",tmp_str);
		fota_strncpy(fota_hmd5_value+(index*2),tmp_str,2);
	}
	Fota_printf("\r\n %s-02:fota_hmd5_value=%s,len=%d\r\n",__func__,fota_hmd5_value,fota_strlen(fota_hmd5_value));
	return (char *)fota_hmd5_value;
	
}

char *fota_hmd5_mid_pid_psecret(char *mid, char *productId, char *productSecret,fota_uint32 fota_time)
{
	char k[128],d[128],out[16],time_str[20],tmp_str[10];
	int lk=0,ld=0;
	int index;

	//Fota_printf("fota_hmd5_mid_pid_psecret:mid=%s,pid=%s,psec=%s\n", mid,productId,productSecret);
	Fota_printf("mid=%s,productId=%s,productSecret=%s,fota_time=%d\r\n",mid,productId,productSecret,fota_time);
	fota_memset(fota_hmd5_value, 0, FOTA_HMD5_LEN);
	fota_memset(k, 0, 128);
	fota_memset(d, 0, 128);
	fota_memset(time_str,0,20);
	fota_itoa(fota_time, time_str, 10);	

	fota_strcpy(d, mid);//what do ya want for nothing?
	fota_strcpy(d+fota_strlen(d), productId);//what do ya want for nothing?
	fota_strcpy(d+fota_strlen(d),time_str);
	fota_strcpy(k, productSecret);//Jefe


	lk=fota_strlen(k);
	ld=fota_strlen(d);
	fota_hmac_md5(k,lk,d,ld,out,16);
	for(index=0;index<16;index++)
	{
		//Fota_printf("int:%d",out[index]);
		fota_sprintf(tmp_str,"%02x",(unsigned char)out[index]);
		//Fota_printf("str:%s",tmp_str);
		fota_strncpy(fota_hmd5_value+(index*2),tmp_str,2);
	}
	Fota_printf("%s:fota_hmd5_value=%s,len=%d\r\n",__func__, fota_hmd5_value,fota_strlen(fota_hmd5_value));
	return (char *)fota_hmd5_value;
}

char *fota_hmd5_did_pid_psecret(char *deviceId, char *productId, char *productSecret,fota_uint32 fota_time)
{
	char k[128],d[128],out[16],time_str[20],tmp_str[10];
	int lk=0,ld=0;
	int index;

	//Fota_printf("fota_hmd5_did_pid_psecret:did=%s,pid=%s,psec=%s\r\n", deviceId,productId,productSecret);
	fota_memset(fota_hmd5_value, 0, FOTA_HMD5_LEN);
	fota_memset(time_str,0,20);
	fota_memset(k, 0, 128);
	fota_memset(d, 0, 128);
	fota_itoa(fota_time, time_str, 10);

	fota_strcpy(d, deviceId);//what do ya want for nothing?
	fota_strcpy(d+fota_strlen(d), productId);//what do ya want for nothing?
	fota_strcpy(d+fota_strlen(d),time_str);	
	fota_strcpy(k, productSecret);//Jefe

	lk=fota_strlen(k);
	ld=fota_strlen(d);
	fota_hmac_md5(k,lk,d,ld,out,16);
	for(index=0;index<16;index++)
	{
		//Fota_printf("int:%d",out[index]);
		sprintf(tmp_str,"%02x",(unsigned char)out[index]);
		//Fota_printf("str:%s",tmp_str);
		fota_strncpy(fota_hmd5_value+(index*2),tmp_str,2);
	}
	//Fota_printf("fota_hmd5_did_pid_psecret:fota_hmd5_value=%s,len=%d\r\n", fota_hmd5_value,fota_strlen(fota_hmd5_value));
	return fota_hmd5_value;
}
