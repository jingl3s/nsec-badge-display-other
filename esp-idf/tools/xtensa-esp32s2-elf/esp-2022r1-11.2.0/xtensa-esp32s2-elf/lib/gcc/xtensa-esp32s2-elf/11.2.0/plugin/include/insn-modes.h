/* Generated automatically from machmode.def
   by genmodes.  */

#ifndef GCC_INSN_MODES_H
#define GCC_INSN_MODES_H

enum machine_mode
{
  E_VOIDmode,              /* machmode.def:192 */
#define HAVE_VOIDmode
#ifdef USE_ENUM_MODES
#define VOIDmode E_VOIDmode
#else
#define VOIDmode ((void) 0, E_VOIDmode)
#endif
  E_BLKmode,               /* machmode.def:196 */
#define HAVE_BLKmode
#ifdef USE_ENUM_MODES
#define BLKmode E_BLKmode
#else
#define BLKmode ((void) 0, E_BLKmode)
#endif
  E_CCmode,                /* machmode.def:234 */
#define HAVE_CCmode
#ifdef USE_ENUM_MODES
#define CCmode E_CCmode
#else
#define CCmode ((void) 0, E_CCmode)
#endif
  E_BImode,                /* machmode.def:199 */
#define HAVE_BImode
#ifdef USE_ENUM_MODES
#define BImode E_BImode
#else
#define BImode (scalar_int_mode ((scalar_int_mode::from_int) E_BImode))
#endif
  E_QImode,                /* machmode.def:207 */
#define HAVE_QImode
#ifdef USE_ENUM_MODES
#define QImode E_QImode
#else
#define QImode (scalar_int_mode ((scalar_int_mode::from_int) E_QImode))
#endif
  E_HImode,                /* machmode.def:208 */
#define HAVE_HImode
#ifdef USE_ENUM_MODES
#define HImode E_HImode
#else
#define HImode (scalar_int_mode ((scalar_int_mode::from_int) E_HImode))
#endif
  E_SImode,                /* machmode.def:209 */
#define HAVE_SImode
#ifdef USE_ENUM_MODES
#define SImode E_SImode
#else
#define SImode (scalar_int_mode ((scalar_int_mode::from_int) E_SImode))
#endif
  E_DImode,                /* machmode.def:210 */
#define HAVE_DImode
#ifdef USE_ENUM_MODES
#define DImode E_DImode
#else
#define DImode (scalar_int_mode ((scalar_int_mode::from_int) E_DImode))
#endif
  E_TImode,                /* machmode.def:211 */
#define HAVE_TImode
#ifdef USE_ENUM_MODES
#define TImode E_TImode
#else
#define TImode (scalar_int_mode ((scalar_int_mode::from_int) E_TImode))
#endif
  E_QQmode,                /* machmode.def:237 */
#define HAVE_QQmode
#ifdef USE_ENUM_MODES
#define QQmode E_QQmode
#else
#define QQmode (scalar_mode ((scalar_mode::from_int) E_QQmode))
#endif
  E_HQmode,                /* machmode.def:238 */
#define HAVE_HQmode
#ifdef USE_ENUM_MODES
#define HQmode E_HQmode
#else
#define HQmode (scalar_mode ((scalar_mode::from_int) E_HQmode))
#endif
  E_SQmode,                /* machmode.def:239 */
#define HAVE_SQmode
#ifdef USE_ENUM_MODES
#define SQmode E_SQmode
#else
#define SQmode (scalar_mode ((scalar_mode::from_int) E_SQmode))
#endif
  E_DQmode,                /* machmode.def:240 */
#define HAVE_DQmode
#ifdef USE_ENUM_MODES
#define DQmode E_DQmode
#else
#define DQmode (scalar_mode ((scalar_mode::from_int) E_DQmode))
#endif
  E_TQmode,                /* machmode.def:241 */
#define HAVE_TQmode
#ifdef USE_ENUM_MODES
#define TQmode E_TQmode
#else
#define TQmode (scalar_mode ((scalar_mode::from_int) E_TQmode))
#endif
  E_UQQmode,               /* machmode.def:243 */
#define HAVE_UQQmode
#ifdef USE_ENUM_MODES
#define UQQmode E_UQQmode
#else
#define UQQmode (scalar_mode ((scalar_mode::from_int) E_UQQmode))
#endif
  E_UHQmode,               /* machmode.def:244 */
#define HAVE_UHQmode
#ifdef USE_ENUM_MODES
#define UHQmode E_UHQmode
#else
#define UHQmode (scalar_mode ((scalar_mode::from_int) E_UHQmode))
#endif
  E_USQmode,               /* machmode.def:245 */
#define HAVE_USQmode
#ifdef USE_ENUM_MODES
#define USQmode E_USQmode
#else
#define USQmode (scalar_mode ((scalar_mode::from_int) E_USQmode))
#endif
  E_UDQmode,               /* machmode.def:246 */
#define HAVE_UDQmode
#ifdef USE_ENUM_MODES
#define UDQmode E_UDQmode
#else
#define UDQmode (scalar_mode ((scalar_mode::from_int) E_UDQmode))
#endif
  E_UTQmode,               /* machmode.def:247 */
#define HAVE_UTQmode
#ifdef USE_ENUM_MODES
#define UTQmode E_UTQmode
#else
#define UTQmode (scalar_mode ((scalar_mode::from_int) E_UTQmode))
#endif
  E_HAmode,                /* machmode.def:249 */
#define HAVE_HAmode
#ifdef USE_ENUM_MODES
#define HAmode E_HAmode
#else
#define HAmode (scalar_mode ((scalar_mode::from_int) E_HAmode))
#endif
  E_SAmode,                /* machmode.def:250 */
#define HAVE_SAmode
#ifdef USE_ENUM_MODES
#define SAmode E_SAmode
#else
#define SAmode (scalar_mode ((scalar_mode::from_int) E_SAmode))
#endif
  E_DAmode,                /* machmode.def:251 */
#define HAVE_DAmode
#ifdef USE_ENUM_MODES
#define DAmode E_DAmode
#else
#define DAmode (scalar_mode ((scalar_mode::from_int) E_DAmode))
#endif
  E_TAmode,                /* machmode.def:252 */
#define HAVE_TAmode
#ifdef USE_ENUM_MODES
#define TAmode E_TAmode
#else
#define TAmode (scalar_mode ((scalar_mode::from_int) E_TAmode))
#endif
  E_UHAmode,               /* machmode.def:254 */
#define HAVE_UHAmode
#ifdef USE_ENUM_MODES
#define UHAmode E_UHAmode
#else
#define UHAmode (scalar_mode ((scalar_mode::from_int) E_UHAmode))
#endif
  E_USAmode,               /* machmode.def:255 */
#define HAVE_USAmode
#ifdef USE_ENUM_MODES
#define USAmode E_USAmode
#else
#define USAmode (scalar_mode ((scalar_mode::from_int) E_USAmode))
#endif
  E_UDAmode,               /* machmode.def:256 */
#define HAVE_UDAmode
#ifdef USE_ENUM_MODES
#define UDAmode E_UDAmode
#else
#define UDAmode (scalar_mode ((scalar_mode::from_int) E_UDAmode))
#endif
  E_UTAmode,               /* machmode.def:257 */
#define HAVE_UTAmode
#ifdef USE_ENUM_MODES
#define UTAmode E_UTAmode
#else
#define UTAmode (scalar_mode ((scalar_mode::from_int) E_UTAmode))
#endif
  E_SFmode,                /* machmode.def:229 */
#define HAVE_SFmode
#ifdef USE_ENUM_MODES
#define SFmode E_SFmode
#else
#define SFmode (scalar_float_mode ((scalar_float_mode::from_int) E_SFmode))
#endif
  E_DFmode,                /* machmode.def:230 */
#define HAVE_DFmode
#ifdef USE_ENUM_MODES
#define DFmode E_DFmode
#else
#define DFmode (scalar_float_mode ((scalar_float_mode::from_int) E_DFmode))
#endif
  E_SDmode,                /* machmode.def:270 */
#define HAVE_SDmode
#ifdef USE_ENUM_MODES
#define SDmode E_SDmode
#else
#define SDmode (scalar_float_mode ((scalar_float_mode::from_int) E_SDmode))
#endif
  E_DDmode,                /* machmode.def:271 */
#define HAVE_DDmode
#ifdef USE_ENUM_MODES
#define DDmode E_DDmode
#else
#define DDmode (scalar_float_mode ((scalar_float_mode::from_int) E_DDmode))
#endif
  E_TDmode,                /* machmode.def:272 */
#define HAVE_TDmode
#ifdef USE_ENUM_MODES
#define TDmode E_TDmode
#else
#define TDmode (scalar_float_mode ((scalar_float_mode::from_int) E_TDmode))
#endif
  E_CQImode,               /* machmode.def:265 */
#define HAVE_CQImode
#ifdef USE_ENUM_MODES
#define CQImode E_CQImode
#else
#define CQImode (complex_mode ((complex_mode::from_int) E_CQImode))
#endif
  E_CHImode,               /* machmode.def:265 */
#define HAVE_CHImode
#ifdef USE_ENUM_MODES
#define CHImode E_CHImode
#else
#define CHImode (complex_mode ((complex_mode::from_int) E_CHImode))
#endif
  E_CSImode,               /* machmode.def:265 */
#define HAVE_CSImode
#ifdef USE_ENUM_MODES
#define CSImode E_CSImode
#else
#define CSImode (complex_mode ((complex_mode::from_int) E_CSImode))
#endif
  E_CDImode,               /* machmode.def:265 */
#define HAVE_CDImode
#ifdef USE_ENUM_MODES
#define CDImode E_CDImode
#else
#define CDImode (complex_mode ((complex_mode::from_int) E_CDImode))
#endif
  E_CTImode,               /* machmode.def:265 */
#define HAVE_CTImode
#ifdef USE_ENUM_MODES
#define CTImode E_CTImode
#else
#define CTImode (complex_mode ((complex_mode::from_int) E_CTImode))
#endif
  E_SCmode,                /* machmode.def:267 */
#define HAVE_SCmode
#ifdef USE_ENUM_MODES
#define SCmode E_SCmode
#else
#define SCmode (complex_mode ((complex_mode::from_int) E_SCmode))
#endif
  E_DCmode,                /* machmode.def:267 */
#define HAVE_DCmode
#ifdef USE_ENUM_MODES
#define DCmode E_DCmode
#else
#define DCmode (complex_mode ((complex_mode::from_int) E_DCmode))
#endif
  MAX_MACHINE_MODE,

