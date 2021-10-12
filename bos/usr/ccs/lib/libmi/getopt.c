static char sccsid[] = "@(#)84	1.1  src/bos/usr/ccs/lib/libmi/getopt.c, cmdpse, bos411, 9428A410j 5/7/91 13:08:19";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** getopt.c 2.1, last change 11/14/90
 **/


#include <stdio.h>
#include <pse/common.h>

	char	noshare * optarg;
	int	noshare optind = 1;
	int	noshare opterr = 1;
extern	char	noshare * program_name;
	
	int	getopt(   int argc, char ** argv, char * optstr   );

int
getopt (argc, argv, optstr)
	int	argc;
	char	** argv;
	char	* optstr;
{
	char	* arg;
	char	ch;
	char	* errmsg = nilp(char);
	char	* cp1;
	int	i1;
	char	* opt;
static	int	optchind = 1;

top:
	if (optind >= argc
	|| !(opt = argv[optind])
	||  opt[0] != '-'
	||  optchind <= 0)
		return -1;
	if (!opt[optchind]) {
		optind++;
		for (i1 = optind; i1 < argc  &&  argv[i1]; i1++) {
			if (argv[i1] == optarg) {
				optind = i1 + 1;
				break;
			}
		}
		optchind = 1;
		goto top;
	}
	if (opt[1] == '-') {
		optchind = -1;
		optind++;
		return -1;
	}
	arg = argv[optind + 1];
	for (cp1 = optstr; ch = *cp1; cp1++) {
		if (ch == opt[optchind]) {
			if (cp1[1] == ':') {
				for (i1 = ++optind; i1 < argc  &&  argv[i1]; i1++) {
					if (argv[i1] == optarg) {
						arg = argv[i1 + 1];
						break;
					}
				}
				if (!(optarg = arg)  ||  arg[0] == '-')
					errmsg = "%s: option '%c' not followed by argument\n";
				else {
					optind++;
					optchind = 0;
				}
			}
			break;
		}
	}
	if (!ch)
		errmsg = "%s: unknown option '%c'\n";
	if (errmsg  &&  opterr) {
		fprintf(stderr, errmsg, program_name, opt[optchind]);
		ch = '?';
	}
	optchind++;
	if (!opt[optchind]) {
		optind++;
		optchind = 1;
	}
	return ch;
}
