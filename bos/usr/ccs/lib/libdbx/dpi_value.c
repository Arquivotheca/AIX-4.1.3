static char sccsid[] = "@(#)26    1.24.2.3  src/bos/usr/ccs/lib/libdbx/dpi_value.c, libdbx, bos411, 9428A410j 9/14/93 11:51:49";
/*
 *   COMPONENT_NAME: LIBDBX
 *
 *   FUNCTIONS: dpi_set_var_path
 *		add_cobol_subscripts
 *		dpi_fix_var_path
 *		send_command
 *		set_format
 *		unset_format
 *		get_dbx_result
 *		reverseRange
 *		dpi_get_symname
 *		dpi_variable_address
 *		dpi_setcase
 *		dpi_variable_value
 *		getArrayValues
 *		rangeString
 *		arrayString
 *		totalArrayString
 *		dpi_array_subscripts_values
 *		dpi_release_array
 *		reverseIntArray
 *		dpi_report_newProcessId
 *		dpi_get_ident
 *		dpi_convert_case
 *		dpi_hidden
 *
 *   ORIGINS: 27, 83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1991
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/


#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
#include <setjmp.h>
#include <sys/termio.h>
#ifdef KDBX
#include <stdlib.h>
#endif /* KDBX */

#include    "defs.h"
#include    "envdefs.h"
#include    "decode.h"
#include    "mappings.h"
#include    "names.h"
#include    "symbols.h"
#include    "process.h"
#include    "scanner.h"

#ifdef _NO_PROTO
extern void 	*realloc();
extern char	*strchr();
extern char 	*strtok();
extern char	*strrchr();
#else
extern void 	*realloc(void *ptr, size_t size);
extern char	*strchr(const char *s, int c);
extern char 	*strtok(char *s1, const char *s2);
extern char	*strrchr(const char *s, int c);
#endif

#define MORE_MEMORY	30
#define MAX_INT_DIGIT_LEN	12

extern int	*envptr;            /* setjmp/longjmp data */
extern Address	assign_addr;	    /* last address assigned to by user */

char	command[MAXLINESIZE];

boolean unsetFormat;		/* Indicates the format has been changed     */
boolean changeBackFormat;	/* Indicates the new format overrode another */
boolean reverseSubscripts;	/* Setup in var_lang_init; Indicates subscripts
				 * evaluated in reverse order */

/*
 * dpi_set_var_path:
 * Dummy routine, not used anymore, but it needs to stay around for
 * compatibility
 */
int dpi_set_var_path(mode)
int mode;
{
}


/*
 * NAME: add_cobol_subscripts
 *
 * FUNCTION: Adds subscripts to a COBOL variable name
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES: 
 *
 * PARAMETERS:
 *	vardecl	- VarStruct describing COBOL variable
 *	newvar	- name to be changed
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
static void add_cobol_subscripts( vardecl, newvar )
struct VarStruct *vardecl;
char **newvar;
{
    struct VarStruct	*dad;
    Range		**rnge = NULL;
    char		*open_paren;
    char		*subscripts = NULL;
    int			i;
    unsigned int	len;	/* Current amount of allocated memory for
				 * subscripts
				 */
    unsigned int	len_subscripts;	/* Current number of characters written
					 * into subscripts
					 */

    if ( vardecl->class != V_ARRAY ) {
	if( find_dimension_of_cobol_arrays( vardecl->symbol )  ||
	    ( vardecl->parent &&
	      ( vardecl->parent->class == V_MOD_ARRAY ||
		vardecl->parent->class == V_GROUP ))) {
	    /*
	     * Find the dimension of the cobol array variables, and then make a
	     * string of which dimension needs to be printed out.
	     */
	    dad = vardecl;
	    rnge = NULL;
	    while ( dad ) {
		if ( dad->class == V_ARRAY || dad->class == V_MOD_ARRAY ) {
		    rnge = dad->range;
		    break;
		} else
		    dad = dad->parent;
	    }
	}
    } else if( vardecl->parent->class == V_GROUP ||
	       vardecl->chain[0]->class == V_TYPE ) {
	/*
	 * This is the special case 01 arry occurs 7 times pic xx.
	 */
	rnge = vardecl->range;
    }
    i = 0;
    if ( rnge ) {
	len = MORE_MEMORY;
	subscripts = malloc (len * sizeof(char));
	strcpy(subscripts, "(");
	len_subscripts = 2;	/* One for the '(' and one for the NULL */
	while ( rnge[i] ) {
	    len_subscripts += sprintf(subscripts, "%s%d", subscripts,
				      rnge[i]->base);
	    if( len_subscripts + 1 + MAX_INT_DIGIT_LEN >= len ) {
		/*
		 * Check if the ',' and the maximum length of an integer will
		 * exceed the allocated length of subscripts.  If so, need to
		 * increase its size
		 */
		len += MORE_MEMORY;
		subscripts = realloc( subscripts, len * sizeof( char ) );
	    }
	    i++;
	    if ( rnge[i] ) {
		strcat(subscripts, "," );
		len_subscripts += 1;
	    }
	}
	strcat( subscripts, ")" );
     
	open_paren = strchr(*newvar, '(');
	if ( open_paren ) {
	    *open_paren = '\0';
	}
	len = strlen(*newvar) + strlen(subscripts) + 1;
	*newvar = realloc(*newvar, len);
	strcat( *newvar, subscripts );
	dispose( subscripts );
    }
    if ( streq( vardecl->name, "FILLER")) {
	*newvar = realloc( *newvar, strlen( *newvar ) +
	strlen( vardecl->parent->name ) + 2 );
	sprintf( *newvar, "%s.%s", vardecl->parent->name, vardecl->name );
    }
}


