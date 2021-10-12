static char sccsid[] = "@(#)18  1.17  src/bos/usr/lib/methods/common/tcpcfglb.c, cmdnet, bos411, 9428A410j 4/10/91 16:59:26";
/*
 * COMPONENT_NAME:  CMDNET   Network Configuration 
 *
 * FUNCTIONS: tcp_getopt, getval, nexttoken, get_file_attr,
 *            split_att_val, shell, nextattr, errmsg, cfg_init_db, 
 *            tcp_getattr, argify.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989.
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * tcpcfglb.c - a small library of routines shared by various programs
 * in the network configuration component.
 */

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mdio.h>		/* for led setting */
#include <nlist.h>
#include <sys/file.h>
#include <string.h>
#include "tcpcfg.h"

#include "tcpcfglb_msg.h"
#define MSG_SET MS_TCPCFGLB
#include "msg_hdr.h"
extern nl_catd catd;

#define  MAXRECLEN  256		/* maximum line length in attribute file */

extern char progname[];

/* the stats below, should match the defined constands in cfgdb.h */
char *stats[] = { "DEFINED", "AVAILABLE", "STOPPED" };

/* the following two globals are used in command line processing,
   as in getopt(). */
int	optind = 1;		/* index of next argv			*/
char	*optarg;		/* argument for current option		*/
/*
 * NAME: tcp_getopt
 *                                                                    
 * FUNCTION:  parses command line
 *                                                                    
 * NOTES:
 *
 * RETURNS:  
 *     current command line switch, a character.
 *     -1 if error, such as no more parameters.
 */
tcp_getopt(int argc, char *argv[])
{
	char sw;

	if (optind >= argc)	/* no more args */
		return(-1);
	if (argv[optind][0] != '-') { /* arg not a switch */
		optind++;	/* try next arg */
		if (argv[optind][0] != '-') /* also not a switch */
			return(-1);
	}
	sw = argv[optind][1]; /* get the switch character */
	if (argv[optind][2] != (char)NULL) /* value concatentated w/switch */
		optarg = &(argv[optind][2]);
	else if (argv[++optind][0] != '-') /* value may be nextarg */
		optarg = argv[optind];
	
	return(sw);
}


/*
 * NAME: getval
 *                                                                    
 * FUNCTION:  finds value of attr in list, and returns list with attr deleted.
 *                                                                    
 * NOTES:
 *
 *
 *
 * RETURNS:  
 *     pointer to character string which is the value corresponding to 
 *         input attribute.
 *     NULL if attribute not found in list.
 */
char *
getval(char *attr,
       char *list)
{
	static char buf[256];
	char *p, *p1, *p2;
	int n;
	
	if ((p1 = strstr(list, attr)) == NULL)
		return(NULL);
	p = p1 + strlen(attr);	/* move past attribute name */
	p += strspn(p, " =");	/* skip spaces, equal sign */
	n = strcspn(p, " ");
	strncpy(buf, p, n); /* return the value in buf */
	buf[n] = '\0';
	p2 = p + strlen(buf);	/* measure end of value in input list */
	strcpy(p1, p2);		/* remove attr/val we just found */
	return(buf);
}


/*
 * NAME: nexttoken
 *                                                                    
 * FUNCTION: 
 * returns pointer to next token in list, and returns list with token deleted.
 *                                                                    
 * NOTES:
 *
 *
 *
 * RETURNS:  
 *     returns pointer to next token (delimited by spaces or tabs) in
 *         given list of tokens.
 *     NULL if no token found.
 */
char *
nexttoken(char *list)
{
	static char buf[256];
	char *p1, *p2;
	
	p1 = list + strspn(list, " \t");
	if (*p1 == 0)  return(0);
	p2 = p1 + strcspn(p1, " \t");	/* move past token name */
	strncpy(buf, p1, (int) (p2-p1)); /* return the token in buf */
	buf[p2-p1] = '\0';
	strcpy(list, p2);	/* remove token we just found */
	return(buf);
}


/*
 * NAME: get_file_attr
 *                                                                    
 * FUNCTION: 
 *  gets the attributes from file 'filename' and append them to buffer 'ap'
 *  limit is the end of the buffer.
 *                                                                    
 * NOTES:
 *
 * RETURNS:
 *     0 if successful.
 *     -1 on error.
 */
