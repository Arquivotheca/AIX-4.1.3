static char sccsid[] = "@(#)23	1.3.2.5  src/bos/usr/ccs/lib/libc/replacement.c, libcstr, bos411, 9428A410j 3/30/94 15:01:44";
/*
 *   COMPONENT_NAME: LIBCSTR
 *
 *   FUNCTIONS: do_replacement
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#if 0

#pragma alloca

#define _ILS_MACROS
#include <sys/localedef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <regex.h>
#include <ctype.h>

/*******************************************************
  FUNCTION: do_replacement
  PURPOSE: does replacement strings for collation
*******************************************************/
char *do_replacement(_LC_collate_objhdl_t hdl, const char *str, int order, char *outstr)
{
    char *replacement_ptr;
    char *outbuf_ptr;
    char *outbuf[2];
    char *str_ptr;
    char *ptr;
    char temp[2];
    int i;
    int space_available;
    int space_used;
    int j;
    int subs;
    char Reg_Exp=0;
    unsigned char backref;
    unsigned char buffer;
    
    int status;
    regex_t cmp_reg;
    regmatch_t match_reg[10];
    short order_value;

    /**********
      set up the pointers to the original string and
      the return string
    **********/
    str_ptr = str;
    buffer = 0;
    outbuf[0] = (char *)NULL;
    outbuf[1] = (char *)NULL;

    space_available = strlen(str)*4 * MB_CUR_MAX;

    /*********
      for each sub string, compile the pattern and try to match it in str
    **********/
    for (subs=0; subs<__OBJ_DATA(hdl)->co_nsubs; subs++) {

	/**********
	  check if this sub string is used in this order
	**********/
	if (__OBJ_DATA(hdl)->co_nord < 2)
	    order_value = __OBJ_DATA(hdl)->co_subs[subs].ss_act.n[order];
	else 
	    order_value = __OBJ_DATA(hdl)->co_subs[subs].ss_act.p[order];
	
	/**********
	  determine which buffer to put the output in
	**********/
	buffer = subs % 2;
	if (outbuf[buffer] == (char *)NULL){
	    if (((outbuf[buffer] = (char *)alloca(space_available+1))) ==
		(char *)NULL){
		perror("alloca");
		strcpy(outstr, str);
		return(outstr);
	    }
	}
	if (!(order_value & _SUBS_ACTIVE)) {
		str_ptr = outbuf[buffer];
		continue;
	}
	outbuf_ptr = outbuf[buffer];


	/**********
	  if this is not a regular-expressions, just do a sub
	**********/
	if (!(order_value & _SUBS_REGEXP)) {
	    while((ptr=strstr(str_ptr,__OBJ_DATA(hdl)->co_subs[subs].ss_src)) !=(char *)NULL){
		strncpy(outbuf_ptr, str_ptr, (ptr - str_ptr));
		outbuf_ptr += ptr-str_ptr;
		strcpy(outbuf_ptr, __OBJ_DATA(hdl)->co_subs[subs].ss_tgt);
		outbuf_ptr += strlen(__OBJ_DATA(hdl)->co_subs[subs].ss_tgt);
		str_ptr = ptr + strlen(__OBJ_DATA(hdl)->co_subs[subs].ss_src);
	    }
	}
	/**********
	  could be a regular expression
	**********/
	else {
	    /**********
	      compile the pattern, if it fails, return the string
	      **********/
	    Reg_Exp = 1;
	    if (regcomp(&cmp_reg, __OBJ_DATA(hdl)->co_subs[subs].ss_src, 0) != 0) {
		/**********
		  free the outbuffers
		  **********/
		(void) regfree(&cmp_reg);
		strcpy(outstr, str);
		return(outstr);
	    }
	    
	    /**********
	      go thru the string searching for matches.
	      **********/
	    status = regexec(&cmp_reg, str_ptr, 10, &match_reg[0], 0);
	    while (status == 0) {
		/**********
		  place everything before the match in the sub string
		  *********/
		for (i=0; i<match_reg[0].rm_so; i++)
		    *outbuf_ptr++ = str_ptr[i];
		
		/*********
		  we are sitting at the part to be replaced
		  **********/
		for (replacement_ptr=__OBJ_DATA(hdl)->co_subs[subs].ss_tgt;
		     *replacement_ptr;
		     replacement_ptr++) {
		    /**********
		      if the replacement string contains \1-\9, then
		      keep the original part of the string there
		      *********/
		    if ((*replacement_ptr == '\\') &&
			isdigit(*(replacement_ptr+1))) {
			replacement_ptr++;
			temp[0] = *replacement_ptr;
			backref = atoi(temp);
			for (j=match_reg[backref].rm_so;
			     j<match_reg[backref].rm_eo; j++)
			    *outbuf_ptr++ = str_ptr[j];
		    }
		    /**********
		      otherwise just put in the replacement string
		      **********/
		    else
			*outbuf_ptr++ = *replacement_ptr;
		}
		/**********
		  look for another match after 
		  **********/
		str_ptr = &str_ptr[match_reg[0].rm_eo];
		status = regexec(&cmp_reg, str_ptr, 10, &match_reg[0], REG_NOTBOL);
	    }
	}
	    
	/**********
	  put everything after the matches back in the string
	  *********/
	strcpy(outbuf_ptr, str_ptr);

	/**********
	  for the next time around set str_ptr equal to the last
	  out buffer
	*********/
	str_ptr = outbuf[buffer];

    }

    /**********
      if the buffer is null, then none of the replacement strings 
      were active for this order, get some space and copy the original
      string into it
    **********/
    if(Reg_Exp)
    	(void) regfree(&cmp_reg);
    if (outbuf[buffer] == (char *)NULL){
	strcpy(outstr, str);
	return(outstr);
    }
    /**********
      return the new string
    **********/
    strcpy(outstr, outbuf[buffer]);
    return(outstr);
}

#endif