/* 
 * dpi_fix_var_path:
 * return a new name of the variable including prefix.
 */
char *dpi_fix_var_path(vardecl)
struct VarStruct *vardecl;
{
  char *p, *q;
  Symbol s = NULL;
  char *newvar;

  if ( vardecl->symbol->name ) {
    /*
     * Get the Symbol for variable with this name that is in scope
     */
    Node n = which(vardecl->symbol->name, WOTHER);
    assert(n->op == O_SYM); /* unsure if this is a valid assumption */
    s = n->value.sym;
  }

  /*
   * Construct the variable name with or without scope information
   */
  if ( s && s == vardecl->symbol ) {
    /* Don't need scope information */
    newvar = malloc( (strlen( vardecl->name ) + 1) * sizeof( char ));
    strcpy( newvar, vardecl->name );
  } else {
    /* Need scope information */
    newvar = malloc( (strlen( vardecl->name ) + strlen( vardecl->scope ) + 1)
		     * sizeof( char ));
    for (p=vardecl->name, q=newvar; *p=='*' or *p=='(' or *p=='&'; p++, q++) {
      *q = *p;
    } 
    /*
     * Put the scope information before the name of the variable
     */
    strcpy(q, vardecl->scope);
    strcat(newvar, p);
  }

  /*
   * We're done now with all the languages except cobol.
   */
  if ( islang_cobol( vardecl->symbol->language ) ) {
	add_cobol_subscripts( vardecl, &newvar );
  }
  return newvar;
}


/*
 *  sends command to dbx and returns result in parameter result
 *  Returns: 0 for success; -1 failure
 */
static int	send_command( command, result )
char    *command;
char	**result;
{
    int	    dbx_mask;
    int     rtcode;

    numerrors( );
    msgbegin;

    /*
     * Need to resetinput() to insure that lex buffer is cleared
     */
    resetinput();
    rtcode = dpi_command( command, &dbx_mask );
    if( numerrors() ) {
	/* If any errors occurred return failure */
	rtcode = -1;
    }
    /*
     * Output from the command will be placed in result
     */
    msgend( *result );
    return rtcode;
}


/*
 *  sets format to given mode
 */
static int set_format( mode )
FormatMode	mode;
{
    int		 rtcode = 0;
    char	 *result;

    switch( mode ) {
	case HEXINTS:
	    /* If $hexints is already set then do nothing */
	    if( !varIsSet( "$hexints" ) ) {
		/*
		 * Indicate that we are modifying the format, check if $hexints
		 * is overriding $octints, so it can be reset after this command
		 */
		unsetFormat = true;
		changeBackFormat = varIsSet( "$octints" );
		rtcode = send_command( "set $hexints\n", &result );
		dispose( result );
	    }
	    break;
	case OCTINTS:
	    /* If $octints is already set then do nothing */
	    if( !varIsSet( "$octints" ) ) {
		/*
		 * Indicate that we are modifying the format, check if $octints
		 * is overriding $hexints, so it can be reset after this command
		 */
		unsetFormat = true;
		changeBackFormat = varIsSet( "$hexints" );
		rtcode = send_command( "set $octints\n", &result );
		dispose( result );
	    }
	    break;
	case HEXCHARS:
	    /* If $hexchars is already set then do nothing */
	    if( !varIsSet( "$hexchars" ) ) {
		/*
		 * Indicate we are modifying the format
		 */
		unsetFormat = true;
		rtcode = send_command( "set $hexchars\n", &result );
		dispose( result );
	    }
	    break;
	case HEXSTRINGS:
	    /* If $hexstrings is already set then do nothing */
	    if( !varIsSet( "$hexstrings" ) ) {
		/*
		 * Indicate we are modifying the format
		 */
		unsetFormat = true;
		rtcode = send_command( "set $hexstrings\n", &result );
		dispose( result );
	    }
	    break;
	case HEX:
	    /* If $hexints is already set then don't change it */
	    if( !varIsSet( "$hexints" ) ) {
		/*
		 * Indicate that we are modifying the format, check if $hexints
		 * is overriding $octints, so it can be reset after this command
		 */
		unsetFormat = true;
		changeBackFormat = varIsSet( "$octints" );
		rtcode = send_command( "set $hexints\n", &result );
		dispose( result );
	    }
	    /* If $hexchars is already set then don't change it */
	    if( !varIsSet( "$hexchars" ) ) {
		/*
		 * Indicate that we are modifying the format
		 */
		unsetFormat = true;
		rtcode = send_command( "set $hexchars\n", &result );
		dispose( result );
	    }
	    break;
    }
    return rtcode;
}


