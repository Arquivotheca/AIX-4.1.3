static char sccsid[] = "@(#)27	1.56.3.12  src/bos/usr/ccs/lib/libdbx/dpi_var.c, libdbx, bos411, 9428A410j 6/27/94 14:08:27";
/*
 *   COMPONENT_NAME: LIBDBX
 *
 *   FUNCTIONS: adc_function_list
 *		adc_namecmp
 *		adc_var_lang_init
 *		adc_variable_list
 *		add_inlined_index
 *		arrayElemName
 *		c_changeName
 *		c_varDecl
 *		changeArrayDecl
 *		changeDecl
 *		changeSubscript
 *		changeSubscriptForCobolArrays
 *		className
 *		cobol_changeName
 *		cobol_varDecl
 *		count_nestedclass_members
 *		cpp_varDecl
 *		create_varStruct_for_cobol_arrays
 *		declLines
 *		dpi_array_element_subscripts
 *		dpi_change_array_subscripts
 *		dpi_current_array_element_name
 *		dpi_decl_lines
 *		dpi_expand_variable
 *		dpi_expand_variable_inline
 *		dpi_file_cmp
 *		dpi_find_externals
 *		dpi_find_inlined_variables
 *		dpi_free_variable
 *		dpi_get_files
 *		dpi_get_func_scope
 *		dpi_get_lang
 *		dpi_get_scope
 *		dpi_get_var_offset
 *		dpi_if_cobol
 *		dpi_max_array_element_name
 *		dpi_set_variable_format
 *		dpi_symbol_cmp
 *		dpi_unexpand_variable_inline
 *		dpi_var_element
 *		dpi_var_scope
 *		dpi_variable_block
 *		dpi_variable_list
 *		dpi_variable_name
 *		dpi_variable_offset
 *		fieldName
 *		fill_base_for_cobol_array_elements
 *		find_dimension_of_cobol_arrays
 *		fortran_changeName
 *		fortran_varDecl
 *		free_chain
 *		free_range
 *		free_sourcepath
 *		get_include_files
 *		is_variable_a_cobol_array
 *		propagate_offset
 *		reset_index_under_union
 *		subscriptString
 *		subtract_inlined_index
 *		targetName
 *		varClass
 *		var_arrayChain
 *		var_chain
 *		var_class
 *		var_element
 *		var_lang_init
 *		is_anon_union
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *   OBJECT CODE ONLY SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
#include <setjmp.h>
#include <sys/termio.h>
#include    "defs.h"
#include    "envdefs.h"
#include    "process.h"
#include    "symbols.h"
#include    "lists.h"

#include <stdio.h>
#include <filehdr.h>
#include <ldfcn.h>
#include  <syms.h>
#include  <aouthdr.h>
#include  <scnhdr.h>
#include  <dbxstclass.h>
#include "mappings.h"
#include "object.h"
#include "cma_thread.h"
#include "k_thread.h"
#define  is_variable_a_cobol_array(s)  find_dimension_of_cobol_arrays(s)
#define  INLINED_RECORD          0
#define  DECLARATION_RECORD      1
#define AVG_NO_FILES	10
/* define macro to indicate whether a symbol for fortran symbol
   of class TYPE is a derived type */
#define isfortranderived(symbol) ( \
    (symbol->class == TYPE)                    && \
    (symbol->language == fLang) && \
    (symbol->type != nil)                      && \
    ((symbol->type->class == RECORD)           || \
     (symbol->type->class == PACKRECORD)) \
)

FILHDR   filhdr;
LDFILE *aout = 0;

char * debugtab = 0;	/* Used for holding symbols in the .debug section */
char * stringtab = 0;		/* Used for holding string table */
extern int loadcnt;
int dpi_symbol_cmp();
void dpi_get_scope();

extern VarClass  varClass( );

extern int    	*envptr;            /* setjmp/longjmp data */
extern boolean  unique_fns;
extern int  lazy;

extern Language cLang;
extern Language cppLang;
extern Language cobLang;


int        	declindex = 0;			/* next display index */
boolean		reverseSubscripts = false;	/* true for fortran */
char		*beg_delim = "[";
char		*mid_delim = "][";
char		*end_delim = "]";
VarLanguage  	var_lang;
void		var_lang_init();
int		c_varDecl( );
int		cpp_varDecl( );
int             cobol_varDecl( );
int		c_changeName( );
int             cobol_changeName( );
int		fortran_varDecl( );
int		fortran_changeName( );
int		( *varDecl )( ) = c_varDecl;
int		( *changeName )( ) = c_changeName;

public Symbol *dump_externs();
void dpi_var_scope();

/*
 * NAME: dpi_variable_list
 *
 * FUNCTION: Returns all the variables within the given scope in an array of
 *	     VarStructs
 *
 * NOTES: Used only by xde
 *	Method used to generate the lists:
 *		1a) LOCAL list, get the symbol associated with the function
 *		1b) GLOBAL list, get the symbol associated with the file
 *		2) Call var_lang_init() on the symbol to set the language
 *		3a) LOCAL list, call get_all_local() to generate a list of all
 *		    local variables in the specified function
 *		3b) GLOBAL list, call dump_externs() to generate a list of all
 *		    file static variables in the file, and all external
 *		    variables defined in the set of files specified by
 *		    file_lookup()
 *		4) Sort the list alphabetically
 *		5) LOCAL list, generate list of names with scopes for hiddens
 *		6) Generate array of VarStructs from the list
 *
 * PARAMETERS:
 *	module		- Name of block from which to get local variables
 *	scope		- Either LOCAL or GLOBAL; Indicates which list to obtain
 *	vars		- Will be filled in with array of VarStructs; Will
 *			  allocate an array of pointers to VarStructs, Each
 *			  VarStruct will also be allocated.
 *	lang		- Will be set to the language of the current block
 *	file_lookup	- Used if scope is GLOBAL to delimit list of external
 *			  variables obtained
 *	names		- Filled in with array of strings which contains scope
 *			  information on any hidden local variables when scope
 *			  is LOCAL; Will allocate array of pointers to character
 *			  strings, each character string will also be allocated.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: declindex is set
 *
 * RETURNS: -1 if failure;
 *	    Otherwise the number of variables in the returned array
 */
int 	dpi_variable_list( module, scope, vars, lang, file_lookup, names )
char    		*module;
int			scope;
struct VarStruct      	***vars;
VarLanguage		*lang;
#ifdef _NO_PROTO
int			(*file_lookup)();
#else
int			(*file_lookup)( char *, struct SUFLIST * );
#endif
char			***names;
{
    int 	index, names_len;
    int		numvars = 0;
    char	*filename;
    char	*fileptr;
    char	*period_ptr, **scope_arr = NULL;
    Name    	modulename;
    Symbol    	s;
    Symbol	*symarr = NULL;
    Symbol	modulesym;
    struct VarStruct     **vptr;
    jmp_buf     env;
    int         *svenv;

    svenv = envptr;
    envptr = env;
    if (setjmp( env )) {
        return -1;
    }

    /*   get symbol associated with function name   */
    if ( scope == LOCAL ) {
	if ( !strcmp( module, "." ) ) {
	    /* Don't get locals for the program level */
            envptr = svenv;
            return 0;
        }
        modulename = identname( module, false );
	/*
	 * Use the current function as our scope, unless the name does not match
	 * modulename.  This will happen at __exit, when curfunc still points to
	 * main(), but we don't want to get local variables for main()
	 */
	modulesym = curfunc;
	if( modulesym->name != modulename ) {
	    modulesym = lookup( modulename );
	    while( modulesym != nil &&
		   !( modulesym->name == modulename &&
		      isroutine( modulesym ) )) {
		modulesym = modulesym->next_sym;
	    }
	}
    } else if ( scope == GLOBAL ) {
	if ( !cursource ) {
            envptr = svenv;
            return 0;
         }
	/* Get the name of the file without including any path information */
	if ( !( fileptr = strrchr( cursource, '/' ) ) )
	    fileptr = cursource;
	else
	    fileptr++;
	/* If we have been called with the -u option,
	 * add the '@' to the name
	 */
	if (unique_fns) {
		filename = malloc( (strlen( fileptr ) + 2) * sizeof( char ) );
		*filename = '@';
		strcpy(filename + 1, fileptr );
	} else {
		filename = malloc( (strlen( fileptr ) + 1) * sizeof( char ) );
		strcpy( filename, fileptr );
	}
	/* Remove the extension from the filename */
	if ( period_ptr = strrchr( filename, '.' ) )
	    *period_ptr = nil;
	/* Find the filename in dbx's symbol heap */
        modulename = identname( filename, false );
	modulesym = lookup( modulename );
	while( modulesym != nil &&
	      !(modulesym->name == modulename && modulesym->class == MODULE )) {
	    modulesym = modulesym->next_sym;
	}
	dispose( filename );
    } else {	/* Invalid scope */
        envptr = svenv;
        return -1;
    }
    if ( !modulesym ) {
        envptr = svenv;
        return 0;
    }

    /*   initialize language-dependent dpi variables   */

    var_lang_init( modulesym );
    *lang = var_lang;

    /*   unsupported language   */

    if ( *lang == UNSUPPORTED_LANG ) {
        envptr = svenv;
	return 0;
    }

    /*   calculate number of arguments and variables   */

    if ( scope == LOCAL )
	get_all_local( modulesym, &symarr, &numvars );
    else if ( scope == GLOBAL )
	symarr = dump_externs(fileptr, modulesym, &numvars, file_lookup);

    /*
     * Sort the list alphabetically
     */
    qsort( symarr, numvars, sizeof( *symarr ), dpi_symbol_cmp );

    if( scope == LOCAL ) {
	/*
	 * Allocate enough space to store scoping information for variables
	 */
	if( numvars > 0 ) {
	     /*
	      * Use calloc() here since this will initialize all entries to
	      * NULL and we count on that below
	      */
	     scope_arr = (char **) calloc( numvars, sizeof( char *));

	     /*
	      * Need to remove any duplicate variables from the list,
	      * and get scope information for any hidden variables
	      */
	     dpi_get_scope( numvars, symarr, scope_arr );
	}

	*names = (char **)malloc( (numvars + 1) * sizeof( char * ));
	for( index = 0; index < numvars; index++ ) {
	  s = symarr[index];
	  if( s ) {
	    if( scope_arr[index] ) {
		/*
		 * Add the length of the name and scope information, plus 4
		 * characters for the space, '(', ')', and NULL
		 */
		names_len = strlen(symname( s )) +
			    strlen( scope_arr[index] ) + 4;
		*(*names + index) = malloc( names_len * sizeof( char ));
		sprintf( *(*names + index), "%s (%s)", symname( s ),
			 scope_arr[index] );
	    } else {
		/*
		 * Add the length of the name plus 1 for the NULL
		 */
		names_len = strlen(symname( s )) + 1;
		*(*names + index) = malloc( names_len * sizeof( char ));
		sprintf( *(*names + index), "%s", symname( s ));
	    }
	  }
	}
	*(*names + index) = NULL;

	for( index = 0; index < numvars; index++ )
	    if( scope_arr[index] ) free( scope_arr[index] );
	if( scope_arr ) free( scope_arr );
    }

    /*   allocate space for variable list   */
    *vars = ( struct VarStruct ** ) malloc( (numvars + 1) *
					    sizeof( struct VarStruct * ) );
    vptr = *vars;

    /*   build variables   */

    for ( index = 0; index < numvars; index++, vptr++ ) {
        *vptr = ( struct VarStruct * )malloc( sizeof( struct VarStruct ) );
        s = symarr[index];
        declindex = 0;
	/* UNKNOWN_LIST parameter is used for ADC but is ignored for xde */
        if ( var_element( s, s->type, nil, false, *vptr, UNKNOWN_LIST ) ) {
	    *vptr = ( struct VarStruct * )nil;
	    return -1;
	}
    }
    *vptr = ( struct VarStruct * )nil;

    /*
     * No longer need symarr, free the storage
     */
    dispose( symarr );

    envptr = svenv;
    return numvars;
}


/*
 * NAME: dpi_symbol_cmp
 *
 * FUNCTION: Compares two symbol names
 *
 * PARAMETERS:
 *	a	- First function table symbol to compare
 *	b	- Second function table symbol to compare
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: 0 if a and b have the same name
 *	    Number < 0 if a < b alphabetically
 *	    Number > 0 if a > b alphabetically
 */
int dpi_symbol_cmp(a, b)
Symbol *a;
Symbol *b;
{
     return( strcmp( (*a)->name->identifier, (*b)->name->identifier) );
}

/*
 * NAME: dpi_func_symbol_cmp
 *
 * FUNCTION: Compares two function table symbol names
 *
 * PARAMETERS:
 *	a	- First function table symbol to compare
 *	b	- Second function table symbol to compare
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: 0 if a and b have the same name
 *	    Number < 0 if a < b alphabetically
 *	    Number > 0 if a > b alphabetically
 */
int dpi_func_symbol_cmp(a, b)
Symbol *a;
Symbol *b;
{
    if (*a == *b)
	/*
	 * Identical symbols, return 0
	 */
	return 0;
    else if ((*a)->name == (*b)->name)
    {
	/*
	 * Identical names:
	 *	See if scope info is the same, if not return compare result
	 *	otherwise see if parameter info is the same and return result
	 */
	char	*p1, *p2;
	int	l1 = 0, l2 = 0;
	int	ret;
                    
	dpi_var_scope(*a, &p1, &l1);
	dpi_var_scope(*b, &p2, &l2);
	ret = strcmp(p1, p2);
	free(p1);
	free(p2);

	return ret ? ret : strcmp(symname(*a), symname(*b));
    }
    else
	/*
	 * Completely different, return comparison of names.
	 */
	return( strcmp( ident((*a)->name), ident((*b)->name)) );
}

                    
/*
 * NAME: dpi_var_scope
 *
 * FUNCTION: Retrieves the scoping information from a dbx Symbol
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES:
 *	1) Recursively calls itself walking up to the top block.
 *	2) Allocates memory to store the scope information
 *	3) Returns a string ending with a '.'
 *
 * PARAMETERS:
 *	sym	- Variable to get the scope
 *	scope	- Scope of the variable will be stored here
 *	length	- Should be initialized to 0 by caller; Will contain length of
 *		  string in scope
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * TESTING:
 *	1) Test correct scope information
 *
 * RETURNS: NONE
 *	scope - is filled in with scope information
 */
void dpi_var_scope(sym, scope, length)
Symbol sym;
char **scope;
int	*length;
{
	if ( sym->block ) {
		/*
		 * Set length to the current length of the scope information we
		 * have seen so far; Adding one for the '.' to put on the end
		 */
		*length += strlen( ident(sym->name) ) + 1;
		/* Recursively call next block */
		dpi_var_scope( sym->block, scope, length );
		strcat( *scope, ident(sym->name) );
		strcat( *scope, ".");
	} else {
		/*
		 * Set length to the current length of the scope information we
		 * have seen so far; Adding one for the '.' and one for NULL
		 * will be added on the malloc, so length will indicate the
		 * length of the scope when we return
		 */
		*length += strlen( ident(sym->name) ) + 1;
		/* Are at the top, return name */
		*scope = malloc( (*length + 1) * sizeof(char) );
		sprintf( *scope, ".%s", ident(sym->name) );
	}
}


/*
 * NAME: dpi_get_scope
 *
 * FUNCTION: Returns scope information for hidden variables
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES:
 *	1) Starting at the last item in parameter symarr, checks if the previous
 *	   item has the same name
 *	2a) If a match is found, and both items have the same symbol pointer,
 *	    then they are actually the same item.  The extra one is set to NULL
 *	    and cleared from the symarr.
 *	2b) If a match is found, and the items do not have the same symbol
 *	    pointer, then they are in different scopes.
 *		a) Use dpi_var_scope() to get the scope information for both
 *		   items.
 *		b) Set the corresponding element in scope_arr[] to the scope
 *		   information, for both matching items.
 *
 * PARAMETERS:
 *	num_items	- Number of Symbols in symarr
 *	symarr		- Sorted array of Symbols
 *	scope_arr	- Any items with the same name in symarr will have their
 *			  scope filled in the corresponding element of this
 *			  array
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * TESTING:
 *	1) Generation of scope information in variable list.
 *
 * RETURNS: NONE
 *	scope_arr	- Any scoping information necessary is returned here
 */
