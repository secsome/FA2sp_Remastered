#pragma once


#define LZO_VERSION             0x0200
#define LZO_VERSION_STRING      "0.20"
#define LZO_VERSION_DATE        "11 Aug 1996"


#include <limits.h>             /* CHAR_BIT, UINT_MAX, ULONG_MAX */
#if !defined(CHAR_BIT) || (CHAR_BIT != 8)
#  error invalid CHAR_BIT
#endif

//#ifdef __cplusplus
//extern "C" {
//#endif


/***********************************************************************
// defines
************************************************************************/

#if defined(__MSDOS__) || defined(MSDOS)
#  define __LZO_MSDOS
#  if (UINT_MAX < 0xffffffffL)
#    define __LZO_MSDOS16 
#  endif
#endif


/***********************************************************************
// integral and pointer types
************************************************************************/

/* Unsigned type with 32 bits or more */
#if (UINT_MAX >= 0xffffffffL)
typedef unsigned int     lzo_uint;
typedef int              lzo_int;
#  define LZO_UINT_MAX      UINT_MAX
#elif (ULONG_MAX >= 0xffffffffL)
typedef unsigned long    lzo_uint;
typedef long             lzo_int;
#  define LZO_UINT_MAX      ULONG_MAX
#else
#  error lzo_uint
#endif


/* Memory model that allows to access memory at offsets of lzo_uint.
 * Huge pointers (16 bit MSDOS) are somewhat slow, but they work
 * fine and I really don't care about 16 bit compiler
 * optimizations nowadays.
 */
#if (LZO_UINT_MAX <= UINT_MAX)
#  define __LZO_MMODEL
#elif defined(__LZO_MSDOS16)
#  define __LZO_MMODEL      huge
#  define __LZO_ENTRY       __cdecl
#else
#  error __LZO_MMODEL
#endif


 /* no typedef here because of const-pointer issues */
#define lzo_byte            unsigned char __LZO_MMODEL
#define lzo_voidp           void __LZO_MMODEL *
#define lzo_bytep           unsigned char __LZO_MMODEL *
#define lzo_uintp           lzo_uint __LZO_MMODEL *
#define lzo_intp            lzo_int __LZO_MMODEL *
#define lzo_voidpp          lzo_voidp __LZO_MMODEL * 
#define lzo_bytepp          lzo_bytep __LZO_MMODEL * 


/* Unsigned type that can store all bits of a lzo_voidp */
typedef unsigned long       lzo_ptr_t;

/* Align a pointer on a boundary that is a multiple of 'size' */
#define LZO_ALIGN(ptr,size) \
    ((lzo_voidp) (((lzo_ptr_t)(ptr) + (size)-1) & ~((lzo_ptr_t)((size)-1))))


/***********************************************************************
// function types
************************************************************************/

//#ifdef __cplusplus
//#  define LZO_EXTERN_C          extern "C"
//#else
#  define LZO_EXTERN_C          extern
//#endif


#if !defined(__LZO_ENTRY)       /* calling convention */
#  define __LZO_ENTRY
#endif
#if !defined(__LZO_EXPORT)      /* DLL export (and maybe size) information */
#  define __LZO_EXPORT
#endif

#if !defined(LZO_EXTERN)
#  define LZO_EXTERN(_rettype)  LZO_EXTERN_C _rettype __LZO_ENTRY __LZO_EXPORT
#endif


typedef int __LZO_ENTRY
(__LZO_EXPORT* lzo_compress_t)  (const lzo_byte* src, lzo_uint  src_len,
    lzo_byte* dst, lzo_uint* dst_len,
    lzo_voidp wrkmem);

typedef int __LZO_ENTRY
(__LZO_EXPORT* lzo_decompress_t)(const lzo_byte* src, lzo_uint  src_len,
    lzo_byte* dst, lzo_uint* dst_len,
    lzo_voidp wrkmem);


/* a progress indicator callback function */
typedef void __LZO_ENTRY
(__LZO_EXPORT* lzo_progress_callback_t)(lzo_uint, lzo_uint);


/***********************************************************************
// error codes and prototypes
************************************************************************/

/* Error codes for the compression/decompression functions. Negative
 * values are errors, positive values will be used for special but
 * normal events.
 */
#define LZO_E_OK                    0
#define LZO_E_ERROR                 (-1)
#define LZO_E_NOT_COMPRESSIBLE      (-2)    /* not used right now */
#define LZO_E_EOF_NOT_FOUND         (-3)
#define LZO_E_INPUT_OVERRUN         (-4)
#define LZO_E_OUTPUT_OVERRUN        (-5)
#define LZO_E_LOOKBEHIND_OVERRUN    (-6)
#define LZO_E_OUT_OF_MEMORY         (-7)    /* not used right now */


 /* this should be the first function you call. Check the return code ! */