get_file_attr(char *fname,	/* filename of attribute file */
	      char *ap[],	/* pointer to output attribute list */
	      int *ind,		/* where to start putting attr in list*/
	      int max)		/* max number of attribute lines to read. */
{
	FILE *fp;
	char *bp;
	int len=0, count = 0;

	if ((fp = fopen(fname, "r")) == 0)
		return(-1);

	bp = (char *)malloc(MAXRECLEN);
	while ((int)fgets(bp, MAXRECLEN, fp) > 0) {
		len = strlen(bp) - 1;
		*(bp + len) = '\0'; /* delete trailing newline */
		*ap = bp;
		ap++;
		(*ind)++;
		count++;
		if (max && count > max) return(-2);
		bp = (char *)malloc(MAXRECLEN);
	}
	close(fp);
	return(0);
}


/*
 * NAME: split_att_val
 *                                                                    
 * FUNCTION: 
 * takes a string of the form:  att=val, replaces the '=' with a '\0',
 *                                                                    
 * NOTES:
 *     this routine modifies the input string.
 *
 *
 * RETURNS:
 *     pointer to val in att=val string.
 *     NULL if no '=' found.
 */
char *
split_att_val(char *ptr)	/* input string */
{
    char *p;

    for (p=ptr; *p && *p != '='; p++) ;
    if (*p != '=')
	return(NULL);
    *p++ = '\0';		/* replace '=' with '\0' */
    return(p);
}


/*
 * NAME: argify
 *                                                                    
 * FUNCTION: splits a string into separate tokens
 *                                                                    
 * NOTES:
 *
 *
 *
 * RETURNS:  
 *     nothing.
 */
argify(char *string, char **list)
{
	char *p1, *p2 = string;
	int i=0;
	
	for (;;) {
		p1 = p2 + strspn(p2, " \t"); /* skip initial delimiters */
		if (!*p1)
			break;
		p2 = p1 + strcspn(p1, " \t"); /* move past the token */
		*list = p1;
		list++;
		if (!*p2)
			break;
		*p2++ = 0;
	}
	*list = 0;
}

/*
 * NAME: shell
 *                                                                    
 * FUNCTION: execs a command line.
 *                                                                    
 * NOTES:
 *
 *
 *
 * RETURNS:  
 *     0 if successful.
 *     return code of failed program or system() call.
 */
shell(char *cmd)
{
	char *argv[128];
	int rc = 0;
	char *p1, *p2;
	static char argbuf[1024];
	int status = 0;

	strcpy(argbuf, cmd);
	argify(argbuf, argv);
	
	if ((rc = fork()) == 0) {
		execv(argv[0], argv);
	}
	if (rc == -1) {
		ERR(Msg(FORKFAIL,
			"0821-104: Cannot start process %2$s.\n\tThe error number is %1$d.\n"),
		    cmd, errno);
	} else {
		wait(&status);
		if (status)
			ERR(Msg(EXECFAIL,
				"0821-103:  The command %s failed.\n"),
			    cmd);
	}
	
	return(status);
}


/*
 * NAME: nextattr
 *                                                                    
 * FUNCTION: parses the next attribute/value pair (attr=val) from an
 *           attribute list.
 *                                                                    
 * NOTES:
 *   uses the attrvalpair structure. 
 *
 *
 * RETURNS:  
 *     a pointer to the attrvalpair structure of next attr/val pair found.
 *     0 if no more attr/val pairs in input list.
 */
struct attrvalpair *
nextattr(char *list)
{
	static struct attrvalpair avp;
	char *p1, *p2;
	
	if (strchr(list, '=') == 0)
		return(0);
	p1 = list + strspn(list, " "); /* find first non-space */
	p2 = p1 + strcspn(p1, " ="); /* find end of attribute name */
	strncpy(avp.attr, p1, p2-p1);
	avp.attr[p2-p1] = '\0';
	p1 = p2 + strspn(p2, " ="); /* find start of value */
	p2 = p1 + strcspn(p2, " ") - 1; /* find end of value */
	strncpy(avp.val, p1, p2-p1);
	avp.val[p2-p1] = '\0';
	strcpy(list, p2);	/* remove attr/value pair from list */
	return(&avp);
}