void dpi_get_scope(num_items, symarr, scope_arr)
int num_items;
Symbol *symarr;
char **scope_arr;
{
    int		index, match_index, dot, length;

    for( index = num_items - 1; index > 0; index-- ) {
	if( (*(symarr + index))->name == (*(symarr + index - 1))->name )
	    if( symarr[index-1] == symarr[index] ) {
		/*
		 * Have the same symbol listed twice, remove the extra one
		 */
		symarr[index] = NULL;
		scope_arr[index-1] = scope_arr[index];
		scope_arr[index] = NULL;
	    } else {
		/*
		 * Have two symbols with same name, get their scope information
		 */
		match_index = index - 1;
		length = 0;
		dpi_var_scope( symarr[index], scope_arr + index, &length );
		/*
		 * Remove trailing '.'
		 */
		dot = length - 1;
		*(scope_arr[index] + dot) = '\0';

		length = 0;
		dpi_var_scope( symarr[match_index], scope_arr + match_index,
			       &length );
		/*
		 * Remove trailing '.'
		 */
		dot = length - 1;
		*(scope_arr[match_index] + dot) = '\0';
	    }
    }
}


/*
 * NAME: dpi_get_func_scope
 *
 * FUNCTION: Returns scope information for hidden or overloaded functions
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES:
 *	1) Starting at the last item in parameter symarr, checks if the previous
 *	   item has the same name
 *	2a) If a match is found, and both items have the same symbol pointer,
 *	    then they are actually the same item.  The extra one is set to NULL
 *	    and cleared from the symarr.
 *	2b) If a match is found, and the items do not have the same symbol
 *	    pointer, then they are in different scopes.
 *		a) Use dpi_var_scope() to get the scope information for both
 *		   items.
 *		b1) If the scope information is the same, use printdecl() to get
 *		    the overloaded name for both items.
 *		b2) Otherwise use the scope information for both items.
 *		c) Set the corresponding element in scope_arr[] to the specified
 *		   information, for both matching items.
 *
 * PARAMETERS:
 *	num_items	- Number of Symbols in symarr
 *	symarr		- Sorted array of Symbols
 *	scope_arr	- Any items with the same name in symarr will have their
 *			  scope filled in the corresponding element of this
 *			  array
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * TESTING:
 *	1) Generation of scope information in function list.
 *	2) Generation of overloaded information in function list.
 *
 * RETURNS: NONE
 *	scope_arr	- Any scoping information necessary is returned here
 */
void dpi_get_func_scope(num_items, symarr, scope_arr)
int num_items;
Symbol *symarr;
char **scope_arr;
{
    int		index, match_index, dot, length;
    char	*param_name;

    for( index = num_items - 1; index > 0; index-- ) {
	if( (*(symarr + index))->name == (*(symarr + index - 1))->name)
	    if( symarr[index-1] == symarr[index] ) {
		/*
		 * Have the same symbol listed twice, remove the extra one
		 * Copy the scope info from the one we deleted incase
		 * it is needed.
		 */
		symarr[index] = NULL;
		scope_arr[index-1] = scope_arr[index];
		scope_arr[index] = NULL;
	    } else {
		/*
		 * Have two symbols with same name, get their scope information
		 */
		match_index = index - 1;
		length = 0;
		dpi_var_scope( symarr[index], scope_arr + index, &length );
		/*
		 * Remove trailing '.'
		 */
		dot = length - 1;
		*(scope_arr[index] + dot) = '\0';

		length = 0;
		dpi_var_scope( symarr[match_index], scope_arr + match_index,
			       &length );
		/*
		 * Remove trailing '.'
		 */
		dot = length - 1;
		*(scope_arr[match_index] + dot) = '\0';

		if( strcmp( scope_arr[index], scope_arr[match_index] ) == 0 ) {
		    /*
		     * These two functions are in the same scope.  We need to
		     * use the parameter information to distinguish them.
		     *
		     * Free the previous obtained scope information
		     */
		    free( scope_arr[index] );
		    free( scope_arr[match_index] );

		    /*
		     * Get the parameter information
		     */
		    param_name = symname( symarr[index] );
		    scope_arr[index] = malloc((strlen(param_name) + 1) *
					      sizeof( char ));
		    strcpy( scope_arr[index], param_name );

		    param_name = symname( symarr[match_index] );
		    scope_arr[match_index] = malloc((strlen(param_name) + 1) *
						    sizeof( char ));
		    strcpy( scope_arr[match_index], param_name );
		}
	    }
    }
}


/*
 * NAME: adc_function_list
 *
 * FUNCTION: Returns all the functions accessible by the program
 *
 * NOTES: Used by ADC only
 *
 * PARAMETERS:
 *	funcs		- Functions will be returned here
 *	file_lookup	- Used to delimit the list of functions returned
 *	func_syms	- Array of symbols associated with functions returned
 *			  here - allocated memory needs to be freed by caller
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: -1 on failures; Otherwise returns the number of functions found in
 *	    the specified scope
 */
int adc_function_list(funcs, file_lookup, func_syms)
char **funcs;
#ifdef _NO_PROTO
int (*file_lookup) ();
#else
int (*file_lookup) ( char *, struct SUFLIST * );
#endif
Symbol **func_syms;
{
    int		length = 0;
    int 	numfuncs = 0, tmp_numfuncs = 0;
    Symbol    	*symarr = NULL;
    int 	index;
    jmp_buf     env;
    int         *svenv;
    char	*end_of_list, **scope_arr = NULL;

    svenv = envptr;
    envptr = env;
    if (setjmp( env )) {
        return -1;
    }

    get_functions_list(&symarr, &tmp_numfuncs, file_lookup);

    /*
     * Sort the Function list
     */
    qsort( symarr, tmp_numfuncs, sizeof( *symarr ), dpi_func_symbol_cmp );

    /*
     * Allocate enough space to store scoping information for functions
     */
    if( tmp_numfuncs > 0 ) {
	/*
	 * Uses calloc() to clear all entries since code below depends on this
	 */
	scope_arr = (char **) calloc( tmp_numfuncs, sizeof( char * ) );

	/*
	 * Need to remove any duplicate functions from the list, and get scope
	 * information for any functions with the same name.
	 */
	dpi_get_func_scope( tmp_numfuncs, symarr, scope_arr );
    }

    /*
     * Convert the sorted list of functions to a single
     * character string with newlines
     */
    for( index = 0; index < tmp_numfuncs; index++ ) {
	if( symarr[index] ) {
	    /* Add the length of the function plus 1 for newline */
	    length += strlen( symarr[index]->name->identifier ) + 1;
	    if( scope_arr[index] ) {
		/*
		 * Add the length of the scope information plus 3
		 * characters for the tab, '(' and ')'
		 */
		length += strlen( scope_arr[index] ) + 3;
	    }
	}
    }

    *func_syms = (Symbol *) malloc( tmp_numfuncs * sizeof( Symbol ) );

    *funcs = malloc( (length + 1) * sizeof( char ) );
    **funcs = '\0';
    end_of_list = *funcs;

    for( index = 0, numfuncs = 0; index < tmp_numfuncs; index++ ) {
	if( symarr[index] ) {
	    if( scope_arr[index] ) {
		end_of_list += sprintf( end_of_list, "%s\t(%s)\n",
			symarr[index]->name->identifier, scope_arr[index] );
	    } else {
		end_of_list += sprintf( end_of_list, "%s\n",
					symarr[index]->name->identifier );
	    }
	    (*func_syms)[numfuncs] = symarr[index];
	    /*
	     * numfuncs will be set to the actual number of functions found
	     */
	    numfuncs++;
	}
    }
    /* Clear trailing newline */
    if( end_of_list != *funcs ) {
	*(end_of_list - 1) = '\0';
    }

    /*
     * No longer need symarr or scope_arr; free the storage
     */
    free(symarr);
    for( index = 0; index < tmp_numfuncs; index++ ) {
	if( scope_arr[index] ) free( scope_arr[index] );
    }
    if( scope_arr ) free( scope_arr );

    envptr = svenv;
    return numfuncs;
}


/*
 * NAME: adc_variable_list
 *
 * FUNCTION: Returns all the dbx symbols within a given scope in an array of
 *	     VarStructs
 *
 * NOTES: Used by ADC only
 *
 * PARAMETERS:
 *	module		- File or Function name for current scope
 *	scope		- LOCAL_AND_NESTED, PROGRAM_EXTERNALS or
 *			  FILE_STATICS; Indicates what scope to use when
 *			  retrieving the variables
 *	vars		- Variables will be returned here
 *	file_lookup	- If scope is PROGRAM_EXTERNALS, this will be used to
 *			  delimit the variables retrieved
 *      all_list        - If scope == PROGRAM_EXTERNALS then list of all
 *                        externals will be returned here
 *      all_list_num    - If scope == PROGRAM_EXTERNALS then number in all_list
 *                        will be returned here
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: -1 if failure; Otherwise will return the number of variables found
 *	    in the specified scope
 */
int adc_variable_list(module, scope, vars, file_lookup, all_list, all_list_num)
char *module;
int scope;
VariableList ***vars;
#ifdef _NO_PROTO
int (*file_lookup)();
#else
int (*file_lookup)( char *, struct SUFLIST * );
#endif
VariableList ***all_list;
int *all_list_num;
{
    int 	index, numvars = 0;
    char	*filename;
    char	*fileptr, *sname;
    char	*period_ptr, **scope_arr = NULL;
    Name    	modulename;
    Symbol    	s;
    Symbol    	*symarr = NULL, *all_symarr = NULL;
    Symbol	modulesym;
    VariableList    **ptr;
    int        	adc_namecmp( );
    jmp_buf     env;
    int         *svenv;
    boolean	resort = false;	/* If scope == LOCAL_AND_NESTED, then this will
				 * be checked to determine if any hiddens were
				 * encountered requiring us to resort the list
				 */

    svenv = envptr;
    envptr = env;
    if (setjmp( env )) {
        return -1;
    }

   /* Setup modulesym for file statics and locals */
   switch (scope) {
	case LOCAL_AND_NESTED:
            if ( *module == '.' && !strcmp( module, "." ) ) {
		/* Don't get locals for the program level */
                envptr = svenv;
                return 0;
             }

	    /*
	     * Find symbol associated with function name
	     */
	    modulename = identname( module, false );

	    /*
	     * Use the current function as our scope, unless the name does not
	     * match modulename.  This will happen at __exit, when curfunc still
	     * points to main(), but we don't want to get local variables for
	     * main()
	     */
	    modulesym = curfunc;
	    if( modulesym->name != modulename ) {
		modulesym = lookup( modulename );
		while( modulesym != nil &&
		       !( modulesym->name == modulename &&
			  isroutine( modulesym ) )) {
		    modulesym = modulesym->next_sym;
		}
	    }
    	    if ( !modulesym ) {	/* module was not found in dbx's symbol table */
               envptr = svenv;
               return -1;
            }
	    break;
	case PROGRAM_EXTERNALS:
	    break;
	case FILE_STATICS:
	    if ( !cursource ) {
                envptr = svenv;
                return 0;
            }
	    /* Get the basename of the file */
	    if ( !( fileptr = strrchr( cursource, '/' ) ) )
	        fileptr = cursource;
	    else
	        fileptr++;
	    /* If we have been called with the -u option,
	     * add the '@' to the name
	     */
	    if (unique_fns) {
		    filename = malloc(( strlen(fileptr) + 2 ) * sizeof(char));
		    *filename = '@';
		    strcpy(filename + 1, fileptr );
	    } else {
		    filename = malloc(( strlen(fileptr) + 1 ) * sizeof(char));
		    strcpy( filename, fileptr );
	    }
	    /* Remove the extension from the filename */
	    if ( period_ptr = strrchr( filename, '.' ) )
	        *period_ptr = '\0';
   	    /*  Find symbol associated with filename */
            modulename = identname( filename, false );
	    modulesym = lookup( modulename );
	    while( modulesym != nil &&
		   !( modulesym->name == modulename &&
		      modulesym->class == MODULE )) {
		modulesym = modulesym->next_sym;
	    }
	    dispose( filename );
    	    if ( !modulesym ) {	/* module was not found in dbx's symbol table */
               envptr = svenv;
               return -1;
            }
	    break;
	default:	/* Not a recognized scope, do nothing */
            envptr = svenv;
            return 0;
    }

    /*   calculate number of arguments and variables   */

    switch (scope) {
	case PROGRAM_EXTERNALS:
            get_externs_list(&symarr, &numvars, file_lookup, &all_symarr,
	                     all_list_num);
	    break;
        case FILE_STATICS:
	    get_file_static_list(modulesym, &symarr, &numvars);
	    break;
        case LOCAL_AND_NESTED:
	    get_all_local (modulesym, &symarr, &numvars);
	    break;	
    }

    /* create the variable list */

    *vars = (VariableList **) malloc((numvars + 1) * sizeof( VariableList * ));

    if( scope == LOCAL_AND_NESTED ) {
	/*
	 * Sort current list for dpi_get_scope()
	 */
	qsort(symarr, numvars, sizeof( *symarr ), dpi_symbol_cmp );

	/*
	 * Allocate enough space to store scoping information for variables
	 */
	if( numvars > 0 ) {
	    /*
	     * Use calloc() here since this will initialize all entries to
	     * NULL and we count on that below
	     */
	    scope_arr = (char **) calloc( numvars, sizeof( char * ) );

	    /*
	     * Need to remove any duplicate variables from the list,
	     * and get scope information for any hidden variables
	     */
	    dpi_get_scope( numvars, symarr, scope_arr );
	}

	for ( index = 0, ptr = *vars; index < numvars; ptr++, index++ ) {
	  *ptr = ( VariableList * ) malloc( sizeof( VariableList ));
	  s = symarr[index];
	  (*ptr)->key = s;
	  if( s ) {
	    sname = symname( s );
	    if( scope_arr[index] ) {
		resort = true;
		/*
		 * Add the length of the scope information plus 4
		 * characters for the tab, '(' , ')', and NULL
		 */
                (*ptr)->name = malloc( ( strlen( sname ) +
                                         strlen( scope_arr[index] ) + 4 ) *
                                       sizeof( char ) );
                sprintf( (*ptr)->name, "%s\t(%s)", sname, scope_arr[index] );
            } else {
                (*ptr)->name = malloc((strlen(sname) + 1) * sizeof(char));
                strcpy( (*ptr)->name, sname);
	    }
	  }
	}
	*ptr = ( VariableList * )nil;

	/*
	 * No longer need scope_arr, free the storage
	 */
	for( index = 0; index < numvars; index++ )
	    if( scope_arr[index] ) free( scope_arr[index] );
	if( scope_arr ) free( scope_arr );
    } else {	/* Not LOCAL_AND_NESTED */
	for ( index = 0, ptr = *vars; index < numvars; ptr++, index++ ) {
	    *ptr = ( VariableList * ) malloc( sizeof( VariableList ));
	    s = symarr[index];
	    (*ptr)->key = s;
            sname = symname(s);
            (*ptr)->name = malloc((strlen(sname) + 1) * sizeof(char));
            strcpy( (*ptr)->name, sname);
	}
	*ptr = ( VariableList * )nil;
        if( scope == PROGRAM_EXTERNALS ) {
            *all_list = (VariableList **) malloc((*all_list_num + 1) *
                                                 sizeof( VariableList * ));
            for( index = 0, ptr = *all_list; index < *all_list_num;
                 ptr++, index++ ) {
                *ptr = ( VariableList * ) malloc( sizeof( VariableList ));
                s = all_symarr[index];
                (*ptr)->key = s;
                sname = symname(s);
                (*ptr)->name = malloc((strlen(sname) + 1) * sizeof(char));
                strcpy( (*ptr)->name, sname );
            }
            *ptr = (VariableList *)nil;
            free( all_symarr );
            qsort( (char *)*all_list, *all_list_num, sizeof( **all_list ),
                   adc_namecmp );
        }
    }

    /* No longer need symarr, free the storage */
    free(symarr);

    /*
     * Always sort lists except for LOCAL_AND_NESTED list.  LOCAL_AND_NESTED
     * list was already sorted and only needs to be sorted again if the resort
     * flag is set
     */
    if( scope != LOCAL_AND_NESTED || resort ) {
	/*   sort variables alphabetically   */
	qsort(( char * )*vars, numvars, sizeof( **vars ), adc_namecmp );
    }

    envptr = svenv;
    return numvars;
}


/*
 * NAME: var_lang_init
 *
 * FUNCTION: Sets the language variables for the given function or module symbol
 *
 * PARAMETERS:
 *	func	- Function or module to use to set language variables
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: The following external variables are set here:
 *	var_lang
 *	beg_delim
 *      mid_delim
 *	end_delim
 *	varDecl
 *	changeName
 *	reverseSubscripts
 *
 * RETURNS: NONE
 */
