static char sccsid[] = "@(#)74  1.7  src/bos/usr/lib/pios/piosubr3.c, cmdpios, bos41B, 9504A 1/17/95 07:52:37";
/*
 * COMPONENT_NAME: (CMDPIOS) Printer Backend
 *
 * FUNCTIONS:  piogetrefs(), piovalav(), vav_getmsg(), vav_error(),
 *	       vav_entry(), vav_list(), vav_ring(), vav_range(),
 *	       vav_validate(), vav_opcmp()
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T
 * All Rights Reserved
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
 * The copyright notice above does not evidence any
 * actual or intended publication of such source code.
 */

#define _ILS_MACROS
#include <sys/types.h>
#include <sys/limits.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <memory.h>
#include <nl_types.h>
#include <ctype.h>
#include "pioformat.h"
#include "piolimits.h"

/* Default messages */
#define DEFMSG_LIM_BADATTR	"The attribute name %.2s is not valid\n"
#define DEFMSG_LIM_ILLOP	"The limits field of %.2s attribute contains" \
				" the illegal op %c\n"
#define DEFMSG_LIM_NOBEGIN	"The limits field of %.2s attribute does not" \
				" contain a '[' after the op %c\n"
#define DEFMSG_LIM_NOEND	"The limits field of %.2s attribute does not" \
				" contain a matching ']' after the op %c\n"
#define DEFMSG_LIM_ILLENTRY_RANGE "The limits field of %.2s attribute does not"\
				" specify a valid entry type for range op %c\n"
#define DEFMSG_LIM_INVENTRY	"The limits field of %.2s attribute"\
				" specifies an invalid entry type for op %c\n"
#define DEFMSG_VAL_NOTALNMENTRY	"The value of %.2s attribute is not"\
				" of alphanumeric entry type\n"
#define DEFMSG_VAL_NOTNMRCENTRY	"The value of %.2s attribute is not"\
				" of numeric entry type\n"
#define DEFMSG_VAL_NOTHEXENTRY	"The value of %.2s attribute is not"\
				" of hexadecimal entry type\n"
#define DEFMSG_VAL_NOTFILEENTRY	"The value of %.2s attribute is not"\
				" of filename entry type\n"
#define DEFMSG_LIM_LISTGENERR	"Error in generating a list from the limits"\
				" field of %.2s attribute for list op %c\n"
#define DEFMSG_LIM_LISTREADERR	"Error in reading the list from the limits"\
				" field of %.2s attribute for list op %c\n"
#define DEFMSG_VAL_INVVAL_LIST	"The value of %.2s attribute is not in the"\
				" list generated from the limits field\n"
#define DEFMSG_LIM_RANGEFMTERR	"The limits field of %.2s attribute specifies"\
				" an invalid range format for range op %c\n"
#define DEFMSG_VAL_NOTINT	"The value of %.2s attribute is not an "\
				" integer\n"
#define DEFMSG_VAL_OUTOFBNDS	"The value of %.2s attribute falls out of "\
				" bounds specified in the range format\n"
#define DEFMSG_LIM_VALIDNOTINT	"The limits field of %.2s attribute does not"\
				" specify an integer for validate op %c\n"
#define DEFMSG_LIM_INVRINGFMT	"The limits field of %.2s attribute specifies"\
				" an invalid ring format for ring op %c\n"
#define DEFMSG_VAL_NOTINRING	"The value of %.2s attribute is not in the "\
				" ring list specified in the limits field\n"
#define DEFMSG_VAL_VALIDFAILURE	"The value %d specified in the validate op %c"\
				" in the limits field for %.2s attribute"\
				" indicates failure\n"

/* Local function declarations */
static const char	*vav_getmsg(const char *, int, int);
static void		vav_error(int, const char *, ...);
static int		vav_entry(const char *, ushort_t, const char *,
				  const char *, const opinfo_t *);
static int		vav_list(const char *, ushort_t, const char *,
				 const char *, const opinfo_t *);
static int		vav_ring(const char *, ushort_t, const char *,
				 const char *, const opinfo_t *);
static int		vav_range(const char *, ushort_t, const char *,
				  const char *, const opinfo_t *);
static int		vav_validate(const char *, ushort_t, const char *,
				     const char *, const opinfo_t *);
#if defined(SORT_OPTBL) || defined(USE_BSEARCH)
static int		vav_opcmp(const void *, const void *);
#endif				/* SORT_OPTBL || USE_BSEARCH */

/* Local variable definitions */
static const struct lktable	lktbl[] = {
					   { YES_STRING, TRUE },
					   { NO_STRING, FALSE },
					   { NULL, FALSE }
					  };
	/* Make sure that the table below is in **SORTED** order.
	   Else, provide compile time flag -DSORT_OPTBL so that qsort() is
	   performed in piovalav() function. */
	/* Also, if binary search is desired for looking up an op in the
	   table, provide compile time flag -DUSE_BSEARCH.  Else, linear
	   search is performed in piovalav() function, which is more
	   efficient for a small set of records. */