/*
 *  unsets format from mode
 *  So if this completes successfully changeBackFormat and unsetFormat
 *  will be False.  Returns -1 if send_command fails otherwise it returns 0.
 */
static int unset_format( mode )
FormatMode	mode;
{
    char	 *result;
    int		 rtcode = 0;

    if ( unsetFormat ) {
	switch( mode ) {
	    case HEXINTS:
		if (changeBackFormat )
		    /* If $hexints overrode $octints set it back */
		    rtcode = send_command( "set $octints\n", &result );
		else
		    rtcode = send_command( "unset $hexints\n", &result );
		dispose( result );
		break;
	    case OCTINTS:
		if (changeBackFormat )
		    /* If $octints overrode $hexints set it back */
		    rtcode = send_command( "set $hexints\n", &result );
		else
		    rtcode = send_command( "unset $octints\n", &result );
		dispose( result );
		break;
	    case HEXCHARS:
		rtcode = send_command( "unset $hexchars\n", &result );
		dispose( result );
		break;
	    case HEXSTRINGS:
		rtcode = send_command( "unset $hexstrings\n", &result );
		dispose( result );
		break;
	    case HEX:
		if( varIsSet( "$hexints" ) ) {
		    rtcode = send_command( "unset $hexints\n", &result );
		    dispose( result );
		}
		if( varIsSet( "$hexchars" ) ) {
		    rtcode = send_command( "unset $hexchars\n", &result );
		    dispose( result );
		}
		if (changeBackFormat )
		    /* If $hexints overrode $octints set it back */
		    rtcode = send_command( "set $octints\n", &result );
		break;
	}
	unsetFormat = false;
	changeBackFormat = false;
    }
    return rtcode;
}
  

/*
 *  sends command to dbx with mode set and returns result in parameter result
 *  Command's case is never converted, it is always sent in mixed case mode.
 *  Return: 0 for success; -1 failure
 */
int     get_dbx_result(command, mode, result)
char *command;
FormatMode mode;
char **result;
{
    cases     savecase = mixed;
    int	      rc = 0;

    if ( set_format( mode ) == -1 )
	return -1;
    /*
     * Check if the case needs to be modified
     */
    if ( symcase != mixed ) {
	savecase = symcase;
	symcase = mixed;
    }
    /*
     * Result will be filled in with the command output
     */
    rc = send_command( command, result );
    if ( savecase != mixed ) symcase = savecase;
    if ( unset_format( mode ) == -1 )
	rc = -1;
    return rc;
}


/*
 *  reverse the pointers to Range structures in a Range array
 */
void reverseRange( array, num )
Range	**array;
int	num;
{
    int	i, j;
    Range *r;

    for ( i = 0, j = num - 1; i < j; i++, j-- ) {
	r = array[i];
	array[i] = array[j];
	array[j] = r;
    }
}


/*
 * NAME: dpi_get_symname
 *
 * FUNCTION: Returns the name of the variable with the base name substituted
 *	with its Symbol address
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES: Only use this function for direct commands to dbx.  The output from
 *	this function should never be seen by the user.
 *
 * PARAMETERS:
 *	vardecl	- VarStruct describing variable
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: Name with symbol address
 */
char *dpi_get_symname(vardecl)
struct VarStruct *vardecl;
{
    int			length, prefix_len;
    char		*newvar;
    char		*name;
    char		*suffix;
    char		*topname;

    /*
     * Get the name of the base variable for this variable.  (In other words if
     * this variable is actually a member of a structure, topname will be the
     * name of the structure.)
     */
    topname = symname( vardecl->symbol );

    /*
     * Find this name in the name of the variable
     */
    name = strstr( vardecl->name, topname );

