static char sccsid[] = "@(#)91	1.13  src/bos/usr/ccs/lib/libcfg/attrval.c, libcfg, bos412, 9444b412 10/31/94 16:39:03"; 
/*
 * COMPONENT_NAME: (LIBCFG)  Generic library config support
 *
 * FUNCTIONS: attrval
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#define		_ILS_MACROS

#include	<stdio.h>
#include	<sys/types.h>
#include	<string.h>
#include	<ctype.h>

#include	<sys/cfgdb.h>
#include	<sys/cfgodm.h> /* classes structure dcls */

/*------------------*/ 
/* Global variables */
/*------------------*/ 

static char	AttrName[ATTRNAMESIZE];      /* Attr name in input pair */
static char	AttrValu[ATTRVALSIZE];       /* Attr value in input pair */
static int nFlag, lFlag, rFlag, mFlag;   /* Boolean rep field flags */
static int UFlag;                        /* Boolean generic field flags */
static int GFlag;                        /* Boolean type field flags */
static int ErrCnt;                       /* Error count */
static int ErrLen;                       /* Error buffer length */
static char *ErrBuf;                     /* Error buffer (attr names) */

/*
 *
 * NAME: attrval
 *
 * FUNCTION:
 *  The "attrval" routine verifies attribute(s) value(s) are valid
 *  for a given device.
 *
 * INPUT PARAMETERS:
 *  utype   -- Device uniquetype
 *  pattr   -- Input string of attribute name / value pairs,
 *              e.g., "attr1=val1 attr2=val2 ..."
 *  errattr -- Pointer to string containing attribute names in 
 *              error, e.g., "attr3 attr7 ....."
 *
 * RETURN VALUE DESCRIPTION:
 *  0        -- successful
 *  non-zero -- attr error count
 *
 * GLOBAL VARIABLES REFERENCED :
 *  all
 */