static const opinfo_t		optbl[] = {
					   { COMPOP, BRACKETOPND, NULL },
					   { DISPLAYOP, ONECHAROPND, NULL },
					   { ENTRYOP, ONECHAROPND, vav_entry },
					   { CTLISTOP, ONECHAROPND, NULL },
					   { RANGEOP, BRACKETOPND, vav_range },
					   { HELPMSGOP, BRACKETOPND, NULL },
					   { HELP1MSGOP, BRACKETOPND, NULL },
					   { LISTOP, BRACKETOPND, vav_list },
					   { MLISTOP, BRACKETOPND, vav_list },
					   { REQDOP, ONECHAROPND, NULL },
					   { RINGOP, BRACKETOPND, vav_ring },
					   { SEQNOOP, BRACKETOPND, NULL },
					   { MRINGOP, BRACKETOPND, vav_ring },
					   { VALIDATEOP, BRACKETOPND,
					     vav_validate }
					  };
static char			*mbufp;		/* msg buffer pointer */
static char			noentry;	/* flag for no entry op */
static char			range;		/* flag for range op */
static char			ctlmode;	/* flag for cmd_to_list_mode */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           piogetrefs                                                   *
*                                                                              *
* DESCRIPTION:    Fetch referenced attributes (or just flags) for a given      *
*                 attribute (typically a pipeline, ex. "ia").  This function   *
*                 performs piogetstr() on the given attribute, and marks all   *
*                 the referenced attributes by going through the 'flags' field *
*                 of all the attributes (or just flags) to see if the field    *
*                 is marked with USEDBYSOMEONE.  Then, it builds a list of     *
*                 these and returns back a pointer to it.  However, since      *
*                 'flags' field will be checked for each attribute (or flag),  *
*                 this function will initially save all the original flags     *
*                 the attributes (or just flags).  And, after marking the      *
*                 referenced attributes, it will restore the original flag     *
*                 values.                                                      *
*                                                                              *
* PARAMETERS:     attrnm	attribute name                                 *
*                 flag		flag to denote if all attributes (or just      *
*                 		flags) referenced should be returned           *
*                 		( 0 - all ; else - flags only )                *
*                                                                              *
* RETURN VALUES:  < 0		error                                          *
*                 0		no referenced attributes                       *
*                 > 0		number of referenced attributes                *
*                                                                              *
*******************************************************************************/
int
piogetrefs(const char *attrnm, int flag, char (**refattrs)[ATTRNAMELEN])
{
#define SAVE_FLAGS \
   do { \
      /* Save (and nullify) the 'flags' field values for all the attributes. */\
      MALLOC(vp, objtot*sizeof(*sflags)); \
      sp = sflags = (short *)vp; \
      for (ap = attr_table; ap < attr_table+objtot; ) \
         *sp++ = ap->flags, \
         ap++->flags = 0; \
      sav_piomode = piomode; \
   } while(0)
#define RESTORE_FLAGS \
   do { \
      /* Restore the original 'flags' field values for all the attributes. */ \
      for (ap = attr_table, sp = sflags; ap < attr_table+objtot; ap++, sp++) \
         ap->flags = *sp; \
      piomode = sav_piomode; \
      free((void *)sflags); \
   } while(0)
   register struct odm		*op = (struct odm *)odm_table;
   register struct attr_info	*ap;
   short			*sflags;
   register short		*sp;
   void				*vp;
   char				wkbuf[WKBUFLEN+1];
   register char		(*rp)[ATTRNAMELEN];
   int				refano = 0;
   int				sav_piomode;

   /* Save (and nullify) the 'flags' field values for all the attributes. */
   SAVE_FLAGS;

   /* Evaluate the value of the given attribute, thus marking all the
      referenced attributes. */
   if (piogetstr((char *)attrnm, wkbuf, (int)(sizeof wkbuf), NULL) < 0)
   {
      RESTORE_FLAGS;
      return ERRVAL;
   }

   /* Determine the no of referenced attributes. */
   for (ap = attr_table; op < (struct odm *)odm_table+objtot; ap++, op++)
      if ((!flag  ||  *op->name == *FLAGCHAR)  &&  ap->flags & USEDBYSOMEONE)
	 refano++;

   /* Build a list of referenced attributes. */
   if (refano)
   {
      MALLOC(vp, refano*sizeof(*rp));
      *refattrs = rp = (char (*)[ATTRNAMELEN])vp;
      for (ap = attr_table, op = (struct odm *)odm_table;
	   op < (struct odm *)odm_table+objtot; ap++, op++)
         if ((!flag  ||  *op->name == *FLAGCHAR)  &&  ap->flags & USEDBYSOMEONE)
	    **rp = *op->name, *(*rp+++1) = *(op->name+1);
   }
   else
      *refattrs = NULL;

   /* Restore the original 'flags' field values for all the attributes. */
   RESTORE_FLAGS;

   return refano;
}	/* end - piogetrefs() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           piovalav                                                     *
*                                                                              *
* DESCRIPTION:    Validate an attribute value against its limit string.        *
*                 Initially, evaluate the value string and the limit string    *
*                 of the given attribute.  Then perform validation, storing    *
*                 error messages in a static buffer, if any encountered.       *
*                 If validation is completely successful, return NULL.  Else,  *
*                 return pointer to the static message buffer.                 *
*                 The following assumptions are made for validation:           *
*                 	a) if limits field is a null string, return;           *
*                 	b) if limits field contains a 'V' op, evaluate the     *
*                 	   expression (within brackets) and use the result     *
*                 	   for validation success/failure;                     *
*                 	c) if limits field contains an 'E' op with an arg      *
*                 	   other than 'n' (i.e., not a non-entry field),       *
*                 	   further validation is skipped for ring and list     *
*                 	   type fields; however, the corresponding entry type  *
*                 	   (ex. 'E#' for numeric) validation is performed;     *
*                 	d) if 'E' is not specified, 'Et' is considered to be   *
*                 	   the default, and validation is performed as         *
*                 	   specified in the limits field;                      *
*                 	e) for range type fields, validation is always         *
*                 	   performed (typically, entry type would be 'E#/Ex'); *
*                 	f) for numeric values, the following lookup values     *
*                 	   are always applied by default:                      *
*                 		 	+	=	1                      *
*                 		 	!	=	0                      *
*                 	g) listed below are various ops and their formats:     *
*                 		1) E?                                          *
*                 			- where ? can be                       *
*                 				n  for  "no entry"             *
*                 				t  for  "text entry"           *
*                 				#  for  "numeric entry"        *
*                 				x  for  "hex entry"            *
*                 				f  for  "file entry"           *
*                 				r  for  "alphanumeric entry"   *
*                 		2) L[...]  M[...]                              *
*                 			- list and multi-select list           *
*                 		   expression within the brackets will be      *
*                 		   run as a ksh command, which should generate *
*                 		   a list of newline-separated choices against *
*                 		   which the attribute value is validated;     *
*                 		3) F?	     -	command to list mode           *
*				   	- where ? can be                       *
*						1  for  first field	       *
*						2  for  second field	       *
*						a  for  all fields	       *
*                 		4) R[msgcat,setno,msgno;...=...]               *
*                 		   T[msgcat,setno,msgno;...=...]               *
*                 			- ring and multi-select ring           *
*                 		   msg catalog info and disp values are        *
*                 		   mutually optional; aix values are optional; *
*                 		5) G[lower..higher]                            *
*                 			- range list                           *
*                 		   lower and upper bounds are mutually         *
*                 		   optional; upper bound must be >= lower one; *
*                 		6) V[...]                                      *
*                 			- validating expression                *
*                 		   expression is evaluated and the result      *
*                 		   indicates success/failure                   *
*                 			0	-	success                *
*                 			else	-	failure                *
*                 		7) ops used for building SMIT dialogs, but not *
*                 		   used in validation                          *
*                 		   	S[...]	     -	sequence no.           *
*                 		   	Dy or Dn     -	display mode           *
*                 		   	Q?	     -	required               *
*                 			C[a1,a2,...] -	composite flag         *
*                 			H[cat,set,msg] - help msg specs        *
*                 			I[id,base,book] - help msg specs       *
*                                                                              *
* PARAMETERS:     anm		attribute name                                 *
*                                                                              *
* RETURN VALUES:  NULL		successful                                     *
*                 mbuf		validation failed; error messages are returned *
*                                                                              *
*******************************************************************************/
char *
piovalav(const char *anm)
{
#define ADD_OPNDINFO_ELEMENT(cp, len, oip) \
			do { \
			   MALLOC(odip, sizeof(opndinfo_t)); \
			   odip->nextp = NULL; \
			   odip->opndp = cp; \
			   odip->opndlen = len; \
			   odip->oi = *oip; \
			   if (!opndhead) \
			      opndtail = opndhead = odip; \
			   else \
			      opndtail->nextp = odip, \
			      opndtail = odip; \
			} while(0)
   struct odm			*op;
   char				val[10*(WKBUFLEN+1)];
   char				lim[10*(WKBUFLEN+1)];
   const char			*valp;
   const char			*limp;
   register const char		*cp;
   const char			*endp;
   register int			errsno = 0;
   opndinfo_t			*opndhead = NULL;
   opndinfo_t			*opndtail;
   register opndinfo_t		*odip;
   register opndinfo_t		*savodip;
   size_t			maxlen;
#  ifdef SORT_OPTBL
   static char			sortedflag = FALSE;
#  endif		/* ifdef SORT_OPTBL */
   register struct attr_info	*ap;
   short			*sflags;
   register short		*sp;
   void				*vp;
   int				sav_piomode;

   noentry = FALSE, range = FALSE, ctlmode = 'a';

   if (mbufp)
      free(mbufp), mbufp = NULL;

   if (!(op = get_odmtab_ent((char *)anm)))
   {
      vav_error(MSG_LIM_BADATTR, DEFMSG_LIM_BADATTR, anm);
      return mbufp;
   }

   /* If limit string is a null string, validation need not be done. */
   if (!op->limlength)
      return NULL;

   /* Save (and nullify) the 'flags' field values for all the attributes. */
   SAVE_FLAGS;

   /* Evaluate the value and limit strings. */
   if (piogetstr((char *)anm, val, (int)(sizeof val), NULL) < 0)
   {
      vav_error(MSG_LIM_BADATTR, DEFMSG_LIM_BADATTR, anm);
      RESTORE_FLAGS;
      return mbufp;
   }
   valp = val;
   if (memchr((void *)(limp = (const char *)GET_ATTR_LIMSTR(op)), ESCCHAR,
	      (size_t)op->limlength))
   {
      char				panm[ATTRNAMELEN+1] = "#";

      *(panm+1) = *anm, *(panm+2) = *(anm+1);
      if (piogetstr(panm, lim, (int)(sizeof lim), NULL) < 0)
      {
         vav_error(MSG_LIM_BADATTR, DEFMSG_LIM_BADATTR, anm);
	 RESTORE_FLAGS;
         return mbufp;
      }
      limp = lim;
   }
   else
   {
      (void) memcpy((void *)lim, (void *)limp,
		    maxlen = MIN((size_t)op->limlength, sizeof(lim)-1)),
      *(lim+maxlen) = 0;
      limp = lim;
   }
#  ifdef DEBUG
   (void) printf("Attribute name = %.2s; value = %s; limit = %s\n",
		 anm,valp,limp);
#  endif		/* DEBUG */

#  ifdef SORT_OPTBL
   if (!sortedflag)
      qsort((void *)optbl, ARRAYSIZ(optbl), sizeof(*optbl), vav_opcmp),
      sortedflag = TRUE;
#  endif		/* ifdef SORT_OPTBL */

   /* Parse the limits field, and check if entry type ('E') and range type
      ('G') ops are specified.  At the same time, build a list of operand
      information elements which will be used to perform actual processing.
      If any unrecognized ops appear, flag them as error. */
   for (cp = limp, endp = limp+strlen(limp); cp < endp; )
   {
      register const opinfo_t		*oip;
#     ifdef USE_BSEARCH
      static opinfo_t			toi = { 0, ONECHAROPND, NULL };
#     endif		/* ifdef USE_BSEARCH */

#     ifdef USE_BSEARCH
      if (toi.optype = *cp, !(oip = (const opinfo_t *)
	  bsearch((const void *)&toi, (const void *)optbl, ARRAYSIZ(optbl),
		  sizeof(*optbl), vav_opcmp)))
#     else		/* !ifdef USE_BSEARCH */
      for (oip = optbl; oip < optbl+ARRAYSIZ(optbl); oip++)
	 if (oip->optype == *cp)
	    break;
      if (oip == optbl+ARRAYSIZ(optbl))
#     endif		/* ifdef USE_BSEARCH */
      {
	 vav_error(MSG_LIM_ILLOP, DEFMSG_LIM_ILLOP, anm, (char)*cp);
	 errsno++;
	 cp++;
	 continue;
      }
      switch (oip->optype)
      {
	 case ENTRYOP:				/* 'E' */
	    if (*++cp == 'n')
	       noentry = TRUE;
	    ADD_OPNDINFO_ELEMENT(cp, 1, oip);
	    cp++;
	    break;

	 case CTLISTOP:				/* 'F' */
	    ctlmode = *++cp;
	    ADD_OPNDINFO_ELEMENT(cp, 1, oip);
	    cp++;
	    break;

	 case RANGEOP:				/* 'G' */
	    if (*(cp+1) != SETBEGINCHAR)
	    {
	       vav_error(MSG_LIM_NOBEGIN, DEFMSG_LIM_NOBEGIN, anm, (char)*cp);
	       errsno++;
	       cp++;
	       continue;
	    }
	    range = TRUE;		/* fall through */

	 default:
	    if (oip->opndtype == ONECHAROPND)
	    {
	       ++cp;
	       ADD_OPNDINFO_ELEMENT(cp, 1, oip);
	       cp++;
	    }
	    else
	    {
	       int		bkslsno = 0;
	       char		endfound = FALSE;
	       const char	*savcp;

	       if (*++cp != SETBEGINCHAR)
	       {
	          vav_error(MSG_LIM_NOBEGIN, DEFMSG_LIM_NOBEGIN, anm,
			    (char)*(cp-1));
	          errsno++;
	          continue;
	       }
	       for (savcp = ++cp; cp < endp  &&  !endfound; cp++)
		  switch(*cp)
		  {
		     case '\\':
			bkslsno++;
			break;

		     case SETENDCHAR:
			if (!(bkslsno & 0x1))
			{
			   endfound = TRUE;
			   break;
			}
			bkslsno = 0;
			break;

		     default:
			bkslsno = 0;
			break;
		  }
	       if (!endfound)
	       {
	          vav_error(MSG_LIM_NOEND, DEFMSG_LIM_NOEND, anm, oip->optype);
	          errsno++;
		  cp = savcp;
	          continue;
	       }
	       ADD_OPNDINFO_ELEMENT(savcp, cp-savcp-1, oip);
	    }
      }
   }

   /* Check for a few valid conditions. */
   if (range  &&  noentry)
   {
      vav_error(MSG_LIM_ILLENTRY_RANGE, DEFMSG_LIM_ILLENTRY_RANGE, anm,
		RANGEOP);
      errsno++;
   }

   /* Parse the operand info list that was built in the previous parsing
      step.  Make sure that the assumptions stated in the logic are observed. */
   for (odip = opndhead; odip; odip = odip->nextp)
      if (odip->oi.opproc  &&
	  !odip->oi.opproc(odip->opndp, odip->opndlen, anm, valp, &odip->oi))
	 errsno++;

   /* Free the operand info list elements. */
   for (odip = opndhead; odip; odip = savodip)
      savodip = odip->nextp,
      free(odip);

   /* Restore the original 'flags' field values for all the attributes. */
   RESTORE_FLAGS;

   return (errsno ? mbufp : NULL);
#undef RESTORE_FLAGS
#undef SAVE_FLAGS
#undef ADD_OPNDINFO_ELEMENT
}	/* end - piovalav() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           vav_getmsg                                                   *
*                                                                              *
* DESCRIPTION:    Fetch an error message and put it in a msg buffer.           *
*                                                                              *
* PARAMETERS:     catnm		catalog name                                   *
*                 setno		set no.                                        *
*                 msgno		message no.                                    *
*                                                                              *
* RETURN VALUES:  message	success                                        *
*                 NULL		failure                                        *
*                                                                              *
*******************************************************************************/
static char const *
vav_getmsg(const char *catnm, int setno, int msgno)
{
   static char			prevcatnm[FILENAME_MAX+1];
   static nl_catd		md = (nl_catd)-1;
   char				defmcpath[PATH_MAX+1];
   const char			*const dmsg = "d u m m y";
   char				*mp;

   if (strcmp(catnm, prevcatnm))
   {
      (void) strncpy(prevcatnm, catnm, sizeof(prevcatnm)-1),
      *(prevcatnm+sizeof(prevcatnm)-1) = 0;
      if (md != (nl_catd)-1)
         md = ((void) catclose(md), (nl_catd)-1);
   }

   if (md == (nl_catd)-1  &&
       (md = catopen((char *)catnm, NL_CAT_LOCALE)) == (nl_catd)-1)
   {
      (void) strncpy(defmcpath, DEFMC_PREFIXPATH, sizeof(defmcpath)-1),
      *(defmcpath+sizeof(defmcpath)-1) = 0;
      (void) strncat(defmcpath, catnm, sizeof(defmcpath)-strlen(defmcpath)-1),
      *(defmcpath+sizeof(defmcpath)-1) = 0;
      if ((md = catopen(defmcpath, NL_CAT_LOCALE)) == (nl_catd)-1)
	 return NULL;
   }

   return strcmp(mp = catgets(md, setno, msgno, (char *)dmsg),
		 (char const *)dmsg) ? mp : NULL;
}	/* end - vav_getmsg() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           vav_error                                                    *
*                                                                              *
* DESCRIPTION:    Fetch an error message and put it in a msg buffer.           *
*                                                                              *
* PARAMETERS:     msgno		message no.                                    *
*                 defmsg	default error message                          *
*                                                                              *
* RETURN VALUES:                                                               *
*                                                                              *
*******************************************************************************/
static void
vav_error(int msgno, const char *defmsg, ...)
{
   static FILE		*tmpfp;
   static size_t	memacqd;
   static size_t	memused;
   va_list		vap;
   const char		*msgp;
   ushort_t		parseerr = FALSE;
   int			msgcnt;

   if (!mbufp)
      memacqd = memused = 0;

   if (!(msgp = vav_getmsg(MF_PIOBE, MF_PIOBESETNO, msgno)))
      msgp = defmsg;

   if (!tmpfp)
      tmpfp = tmpfile();
   else
      rewind(tmpfp);

   if (tmpfp)
   {
      va_start(vap, defmsg);
      if ((msgcnt = vfprintf(tmpfp, msgp, vap)) < 0)
	 parseerr = TRUE;
      va_end(vap);
   }
   else
      parseerr = TRUE;

   if (parseerr)
      msgcnt = strlen(msgp);

   while (memacqd < memused+msgcnt+1)
   {
      REALLOC(mbufp, mbufp, memacqd+MSGBUFLEN);
      memacqd += MSGBUFLEN;
   }

   if (!parseerr)
   {
      register size_t		trdcnt;
      register const char	*cp;
      size_t			nread;

      for ((void) rewind(tmpfp), trdcnt = (size_t)msgcnt, cp = mbufp+memused;
	   trdcnt > 0;
	   cp += nread, trdcnt -= nread)
      {
         nread = trdcnt < RDBUFSIZE ? trdcnt : RDBUFSIZE;
         if (fread((void *)cp, nread, (size_t)1, tmpfp) != (size_t)1  ||
	     ferror(tmpfp))
	 {
	    parseerr = TRUE;
	    break;
	 }
      }
   }
   else
      (void) memcpy((void *)(mbufp+memused), (void *)msgp, (size_t)msgcnt);
   *(mbufp+memused+msgcnt) = 0;

   memused += msgcnt;

   return;
}	/* end - vav_error() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           vav_entry                                                    *
*                                                                              *
* DESCRIPTION:    Validate entry type operation.                               *
*                                                                              *
* PARAMETERS:     limp		pointer to limit string                        *
*                 len		length of operand                              *
*                 an		attribute name                                 *
*                 valp		pointer to value string                        *
*                 oip		pointer to operation info                      *
*                                                                              *
* RETURN VALUES:  FALSE		failure                                        *
*                 TRUE		success                                        *
*                                                                              *
*******************************************************************************/
static int
vav_entry(const char *limp, ushort_t len, const char *an,
	  register const char *valp, const opinfo_t *oip)	/*ARGSUSED*/
{
   /* For range fields, ensure that entry type is numeric. */
   if (range  &&  !(*limp == '#' || *limp == 'x'))
   {
      vav_error(MSG_LIM_ILLENTRY_RANGE, DEFMSG_LIM_ILLENTRY_RANGE, an, RANGEOP);
      return FALSE;
   }

   /* Skip further processing for range fields, as the corresponding
      validation will be done in range-processing function. */
   if (range)
      return TRUE;

   /* Process the entry type and perform validation. */
   switch(*limp)
   {
      case 'n':			/* no entry */
	 /* No validation is necessary. */
	 break;

      case 't':			/* text entry */
	 /* No validation is necessary as any text is valid. */
	 break;

      case 'r':			/* alphanumeric entry */
	 for ( ; *valp; valp++)
	    if (!(isalnum((int)*valp) || isspace((int)*valp)))
	    {
	       vav_error(MSG_VAL_NOTALNMENTRY, DEFMSG_VAL_NOTALNMENTRY, an);
	       return FALSE;
	    }
	 break;

      case '#':			/* numeric */
	 for ( ; *valp; valp++)
	    if (!isdigit((int)*valp))
	    {
	       vav_error(MSG_VAL_NOTNMRCENTRY, DEFMSG_VAL_NOTNMRCENTRY, an);
	       return FALSE;
	    }
	 break;

      case 'x':			/* hexadecimal */
	 for ( ; *valp; valp++)
	    if (!isxdigit((int)*valp))
	    {
	       vav_error(MSG_VAL_NOTHEXENTRY, DEFMSG_VAL_NOTHEXENTRY, an);
	       return FALSE;
	    }
	 break;

      case 'f':			/* file entry */
	 {
	    struct stat		fst;

	    if (stat(valp, &fst) == -1)
	    {
	       vav_error(MSG_VAL_NOTFILEENTRY, DEFMSG_VAL_NOTFILEENTRY, an);
	       return FALSE;
	    }
	 }
	 break;

      default:			/* none of the above */
	 vav_error(MSG_LIM_INVENTRY, DEFMSG_LIM_INVENTRY, an, oip->optype);
	 return FALSE;
   }

   return TRUE;
}	/* end - vav_entry() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           vav_list                                                     *
*                                                                              *
* DESCRIPTION:    Validate list operation.                                     *
*                                                                              *
* PARAMETERS:     limp		pointer to limit string                        *
*                 len		length of operand                              *
*                 an		attribute name                                 *
*                 valp		pointer to value string                        *
*                 oip		pointer to operation info                      *
*                                                                              *
* RETURN VALUES:  FALSE		failure                                        *
*                 TRUE		success                                        *
*                                                                              *
*******************************************************************************/
static int
vav_list(const char *limp, ushort_t len, const char *an, const char *valp,
	 const opinfo_t *oip)
{
   FILE				*pfp;
   char				wkbuf[WKBUFLEN+1];
   size_t			maxlen;
   register char		*cp;

   /* If type-in entry is allowed, skip list validation. */
   if (!noentry)
      return TRUE;

   /* Execute the list command and check to see if the attribute value is
      one of the choices generated. */
   (void) strncpy(wkbuf, limp, maxlen = MIN((size_t)len, sizeof(wkbuf)-1)),
   *(wkbuf+maxlen) = 0;
   if (!(pfp = popen(wkbuf, "r")))
   {
      vav_error(MSG_LIM_LISTGENERR, DEFMSG_LIM_LISTGENERR, an, oip->optype);
      return FALSE;
   }
   while (fgets(wkbuf, (int)sizeof wkbuf, pfp))
   {
      (void) strtok(wkbuf, "\n");
      switch (ctlmode)
      {
	 case '1':			/* first field */
	    if ((cp = strtok(wkbuf, WHSPCHRS), cp) && !strcmp(valp, cp))
	       return TRUE;
	    break;
	 case '2':			/* second field */
	    if ((cp = strtok(wkbuf, WHSPCHRS), cp) &&
		(cp = strtok(NULL, WHSPCHRS), cp) && !strcmp(valp, cp))
	       return TRUE;
	    break;
	 default:			/* entire string */
	    if (!strcmp(valp, wkbuf))
	       return TRUE;
      }
   }
   if (ferror(pfp))
   {
      vav_error(MSG_LIM_LISTREADERR, DEFMSG_LIM_LISTREADERR, an, oip->optype);
      return FALSE;
   }

   return ((void) pclose(pfp),
	   vav_error(MSG_VAL_INVVAL_LIST, DEFMSG_VAL_INVVAL_LIST, an), FALSE);
}	/* end - vav_list() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           vav_ring                                                     *
*                                                                              *
* DESCRIPTION:    Validate ring operation.                                     *
*                                                                              *
* PARAMETERS:     limp		pointer to limit string                        *
*                 len		length of operand                              *
*                 an		attribute name                                 *
*                 valp		pointer to value string                        *
*                 oip		pointer to operation info                      *
*                                                                              *
* RETURN VALUES:  FALSE		failure                                        *
*                 TRUE		success                                        *
*                                                                              *
*******************************************************************************/
static int
vav_ring(const char *limp, ushort_t len, const char *an, const char *valp,
	 const opinfo_t *oip)
{
   const char			*avls;
   const char			*endp = limp+len;
   const char			*cp;
   char				wkbuf[10*WKBUFLEN+1];
   int				val;
   register const struct lktable *lkp;
   char				lkflag = FALSE;
   size_t			maxlen;
   char				lkflag1 = FALSE; /* mutually xcl. with lkflag */

   /* If type-in entry is allowed, skip ring validation. */
   if (!noentry)
      return TRUE;

   /* Parse the ring list, and make sure that the attribute value is one of
      the aix values listed in the ring list.
      Notes:	1. The format is:
		   R[catname,setno,msgno;d1,d2,...=a1,a2,...]
		2. By default, if provided, use the real aix values for
		   ring list choices.
		2. In case aix values are missing, use message catalog info.,
		   if provided.
		3. However, if real aix values and message catalog info. is
		   not provided, then use the real disp values.  If these
		   are also missing, flag it as an error.
    */
   if (avls = memchr((void *)limp, RINGAIXVSEP, (size_t)len))
      avls++;
   else
   {
      if (cp = memchr((void *)limp, RINGMSGSEP, (size_t)len))
      {
	 if (cp == limp)
	    avls = ++cp;
	 else
	 {
	    char 		mcnm[PATH_MAX+1] = RING_MC;
	    int			setno = RING_MCSETNO;
	    int			msgno;
	    register const char	*sp;
	    const char		*tp;

	    do
	    {
	       /* Get msg no. */
	       for (sp = cp-1; *sp != RINGLISTSEP; )
		  if (sp == limp)
		     break;
		  else
		     sp--;
	       if (msgno = (int)strtol(tp = *sp == RINGLISTSEP ? sp+1 : sp,
				       (char **)&tp, 10), *tp != RINGMSGSEP)
	       {
		  vav_error(MSG_LIM_INVRINGFMT, DEFMSG_LIM_INVRINGFMT,
			    an, oip->optype);
		  return FALSE;
	       }
	       if (*sp != RINGLISTSEP  ||  sp == limp)
		  break;

	       /* Get set no. */
	       for (--sp; *sp != RINGLISTSEP; )
		  if (sp == limp)
		     break;
		  else
		     sp--;
	       if (setno = (int)strtol(tp = *sp == RINGLISTSEP ? sp+1 : sp,
				       (char **)&tp, 10), *tp != RINGLISTSEP)
	       {
		  vav_error(MSG_LIM_INVRINGFMT, DEFMSG_LIM_INVRINGFMT,
			    an, oip->optype);
		  return FALSE;
	       }
	       if (*sp != RINGLISTSEP  ||  sp == limp)
		  break;

	       /* Get message catalog name. */
	       (void) memcpy((void *)mcnm, (void *)limp,
			     /***  COMPILER ERROR! ***/
			     /*
			     maxlen = MIN(sp-limp, sizeof(mcnm)-1)),
			      */
			     maxlen = MIN((char *)sp-limp, sizeof(mcnm)-1)),
	       *(mcnm+maxlen) = 0;
	    } while (0);

	    avls = (sp = vav_getmsg(mcnm, setno, msgno)) ?
		   endp = sp+strlen(sp), sp : ++cp;
	 }
      }
      else
	 avls = limp;
   }

   if (avls >= endp)
   {
      vav_error(MSG_LIM_INVRINGFMT, DEFMSG_LIM_INVRINGFMT, an, oip->optype);
      return FALSE;
   }

   /* Validate the attribute value against the list. */
   if (*valp)
      if (val = (int)strtol(valp, (char **)&cp, 10), !*cp)
         lkflag = TRUE;
      else
	 for (lkp = lktbl; lkp->str; lkp++)
	    if (!strcmp(lkp->str, valp))
	    {
	       lkflag1 = TRUE;
	       break;
	    }
   (void) memcpy((void *)wkbuf, (void *)avls,
		 /***  COMPILER ERROR! ***/
		 /*
		 maxlen = MIN(endp-avls, sizeof(wkbuf)-1)),
		  */
		 maxlen = MIN((char *)endp-avls, sizeof(wkbuf)-1)),
   *(wkbuf+maxlen) = 0;
   for (cp = strtok(wkbuf, LISTSEPCHRSTR); cp; cp = strtok(NULL, LISTSEPCHRSTR))
   {
      if (!strcmp(cp, valp))
	 return TRUE;
      if (lkflag1)
      {
	 if (val = (int)strtol(cp, (char **)&endp, 10),
	     !*endp && val == lkp->value)
	    return TRUE;
	 continue;
      }
      if (!lkflag)
	 continue;
      for (lkp = lktbl; lkp->str  &&  strcmp(lkp->str, cp); lkp++)
	 ;
      if (lkp->str  &&  lkp->value == val)
	 return TRUE;
   }

   return (vav_error(MSG_VAL_NOTINRING, DEFMSG_VAL_NOTINRING, an, oip->optype),
	   FALSE);
}	/* end - vav_ring() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           vav_range                                                    *
*                                                                              *
* DESCRIPTION:    Validate range operation.                                    *
*                                                                              *
* PARAMETERS:     limp		pointer to limit string                        *
*                 len		length of operand                              *
*                 an		attribute name                                 *
*                 valp		pointer to value string                        *
*                 oip		pointer to operation info                      *
*                                                                              *
* RETURN VALUES:  FALSE		failure                                        *
*                 TRUE		success                                        *
*                                                                              *
*******************************************************************************/
static int
vav_range(const char *limp, ushort_t len, const char *an, const char *valp,
	  const opinfo_t *oip)
{
   long				val;
   long				lbnd;
   long				ubnd;
   char				*cp;
   char				wkbuf[WKBUFLEN+1];
   size_t			maxlen;
   uchar_t			lspcd;
   uchar_t			uspcd;

   /* Parse the lower and upper bounds, and ensure that the value is between
      the bounds. */
   (void) strncpy(wkbuf, limp, maxlen = MIN((size_t)len, sizeof(wkbuf)-1)),
   *(wkbuf+maxlen) = 0;
   if ((lspcd = *wkbuf != RANGELISTSEP, lbnd = strtol(wkbuf, &cp, 10),
	*cp != RANGELISTSEP)  ||
       ((cp += 2) > wkbuf+strlen(wkbuf))  ||
       (uspcd = *cp, ubnd = strtol(cp, &cp, 10), *cp))
   {
      vav_error(MSG_LIM_RANGEFMTERR, DEFMSG_LIM_RANGEFMTERR, an, oip->optype);
      return FALSE;
   }
   if (val = strtol(valp, &cp, 10), *cp)
   {
      vav_error(MSG_VAL_NOTINT, DEFMSG_VAL_NOTINT, an);
      return FALSE;
   }
   if (lspcd && val < lbnd  ||  uspcd && val > ubnd)
   {
      vav_error(MSG_VAL_OUTOFBNDS, DEFMSG_VAL_OUTOFBNDS, an);
      return FALSE;
   }

   return TRUE;
}	/* end - vav_range() */



