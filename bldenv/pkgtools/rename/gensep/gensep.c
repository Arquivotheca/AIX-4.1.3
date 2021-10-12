static char sccsid[] = "@(#)46  1.8  src/bldenv/pkgtools/rename/gensep/gensep.c, pkgtools, bos412, GOLDA411a 3/10/92 16:21:18";
/*
* COMPONENT_NAME: gensep.c
*
* FUNCTIONS:
*	main()		Program control function
*	procOptions()	Read 'ptfoptions' & call linkOptions for each opt
*	linkOptions()	Put opt PTF/opt pair in link-list if not already there
*	procPkg()	Read 'ptf_pkg'; write 'wk_ptf_pkg'; call linkPreq
*	linkPreq()	Put pkg PTF/opt pair in link-list if not already there
*	writePreq()	Write *.pre.pre for each match between the 2 link-lists
*	(ORBIT link-list functions)
*
* ORIGINS: 27
*
* IBM CONFIDENTIAL -- (IBM Confidential Restricted when
* combined with the aggregated modules for this product)
*                  SOURCE MATERIALS
* (C) COPYRIGHT International Business machines Corp. 1989
* All Rights Reserved
*
* US Government Users Restricted Rights - Use, duplication or
* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*
*	This program is called by 'genptf' as part of the update process.
*/

#include <stdio.h>
#include <string.h>

#define MAXBUFF	4096			/* Max size of input buffer.	*/
#define MAXLEN	1024			/* Max size of line fields, etc.*/
#define MAXDIR	300			/* Max size of file paths, etc.	*/
#define MAXKWD	80			/* Max size of words, etc.	*/

typedef struct {			/* Struct for ptf_pkg link-list.*/
	char	option[MAXKWD];			/* Option name		*/
	char	depPTF[16];			/* PTF optn dependent on*/
	char	function;			/* i, c, p		*/
} pkgEntry;

typedef struct {			/* Struct for ptfoptions lk-lst.*/
	char	ptf[16];			/* PTF of dependency.	*/
	char	option[MAXKWD];			/* Option of dependency.*/
} ptfOptEntry;

struct Link {				/* Standard link struct decl.	*/
	struct Link	*next;
	char		*data;
};
typedef struct Link	Link;

struct List {				/* Standard list struct decl.	*/
	Link	*last;
	Link	*current;
	int	count;
};
typedef struct List	List;

/********************* Declare some function types. **************/

List	*listNew();
Link	*linkNew();
void	listAppend();
char	*listGet();
char	*malloc();
void	listInsert();
void	listRewind();
void	listInit();
char	*substr();
void	linkOptions();
void	linkPreq();

/******************** Declare some global variables. ************/

ptfOptEntry	*PtfOptEntry;			/* Pntr to ptfOpt entry.*/
pkgEntry	*PkgEntry;			/* Pntr to ptfPkg entry.*/
List		*pkgList;			/* Pntr to ptf-pkg list.*/
List		*optList;			/* Pntr to ptfOptn list.*/
char		*whoami;			/* This pgm name.	*/
int		whoLen;				/* Length of 'whoami'.	*/
char		preqOut[MAXDIR];		/* Name of output preq. */
FILE		*fpPkg;				/* Pntr to input pkg fl.*/
FILE		*fpPkgOut;			/* Pntr to output pkg fl*/
FILE		*fpPreq;			/* Pntr to output preqs.*/

/*  */
	/************************************************
	*						*
	*	Main() function is just used to 	*
	*	control process flow.			*
	*						*
	************************************************/

int
main(ac, av)

int	ac;
char	**av;
{
	int	rc;				/* Return code from fctn*/

	/* Set program name and name length, for use in error messages.	*/
	whoami = ((whoami = strrchr(av[0], '/')) == NULL ? av[0] : whoami+1);
	whoLen = strlen(whoami);
	pkgList = listNew();		/* Start new ptf_pkg link-list.	*/
	optList = listNew();		/* Start new ptfOptn link-list.	*/
	listInit(pkgList);		/* Init list pntrs to zero.	*/
	listInit(optList);


	rc = procPkg();			/* Process the ptf_pkg file.	*/

	rc = procOptions();		/* Load ptf options into linklist*/

	if (!rc)
		rc = writePreq();	/* Write the *.pre.pre files.	*/

	exit(rc);

}	/* End of main() ***  */

	/********************************************************
	*							*
	*	procOptions() function reads 'ptfoptions' file	*
	*	and calls linkOptions() for each PTF/option	*
	*	pair, so that unique link-list entries created.	*
	*							*
	********************************************************/