int attrval(utype, pattr, errattr)
	char	*utype;
	char	*pattr;
	char	**errattr;
{
	char values[ATTRVALSIZE];       /* attribute possible values list */
	char *subvals[ATTRVALSIZE/2+1]; /* pointers into possible values list */
	int rc;                         /* general purpose return code variable */ 
	int	i;			                    /* general purpose loop invariant */
	int	mult;			                  /* flag to keep track of mult input values */
	unsigned long num;	            /* argument count for possible values array */
	int	parsed_ok;                  /* bool name/value pair parsed ok */
	char *substr;                   /* value substring ptr / loop invariant */ 
	char *tokstr = ",";             /* value seperator for multiple values */ 
	char *input_ptr;                /* current position in the input string */ 
	char *end_ptr;                  /* end pointer for strtoul function */ 
	unsigned long inpval;           /* Input value converted to long */
	unsigned long bval;             /* Boundry value for range attributes */
	unsigned long range[3];         /* Upper, lower, increment array for range */

	/* initialize variables */
	ErrBuf = *errattr = NULL; /* set return pointer to 0 */
	ErrLen = 0;               /* error buffer length */
	ErrCnt = 0;               /* error count */
	input_ptr = pattr;        /* set input attr value pair pointer */

	/*=======================================================*/
	/* Loop parsing the input string into name / value pairs */
	/*=======================================================*/

	while (get_next_pair(&input_ptr, &parsed_ok)) { 

		/* Check to see if name/val pair parsed ok */
		if (!parsed_ok) {
			errproc();	      /* put attr in error buf. */
			continue;
		}

		/* Get PdAt possible values string and set global flags */
		rc = get_values_and_flags(utype, AttrName, values);
		if (rc != 0) {
			errproc();	/* put attr in error buf. */
			continue;
		}

		/* If not user modifiable reject */
		if (!UFlag) {
			errproc();	/* put attr in error buf. */
			continue;
		}

		/*===========================================================*/
		/* Grouped attributes are verified by checking that each     */
		/* attribute in the group has at least as many list elements */
		/* (or range steps) as the attribute value passed in (which  */
		/* is the index into each grouped member's possible values). */
		/*===========================================================*/

		if (GFlag) {

			/* Input value is index of group list elements */
			inpval = strtoul(AttrValu, &end_ptr, 0);
			if (*end_ptr != ' ' && *end_ptr != '\0')
			{
				errproc();  /* put attr in error buf. */
				continue;
			}

			/* Get grouped attributes names from possible values string */
			parse_list(values, &num, subvals);
			if (num == 0) {
				errproc(); 	/* put attr in error buf */
				continue;
			}

			/* Verify each attr in group has at least inpval choices */
			for (i = 0 ; i < num ; i++) {
				rc = chk_attr_in_group(utype, inpval, subvals[i]);
				if (rc < 0) {
					errproc();  	/* put attr in errbuf */
					break;
				}
			}
			continue;	/* continue with next set */
		}

		/*===========================================================*/
		/* Process single and multiple input values for an attribute */
		/*===========================================================*/

		/*-------------------------------------------------*/
		/* Obtain the list values or the range definition. */
		/*-------------------------------------------------*/

		if (rFlag) /* range : implies numeric */ {
			/* Get range definition from possible values string */
			rc = parse_range(values, range);
			if (rc) {
				errproc(); 	/* put attr in error buf */
				continue;
			}
		}
		else if (lFlag) {
			/* Get list elements from possible values string */
			parse_list(values, &num, subvals);
			if (num == 0) {
				errproc(); 	/* put attr in error buf */
				continue;
			}
		}
		else /* !rFlag && !lFlag */
			continue;

		/* 'm' flag must be set if multiple values are supplied.  */
		/* We know that ',' chars must be delimiters since r or l */ 
		/* flag was set.                                          */
		if (strchr(AttrValu, ',') && !mFlag) {
			errproc();  /* put attr in error buf */
			continue;
		}

		/*------------------------------------------------------*/
		/* Loop through the comma seperated input value(s), and */
		/* verify each input value.                             */
		/*------------------------------------------------------*/

		substr = strtok(AttrValu, tokstr);
		if (!substr) /* No possible value - null string or all "," chars */ {
			errproc();  /* put attr in error buf */
			continue;
		}
		for ( ; substr ; substr = strtok(NULL, tokstr)) {

			/*--------------------------*/
			/* Validate range attribute */
			/*--------------------------*/

			if (rFlag) /* range : implies numeric */ {

				/* Check input value against range definition */

				/* Falls within allowable range... */
				inpval = strtoul(substr, &end_ptr, 0);
				if (*end_ptr != '\0')
				{
					/* Extra characters after strtoul() parsed numeric from strtok() */
					errproc();  /* put attr in error buf. */
					break; /* from input value loop */
				}
				if (inpval < range[0] || inpval > range[1]) {
					errproc(); 	/* put attr in error buf */
					break;      /* from input value loop */
				}

				/* ...and on a valid boundry */
				bval = ((inpval - range[0]) / range[2]) * range[2] + range[0];
				if (bval != inpval) {
					errproc(); 	/* put attr in error buf */
					break;      /* from input value loop */
				}
			} /* end if (rFlag), processing range attribute */

			/*-------------------------*/
			/* Validate list attribute */
			/*-------------------------*/

			else /* !rFlag : implies a list */ {

				/* Match input value with list element value if possible */

				if (nFlag) /* numeric list */ {

					inpval = strtoul(substr, &end_ptr, 0);
					if (*end_ptr != '\0')
					{
						/* Extra characters after strtoul() parsed numeric from strtok() */
						errproc();  /* put attr in error buf. */
						continue;
					}

					for (i = num ; i-- ; ) 
						if (inpval == strtoul(subvals[i], (char **)NULL, 0))
							break; /* from matching loop */
				}
				else /* !nFlag : implies string list */ {

					for (i = num ; i-- ; ) 
						if (!strcmp(substr, subvals[i]))
							break; /* from matching loop */
				}

				/* Fail if no match found */

				if (i >= num) {
					errproc(); 	/* put attr in error buf */
					break;      /* from input value loop */
				}

			} /* end else !rFlag , processing list attributes */

		} /* end for () parsing single/multiple input values */

	} /* end while() parsing name/val pairs */

	if (!parsed_ok)
		errproc(); 	/* put attr in error buf */

	*errattr = ErrBuf;
	return ErrCnt;	/* return error count */
}

/*
 * NAME: get_next_pair
 *
 * FUNCTION:
 *  parsed_ok the input line for attr name and expected attr. value
 *
 * INPUT PARAMETERS:
 *  input_ptr - pointer into string to parse next name/value pair from 
 *  parsed_ok - Returns FALSE if name/value pair not parsable, TRUE otherwise
 *
 * RETURN VALUE DESCRIPTION:
 *  TRUE  -- Call again
 *  FALSE -- No pair to parse out, end of string 
 *
 * GLOBAL VARIABLES REFERENCED :
 *  AttrName
 *  AttrValu
 */