LZO_EXTERN(int) lzo_init(void);

/* version functions (useful for shared libraries) */
LZO_EXTERN(unsigned) lzo_version(void);
LZO_EXTERN(const char*) lzo_version_string(void);

/* string functions */
LZO_EXTERN(int)
lzo_memcmp(const lzo_voidp _s1, const lzo_voidp _s2, lzo_uint _len);
LZO_EXTERN(lzo_voidp)
lzo_memcpy(lzo_voidp _dest, const lzo_voidp _src, lzo_uint _len);
LZO_EXTERN(lzo_voidp)
lzo_memmove(lzo_voidp _dest, const lzo_voidp _src, lzo_uint _len);
LZO_EXTERN(lzo_voidp)
lzo_memset(lzo_voidp _s, int _c, lzo_uint _len);

/* checksum functions */
LZO_EXTERN(lzo_uint)
lzo_adler32(lzo_uint _adler, const lzo_byte* _buf, lzo_uint _len);

/* misc. */
LZO_EXTERN(int) lzo_assert(int _expr);
LZO_EXTERN(int) _lzo_config_check(void);

#if defined(__LZO_MSDOS16) && defined(__TURBOC__)
#  if (__TURBOC__ < 0x452)
#    error You need a newer compiler version
#  endif
#endif

#if defined(__LZO_MSDOS) || defined(__i386__) || defined(__386__)
#  if !defined(__LZO_i386)
#    define __LZO_i386
#  endif
#endif


/***********************************************************************
//
************************************************************************/

#include <stddef.h>			/* ptrdiff_t, size_t */
#include <string.h>			/* memcpy, memmove, memcmp, memset */

#if 0 && !defined(assert)
#  error <assert.h> not included
#endif

#if defined(__BOUNDS_CHECKING_ON)
#  include <unchecked.h>
#else
#  define BOUNDS_CHECKING_OFF_DURING(stmt)		stmt
#  define BOUNDS_CHECKING_OFF_IN_EXPR(expr)		(expr)
#endif

/* ptrdiff_t */
#if (UINT_MAX >= 0xffffffffL)
typedef ptrdiff_t        lzo_ptrdiff_t;
#else
typedef long             lzo_ptrdiff_t;
#endif


#ifdef __cplusplus
#  define LZO_UNUSED(parm)
#else
#  define LZO_UNUSED(parm)	parm
#endif


#if !defined(__inline__) && !defined(__GNUC__)
#  if defined(__cplusplus)
#    define __inline__		inline
#  else
#    define __inline__		/* nothing */
#  endif
#endif


/***********************************************************************
// compiler and architecture specific stuff
************************************************************************/

/* Some defines that indicate if memory can be accessed at unaligned
 * addresses. You should also test that this is actually faster if
 * it is allowed by your system.
 */

#if 1 && defined(__LZO_i386)
#  if !defined(LZO_UNALIGNED_OK_2)
#    define LZO_UNALIGNED_OK_2
#  endif
#  if !defined(LZO_UNALIGNED_OK_4)
#    define LZO_UNALIGNED_OK_4
#  endif
#endif


#if defined(LZO_UNALIGNED_OK_2) || defined(LZO_UNALIGNED_OK_4)
#  if !defined(LZO_UNALIGNED_OK)
#    define LZO_UNALIGNED_OK
#  endif
#endif


 /* Definitions for byte order, according to significance of bytes, from low
  * addresses to high addresses. The value is what you get by putting '4'
  * in the most significant byte, '3' in the second most significant byte,
  * '2' in the second least significant byte, and '1' in the least
  * significant byte.
  */

#define	LZO_LITTLE_ENDIAN		1234
#define	LZO_BIG_ENDIAN			4321
#define	LZO_PDP_ENDIAN			3412

  /* The byte order is only needed if we use LZO_UNALIGNED_OK */
#if !defined(LZO_BYTE_ORDER)
#  if defined(__LZO_i386)
#    define LZO_BYTE_ORDER		LZO_LITTLE_ENDIAN
#  elif defined(__mc68000__)
#    define LZO_BYTE_ORDER		LZO_BIG_ENDIAN
#  elif defined(__BYTE_ORDER)
#    define LZO_BYTE_ORDER		__BYTE_ORDER
#  endif
#endif

