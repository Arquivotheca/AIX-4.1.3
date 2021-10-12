static char sccsid[] = "@(#)01        1.2.1.2  src/bos/usr/bin/scls/scls.c, cmdpse, bos411, 9428A410j 11/16/93 09:09:23";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   FUNCTIONS: MSGSTR
 *		cpp_alloc_len
 *		create_cpp
 *		find_module
 *		get_cp
 *		get_info
 *		get_names
 *		get_num
 *		main
 *		name_cmp
 *		print_list
 *		print_module
 *		qsort
 *		
 *
 *   ORIGINS: 27,63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/** Copyright (c) 1990  Mentat Inc.
 ** scls.c 2.2, last change 11/19/90
 **/


#include <pse/common.h>
#include <sys/stropts.h>
#include <stdio.h>
#include <pse/nd.h>

#include "scls_msg.h"
#include <locale.h>
nl_catd catd;
#define MSGSTR(n,s)	catgets(catd,MS_SCLS,n,s)

#ifndef	NULL_DEV_NAME
#define	NULL_DEV_NAME	"/dev/nuls"
#endif
#ifndef	SC_MODULE
#define	SC_MODULE	"sc"
#endif

#define	MOD_TYPE_WHO_KNOWS	0
#define	MOD_TYPE_MODULE		1
#define	MOD_TYPE_DEVICE		2

staticf	int	cpp_alloc_len(   char ** cpp   );
staticf	void	create_cpp(   char ** cpp, char * buf   );
staticf	boolean	find_module(   char * module, char ** cpp   );
staticf	int	get_info(   int fd, char * module, int mod_type   );
staticf	char	** get_names(   int fd   );
extern	char	* malloc(   int size   );
staticf	int	name_cmp(   char * e1, char * e2   );
staticf	void	print_list(   char ** namesp   );
staticf	void	print_module(   char * buf, int fd, boolean count_listing, int mod_type   );
extern	void	qsort(   char * base, unsigned nel, unsigned width, int (*compar)()   );
extern	char	* strchr(   char * str, char c   );
extern	char	* strrchr(   char * str, char c   );


extern	int	noshare errno;
	char	* info_type_arr[] = { "info", "minfo", "dinfo" };
	char	name_buf[4096], info_buf[3000];
static	char	format_hdr[] = "%-9s%-6s%-7s%-7s%-10s%-8s%-8s%-8s%s\n\r";
static	char	format_cnt_hdr[] = "%-9s%-4s%-4s%-6s%-10s%-10s%-10s%s\n\r";
extern	char	noshare * optarg;
extern	int	noshare optind;