int
procOptions()
{
	char	buf[MAXBUFF];		/* Input data buffer.		*/
	char	ptf[16];		/* Input PTF number string.	*/
	char	option[MAXLEN];		/* Input option(s).		*/
	char	optn[MAXLEN];		/* Parsed option.		*/
	char	optFile[MAXDIR];	/* Name of PTF options file.	*/
	FILE	*fpOpt;			/* Pntr to input options file.	*/
	int	rc;			/* Process return code.		*/
	char	*TOP;			/* Pntr to $TOP env var.	*/
	char	*token;			/* Pntr to text bounded by comma*/

	rc = 0;
	/**** $TOP environment variable must be set for this program ****/
	if ((TOP = getenv("TOP")) == NULL ) {
	    printf("%s: Cannot read env variable TOP.\n", whoami);
	    return(10);
	}
	strcpy(optFile, TOP);		/* Construct the PTF option	*/
	strcat(optFile, "/HISTORY");     /*   file name before trying	*/
	strcat(optFile, "/ptfoptions"); /*   to open it for input.	*/
	if ((fpOpt = fopen(optFile, "r")) == NULL) {
	    printf("%s: Cannot open input %s.\n", whoami, optFile);
	    return(12);
	}
	while (fgets(buf, MAXBUFF, fpOpt) != NULL) {
	    sscanf(buf, "%s %s\n", ptf, option); /* Split out ptf & opt.*/
	    token = strtok(option, ",");

	    while (token != NULL) {		/* For each option,	*/
		strcpy(optn, token);		/*   store parsed optn, */
		linkOptions(ptf, optn);		/*   call linkOptions()	*/
		token = strtok(NULL, ",");	/*   for link-list	*/
	    }					/*   entry candidate.	*/
	}
	fclose(fpOpt);

	return(rc);

}	/* End of procOptions() ***  */

	/********************************************************
	*							*
	*	linkOptions() function accepts a PTF/option	*
	*	pair from procOptions parent function, and	*
	*	creates a link-list entry for that pair if	*
	*	that entry does not already exist and that	*
	*	PTF is in the pkgEntry list.			*
	*							*
	********************************************************/

void
linkOptions(ptfno, opt)

char		*ptfno, *opt;
{
	listRewind(optList);			/* Search from the start*/

	/** Look for this PTF/option until found or list is exhausted.	*/
	/** If found, exit the function; if not found, create an entry.	*/

	while (!listEnd(optList)) {
	    PtfOptEntry = (ptfOptEntry *) listGet(optList);
	    if (!strcmp(PtfOptEntry->ptf, ptfno) &&
		!strcmp(PtfOptEntry->option, opt) )
	    return;
	}					/* Not found, if here.	*/
	listRewind(pkgList);			/* Start search for this*/
	while (!listEnd(pkgList)) {		/*   PTF in pkgList.	*/
	    PkgEntry = (pkgEntry *) listGet(pkgList);
	    if ( !strcmp(PkgEntry->depPTF, ptfno) ) {
		PtfOptEntry = (ptfOptEntry *) malloc(MAXDIR);
		strcpy(PtfOptEntry->ptf, ptfno);/* So, create an entry.	*/
		strcpy(PtfOptEntry->option, opt);
		listAppend(optList, PtfOptEntry);
		return;
	    }
	}
	return;				/* Not found, return no entry.	*/

}	/* End of linkOptions() ***  */

	/********************************************************
	*							*
	*	procPkg() function reads 'ptf_pkg' file and 	*
	*	creates a reformatted 'wk_ptf_pkg' file;	*
	*	scans each line of test, creating a PTF/option	*
	*	pair for each ifreq/coreq/prereq PTF it finds	*
	*	paired with each option it finds on that line;	*
	*	and calls linkPreq() for each PTF/option pair.	*
	*							*
	********************************************************/

