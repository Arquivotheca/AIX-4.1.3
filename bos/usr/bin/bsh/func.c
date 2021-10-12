static char sccsid[] = "@(#)99	1.19  src/bos/usr/bin/bsh/func.c, cmdbsh, bos411, 9428A410j 4/22/94 18:20:48";
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS: freefunc freetree freearg freeio freereg prf prarg prio resource
 *	      intochar
 *
 * ORIGINS: 3, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * 1.9  com/cmd/sh/sh/func.c, cmdsh, bos324
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1
 */

/* define to use faster MACROS instead of functions for performance */
#define _ILS_MACROS

#include	"defs.h"
#include	<sys/resource.h>
#include	<sys/param.h>
#include	<ctype.h>

freefunc(n)
	struct namnod 	*n;
{
	freetree((struct trenod *)(n->namenv));
}


freetree(t)
	register struct trenod *t;
{
	if (t)
	{
		register int type;

		if (t->tretyp & CNTMSK)
		{
			t->tretyp--;
			return;
		}

		type = t->tretyp & COMMSK;

		switch (type)
		{
			case TFND:
				free(fndptr(t)->fndnam);
				freetree(fndptr(t)->fndval);
				break;

			case TCOM:
				freeio(comptr(t)->comio);
				free_arg(comptr(t)->comarg);
				free_arg(comptr(t)->comset);
				break;

			case TFORK:
				freeio(forkptr(t)->forkio);
				freetree(forkptr(t)->forktre);
				break;

			case TPAR:
				freetree(parptr(t)->partre);
				break;

			case TFIL:
			case TLST:
			case TAND:
			case TORF:
				freetree(lstptr(t)->lstlef);
				freetree(lstptr(t)->lstrit);
				break;

			case TFOR:
			{
				struct fornod *f = (struct fornod *)t;

				free(f->fornam);
				freetree(f->fortre);
				if (f->forlst)
				{
					freeio(f->forlst->comio);
					free_arg(f->forlst->comarg);
					free_arg(f->forlst->comset);
					free(f->forlst);
				}
			}
			break;

			case TWH:
			case TUN:
				freetree(whptr(t)->whtre);
				freetree(whptr(t)->dotre);
				break;

			case TIF:
				freetree(ifptr(t)->iftre);
				freetree(ifptr(t)->thtre);
				freetree(ifptr(t)->eltre);
				break;

			case TSW:
				free(swptr(t)->swarg);
				freereg(swptr(t)->swlst);
				break;
		}
		free(t);
	}
}

free_arg(argp)
	register struct argnod 	*argp;
{
	register struct argnod 	*sav;

	while (argp)
	{
		sav = argp->argnxt;
		free(argp);
		argp = sav;
	}
}


freeio(iop)
	register struct ionod *iop;
{
	register struct ionod *sav;

	while (iop)
	{
		if (iop->iofile & IODOC)
		{
                        uchar_t  ok_to_unlink = 1 ;

			if (fiotemp == iop)
				fiotemp = iop->iolst;
			else
			{
				struct ionod *fiop = fiotemp;

				while (fiop->iolst != iop && fiop)
					fiop = fiop->iolst;
	
                                if ( fiop )
                                        fiop->iolst = iop->iolst;
                                else
                                        ok_to_unlink = 0 ;
                        }
 
                        if ( ok_to_unlink )
                        {
#ifdef DEBUG
                                prs("unlinking ");
                                prs(iop->ioname);
                                newline();
#endif
 
                                unlink(NLSdecode(iop->ioname));

			}
		}
		free(iop->ioname);
		free(iop->iolink);
		sav = iop->ionxt;
		free(iop);
		iop = sav;
	}
}


freereg(regp)
	register struct regnod 	*regp;
{
	register struct regnod 	*sav;

	while (regp)
	{
		free_arg(regp->regptr);
		freetree(regp->regcom);
		sav = regp->regnxt;
		free(regp);
		regp = sav;
	}
}


