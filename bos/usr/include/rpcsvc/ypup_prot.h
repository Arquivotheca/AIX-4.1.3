/* @(#)46	1.4  src/bos/usr/include/rpcsvc/ypup_prot.h, libcyp, bos411, 9428A410j 6/16/90 00:23:25 */
/* 
 * COMPONENT_NAME: (LIBCYP) Yellow Pages Library
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 24 
 *
 * Copyright (c) 1986 Sun Microsystems, Inc.
 */
/* 
 * (#)ypupdate_prot.h	1.2 88/03/31 4.0NFSSRC; from 1.4 88/02/08 
 * Copyr 1986, Sun Micro 
 */

#define MAXMAPNAMELEN 255
#define MAXYPDATALEN 1023
#define MAXERRMSGLEN 255

#define YPU_PROG ((u_long)100028)
#define YPU_VERS ((u_long)1)
#define YPU_CHANGE ((u_long)1)
extern u_int *ypu_change_1();
#define YPU_INSERT ((u_long)2)
extern u_int *ypu_insert_1();
#define YPU_DELETE ((u_long)3)
extern u_int *ypu_delete_1();
#define YPU_STORE ((u_long)4)
extern u_int *ypu_store_1();


typedef struct {
	u_int yp_buf_len;
	char *yp_buf_val;
} yp_buf;
bool_t xdr_yp_buf();


struct ypupdate_args {
	char *mapname;
	yp_buf key;
	yp_buf datum;
};
typedef struct ypupdate_args ypupdate_args;
bool_t xdr_ypupdate_args();


struct ypdelete_args {
	char *mapname;
	yp_buf key;
};
typedef struct ypdelete_args ypdelete_args;
bool_t xdr_ypdelete_args();