int
procPkg()
{
	char	buff[MAXBUFF];			/* Input line from pkg. */
	char	ptfFld[16];			/* 1st field from pkg.	*/
	char	apars[MAXLEN];			/* 2nd field from pkg.	*/
	char	fileName[MAXDIR];		/* 3rd field from pkg.	*/
	char	optns[MAXLEN];			/* 4th field from pkg.	*/
	char	ifreqs[MAXLEN];			/* 5th field from pkg.	*/
	char	coreqs[MAXLEN];			/* 6th field from pkg.	*/
	char	preqs[MAXLEN];			/* 7th field from pkg.	*/
	char	reqs[MAXLEN];			/* Work area for i,c,p. */
	char	*token;				/* Pntr to text in comma*/
	char	ptfno[16];			/* Temp PTF storage.	*/
	char	*ptr;				/* working ptr.		*/

	system("mv wk_ptf_pkg ptf_pkg");	/* Rename ptf-pkg file.	*/
	if ((fpPkg = fopen("ptf_pkg", "r")) == NULL) {
	    printf("%s: Cannot open input \"ptf_pkg\" file.\n", whoami);
	    return(21);
	}
	if ((fpPkgOut = fopen("wk_ptf_pkg", "w")) == NULL) {
	    printf("%s: Cannot open output \"wk_ptf_pkg\" file.\n", whoami);
	    return(22);
	}
	while (fgets(buff, MAXLEN, fpPkg) != NULL ) {
	    /* The next long stretch of code parses the seven fields	*/
	    /* of each input line into their respective storage areas.	*/
	    /* (sscanf and strtok not used here because empty or null	*/
	    /* fields are acceptable, but they skip such fields.)	*/
	    ptr = strchr(buff, '|');		/* Look for 1st field.	*/
	    if (ptr == NULL) {
		printf("%s: Vertical bar separator not found in ptf_pkg!\n",
			whoami);
		printf("%*c  Cannot process further with wrong format.\n",
			whoLen, ' ');
		return(23);
	    }
	    *ptr = '\0';			/* End fld1 w/stopper.	*/
	    strcpy(ptfFld, buff);		/* Put 1st fld in ptfFld*/
	    ptr += 1;  strcpy(buff, ptr);	/* Squeeze out 1st fld.	*/
	    ptr = strchr(buff, '|');		/* Look for 2nd field.	*/
	    if (ptr == NULL) {
		printf("%s: Not enough vertical bar separators in ptf_pkg!\n",
			whoami);
		printf("%*c  Cannot process %s %s\n",
			whoLen, ' ', ptfFld, buff);
		return(23);
	    }
	    *ptr = '\0';			/* End fld2 w/stopper.	*/
	    strcpy(apars, buff);		/* Put 2nd fld in apars */
	    ptr += 1;  strcpy(buff, ptr);	/* Squeeze out 2nd fld.	*/
	    ptr = strchr(buff, '|');		/* Look for 3rd field.	*/
	    if (ptr == NULL) {
		printf("%s: Not enough vertical bar separators in ptf_pkg!\n",
			whoami);
		printf("%*c  Cannot process %s %s %s\n",
			whoLen, ' ', ptfFld, apars, buff);
		return(23);
	    }
	    *ptr = '\0';			/* End fld3 w/stopper.	*/
	    strcpy(fileName, buff);		/* Put 3rd fld in filenm*/
	    ptr += 1;  strcpy(buff, ptr);	/* Squeeze out 3rd fld.	*/
	    ptr = strpbrk(buff, "|\n\0");	/* Look for 4th field.	*/
	    if (ptr == NULL) {
		printf("%s: Cannot find 'options' field in ptf_pkg!\n",
			whoami);
		printf("%*c  Cannot process %s %s %s %s\n",
			whoLen, ' ', ptfFld, apars, fileName, buff);
		return(23);
	    }
	    *ptr = '\0';			/* End fld4 w/stopper.	*/
	    strcpy(optns, buff);		/* Put 4th fld in optns	*/
	    ptr += 1;  strcpy(buff, ptr);	/* Squeeze out 4th fld.	*/
	    ptr = strpbrk(buff, "|\n\0");	/* Look for 5th field.	*/
	    if (ptr != NULL) {
		*ptr = '\0';			/* End fld5 w/stopper.	*/
		strcpy(ifreqs, buff);		/* Put 5th fld in ifreqs*/
		ptr += 1;  strcpy(buff, ptr);	/* Squeeze out 5th fld.	*/
		ptr = strpbrk(buff, "|\n\0");	/* Look for 6th field.	*/
		if (ptr != NULL) {
		    *ptr = '\0';		/* End fld6 w/stopper.	*/
		    strcpy(coreqs, buff);	/* Put 6th fld in coreqs*/
		    ptr += 1;  strcpy(buff, ptr);
		    ptr = strpbrk(buff, "|\n\0");  /* Look for 7th fld.	*/
		    if (ptr != NULL) {
			*ptr = '\0';		/* End fld7 w/stopper.	*/
			strcpy(preqs, buff);	/* Put 7th fld in preqs	*/
		    }
		}
	    }

	/**** Replace intrafield space separators with commas. **********/
	    ptr = strchr(apars, ' ');		/* Find space in apars	*/
	    while (ptr != NULL) {
		*ptr = ',';			/* replace with comma	*/
		ptr = strchr(apars, ' ');	/* until all done.	*/
	    }   
	    ptr = strchr(optns, ' ');		/* Find space in optns	*/
	    while (ptr != NULL) {
		*ptr = ',';			/* replace with comma	*/
		ptr = strchr(optns, ' ');	/* until all done.	*/
	    }   
	    ptr = strchr(ifreqs, ' ');		/* Find space in ifreq	*/
	    while (ptr != NULL) {
		*ptr = ',';			/* replace with comma	*/
		ptr = strchr(ifreqs, ' ');	/* until all done.	*/
	    }   
	    ptr = strchr(coreqs, ' ');		/* Find space in coreq	*/
	    while (ptr != NULL) {
		*ptr = ',';			/* replace with comma	*/
		ptr = strchr(coreqs, ' ');	/* until all done.	*/
	    }   
	    ptr = strchr(preqs, ' ');		/* Find space in preqs	*/
	    while (ptr != NULL) {
		*ptr = ',';			/* replace with comma	*/
		ptr = strchr(preqs, ' ');	/* until all done.	*/
	    }   
	    if ( !strcmp(fileName, "/ins_chg") ) /* Edit fileName to be */
		strcpy(fileName, "ins_chg");	 /* ins_chg w/o slash.	*/

	    fprintf(fpPkgOut, "%s %s %s %s\n",	/* Write new form of 	*/
		ptfFld, apars, fileName,optns);	/*  wk_ptf_pkg file.	*/

	/****** Step thru each option and create a pkg list entry    ****/
	/****** for each unique combination of option name and       ****/
	/****** ifreq/coreq/preq PTF Number.			     ****/
	    ptr  = optns + strlen(optns);	/* Pnt to end of opt.	*/
	    *ptr = ',';				/* Put comma at end.	*/
	    ptr +=1;  *ptr = '\0';		/* Put null after comma.*/
	    while ((ptr  = strchr(optns, ',')) != NULL ) {
		*ptr = '\0';			/* Change comma to null.*/
		strcpy(reqs, ifreqs);		/* Put ifreqs in work.	*/
	    	token = strtok(reqs, ",");
	    	while (token != NULL) {		/* Look for dependent	*/
			strcpy(ptfno, token);	/* PTF numbers & call	*/
			linkPreq(optns, ptfno, 'i'); /* linkPreq() if	*/
			token = strtok(NULL, ",");   /* ifreq found.	*/
	    	}

		strcpy(reqs, coreqs);		/* Put coreqs in work.	*/
	    	token = strtok(reqs, ",");
	    	while (token != NULL) {
			strcpy(ptfno, token);
			linkPreq(optns, ptfno, 'c');
			token = strtok(NULL, ",");
	    	}

		strcpy(reqs, preqs);		/* Put preqs in work.	*/
	    	token = strtok(reqs, ",");
	    	while (token != NULL) {
			strcpy(ptfno, token);
			linkPreq(optns, ptfno, 'p');
			token = strtok(NULL, ",");
	    	}
	    strcpy(optns, ptr + 1);		/* Squeeze out used optn*/
	    }	/* End of while for options. */
	}	/* End of while for read. */

	fclose(fpPkg);
	fclose(fpPkgOut);

	return;

}	/* End of procPkg	 */

	/********************************************************
	*							*
	*	linkPreq() function accepts an option-name,	*
	*	PTF number, and type (i = ifreq, c = coreq,	*
	*	p = prereq) and scans the ptf-pkg prereq	*
	*	link-list for a match on option and PTF nbr.	*
	*	If a match is found, the function exits; if	*
	*	a match is not found, a link-list entry is	*
	*	created for that option-PTF pair.		*
	*							*
	********************************************************/