void var_lang_init( func )
Symbol	func;
{
    if ( func->language == fLang ) {
	var_lang = FORTRAN_LANG;
	beg_delim = "(";
        mid_delim = ",";
        end_delim = ")";
        varDecl = fortran_varDecl;
	changeName = fortran_changeName;
	reverseSubscripts = true;
    } else if ( func->language == cLang ) {
	var_lang = C_LANG;
	beg_delim = "[";
        mid_delim = "][";
        end_delim = "]";
        varDecl = c_varDecl;
	changeName = c_changeName;
	reverseSubscripts = false;
    } else {
	var_lang = UNSUPPORTED_LANG;
    }
}


/*
 * NAME: adc_var_lang_init
 *
 * FUNCTION: Sets the language variables for the given function or module symbol
 *
 * NOTES: Used by ADC only
 *
 * PARAMETERS:
 *	s	- Symbol to use to set the language variables
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: The following external variables are set:
 *	beg_delim
 *	mid_delim
 *	end_delim
 *	varDecl
 *	changeName
 *	reverseSubscripts
 *
 * RETURNS: NONE
 */
void adc_var_lang_init( s )
Symbol	s;
{
    if ( s->language == cLang ) {
	beg_delim = "[";
        mid_delim = "][";
        end_delim = "]";
        varDecl = c_varDecl;
	changeName = c_changeName;
	reverseSubscripts = false;
    } else if ( s->language == cppLang ) {
	beg_delim = "[";
        mid_delim = "][";
        end_delim = "]";
        varDecl = cpp_varDecl;
	changeName = c_changeName;
	reverseSubscripts = false;
    } else if ( s->language == cobLang ) {
        beg_delim = "(";
        mid_delim = ",";
        end_delim = ")";
        varDecl = cobol_varDecl;
        changeName = cobol_changeName;
        reverseSubscripts = false;
    } else if ( s->language == fLang ) {
	beg_delim = "(";
        mid_delim = ",";
        end_delim = ")";
        varDecl = fortran_varDecl;
	changeName = fortran_changeName;
	reverseSubscripts = true;
    }
}


/*
 * NAME: dpi_expand_variable
 *
 * FUNCTION: Creates the underlying VarStruct for either a tag or pointer
 *	VarStruct.
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES: Creates the underlying VarStruct for either a tag or pointer
 *	VarStruct.  In the case of a tag VarStruct this VarStruct would be
 *	either a union, record ( structure ), or enum.  For a pointer VarStruct,
 *	the underlying VarStruct, would represent whatever variable type the
 *	pointer pointed to.
 *
 * PARAMETERS:
 *	var	- Variable to create VarStruct from
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: 
 *  -1:	failure
 *   0:	success
 */
int dpi_expand_variable(var)
struct VarStruct *var;
{
    struct VarStruct    *expvar;
    struct VarStruct    **endchain;
    Symbol 	t, varkey;

    /* Check for invalid variable type */

    if ( var->class != V_TAG && var->class != V_PTR )
        return -1;

    /* Has the variable already been expanded? */

    else if ( var->chain != nil )
        return 0;
    varkey = ( Symbol )var->key;

    /*  create VarStruct pointer array of one and end with nil pointer */

    var->chain =
         ( struct VarStruct ** )calloc( 2, sizeof( struct VarStruct * ) );
    *var->chain = ( struct VarStruct * )malloc( sizeof( struct VarStruct ) );
    endchain = var->chain;
    *++endchain = nil;

    /* Initialize the chainptr structure (expanded variable) */

    expvar = *var->chain;
    expvar->scope = var->scope;
    expvar->symbol = var->symbol;
    if ( var->class == V_TAG )
	expvar->key = varkey->type;
    else if ( var->class == V_PTR ) {
	if ( varkey->type->class == TAG ) {

	    /*  must handle dbx symbol TAG->TAG idiosyncracy */

	    for ( t = varkey->type; t->class == TAG; t = t->type )
		expvar->key = t;

	    /*   if also untagged structure set key to RECORD/UNION   */

	    t = ( Symbol )expvar->key;
	    if ( nilname( expvar->key ) )
	        expvar->key = ( ( Symbol )( expvar->key ) )->type;
	    t = ( Symbol )expvar->key;
	} else
	    expvar->key = varkey->type;
    }
    expvar->class =  varClass((Symbol )( expvar->key ));
    expvar->parent = var;
    expvar->implode = nil;
    expvar->format = NOFORMAT;
    expvar->inline = false;
    expvar->chain = nil;
    expvar->range = nil;

    /* Construct the name and basename for the chainptr structure */

    if ( var->class == V_TAG ) {
        expvar->name = malloc(( strlen( var->name ) + 1 ) * sizeof(char));
        strcpy( expvar->name, var->name );
        expvar->basename = malloc(( strlen(var->basename) + 1) * sizeof(char));
        strcpy( expvar->basename, var->basename );
    } else if ( var->class == V_PTR ) {
        expvar->name = malloc(( strlen( var->name ) + 2 ) * sizeof(char));
        strcpy( expvar->name, "*" );
        strcat( expvar->name, var->name );
        expvar->basename = malloc(( strlen(var->basename) + 2) * sizeof(char));
        strcpy( expvar->basename, "*" );
        strcat( expvar->basename, var->basename );
    }

    /* Construct the declaration for the chainptr structure */

    if ( (*varDecl)( var->key, expvar->key, expvar ) )
	return -1;

    /* Initialize decl index and decl lines for all variable types */    

    declindex = 0;
    if ( expvar->class == V_RECORD || expvar->class == V_UNION ) {
        declindex = 1;
        /* Increment the index into the declaration to
           handle fortran derived types that have a sequence line
           (i.e. class of the symbol = PACKRECORD)
        */
        if ((expvar->key->language == fLang) &&
            (expvar->key->class == PACKRECORD))
        {
           declindex++;
        }
        expvar->implode = expvar;

	/*  create VarStructs for structure fields  */

        if ( var_chain( expvar ) == -1 )
	    return -1;
        expvar->index = declindex++;
	expvar->decl_lines = expvar->index;
    } else if( expvar->class == V_CLASS ) {
        declindex = 1;
        expvar->implode = expvar;

	/*
	 * Create VarStructs for class fields
	 */
        if ( var_class( expvar ) == -1 )
	    return -1;
        expvar->index = declindex++;
	expvar->decl_lines = expvar->index;
    } else if ( expvar->class == V_SCAL ) {
        declindex = 0;
        expvar->chain = nil;
        expvar->index = declindex++;
	expvar->decl_lines = expvar->index;
    } else if ( expvar->class == V_ARRAY ) {

	/*  set V_ARRAY fields and create VarStruct for array element  */

        if ( var_arrayChain( var->key, &expvar ) )
	    return -1;
    } else if ( var->class == V_PTR ) {
        expvar->chain = nil;
        if ( expvar->class == V_TAG )
            expvar->index = 0;		/*  new window (display) */
        else {
            expvar->index = declLines( expvar->decl ) - 1;
	}
	expvar->decl_lines = expvar->index;
    }
    if ( ( var->class == V_TAG )  || ( var->parent && 
        ( var->parent->class == V_RECORD ||
	  var->parent->class == V_UNION || var->parent->class == V_CLASS ) ) ) {
        expvar->implode = var->implode ? var->implode : var;
    } else {
        expvar->implode = var;
    }
    return 0;
}


/*
 *  this routine peforms the following actions, such that the tagged structure's
 *  declaration can be folded inline within the current declaration:
 *  op == DECLARATION_RECORD:
 *     resets the decl_indices for siblings below the tagged VarStruct, and
 *     their children.
 *  op == INLINED_RECORD:
 *     resets the decl_indices for the tagged VarStruct and its children.
 */

static void  add_inlined_index( var, tag_decl_index, index_to_add, op )
struct VarStruct *var;	/* variable with which to start exploring */
int tag_decl_index;	/* index after which additions begin */
int index_to_add;	/* index with which to add onto each decl_index */
int op;		/* addition operation: INLINED_RECORD, DECLARATION_RECORD */
{

   struct VarStruct   **chainptr;

   if ( op == DECLARATION_RECORD ) {
       if ( var->index > tag_decl_index ) 
           var->index += index_to_add;
   }
   else  /* op == INLINED_RECORD */
       var->index += index_to_add;
   var->decl_lines = var->index;

   /*-------------------------------------------------------------------+
   | Walk down the variable's chain, recursively calling the routine.   |
   | Don't walk down variables of debugger class TAG or PTR, since that | 
   | involves another declaration with a different set of indices.      |
   +-------------------------------------------------------------------*/
   if (( var -> class != V_TAG ) && ( var -> class != V_PTR )) {
        for ( chainptr = var -> chain; *chainptr; chainptr++ )
             add_inlined_index( *chainptr, tag_decl_index, index_to_add, op );
   }
}


/*
 *  this routine peforms the following actions, such that the tagged structure's
 *  declaration can be unfolded inline within the current declaration:
 *  op == DECLARATION_RECORD:
 *     resets the decl_indices for siblings below the tagged VarStruct, and
 *     their children.
 *  op == INLINED_RECORD:
 *     resets the decl_indices for the tagged VarStruct and its children.
 */

static void  subtract_inlined_index( var, tag_decl_index,
				     index_to_subtract, op )
struct VarStruct *var;	/* variable with which to start exploring */
int tag_decl_index;	/* index after which subtractions begin */
int index_to_subtract;	/* index with which to subtract from each decl_index */
int op;		/* subtraction operation: INLINED_RECORD, DECLARATION_RECORD */
{

   struct VarStruct   **chainptr;

   if ( op == DECLARATION_RECORD ) {
       if ( var->index > tag_decl_index ) 
           var->index -= index_to_subtract;
   }
   else  /* op == INLINED_RECORD */
       var->index -= index_to_subtract;
   var->decl_lines = var->index;
   
   /*-------------------------------------------------------------------+
   | Walk down the variable's chain, recursively calling the routine.   |
   | Don't walk down variables of debugger class TAG or PTR, since that | 
   | involves another declaration with a different set of indices.      |
   +-------------------------------------------------------------------*/
   if (( var -> class != V_TAG ) && ( var -> class != V_PTR )) {
        for ( chainptr = var -> chain; *chainptr; chainptr++ )
             subtract_inlined_index( *chainptr, tag_decl_index,
				     index_to_subtract, op );
   }
}


/*
 *  this routine peforms the following actions, such that the tagged structure's
 *  declaration can be folded inline within the current declaration:
 *      1) resets the decl_indices for the tagged VarStruct and its children.
 *      2) resets the decl_indices for siblings below the tagged VarStruct, and
 *	   their children.
 *      3) marks the current VarStruct -> inline = true.
 *      4) resets the tagged VarStruct -> class to V_INLINE.
 */
void  dpi_expand_variable_inline(decl_var, record_var)
struct VarStruct *decl_var;  /* the declaration where inlining occurs */
struct VarStruct *record_var; /* the declaration to inline */
{
    struct VarStruct    *tag_var;

    tag_var = record_var -> parent;
    /*------------------------------------------------------+
    | Add the inlined variable's decl lines to the children |
    | of decl_var below the tagged structure's decl_index.  |
    +------------------------------------------------------*/
    add_inlined_index( decl_var, tag_var->index, record_var->index,
		       DECLARATION_RECORD );    

    /*---------------------------------------------------------------------+
    | Add the tagged structure's decl_index to the children of record_var. |
    +---------------------------------------------------------------------*/
    add_inlined_index( record_var, 0, tag_var->index, INLINED_RECORD );

    /*---------------------------------------------------------------------+
    | Set var->inline to True and reset class type from V_TAG to V_INLINE. |
    +---------------------------------------------------------------------*/
    decl_var -> inline = true;
    tag_var -> class = V_INLINE;
}


/*  Recursively searches the variable's chain to determine 
 *  if there exists any VarStructs whose class == V_INLINE.
 */

void  dpi_find_inlined_variables( var, var_to_inline )
struct VarStruct *var;	/* variable with which to start exploring */
struct VarStruct **var_to_inline;	/* return status: variable whose
					   class is INLINE if found */
{

    struct VarStruct      **chainptr;

   /*-------------------------------------------------+
   | Search for variables of debugger class V_INLINE. |
   +-------------------------------------------------*/
   if ( var -> class == V_INLINE )
       *var_to_inline = *( var -> chain );
    
   /*-------------------------------------------------------------------+
   | Walk down the variable's chain, recursively calling the routine.   |
   | Don't walk down variables of debugger class TAG or PTR, since that | 
   | involves another declaration with a different set of indices.      |
   +-------------------------------------------------------------------*/
   else if (( var -> class != V_TAG ) && ( var -> class != V_PTR )) {
        for ( chainptr = var -> chain; *chainptr; chainptr++ ) 
             dpi_find_inlined_variables( *chainptr, var_to_inline );
   }
}


/*
 *  this routine peforms the following actions, such that the tagged structure's
 *  declaration can be unfolded inline within the current declaration:
 *      1) resets the tagged VarStruct -> class to V_TAG.
 *      2) resets the decl_indices for the tagged VarStruct and its children.
 *      3) resets the decl_indices for siblings below the tagged VarStruct, and
 *	   their children.
 *      4) if no other VarStructs are folded inline, marks the current
 *	   VarStruct -> inline = false.
 */
void  dpi_unexpand_variable_inline(decl_var, record_var)
struct VarStruct *decl_var;  /* the decl where unexpansion will occur */
struct VarStruct *record_var; /* the inlined declaration to unexpand */
{
    struct VarStruct    *tag_var;
    struct VarStruct    *var_to_inline=NULL;

    /*-----------------------------------------+
    | Reset class type from V_INLINE to V_TAG. |
    +-----------------------------------------*/
    tag_var = record_var -> parent;
    tag_var -> class = V_TAG;
	
    /*-------------------------------------------------------------+
    | Subtract the tagged structure's decl_index from the children |
    | of record_var.						   |
    +-------------------------------------------------------------*/
    subtract_inlined_index( record_var, 0, tag_var->index, INLINED_RECORD );

    /*-------------------------------------------------------------+
    | Subtract the inlined variable's decl lines from the children |
    | of decl_var below the tagged structure's decl_index.         |
    +-------------------------------------------------------------*/
    subtract_inlined_index( decl_var, tag_var->index, record_var->index,
			    DECLARATION_RECORD );    

    /*------------------------------------------------------+
    | Check for the existance of further inlined variables. |
    | If none are found, set var->inline = false.           |
    +------------------------------------------------------*/
    dpi_find_inlined_variables( decl_var, &var_to_inline );
    if ( !var_to_inline )
  	decl_var -> inline = false;
}


/*
 *  this routine is used to expand a record or union VarStruct.
 *  It creates a VarStruct array and fills it with the elements
 *  of the record or union by calling var_element.
 */
int        var_chain( var )
struct VarStruct    *var;
{
    Symbol    t;
    int        num;
    struct VarStruct    **fields;

    t = ( Symbol )var->key;

    /*  get number of fields  */

    num = 0;
    while ( t = t->chain )
        num++;

    /*  create VarStruct pointer array of num + 1 */

    var->chain =
         ( struct VarStruct ** )calloc( num + 1, sizeof( struct VarStruct * ));

    /*  create VarStruct for each FIELD Symbol  */

    fields = var->chain;
    t = ( Symbol )var->key;
    while ( t = ( Symbol )t->chain ) {
        *fields = ( struct VarStruct * )malloc( sizeof( struct VarStruct ) );
        if ( var_element( t, t->type, var, false, *fields, UNKNOWN_LIST ) )
	    return -1;
        fields++;
    }
    *fields = ( struct VarStruct * )nil;
    return num;
}


typedef struct {
    Symbol	anon_union_class;
    struct VarStruct    **field;
} anon_union_tree;

/*
 * NAME: is_anon_union
 *
 * FUNCTION: Return true if the symbol is an anonymous union
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES: 
 *
 * PARAMETERS:
 *	t		Symbol to check
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * TESTING:
 *
 * RETURNS: true/false
 */
static	boolean	is_anon_union(t)
Symbol	t;
{
    boolean	ret;

/*
     * If the class is nested class, the name is a temporary,
     * the type is a class and a union, and 
     * and the next symbol on the chain shares
     * the type and is not a temporary name, then this is an anonymous
     * union.  If it is a named union, then the next_sym would
     * be the name of the union.
     */
    return (t->class == NESTEDCLASS && cpp_tempname(t->name) &&
	    rtype(t->type)->class == CLASS &&
	    rtype(t->type)->symvalue.class.key == 'u' &&
	     !(t->next_sym != nil && !cpp_tempname(t->next_sym->name)
	      && rtype(t->next_sym->type) == rtype(t->type)));
}