/*
 * NAME: errmsg
 *                                                                    
 * FUNCTION: prints out an error message with variable number of parameters.
 *                                                                    
 * RETURNS:  nothing.
 */
errmsg(a, b, c, d, e)
	char *a, *b, *c, *d, *e;
{
	fprintf(stderr, a, b, c, d, e);
}


/*
 * NAME: cfg_init_db
 *
 * FUNCTION: calls odm_set_path(), odm_initialize(), and odm_lock().
 *
 * RETURNS: nothing, exits on error.
 */
cfg_init_db(int phase)
{
	int fd;

	/* set the leds */
	if (phase == 2)
		if ((fd = open("/dev/nvram", O_RDWR)) != -1)
			ioctl(fd, MIONVLED, LEDVAL);

	if ((int)odm_set_path(OBJREPOS) == -1) {
		ERR(Msg(BADPATH,
		"0821-102: Cannot set the default path to %s.\n"),
		    OBJREPOS);
		CFGEXIT(E_ODMINIT);
	}

	if (odm_initialize() == -1) {
		ERR(Msg(INITFAIL,
		"0821-108: The database initialization failed.\n"));
		CFGEXIT(E_ODMINIT);
	}
	if (odm_lock(LOCKFILE, 0) == -1) {
		ERR(Msg(NOLOCK,
			"0821-109: Another process owns the lock on the configuration database.\n"));
		CFGEXIT(E_ODMLOCK);
	}
}


/*---------------------------------------------------------------

  tcp_getattr()

  Description: This routine is queries the Customized Attribute
  object class given a device logical name and attribute name to
  search on. If the object containing the attribute information
  exists, then this structure is returned to the calling routine.
  Otherwise, assume that the attribute took on a default value
  which is stored in the Predefined Attribute object class. This
  class is queried, and if the attribute information is found, the
  relevant information is copied into a Customized Attribute
  structure which is returned to the calling routine. If no object
  is found in Predefined Attribute object class, then NULL is
  returned indicating an error.

  Input: device logical name
	 attribute name
	 getall
	 how_many

  Returns: pointer to a structure which contains the attribute information
	   NULL if failed

  ---------------------------------------------------------------*/