void
linkPreq(opt, ptfno, type)

char	*opt, *ptfno, type;
{
	listRewind(pkgList);
	while (!listEnd(pkgList)) {		/* Look for link-list	*/
	    PkgEntry = (pkgEntry *) listGet(pkgList);	/* match.	*/
	    if ( !strcmp(PkgEntry->depPTF, ptfno) &&	/* PTF nbr =?	*/
		 !strcmp(PkgEntry->option, opt)   &&	/* Option  =?	*/
		 PkgEntry->function == type )		/* Function=?	*/
	    return;				/* If matched, exit.	*/
	}
	PkgEntry = (pkgEntry *) malloc(MAXDIR);	/* Not matched, set up	*/
	strcpy(PkgEntry->option, opt);		/*   new entry in the	*/
	strcpy(PkgEntry->depPTF, ptfno);	/*   ptf-pkg link-list.	*/
	PkgEntry->function = type;
	listAppend(pkgList, PkgEntry);
	return;

}	/* End of linkPreq() ***  */

	/********************************************************
	*							*
	*	writePreq() function is called after the	*
	*	ptfOption link-list and the ptf_pkg link-list	*
	*	have both been created.  This function uses	*
	*	the ptf_pkg list as the driver to compare PTF	*
	*	numbers against those in the ptfoption list.	*
	*	Matched PTF numbers are written, along with	*
	*	text for the type(function) and option-name,	*
	*	to <option>.pre.pre file.			*
	*	A later update process will merge these files	*
	*	as text into the 'lppname' file.		*
	*							*
	********************************************************/

