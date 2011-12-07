/* crypto/md5/md5.h */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#ifndef HEADER_MD5_H
#define HEADER_MD5_H

#include <stddef.h>

#ifndef MD32_REG_T
#define MD32_REG_T long
/*
 * This comment was originaly written for MD5, which is why it
 * discusses A-D. But it basically applies to all 32-bit digests,
 * which is why it was moved to common header file.
 *
 * In case you wonder why A-D are declared as long and not
 * as MD5_LONG. Doing so results in slight performance
 * boost on LP64 architectures. The catch is we don't
 * really care if 32 MSBs of a 64-bit register get polluted
 * with eventual overflows as we *save* only 32 LSBs in
 * *either* case. Now declaring 'em long excuses the compiler
 * from keeping 32 MSBs zeroed resulting in 13% performance
 * improvement under SPARC Solaris7/64 and 5% under AlphaLinux.
 * Well, to be honest it should say that this *prevents* 
 * performance degradation.
 *				<appro@fy.chalmers.se>
 * Apparently there're LP64 compilers that generate better
 * code if A-D are declared int. Most notably GCC-x86_64
 * generates better code.
 *				<appro@fy.chalmers.se>
 */
#endif


#define DATA_ORDER_IS_BIG_ENDIAN

/*
 * Engage compiler specific rotate intrinsic function if available.
 */
#undef ROTATE
#ifndef PEDANTIC
# if defined(_MSC_VER) || defined(__ICC)
#  define ROTATE(a,n)	_lrotl(a,n)
# elif defined(__MWERKS__)
#  if defined(__POWERPC__)
#   define ROTATE(a,n)	__rlwinm(a,n,0,31)
#  elif defined(__MC68K__)
    /* Motorola specific tweak. <appro@fy.chalmers.se> */
#   define ROTATE(a,n)	( n<24 ? __rol(a,n) : __ror(a,32-n) )
#  else
#   define ROTATE(a,n)	__rol(a,n)
#  endif
# elif defined(__GNUC__) && __GNUC__>=2 && !defined(OPENSSL_NO_ASM) && !defined(OPENSSL_NO_INLINE_ASM)
  /*
   * Some GNU C inline assembler templates. Note that these are
   * rotates by *constant* number of bits! But that's exactly
   * what we need here...
   * 					<appro@fy.chalmers.se>
   */
#  if defined(__i386) || defined(__i386__) || defined(__x86_64) || defined(__x86_64__)
#   define ROTATE(a,n)	({ register unsigned int ret;	\
				asm (			\
				"roll %1,%0"		\
				: "=r"(ret)		\
				: "I"(n), "0"(a)	\
				: "cc");		\
			   ret;				\
			})
#  elif defined(_ARCH_PPC) || defined(_ARCH_PPC64) || \
	defined(__powerpc) || defined(__ppc__) || defined(__powerpc64__)
#   define ROTATE(a,n)	({ register unsigned int ret;	\
				asm (			\
				"rlwinm %0,%1,%2,0,31"	\
				: "=r"(ret)		\
				: "r"(a), "I"(n));	\
			   ret;				\
			})
#  elif defined(__s390x__)
#   define ROTATE(a,n) ({ register unsigned int ret;	\
				asm ("rll %0,%1,%2"	\
				: "=r"(ret)		\
				: "r"(a), "I"(n));	\
			  ret;				\
			})
#  endif
# endif
#endif /* PEDANTIC */

#ifndef ROTATE
#define ROTATE(a,n)     (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))
#endif

#if defined(DATA_ORDER_IS_BIG_ENDIAN)

#ifndef PEDANTIC
# if defined(__GNUC__) && __GNUC__>=2 && !defined(OPENSSL_NO_ASM) && !defined(OPENSSL_NO_INLINE_ASM)
#  if ((defined(__i386) || defined(__i386__)) && !defined(I386_ONLY)) || \
      (defined(__x86_64) || defined(__x86_64__))
#   if !defined(B_ENDIAN)
    /*
     * This gives ~30-40% performance improvement in SHA-256 compiled
     * with gcc [on P4]. Well, first macro to be frank. We can pull
     * this trick on x86* platforms only, because these CPUs can fetch
     * unaligned data without raising an exception.
     */
#   define HOST_c2l(c,l)	({ unsigned int r=*((const unsigned int *)(c));	\
				   asm ("bswapl %0":"=r"(r):"0"(r));	\
				   (c)+=4; (l)=r;			})
#   define HOST_l2c(l,c)	({ unsigned int r=(l);			\
				   asm ("bswapl %0":"=r"(r):"0"(r));	\
				   *((unsigned int *)(c))=r; (c)+=4; r;	})