#define CRITLEN 256
struct CuAt 
*tcp_getattr(char *devname,
	     char *attrname,
	     int getall,
	     int *how_many)
{
	struct CuDv	*cudv;		/* pointer to CuDv object */
	struct CuAt     *newobj;        /* pointer to CuAt object */
	struct CuAt     *rval;	/* pointer to returned CuAt object(s) */
	struct CuAt	*cuobj;		/* pointer to CuAt object */
	struct CuAt	*cuat;		/* temporary holder for CuAt object */
	struct PdAt	*pdobj;		/* pointer to PdAt object */
	struct PdAt	*pdat;		/* temporary holder for PdAt object */
	struct listinfo pdat_info;	/* info about PdAt retrieved objects */
	struct listinfo cuat_info;	/* info about CuAt retrieved objects */
	struct listinfo cudv_info;	/* info about CuDv retrieved objects */
	char   criteria[CRITLEN];	/* search criteria string */
	register int    i, j;           /* loop counters */
	int    found, listsize;

	*how_many = 0;
	/* need to query CuDv for the uniquetype */
	strcpy(criteria, "name = '");
	strcat(criteria, devname);
	strcat(criteria, "'");
	cudv = get_CuDv_list(CuDv_CLASS, criteria, &cudv_info, 1, 2);
	if ((int) cudv == -1 || cudv_info.num == 0)
	{
		ERR(Msg(GETCUDV_ERR,
	   "0821-106: Cannot get records from CuDv.\n"));
		return(NULL);
	}

	if (getall)
		sprintf(criteria, "uniquetype = '%s'", cudv->PdDvLn_Lvalue);
	else
		sprintf(criteria, "uniquetype = '%s' AND attribute = '%s'",
				     cudv->PdDvLn_Lvalue, attrname);

	pdat = get_PdAt_list(PdAt_CLASS, criteria, &pdat_info, 1, 1);
	if ((int) pdat == -1)
	{
		ERR(Msg(GETPDAT_ERR,
      "0821-107: Cannot get records from PdAt.\n"));
		return(NULL);
	}
	/* it probably really is an error if no predefined attributes */
	if (!pdat_info.num)
	{
		ERR(Msg(NOPDAT_OBJ_ERR,
	"0821-111: no attributes found in PdAt for the criteria:\n\t%s\n"), criteria);
		return(NULL);
	}

	if (getall) {
		strcpy(criteria, "name = '");
		strcat(criteria, devname);
		strcat(criteria, "'");
	} else {
		strcpy(criteria, "name = '");
		strcat(criteria, devname);
		strcat(criteria, "' AND attribute = '");
		strcat(criteria, attrname);
		strcat(criteria, "'");
	}

	cuat = get_CuAt_list(CuAt_CLASS, criteria, &cuat_info, 1, 1);
	if ((int) cuat == -1)
	{
		ERR(Msg(GETCUAT_ERR,
    "0821-105: Cannot get records from CuAt.\n"));
		return(NULL);
	}

	*how_many = cuat_info.num;
	if (!getall) /* just return one cuat */
	{
		if (cuat_info.num > 0)
			return(cuat);
		cuat = (struct CuAt *)malloc(sizeof(struct CuAt));
		if (!cuat) {
			ERR(Msg(NOMEM,
				"0821-110: There is not enough memory available for new attribute.\n"));
			return(NULL);
		}
		memcpy(cuat, 0, sizeof(struct CuAt));
		strcpy(cuat->name, devname);
		strcpy(cuat->attribute, pdat->attribute);
		strcpy(cuat->value, pdat->deflt);
		strcpy(cuat->type, pdat->type);
		strcpy(cuat->generic, pdat->generic);
		strcpy(cuat->rep, pdat->rep);
		cuat->nls_index = pdat->nls_index;
		if (cuat_info.num == 0 &&
		    *(pdat->deflt) != 0 && pdat->deflt != 0) {
			*how_many = 1;
		}
		return(cuat);
	}

	/* here we are returning all attributes, so we copy all found
	   customized objects into the returned customized list */

	listsize = (cuat_info.num + pdat_info.num) * sizeof(struct CuAt);
	newobj = (struct CuAt *) malloc(listsize);
	if (!newobj) {
		ERR(Msg(NOMEM,
			"0821-110: There is not enough memory available new attribute.\n"));
		return(NULL);
	}
	rval = newobj;		/* we return the new list */
	memmove(newobj, cuat, sizeof(struct CuAt) * cuat_info.num);
	newobj += cuat_info.num; /* move to end of list */

	/* copy all the PdAt information to a CuAt structure
	   for every predefined attribute found */

	found = FALSE;

	for (i = 0, pdobj = pdat; i < pdat_info.num; i++, pdobj++)
	{
		found = FALSE;
		for (j = 0, cuobj = cuat; j < cuat_info.num; j++, cuobj++)
		{
			if (!strcmp(cuobj->attribute, pdobj->attribute))
			{
				found = TRUE;
				break;
			}

		} /* end of for */

		if (!found && *(pdobj->deflt) != 0 && pdobj->deflt != 0)
		{
			(*how_many)++;

			memcpy(newobj, 0, sizeof(struct CuAt));
			strcpy(newobj->name, devname);
			strcpy(newobj->attribute, pdobj->attribute);
			strcpy(newobj->value, pdobj->deflt);
			strcpy(newobj->type, pdobj->type);
			strcpy(newobj->generic, pdobj->generic);
			strcpy(newobj->rep, pdobj->rep);
			newobj->nls_index = pdobj->nls_index;
			newobj++;
		}
		
	} /* end of for */
		
	/* deallocate cudv, cuat, pdat lists */
	odm_free_list(cudv, &cudv_info);
	odm_free_list(cuat, &cuat_info);
	odm_free_list(pdat, &pdat_info);

	return(rval);
		
} /* end of tcp_getattr */	


