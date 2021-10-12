/* @(#)68	1.6  src/bos/kernel/sys/err_rec.h, cmderrlg, bos411, 9428A410j 9/28/93 10:59:25 */

/*
 * COMPONENT_NAME:            include/sys/err_rec.h
 *
 * FUNCTIONS: header file for system error log entry
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_ERR_REC
#define _H_ERR_REC

#define ERR_FALSE 0
#define ERR_TRUE  1

#define ERRSET_DESCRIPTION 1
#define ERRSET_PROBCAUSES  2
#define ERRSET_USERCAUSES  3
#define ERRSET_INSTCAUSES  4
#define ERRSET_FAILCAUSES  5
#define ERRSET_RECACTIONS  6
#define ERRSET_DETAILDATA  7
#define NERRSETS           7

#define ERR_NAMESIZE 16

struct err_rec0 {
	unsigned error_id;
	char     resource_name[ERR_NAMESIZE];
};

struct err_rec {
	unsigned error_id;
	char     resource_name[ERR_NAMESIZE];
	char     detail_data[1];
};
#define ERR_REC_SIZE (sizeof(struct err_rec0))
#define ERR_REC_MAX  230
#define ERR_REC_MAX_SIZE (ERR_REC_SIZE + ERR_REC_MAX)

#define ERR_REC(N) \
struct { \
	unsigned error_id; \
	char     resource_name[ERR_NAMESIZE]; \
	char     detail_data[N]; \
}

#endif

