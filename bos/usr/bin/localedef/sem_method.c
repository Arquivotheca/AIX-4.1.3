static char sccsid[] = "@(#)52	1.6  src/bos/usr/bin/localedef/sem_method.c, cmdnls, bos411, 9436D411a 9/7/94 21:01:22";
/*
 * COMPONENT_NAME: (CMDNLS) Locale Database Commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/types.h>
#include <sys/localedef.h>
#include <sys/lc_sys.h>
#include <sys/limits.h>
#include <ctype.h>
#include <errno.h>
#include "semstack.h"
#include "symtab.h"
#include "method.h"
#include "err.h"

/* 
** GLOBAL variables 
*/
std_method_t loc_std_methods[LAST_METHOD+1];	/* array containing user     */
						/* defined methods           */

static char *lib_array[LAST_METHOD+1];	/* array containing all the libraries */
			                /* specified in methods source file */

static char *tmpmethod;		/* pointer to the temporary methods file     */

static char libpath[_POSIX_PATH_MAX]=LIBC;

static char *libpath_ptr = libpath;

char *lib_str;			/* pointer to the string of concatenated     */
				/* libraries names			     */

extern int private_table;
extern char *tmp_method_path;

/* FUNCTION: set_index 
*
*  DESCRIPTION:
*	Used to set the global index and the global name for a global
*	method.
*
*/
void 
set_index(int index, char *name)
{
	extern int g_tbl_index;
	extern char g_method_name[];

	g_tbl_index = index;
	strcpy(g_method_name, name);
}

/*
*  FUNCTION: set_method_private
*
*  DESCRIPTION:
*  	Used to set the c_symbol name and the library name for the new
*	method. An index into the methods array is passed to this routine
*	from the grammar. The c_symbol is found on the stack as an item.
*	The library name has already been set (or inherited).
*/
void set_method_private(int index)
{
    	item_t *it1;
	int len;
	


	/* get the string for the new method name off of the stack */
	it1=sem_pop();

	/* the item can not be null and must be a string */
	if (it1 == NULL || it1->type != SK_STR){ 
	   INTERNAL_ERROR;
	}

	/* add the info to the loc_std_methods table */
	/* Since the table currently just points to the std_methods table */
	/* space must be allocated before copying the information over. */

	loc_std_methods[index].c_symbol[USR_DEFINED] = strdup(it1->value.str);
	if (loc_std_methods[index].c_symbol[USR_DEFINED] == NULL)
	    error(ERR_MEM_ALLOC_FAIL);
	destroy_item(it1);

	loc_std_methods[index].pkg_name[USR_DEFINED] = strdup(libpath_ptr);
	if (loc_std_methods[index].pkg_name[USR_DEFINED] == NULL)
	    error(ERR_MEM_ALLOC_FAIL);
}

/*
*  FUNCTION: set_method_global
*
*  DESCRIPTION:
*  	Used to set the index for the global method. An index into the methods 
*       array is passed to this routine from the grammar. The index in the
*	global table is also passed to this routine. There should be no
*	library specified (all routines are in libc.a).
*
*/
void set_method_global(int loc_index, int tbl_index, char *name)
{
	int len;
	


	/* add the info to the loc_std_methods table */
	/* Since the table currently just points to the std_methods table */
	/* space must be allocated before copying the information over. */

	loc_std_methods[loc_index].c_symbol[USR_DEFINED] = strdup(name);
	if (loc_std_methods[loc_index].c_symbol[USR_DEFINED] == NULL)
	    error(ERR_MEM_ALLOC_FAIL);

	loc_std_methods[loc_index].method[USR_DEFINED] = tbl_index;
}

/*
*  FUNCTION: set_method_lib_path
*
*  DESCRIPTION:
*  Sets the libpath for finding the method. This variable is global because
*  libpath can be inherited.
*/