/*
 * NAME: reset_index_under_union
 *
 * FUNCTION: Reset the indexes of all fields after an anonymous union is met
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES: 
 *
 * PARAMETERS:
 *	anon_union_member	- Anonynmous Union symbol structure
 *	fields			- First field following anonymous union
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * TESTING:
 *
 * RETURNS: Number of members in anonymous union
 */
int reset_index_under_union( anon_union_member, fields )
Symbol	anon_union_member;
struct VarStruct	**fields;
{
    int	num_in_current_nested = 0;
    int	num_in_nested = 0;
    int	num_to_add = 1;		/* Indicates number of lines to add to all items
				 * occurring after the union; this may include
				 * public:, etc.
				 */
    unsigned	section;	/* Indicates which section we are in of class */
    struct VarStruct	**current_field = fields;

    while( *current_field != NULL ) {
	/*
	 * Add one for beginning union line for field and all of its children
	 */
	add_inlined_index( *current_field, 0, 1, INLINED_RECORD );
	current_field++;
    }

    /*
     * Walk down anonymous union looking for the end
     */
    current_field = fields;
    section = anon_union_member->symvalue.member.access;
    while( anon_union_member != NULL && *current_field != NULL ) {
	if( anon_union_member->class == NESTEDCLASS ) {
	    if(is_anon_union(anon_union_member))
	    {
		/*
		 * This is also an anonymous union, reset the indexes for this
		 */
		num_in_nested += reset_index_under_union(
			(rtype(anon_union_member->type))->chain, current_field);
		/*
		 * Skip over members of anonymous union
		 */
		while( num_in_nested > 0 ) {
		    current_field++;
		    num_in_nested--;
		}
	    }
	    /*
	     * The older (pre cset) c++ compilers had an extra entry for
	     * anonymous unions that we need to skip if it is there.
	     */
	    if (anon_union_member->name == anon_union_member->next_sym->name)
		anon_union_member = anon_union_member->next_sym;
	} else {
	    current_field++;
	    num_in_current_nested++;
	}
	anon_union_member = anon_union_member->next_sym;

	/*
	 * If the access is different, and it is not a friend access need to
	 * account for extra line added for access information; friend
	 * information would go on the same line
	 */
	if( anon_union_member != NULL &&
	    section != anon_union_member->symvalue.member.access &&
	    anon_union_member->symvalue.member.access != 0 ) {
	    section = anon_union_member->symvalue.member.access;
	    num_to_add++;
	}
    }

    while( *current_field != NULL ) {
	/*
	 * Add one for end union line and any section names;
	 * for field and all of its children
	 */
	add_inlined_index( *current_field, 0, num_to_add, INLINED_RECORD );
	current_field++;
    }
    /*
     * Add for beginning and end of union, and any public: etc. items
     */
    declindex += num_to_add + 1;
    return num_in_current_nested;
}


/*
 * NAME: count_nestedclass_members
 *
 * FUNCTION: Counts number of lines for a definition of a nested class
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES: 
 *
 * PARAMETERS:
 *	nested_class	- Symbol for nested_class
 *	anon_union	- Set to true if nested_class is actually an anonymous
 *			  union
 *	next_sym	- Symbol following this one in containing class
 *	nested		- True if this is a member of a nested class
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: Number of lines for nested class
 */
int count_nestedclass_members( nested_class, anon_union, next_sym, nested )
Symbol	nested_class;
Boolean	*anon_union;
Symbol	next_sym;
Boolean nested;
{
    int		num = 2;	/* Initialized for first and last line */
    Symbol	tmp = nested_class->type;
    unsigned	section;	/* Indicates which section we are in of class */

    while( tmp->class == TAG ) tmp = tmp->type;
    if( tmp->class == CLASS ) {
	/*
	 * Setup section
	 */
	switch( tmp->symvalue.class.key ) {
	    case 'c':
		section = PRIVATE;
		break;
	    default:
		section = PUBLIC;
		break;
	    case 's':
		/*
		 * A Structure must exist in the class, the next item will
		 * be the structure
		 */
		return 0;
		break;
	    case 'u':
		if( nested == false || nested_class->isAnonMember ||
		    nested_class->name == next_sym->name ||
		    is_anon_union(nested_class))
	       {
		    /*
		     * This is an anonymous union, we won't count any of the
		     * lines.  The members are chained off the containing class
		     */
		    *anon_union = true;
		    return 0;
		} else {
		    /*
		     * Just count the first line of the union, the last line
		     * will have its own symbol
		     */
		    num = 1;
		    section = PUBLIC;
		}
		break;
	}
	tmp = tmp->chain;
	while( tmp != NULL ) {
	    /*
	     * Don't count compiler generated symbols or enums
	     */
	    if( !tmp->symvalue.member.isCompGen && tmp->class != CONST ) {
		if( tmp->symvalue.member.access != section &&
		    tmp->symvalue.member.access != 0 ) {
		    /*
		     * The section has changed, increment the count to account
		     * for the public:\n, etc that will be displayed
		     * If the access is friend we won't come here since that
		     * information is placed on the same line
		     */
		    section = tmp->symvalue.member.access;
		    num++;
		}
		if( tmp->class == NESTEDCLASS ) {
		    num += count_nestedclass_members( tmp, anon_union,
						      tmp->next_sym, true );
		    if( *anon_union == true ) {
			*anon_union = false;
			num += 2;
		    }
		} else {
		    num++;
		}
	    }
	    tmp = tmp->next_sym;
	}
	return num;
    } else {
	/*
	 * If this is not a CLASS then it will only take one line
	 */
	return 1;
    }
}


/*
 * NAME: var_class
 *
 * FUNCTION: Used to expand a CLASS VarStruct.  It creates a VarStruct array and
 *	fills it with the elements of the class by calling var_element.
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES: 
 *
 * PARAMETERS:
 *	var	- Top level CLASS
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES:
 *	declindex	- Modified to take into account special features of
 *			  class declaration
 *
 * RETURNS: Number of fields found
 */
int        var_class( var )
struct VarStruct    *var;
{
    Symbol    t;
    int        num = 0, index;
    struct VarStruct    **fields;
    unsigned	orig_section, section;
    Boolean	anon_union = false;
    Boolean	derived = false;
    anon_union_tree	*nested_unions = NULL;
    int			size_nested_unions = 0;
    int			num_nested_unions = 0;

    /*
     * Get number of fields
     * First count the number of derived and virtual classes
     */
    t = var->key->type;
    while( t && t->class == BASECLASS ) {
	num++;
	t = t->chain;
    }

    /*
     * Count the number of members of class
     */
    t = var->key->chain;
    while( t != NULL ) {
	if( !t->symvalue.member.isCompGen && t->class != CONST ) {
	    num++;
	}
	t = t->next_sym;
    }

    /*
     * Create VarStruct pointer array of num + 1; Last one will be NULL
     */
    var->chain =
         (struct VarStruct **)malloc( (num + 1) * sizeof(struct VarStruct *));

    /*
     * Create VarStruct for each FIELD Symbol
     */
    fields = var->chain;

    /*
     * Check what storage access is the default for this class.
     * Save this information in orig_section in case this class has derived or
     * virtual classes
     */
    switch( var->key->symvalue.class.key ) {
	case 'c':
	    orig_section = section = PRIVATE;
	    break;
	case 'u':
	case 's':
	default:
	    orig_section = section = PUBLIC;
	    break;
    }

    /*
     * First create the derived or virtual class VarStructs
     */
    t = var->key->type;
    while( t && t->class == BASECLASS ) {
	if( derived != true ) {
	    /*
	     * Account for "Parents:" that was inserted in declaration
	     */
	    derived = true;
	    declindex++;
	}
	*fields = (struct VarStruct *)malloc(sizeof(struct VarStruct));
	if(var_element(t, t->type, var, false, *fields, UNKNOWN_LIST))
	    return -1;
	fields++;
	t = t->chain;
    }

    /*
     * Create VarStructs for members
     */
    t = var->key->chain;

    while( t != NULL ) {
	/*
	 * Don't create a VarStruct for compiler generated symbols or
	 * enumerated constants promoted from nested enumerators
	 */
	if( !t->symvalue.member.isCompGen && t->class != CONST ) {
	    if( derived && t->symvalue.member.access == orig_section ) {
		/*
		 * Account for the access line added to declaration
		 */
		declindex++;
	    }
            /* After first member of class is processed, 
               no need to check if member's access matches default
               since index will get incremented properly due  
               to section changing. 
            */
	    derived = false;

	    if( t->symvalue.member.access != section &&
		t->symvalue.member.access != 0 ) {
		/*
		 * The section has changed, increment the count to account for
		 * the public:\n, etc that will be displayed.  If the access is
		 * friend we won't come here since that information is placed on
		 * the same line
		 */
		section = t->symvalue.member.access;
		declindex++;
	    }
	    if( t->class == NESTEDCLASS ) {
		if( !t->isAnonMember || !( cpp_tempname(t->name) &&
		    rtype(t->type)->class == CLASS &&
		    (rtype(t->type)->symvalue.class.key != 'u' ||
		     (t->next_sym != nil && !cpp_tempname(t->next_sym->name) &&
		     rtype(t->next_sym->type) == rtype(t->type)))))
		{
		    declindex += count_nestedclass_members( t, &anon_union,
							    t->next_sym, false);
		    if( anon_union == true && is_anon_union(t) &&
		       !t->isAnonMember)
		    {
			if( num_nested_unions >= size_nested_unions ) {
			    if( size_nested_unions == 0 ) {
				size_nested_unions = 5;
				nested_unions =
				(anon_union_tree *)malloc( size_nested_unions *
						sizeof( anon_union_tree ) );
			    } else {
				size_nested_unions += 5;
				nested_unions =
				(anon_union_tree *)realloc( nested_unions,
						size_nested_unions *
						sizeof( anon_union_tree ) );
			    }
			}
			nested_unions[num_nested_unions].anon_union_class =
								rtype(t->type);
			nested_unions[num_nested_unions].field = fields;
			num_nested_unions++;
			anon_union = false;
		    }
		}
	    } else {
		*fields = (struct VarStruct *)malloc(sizeof(struct VarStruct));
		if(var_element(t, t->type, var, false, *fields, UNKNOWN_LIST))
		    return -1;
		fields++;
	    }
	}
	t = t->next_sym;
    }
    *fields = ( struct VarStruct * )nil;

    for( index = 0; index < num_nested_unions; index++ ) {
	(void)reset_index_under_union(
				nested_unions[index].anon_union_class->chain,
				nested_unions[index].field );
    }

    return num;
}


/*
 * NAME: className
 *
 * FUNCTION: Construct the name of the variable which is a BASECLASS
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES: 
 *
 * PARAMETERS:
 *	fatherName	- Name of the father of the variable
 *	s		- Variable to generate name for
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: Name - memory is allocated needs to be freed by caller
 */
static char    *className( fatherName, s )
char	*fatherName;
Symbol              s;
{
    char    *name;
    char    *s_name = symname(s);

    /*
     * 6 == 4 (parenthesis) + 1 (slash) + 1 (NULL)
     */
    name = malloc( (strlen(fatherName) + strlen(s_name) + 6 ) * sizeof(char));
    sprintf( name, "((%s)\\%s)", fatherName, s_name );
    return name;
}


/*
 * NAME: var_element
 *
 * FUNCTION: Creates a VarStruct from a dbx Symbol.
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES: 
 *
 * PARAMETERS:
 *	s	- Symbol used to obtain name
 *	stype	- Symbol used for other data
 *	father	- VarStruct father
 *	expflag	- not currently used
 *	var	- VarStruct to be filled
 *	scope	- Which list this variable is from
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: 
 *  -1:	failure
 *   0:	success
 */
int var_element(s, stype, father, expflag, var, scope)
Symbol s;
Symbol stype;
struct VarStruct *father;
Boolean expflag;
struct VarStruct *var;
int scope;
{
    char    *arrayElemName( ), *targetName( ), *fieldName( );
    int     upperclass;
    Symbol  t;
    int     dims, length = 0;

    if( s->class == CONST ) {
	/*
	 * If the variable is a constant than we want to ignore its type and
	 * treat it as a constant
	 */
	stype = s;
    }

    /* Initialize the element structure */

    if ( !father ) {
	var->symbol = s;
	if ( s->block ) {
	   dpi_var_scope ( s->block, &var->scope, &length );
	} else var->scope = NULL;
    } else {
	var->symbol = father->symbol;
	var->scope = father->scope;
    }
    var->parent = father;
    var->implode = 
        father ? ( father->implode ? father->implode : father ) : father;
    var->chain = nil;
    var->range = nil;
    var->format = NOFORMAT;
    var->inline = false;
    var->decl = 0;
    var->decl_lines = 0;

    /*
     * Reinitialize language dependent variables based 
     * upon the language of the selected variable.
     */
    var_lang_init( s );

    /* Initialize the name and basename of the element structure
     * based upon the type (class) of the variable.
     */
    upperclass = father ? father->class : nil;
    switch( upperclass) {
    case V_ARRAY:
        var->name = arrayElemName( father->name, father->range );
        var->basename = arrayElemName( father->basename, father->range );
	break;
    case V_RECORD:
    case V_UNION:
        var->name = fieldName( father->name, s );
        var->basename = fieldName( father->basename, s );
	break;
    case V_CLASS:
	if( s->class == BASECLASS ) {
	    var->name = className( father->name, s );
	} else {
	    var->name = fieldName( father->name, s );
	}
	var->basename = fieldName( father->basename, s );
	break;
    case V_PTR:
        var->implode = father;
        var->name = targetName( father->name );
        var->basename = targetName( father->basename );
	break;
    default:
        var->name = malloc(( strlen( symname( s )) + 1 ) * sizeof(char));
        strcpy( var->name, symname( s ) );
        var->basename = malloc(( strlen( symname( s )) + 1 ) * sizeof(char));
        strcpy( var->basename, symname( s ) );
	break;
    }

    /*
     * if stype is TAG or stype is a fortran derived type (i.e.
       equivalent to TAG) set stype to last TAG Symbol or fortran derived
       type Symbol in type chain
     * If stype is CPPREF set stype to type of reference variable
     */
    for( t = stype; t &&
        ( t->class == TAG || t->class == CPPREF ||
	  ( t->class == TYPE && t->language != fLang ) ||
	  isfortranderived(t));
	 t = stype->type )
	stype = t;

    if ((stype && ((stype->class == TAG ) ||
		   ( stype->class == TYPE && stype->language != fLang ) ||
                   (isfortranderived(stype)))))
    {
        if ( nilname( stype ) || cpp_tempname(stype->name) || expflag ) {

	    /*  untagged structure: get data from Symbol below TAG  */

            var->key = stype->type;
            var->class = varClass( stype->type);
     	    if ( (*varDecl)( s, stype, var ) )
		return -1;
            if ( var->class != V_SCAL ) {
                declindex++;
		if( var->class == V_CLASS ) {
		    if ( var_class( var ) == -1 )
		    return -1;
		} else {
		    if ( var_chain( var ) == -1 )
		    return -1;
		}
                var->index = declindex++;
            } else
                var->index = declLines( var->decl ) - 1 + declindex++;
  	    var->decl_lines = var->index;
        } else {

	    /*  tagged structure:  get data from TAG Symbol  */

            var->key = stype;
            var->class = varClass( stype);
     	    if ( ( *varDecl )( s, stype, var ) )
		return -1;
            var->index = declLines( var->decl ) - 1 + declindex++;
	    var->decl_lines = var->index;
        }
    } else if ( stype && stype->class == ARRAY ) {
     	if ( (*varDecl)( s, stype, var ) )
	    return -1;

	/*  set V_ARRAY fields and create VarStruct for array element  */

	var->index = declLines(var->decl) - 1;
	var->decl_lines = var->index; /* used in case of COBOL only */
        if ( var_arrayChain( s, &var ) )
	    return -1;
    } else if ( stype && stype->class == PTR ) {
        var->class = varClass(stype);
     	if ( (*varDecl)( s, stype, var ) )
	    return -1;
	declindex += declLines( var->decl );  /* points to line beyond decl */
        var->index = declindex - 1;
	var->decl_lines = var->index;
        var->key = stype;
    } else if ( stype && ( stype->class == GROUP || stype->class == RGROUP ||
                ( stype->chain && stype->chain->class == COND ))) {
            var->class = varClass( stype);
	    if ( (*varDecl)( s, stype, var ) )
                return -1;
            var->key = stype;
            var->decl_lines = declLines( var->decl );
            var->index = declindex++;
            var_chain( var );
    } else if ( (dims = is_variable_a_cobol_array( s )) ) {
        if ( ! var->parent ) {
		if ( (*varDecl)( s, stype, var ) )
		    return -1;
                create_varStruct_for_cobol_arrays( s, &var);
                var->class = V_MOD_ARRAY;
                return 0;
        } else {
                var->class = varClass( stype);
		if ( (*varDecl)( s, stype, var ) )
		    return -1;
		var->index = declindex;
		var->decl_lines = declindex++;
        }
    } else 
    {
             
        /* handle thread, mutex, condition variable and attribute symbols */
#if defined (CMA_THREAD) || defined (K_THREADS)
        if ((s->type->class == RECORD) &&
           (((lib_type == C_THREAD) &&
             (isThreadObjectSymbol_cma(s) == true))  ||
            ((lib_type == KERNEL_THREAD) &&
             (isThreadObjectSymbol_k(s) == true))))
        {
            var->key = s->type;
            var->class = varClass( s->type);
            if ( (*varDecl)( var->key, stype, var ) )
            {
               return -1;
            }
            declindex++;
            if ( var_chain( var ) == -1 )
            {
                return -1;
            }
        }
        else
#endif /* CMA_THREAD || K_THREADS */
        {
            if ( stype )
                var->class = varClass( stype);
            else
                var->class = varClass( s);
            if( var->class == V_FFUNC ) {
                /*
                 * If this is a function member we need to set the symbol to the
                 * actual symbol for the function
                 */
                var->symbol = s->symvalue.member.attrs.func.funcSym;
            }
            if ( (*varDecl)( s, stype, var ) )
                return -1;
            var->key = stype;
        }
        var->index = declindex++;
        var->decl_lines = var->index;
    }
    return 0;
}

