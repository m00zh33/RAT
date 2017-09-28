/*\
 *
 * EnRUPT - Fast Block/Stream Cipher/HASH/MAC/RNG/PRNG Function
 * Designed by Sean O'Neil <sean><cryptolib><com>
 * http://www.enrupt.com/
 * EnRUPT and all its implementations are in the public domain.
 *
 * enrupt.h - A reference C implementation version 1.09
 * This implementation is for reference only.
 * It is very slow in most cases.
 * Only little-endian byte order is supported in this version.
 *
 * Note: You only need this file.
 * The C program is only for performance testing.
 *
\*/


#ifndef ENRUPT_H
#define ENRUPT_H

#if (defined(_MSC_VER)||defined(__INTEL_COMPILER))
        #include <stdlib.h>
        #ifndef u32
                #define u32         unsigned long
        #endif
        #pragma intrinsic (_lrotr)
        #if !defined(rotr)
                #define rotr(x,n)   _lrotr(x,n)
        #endif
        #if !defined(__inline__)
                #define __inline__  __forceinline
        #endif
#elif !defined(rotr)
        #include <stdint.h>
        #ifndef u32
                #define u32         uint_fast32_t
        #endif
        #define rotr(x,n)           (((((u32)(x))>>((n)&31))&0xFFFFFFFFUL)|((((u32)(x))<<((0-(n))&31))&0xFFFFFFFFUL))
#endif

// EnRUPT round functions:

//#define rupt(x0,x2,k,r) (rotr(2*(x0)^(x2)^(k)^(r),8)*9)	// An inline function is faster in MSVC
static __inline__ u32 rupt(const u32 x0, const u32 x2, const u32 k, const u32 r) { return rotr(2*x0^x2^k^r,8)*9; }
#define rupt1(x0,x1,x2,k,r) ((x1)^=rupt(x0,x2,k,r)^(k))
#define rupts(x0,x1,x2,x3,x4,x5,r,d,t) ((x1)^=t=rupt(x0,x2,d,r*2+1),(d)^=(t)^(x4),(x2)^=t=rupt(x1,x3,d,r*2+2),r++,(d)^=(t)^x5)
#define irrupt(x,xw,r,d,t) rupts(x[r*2%(xw)],x[(r*2+1)%(xw)],x[(r*2+2)%(xw)],x[(r*2+3)%(xw)],x[(r*2+1+(xw)/2)%(xw)],x[(r*2+2+(xw)/2)%(xw)],r,d,t)

// Block EnRUPT for blocks and keys of any size:

#define enRUPT(x,xw,k,kw,r) {for (r=1; r<=8*(xw)+4*(kw); r++) rupt1 (x[(r-1)%(xw)],x[r%(xw)],x[(r+1)%(xw)],k[r%(kw)],r);}
#define unRUPT(x,xw,k,kw,r) {for (r=8*(xw)+4*(kw); r!=0; r--) rupt1 (x[(r-1)%(xw)],x[r%(xw)],x[(r+1)%(xw)],k[r%(kw)],r);}

// EnRUPT stream modes:

#define irRUPT_init(x,xw,k,kw,r,d,t,u) \
{\
        for (u = 0; u < xw; u++) x[u] = 0; r = 0; d = 0;\
        if (kw)\
        {\
                while (r < kw) mcRUPT (x, xw, r, k[r], d, t);\
                for (u = 0; u < 2*(xw); u++) irrupt (x, xw, r, d, t);\
        }\
}

// The following word-wise data processing does not include handling of loose odd bytes between calls

#define RUPT(x,xw,r,d,p,c,e,t)	((e?c:p)=(e?p:c)^irrupt(x,xw,r,d,t))
#define aeRUPT(x,xw,r,d,p,c,e)	(RUPT(x,xw,r,d,p,c,e),d=c)
#define mcRUPT(x,xw,r,d,p,t)	(d=irrupt(x,xw,r,d,t)^(p))

#define irRUPT(x,xw,r,d,p,t,u) \
{\
        irrupt (x, xw, r, d, t);\
        irrupt (x, xw, r, d, t);\
        mcRUPT (x, xw, r, d, p, t);\
}

// The following word-wise finalization does not include bitwise padding

#define irRUPT_fini(h,hw,x,xw,r,d,t,u) \
{\
        for (u = 0; u < 2*(xw); u++) irrupt (x, xw, r, d, t);\
        for (u = 0; u < hw; u++) h[u] = irrupt (x, xw, r, d, t);\
}

// mdRUPT-max hash function without padding:

static __inline__ void mdRUPT_hash (u32 * const h, const u32 hw, u32 * const x, const u32 xw, const u32 * k, const u32 kw)
{
        u32	d, i, j, t;

        for (i = 0; i < xw; i++) x[i] = 0;
        enRUPT (x, xw, k, kw, i)
        for (i = 0, d = 0; i < hw;) h[i-1] = irrupt (x, xw, j, d, t);
        for (i = 0; i < xw; i++) x[i] = 0;
}

// The above functions do not include memory allocation or other context handling

#endif // ENRUPT_H