main (argc, argv)
	int	argc;
	char	** argv;
{
	char	* module, ** cpp, ** namesp, ** new_namesp;
	int	c, fd;
	boolean	long_listing;
	boolean	count_listing;

	count_listing = false;
	long_listing = false;
	module = SC_MODULE;

	setlocale(LC_ALL, "");
	catd = catopen(MF_SCLS, NL_CAT_LOCALE);
	set_program_name(argv[0]);

	namesp = nilp(char *);
	while ((c = getopt(argc, argv, "?clm:")) != -1) {
		switch (c) {
		case 'c':
			count_listing = true;
			fallthru;
		case 'l':
			long_listing = true;
			break;
		case 'm':
			module = optarg;
			break;
		case '?':
		default:
			usage(MSGSTR(USAGE1, "[-l] [-c] [-m sc_module_name] [modules_to_list]"));
		}
	}
	if ((fd = stream_open(NULL_DEV_NAME, 2)) == -1) {
		fprintf(stderr, MSGSTR(MSG11, "scls: couldn't open '%s', errno == %d\n"), NULL_DEV_NAME, errno);
		exit(1);
	}
	if (stream_ioctl(fd, I_PUSH, module) == -1) {
		fprintf(stderr, MSGSTR(MSG12, "scls: couldn't push module '%s', errno == %d\n"), module, errno);
		goto done;
	}
	if (!(namesp = get_names(fd))) {
		fprintf(stderr, MSGSTR(MSG13, "scls: couldn't retrieve module information\n"));
		goto done;
	}
	if (module = getarg(argc, argv)) {
		if (!(new_namesp = (char **)malloc(cpp_alloc_len(namesp)))) {
			fprintf(stderr, MSGSTR(MSG14, "scls: couldn't allocate memory\n"));
			goto done;
		}
		cpp = new_namesp;
		do {
			if (find_module(module, namesp))
				*cpp++ = module;
			else
				printf(MSGSTR(MSG15, "scls: %s not found\n"), module);
		} while (module = getarg(argc, argv));
		*cpp = nilp(char);
		free(namesp);
		namesp = new_namesp;
		qsort((char *)namesp, (unsigned)(cpp-namesp), (unsigned)sizeof(char *),name_cmp);
	}

	if (long_listing) {
		if (!*namesp)
			goto done;
		if (count_listing)
			printf(MSGSTR(MSG16, "name    type   major  put-cnt  srv-cnt   open-cnt  close-cnt admin-cnt\n"));
/******			printf(format_cnt_hdr, "name", "type", "major", "put-cnt", "srv-cnt", "open-cnt", "close-cnt", "admin-cnt"); ******/
		else
			printf(MSGSTR(MSG17, "name     type  major  idnum  idname   minpsz  maxpsz  lowat   hiwat\n")); /* FIELDS HAVE BEEN DELETED XXXXX */
/******			printf(format_hdr, "name", "type", "major", "idnum", "idname", "minpsz", "maxpsz", "lowat", "hiwat", "put cnt", "srv","open", "close", "admin"); ******/
		for (cpp = namesp; *cpp; cpp++) {
			if ( cpp[1]  &&  strcmp(cpp[0], cpp[1]) == 0 ) {
				print_module(*cpp++, fd, count_listing, MOD_TYPE_DEVICE);
				print_module(*cpp, fd, count_listing, MOD_TYPE_MODULE);
			} else
				print_module(*cpp, fd, count_listing, MOD_TYPE_WHO_KNOWS);
		}
	} else
		print_list(namesp);
done:
	stream_close(fd);
	if (namesp)
		free(namesp);

	return 0;
}

staticf int
cpp_alloc_len (cpp)
	char	** cpp;
{
	int	cnt;

	cnt = 0;
	if (cpp) {
		while (*cpp++)
			cnt++;
		cnt++;
	}
	return (cnt * sizeof(char *));
}

staticf void
create_cpp (cpp, buf)
	char	** cpp;
	char	* buf;
{
reg	char	* cp;
	int	index;

	for (index = 0, cp = buf; cp  &&  *cp; index++) {
		cpp[index] = cp;
		while (*cp++)
			noop;
	}
	cpp[index] = nilp(char);
}

staticf boolean
find_module (module, cpp)
	char	* module;
	char	** cpp;
{
	for ( ; *cpp; cpp++) {
		if (strcmp(module, *cpp) == 0)
			return true;
	}
	return false;
}

staticf int
get_info (fd, module, mod_type)
	int	fd;
	char	* module;
	int	mod_type;
{
	struct strioctl	stri;

	stri.ic_cmd = ND_GET;
	stri.ic_timout = 0;
	sprintf(info_buf, "%s%c%s", info_type_arr[mod_type], '\0', module);
	stri.ic_dp = info_buf;
	stri.ic_len = sizeof(info_buf);
	return stream_ioctl(fd, I_STR, (char *)&stri);
}

staticf char **
get_names (fd)
	int	fd;
{
reg	char	* cp;
	char	** cpp;
	struct strioctl	stri;
	int	cnt;

	stri.ic_cmd = ND_GET;
	stri.ic_timout = 0;
	sprintf(name_buf, "names");
	stri.ic_dp = name_buf;
	stri.ic_len = sizeof(name_buf);
	if (stream_ioctl(fd, I_STR, (char *)&stri) == -1)
		return nil(char **);
	for (cnt = 0, cp = name_buf; cp  &&  *cp; cnt++) {
		while (*cp++)
			noop;
	}
	if (cnt == 0  ||  !(cpp = (char **)malloc((cnt+1)*sizeof(char *))))
		return nil(char **);
	create_cpp(cpp, name_buf);
	qsort((char *)cpp, (unsigned)cnt, (unsigned)sizeof(char *), name_cmp);
	return cpp;
}