/*
	Checks to see if the cobol variable is an array or not, if it is
	an array then find its dimension. We find the dimension of the array
	by traversing the parent's chain to see how many parents are of type
	array.
*/
int find_dimension_of_cobol_arrays(s)
Symbol s;
{
    Symbol p;
    int dim = 0;

    if ( s->language == cobLang ) {
        p = s;
        while ( p ) {
	    if (p->type->class == ARRAY)
	       dim++;
	    p = p->symvalue.field.parent;
        }
    }
    return dim;
}

/*
 * this routine serves as an abstract front end onto var_element()
 * Returns -1 (failure)  0 (success)
 */
int  dpi_var_element( key, var, scope )
void                *key;
struct VarStruct    **var;
int                 scope;
{
   Symbol   s;

   s = (Symbol)key;
   adc_var_lang_init( s );
   *var = ( struct VarStruct * ) malloc( sizeof( struct VarStruct ));
   declindex = 0;
   touch_file( s );
   return( var_element( s, s -> type, NULL, false, *var, scope ));
}


/*
 * NAME: dpi_variable_block
 *
 * FUNCTION: Returns the Symbol's block->name; Used for variable retainment
 *	     in ADC. The value returned should not be freed or changed by
 *           the caller.
 *
 * PARAMETERS:
 *	s	- Symbol to get the block
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: The name represented by the Symbol
 */
char  *dpi_variable_block( s )
Symbol	s;
{
   return( symname( s -> block ));
}


/*
 * NAME: dpi_variable_name
 *
 * FUNCTION: Returns the Symbol's name; Used for variable retainment
 *	     in ADC
 *
 * PARAMETERS:
 *	s	- Symbol to get the name
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: The name represented by the Symbol
 */
char  *dpi_variable_name( s )
Symbol        s;
{
   return( symname( s ));
}


/*
 * NAME: fortran_varDecl
 *
 * FUNCTION: Creates a declaration for a Fortran VarStruct
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES:
 *
 * PARAMETERS:
 *      s       - Variable to get Declaration of
 *      stype   - Type of variable
 *      var     - VarStruct to be updated with declaration
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: 0  (successful)
 *          -1 (failure)
 */
int fortran_varDecl( s, stype, var )
Symbol    s;
Symbol    stype;
struct VarStruct    *var;
{
    char    *nameptr;
    char    *ssptr;
    char    *newstring;
    char    *newdecl;
    char    *fmtnewdecl;
    char    *derivedname;
    char    *typeline;
    char    *basename;
    char    *std;
    char    *ends;
    int      length_vardecl;
    int      length_typeline;

#define TYPE_LINE     "type %s\n"
#define END_TYPE_LINE "end type "

    if ( var->parent ) {
	var->decl = malloc(( strlen( var->parent->decl ) + 1 ) * sizeof(char));
	strcpy( var->decl, var->parent->decl );
	if ( var->parent->class == V_ARRAY ) {

	    /*
	     * Search for parent's basename and remove (preserving decl type).
	     * For example: for a parent's declaration of "integer*4 a(20)",
             * transform the child's declaration to "integer*4 a(1)".
             */

	    ssptr = var->decl;
	    while ( *ssptr != *beg_delim && *ssptr != '\n' ) {
	    	nameptr = strstr( ssptr, var->parent->basename );
		if ( !nameptr )
		    return -1;
		ssptr = nameptr + strlen( var->parent->basename );
		if ( ssptr > var->decl + strlen( var->decl ) )
		    return -1;
	    }
            *nameptr = '\0';

	    /*   append var's basename   */

	    var->decl = ( char * )realloc( var->decl, strlen( var->decl ) +
		strlen( var->basename ) + 1 );
	    strcat( var->decl, var->basename );
	}
        else
        {
           if (( var->parent->class == V_TAG ) ||
               ( s->class == FIELD))
           {
              msgbegin;
              if (lazy)
                touch_sym(s);
              printdecl( s );
              msgcopyend( var->decl );
              if ( var->parent->class == V_TAG )
              {
                length_vardecl = strlen(var->decl);
                /* update the declaration to put () around the derived
                   type name and to add the basename
                   on the "type" and "end type" lines.
                   if any expected fields are not found, the existing
                   declaration will not be changed.
                */
                /* Allocate strings to hold the derived name and the
                   new version of the "type (derivedname) varname" line
                */
                derivedname = ( char * )malloc(length_vardecl + 1);
                typeline = ( char * )malloc(length_vardecl +
                    strlen( var->basename) + 3);
                *derivedname = '\0';
                /* search the current declaration and fetch the
                   name of the derived type
                */
                sscanf(var->decl,TYPE_LINE,derivedname);
                /* only continue if "type" line was found */
                if (*derivedname != '\0')
                {
                  /* locate the position in the current declaration of the
                     start of the line past the type line
                  */
                  std =strchr(var->decl,'\n');
                  if (std != NULL)
                  {
                    std++;
                    /* locate the position in the current declaration of the
                       start of the line containing the "end type ..."
                    */
                    ends = strstr(std,END_TYPE_LINE);
                    /* only continue if "end type" line was found */
                    if (ends != NULL)
                    {
                       /* allocate a new string to hold the new format
                          of the declaration
                          (7 = 2 sets of () + 2 spaces  + null terminator)
                       */
                       newdecl = ( char * )malloc(length_vardecl +
                           (2 * strlen( var->basename)) + 7);
                       *newdecl = '\0';
                       /* build a new "type" line */
                       /* if basename is a qualified name of the
                          form xxx.yyy.zzz, use just the name (i.e. zzz).
                          If not qualified, use the existing basename.
                       */
                       basename = strrchr(var->basename,'.');
                       if (basename == NULL)
                       {
                          sprintf(typeline,"type (%s) %s\n",
                                derivedname,var->basename);
                       }
                       else
                       {
                          basename++;
                          sprintf(typeline,"type (%s) %s\n",
                                derivedname,basename);
                       }
                       /* Build the new declaration using the new "type" line,
                          the lines between the "type" and "end type" line
                          from the existing declaration, and the new
                          "end type" line
                       */
                       fmtnewdecl = newdecl;
                       length_typeline = strlen(typeline);
                       memcpy(fmtnewdecl,typeline,length_typeline);
                       fmtnewdecl += length_typeline;
                       /* if there are lines between the "type" line and the
                          "end type" line, then concatenate those lines
                          to the new declaration.
                       */
                       if (ends != std)
                       {
                           memcpy(fmtnewdecl,std,ends-std);
                           fmtnewdecl += ends-std;
                       }
                       /* concatenate a new "end type" line to the new
                          declaration
                       */
                       sprintf(fmtnewdecl,"end %s",typeline);
                       /* free the memory allocated for the existing variable
                          declaration and memory allocated for temporary
                          basename
                       */
                       free(var->decl);
                       /* update pointer in varStruct to new declaration */
                       var->decl = newdecl;
                    }
                  }
                }
                free(typeline);
                free(derivedname);
              }
           }
        }

    } else {
    	msgbegin;
        if (lazy)
            touch_sym(s);
    	printdecl( s );
    	msgcopyend( var->decl );

        /*
         * Check if the variable has an unknown type and
         * change the declaration if it does
         */
        if( *var->decl == '&' ) {
            newstring = malloc( (15 + strlen(var->name) ) * sizeof(char));
            sprintf( newstring, "Unknown Type %s\n", var->name );
            dispose( var->decl );
            var->decl = newstring;
        }
    }
    return 0;
}

/*
 *  this routine creates a declaration for a c VarStruct
 *  Always returns 0.
 */

int c_varDecl( s, stype, var )
Symbol    s;
Symbol    stype;
struct VarStruct    *var;
{
    char    *fieldname;
    char    *newstring;

    if ( var->parent->class == V_ARRAY ) {
	var->decl = malloc(( strlen( var->parent->decl ) + 1 ) * sizeof(char));
	strcpy( var->decl, var->parent->decl );

	if ( var->parent->parent && var->parent->parent->class == V_PTR ) {

	    /* weird decl: truncate at '[' */

	    fieldname = malloc( 2 );
	    strcpy( fieldname, "[" );
	} else {

            /* truncate at parent's basename */

	    fieldname = malloc(( strlen(var->parent->basename) + 2) *
                                 sizeof(char));
            sprintf( fieldname, "%s[", var->parent->basename );
	}

	/*
	 * append var's basename to decl.
	 * For example: for a fieldname of "characters[",
         * var->decl of "unsigned char characters[5]",
	 * and var->basename of "characters[0]", 
         * transform var->decl to "unsigned char characters[0]".
         */

	changeDecl( fieldname, var->basename, &var->decl );
	dispose( fieldname );	
    } else if ( var->parent->class == V_PTR ) {
	var->decl = malloc( strlen( var->parent->decl ) + 1 );
	strcpy( var->decl, var->parent->decl );
	if ( strstr( var->decl, "* " ) )

	    /* many dpi_var routines search on basename in decl.
	       Therefore straight copy will not do here.  Replace
	       '* name' with '*name', which is basename. */

	    changeDecl( "* ", var->basename, &var->decl );
	if ( !strstr( var->decl, var->basename) )
	    changeDecl( var->parent->basename, var->basename, &var->decl );
    } else if ( var->parent->class == V_TAG ) {
        msgbegin;
        if (lazy)
            touch_sym(s);
        printdecl( s );
        msgcopyend( var->decl );

	/*
         * decl returned has no name; remove ';' and append basename.
         * For example: for var->decl of 
         *    "struct _tag1 {\n    int one;\n    int two;\n};\n"
	 * and var->basename of "plain_one_tag", append basename to decl,
         * resulting in: 
         *    "struct _tag1 {\n    int one;\n    int two;\n} plain_one_tag\n"
         */

	changeDecl( ";", var->basename, &var->decl );

    } else if ( s->class == FIELD || s->class == MEMBER ) {
        msgbegin;
        if (lazy)
            touch_sym(s);
        printdecl( s );
        msgcopyend( var->decl );

	/*
	 * decl returns with field name; we need actual basename. 
	 * create unique string to search on and call routine to
	 * truncate there and append basename.
	 * For example: for a fieldname of "one", var->decl of "int one;",
	 * and var->basename of "plain_one.one", 
         * transform var->decl to "int plain_one.one" where plain_one
         * refers to the parent structure for this field element.
         */

	fieldname = malloc(( strlen( symname( s )) + 2 ) * sizeof(char));
	strcpy( fieldname, symname( s ) );
	if ( stype->class == ARRAY ) {
	    strcat( fieldname, "[" );
	    changeArrayDecl( fieldname, var->basename, &var->decl );
	} else {
	    strcat( fieldname, ";" );
	    changeDecl( fieldname, var->basename, &var->decl );
	} dispose( fieldname );	
    } else {
        msgbegin;
        if (lazy)
            touch_sym(s);
        printdecl( s );
        msgcopyend( var->decl );

        /*
         * Check if the variable has an unknown type and
         * change the declaration if it does
         */
        if( *var->decl == '&' ) {
            newstring = malloc( (15 + strlen(var->name) ) * sizeof(char));
            sprintf( newstring, "Unknown Type %s\n", var->name );
            dispose( var->decl );
            var->decl = newstring;
        }
    }
    return 0;
}


/*
 * NAME: cpp_varDecl
 *
 * FUNCTION: Creates a declaration for a C++ VarStruct
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES: 
 *
 * PARAMETERS:
 *	s	- Variable to get Declaration of
 *	stype	- Type of variable
 *	var	- VarStruct to be updated with declaration
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: 0
 */
#define	DERIVED_CLASS	"        class %s \n"
#define	PRIVATE_SEC	"    private:\n"
#define	PUBLIC_SEC	"    public:\n"
#define	DERIVED		"    Parents:\n"
#define INCR_AMOUNT	50
int cpp_varDecl( s, stype, var )
Symbol    s;
Symbol    stype;
struct VarStruct    *var;
{
    int		rc;
    char	*section;
    char	*new_lines;
    char	*first_line;
    int		size_new_lines = INCR_AMOUNT;
    int		length, items_in = 0;
    int		len_derived_class;
    unsigned	orig_section_name;
    Symbol	baseclass, tmp;
    char	*name_baseclass;

    /*
     * Use regular C declaration function to get basic declaration
     */
    rc = c_varDecl( s, stype, var );
    /*
     * For non-Class variables nothing else needs to be done
     */
    if( var->class == V_CLASS ) {
	/*
	 * Need to search for derived and virtual classes and add their
	 * declaration into the declaration of the base class
	 */
	len_derived_class = strlen( DERIVED_CLASS );

	/*
	 * Save off the access type of the base class
	 */
	switch( var->key->symvalue.class.key ) {
	    case 'c':
		orig_section_name = PRIVATE;
		break;
	    case 'u':
	    case 's':
	    default:
		orig_section_name = PUBLIC;
		break;
	}
	baseclass = var->key->type;
	new_lines = malloc( size_new_lines * sizeof( char ) );
	while( baseclass && baseclass->class == BASECLASS ) {
	    /*
	     * This is a derived or virtual class; Need to add its name to
	     * declaration along with any other derived or virtual classes.
	     *
	     * If this is the first one found add section header
	     */
	    if( items_in == 0 ) {
		items_in += sprintf( new_lines, DERIVED );
	    }

	    name_baseclass = symname( baseclass );
	    length = strlen( name_baseclass );
	    while( (items_in + length + len_derived_class + 1 ) >
		   size_new_lines ) {
		if( (length + len_derived_class + 1) > INCR_AMOUNT ) {
		    size_new_lines += length + len_derived_class + 1;
		} else {
		    size_new_lines +=INCR_AMOUNT;
		}
		new_lines = (char *)realloc( new_lines,
					     size_new_lines * sizeof(char));
	    }

	    /*
	     * Append derived class declaration to others
	     */
	    items_in += sprintf( new_lines + items_in, DERIVED_CLASS,
				 name_baseclass );

	    baseclass = baseclass->chain;
	}

	if( items_in != 0 ) {

	    /*
	     * Check if a section name needs to be added after the
	     * derived/virtual class declarations.
	     */
	    tmp = var->key->chain;
	    while( tmp->symvalue.member.isCompGen || tmp->class == CONST ) {
		tmp = tmp->next_sym;
	    }
	    if( orig_section_name == tmp->symvalue.member.access ) {
		/*
		 * Add access section information
		 */
		switch( orig_section_name ) {
		    case PRIVATE:
			section = PRIVATE_SEC;
			break;
		    case PUBLIC:
			section = PUBLIC_SEC;
			break;
		}
		length = strlen( section );
		if( (items_in + length + 1) > size_new_lines ) {
		    size_new_lines = items_in + length + 1;
		    new_lines = (char *)realloc( new_lines,
						 size_new_lines * sizeof(char));
		}

		strcpy( new_lines + items_in, section );
		items_in += length;
	    }

	    /*
	     * We have found some derived/virtual classes, add their lines
	     * into the declaration for the variable after the first line
	     */
	    length = strlen( var->decl ) + items_in + 1;
	    var->decl = (char *)realloc(var->decl, length * sizeof(char));
	    first_line = strchr( var->decl, '\n' );
	    first_line++;
	    memmove( first_line + items_in, first_line,
		     strlen( first_line ) + 1 );
	    memcpy( first_line, new_lines, items_in );
	}
    }
    return rc;
}


