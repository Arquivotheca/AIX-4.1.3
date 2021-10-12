#ifndef uuid__v0_included
#define uuid__v0_included
#include "idl_base.h"
#include "nbase.h"
typedef ndr__char uuid__string_t[37];
extern  void uuid__gen
#ifdef __STDC__
 (
  /* [out] */uuid__t *uuid);
#else
 ( );
#endif
extern  void uuid__encode
#ifdef __STDC__
 (
  /* [in] */uuid__t *uuid,
  /* [out] */uuid__string_t s);
#else
 ( );
#endif
extern  void uuid__decode
#ifdef __STDC__
 (
  /* [in] */uuid__string_t s,
  /* [out] */uuid__t *uuid,
  /* [out] */status__t *st);
#else
 ( );
#endif
extern  ndr__boolean uuid__equal
#ifdef __STDC__
 (
  /* [in] */uuid__t *u1,
  /* [in] */uuid__t *u2);
#else
 ( );
#endif
extern  ndr__long_int uuid__cmp
#ifdef __STDC__
 (
  /* [in] */uuid__t *u1,
  /* [in] */uuid__t *u2);
#else
 ( );
#endif
extern  ndr__ulong_int uuid__hash
#ifdef __STDC__
 (
  /* [in] */uuid__t *u,
  /* [in] */ndr__ulong_int modulus);
#else
 ( );
#endif
#endif