    if( name != NULL && !islang_cobol( vardecl->symbol->language ) ) {
	/*
	 * Get the length of any characters preceding the base name
	 */
	length = prefix_len = name - vardecl->name;

	/*
	 * Find the part of the name following the base name, and get its length
	 */
	for( suffix = name; *topname != '\0' && suffix != '\0';
	     topname++, suffix++ ) {
	    if( *suffix != *topname ) break;
	}
	length += strlen( suffix );

	/*
	 * Add 10 for ^Z, symbol address, and NULL
	 */
	length += 10;
	newvar = malloc( length * sizeof( char ) );
	strncpy( newvar, vardecl->name, prefix_len );
	sprintf( newvar + prefix_len, "%X%s", vardecl->symbol, suffix );
    } else {
	/*
	 * Just use the variable's name if COBOL or if the base name is not
	 * part of the real name
	 */
	newvar = malloc( (strlen( vardecl->name ) + 1) * sizeof( char ));
	strcpy( newvar, vardecl->name );
    }

    /*
     * We're done now with all the languages except COBOL.
     */
    if ( islang_cobol( vardecl->symbol->language ) ) {
	add_cobol_subscripts( vardecl, &newvar );
    }
    return newvar;
}


/*
 *  this routine returns the offset from the variable in parameter offset, 
 *  represented by the given VarStruct, of the address of the last variable
 *  assigned to by a dbx user ( assign_addr ).  If this value if NOADDR then the
 *  address is not in the scope of the variable.  If this value is -1 then there
 *  has been no assignment.  The assigned-to address must be within the scope of
 *  the variable represented by the VarStruct.
 *  Returns: 0 for success; -1 for failure;
 */

int     dpi_variable_address( var, numElems, offset )
struct VarStruct  *var;
int	numElems;
int	*offset;
{
    Address    addr;
    int	       dim;
    int	       size;
    int	       totalElems;
    char       *result;
    struct Range    **varDims;
    int	       numDims, idim, trueBase, temp_trueBase;
    char *tmpname;

    if ( assign_addr == NOADDR ) {
	*offset = -1;
	return 0;
    }

    switch ( var->class ) {
    case V_PTR:
    case V_SCAL:
    case V_TYPE:
        if ( var->symbol->class == FPTEE ) 
	    *offset = 0;
        else {
    	    /*   get address   */
	    tmpname = dpi_get_symname(var);
            sprintf( command, "print &%s\n", tmpname );
	    dispose(tmpname);
	
	    if ( get_dbx_result( command, HEXINTS, &result )  == -1 )
	        return -1;
	    addr = result ? (Address)strtoul( result, NULL, 16 ) : NOADDR;
            dispose( result );
	    /*
	     * If the address is not the same then this is not within the scope
	     * of the variable
	     */
	    *offset = addr == assign_addr ? 0 : NOADDR;
	}
	break;

    case V_ARRAY:
    case V_MOD_ARRAY:

	/*   get address   */
	tmpname = dpi_get_symname(var);
        sprintf( command, "print &%s\n", tmpname );

	if ( get_dbx_result( command, HEXINTS, &result )  == -1 ) {
	    dispose(tmpname);
	    return -1;
	}
	addr = result ? (Address)strtoul( result, NULL, 16 ) : NOADDR;
        dispose( result );

	/*   get size   */
        sprintf( command, "print sizeof (%s)\n", tmpname );
	dispose(tmpname);

	if ( get_dbx_result( command, HEXINTS, &result )  == -1 )
	    return -1;
	/*
	 * If $hexints/$octints is set this may be in hex/octal
	 */
	size = result ? (Address)strtoul( result, NULL, 16 ) : 0;
        dispose( result );

	/*   memory location within array displaying?   */
	/* Check if address is within scope of specified variable
	 * If $unsafebounds is set then we need to assume the address is within
	 * the range of the array and let the checking below determine if it
	 * really is.
	 */
	if( (assign_addr >= addr && assign_addr < addr + size)
	    || varIsSet( "$unsafebounds" ) ) {
	  if( islang_cobol( var->symbol->language ) &&
	      ( var->class == V_MOD_ARRAY ||
		( var->parent->class == V_GROUP ||
		  var->chain[0]->class == V_TYPE ))) {
	    *offset = assign_addr - addr;
	    *offset = *offset == 0 ? *offset : NOADDR;
	  } else {
	    /* calc total number of elements and trueBase */
	    /* trueBase points at the top element of the display */
	    totalElems = 1;
	    trueBase = 0;
    	    /*   if fortran, reverse order of range   */
    	    if ( reverseSubscripts ) {
	      /* Set numDims to the number of dimensions */
              for ( numDims = 0, varDims = var->range;
		    *varDims; numDims++, varDims++ );
	      reverseRange( var->range, numDims );
	    }
	    for ( dim = 0; var->range[dim]; dim++ ) {
	        totalElems 
		    *= var->range[dim]->upper - var->range[dim]->lower + 1;
		temp_trueBase 
		    = (var->range[dim]->base - var->range[dim]->lower);
		for (idim = dim + 1; var->range[idim]; idim++) {
		   temp_trueBase  
		    *= var->range[idim]->upper - var->range[idim]->lower + 1;
		}
		trueBase += temp_trueBase;
	    }
            /*   if fortran, reverse back order of range   */
            if ( reverseSubscripts )
	      reverseRange( var->range, numDims );
	    /* Determine size of each element */
	    *offset = (( assign_addr - addr ) * totalElems) / size;
	    *offset = ((*offset >= trueBase) &&
                       (*offset <= trueBase + numElems)) ? *offset : NOADDR;
	  }
	} else
	    *offset = NOADDR;
	break;
    default:
	*offset = -1;
	break;
    }
    return 0;
}