/*
 * NAME: adc_namecmp
 *
 * FUNCTION: Compares the names of two VariableLists
 *
 * NOTES: Used by ADC only
 *
 * PARAMETERS:
 *	a	- First VariableList to compare
 *	b	- Second VariableList to compare
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: 0 if the two VariableLists have the same name
 *	    Number < 0 if a < b alphabetically
 *	    Number > 0 if a > b alphabetically
 */
int        adc_namecmp( a, b )
VariableList    **a;
VariableList    **b;
{
     return( strcmp(( *a ) -> name, ( *b ) -> name ));
}


/*
 * changes given character string by truncating at oldString in last line
 * and appending newString.  Used to change VarStruct decl element.
 * For example: for an oldString of "characters[", newString of "characters[0]",
 * and decl of "unsigned char characters[5]", 
 * transform decl to "unsigned char characters[0]".
 */
                    
int  changeDecl( oldString, newString, decl )
char	*oldString;
char	*newString;
char	**decl;
{
    char    *declend;

    /* find last line */

    declend = strrchr( *decl, '\n' );
    if ( !declend )			/* no newline */
	declend = *decl;
    else if ( strlen( declend ) == 1 ) {	/* end of decl */
	*declend = nil;
	declend = strrchr( *decl, '\n' );
	if ( !declend )
	    declend = *decl;
    }

    /* remove oldString and any remaining chars and append newString */

    declend = strstr( declend, oldString );
    if ( declend )
	*declend = nil;
    *decl = 
	( char * )realloc( *decl, strlen( *decl ) + strlen( newString ) + 3 );
    strcat( *decl, " " );
    strcat( *decl, newString );
    strcat( *decl, "\n" );
    return 0;
}


/*
 * changes given character string by replacing oldString in last line
 * with newString.  Used to change array VarStruct decl element.  Could
 * be combined with changeDecl. (Used when declaration to change is an 
 * ARRAY type variable, and that variable is an element of a structure/union).
 * For example: for an oldString of "one[", newString of "plain_one[0].one",
 * and decl of "int  one[5]", 
 * transform decl to "int  plain_one[0].one[5]".
 * Returns -1 (failure)  0 (success)
 */

int  changeArrayDecl( oldString, newString, decl )
char	*oldString;
char	*newString;
char	**decl;
{
    char    *declend;
    char    *ssstr;

    /* find last line */

    declend = strrchr( *decl, '\n' );
    if ( !declend )			/* no newline */
	declend = *decl;
    else if ( strlen( declend ) == 1 ) {	/* end of decl */
	*declend = nil;
	declend = strrchr( *decl, '\n' );
	if ( !declend )
	    declend = *decl;
    }

    /* remove oldString and append newString */

    declend = strstr( declend, oldString );
    if ( declend )
	*declend = nil;
    else
	return -1;
    declend += strlen( oldString ) - 1;		/* point to subscript */
    ssstr = malloc(( strlen( declend ) + 1 ) * sizeof(char));
    strcpy( ssstr, declend );			/* subscripts */
    *decl = 
	( char * )realloc( *decl, strlen( *decl ) + strlen( newString ) 
	+ strlen( ssstr ) + 3 );
    strcat( *decl, " " );
    strcat( *decl, newString );
    strcat( *decl, ssstr );
    strcat( *decl, "\n" );
    dispose( ssstr );
    return 0;
}

/*
 *  this routine truncates the char string name after basename and then
 *  appends subscstr.  Used to change the subscript of the name field
 *  of a fortran VarStruct, when performing the "( )" operation.
 *  For example: for a basename "a", subscstr of "(4)", and name of "a(1)",
 *  transform name to "a(4)".
 *  Returns -1 (failure)  0 (success)
 */

int  fortran_changeName( basename, subscstr, name )
char    *basename;
char    *subscstr;
char    **name;
{
    char    *sbsc_ptr;

    sbsc_ptr = strstr( *name, basename );
    if ( !sbsc_ptr )
	return -1;
    sbsc_ptr += strlen( basename );	/* subscript after basename */
    *sbsc_ptr = nil;
    *name = 
        ( char * )realloc( *name, strlen( *name ) + strlen( subscstr ) + 1 );
    strcat( *name, subscstr );		/* add new subscript string to base */
    return 0;
}

/*
 *  this routine changes the character string name, by searching for the
 *  first c subscript string following basename, and replacing it with
 *  subscstr.  Use to change a subscript of the name field of a c
 *  VarStruct, when performing the "[ ]" operation.
 *  For a simple example: for a basename "characters", subscstr of "[3]",
 *  and name of "characters[0]",
 *  transform name to "characters[3]".
 *  Returns -1 (failure)  0 (success)
 */

int c_changeName( basename, subscstr, name )
char    *basename;
char    *subscstr;
char    **name;
{
    char    *sbsc_ptr;
    char    *suff_ptr;
    char    *suffix;

    sbsc_ptr = strstr( *name, basename );
    if ( !sbsc_ptr )
	return -1;
    sbsc_ptr += strlen( basename );	/* subscript after basename */
    sbsc_ptr = strchr( sbsc_ptr, '[' );
    suff_ptr = sbsc_ptr;
    while ( *suff_ptr++ == '[' )
        while ( *suff_ptr++ != ']' )
            ;
    suff_ptr--;				/* first char after last subscript */
    *sbsc_ptr = nil;
    suffix = malloc(( strlen( suff_ptr ) + 1 ) * sizeof(char));
    strcpy( suffix, suff_ptr );		/* first char after last subscript */
    *name = 
        ( char * )realloc( *name, strlen( *name ) + strlen( subscstr ) + 
        strlen( suffix ) + 1 );
    strcat( *name, subscstr );		/* add new subscript string to base */
    strcat( *name, suffix );		/* add any remaining chars to end */
    dispose( suffix );
    return 0;
}

/*
 *  this routine initializes an array VarStruct and creates an
 *  underlying VarStruct representing an element of the array
 *  Returns -1 (failure)  0 (success)
 */

int  var_arrayChain( sym, varaddr )
Symbol          	sym;
struct VarStruct    	**varaddr;
{
    Symbol      t;
    Symbol      tchain;
    int         i, num;
    Range       **dptr;
    Symbol      varkey;
    struct VarStruct    *var;
    struct VarStruct    **endchain;

    /*
     * Special handling for COBOL arrays
     */
    if ( sym->language == cobLang ) {
        create_varStruct_for_cobol_arrays(sym, varaddr);
        return 0;
    }
    var = *varaddr;
    var->class = V_ARRAY;

    /* get number of dimensions */
    for ( num = 0, t = sym->type; t->class == ARRAY; num++, t = t->type )
        ;

    /*
     * Initialize the range structure for each array dimension:
     *    upper, lower, base.
     */
    var->key = t;
    var->range = ( Range ** )calloc( num + 1, sizeof( Range * ) );
    dptr = var->range;
    t = sym->type;
    for ( i = 0; i < num; i++ ){
        *dptr = ( Range * )malloc( sizeof( Range ) );
        tchain = t->chain;
	/* Fortran range type can be of not type const, if so */
	/* we need to use getbound() to get its real value.   */
	/* Constant range type == R_CONST == 0                */
	if (tchain->symvalue.rangev.lowertype)
	  getbound( t, tchain->symvalue.rangev.lower, 
		    tchain->symvalue.rangev.lowertype, &((*dptr)->lower) );
	else
          (*dptr)->lower = tchain->symvalue.rangev.lower;
	if (tchain->symvalue.rangev.uppertype)
	  getbound( t, tchain->symvalue.rangev.upper, 
		    tchain->symvalue.rangev.uppertype, &((*dptr)->upper) );
	else
          (*dptr)->upper = tchain->symvalue.rangev.upper;
        (*dptr)->base = (*dptr)->lower;
        dptr++;
        t = t->type;
    }
    *dptr = ( Range * )nil;

    varkey = var->key;

    /*  create VarStruct pointer array (for array element chain) */

    var->chain =
         ( struct VarStruct ** )calloc( 2, sizeof( struct VarStruct * ) );
    *var->chain = ( struct VarStruct * )malloc( sizeof( struct VarStruct ) );

    /*  create VarStruct for array element  */

    if ( var_element( sym, t, var, false, *var->chain, UNKNOWN_LIST ) )
	return -1;
    var->index = (*var->chain)->index;		/* set to element index */
    var->decl_lines = var->index;
    endchain = var->chain;
    *++endchain = nil;				/* nil end of VarStruct arr */
    return 0;
}


/*
 * Resets the base element for the array, and all appropriate children.
 */

void  fill_base_for_cobol_array_elements( var, dims, base)
struct VarStruct        *var;
int                     dims;
int                     base;
{
        struct VarStruct **chain;
        int     i = 0;

       /*
        *  reset base of array to match 'base' argument.
        */
        if ( var->class == V_ARRAY || var->class == V_MOD_ARRAY ) {
                var->range[dims]->base = base;
                chain = var->chain;
        }
        else if ( var->class == V_GROUP )
                chain = var->chain;
        else
                return;
        /*
         * Recursively update base of children of type ARRAY or GROUP.
         */
        while ( chain[i] ) {
                if ( chain[i]->class == V_GROUP || chain[i]->class == V_ARRAY) {
                    fill_base_for_cobol_array_elements( chain[i], dims, base);
                }
                i++;
        }
}


/*
 *  this routine changes the subscript of the name, basename and
 *  decl fields of the array element VarStruct and any underlying 
 *  VarStructs.  If changeBase is set, the range base of the array 
 *  VarStruct is changed.
 *  Returns -1 (failure)  0 (success)
 */
  
int dpi_change_array_subscripts( arrayVar, arrayDims, changeBase )
struct VarStruct    	*arrayVar;
int                    	*arrayDims;
boolean                	changeBase;
{
    char    	charr[20];
    char    	*ssptr;
    char    	*subscstr;
    struct Range    	**vDims;
    int		*vBase;
    int		numDims;

    ssptr = &charr[0];
    /*
     * Check for invalid variable type, unitialized array chain,
     * unitialized array range, and invalid array dimensions argument.
     */
    if ( arrayVar->class != V_ARRAY && arrayVar->class != V_MOD_ARRAY )
        return -1;
    if ( arrayVar->chain == nil )
        return -1;
    if ( ( vDims = arrayVar->range ) == nil )
        return -1;
    if ( ( vBase = arrayDims ) == nil )
        return -1;

    /*
     * build subscript string from range base.
     * If changeBase == True, reset base of array to match arrayDims argument.
     */

    numDims = 0;
    while ( *vDims ) {
        if ( changeBase ) {
	   if( islang_cobol( arrayVar->symbol->language ) )
		fill_base_for_cobol_array_elements(arrayVar, numDims, *vBase++);
	   else
            	(*vDims)->base = *vBase++;
	}
        vDims++;
	numDims++;
    }
    subscriptString( arrayDims, numDims, &subscstr );
        
    /* change subscript of array element, to match array base element */

    if ( changeSubscript
	( *arrayVar->chain, arrayVar->basename, arrayVar->name, subscstr ) )
	return -1;
    dispose( subscstr );
    return 0;
}


/*
 *  this routine changes the subscript of the name, basename, and decl
 *  fields of a VarStruct and any underlying VarStructs.
 *  (i.e. from "intarray[0]" to "intarray[3]" for the basename, name and decl).
 *  Returns -1 (failure)  0 (success)
 */

int changeSubscript( var, basename, name, subscstr )
struct VarStruct    *var;
char                *basename;
char                *name;
char                *subscstr;
{
    char                *nameptr;
    char                *ssptr;
    struct VarStruct    **varptr;

    /*  If the variable is an COBOL array, then this routine doesn't do the
        job, this is because the information stored in the decl part of the
        varstruct is different for C and COBOL.
    */
    if ( dpi_if_cobol( var->symbol ) ) {
        changeSubscriptForCobolArrays( var, subscstr );
        return 0;
    }
    /*  remove var->basename from var->decl */

    nameptr = strrchr( var->decl, '\n' );
    if ( !nameptr )
        nameptr = var->decl;
    else {
	*nameptr = nil;
    	nameptr = strrchr( var->decl, '\n' );
    	if ( !nameptr )
    	    nameptr = var->decl;
    }
    nameptr = strstr( nameptr, var->basename );
    if ( !nameptr )
	return -1;
    *nameptr = nil;
    if ( var->class == V_ARRAY ) {
        /* subscript pointer needed */
        nameptr += strlen( var->basename );
        ssptr = malloc(( strlen( nameptr ) + 1 ) * sizeof(char));
        strcpy( ssptr, nameptr );
    }

    /* change var->basename and var->name */

    if ( (*changeName)( basename, subscstr, &var->basename ) )
	return -1;
    if ( (*changeName)( name, subscstr, &var->name ) )
	return -1;

    /* append changed var->basename to var->decl */

    if ( var->class == V_ARRAY ) {
        var->decl = ( char * )realloc
            ( var->decl, strlen( var->decl ) + strlen( var->basename ) 
            + strlen( ssptr ) + 2 );
        strcat( var->decl, var->basename );
        strcat( var->decl, ssptr );
        strcat( var->decl, "\n" );
        dispose( ssptr );
    } else {
        var->decl = ( char * )realloc
            ( var->decl, strlen( var->decl ) + strlen( var->basename ) + 2 );
        strcat( var->decl, var->basename );
        strcat( var->decl, "\n" );
    }

    /* proceed down the chain, to update the children of the array. */

    if ( var->chain )
        for ( varptr = var->chain; *varptr; varptr++ )
            if ( changeSubscript( *varptr, basename, name, subscstr ) )
		return -1;

    return 0;
}


/*
 *  this routine changes the subscript of the name, basename, and decl
 *  fields of a VarStruct and any underlying VarStructs for a Cobol ARRAY.
 *  Always returns 0.
 */

int changeSubscriptForCobolArrays( var, subscstr )
struct VarStruct    *var;
char                *subscstr;
{
	char *open_parenthesis;
	unsigned int	length_of_subscript;
	char *p, *q;
	unsigned int i;

	/* Change the name, so that the name reflects the new subscript info */
	open_parenthesis = strchr( var->name, '(' );
	p = subscstr;
	q = strrchr ( var->basename, '(' );
	length_of_subscript = strlen( subscstr );
	for (i=0; i < length_of_subscript; i++ ) {
		*open_parenthesis++ = *p;
		*q++ = *p++;
	}

	/* Proceed down the chain, to update the children of the array */
	i = 0;
	while ( var->chain[i] ) {
	    if ( var->chain[i]->class == V_ARRAY ) {
		char str[256];
		char *close_paren;

		strcpy(str, subscstr);
		close_paren = strrchr(str, ')' );
		if (close_paren)
                    *close_paren = '\0';
	        changeSubscriptForCobolArrays( var->chain[i]->chain[0], 
			str);
	    }
	    i++;
	}
	return 0;
}
          
/*
 *  this routine returns the name of an array element VarStruct
 *  given the name of the above array VarStruct and its range.
 */

char    *arrayElemName( fatherName, range )
char	*fatherName;
struct Range	**range;
{
    char            *name;
    char	    *subscripts;
    int             i, num;
    int		    *dims;
    struct Range    **rangeptr;

    if ( *fatherName == '*' ) {
        name = malloc(( strlen( fatherName ) + 3 ) * sizeof(char));
        strcpy( name, "(" );
        strcat( name, fatherName );
        strcat( name, ")" );
    } else {
        name = malloc(( strlen( fatherName ) + 1 ) * sizeof(char));
        strcpy( name, fatherName );
    }
    
    /*   get number of dimensions   */

    rangeptr = range;
    num = 0;
    while ( *rangeptr++ ) {
	num++;
    }

    /*   create array of dimension lower bounds   */

    dims = ( int * )calloc( num, sizeof( int ) );
    rangeptr = range;
    for ( i = 0; i < num; i++ )
	dims[i] = ( *rangeptr++ )->lower;

