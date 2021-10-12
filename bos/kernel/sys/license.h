/* @(#)70	1.6  src/bos/kernel/sys/license.h, cmdlicense, bos411, 9428A410j 6/6/94 18:18:34 */
/*
 *   COMPONENT_NAME: CMDLICENSE
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef	_H_LICENSE
#define	_H_LICENSE

#include <sys/types.h>

#define	VENDOR_NAME		"AIX Base Operating System"
#define	VENDOR_ID		"90b82be84513.02.81.23.86.fb.00.00.00"
#define	VENDOR_KEY		46133

/* get_nodelock_info code */
#define	NL_PRODUCT_NAME		"AIX Fixed User License"
#define	NL_PRODUCT_ID		5765393
#define	NL_PRODUCT_VERSION	"4.1"
#define	NL_PRODUCT_VLEN		(sizeof(NL_PRODUCT_VERSION) -1)

/* monitord code */
#define	FLT_PRODUCT_NAME	"AIX Floating User License"
#define	FLT_PRODUCT_ID		5765394
#define	FLT_PRODUCT_VERSION	"4.1"
#define	FLT_PRODUCT_VLEN	(sizeof(FLT_PRODUCT_VERSION)-1)

#define	NODELOCK_FILE		"/usr/lib/netls/conf/nodelock"
#define	FLOATING_ON		"/etc/security/.netls"
#define	NETLS_ERROR_LOG		"/var/security/monitord_log"

#define	MONITORD_PIPE		"/etc/security/monitord_pipe"
#define	MONITORD_PROGRAM	"/usr/sbin/monitord"
#define	REQUEST_LIC		1
#define	RELEASE_LIC		(-1)

#define LOGIN_FILE_PATH "/etc/security/login.cfg"
#define MAX_STANZA_LENGTH 100
#define LOGIN_HEADER        "usw:"
#define LOGIN_HEADER_LENGTH 4
#define LOGIN_ENTRY         "maxlogins"
#define LOGIN_ENTRY_LENGTH  9
#define GET_HEADER          1
#define GET_ENTRY           2


struct	monitord_request
{
	pid_t	request_type;
	pid_t	login_pid;
};

struct login_file_type {
               int file_id;           /* The id returned from open() */
               off_t file_size;       /* The number of bytes in the file */
               char *stanza_start;    /* points to the 'usw:' */
               char *maxlogins_start;  /* points to 'maxlogins' */
               int had_to_malloc;     /* Boolean if the mem_file was malloced */
               char *mem_file;        /* Pointer to the beginning of the file */
                      };

struct login_file_type  *open_login_file();
void find_users_stanza();
char *get_stanza_line();

extern	int	_FixedLicenseCount(void);
extern	void	_FloatingReleaseLicense(pid_t);

#endif /* _H_LICENSE */