void set_method_lib_path()
{
	item_t *it1;
	int len;
	int i;

	/* pop the library name off the stack */
	it1 = sem_pop();

	/* library name must not be NULL and it must be a string */
	if (it1 == NULL || it1->type != SK_STR) {
	    INTERNAL_ERROR;
	}

	/* Check the length of the new library path, and copy over */
	/* the string once the correct amount of space is available */
	len = strlen(it1->value.str);
	if (strlen(libpath_ptr) <= len) {
	    free(libpath_ptr);
	    libpath_ptr = malloc(len+2);
	}
	if (libpath_ptr == NULL) {
	    error(ERR_MEM_ALLOC_FAIL);
	}
	strcpy(libpath_ptr, it1->value.str);
	destroy_item(it1);

	/* add the library to the lib_array so that it can be linked */
	/* in later. 						     */
	for (i=0; i<LAST_METHOD;i++) {
	    if (lib_array[i] == NULL) {
		lib_array[i] = strdup(libpath_ptr);
		if (lib_array[i] == NULL) {
		    error(ERR_MEM_ALLOC_FAIL);
		}
	    	break;
	    }
	    else if (!strcmp(lib_array[i],libpath_ptr)){
		break;
	    }
	}
}
		
/*
*  FUNCTION: init_loc_std_methods
*
*  DESCRIPTION:
*  Set the loc_std_methods table to point to the std_methods table.
*  This is the table used by localedef to determine what methods
*  are used. The lib_array is also initialized in this routine.
*/

void init_loc_std_methods()
{

	int i;

	/* set up pointers for each method */
	for (i=0;i<=LAST_METHOD;i++){
	    loc_std_methods[i] = std_methods[i];
	}


	/* The library array contains the names of the different   */
	/* libraries used for the locale being built. Each locale  */
	/* uses at least the /usr/lib/libc.a library		   */

	for (i = 0; i <= LAST_METHOD; i++) 
	    lib_array[i] = NULL;
}


/*
*  FUNCTION: build_method_file
*
*  DESCRIPTION:
*  Builds the temporary method file for localedef to load
*/
void build_method_file()
{
	FILE *method_fp;
	int i,j;

	tmpmethod = tempnam("./","method");
	strcat(tmpmethod,".c");
	method_fp = fopen(tmpmethod,"w");

	if (method_fp == NULL) {
	    error(ERR_WRT_PERM, tmpmethod);
	}

	/* include files needed			*/
	fprintf(method_fp,"#include <sys/limits.h>\n");

	/* for pointer methods, the functions need to be "externed" */
	/* if there is no function name, put a "0" to be put out in */
	/* the method structure */

	fprintf(method_fp,"\n");

	for (i=0;i<=LAST_METHOD;i++) {

	        if (loc_std_methods[i].c_symbol[USR_DEFINED] == NULL) {
	            free(loc_std_methods[i].c_symbol[USR_DEFINED]);
	            loc_std_methods[i].c_symbol[USR_DEFINED] = malloc(3);
		    if (loc_std_methods[i].c_symbol[USR_DEFINED] == NULL) {
			error(ERR_MEM_ALLOC_FAIL);
		    }
	            strcpy(loc_std_methods[i].c_symbol[USR_DEFINED], "0");
	        }

	        else{
	            fprintf(method_fp,"extern %s(); \n",
	 	            loc_std_methods[i].c_symbol[USR_DEFINED]);
	        }
	}

#ifndef _PTR_METH

	/* if using the index method and a private table is needed,   */
	/* a temporary methods table needs to be created	      */
     
        if (private_table) {
    		fprintf(method_fp, "\n");
		fprintf(method_fp, "int (*_tmp_method_tbl[])() = {\n");
		for (i=0; i<=LAST_METHOD;i++){
	    		fprintf(method_fp, "\t%s, \n",
			    loc_std_methods[i].c_symbol[USR_DEFINED]);
		}
		fprintf(method_fp, "};\n");
	}

#endif
	
	/* build the temporary std_methods file, only putting out the */
	/* information for the user defined methods 		       */

	fprintf(method_fp,"\ntypedef struct { \n");
	fprintf(method_fp,"\t\t  char *method_name;\n");
	fprintf(method_fp,"\t\t  char *c_symbol[%d];\n",MX_METHOD_CLASS);
	fprintf(method_fp,"\t\t  char *pkg_name[%d];\n",MX_METHOD_CLASS);
#ifdef _PTR_METH
	fprintf(method_fp,"\t\t  void *(*method[%d])();\n",MX_METHOD_CLASS);
#else
	fprintf(method_fp,"\t\t  int method[%d];\n",MX_METHOD_CLASS);
#endif
	fprintf(method_fp,"\t\t } std_method_t;\n");

	fprintf(method_fp,"\n\nstd_method_t _tmp_std_methods[]={\n");

	for (i=0;i<=LAST_METHOD;i++) {
	    fprintf(method_fp,"{\"%s\",\n",loc_std_methods[i].method_name);

	    fprintf(method_fp,"  {0, 0, 0, \"%s\"},\n",
		    loc_std_methods[i].c_symbol[USR_DEFINED]);

	    fprintf(method_fp,"  {0, 0, 0, \"%s\"},\n",
		    loc_std_methods[i].pkg_name[USR_DEFINED]);

#ifdef _PTR_METH

	    /* print out the symbol for the method instance */

	    fprintf(method_fp,"  {0, 0, 0, %s}},\n",
		    loc_std_methods[i].c_symbol[USR_DEFINED]);

#else
	
	    /* print out the index for the method in the method table */

	    if (private_table) {
	    	fprintf(method_fp,"  {-1, -1, -1, %d}},\n",i);
	    }
	    else {
	        fprintf(method_fp,"  {-1, -1, -1, %d}},\n",
			loc_std_methods[i].method[USR_DEFINED]);
	    }

#endif

	}
	fprintf(method_fp,"};\n\n");


	/* this will return the address of the temporary std methods  */
	/* table and the temporary method table 		      */
        fprintf(method_fp,"\t typedef struct {\n");
	fprintf(method_fp,"\t\t std_method_t * std_meth_ptr;\n");
#ifndef _PTR_METH
	fprintf(method_fp,"\t\t void * (**method_tbl_ptr)();\n");
#endif
	fprintf(method_fp,"} std_meth_ptr_t;\n");

	fprintf(method_fp, "\tstd_meth_ptr_t std_method_hdl = {\n");
	fprintf(method_fp, "\t (std_method_t *)&_tmp_std_methods,\n");
#ifndef _PTR_METH
	if (private_table) 
		fprintf(method_fp, "\t (void*(**)())&_tmp_method_tbl,\n");
	else
		fprintf(method_fp, "\t (void*(**)())0, \n");
#endif
	fprintf(method_fp, "};\n");


	fclose(method_fp);

	/* now we will free all the memory used in the table */

#ifdef _PTR_METH /* free table now if using pointer method, otherwise,  */
		 /* keep around for later for building method_tbl in    */
		 /* gen.c						*/

	for (i=0;i<=LAST_METHOD;i++){

	    if (loc_std_methods[i].method_name != NULL)
	        free(loc_std_methods[i].method_name);

	    if (loc_std_methods[i].c_symbol[USR_DEFINED] != NULL)
	        free(loc_std_methods[i].c_symbol[USR_DEFINED]);

	    if (private_table) {
	    	if (loc_std_methods[i].pkg_name[USR_DEFINED] != NULL)
	        	free(loc_std_methods[i].pkg_name[USR_DEFINED]);
	    }

	}

#endif
}