/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           vav_validate                                                 *
*                                                                              *
* DESCRIPTION:    Validate validation operation.                               *
*                                                                              *
* PARAMETERS:     limp		pointer to limit string                        *
*                 len		length of operand                              *
*                 an		attribute name                                 *
*                 valp		pointer to value string                        *
*                 oip		pointer to operation info                      *
*                                                                              *
* RETURN VALUES:  FALSE		failure                                        *
*                 TRUE		success                                        *
*                                                                              *
*******************************************************************************/
static int
vav_validate(const char *limp, ushort_t len, const char *an, const char *valp,
	     const opinfo_t *oip)	/*ARGSUSED*/
{
   long				val;
   char				*cp;
   char				wkbuf[WKBUFLEN+1];
   size_t			maxlen;

   /* If the limits string is a 0, validation is successful.  Else, it is
      not. */
   (void) strncpy(wkbuf, limp, maxlen = MIN((size_t)len, sizeof(wkbuf)-1)),
   *(wkbuf+maxlen) = 0;
   if (val = strtol(wkbuf, &cp, 10), *cp)
   {
      vav_error(MSG_LIM_VALIDNOTINT, DEFMSG_LIM_VALIDNOTINT, an, oip->optype);
      return FALSE;
   }

   return (val ? vav_error(MSG_VAL_VALIDFAILURE, DEFMSG_VAL_VALIDFAILURE, val,
			   oip->optype, an), FALSE : TRUE);
}	/* end - vav_validate() */



#if defined(SORT_OPTBL) || defined(USE_BSEARCH)
/*******************************************************************************
*                                                                              *
*                                                                              *
* NAME:           vav_opcmp                                                    *
*                                                                              *
* DESCRIPTION:    Compare two optbl elements.                                  *
*                                                                              *
* PARAMETERS:     cvp1		optbl element 1                                *
*                 cvp2		optbl element 2                                *
*                                                                              *
* RETURN VALUES:  -ve		ope1 < ope2                                    *
*                 0		ope1 == ope2                                   *
*                 +ve		ope1 > ope2                                    *
*                                                                              *
*******************************************************************************/
static int
vav_opcmp(const void *cvp1, const void *cvp2)
{
   return (int)((opinfo_t const *)cvp1)->optype-
      (int)((opinfo_t const *)cvp2)->optype;
}	/* end - vav_opcmp() */
#endif				/* SORT_OPTBL || USE_BSEARCH */