/**************************************************************************
* Name: dpi_setcase							  *
*									  *
* Purpose: Changes the case for dbx to parameter newcase, and returns the *
*	   previous case.						  *
* Detail: 								  *
* Returns: Previous case						  *
* Notes: 								  *
**************************************************************************/
cases dpi_setcase(newcase)
cases newcase;
{
  cases savecase;

  savecase = symcase;
  symcase = newcase;
  return savecase;
}


/*
 *  returns value in value parameter of variable represented by name field in
 *  given varStruct in mode of format field
 *  Returns: 0 for success; -1 for failure
 */
int dpi_variable_value(var, value)
struct VarStruct *var;
char **value;
{
    char      *tmpname;

    if ( var->class == V_EXPRESSION ) 
        sprintf( command, "print %s\n", var->name );
    else {
        tmpname = dpi_get_symname(var);
        sprintf( command, "print %s\n", tmpname );
        dispose(tmpname);
    }
    return get_dbx_result( command, var->format, value );
}


/*
 *  given the dbx output from a print subrange command, as a character
 *  string, return two arrays of pointers.  The first array contains
 *  pointers to each of the subscript strings in subs, the second to each of
 *  the value strings in vals.
 *
 *  eg., if the user types "print a[1..2][1..3]"
 *  dbx output might be
 *      "[1][1] = 2
 *       [1][2] = 3
 *       [1][3] = 4
 *       [2][1] = 3
 *       [2][2] = 4
 *       [2][3] = 5
 *      "
 *
 *  this routine would return arrays
 *      ( "[1][1]", "[1][2]", "[1][3]", "[2][1]", "[2][2]", "[2][3]" ) 
 *  and
 *      ( "2", "3", "4", "3", "4", "5" ).
 *
 */
static void     getArrayValues( str, nelem, subs, vals )
char    *str;
int     nelem;
char    ***subs;
char    ***vals;
{
    char    **sptr;
    char    **vptr;
    int     i;

    *subs = ( char ** )malloc( (nelem + 1) * sizeof( char * ) );
    *vals = ( char ** )malloc( (nelem + 1) * sizeof( char * ) );
    sptr = *subs;
    vptr = *vals;
    *sptr++ = strtok( str, "=" );
    *vptr++ = strtok( nil, "\n" );
    for ( i = 1; i < nelem; i++ ) {
        *sptr++ = strtok( nil, "=" );
        *vptr++ = strtok( nil, "\n" );
    }
    *sptr = nil;
    *vptr = nil;
}


/*  return the lower and upper bounds of a range structure in
 *  dbx subrange notation, needs to be freed by caller.  eg, a return value
 *  could be "[0..5][0..4]".
 */
static char    *rangeString( dims )
Range   **dims;
{
    char    brack[(MAX_INT_DIGIT_LEN * 2) + 5];	/* Brack needs to be big enough
						 * to hold two integers, '[',
						 * two '.', ']', and a NULL
						 */
    char    *str;
    int	    len = MORE_MEMORY;	/* Current amount of allocated memory for str */
    int	    len_str = 1;	/* Current number of characters in str */

    str = malloc( MORE_MEMORY * sizeof( char ));
    *str = '\0';
    while ( *dims ) {
        len_str += sprintf( brack, "[%d..%d]", (*dims)->lower, (*dims)->upper );
	if( len_str >= len ) {
	    /* Need to allocate more memory to hold str */
	    len += MORE_MEMORY;
	    str = realloc( str, len );
	}
        strcat( str, brack );
        dims++;
    }
    return str;
}


/*
 *  this routine gets the dbx output from a print subrange command,
 *  as a character string, and appends it to a like string in arrstr.
 *  Returns: 0 for success; -1 for failure
 */