#if defined(LZO_UNALIGNED_OK)
#  if !defined(LZO_BYTE_ORDER)
#    error LZO_BYTE_ORDER is not defined
#  elif (LZO_BYTE_ORDER != LZO_LITTLE_ENDIAN) && \
        (LZO_BYTE_ORDER != LZO_BIG_ENDIAN)
#    error invalid LZO_BYTE_ORDER
#  endif
#endif


/***********************************************************************
// optimization
************************************************************************/

/* gcc 2.6.3 and gcc 2.7.2 have a bug */
#define LZO_OPTIMIZE_GNUC_i386_IS_BUGGY

/* Help the optimizer with register allocation.
 * Don't activate this macro for a fair comparision with other algorithms.
 */
#if 1 && defined(NDEBUG) && !defined(__BOUNDS_CHECKING_ON)
#  if defined(__GNUC__) && defined(__i386__)
#    if !defined(LZO_OPTIMIZE_GNUC_i386_IS_BUGGY)
#      define LZO_OPTIMIZE_GNUC_i386
#    endif
#  endif
#endif


 /***********************************************************************
 //
 ************************************************************************/

#define LZO_BYTE(x)			((unsigned char) (x))

#define LZO_MAX(a,b)		((a) >= (b) ? (a) : (b))
#define LZO_MIN(a,b)		((a) <= (b) ? (a) : (b))

#define lzo_sizeof(x)		((lzo_uint) (sizeof(x)))

#define LZO_HIGH(x)			((lzo_uint) (sizeof(x)/sizeof(*(x))))

 /* this always fits into 16 bits */
#define LZO_SIZE(bits)		(1u << (bits))
#define LZO_MASK(bits)		(LZO_SIZE(bits) - 1)

#define LZO_LSIZE(bits)		(1ul << (bits))
#define LZO_LMASK(bits)		(LZO_LSIZE(bits) - 1)

#define LZO_USIZE(bits)		((lzo_uint) 1 << (bits))
#define LZO_UMASK(bits)		(LZO_USIZE(bits) - 1)


/***********************************************************************
// ANSI C preprocessor macros
************************************************************************/

#define _LZO_STRINGIZE(x)			#x
#define _LZO_MEXPAND(x)				_LZO_STRINGIZE(x)

/* concatenate */
#define _LZO_CONCAT2(a,b)			a ## b
#define _LZO_CONCAT3(a,b,c)			a ## b ## c
#define _LZO_CONCAT4(a,b,c,d)		a ## b ## c ## d
#define _LZO_CONCAT5(a,b,c,d,e)		a ## b ## c ## d ## e

/* expand and concatenate (by using one level of indirection) */
#define _LZO_ECONCAT2(a,b)			_LZO_CONCAT2(a,b)
#define _LZO_ECONCAT3(a,b,c)		_LZO_CONCAT3(a,b,c)
#define _LZO_ECONCAT4(a,b,c,d)		_LZO_CONCAT4(a,b,c,d)
#define _LZO_ECONCAT5(a,b,c,d,e)	_LZO_CONCAT5(a,b,c,d,e)


/***********************************************************************
//
************************************************************************/

/* Generate compressed data in a deterministic way.
 * This is fully portable, and compression can be faster as well.
 * A reason NOT to be deterministic is when the block size is
 * very small (e.g. 8kB) or the dictionary is big, because
 * then the initialization of the dictionary becomes a relevant
 * magnitude for compression speed.
 */
#define LZO_DETERMINISTIC


 /***********************************************************************
 //
 ************************************************************************/

#if 0 
 /* This line causes problems on some architectures */
#define LZO_CHECK_MPOS_DET(m_pos,m_off,in,ip,max_offset) \
	(BOUNDS_CHECKING_OFF_IN_EXPR( \
		(m_off = ip - m_pos) > max_offset ))

#else
 /* This is the safe (but slower) version */
#define LZO_CHECK_MPOS_DET(m_pos,m_off,in,ip,max_offset) \
		(m_pos == NULL || (m_off = ip - m_pos) > max_offset)
#endif


/* m_pos may point anywhere...
 * This marco is probably a good candidate for architecture specific problems.
 * Try casting the pointers to lzo_ptr_t before comparing them.
 */
#define LZO_CHECK_MPOS_NON_DET(m_pos,m_off,in,ip,max_offset) \
	(BOUNDS_CHECKING_OFF_IN_EXPR( \
		(m_pos < in || (m_off = ip - m_pos) <= 0 || m_off > max_offset) ))
