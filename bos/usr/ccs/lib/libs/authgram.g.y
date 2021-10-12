/* @(#)12	1.2  src/bos/usr/ccs/lib/libs/authgram.g.y, libs, bos411, 9428A410j 8/31/93 12:42:49 */
/*
 *   COMPONENT_NAME: LIBS
 *
 *   FUNCTIONS: _create_parse_tree 
 *		_release_parse_tree
 *		_mkleaf
 *		_mknode	
 *		yyerror
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 *  Function:   This parser creates the parse tree from the grammar
 *	        used during user authentication.  The "SYSTEM" 
 *	        authentication line specifies the per user
 *		authentication domains that he/she can exist in.  
 *
 *		For example:  
 *
 *		SYSTEM=DCE OR (DCE[UNAVAIL] AND COMPAT)
 *		
 *		This line says that DCE should be tried first.  IF
 *		DCE does not return SUCCESS (DCE implies DCE[SUCCESS])
 *		then the OR condition is considered.  If DCE returned
 *		UNAVAIL and the COMPAT method returned success then
 *		the user may log on.  In other words this line describes
 *		acceptable return codes from authentication methods.  
 *		Using boolean logic should evaluate the expression to 
 *		True(acceptable login) or False(user cannot log on). 
 *
 * 		These particular routines create a parse tree for this 
 *		grammar that can then be run by the authenticate() routine.
 *		For instance the above grammar would be parsed into.
 *
 *				OR
 *			       /   \
 *			      /     \
 *		             /       \
 *		     DCE[SUCCESS]    AND
 *				    /   \
 *				   /     \
 *				  /       \
 *			   DCE[UNAVAIL]   COMPAT
 *
 *
 *		The BACKUS NAUR FORM (BNF) for this grammar is:
 *
 *			"SYSTEM"   ::=	EXPRESSION
 *
 *			EXPRESSION ::=	PRIMITIVE |
 *					"(" EXPRESSION ")" |
 *					EXPRESSION OPERATOR EXPRESSION
 *
 *			PRIMITIVE  ::=	METHOD |
 *					METHOD "[" RESULT "]"
 *
 *			ATOM	   ::=	METHOD |
 *					RESULT |
 *					OPERATOR
 *
 *		Some liberty has been used when implementing the yacc
 * 		grammar, but it correctly implements this BNF.
 */
%union {
	int  cmd;
	char *string;
}
%token <string> METHOD 			/* Non reserved word is a method */ 
%token <cmd> 	AND OR  		/* Operators 			 */
%token <cmd>	SUCCESS FAILURE UNAVAIL NOTFOUND NONE WILDCARD
%token <cmd>	RBRACKET LBRACKET	/* delimit return values 	 */
%type  <cmd>	primitive
%type  <cmd>	result
%type  <cmd>	expression
%left  OR
%left  AND		/* AND is a higher precedence than OR */
%{
#include <usersec.h>
#include "libs.h"

static struct secgrammar_tree *_mknode(int,struct secgrammar_tree *,
					struct secgrammar_tree*);
static struct secgrammar_tree *_mkleaf(char *, int);
       struct secgrammar_tree *_create_parse_tree(char *);
       void   		       _release_parse_tree(struct secgrammar_tree *);

int _yyinputerror;
%}
%%
line:		  expression '\n'
			{ return($1); }
		;
expression:	  primitive 
			{ $$ = $1; }
		| expression AND expression %prec AND
			{ $$ = _mknode(AND, $1, $3); }
		| expression OR expression  %prec OR
			{ $$ = _mknode(OR, $1, $3); }
		;

primitive:	  METHOD result
			{ $$ = _mkleaf($1, $2); }
		| METHOD
			{ $$ = _mkleaf($1, SUCCESS); }
		| '(' expression ')'
			{ $$ = $2; }
		/* 
		 * Create the leaf for this branch.  If no result has
		 * been specified to check, the implication is "success"
		 * only.  The leaf creation will return an address that
		 * will be passed back to the branch fork. 
		 */
		;

result:		  LBRACKET SUCCESS  RBRACKET
			{ $$ = $2; }
		| LBRACKET FAILURE  RBRACKET
			{ $$ = $2; }
		| LBRACKET UNAVAIL  RBRACKET
			{ $$ = $2; }
		| LBRACKET NOTFOUND RBRACKET
			{ $$ = $2; }
		| LBRACKET WILDCARD RBRACKET
			{ $$ = $2; }
		/*
		 * Return the result to check.  A result is used to
		 * specify what type return values are acceptable to
		 * be received from a method.  If all methods return
		 * acceptable values, the user is allowed to log in.
		 */
		;
%%
/*
 * NAME: _create_parse_tree()
 * 
 * FUNCTION: This routine sets up parser input variables and calls the
 * 	     parser.  The parser is responsible for creating a parse tree 
 *	     for the supplied grammar. 
 *
 * Returns:  NULL 	    - empty parse tree
 *	     Memory address - address of parse tree's root
 */