static int arrayString(varname, dims, arrstr)
struct VarStruct *varname;
struct Range **dims;
char **arrstr;
{
    char    *tempstr;
    char    *crptr;
    char    *tmpname;

    /* get subscript and values of range of array elements */

    tmpname = dpi_get_symname(varname);
    tempstr = rangeString( dims );
    if ( *varname->name == '*' ) {
    	sprintf( command, "print (%s)%s\n", tmpname, tempstr );
    } else {
    	sprintf( command, "print %s%s\n", tmpname, tempstr );
    }
    dispose(tmpname);
    dispose(tempstr);

    if ( get_dbx_result( command, varname->format, &tempstr )  == -1 )
	return -1;
    /*
     * If nothing was returned report failure
     */
    if ( tempstr == nil )
	return -1;

    /* remove empty last line */

    crptr = strrchr( tempstr, '\n' );
    if ( !crptr )
      	return -1;
    *crptr = '\0';
    crptr = strrchr( tempstr, '\n' );
    if ( !crptr )
  	*tempstr = '\0';
    else
        *(crptr + 1) = '\0';

    /* append array to range string */

    *arrstr = realloc( *arrstr, strlen( *arrstr ) + strlen( tempstr ) + 1 );
    strcat( *arrstr, tempstr );
    dispose( tempstr );
    return 0;
}

 
/*
 * Return a character string containing the subscripts and values of 
 * consecutive array elements in parameter arrstr beginning with the given array
 * element for the given number of elements. The actual number of values is also
 * returned.
 * 
 * This routine repeatedly fills a Range structure with lower and upper bounds
 * and calls arrayString, which appends a given char string with subscripts and
 * values for these elements.
 * 
 * The reason for repeated calls is to obtain consecutive values of the element
 * array.  For example, for the array arr[10][10][10], to obtain 300 values
 * beginning with arr[2][3][4], one must send the following ranges to 
 * arrayString in Range structure form:
 *
 * 		arr[2..2][3..3][4..9]
 * 		arr[2..2][4..9][0..9]
 * 		arr[3..4][0..9][0..9]
 * 		arr[5..5][0..2][0..9]
 * 		arr[5..5][3..3][0..3]
 *
 * For fortran, the order of subscripts is reversed ( the leftmost loops
 * fastest ).  We therefore reverse the range pointers upon entry in order
 * for the algorithm to work correctly. Before calling arrayString we reverse
 * back the range pointers so that the correct subscripts and values are
 * printed.  This must be reversed after calling arrayString and before
 * exiting this routine.
 *
 * Returns: 0 for success; -1 for failure
 */
static int    	totalArrayString( var, beginElement, numElements, arrstr,
				  numValues )
struct VarStruct	*var;
int        		*beginElement;	/* Array with each beginning element
					 * for each dimension */