prf(t)
	register struct trenod	*t;
{
	sigchk();

	if (t)
	{
		register int	type;

		type = t->tretyp & COMMSK;

		switch(type)
		{
			case TFND:
			{
				register struct fndnod *f = (struct fndnod *)t;

				prs_buff(f->fndnam);
				prs_buff("(){\n");
				prf(f->fndval);
				prs_buff("\n}");
				break;
			}

			case TCOM:
			{
				prarg(comptr(t)->comset);
				prarg(comptr(t)->comarg);
				prio(comptr(t)->comio);
				break;
			}

			case TFORK:
				prf(forkptr(t)->forktre);
				prio(forkptr(t)->forkio);
				if (forkptr(t)->forktyp & FAMP)
					prs_buff(" &");
				break;

			case TPAR:
				prs_buff("( ");
				prf(parptr(t)->partre);
				prs_buff(" )");
				break;

			case TFIL:
				prf(lstptr(t)->lstlef);
				prs_buff(" | ");
				prf(lstptr(t)->lstrit);
				break;

			case TLST:
				prf(lstptr(t)->lstlef);
				prc_buff(NL);
				prf(lstptr(t)->lstrit);
				break;

			case TAND:
				prf(lstptr(t)->lstlef);
				prs_buff(" && ");
				prf(lstptr(t)->lstrit);
				break;

			case TORF:
				prf(lstptr(t)->lstlef);
				prs_buff(" || ");
				prf(lstptr(t)->lstrit);
				break;

			case TFOR:
				{
					register struct argnod	*arg;
					register struct fornod 	*f = (struct fornod *)t;

					prs_buff("for ");
					prs_buff(f->fornam);

					if (f->forlst)
					{
						arg = f->forlst->comarg;
						prs_buff(" in");

						while(arg != ENDARGS)
						{
							prc_buff(SP);
							prs_buff(arg->argval);
							arg = arg->argnxt;
						}
					}

					prs_buff("\ndo\n");
					prf(f->fortre);
					prs_buff("\ndone");
				}
				break;

			case TWH:
			case TUN:
				if (type == TWH)
					prs_buff("while ");
				else
					prs_buff("until ");
				prf(whptr(t)->whtre);
				prs_buff("\ndo\n");
				prf(whptr(t)->dotre);
				prs_buff("\ndone");
				break;

			case TIF:
			{
				struct ifnod *f = (struct ifnod *)t;

				prs_buff("if ");
				prf(f->iftre);
				prs_buff("\nthen\n");
				prf(f->thtre);

				if (f->eltre)
				{
					prs_buff("\nelse\n");
					prf(f->eltre);
				}

				prs_buff("\nfi");
				break;
			}

			case TSW:
				{
					register struct regnod 	*swl;

					prs_buff("case ");
					prs_buff(swptr(t)->swarg);
					prs_buff(" in\n");

					swl = swptr(t)->swlst;
					while(swl)
					{
						struct argnod	*arg = swl->regptr;

						if (arg)
						{
							prs_buff(arg->argval);
							arg = arg->argnxt;
						}

						while(arg)
						{
							prs_buff(" | ");
							prs_buff(arg->argval);
							arg = arg->argnxt;
						}

						prs_buff(")");
						prf(swl->regcom);
						prs_buff(";;\n");
						swl = swl->regnxt;
					}
					prs_buff("esac\n");
				}
				break;
			} 
		} 

	sigchk();
}

prarg(argp)
	register struct argnod	*argp;
{
	while (argp)
	{
		prs_buff(argp->argval);
		prc_buff(SP);
		argp=argp->argnxt;
	}
}


prio(iop)
	register struct ionod	*iop;
{
	register int	iof;
	register uchar_t	*ion;

	while (iop)
	{
		iof = iop->iofile;
		ion = iop->ioname;

		if (*ion)
		{
			prn_buff(iof & IOUFD);

			if (iof & IODOC)
				prs_buff("<<");
			else if (iof & IOMOV)
			{
				if (iof & IOPUT)
					prs_buff(">&");
				else
					prs_buff("<&");

			}
			else if ((iof & IOPUT) == 0)
				prc_buff('<');
			else if (iof & IOAPP)
				prs_buff(">>");
			else
				prc_buff('>');

			prs_buff(ion);
			prc_buff(SP);
		}
		iop = iop->ionxt;
	}
}


#define	HARD	FALSE	
#define SOFT	TRUE
#define	BOTH	-2
#define	NOSET	-1
#ifdef OSF
#define	LIMITS	8	
#else
#define LIMITS	6
#endif