staticf int
name_cmp (e1, e2)
reg	char	* e1;
reg	char	* e2;
{
	return strcmp(*(char **)e1, *(char **)e2);
}

staticf void
print_list (namesp)
	char	** namesp;
{
	int	max_per_line;
	char	** cpp;
	char	fmt[10];
reg	int	i1, i2;
	int	last_partial, cnt, num_lines;

	for (cnt = 0, i1 = 0, cpp = namesp; *cpp; cpp++, cnt++)
		i1 = MAX(i1, strlen(*cpp)+1);
	i1 += 1;
	sprintf(fmt, "%%-%ds", i1);
	max_per_line = 80 / i1;
	num_lines = cnt / max_per_line;
	if (last_partial = cnt % max_per_line) {
		for (;;) {
			if (last_partial + num_lines > (max_per_line-1))
				break;
			max_per_line--;
			last_partial += num_lines;
		}
		num_lines++;
	} else
		last_partial = max_per_line;
	for (i1 = 0, i2 = 0, cpp = namesp; cnt--; ) {
		if (++i1 == max_per_line) {
			printf(cpp[i2]);
			i1 = i2 = 0;
			cpp++;
			printf("\n");
		} else {
			printf(fmt, cpp[i2]);
			i2 += num_lines;
			if (i1 > last_partial)
				i2--;
		}
	}
	if (i1 != 0)
		printf("\n");
}

staticf char *
get_cp (buf, name)
	char	* buf;
	char	* name;
{
reg	char	* cp;
	int	name_len;

	name_len = strlen(name);
	for (cp = buf; cp[0]  &&  cp[1]; cp += (strlen(cp)+1)) {
		if (cp[0] == name[0]  &&  strncmp(cp, name, name_len) == 0) {
			if (cp = strchr(cp, '='))
				cp += 3;
			return cp;
		}
	}
	return nilp(char);
}

staticf int
get_num (buf, name)
	char	* buf;
	char	* name;
{
reg	char	* cp;

	if (cp = get_cp(buf, name))
		return atoi(cp);
	return -1;
}

staticf void
print_module (buf, fd, count_listing, mod_type)
	char	* buf;
	int	fd;
	boolean	count_listing;
	int	mod_type;
{
	boolean	is_device;
	int	i1;
reg	char	* cp;

	printf("%-9s", buf);
	if (get_info(fd, buf, mod_type) == -1) {
		printf(MSGSTR(MSG18, "scls: no information available\n"));
		return;
	}
	info_buf[3] = '\0';
	printf(" %-5s", info_buf);	/* module type */
	is_device = strcmp(info_buf, "mod") != 0;
	if (is_device) {
		i1 = get_num(info_buf, "device number");
		printf("%5d ", i1);
	} else
		printf("%6c", ' ');
	if (!count_listing) {
		i1 = get_num(info_buf, "idnum");
		printf("%6d  ", i1);
		cp = get_cp(info_buf, "idname");
		printf("%-8s", cp);
		i1 = get_num(info_buf, "minpsz");
		printf("%8d", i1);
		i1 = get_num(info_buf, "maxpsz");
		printf("%8d", i1);
		i1 = get_num(info_buf, "lowat");
		printf("%7d", i1);
		i1 = get_num(info_buf, "hiwat");
		printf("%8d", i1);
	} else {
		i1 = get_num(info_buf, "put count");
		if (i1 == -1) {
			printf("\n");
			return;
		}
		printf("%10d", i1);
		i1 = get_num(info_buf, "service count");
		printf("%10d", i1);
		i1 = get_num(info_buf, "open count");
		printf("%11d", i1);
		i1 = get_num(info_buf, "close count");
		printf("%11d", i1);
		i1 = get_num(info_buf, "admin count");
		printf("%10d", i1);
	}
	printf("\n");
}