int        		numElements;
char    		**arrstr;
int        		*numValues;
{
    int                i;
    int                num;		/* number of remaining elements */
    int                numDims;		/* Number of dimensions */
    int                rng_mult;	/* magnitude of current subscript */
    int                trun_num;	/* range of current subscript */
    boolean            reverse = false; /* reverse direction */
    struct Range    **elemRange;	/* current bounds structure */
    struct Range    **dimptr;		/* points to current subscript
					   of current bounds structure */
    struct Range    **varDims;		/* points to current subscript
					   of given bounds structure */
    if ( var->class != V_ARRAY )
	return -1;

    num = numElements;

    /*   calculate number of dimensions   */

    for ( numDims = 0, varDims = var->range; *varDims; numDims++, varDims++ )
        ;
    varDims--;

    /*   if fortran, reverse order of range   */

    if ( reverseSubscripts )
	reverseRange( var->range, numDims );

    /*   fill range structure with beginning element values   */

    elemRange = ( Range ** )malloc( (numDims + 1) * sizeof( Range * ) );
    dimptr = elemRange;
    for ( i = 0; i < numDims; i++ ) {
        *dimptr = ( Range * )malloc( sizeof( Range ) );
        (*dimptr)->lower = (*dimptr)->upper = (*dimptr)->base = *beginElement++;
        dimptr++;
    }
    *dimptr-- = ( Range * )nil;		/* point to least subscript */
    if ( reverseSubscripts )
	reverseRange( elemRange, numDims );

   /*
    *   this section calculates range values for consecutive elements 
    *   and calls arrayString.  It begins at the rightmost subscript
    *   and proceeds to the left. (Which is reversed for FORTRAN)
    */

    rng_mult = 1;
    while ( trun_num = num / rng_mult ) {

	/*
	 * Bump the lower range if not least subscript, since this range has
	 * already been retrieved in a lower subscript.
	 */  

        if ( *(dimptr+1) )
            (*dimptr)->lower++; 
	/*
	 * Check if it falls in the correct range
	 */
        if ( (*dimptr)->lower <= (*varDims)->upper ) {
            if ( (*dimptr)->lower + trun_num > (*varDims)->upper ) {
                (*dimptr)->upper = (*varDims)->upper;
            } else {

		/*  finished moving in this direction: dbx subrange
		    request will not have different values for
		    greater subscripts  */

                (*dimptr)->upper = (*dimptr)->lower + trun_num - 1;
                reverse = true;
            }
	    /*  decrement remaining number of elements  */
            num -= ( (*dimptr)->upper - (*dimptr)->lower + 1 ) * rng_mult;

	    /*   if fortran, reverse order of subscripts   */

       	    if ( reverseSubscripts )
		reverseRange( elemRange, numDims );

	    /*  get dbx subrange subscripts and values  */

            if ( arrayString( var, elemRange, arrstr ) )
                return -1;

	    /*   if fortran, reverse back order of subscripts   */

       	    if ( reverseSubscripts )
		reverseRange( elemRange, numDims );

            if ( reverse )
                break;  /* done in this direction */
        }
	/*
	 * Check if we have reached the final dimension
	 */
        if ( dimptr == elemRange )
            break; 
        rng_mult *= (*varDims)->upper - (*varDims)->lower + 1;
        (*dimptr)->lower = (*varDims)->lower;
        (*dimptr)->upper = (*varDims)->upper;
        varDims--;
        dimptr--;
    }
   /*
    *   this section calculates range values for consecutive elements 
    *   and calls arrayString.  It begins at the subscript where it
    *   finished above and proceeds to the right.
    */

    /*  while elements remaining and not rightmost subscript */
    while ( num && *(dimptr + 1 ) ) {
	/* set lower and upper bounds to one more than upper bound
	   and test if array exhausted */
	if( (*dimptr)->upper + 1 > (*varDims)->upper )
            break;
	(*dimptr)->upper++;
	(*dimptr)->lower = (*dimptr)->upper;
        varDims++;
        dimptr++;
        rng_mult /= (*varDims)->upper - (*varDims)->lower + 1;
        trun_num = num / rng_mult;
        if ( trun_num ) {
            (*dimptr)->upper = (*dimptr)->lower + trun_num - 1;
            num -= ( (*dimptr)->upper - (*dimptr)->lower + 1 ) * rng_mult;

	    /*   if fortran, reverse order of subscripts   */

       	    if ( reverseSubscripts )
		reverseRange( elemRange, numDims );

            if ( arrayString( var, elemRange, arrstr ) )
                return -1;

	    /*   if fortran, reverse back order of subscripts   */

       	    if ( reverseSubscripts )
		reverseRange( elemRange, numDims );

        } else
	    /*  skip this subscript and set upper so it will be changed
		correctly when tested at top of loop  */
            (*dimptr)->upper = (*dimptr)->lower - 1;
    }

    /*
     * If $unsafebounds is set then we may need to increment the last subscript
     * past the upper bound in order to show enough elements
     */
    if( varIsSet( "$unsafebounds" ) && num > 0 ) {
	if( num == numElements ) {
	    /*
	     * All the elements started outside the bounds, reset
	     */
	    while( *(dimptr+1) != NULL ) {
		(*dimptr)->lower = (*dimptr)->upper = (*dimptr)->base;
		dimptr++;
	    }
	    (*dimptr)->lower = (*dimptr)->upper = (*dimptr)->base;
	} else {
	    /*
	     * Point dimptr to the least subscript
	     */
	    varDims = var->range;
	    while( *(dimptr+1) != NULL ) {
		(*dimptr)->lower = (*dimptr)->upper = (*varDims)->upper;
		dimptr++;
		varDims++;
	    }
	    if( (*dimptr)->upper < (*dimptr)->base ) {
		(*dimptr)->upper = (*dimptr)->lower = (*dimptr)->base;
	    } else if( (*dimptr)->lower <= (*dimptr)->upper
		       && (*dimptr)->upper <= (*varDims)->upper ) {
		/*
		 * We have already obtained some of the elements for this range,
		 * reset lower to upper so we don't repeat any elements
		 */
		(*dimptr)->upper++;
		(*dimptr)->lower = (*dimptr)->upper;
	    }
	}
	/*
	 * Add the number to show to the last element
	 */
	(*dimptr)->upper = (*dimptr)->upper + num - 1;
	num = 0;

	if ( reverseSubscripts )
	    reverseRange( elemRange, numDims );
	if ( arrayString( var, elemRange, arrstr ) )
	    return -1;
	if ( reverseSubscripts )
	    reverseRange( elemRange, numDims );
    }

    /*   if fortran, reverse back order of range   */

    if ( reverseSubscripts )
	reverseRange( var->range, numDims );

    *numValues = numElements - num;
    return 0;
}