static int get_next_pair(input_p, parsed_ok)
	char **input_p;
	int *parsed_ok;
{
	unsigned int j = 0;		/* char count */
	char ch;              /* current char (less dereferencing) */

	/* Init returned value */
	*parsed_ok = TRUE;

	/*---------------------------*/
	/* Get attribute name string */
	/*---------------------------*/

	/* Skip leading white space */
	for ( ; isspace(**input_p) ; (*input_p)++);

	/* Copy attribute name into global array */
	ch = **input_p;
	for ( ; ch != '\0' && ch != '=' && !isspace(ch) ; j++, ch = *(++(*input_p)))
		if (j < ATTRNAMESIZE)
			AttrName[j] = ch;

	/* Terminate attribute name string with null char */
	if (j >= ATTRNAMESIZE)
		AttrName[ATTRNAMESIZE - 1] = '\0',
		*parsed_ok = FALSE;
	else
		AttrName[j] = '\0';

	/* Valid name/val pair name is always terminated by '=' */
	if (ch != '=')
	{
		if (ch == '\0' && j == 0) /* Valid end of string condition */
			return FALSE; /* No more pairs */

		/* Anything else is invalid : whitespace, no "=val" part */
		*parsed_ok = FALSE;
		return TRUE;
	}

	/*----------------------------*/
	/* Get attribute value string */
	/*----------------------------*/
	
	/* Copy attribute value into global array */
	j = 0;
	ch = *(++(*input_p));
	for ( ; ch != '\0' && !isspace(ch) ; j++, ch = *(++(*input_p)))
		if (j < ATTRVALSIZE)
			AttrValu[j] = ch;

	/* Terminate attribute value string with null char */
	if (j >= ATTRVALSIZE)
		AttrValu[ATTRVALSIZE - 1] = '\0',
		*parsed_ok = FALSE;
	else
		AttrValu[j] = '\0';

	return TRUE;
}

/*
 * NAME: get_values_and_flags
 *
 * FUNCTION:
 *   Get attribute possible values and set global flags variables
 *
 * INPUT PARAMETERS:
 *  utype     - Device uniquetype
 *  attr_nm   - Attribute name
 *  values    - Attribute possible values returned 
 *
 * RETURN VALUE DESCRIPTION:
 *	0 -- successful
 *	-1 -- failed
 *
 * GLOBAL VARIABLES REFERENCED : 
 *  nFlag - 'numeric value' rep field flag
 *  lFlag - 'list of values' rep field flag
 *  rFlag - 'range notation' rep field flag
 *  mFlag - 'multiple value' rep field flag
 *  GFlag - 'GROUP attribute' type field flag
 *  UFlag - 'User modifiable attribute' generic field flag 
 */
static int get_values_and_flags(utype, attr_nm, values)
	char	*utype;
	char	*attr_nm;
	char	values[];
{
	struct listinfo objinfo;
	struct PdAt pdat, *pdat_rc;
	char	*c;
	int	i;
	char query_str[UNIQUESIZE + ATTRNAMESIZE + 31];

	nFlag=lFlag=GFlag=rFlag=UFlag=mFlag=0;

	/* Get PdAt object */
	sprintf(query_str, "uniquetype = '%s' and attribute = '%s'", utype, attr_nm);
	pdat_rc = odm_get_first(PdAt_CLASS, query_str, &pdat);
	if (pdat_rc == (struct PdAt *)-1 || pdat_rc == (struct PdAt *)NULL)
		return -1;

	/* Set global flags */
	if (strchr(pdat.generic, 'U') != 0) UFlag = 1;
	if (strchr(pdat.type,    'G') != 0) GFlag = 1;
	if (strchr(pdat.rep,     'r') != 0) rFlag = 1;
	if (strchr(pdat.rep,     'l') != 0) lFlag = 1;
	if (strchr(pdat.rep,     'n') != 0) nFlag = 1;
	if (strchr(pdat.rep,     'm') != 0) mFlag = 1;

	strcpy(values, pdat.values);	/* copy into allocated memory */	
	return 0;
}

/*
 * NAME: chk_attr_in_group
 *
 * FUNCTION:
 *	 Verify enough values in list or range to satisfy the number specified.
 *
 * INPUT PARAMETERS:
 *  utype   -- device unique type
 *  number  -- minimum number of possible values
 *  attr_nm -- attribute name
 *
 * RETURN VALUE DESCRIPTION:
 *		0 -- successful
 *		-1 -- failed
 *
 * GLOBAL VARIABLES REFERENCED : 
 *  lFlag - 'list of values' rep field flag
 *  rFlag - 'range notation' rep field flag
 */