    /*   put subscripts in string form and append to name   */

    subscriptString( dims, num, &subscripts );
    name = ( char * )realloc( name, strlen( name ) + strlen( subscripts ) + 1);
    strcat( name, subscripts );
    dispose( subscripts );

    return name;
}


/*
 * NAME: fieldName
 *
 * FUNCTION: Returns the name of a VarStruct of a field of a structure/union,
 *	given the name of the structure/union VarStruct and the symbol
 *	representing the field.
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES: For example: Returns "plain_one.one" or "plain_one->one".
 *
 * PARAMETERS:
 *	fatherName	- Name of parent of symbol
 *	s		- Symbol to get full name of
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: Name of field - allocated memory caller needs to free
 */
char    *fieldName( fatherName, s )
char	*fatherName;
Symbol              s;
{
    char    *name;
    char    *s_name;

    s_name = symname( s );

    if ( *fatherName == '*' ) {
        if ( *( fatherName + 1 ) == '*' ) {
            name = 
                malloc(( strlen( fatherName ) + strlen( s_name ) + 4 ) *
                         sizeof(char));
            strcpy( name, "(" );
            strcat( name, fatherName + 1 );
            strcat( name, ")" );
        } else {
            name = 
                malloc(( strlen( fatherName ) + strlen( s_name ) + 2 ) *
                          sizeof(char));
            strcpy( name, fatherName + 1 );
        }
        strcat( name, "->" );
        strcat( name, s_name );
    } else {
        name = malloc(( strlen( fatherName ) + strlen( s_name ) + 2 ) *
                        sizeof(char));
        strcpy( name, fatherName );
        strcat( name, "." );
        strcat( name, s_name );
    }
    return name;
}


/*
 *  this routine returns the name of a VarStruct whose father VarStruct
 *  is a pointer (PTR).
 */
char    *targetName( fatherName )
char	*fatherName;
{
    char    *name;

    name = malloc(( strlen( fatherName ) + 2 ) * sizeof(char));
    strcpy( name, "*" );
    strcat( name, fatherName );
    return name;
}
                
/*
 *  this routine determines the least significant field portion
 *  of the current element of the given VarStruct
 *  Returns  -1 (failure)  0 (success)
 */

int dpi_current_array_element_name( var, suffix )
struct VarStruct    *var;
char        	    **suffix;
{
    char    *elemName;
    char    *lastper;
    char    *lastarr;

    if ( var->chain == nil )
        return -1;
    elemName = ( *var->chain )->name;

    /* Strip off parent (RECORD) portion of name, if applicable */

    lastper = strrchr( elemName, '.' );
    lastarr = strrchr( elemName, '>' );
    if ( lastper || lastarr )
        *suffix = lastper > lastarr ? lastper + 1 : lastarr + 1;
    else
        *suffix = elemName;
    return 0;
}

/*
 *  this routine returns the name of an array VarStruct as shown
 *  in the VarStruct's decl, ie the name of the array concatenated
 *  with the dimensions of its subscripts
 *  Returns  -1 (failure)  0 (success)
 */

int dpi_max_array_element_name( var, elemName )
struct VarStruct    *var;
char                **elemName;
{
    char    *nameptr;
    char    *nlptr;
    char    *whitespace;
    struct Range   **rnge;
    struct Range   *rng;

    if ( var->class != V_ARRAY && var->class != V_MOD_ARRAY )
        return -1;

    if ( dpi_if_cobol( var->symbol ) ) {
	/* find the number of dimensions, and mix them with the name of the
	   variable to form basename for arrays. It will be something like
	   this.

		name(1,1,1)
	*/
	int i = 0;
 	char * name;

	/*
         * Concatenate upper range dimensions onto variable array name (Cobol)
         */
	rnge = var->range;
	rng = *var->range;
	name = (char *) malloc( (unsigned)(strlen(var->name)+20) );
	strcpy( name, var->name );
	strcat( name, "(" );
	while ( rnge[i] ) {
		sprintf(name, "%s%d", name, rnge[i]->upper);
		i++;
		if ( rnge[i] ) {
			 strcat(name, ", ");
		}
	}
	strcat( name, ")\n");
	*elemName = name;
	return 0;
    }

    /*
     * The declaration will be of the form:  type  variable subscript \n
     * where type may be more than one line long.  We want to return only
     * the name of the variable and the subscripts
     * 
     * First find the last line of the declaration
     */
    nlptr = strrchr( var->decl, '\n' );
    if( nlptr ) {
	*nlptr = nil;		    	      /* strip final newline */
    }

    nameptr = strrchr( var->decl, '\n' );
    if( !nameptr ) {
	nameptr = var->decl;
    }

    /*
     * In case the type information is on the last line, skip to the
     * last blank space
     */
    whitespace = strrchr( nameptr, ' ' );
    if( whitespace ) {
	nameptr = whitespace;
    }

    /*
     * Get the name of the variable with the maximum number of elements
     */
    nameptr = strstr( nameptr, var->basename );   

    if ( !nameptr )
	return -1;

    /*
     * Copy the name and subscripts into elemName to be returned
     */
    *elemName = malloc(( strlen( nameptr ) + 1 ) * sizeof(char));
    strcpy( *elemName, nameptr );

    /*
     * Replace the newline
     */
    if( nlptr ) *nlptr = '\n';
    return 0;
}
    
/*
 *  given an array VarStruct and number (offset), this routine returns the
 *  subscript represented by adding the number to the array range base element.
 *  The subscripts are returned in an array of integers (subscripts).
 *  Returns -1 (failure)  0 (success)
 */

int dpi_array_element_subscripts( var, offset, subscripts )
struct VarStruct    *var;
int                 offset;
int                 **subscripts;
{
    struct Range    **rangePtr;
    int             *subscrPtr;
    int             num, factor, range, quot, numDims;
    int		    *excess, *excessPtr;
    Boolean	    unsafebounds_set;

    unsafebounds_set = varIsSet( "$unsafebounds" );

    /*   calc number of dimensions and point to rightmost dimension   */

    rangePtr = var->range;
    numDims = 0;
    while ( *rangePtr++ )
        numDims++;
    *subscripts = ( int    * )calloc( numDims + 1, sizeof( int ) );
    subscrPtr = *subscripts;
    rangePtr--;

    if( offset == 0 ) {
	/*
	 * Just return the bases for each dimension
	 */
	rangePtr = var->range;
	while( *rangePtr ) {
	    *subscrPtr = (*rangePtr)->base;
	    rangePtr++;
	    subscrPtr++;
	}
	*subscrPtr = -1;
	return 0;
    }

    /*
     * Allocate space to hold excess for each dimension of the array
     * Point to the last item
     */
    excess = ( int * )calloc( numDims + 1, sizeof( int ) );
    excessPtr = excess + numDims - 1;

    /*   if fortran, reverse order of range   */
    if ( reverseSubscripts )
	reverseRange( var->range, numDims );

    factor = 1;
    range = 1;
    num = 0;
    while ( rangePtr-- != var->range ) {
        factor *= range;
	if( (*rangePtr)->base > (*rangePtr)->upper ) {
	    /*
	     * If the subscript starts outside of the range of the array set
	     * excess to the number of elements outside of the range for this
	     * dimension
	     */
	    *excessPtr = (*rangePtr)->base - (*rangePtr)->upper;
	    num += factor * ( (*rangePtr)->upper - (*rangePtr)->lower );
	} else {
	    num += factor * ( (*rangePtr)->base - (*rangePtr)->lower );
	}
	excessPtr--;
        range = (*rangePtr)->upper - (*rangePtr)->lower + 1;
    }
    factor *= range;   /* number of elements in array */
    if( ( num += offset ) >= factor && !unsafebounds_set ) {

    	/*   if fortran, reverse back order of range   */

    	if ( reverseSubscripts )
	    reverseRange( var->range, numDims );

        return -1;
    }

    if( num >= factor ) {
	/*
	 * If there are more elements to get than will be held in the array set
	 * excess for last dimension to the number of elements past the end;
	 * Reset num to the number that can actually be retrieved from the array
	 */
	*(excess + numDims - 1) += num - factor + 1;
	num = factor - 1;
    }

    /*   calculate subscripts of new element   */
    if( unsafebounds_set ) excessPtr = excess;
    while ( *++rangePtr ) {
        range = (*rangePtr)->upper - (*rangePtr)->lower + 1;
        factor /= range;
        quot = num / factor;
	if( unsafebounds_set ) {
	    *subscrPtr++ = (*rangePtr)->lower + quot + *excessPtr++;
	} else {
	    *subscrPtr++ = (*rangePtr)->lower + quot;
	}
        num -= factor * quot;
    }    

    /*   if fortran, reverse order of subscripts   */

    if ( reverseSubscripts )
	reverseIntArray( *subscripts, numDims );

    *subscrPtr = -1;   /* end of subscript array */

    /*   if fortran, reverse back order of range   */

    if ( reverseSubscripts )
	reverseRange( var->range, numDims );

    free( excess );
    return 0;
}

/*
 *  return the number of lines in a character string.
 */
    
int        declLines( decl )
char    *decl;
{
    char    *cptr;
    int	    num;

    num = 0;
    cptr = decl - 1;
    while ( cptr = strchr( cptr + 1, '\n' ) )
        num++;
    return num;
}


/*
 *  abstraction onto declLines().
 */
    
int  dpi_decl_lines( decl )
char   *decl;
{
    return( declLines( decl ));
}


/*
 * NAME: dpi_variable_offset
 *
 * FUNCTION: Adds offset to name field of VarStruct whose parent is a pointer or
 *	tag whose parent is a pointer.  This name change is propagated down to
 *	any underlying VarStructs.
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES: 
 *  For example:
 *	for offset of 3
 *  if mode is PTR_MODE
 *      *a becomes *(a+3)
 *  else if mode is ARRAY_MODE
 *      a[0] becomes a[3]
 *
 * PARAMETERS:
 *	var	- Variable to change name of
 *	mode	- Format of name change
 *	offset	- Offset to change to
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: 
 *	-1:	failure
 *	 0:	success
 */
int dpi_variable_offset( var, mode, offset )
struct VarStruct  *var;
OffsetType	  mode;
int		  offset;
{
    int	    magn;
    int     numdigits;
    struct VarStruct    **fields;
    Symbol  t;
    char    *name;

    /*   get proper base name   */

    if ( var->parent->class == V_PTR )
	name = var->parent->name;
    else if ( ( var->parent->class == V_TAG ) 
	&& ( var->parent->parent->class == V_PTR ) )
    	name = var->parent->parent->name;
    else
	return 0;

    /*   calculate number of digits in offset   */

    numdigits = 1;
    if( offset < 0 ) {
	/*
	 * Add a digit for the negative sign, and set magn to the absolute
	 * value of the offset to find out how many digits are in the offset
	 */
	numdigits++;
	if( offset == INT_MIN ) {
	    magn = INT_MAX;
	} else {
	    magn = -offset;
	}
    } else {
	magn = offset;
    }
    while ( magn > 0 ) {
	numdigits++;
	magn /= 10;
    }

    if ( mode == PTR_MODE ) {
	dispose( var->name );
	if ( offset == 0 ) {
	    var->name = malloc(( strlen( name ) + 2 ) * sizeof(char));
	    sprintf( var->name, "*%s", name );   
	} else {
	    var->name = malloc(( strlen( name ) + numdigits + 7 ) *
                                 sizeof(char));
	    sprintf( var->name, "*(%s + %d)", name, offset );   
	}

    /*   ARRAY_MODE   */

    } else if ( *name == '*' ) {
	dispose( var->name );
	var->name = malloc(( strlen( name ) + numdigits + 5 ) * sizeof(char));
	sprintf( var->name, "(%s)[%d]", name, offset );
    } else {
	dispose( var->name );
	var->name = malloc(( strlen( name ) + numdigits + 3 ) * sizeof(char));
	sprintf( var->name, "%s[%d]", name, offset );
    }

    if ( var->chain )

	/*  proceed down VarStruct chain  */

        if( var->class == V_RECORD ||
	    var->class == V_UNION ||
	    var->class == V_CLASS ) {
    	    fields = var->chain;
    	    t = var->key;
       	    while ( *fields ) {
	        t = t->chain;
	        if ( propagate_offset( var->name, t, *fields ) == -1 )
		    return -1;
                fields++;
	    }
        } else if ( var->class == V_PTR ) {
	    if ( propagate_offset( var->name, 0, *var->chain ) == -1 )
	        return -1;
        }
    return 0;
}	


/*
 * NAME: dpi_get_var_offset
 *
 * FUNCTION: Returns the current offset for the specified variable
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES:
 *	1) If variable name starts with '*' then offset is zero.
 *	2) If variable name ends with ']' and does not start with '*' then
 *	   offset is the value between the last two '[' and ']'
 *	3) Otherwise offset is zero.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * TESTING:
 *	1) Test all sorts of different variables
 *
 * RETURNS: The current offset of the specified variable
 */
int dpi_get_var_offset(var)
struct VarStruct *var;
{
    char	*offset, *offset_no;
    int		length;

    if( *var->name == '*' ) {
	/*
	 * Offset is 0
	 */
	return 0;
    } else if( *(var->name + strlen( var->name ) - 1) == ']' ) {
	/*
	 * Offset is specified in array bound, return this number as an integer
	 */
	offset = strrchr( var->name, '[' );
	offset++;
	length = strlen( offset );
	offset_no = malloc( length * sizeof(char));
	strncpy( offset_no, offset, length - 1 );
	offset_no[length - 1] = '\0';
	return( atoi( offset_no ) );
    } else {
	/*
	 * No offset specified, therefore offset is 0
	 */
	return 0;
    }
}


/*
 * NAME: propagate_offset
 *
 * FUNCTION: Replaces the name field of a VarStruct with a name field derived
 *	from its parent's name field.  It then passes this name down to its
 *	progeny by calling itself.
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES:
 *	For example: given a name of "px_ptr[1]", and var->name "**px_ptr", and
 *	var->parent->name "px_ptr[1]", change var->name to "*px_ptr[1]".
 *
 * PARAMETERS:
 *	name	- Not Used
 *	key	- Symbol for variable
 *	var	- Variable to modify
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: 
 *	 0:	Success
 *	-1:	Failure
 */
int propagate_offset( name, key, var )
char		  *name;    /* not used */
Symbol		  key;
struct VarStruct  *var;
{
    char    *arrayElemName( ), *targetName( ), *fieldName( );
    int     upperclass;
    struct VarStruct    **fields;
    Symbol  t;

    upperclass = var->parent->class;
    if ( upperclass == V_ARRAY ) {
        var->name = arrayElemName( var->parent->name, var->parent->range );
    } else if( upperclass == V_CLASS && key->class == BASECLASS ) {
        var->name = className( var->parent->name, key );
    } else if( upperclass == V_RECORD || upperclass == V_UNION ||
	       upperclass == V_CLASS ) {
        var->name = fieldName( var->parent->name, key );
    } else if ( upperclass == V_PTR ) {
        var->name = targetName( var->parent->name );
    } else if ( upperclass == V_TAG ) {
	var->name = malloc( strlen( var->parent->name ) + 1 );
	strcpy( var->name, var->parent->name );
    } else {
        var->name = malloc( strlen( symname( key ) ) + 1 );
        strcpy( var->name, symname( key ) );
    }
    if ( var->chain )

	/*  proceed down VarStruct chain  */

    	if( var->class == V_RECORD ||
	    var->class == V_UNION ||
	    var->class == V_CLASS ) {
    	    fields = var->chain;
    	    t = ( Symbol )var->key;
       	    while ( *fields ) {
		t = ( Symbol )t->chain;
	        if ( propagate_offset( var->name, t, *fields ) == -1 ) {
		    return -1;
	        }
                fields++;
	    }
        } else if ( var->class == V_PTR || var->class == V_ARRAY 
	    || var->class == V_TAG ) {
	    if ( propagate_offset( var->name, 0, *var->chain ) == -1 ) {
	        return -1;
	    }
        }
    return 0;
} 

/*
 *  set the format field of a VarStruct
 *  Always returns 0.
 */

dpi_set_variable_format( var, format )
struct VarStruct  *var;
FormatMode	format;
{
    var->format = format;
    return 0;
}

/*
 *  given an integer array return a subscript string containing
 *  the integers as subscripts.
 *  For example (in C): given the integer array 'elements': "4,1,4,1",
 *  and 'number': 4 (dimensions), return in 'string' "[4][1][4][1]".
 *  Returns -1 (failure)  0 (success)
 */

