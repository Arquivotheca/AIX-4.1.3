#ifndef time_base_v1_included
#define time_base_v1_included
#include "idl_base.h"
typedef ndr__ulong_int time__clockh_t;
typedef struct NIDL_tag_da1 time__clock_t;
struct NIDL_tag_da1 {
time__clockh_t high;
ndr__ushort_int low;
};
#endif
