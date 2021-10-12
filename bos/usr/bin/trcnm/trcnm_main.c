static char sccsid[] = "@(#)56	1.4  src/bos/usr/bin/trcnm/trcnm_main.c, cmdtrace, bos411, 9428A410j 4/5/94 17:54:25";

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* 
 * This file is not part of any RAS supported commands.  It will
 * be removed or moved to test case as some time.   Therefore
 * no this code will not be ILSized and no messages will be added
 * to the message catalog.
 */

#include <stdio.h>
#include <nlist.h>
#include <locale.h>
#include <cmdtrace_msg.h>

#define MCS_CATALOG "cmdtrace.cat"

#define UNXFILE "/unix"

extern optind;
extern char *optarg;

extern verboseflg;

static statflg;
static quietflg;
static allflg;
static Knlistflg;

main(argc,argv)
char *argv[];
{
	int c;
	char *filename;

	(void) setlocale(LC_ALL,"");
	catinit(MCS_CATALOG);
	filename = UNXFILE;
	while((c = getopt(argc,argv,"aK")) != EOF) {
		switch(c) {
		case 'K':
			Knlistflg++;
			break;
		case 'a':
			allflg++;
			break;
		default:
			usage();
		}
	}
	if(Knlistflg) {
		int i,nsyms;
		struct nlist *nl;

		nsyms =  argc - optind;
		if(nsyms == 0)
			exit(0);
		if((nl = (struct nlist *)calloc(nsyms,sizeof(struct nlist))) == 0) {
			perror("calloc");
			exit(1);
		}
		for(i = optind; i < argc; i++)
			nl[i-optind].n_name = argv[i];
		knlist(nl,nsyms,sizeof(struct nlist));
		for(i = 0; i < nsyms; i++)
			printf("%-20s %08X\n",nl[i].n_name,nl[i].n_value);
		exit(0);
	}
	if(optind < argc)
		filename = argv[optind];
	rptsym_init(filename,allflg);
	if(statflg)
		rptsym_stat();
	if(!quietflg)
		rptsym_dump();
	exit(0);
}

usage()
{

	cat_eprint(CAT_TRCNM_USAGE,"\
usage:\n\
trcnm  -a Filename\n\
trcnm  -K Symbol1 ...\n\
\n\
Write a kernel name list to standard output.\n\
\n\
-a          Print all loader symbols. Default is system calls only.\n\
-K Symbol1  Print the value of the specified symbols via knlist\n\
            system call.\n\
\n\
If the Filename is not specified, the default Filename is %s.\n",UNXFILE);
	exit(1);
}