int subscriptString( elements, number, string )
int	*elements;
int	number;
char	**string;
{
    char	charr[80];
    int		i;
    char	*ssptr;

    if ( number < 1 )
	return -1;
    ssptr = charr;
    sprintf( ssptr, "%s%d\0", beg_delim, elements[0] );
    for ( i = 1; i < number; i++ ) {
	ssptr += strlen( ssptr );
	sprintf( ssptr, "%s%d\0", mid_delim, elements[i] );
    }
    ssptr += strlen( ssptr );
    sprintf( ssptr, "%s\0", end_delim );
    *string = malloc(( strlen( charr ) + 1 ) * sizeof(char));
    strcpy( *string, charr );
    return 0;
}


/*
 *  free the space for a chain of VarStructs
 */

void    free_chain( chain )
struct VarStruct  **chain;
{
    struct VarStruct  **fields;

    fields = chain;
    while ( *fields )
        dpi_free_variable( *fields++ );
    dispose( chain );
}

/*
 *  free the space for a range of a VarStruct
 */

void    free_range( range )
struct Range  **range;
{
    struct Range  **fields;

    fields = range;
    while ( *fields ) {
        dispose( *fields );
        fields++;
    }
    dispose( range );
}


/*
 *  free the space for a VarStruct.
 *  This includes internal character strings (name, basename, decl),
 *  the range structure (for ARRAYS), and chain var structs (children).
 *  Always returns 0.
 */

int	dpi_free_variable( var )
struct VarStruct  *var;
{
    if ( var->chain )
	free_chain( var->chain );
    if ( var->range )
	free_range( var->range );
    dispose( var->name );
    dispose( var->basename );
    dispose( var->decl );
    dispose( var );
    return 0;
}


/*
 * NAME: varClass
 *
 * FUNCTION: Translate a class for a dbx Symbol into a
 *           into a VarStruct class
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * PARAMETERS:
 *      symbol  - dbx Symbol for which class is to be translated
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: VarClass translated from class of dbx Symbol
 */
VarClass  varClass( symbol)
Symbol symbol;
{
    switch ( symbol->class ) {
#ifdef TYPE
/*
 * There is a '#define TYPE(x) x->type' in ldfcn.h,
 * when the 'case TYPE:' is preprocessed
 * the preprocessor substitutes it for: 'case ->type'
 */
#undef TYPE
#endif
    case  TYPE:
          if (isfortranderived(symbol))
          {
             return V_TAG;
          }
          else
          {
             return V_TYPE;
          }
    case  CHARSPLAT:
    case  FSTRING:
    case  PIC:
    case  RPIC:
    case  FPTR:
    case  FPTEE:
    case  CPPREF:
	return V_TYPE;
    case  RECORD:
    case  PACKRECORD:
	return V_RECORD;
    case  CLASS:
    case BASECLASS:
	return V_CLASS;
    case  SCAL:
    case  PTRTOMEM:
	return V_SCAL;
    case  TAG:
	return V_TAG;
    case  VARNT:
    case  UNION:
	return V_UNION;
    case  ARRAY:
	return V_ARRAY;
    case  PTR:
	return V_PTR;
    case  FFUNC:
	return V_FFUNC;
    case  GROUP:
        return V_GROUP;
    case  RGROUP:
        return V_RGROUP;
    case  CONST:
        return V_CONSTANT;
    default:
	return V_UNDEFINED;
    }
}


/* 
   FUNCTION:
	   This function checks the dbx's command line arguments
	   by calling the dbx interface routine dpi_scanargs. If the
	   dbx's command line arguments are missing then this function returns
	   -1.

	   This function also checks for the valid executable by looking
 	   at the magic number. If the file is a directory/nonexecutable
	   then also it returns -1.

*/
		
int dpi_find_externals(argc, argv, rep_out, rep_err, attachflag)
int argc;
char *argv[];
#ifdef _NO_PROTO
int (*rep_out)();
int (*rep_err)();
#else
int (*rep_out)( FILE *, const char *, ...);
int (*rep_err)( FILE *, const char *, ...);
#endif
int *attachflag;
{
	char * filename;
	extern char * objname;
	extern boolean attach;

 	rpt_output = rep_out;
	rpt_error  = rep_err;
	*attachflag = 0 ;
	/* Scan the dbx arguments */
	if ( dpi_scanargs(argc, argv) == -1 )
		return -1;
	*attachflag = attach;
	filename = objname;
	aout = ldopen (filename, (LDFILE *) 0);

	/* Read the file hdr and check for the valid executable by looking
	   at the magic number. */
	if ( ldfhread (aout, &filhdr) == SUCCESS ) {
	    if ( filhdr.f_magic != U802WRMAGIC && 
		 filhdr.f_magic != U802ROMAGIC &&
		 filhdr.f_magic != U802TOCMAGIC ) {
		return -1;
	    }
	}
	else
	    return -1;
	ldclose(aout);
	return 0;
}


int dpi_file_cmp(a, b)
String *a;
String *b;
{
     return( strcoll( *a, *b ) );
}


/*
 * NAME: get_include_files
 *
 * FUNCTION: Add all include files at this level into the file list
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES:
 *	1) Add the passed in include file into the file list
 *	2) Recursively call get_include_files() to add all files included by
 *	   this file
 *	3) Add other include files and those they include that are at the same
 *	   level as the passed in include file.
 *
 * PARAMETERS:
 *	filetab_list	- List to add the include files into; Will be modified
 *	files_in_list	- Count of number of files in filetab_list; Will be
 *			  modified
 *	space_in_list	- Size of current list; May be modified
 *	top_include	- Include file to start adding into the list
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * TESTING:
 *
 * RETURNS: NONE
 */
void get_include_files(filetab_list, files_in_list, space_in_list, top_include)
String **filetab_list;
int *files_in_list;
int *space_in_list;
Incfile *top_include;
{
    Incfile	*next_include;
    char	*basename;

    /*
     * Add all the include files at this level into the file list
     */
    next_include = top_include;

    do {
	if( *files_in_list >= *space_in_list ) {
	    *space_in_list += AVG_NO_FILES;
	    *filetab_list = (String *) realloc( *filetab_list,
				     *space_in_list *sizeof( String ));
	}
	/*
	 * Add the include file into the list; Strip off any directory
	 * information, we just want the basename
	 */
	basename = strrchr( next_include->filename, '/' );
	if( basename == NULL ) {
	    basename = next_include->filename;
	} else {
	    basename++;
	}
	(*filetab_list)[*files_in_list] = basename;
	*files_in_list += 1;

	if( next_include->incl_child ) {
	    /*
	     * Recursively call this function to get all files included
	     * by this file
	     */
	    get_include_files( filetab_list, files_in_list, space_in_list,
			       next_include->incl_child );
	}

	/*
	 * Go to the next include file at this same level
	 */
	next_include = next_include->incl_chain;
    } while( next_include );
}


/*
 * NAME: dpi_get_files
 *
 * FUNCTION: Gets source files known to dbx from current loaded program
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES:
 *	1) Create a list of all file table pointers to real files
 *		1a) If all is set, then this will be a list of all files
 *		1b) If all is not set, then this will be a list of only
 *		    debuggable files
 *	2) Sort the list
 *	3) Remove any duplicate entries in the list
 *	4) Create a newline separated string of the remaining filenames
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * TESTING:
 *	1) No duplicate filenames listed
 *	2) List should be sorted
 *
 * RETURNS:
 *	 0	- On successful completion
 *	-1	- If an error occurs
 */
int dpi_get_files(list, all)
char **list;
int all;
{
    Filetab	*ftp;
    String	*filetab_list;
    int		space_in_list = AVG_NO_FILES;
    int		files_in_list, index;
    int		length = 0;
    char	*marker;
    char        *basename;

    filetab_list = (String *) malloc( space_in_list * sizeof( String ) );
    filetab_list[0] = NULL;
    if( all ) {
	/*
	 * Create a list of file table pointers to all files
	 */
	for( index = 0, files_in_list = 0; index < loadcnt; index++) {
	    for( ftp = filetab[index];
		 ftp < &filetab[index][nlhdr[index].nfiles]; ftp++) {
		if( ftp ) {
		    if( files_in_list >= space_in_list ) {
			space_in_list += AVG_NO_FILES;
			filetab_list = (String *) realloc( filetab_list,
					space_in_list * sizeof(String));
		    }
                    /* find pointer to basename of file name */
                    basename = strrchr( ftp->filename,'/' );
                    if (basename == NULL)
                    {
		       filetab_list[files_in_list] = ftp->filename;
                    }
                    else
                    {
                       filetab_list[files_in_list] = basename+1;
                    }
		    files_in_list++;
		    if( ftp->incl_chain ) {
			/*
			 * This file has include files with executable source in
			 * it, add the include files to the list
			 */
			get_include_files( &filetab_list, &files_in_list,
					   &space_in_list, ftp->incl_chain );
		    }
		}
	    }
	}
    } else {
	/*
	 * Create a list of file table pointers to debuggable files
	 */
	for( index = 0, files_in_list = 0; index < loadcnt; index++) {
	    for( ftp = filetab[index];
		 ftp < &filetab[index][nlhdr[index].nfiles]; ftp++) {
		if( ftp && ftp->lineptr ) {
		    if( files_in_list >= space_in_list ) {
			space_in_list += AVG_NO_FILES;
			filetab_list = (String *) realloc( filetab_list,
					space_in_list * sizeof(String));
		    }
                    /* find pointer to basename of file name */
                    basename = strrchr( ftp->filename,'/' );
                    if (basename == NULL)
                    {
		       filetab_list[files_in_list] = ftp->filename;
                    }
                    else
                    {
                       filetab_list[files_in_list] = basename+1;
                    }
		    files_in_list++;

		    if( ftp->incl_chain ) {
			/*
			 * This file has include files with executable source in
			 * it, add the include files to the list
			 */
			get_include_files( &filetab_list, &files_in_list,
					   &space_in_list, ftp->incl_chain );
		    }
		}
	    }
	}
    }

    /*
     * Sort the list
     */
    qsort( filetab_list, files_in_list, sizeof( String ), dpi_file_cmp );

    /*
     * Remove any duplicate entries in the list
     */

    for( index = files_in_list - 1; index > 0; index-- ) {
	if( strcmp( *(filetab_list + index),
		    *(filetab_list + index - 1)) == 0 ) {
	    filetab_list[index] = NULL;
	} else {
	    length += strlen( filetab_list[index]) + 1;
	}
    }

    /*
     * Since the above loop won't add the length of filetab_list[0], we need
     * to add it here, filetab_list[1] was already checked to see if it was a
     * duplicate, so we don't need to check here.
     */
    length += strlen( filetab_list[0] ) + 1;

    /*
     * Create a newline separated string of the remaining filenames
     */
    *list = (char *)malloc(length + 1);
    if (*list == NULL || length == 0)
	return(-1);
    **list = '\0';
    marker = *list;

    for (index = 0; index < files_in_list; index++) {
	if( filetab_list[index] ) {
	    strcat(marker, filetab_list[index]);
	    marker += strlen(marker);
	    strcpy(marker, "\n");
	    marker += strlen(marker);
	}
    }
    free( filetab_list );

    if( marker != *list ) {
	/*
	 * Clear off trailing newline
	 */
	*(marker - 1) = '\0';
    }
    return 0;
}


/* 
 * Dispose of the strings containing dbx 'use' path information
 */

void free_sourcepath()
{
	ListItem back, forw;

	if ( sourcepath )
		back = sourcepath->head;
	else
		return;
	forw = back;
	while ( forw != (ListItem) nil ) {
		forw = back->next;
		dispose( back );
		back = forw;
	}
	dispose( sourcepath );
}

/*
 *	Process all the relevent COBOL types, and create a valid COBOL
 *	VarStruct.
 *      Returns:  always 0.
*/
cobol_varDecl(s, stype, var)
Symbol s;
Symbol stype;
struct VarStruct  *var;
{
	char	*newstring;

	/* Process COBOL variables that have multiple levels */
	if ( stype->class == GROUP || stype->class == RGROUP ) {
        	msgbegin;
                if (lazy)
                    touch_sym(s);
        	printdecl(s);
        	msgcopyend(var->decl);
		var->index = declLines(var->decl) - 1;
	} else {
		/* process all COBOL variables that have only one level */
        	msgbegin;
                if (lazy)
                    touch_sym(s);
        	printdecl(s);
        	msgcopyend(var->decl);

		/*
 		 * Check if the variable has an unknown type and change the
		 * declaration if it does
		 */
		if( *var->decl == '&' ) {
		    newstring = malloc((15 + strlen(var->name)) * sizeof(char));
		    sprintf( newstring, "Unknown Type %s\n", var->name );
		    dispose( var->decl );
		    var->decl = newstring;
		}
	}
	return 0;
}

/* COBOL stub for function pointer */

cobol_changeName()
{
        return 0;
}

/*
 * Creates the varStruct for variables of type ARRAY in COBOL.
 * Algorithmically, this is almost identical to var_ArrayChain(),
 * which creates an array varStruct for other languages, except for
 * the Symbol argument to getbound(), and the array range structure 
 * is initialized in the reverse order of the other languages.
 * Returns -1 (failure) 0 (success)
 */

int  create_varStruct_for_cobol_arrays(sym, varaddr)
Symbol			sym;
struct VarStruct	**varaddr;
{
	Symbol		    p;
	int		    dims, i;
	Range		    **ranges;
	struct VarStruct    *var;
    	struct VarStruct    **endchain;

	/* 
         * Allocate and initialize the range structure.
         */
	var = *varaddr;
	var->class = V_ARRAY;
	dims = find_dimension_of_cobol_arrays( sym );
	var->range = (Range **) calloc( dims+1, sizeof(Range *) );
	ranges = var->range;
	ranges[dims] = (Range *)nil;
	p = sym;
	i = dims - 1;
	/* Set the range of the VarStruct */
        while ( p ) {
	  if ( p->type->class == ARRAY ) {
		if ( i < 0 ) {
		   	error("internal error: dimensions are incorrect\n");
			break;
		}
		ranges[i] = ( Range * )malloc( sizeof( Range ) );
		if ( p->type->chain->symvalue.rangev.lowertype )
		   getbound(p, p->type->chain->symvalue.rangev.lower,
			p->type->chain->symvalue.rangev.lowertype, 
			&(ranges[i]->lower) );
		else
		   ranges[i]->lower = p->type->chain->symvalue.rangev.lower;
		if ( p->type->chain->symvalue.rangev.uppertype )
		   getbound(p, p->type->chain->symvalue.rangev.upper,
			p->type->chain->symvalue.rangev.uppertype, 
			&(ranges[i]->upper) );
		else
		   ranges[i]->upper = p->type->chain->symvalue.rangev.upper;
		ranges[i]->base  = ranges[i]->lower;
		i--;
	  }
	  p = p->symvalue.field.parent;
        }

	/*
         * Allocate and initialize the element varStruct off the ARRAY variable.
         */
	var->chain = (struct VarStruct **)calloc(2, sizeof(struct VarStruct *));
	*var->chain = (struct VarStruct *)malloc(sizeof(struct VarStruct ));
	if ( var_element(sym, sym->type->type, var, false, *var->chain,
		UNKNOWN_LIST) )
		return -1;

	var->index = (*var->chain)->index;
	endchain = var->chain;
	*++endchain = nil;	/* nil end of VarStruct array */
        return 0;
}

/*
 * Boolean check for COBOL language 
 * Returns 1 (if symbol is cobol symbol), 0 (if symbol is not cobol symbol)
 */

dpi_if_cobol( sym )
Symbol sym;
{
        if ( !sym )
            return 0;

	if( islang_cobol(sym->language) )
		return 1;
	else
		return 0;
}


/*
 * NAME: dpi_get_lang
 *
 * FUNCTION: Returns the language for the specified variable
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES:
 *	1) Checks the name of the language for the variable,
 *	   and returns the specified language.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * TESTING:
 *	1) Check C language variable
 *	2) Check fortran language variable
 *	3) Check cobol language variable
 *	4) Check unsupported language
 *
 * RETURNS: Returns the language
 */
int dpi_get_lang(variable)
Symbol variable;
{
    if( !variable ) return UNSUPPORTED_LANG;

    if ( variable->language == cLang ) 
	return C_LANG;
    else if ( variable->language == cppLang ) 
	return CPLUSPLUS_LANG;
    else if ( variable->language == fLang ) 
	return FORTRAN_LANG;
    else if ( variable->language == cobLang ) 
	return COBOL_LANG;
    else
	return UNSUPPORTED_LANG;
}
