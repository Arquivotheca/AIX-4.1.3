static char sccsid[] = "@(#)74	1.8  src/bos/usr/bin/que/rem/debugr.c, cmdque, bos411, 9428A410j 4/11/94 09:13:05";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*	See commonr.h for BSD lpr/lpd protocol description.*/

	
#include "commonr.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <IN/standard.h>
#include <IN/backend.h>
#include <stdio.h>
#include "outr.h"


#ifdef DEBUG
char *inet_ntoa ();

char * showhost(word)
unsigned int word;
{
        static char str[100] ;

	return(inet_ntoa(word));

/*
	if(getenv("XDEBUG"))
	{
		sprintf(str,"%d.%d.%d.%d\n", 
			(word >> 24), 
			((word << 8) >> 24),
			((word << 16) >> 24), 
			((word  << 24) >> 24)     
		);
	}
	return(str);
*/
}


showjob(j)
struct job *j;
{
	showusers(j->j_u);
	showjobnums(j->j_jn);
	showaixcmds(j->j_ac);
	return(0);
}

showusers(u)
struct users *u;
{
	if(getenv("XDEBUG"))
	{
		fprintf(stderr,"users are... ");
		for (;u;u=u->u_next)
			fprintf(stderr,"%s ",u->u_user);
		fprintf(stderr,"\n");
	}
	return(0);
}


showjobnums(j)
struct jobnum *j;
{
	if(getenv("XDEBUG"))
	{
		fprintf(stderr,"jobnums are... ");
		for (;j;j=j->jn_next)
			fprintf(stderr,"%d " ,j->jn_num);
		fprintf(stderr,"\n");
	}
	return(0);
}


showaixcmds(ac)
struct aixcmds *ac;
{
	if(getenv("XDEBUG"))
	{
		fprintf(stderr,"aixcmds are... ");
		for (;ac;ac=ac->ac_next)
			fprintf(stderr,"%s ",ac->ac_opt);
		fprintf(stderr,"\n");
	}
	return(0);
}

showsin(sin)
struct sockaddr_in sin;
{
	if(getenv("XDEBUG"))
	{
		fprintf(stderr,"sin_family %d ",sin.sin_family);
		fprintf(stderr,"sin_port %d ",sin.sin_port);
		fprintf(stderr,"sin_addr %lx ",sin.sin_addr.s_addr);
		fprintf(stderr,"sin_addr %s \n",showhost(sin.sin_addr.s_addr));
	}
	return(0);
}

showhostent(hp)
struct hostent *hp;
{
	if(getenv("XDEBUG"))
	{
		fprintf(stderr,"h_name %s ",hp->h_name);
		fprintf(stderr,"1st h_aliases %s \n",*(hp->h_aliases));
		fprintf(stderr,"h_addr %s ",showhost(hp->h_addr));
	}
	return(0);
}
	
showopts(c)
struct comline *c;
{
	if(getenv("XDEBUG"))
	{
fprintf(stderr,"c->c_pflg %d	",	c->c_pflg);
fprintf(stderr,"c->c_Oflg %d	",	c->c_Oflg);
fprintf(stderr,"c->c_fflg %d	",	c->c_fflg);
fprintf(stderr,"c->c_cflg %d	",	c->c_cflg);
fprintf(stderr,"c->c_lflg %d	",	c->c_lflg);
fprintf(stderr,"c->c_tflg %d	",	c->c_tflg);
fprintf(stderr,"c->c_nflg %d	",	c->c_nflg);
fprintf(stderr,"c->c_dflg %d	",	c->c_dflg);
fprintf(stderr,"c->c_gflg %d	",	c->c_gflg);
fprintf(stderr,"c->c_vflg %d	",	c->c_vflg);
fprintf(stderr,"c->c_mflg %d	",	c->c_mflg);
fprintf(stderr,"c->c_iflg %d	",	c->c_iflg);
fprintf(stderr,"c->c_wflg %d	",	c->c_wflg);
fprintf(stderr,"c->c_Tflg %d	",	c->c_Tflg);
fprintf(stderr,"c->c_1flg %d	",	c->c_1flg);
fprintf(stderr,"c->c_2flg %d	",	c->c_2flg);
fprintf(stderr,"c->c_3flg %d	",	c->c_3flg);
fprintf(stderr,"c->c_4flg %d	",	c->c_4flg);
fprintf(stderr,"c->c_qflg %d	",	c->c_qflg);
fprintf(stderr,"c->c_xflg %d	",	c->c_xflg);
fprintf(stderr,"c->c_Lflg %d	",	c->c_Lflg);
fprintf(stderr,"c->c_Rflg %d	",	c->c_Rflg);
fprintf(stderr,"c->c_Pflg %d	",	c->c_Pflg);
fprintf(stderr,"c->c_Sflg %d	",	c->c_Sflg);
fprintf(stderr,"c->c_oflg %d	",	c->c_oflg);
fprintf(stderr,"c->c_uflg %d	",	c->c_uflg);
fprintf(stderr,"c->c_numflg %d	",	c->c_numflg);
fprintf(stderr,"c->c_Prem %s	",	c->c_Prem);
fprintf(stderr,"c->c_Srem %s	",	c->c_Srem);
fprintf(stderr,"c->c_orem %s	",	c->c_orem);
fprintf(stderr,"c->c_urem %s	",	c->c_urem);
fprintf(stderr,"c->c_numrem %s	",	c->c_numrem);
fprintf(stderr,"c->c_irem %s	",	c->c_irem);
fprintf(stderr,"c->c_wrem %s	",	c->c_wrem);
fprintf(stderr,"c->c_Trem %s	",	c->c_Trem);
fprintf(stderr,"c->c_1rem %s	",	c->c_1rem);
fprintf(stderr,"c->c_2rem %s	",	c->c_2rem);
fprintf(stderr,"c->c_3rem %s	",	c->c_3rem);
fprintf(stderr,"c->c_4rem %s\n",	c->c_4rem);
	}
	return(0);
}

#endif