int
writePreq()
{
	char	type[8];		/* Type char string put here.	*/
	char	fileName[MAXDIR];	/* Output *.pre.pre file name.  */
	char	found[2];		/* "y" means match found.	*/
	int	matcherr;		/* Count mismatches.		*/

	system("rm -f *.pre.pre");	/* Insure only now files present*/
	matcherr = 0;			/* Init error count.		*/
	listRewind(pkgList);		/* Get ptf-pkg list elements in */
	while (!listEnd(pkgList)) {	/* outer loop to serve as driver*/
	    PkgEntry = (pkgEntry *) listGet(pkgList);
	    strcpy(found, "n");		/* Reset match found switch.	*/
	    listRewind(optList);	/* Now, for each ptf-pkg entry,	*/
	    while (!listEnd(optList)) { /*  scan option list for match.	*/
		PtfOptEntry = (ptfOptEntry *) listGet(optList);
		if (!strcmp(PkgEntry->depPTF, PtfOptEntry->ptf) ) {
		    strcpy(found, "y");	/* Set match found switch.	*/
			if (PkgEntry->function == 'i')
			    strcpy(type, "ifreq");
			else
			if (PkgEntry->function == 'c')
			    strcpy(type, "coreq");
			else
			if (PkgEntry->function == 'p')
			    strcpy(type, "prereq");
	/***** For each match open <option.pre.pre output file. ********/
			strcpy(fileName, PkgEntry->option);
			strcat(fileName, ".pre.pre");
			fpPreq = fopen(fileName, "a");
			fprintf(fpPreq, "*%s %s p=%s\n", type,
				PtfOptEntry->option, PtfOptEntry->ptf);
			fclose(fpPreq);
		}  /* End of if. */

	    }	   /* End of while for options. */

	    if (strcmp(found, "y")) {		/* True if not found.	*/
		printf("%s: Error: No ptfoption match found for %s %s.\n",
			whoami, PkgEntry->option, PkgEntry->depPTF);
		matcherr++;
	    }
	}	/* End of while for pkg list. */
	if (matcherr)
	    return(31);
	else
	    return(0);

}	/*  End of writePreq()   */


	/*----------------------------------------------+
	|		List routines			|
	|	(common	to ORBIT List routines)		|
	+----------------------------------------------*/

List *
listNew()
{
	extern char	*space();
	return (List *)	space(sizeof(List));
}

static Link *
linkNew()
{
	extern char	*space();
	return (Link *)	space(sizeof(Link));
}

void
listInit(l)

List	*l;
{
	l->count = 0;
	l->last	= l->current = (Link *)0;
}

int
listEnd(l)

List	*l;
{
	return (l->current == (Link *) 0);
}

void
listRewind(l)

List	*l;
{
	l->current = l->last->next;
}

char *
listGet(l)

List	*l;
{
	char	*d;

	d = l->current->data;
	l->current = (l->current == l->last) ? (Link *)	0 : l->current->next;
	return(d);
}

void
listInsert(l, data)

List	*l;
char	*data;
{
	Link	*newLink = linkNew();

	if (l->last) {
		newLink->next =	l->last->next;
		l->last->next =	newLink;
	}
	else {
		newLink->next =	newLink;
		l->last	= newLink;
	}
	newLink->data =	data;
	l->count++;
}

void
listAppend(l, data)

List	*l;
char	*data;
{
	listInsert(l, data);
	l->last	= l->last->next;
}

char *
space(size)

int	size;
{
	extern char	*calloc();
	char		*new;

	new = calloc(size, 1);
	return(new);
}

/***** End of gensep *** End of gensep *** End of gensep *****/