/*
 *  return the subscripts, values, and number of values in parameters returned
 *  of the array variable represented by the name field of the given
 *  VarStruct.  The beginning element and number of elements to return
 *  are also given.
 *  Returns: 0 for success; -1 for failure
 */
int      dpi_array_subscripts_values( var, 
         beginElement, numElements, subscripts, values, numValues )
struct VarStruct    	*var;
int        		*beginElement;
int        		numElements;
char    		***subscripts;
char    		***values;
int        		*numValues;
{
    char    *arrstr;

    arrstr = malloc( 1 );
    *arrstr = nil;
    if ( totalArrayString( var, 
        beginElement, numElements, &arrstr, numValues ) )
        return -1;
    getArrayValues( arrstr, *numValues, subscripts, values );
    return 0;
}

/*
 *  free two arrays of pointers received from call to
 *  dpi_array_subscripts_values.
 */

int     dpi_release_array( subscripts, values )
char    **subscripts;
char    **values;
{
    /* free character string of subscripts
       and values received from dbx */

    dispose( *subscripts );  

    /* free arrays of character pointers into above string,
       created by getArrayValues. */

    dispose( subscripts );   
    dispose( values );
    return 0;
}

/*
 *  reverse the values of an integer array
 */

void reverseIntArray( array, num )
int	*array;
int	num;
{
    int	i, j;
    int r;

    for ( i = 0, j = num - 1; i < j; i++, j-- ) {
	r = array[i];
	array[i] = array[j];
	array[j] = r;
    }
}


/*  
	This function always returns the current process id of the
	debuggee.
	Returns -1 if no debbuggee
*/
int dpi_report_newProcessId()
{
	if ( process && !isfinished(process) )
		return ( process->pid );
	else
		return ( -1 );
}


/*
 * NAME: dpi_get_ident
 *
 * FUNCTION: Gets the Name identifier for the specified variable
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES:
 *	1) Depending on how the language specifies to fold the case, convert the
 *	   given name to the specified case.
 *	2) Get the Name identifier for the converted variable.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * TESTING:
 *	1) Test FORTRAN, where input is in lowercase.
 *	2) Test COBOL, where input is in uppercase.
 *	3) Test C, where input is in mixed case.
 *
 * RETURNS: The Name identifier for the specified variable
 */
Name dpi_get_ident(variable)
String variable;
{
    Name	var;
    String	varname;

    switch( symcase ) {
	case lower:
	    varname = malloc( strlen( variable ) + 1 );
	    strcpy( varname, variable );
	    lowercase( varname );
	    var = identname( varname, false );
	    free( varname );
	    break;
	case upper:
	    varname = malloc( strlen( variable ) + 1 );
	    strcpy( varname, variable );
	    uppercase( varname );
	    var = identname( varname, false );
	    free( varname );
	    break;
	default:
	    var = identname( variable, false );
    }

    return var;
}


/*
 * NAME: dpi_convert_case
 *
 * FUNCTION: Converts the case of the identifier based on symcase
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES:
 *	1) Depending on how the language specifies to fold the case, convert the
 *	   given name to the specified case.
 *	2) Returned string should be freed by caller
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * TESTING:
 *	1) Test FORTRAN, where input is in lowercase.
 *	2) Test COBOL, where input is in uppercase.
 *	3) Test C, where input is in mixed case.
 *
 * RETURNS: The identifier name converted based on symcase
 */
String dpi_convert_case(variable)
String variable;
{
    String	varname;

    varname = malloc( strlen( variable ) + 1 );
    strcpy( varname, variable );

    switch( symcase ) {
	case lower:
	    lowercase( varname );
	    break;
	case upper:
	    uppercase( varname );
	    break;
    }
    return varname;
}


/*
 * NAME: dpi_hidden
 *
 * FUNCTION: Determine is specified variable is hidden
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * NOTES: 
 *
 * PARAMETERS:
 *	key		- Symbol structure of variable to check
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * TESTING:
 *
 * RETURNS: True if variable is hidden
 *	    False if variable is not hidden
 */
Boolean dpi_hidden( key )
Symbol key;
{
    Symbol	s;
    jmp_buf     env;
    int		*svenv;

    svenv = envptr;
    envptr = env;
    if (setjmp( env )) {
        return -1;
    }

    /*
     * If no key was passed in then we cannot check
     */
    if( !key ) return false;

    /*
     * Get the Symbol structure for the variable that is in scope
     */
    {
	Node n = which(key->name, WANY);
	assert(n->op == O_SYM); 
	s = n->value.sym;
    }
    
    envptr = svenv;

    /*
     * Check if the passed in variable is hidden
     */
    if( s == key ) {
	return false;
    } else {
	return true;
    }
}