/*
*  FUNCTION: cc_ld_method_file
*
*  DESCRIPTION:
*  routine to create the .o of the temporary method file and link it with
*  the specified libraries.
*/

void cc_ld_method_file()
{
	char *cmdstr;
	char *c_ptr;
	char *tmpmethod_o;
	int i,ret;
	int lib_len,cmd_len,len;
	extern char *tpath;
	extern char *ccopts;
	extern char *ldopts;
	extern verbose;

	if (private_table) {
	    lib_len = 0;
	    for (i=0;i<=LAST_METHOD;i++) {
		if (lib_array[i] != NULL) {
		    lib_len = lib_len + strlen(lib_array[i]) + 1;
		}
		else
		    break;
	    }
	} else
	    lib_len = strlen(LIBC) + 1;

	lib_str = malloc(lib_len+2);

	if (lib_str == NULL) {
	    error(ERR_MEM_ALLOC_FAIL);
	}

	/* create a list of libraries specified by the methods file for */
	/* use in the ld statement - this is also used by in the main   */
	/* part of localedef when the locale is created			*/

	if (private_table) {
	    lib_str[0]='\0';
	    for (i=0; i<=LAST_METHOD;i++) {
		if (lib_array[i] != NULL) {
		    strcat(lib_str,lib_array[i]);
		    strcat(lib_str," ");
		}
		else
		    break;
	    }
	} else {
	    strcat(lib_str,LIBC);
	    strcat(lib_str," ");
	}

	/* No need to compile the temporary method */
	/* if it has already been provided.        */
	if (tmp_method_path) {
	    tmpmethod[strlen(tmpmethod)-2]='\0';
	    return;
	}

	/* create command string for compiling file */
	cmd_len = strlen(tmpmethod) + strlen(tpath) + strlen(ccopts) +
		  sizeof(CC_CMD_FMT) + 32;
	cmdstr = malloc(cmd_len);
	if (cmdstr == NULL){
	    error(ERR_MEM_ALLOC_FAIL);
	}
	sprintf(cmdstr,CC_CMD_FMT,tpath,tmpmethod,ccopts);

	if (verbose)
	     printf("%s\n",cmdstr);

	ret = system(cmdstr);
	free(cmdstr);
	if (ret != 0){
	    error(ERR_BAD_CHDR);
	}

	/* strip off .c */
	tmpmethod[strlen(tmpmethod)-2]='\0';
	tmpmethod_o = malloc(strlen(tmpmethod) + 4);
	if (tmpmethod_o == NULL)
	    error(ERR_MEM_ALLOC_FAIL);
	strcpy(tmpmethod_o,tmpmethod);
	strcat(tmpmethod_o,".o");

	/* create command string for linking file */

	cmd_len =  strlen(tpath) + lib_len + sizeof(LDOPT_CMD_FMT_METH) + 
		   strlen(tmpmethod) + strlen(tmpmethod_o) + 
		   strlen(ldopts) + 32;
	cmdstr = malloc(cmd_len);

	if (cmdstr == NULL){
	    error(ERR_MEM_ALLOC_FAIL);
	}
	sprintf(cmdstr,LDOPT_CMD_FMT_METH,tpath,tmpmethod_o,lib_str,ldopts,tmpmethod);

	if (verbose)
	    printf("%s\n",cmdstr);

	ret = system(cmdstr);
	free(cmdstr);
	if (ret != 0){
	    error(ERR_BAD_CHDR);
	}
	
}