struct secgrammar_tree *
_create_parse_tree(char *grammar)
{
	extern char  *_yyinstring; /* grammar to parse 		    */
	extern int   _yyinptr; 	   /* grammar traversal pointer	    */ 
	struct secgrammar_tree *ptree;

	/* Set up input string for parser.  See authgram.l	    */
	_yyinstring = grammar;
	_yyinptr = 0;	
	_yyinputerror = 0;
	 
 	ptree = (struct secgrammar_tree *)yyparse();

	/*
	 * If I have encountered an error in the grammar, then yacc will
	 * create a null parse tree, or one that is so munged up I can't
	 * release it.  Therefore I will return NULL and allow the caller
	 * to handle any override cases.
	 */
	if (_yyinputerror)
		return((struct secgrammar_tree *)NULL);

	return(ptree);
}

/*
 * NAME: _mknode
 *
 * FUNCTION:  This function is responsible for creating an interior
 * 	      branch of the parse tree.  Whenever the parser discovers
 *	      an operator, this routine is called.  The routine allocates
 *	      a secgrammar_tree node and connects up the left and right 
 *	      branches of this fork in the tree.  The AND and OR are
 *	      specific defines created by yacc and therefore are switched
 *	      to my more independent specifications.
 *
 * RETURNS:   Pointer to this node
 */
static struct secgrammar_tree *
_mknode(int op, 
	struct secgrammar_tree *leftnode, 
	struct secgrammar_tree *rightnode)
{
        struct secgrammar_tree *tptr;

        if ((tptr = (struct secgrammar_tree *)
	    malloc(sizeof(struct secgrammar_tree))) == 
	    (struct secgrammar_tree *)NULL)
	{
		yyerror();
               	return((struct secgrammar_tree *)NULL);
	}

	memset(tptr, 0, sizeof(struct secgrammar_tree));

        tptr->type = NODE;		/* This is an interior node */
        tptr->left = leftnode;		/* Connect left and right branch */
        tptr->right = rightnode;
        if (op == AND)
                tptr->operator = AUTH_AND;
        else
                tptr->operator = AUTH_OR;

        return(tptr);
}

/*
 * NAME: _mkleaf
 *
 * FUNCTION:  This function is responsible for creating a frontier leaf
 * 	      of the parse tree.  Whenever the parser discovers a method
 *	      and expected result, this routine is called.  The routine 
 *	      allocates a secgrammar_tree leaf and stores the method name.
 *	     
 *	      The expected returns are specific defines created by yacc 
 *	      and therefore are switched to my more independent specifications.
 *
 * RETURNS:   Pointer to this leaf 
 */
static struct secgrammar_tree *
_mkleaf(char *method, int rc)
{
        struct secgrammar_tree *tptr;

        if ((tptr = (struct secgrammar_tree *)
	    malloc(sizeof(struct secgrammar_tree))) == 
	    (struct secgrammar_tree *)NULL)
	{
		yyerror();
               	return((struct secgrammar_tree *)NULL);
	}

	memset(tptr, 0, sizeof(struct secgrammar_tree));

        tptr->type = LEAF;
        if ((tptr->name = (char *)malloc(strlen(method)+1)) == (char *)NULL)
	{
		yyerror();
		free(tptr);
                return((struct secgrammar_tree *)NULL);
	}

        strcpy(tptr->name, method);
	switch (rc)
	{
		case SUCCESS:
			tptr->result = AUTH_SUCCESS;
			break;
		case FAILURE:
			tptr->result = AUTH_FAILURE;
			break;
		case UNAVAIL:
			tptr->result = AUTH_UNAVAIL;
			break;
		case NOTFOUND:
			tptr->result = AUTH_NOTFOUND;
			break;
		case WILDCARD:
			tptr->result = AUTH_WILDCARD;
			break;
		defalt: /* Otherwise must return success */
			tptr->result = AUTH_SUCCESS;
	}

        return(tptr);
}

/*
 * NAME: _release_parse_tree
 *
 * FUNCTION: Releases the grammar parse tree created by the _create_parse_tree()
 * 	     routines
 *
 * RETURNS:  void 
 */
void
_release_parse_tree(struct secgrammar_tree *tptr)
{
	if (tptr == (struct secgrammar_tree *)NULL)
		return;

	_release_parse_tree(tptr->left);
	_release_parse_tree(tptr->right);
	
	if (tptr->name)
		free(tptr->name);
	free(tptr);
}
/*
 * NAME: yyerror
 * 
 * FUNCTION: This function replaces the standard definition of yyerror().
 *	     This definition records the fact that an input error occurred
 *	     in the variable _yyinputerror.
 *
 * Returns:  nothing
 */
yyerror(char *errstring)
{
	_yyinputerror = 1;
}