void
resource(int count, uchar_t **option)
{

int	command;	/* resource needing attention */
int	limit;		/* resource bound to be set/displayed */
int	number;		/* conversion holder for current limit */	
int	print;		/* display either a soft or hard setting */
int	n;				/* loop control */ 
long	new_limit;	/* requested limit value */
uchar_t	*value;		/* command line value */ 
char	*buffer;	/* output display buffer */
char	num[10];	/* conversion buffer for 'number' */
struct rlimit rlim;	/* holds current resource settings */

struct limit{
	int	resource;
	char *name;
	int	operend;
	char *unit;
	} limits[] = {
			RLIMIT_CPU, "cputime \t", 1, " seconds\n",
#ifdef OSF
			RLIMIT_FSIZE, "filesize \t", 1024, " 1K-blocks\n",
#else
			RLIMIT_FSIZE, "filesize \t", 512, " 512-blocks\n",
#endif
			RLIMIT_DATA, "datasize \t", 1024, " 1K-blocks\n",
			RLIMIT_STACK, "stacksize \t", 1024, " 1K-blocks\n",
#ifdef OSF
			RLIMIT_CORE, "coredumpsize \t", 1024, " 1K-blocks\n",
#else
			RLIMIT_CORE, "coredumpsize \t", 512, " 512-blocks\n",
#endif
			RLIMIT_RSS, "memory \t\t", 1024, " 1K-blocks\n",
#ifdef OSF
			RLIMIT_NOFILE, "descriptors \t", 1, " files\n",
			RLIMIT_AS, "addressspace \t", 1024, " 1K-blocks\n",
#endif
	};

void intochar();

	/*
	 * Initialize local variables.
	 */
	buffer = (char *)malloc(UBSIZE);
	limit = BOTH;
	command = NOSET;
	value = NULL;
	print = SOFT;

	/*
	 * Decode the command line argument. 
	 */
	for(n=0; n < count; n++)
		NLSdecode(option[n]);

	/*
	 * Test to see whether an option was put on the command
	 * line with the ulimit builtin. If so assign command
	 * the appropriate value and assign the new size to the
	 * value string otherwise assign the default of file
	 * size to command.
	 */
	if ( count > 1 && option[1][0] != '-' ) {
		value = option[1];
		command = RLIMIT_FSIZE;
	}
	else {
		n = 1;
		while(option[n] && option[n][0] == '-') {

			/* check that all flags are followed by nothing,
			 * or a valid combination of flags (H or S with any
			 * other flag) */

			if ( option[n][3] != '\0' )
				error(MSGSTR(M_ULIMITUSAGE, (char *)ulimitusage));	
			else if ( option[n][2] != '\0' && option[n][1] == 'H'
				&&  option[n][2] != 'H' && option[n][2] != 'S' )
			{
				limit = HARD;
				print = HARD;
				option[n][1] = option[n][2];
			}
			else if ( option[n][2] != '\0' && option[n][1] == 'S' 
				&& option[n][2] != 'H' && option[n][2] != 'S' )
			{
				limit = SOFT;
				print = SOFT;
				option[n][1] = option[n][2];
			}
			else if ( option[n][2] == 'H' && option[n][1] != 'H'
				&& option[n][1] != 'S' )
			{
				limit = HARD;
				print = HARD;
			}
			else if ( option[n][2] == 'S' && option[n][1] != 'H'
				&& option[n][1] != 'S' )
			{
				limit = SOFT;
				print = SOFT;
			}
			else if ( option[n][2] != '\0' )
				error(MSGSTR(M_ULIMITUSAGE, (char *)ulimitusage));	

			switch(option[n][1]) {
#ifdef OSF
				case 'a':       /* Address Space */
					command = RLIMIT_AS;
					break;
#endif

				case 'c':       /* Core Segment */
					command = RLIMIT_CORE;
					break;

				case 'd':       /* Data Segment */
					command = RLIMIT_DATA;
					break;

				case 'f':	/* File Segment */
					command = RLIMIT_FSIZE;	
					break;	

				case 'H':       /* Set or print hard limits */ 
					limit = HARD;
					print = HARD;
					break;
#ifdef OSF
				case 'h':       /* Print hard limits */ 
					print = HARD;
					break;
#endif
				case 'm':       /* Memory */
					command = RLIMIT_RSS;
					break;
#ifdef OSF
				case 'n':       /* File Descriptor */
					command = RLIMIT_NOFILE;
					break;
#endif
				case 'S':       /* Set or print soft limits */ 
					limit = SOFT;
					print = SOFT;
					break;

				case 's':       /* Stack Segment */ 
					command = RLIMIT_STACK;	
					break;

				case 't':       /* CPU Time */
					command = RLIMIT_CPU;
					break;

				case '?':
				default:	
					error(MSGSTR(M_ULIMITUSAGE, (char *)ulimitusage));	
			}	

			if ( n+1 == count )
				break;
			else if ( n < count && option[n+1][0] != '-' ) {
				if ( command == NOSET )
					command = RLIMIT_FSIZE;
				value = option[n+1];
				break;
				}
				else
					n++;
		} /* while */
	}

	/*
	 * Now we either are ready to display 
	 * limit/limits or set a limit. 
	 */
	if ( value == NULL ) {
		if ( command == NOSET ) {
			for(n=0;n<LIMITS;n++) {
				if(getrlimit(limits[n].resource,&rlim) < 0) {
					perror("bsh: ulimit");
					exitsh(ERROR);
				}
				else {
					strcat(buffer,limits[n].name);
					if( rlim.rlim_cur == RLIM_INFINITY && rlim.rlim_max == RLIM_INFINITY )
						strcat(buffer,"unlimited\n");	
					else {
						if ( print == SOFT )
							number=rlim.rlim_cur/limits[n].operend;
						else
							number=rlim.rlim_max/limits[n].operend;
						intochar(number, num);
						strcat(buffer,num);
						strcat(buffer,limits[n].unit);		
					}
				}
			}
		}
		else {
			if(getrlimit(limits[command].resource,&rlim) < 0) {
				perror("bsh: ulimit");
				exitsh(ERROR);
			}
			else {
				strcat(buffer,limits[command].name);
					if( rlim.rlim_cur == RLIM_INFINITY && rlim.rlim_max == RLIM_INFINITY )
					strcat(buffer,"unlimited\n");
				else {
					if ( print == SOFT )
						number=rlim.rlim_cur/limits[command].operend;
					else
						number=rlim.rlim_max/limits[command].operend;
					intochar(number, num);
					strcat(buffer,num);
					strcat(buffer,limits[command].unit);
				}
			}
		}
		prs_buff(buffer);
	} /* value */
	else {
		/*
		 * Assign new_limit a value, but test to make
		 * sure we have a valid number from the command line.
		 */
		if ( strcmp((char *)value,"unlimited") ) {
			n = 0;
			while(*value)
				if (isdigit(*value) ) {
					++n;
					++value;
				}
				else {
					error(MSGSTR(M_ULIMITBAD, (char *)ulimitbad));
				}
				value -= n;
				new_limit = atol(value);
		}
		else {
			new_limit = RLIM_INFINITY;
			if (new_limit < 0)	
				error(MSGSTR(M_ULIMITBAD, (char *)ulimitbad));	
		}
		/*
		 * Now lets get a starting point to compare the
		 * requested value against the current value.
		 */
		if ( getrlimit(command,&rlim) < 0 ) {
			perror("bsh: ulimit");
			exitsh(ERROR);
		}

		/*
		 * Now we need to convert the new_limit to 
		 * the same unit of measure that is contained
		 * in the rlim structure.
		 */
		if(new_limit != RLIM_INFINITY)
			new_limit = new_limit * limits[command].operend; 

		/*
		 * We setup the new rlim structure with the requested
		 * values.
		 * -Need to find out if we are currently the superuser.
		 *  If we are then we simply assign the new 
		 *  limit to the appropriate field of the rlim 
		 *  structure based on whether we are setting 
		 *  HARD, SOFT, or BOTH(default) limits. 
		 *
		 * -If we are not superuser we test to insure that the 
		 *  new_value doesn't exceed the current field we are 
		 *  trying to set. The field is dependent on the limit
		 *  value: HARD, SOFT, or BOTH(default). 
		 */ 
		if ( geteuid() == 0 ) {	/*superuser*/
			if ( limit == HARD )
				if ( new_limit >= rlim.rlim_cur )
					rlim.rlim_max = new_limit;
				else
					error(MSGSTR(M_ULIMITHARD, (char *) ulimithard));
			else  if ( limit == SOFT )
				if ( new_limit <= rlim.rlim_max )
					rlim.rlim_cur = new_limit;
				else
					error(MSGSTR(M_ULIMITEXCEED, (char *) ulimitexceed));
			else {
				rlim.rlim_cur = new_limit;
				rlim.rlim_max = new_limit;
			}
		}
		else {	/*not superuser*/
			if ( limit == HARD )
				error(MSGSTR(M_ULIMITNOTSU, (char *)ulimitnotsu));
			else {
				if ( new_limit > rlim.rlim_cur )
					error(MSGSTR(M_ULIMITSOFT, (char *)ulimitsoft));
				else
					rlim.rlim_cur = new_limit;
			}
		}
		/*
		 * Now, finally, we set the new resource limit, but
		 * exit if an error occurs.
		 */
		if ( setrlimit(command,&rlim) < 0 ) {
			perror("bsh: ulimit");
			exitsh(ERROR);
		}
	}
}


/*
 * Convert the integer to character.
 * Put it in the buffer.
 */
void
intochar(int result, char *p1)
{
int k, j, n;

	if ( result != 0 ) {
		for(k=0; result!=0; k++) {
			p1[k] = (result % 10) + '0';
			result /= 10;
		}
		p1[k] = '\0';
		for(j=k-1, k=0; k<j ;k++,j--) {
			n = p1[k];
			p1[k] = p1[j];
			p1[j] = n;
		}
	}
	else {
		*p1++ = '0';
		p1 = '\0';
	}
	return;
}
