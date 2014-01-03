#if !defined(ATOM_HHH)
#define ATOM_HHH

/* 
 The following macros identify the host OS, architecture and compiler. 
 They are derived from http://predef.sourceforge.net/ 
 */

/** @name Identifying the host operating system
 ** @{ */
#if defined(linux)     || \
    defined(__linux)   || \
    defined(__linux__) || \
    defined(__DOXYGEN__)
#define VL_OS_LINUX 1
#endif

#if (defined(__APPLE__) & defined(__MACH__)) || \
     defined(__DOXYGEN__)
#define VL_OS_MACOSX 1
#endif

#if defined(__WIN32__) || \
    defined(_WIN32)    || \
    defined(__DOXYGEN__)
#define VL_OS_WIN 1
#endif

#if defined(_WIN64) || \
    defined(__DOXYGEN__)
#define VL_OS_WIN64 1
#endif
/** @} */

/** @name Identifying the host compiler
 ** @{ */
#if defined(__GNUC__) || defined(__DOXYGEN__)
# if defined(__GNUC_PATCHLEVEL__)
#  define VL_COMPILER_GNUC (__GNUC__ * 10000 \
+ __GNUC_MINOR__ * 100 \
+ __GNUC_PATCHLEVEL__)
# else
#  define VL_COMPILER_GNUC (__GNUC__ * 10000 \
+ __GNUC_MINOR__ * 100)
# endif
#endif

#if defined(_MSC_VER) || defined(__DOXYGEN__)
#define VL_COMPILER_MSC _MSC_VER
#endif

#if defined(__LCC__) || defined(__DOXYGEN__)
#warning "LCC support is experimental!"
#define VL_COMPILER_LCC 1
#endif

/** @} */

/** @name Identifying the host CPU architecture
 ** @{ */
#if defined(i386)     || \
    defined(__i386__) || \
    defined(__DOXYGEN__)
#define VL_ARCH_IX86 300
#elif defined(__i486__)
#define VL_ARCH_IX86 400
#elif defined(__i586__)
#define VL_ARCH_IX86 500
#elif defined(__i686__)
#define VL_ARCH_IX86 600
#elif defined(_M_IX86)
#define VL_ARCH_IX86 _M_IX86
#endif

#if defined(__ia64__) || \
    defined(_IA64)    || \
    defined(__IA64)   || \
    defined(__ia64)   || \
    defined(_M_IA64)  || \
    defined(__DOXYGEN__)
#define VL_ARCH_IA64
#endif
/** @} */

/** @name Identifying the host data model
 ** @{ */
#if defined(__LLP64__) || \
    defined(__LLP64)   || \
    defined(__LLP64)   || \
    (defined(VL_COMPILER_MSC) & defined(VL_OS_WIN64)) || \
    (defined(VL_COMPILER_LCC) & defined(VL_OS_WIN64)) || \
    defined(__DOXYGEN__)
#define VL_COMPILER_LLP64
#endif

#if defined(__LP64__) || \
    defined(__LP64)   || \
    defined(__LP64)   || \
    (defined(VL_OS_MACOSX) & defined(VL_ARCH_IA64)) || \
    defined(__DOXYGEN__)
#define VL_COMPILER_LP64
#endif

#if (!defined(VL_COMPILER_LLP64) & !defined(VL_COMPILER_LP64)) || \
    defined(__DOXYGEN__)
#define VL_COMPILER_ILP32
#endif
/** @} */

/** @name Identifying the host endianness
 ** @{ */
#if defined(__LITTLE_ENDIAN__) || \
    defined(VL_ARCH_IX86)      || \
    defined(VL_ARCH_IA64)      || \
    defined(__DOXYGEN__)
#define VL_ARCH_LITTLE_ENDIAN
#endif

#if  defined(__DOXYGEN__) || \
    !defined(VL_ARCH_LITTLE_ENDIAN)
#define VL_ARCH_BIG_ENDIAN
#endif
/** @} */

#if defined(VL_COMPILER_MSC)
#define VL_INLINE static __inline
# ifdef VL_BUILD_DLL
#  define VL_EXPORT __declspec(dllexport)
# else
#  define VL_EXPORT __declspec(dllimport)
# endif
#endif

#if defined(VL_COMPILER_LCC)
#define VL_INLINE static __inline
# ifdef VL_BUILD_DLL
#  define VL_EXPORT __declspec(dllexport)
# else
#  define VL_EXPORT 
# endif
#endif

#if defined(VL_COMPILER_GNUC) || \
    defined(__DOXYGEN__)
#define VL_INLINE static __inline__
# ifdef VL_BUILD_DLL
#  define VL_EXPORT __attribute__((visibility ("default")))
# else
#  define VL_EXPORT
# endif
#endif

/** ------------------------------------------------------------------
 ** @name Atomic data types
 ** @{
 **/

#if defined(VL_COMPILER_LP64) || defined(VL_COMPILER_LLP64)
typedef long long           vl_int64 ;   /**< @brief Signed 64-bit integer. */
typedef int                 vl_int32 ;   /**< @brief Signed 32-bit integer. */
typedef short               vl_int16 ;   /**< @brief Signed 16-bit integer. */
typedef char                vl_int8  ;   /**< @brief Signed  8-bit integer. */