  MIN_MODE_RANDOM = E_VOIDmode,
  MAX_MODE_RANDOM = E_BLKmode,

  MIN_MODE_CC = E_CCmode,
  MAX_MODE_CC = E_CCmode,

  MIN_MODE_INT = E_QImode,
  MAX_MODE_INT = E_TImode,

  MIN_MODE_PARTIAL_INT = E_VOIDmode,
  MAX_MODE_PARTIAL_INT = E_VOIDmode,

  MIN_MODE_FRACT = E_QQmode,
  MAX_MODE_FRACT = E_TQmode,

  MIN_MODE_UFRACT = E_UQQmode,
  MAX_MODE_UFRACT = E_UTQmode,

  MIN_MODE_ACCUM = E_HAmode,
  MAX_MODE_ACCUM = E_TAmode,

  MIN_MODE_UACCUM = E_UHAmode,
  MAX_MODE_UACCUM = E_UTAmode,

  MIN_MODE_FLOAT = E_SFmode,
  MAX_MODE_FLOAT = E_DFmode,

  MIN_MODE_DECIMAL_FLOAT = E_SDmode,
  MAX_MODE_DECIMAL_FLOAT = E_TDmode,

  MIN_MODE_COMPLEX_INT = E_CQImode,
  MAX_MODE_COMPLEX_INT = E_CTImode,