static int chk_attr_in_group(utype, number, attr_nm)
	char *utype;
	unsigned long	number;
	char *attr_nm;
{
	char	values[ATTRVALSIZE];	/* attribute value */
	char	*p;
	int	i, rc;
	unsigned long possval;
	unsigned long range[3]; /* Upper, lower, increment array for range */

	if (number == 0) /* number is invalid */
		return -1;

	if (get_values_and_flags(utype, attr_nm, values) != 0) 
		return -1;

	/*------------------------*/
	/* Verify range attribute */
	/*------------------------*/

	if (rFlag) { /* RANGE */
		rc = parse_range(values, range);
		if (rc)
			return -1;
	
		possval = (range[1] - range[0]) / range[2] + 1;
	}

	/*-----------------------*/
	/* Verify list attribute */
	/*-----------------------*/

	else if (lFlag) { /* LIST */
		for (p = values, possval = 1 ; *p != '\0' ; *p++)
			if (*p == ',' && ++possval == number)
				break; /* count satify */
	}

	else /* !rflag && !lflag */
		return 0;
	
	/* Verify attr has at least 'number' possible values */
	if (possval < number)
		return -1;

	return 0; /* OK */
}

/*
 * NAME: parse_range
 *
 * FUNCTION:
 *	 Decode input for range value.
 *
 * INPUT PARAMETERS:
 *	 str   -- PdAt values field
 *	 range -- 3 elem array to store lower, upper, increment
 *
 * RETURN VALUE DESCRIPTION:
 *		0 -- successful
 *		-1 -- failed
 *
 * GLOBAL VARIABLES REFERENCED : 
 *  none
 */
static int parse_range(str, range)
	char *str;
	unsigned long	range[];
{

	if (!strchr(str, '-'))
		return -1;

	/* Convert lower-upper,increment string to ulong values */
	range[0] = strtoul(str, (char **)NULL, 0);
	range[1] = strtoul(strchr(str, '-') + 1, (char **)NULL, 0);
	range[2] = (str = strchr(str, ',')) ? strtoul(str + 1, (char **)NULL, 0) : 1;

	if ((range[1] <= range[0]) || range[2] == 0) 
		return -1;
	return 0;
}

/*
 * NAME : parse_list
 *
 * FUNCTION : seperates the specified string into seperate arguments and 
 *            returns pointers to each in the argv array. Commas and white
 *            space (as defined by isspace()) delimit each argument.
 *
 *            Note the input string is modified by this function.
 *
 * PARAMETERS :
 *	cp        - string to chop up
 *	argc      - # of args in argv
 *	argv      - array of pointers to each seperated argument (returned)
 *
 * RETURN VALUE:
 *	0 - ALWAYS successful
 *
 * GLOBAL VARIABLES REFERENCED : 
 *  none
 */
static int parse_list(cp, argc, argv)
	char *cp;
	int *argc;
	char *argv[];
{
	char *substr;
	char *tokstr = ", \t\v\r\n"; /* blank, tab, vert tab, carr ret, new line */

	*argc = 0;

	for (substr = strtok(cp, tokstr) ; substr ; substr = strtok(NULL, tokstr)) 
		argv[(*argc)++] = substr;

	return 0;
}

/*
 * NAME:  errproc
 *
 * FUNCTION:
 *	  Add the attribute in error to the error buffer.
 *
 * INPUT PARAMETERS:
 *
 * RETURN VALUE:
 *	0 -- successful
 *	non-zero -- failed
 *
 * GLOBAL VARIABLES REFERENCED :
 *  AttrName
 *  ErrBuf 
 *  ErrLen
 *  ErrCnt
 */
static int errproc()
{
	int	length;
	char	*p;

	/* extra byte for blank between argument */
	length = strlen(AttrName) + 1;
	if (ErrBuf == NULL) {
		/* allocate extra byte for null termination */
		ErrBuf = (char *)malloc(length + 1);
		if (ErrBuf == NULL)
			return -1;
	}
	else {
		ErrBuf = (char *)realloc(ErrBuf, ErrLen + length + 1);
		if (ErrBuf == NULL)
			return -1;
	}

	/* append to the end of the error buf. */
	p = ErrBuf + ErrLen;
	strcpy(p, AttrName);
	p  += length - 1;	/* put a blank between arg. */
	*p++ = ' ';
	*p = '\0';			/* null termination */
 	ErrLen += length;	/* realign next error msg. char ptr */
	ErrCnt++;
	return 0;
}