#   endif
#  endif
# endif
#endif
#if defined(__s390__) || defined(__s390x__)
# define HOST_c2l(c,l) ((l)=*((const unsigned int *)(c)), (c)+=4, (l))
# define HOST_l2c(l,c) (*((unsigned int *)(c))=(l), (c)+=4, (l))
#endif

#ifndef HOST_c2l
#define HOST_c2l(c,l)	(l =(((unsigned long)(*((c)++)))<<24),		\
			 l|=(((unsigned long)(*((c)++)))<<16),		\
			 l|=(((unsigned long)(*((c)++)))<< 8),		\
			 l|=(((unsigned long)(*((c)++)))    ),		\
			 l)
#endif
#ifndef HOST_l2c
#define HOST_l2c(l,c)	(*((c)++)=(unsigned char)(((l)>>24)&0xff),	\
			 *((c)++)=(unsigned char)(((l)>>16)&0xff),	\
			 *((c)++)=(unsigned char)(((l)>> 8)&0xff),	\
			 *((c)++)=(unsigned char)(((l)    )&0xff),	\
			 l)
#endif

#elif defined(DATA_ORDER_IS_LITTLE_ENDIAN)

#ifndef PEDANTIC
# if defined(__GNUC__) && __GNUC__>=2 && !defined(OPENSSL_NO_ASM) && !defined(OPENSSL_NO_INLINE_ASM)
#  if defined(__s390x__)
#   define HOST_c2l(c,l)	({ asm ("lrv	%0,0(%1)"		\
					:"=r"(l) : "r"(c));		\
				   (c)+=4; (l);				})
#   define HOST_l2c(l,c)	({ asm ("strv	%0,0(%1)"		\
					: : "r"(l),"r"(c) : "memory");	\
				   (c)+=4; (l);				})
#  endif
# endif
#endif
#if defined(__i386) || defined(__i386__) || defined(__x86_64) || defined(__x86_64__)
# ifndef B_ENDIAN
   /* See comment in DATA_ORDER_IS_BIG_ENDIAN section. */
#  define HOST_c2l(c,l)	((l)=*((const unsigned int *)(c)), (c)+=4, l)
#  define HOST_l2c(l,c)	(*((unsigned int *)(c))=(l), (c)+=4, l)
# endif
#endif

#ifndef HOST_c2l
#define HOST_c2l(c,l)	(l =(((unsigned long)(*((c)++)))    ),		\
			 l|=(((unsigned long)(*((c)++)))<< 8),		\
			 l|=(((unsigned long)(*((c)++)))<<16),		\
			 l|=(((unsigned long)(*((c)++)))<<24),		\
			 l)
#endif
#ifndef HOST_l2c
#define HOST_l2c(l,c)	(*((c)++)=(unsigned char)(((l)    )&0xff),	\
			 *((c)++)=(unsigned char)(((l)>> 8)&0xff),	\
			 *((c)++)=(unsigned char)(((l)>>16)&0xff),	\
			 *((c)++)=(unsigned char)(((l)>>24)&0xff),	\
			 l)
#endif

#endif


#ifdef  __cplusplus
extern "C" {
#endif

#ifdef OPENSSL_NO_MD5
#error MD5 is disabled.
#endif

/*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * ! MD5_LONG has to be at least 32 bits wide. If it's wider, then !
 * ! MD5_LONG_LOG2 has to be defined along.			   !
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

#if defined(__LP32__)
#define MD5_LONG unsigned long
#elif defined(OPENSSL_SYS_CRAY) || defined(__ILP64__)
#define MD5_LONG unsigned long
#define MD5_LONG_LOG2 3
/*
 * _CRAY note. I could declare short, but I have no idea what impact
 * does it have on performance on none-T3E machines. I could declare
 * int, but at least on C90 sizeof(int) can be chosen at compile time.
 * So I've chosen long...
 *					<appro@fy.chalmers.se>
 */
#else
#define MD5_LONG unsigned int
#endif

#define MD5_CBLOCK	64
#define MD5_LBLOCK	(MD5_CBLOCK/4)
#define MD5_DIGEST_LENGTH 16

typedef struct MD5state_st
	{
	MD5_LONG A,B,C,D;
	MD5_LONG Nl,Nh;
	MD5_LONG data[MD5_LBLOCK];
	unsigned int num;
	} MD5_CTX;

int MD5_Init(MD5_CTX *c);
int MD5_Update(MD5_CTX *c, const void *data, size_t len);
int MD5_Final(unsigned char *md, MD5_CTX *c);
unsigned char *MD5(const unsigned char *d, size_t n, unsigned char *md);
void MD5_Transform(MD5_CTX *c, const unsigned char *b);
#ifdef  __cplusplus
}
#endif

#endif