/*
*  FUNCTION: load_method
*
*  DESCRIPTION:
*  loads the temporary method file just created. Sets the std_methods
*  table equal to temporary method table for localedef's processing
*  of the charmap and source file. 
*/

void load_method()
{
	int len;
	extern verbose;

	std_meth_ptr_t *q;		/* structure containing the 	*/
					/* address of the std methods   */
					/* and the tmp method table	*/

	if (tmp_method_path)
	    q = (std_meth_ptr_t *)load(tmp_method_path,0,"");
	else
	    q = (std_meth_ptr_t *)load(tmpmethod,0,"");
	if (q == NULL) {
	   perror("localedef");
	   exit(4);
	}
	
	std_methods = q->std_tbl_ptr;
#ifndef _PTR_METH
	if (private_table) {
		__method_table = q->meth_tbl_ptr;
	}

#endif
	/* remove files */
	unlink(tmpmethod);
	len = strlen(tmpmethod);
	strcat(tmpmethod,".c");
	if (verbose) {
	    extern symtab_t cm_symtab;
	    extern char *locname;
	    char *s;

	    s = MALLOC(char, strlen(locname)+14);
	    strcpy(s, locname);
	    strcat(s, "_tmp_meth.c");
	    unlink(s);
	    rename(tmpmethod, s);
	}
	unlink(tmpmethod);
	tmpmethod[len] = '\0';
	strcat(tmpmethod,".o");
	unlink(tmpmethod);	
}


/*
*  FUNCTION: check_methods
*
*  DESCRIPTION:
*  There are certain methods that do not have defaults because they are
*  dependent on the process code and file code relationship. These methods
*  must be specified by the user if they specify any new methods at all
*
*/

void check_methods()
{
	/* list of method names that must be defined by the user if they */
	/* are adding any new methods. They do not have defaults that    */
	/* will work for any given codeset				 */
	int methods_defined[11]={CHARMAP___MBSTOPCS, CHARMAP___MBTOPC,
		CHARMAP___PCSTOMBS, CHARMAP___PCTOMB, CHARMAP_MBLEN,
		CHARMAP_MBSTOWCS, CHARMAP_MBTOWC, CHARMAP_WCSTOMBS,
		CHARMAP_WCSWIDTH, CHARMAP_WCTOMB, CHARMAP_WCWIDTH,};

	int i;

	for (i = 0; i < 11; i++){	
	    if ((loc_std_methods[methods_defined[i]].c_symbol[USR_DEFINED] 
		== NULL) ||
		(loc_std_methods[methods_defined[i]].c_symbol[USR_DEFINED][0]
		== '0')) {
	        /* add new message, this is just a place holder */
		unlink(tmpmethod);
		error(ERR_METHODS);
	    }
	}
}