typedef long long unsigned  vl_uint64 ;  /**< @brief Unsigned 64-bit integer. */
typedef int       unsigned  vl_uint32 ;  /**< @brief Unsigned 32-bit integer. */
typedef short     unsigned  vl_uint16 ;  /**< @brief Unsigned 16-bit integer. */
typedef char      unsigned  vl_uint8 ;   /**< @brief Unsigned  8-bit integer. */

typedef int                 vl_int ;     /**< @brief Same as @c int. */
typedef unsigned int        vl_uint ;    /**< @brief Same as <code>unsigned int</code>. */

typedef int                 vl_bool ;    /**< @brief Boolean. */
typedef vl_int64            vl_intptr ;  /**< @brief Integer holding a pointer. */
typedef vl_uint64           vl_uintptr ; /**< @brief Unsigned integer holding a pointer. */
#endif

#if defined(VL_COMPILER_ILP32)

#ifdef VL_COMPILER_MSC
typedef __int64             vl_int64 ;  
#else
typedef long long           vl_int64 ;  
#endif

typedef int                 vl_int32 ;  
typedef short               vl_int16 ;  
typedef char                vl_int8  ;  

#ifdef VL_COMPILER_MSC
typedef __int64   unsigned  vl_uint64 ; 
#else
typedef long long unsigned  vl_uint64 ; 
#endif
typedef int       unsigned  vl_uint32 ; 
typedef short     unsigned  vl_uint16 ; 
typedef char      unsigned  vl_uint8 ;  

typedef int                 vl_int ;    
typedef unsigned int        vl_uint ;   

typedef int                 vl_bool ;   
typedef vl_int32            vl_intptr ; 
typedef vl_uint32           vl_uintptr ;
#endif
/** @} */

//typedef unsigned char uchar

/** ------------------------------------------------------------------
 ** @name Atomic data types limits
 ** @{ */

/** @brief Largest integer (math constant) */
#define VL_BIG_INT  0x7FFFFFFFL

/** @brief Smallest integer (math constant) */
#define VL_SMALL_INT  (- VL_BIG_INT - 1)

/** @brief Largest unsigned integer (math constant) */
#define VL_BIG_UINT 0xFFFFFFFFUL

/** ------------------------------------------------------------------
 ** @name Common operations
 ** @{ */

/** @brief Min operation
 ** @param x value
 ** @param y value
 ** @return the minimum of @a x and @a y.
 **/
#define VL_MIN(x,y) (((x)<(y))?(x):(y))

/** @brief Max operation
 ** @param x value.
 ** @param y value.
 ** @return the maximum of @a x and @a y.
 **/
#define VL_MAX(x,y) (((x)>(y))?(x):(y))

/** @brief Signed left shift operation
 **
 ** The macro is equivalent to the builtin @c << operator, but it
 ** supports negative shifts too.
 ** 
 ** @param x value.
 ** @param n number of shift positions.
 ** @return @c x << n .
 **/
#define VL_SHIFT_LEFT(x,n) (((n)>=0)?((x)<<(n)):((x)>>-(n)))
/* @} */

#define SAFE_FREE(p) { if (p) free(p); p = NULL; }

/** ------------------------------------------------------------------
 ** @name some basic number
 ** @{ */
/** @brief Logarithm of 2 (math constant)*/
#define VL_LOG_OF_2 0.693147180559945

/** @brief Pi (math constant) */
#define VL_PI 3.141592653589793

/** @brief Single precision epsilon (math constant) 
 **
 ** Difference of the smallest representable number greater
 ** than 1.0 and 1.0.
 **
 **/
#define VL_EPSILON_F 1.19209290E-07F

/** @brief Double precision epsilon (math constant) 
 **
 ** Difference of the smallest representable number greater
 ** than 1.0 and 1.0.
 **/
#define VL_EPSILON_D 2.220446049250313e-16


/** ------------------------------------------------------------------
 ** @name Error handling 
 ** @{ */

/** @brief The number of the last error */
//extern int vl_err_no ;

/** @brief The maximum length of an error description. */
#define VL_ERR_MSG_LEN 1024

/** @brief The description of the last error. */
//extern char vl_err_msg [VL_ERR_MSG_LEN + 1] ;

#define VL_ERR_OK       0  /**< No error */
#define VL_ERR_OVERFLOW 1  /**< Buffer overflow error */
#define VL_ERR_ALLOC    2  /**< Resource allocation error */
#define VL_ERR_BAD_ARG  3  /**< Bad argument or illegal data error */
#define VL_ERR_IO       4  /**< Input/output error */
#define VL_ERR_EOF      5  /**< End-of-file or end-of-sequence error */
#define VL_ERR_NO_MORE  5  /**< End-of-sequence @deprecated */


#include "time.h"
static clock_t tic_mark ; /**< @internal Store clock time for ::vl_tic() */

/** ------------------------------------------------------------------
 ** @brief Set time reference
 **/

static inline 
void vl_tic() 
{
  tic_mark = clock() ;
}

/** ------------------------------------------------------------------
 ** @brief Get time since reference
 **
 ** Returns the processor time elapsed since ::vl_tic() was called.
 **
 ** @return time in seconds.
 **/

static inline
double vl_toc()
{
  return (double) (clock() - tic_mark) / CLOCKS_PER_SEC ;
}


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


#endif