  MIN_MODE_COMPLEX_FLOAT = E_SCmode,
  MAX_MODE_COMPLEX_FLOAT = E_DCmode,

  MIN_MODE_VECTOR_BOOL = E_VOIDmode,
  MAX_MODE_VECTOR_BOOL = E_VOIDmode,

  MIN_MODE_VECTOR_INT = E_VOIDmode,
  MAX_MODE_VECTOR_INT = E_VOIDmode,

  MIN_MODE_VECTOR_FRACT = E_VOIDmode,
  MAX_MODE_VECTOR_FRACT = E_VOIDmode,

  MIN_MODE_VECTOR_UFRACT = E_VOIDmode,
  MAX_MODE_VECTOR_UFRACT = E_VOIDmode,

  MIN_MODE_VECTOR_ACCUM = E_VOIDmode,
  MAX_MODE_VECTOR_ACCUM = E_VOIDmode,

  MIN_MODE_VECTOR_UACCUM = E_VOIDmode,
  MAX_MODE_VECTOR_UACCUM = E_VOIDmode,

  MIN_MODE_VECTOR_FLOAT = E_VOIDmode,
  MAX_MODE_VECTOR_FLOAT = E_VOIDmode,

  MIN_MODE_OPAQUE = E_VOIDmode,
  MAX_MODE_OPAQUE = E_VOIDmode,

  NUM_MACHINE_MODES = MAX_MACHINE_MODE
};

#define CONST_MODE_NUNITS const
#define CONST_MODE_PRECISION const
#define CONST_MODE_SIZE const
#define CONST_MODE_UNIT_SIZE const
#define CONST_MODE_BASE_ALIGN const
#define CONST_MODE_IBIT const
#define CONST_MODE_FBIT const
#define CONST_MODE_MASK const

#define BITS_PER_UNIT (8)
#define MAX_BITSIZE_MODE_ANY_INT (16*BITS_PER_UNIT)
#define MAX_BITSIZE_MODE_ANY_MODE (32*BITS_PER_UNIT)
#define NUM_INT_N_ENTS 1
#define NUM_POLY_INT_COEFFS 1

#endif /* insn-modes.h */
