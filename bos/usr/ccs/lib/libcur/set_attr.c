static char sccsid[] = "@(#)66	1.15  src/bos/usr/ccs/lib/libcur/set_attr.c, libcur, bos411, 9428A410j 3/7/92 19:03:02";
/*
 * COMPONENT_NAME: (LIBCUR) Extended Curses Library
 *
 * FUNCTIONS: sel_attr, setup_attr, switch_stuff, set_stuff,
 *            start_a_new_set, close_out_a_set, add_member_to_set,
 *            check_cnt, avail_attr, pattern_set, bx_attr_set
 *
 *
 * ORIGINS: 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        "cur99.h"
#include        "cur03.h"

 /* this array contains the default set of attributes */
static int  default_attr_set[] = {
    _dREVERSE, _dINVISIBLE,
    _dF_WHITE, _dF_RED, _dF_BLUE, _dF_GREEN, _dF_BROWN, _dF_MAGENTA,
    _dF_CYAN, _dF_BLACK,
    _dB_BLACK, _dB_RED, _dB_BLUE, _dB_GREEN, _dB_BROWN, _dB_MAGENTA,
    _dB_CYAN, _dB_WHITE,
    _dBOLD, _dUNDERSCORE,
 _dTOPLINE, _dBOTTOMLINE, _dRIGHTLINE, _dLEFTLINE,
    _dBLINK, _dDIM,
    _dSTANDOUT,
    _dPROTECTED,
    _dFONT0, _dFONT1, _dFONT2, _dFONT3, _dFONT4, _dFONT5, _dFONT6,
    _dFONT7,
    NULL
};

static struct Ntb {             /* table used to convert attr
				   names to combined mask       */
    char    c1;                 /* termcap name of attr - 2 ch  */
    char    c2;
    int    *at;                 /* pointer to mask integer      */
}                   NTB[] =     /* name/attribute table         */
{                               /* initialization data          */
                        'm', 'b', &BLINK,
			'm', 'd', &BOLD,
			'm', 'h', &DIM,
			'm', 'k', &INVISIBLE,
                        'm', 'r', &REVERSE,
                        'u', 's', &UNDERSCORE,
                        't', 'p', &TOPLINE,
                        'b', 'm', &BOTTOMLINE,
                        'r', 'v', &RIGHTLINE,
                        'l', 'v', &LEFTLINE,
                        's', 'o', &STANDOUT,
                        'c', '0', &F_WHITE,
                        'c', '1', &F_RED,
                        'c', '2', &F_GREEN,
                        'c', '3', &F_BROWN,
                        'c', '4', &F_BLUE,
                        'c', '5', &F_MAGENTA,
                        'c', '6', &F_CYAN,
                        'c', '7', &F_BLACK,
                        'd', '0', &B_BLACK,
                        'd', '1', &B_RED,
                        'd', '2', &B_GREEN,
                        'd', '3', &B_BROWN,
                        'd', '4', &B_BLUE,
                        'd', '5', &B_MAGENTA,
                        'd', '6', &B_CYAN,
                        'd', '7', &B_WHITE,
                        'f', '0', &FONT0,
                        'f', '1', &FONT1,
                        'f', '2', &FONT2,
                        'f', '3', &FONT3,
                        'f', '4', &FONT4,
                        'f', '5', &FONT5,
                        'f', '6', &FONT6,
                        'f', '7', &FONT7,
                        '\0', '\0', NULL
};				/* end of table marker         */



 /* globals for avail_attr() to return string values in */
static char *set_str,
           *reset_str;
				/* various global mnemonic variables */
static int  cur_attr_type,
            last_attr_type,
            bit_loc,
            set_cnt;
int     mask_cnt;
int	fake_invis;		/* TRUE if no invisible mode on TERM */

struct attr_mask    MASK[16];	/* actual structure space for 16 bits
				   worth */

static  ATTR mask[8],
        bits[16];		/* individual masks & bits */
static int *pr_loc;		/* location in priority array */

/*
 * NAME:                sel_attr
 *
 * FUNCTION: initailize the attribute buffer.
 */

sel_attr(set)
int     set[];
{
    register int   *t,
                   *tl;
    t = default_attr_set;	/* set pointer to table start   */
    tl = &default_attr_set[sizeof(default_attr_set) /
				sizeof(default_attr_set[0]) - 1];
				/* set pointer to end of table  */
    while ((t < tl) && *set)	/* while not at either table end */
	*t++ = *set++;
    *t = NULL;			/* protect against ambiguities */
}

/*
 * NAME:                setup_attr
 *
 * FUNCTION: This routine initailizes the attribute
 *      masks according to a set of defaults and the terminals capabilit-
 *      ies.  The defaults can be changed with a call to sel_attr().
 *
 * EXECUTION ENVIRONMENT:
 *
 *      set entries in attribute priority array based on argument
 *      array.
 *
 * EXTERNAL REFERENCES: none
 *
 * DATA STRUCTURES:     int attr_set[], struct attr_mask
 *
 * RETURNS:             none
 */

setup_attr() {			/* do the magic... */
 /* 1st, set up the masking bits... */
    for (bit_loc = 1, bits[0] = 1; bit_loc < 16; bit_loc++)
	bits[bit_loc] = 2 * bits[bit_loc - 1];

 /* init attribute variables to zero - allows reuse of setup_attr */
    NORMAL= REVERSE= BOLD= BLINK= UNDERSCORE= DIM= INVISIBLE= PROTECTED= 0;
    TOPLINE= BOTTOMLINE= RIGHTLINE= LEFTLINE=0;
    STANDOUT= F_BLACK= F_RED= F_GREEN= F_BROWN= F_BLUE= F_MAGENTA= F_CYAN= 0;
    F_WHITE= B_BLACK= B_RED= B_GREEN= B_BROWN= B_BLUE= B_MAGENTA= B_CYAN= 0;
    B_WHITE= FONT0= FONT1= FONT2= FONT3= FONT4= FONT5= FONT6= FONT7=0;

 /* init everything to zero (not necessary, but good style...) */
    bit_loc = mask_cnt = set_cnt = cur_attr_type = last_attr_type = 0;

    sw_mask = '\0';		/* clear 'all-switch-attr' mask */

    pr_loc = default_attr_set;	/* point to the requested/default attrs */

    while (*pr_loc && mask_cnt < 16 && bit_loc < 16) {
				/* main processing loop */

	cur_attr_type = avail_attr(*pr_loc);

	switch (cur_attr_type) {/* do appropriate thing for each type */
	    case 0: 
		pr_loc++;	/* attribute not available */
		continue;
	    case 1: 
		switch_stuff();	/* character attribute */
		break;
	    case 2: 		/* forground color */
		if (last_attr_type == 3 || last_attr_type == 4)
		    close_out_a_set();
		set_stuff();
		break;
	    case 3: 		/* background color */
		if (last_attr_type == 2 || last_attr_type == 4)
		    close_out_a_set();
		set_stuff();
		break;
	    case 4: 		/* character font */
		if (last_attr_type == 2 || last_attr_type == 3)
		    close_out_a_set();
		set_stuff();
		break;
	    default:            /* impossible case...   */
		break;          /* ignored              */
	}			/* end switch */

	pr_loc++;		/* goto next element in list */

    }				/* end while */

    if (set_cnt != 0)		/* if we left a set unfinished, close it
				   now */
	close_out_a_set();

    if (Bx)
	Bxa = bx_attr_set(Bx);	/* get attr set for box 1      */
    if (By)
	Bya = bx_attr_set(By);	/* get attr set for box 2      */


}				/* end setup_attr() */


/*
 * NAME:                switch_stuff
 *
 * FUNCTION: switch the attribute
 */

switch_stuff() {
    if (last_attr_type > 1)
	close_out_a_set();
    if (bit_loc > 15 || mask_cnt > 15)
	return;
    MASK[mask_cnt].start_bit = bit_loc;
    MASK[mask_cnt].type_attr = last_attr_type = cur_attr_type;
    MASK[mask_cnt].num_val = 1;
    MASK[mask_cnt].act_attr = mask[0] = bits[bit_loc];
    MASK[mask_cnt].set[0] = set_str;
    MASK[mask_cnt].reset = reset_str;
    sw_mask |= mask[0];		/* build up total mask for all */
				/* switch type attributes used */
    pattern_set(*pr_loc, mask[0]);
    bit_loc++;
    mask_cnt++;
}


/*
 * NAME:                set_stuff
 *
 * FUNCTION: start/add an attribute set
 */

set_stuff() {
    if (bit_loc > 15 || mask_cnt > 15)
	return;
    if (last_attr_type != cur_attr_type)
	start_a_new_set();
    else
	add_member_to_set();
}


/*
 * NAME:                start_a_new_set
 *
 * FUNCTION: start a new attribute set
 */

start_a_new_set() {
    MASK[mask_cnt].start_bit = bit_loc;
    MASK[mask_cnt].type_attr = last_attr_type = cur_attr_type;
    MASK[mask_cnt].num_val = set_cnt = 1;
    MASK[mask_cnt].act_attr = mask[0] = 0;
    MASK[mask_cnt].set[0] = set_str;
    MASK[mask_cnt].reset = reset_str;
    pattern_set(*pr_loc, 0);
}

/*
 * NAME:                close_out_a_set
 *
 * FUNCTION: complete an attribute set
 */

close_out_a_set() {
    register int    i,
                    j;

    i = check_cnt(set_cnt - 1);
    if (i > 0) {
	for (j = 0; j < i; j++)
	    MASK[mask_cnt].act_attr =
			MASK[mask_cnt].act_attr * 2 + bits[bit_loc];
	bit_loc += i;
    }
    else
	bit_loc = 16;

    set_cnt = 0;
    mask_cnt++;
}

/*
 * NAME:                add_member_to_set
 *
 * FUNCTION: add a new attribute member
 */

add_member_to_set() {
    register int    i;

    MASK[mask_cnt].num_val++;
    MASK[mask_cnt].set[set_cnt] = set_str;
    mask[set_cnt] = mask[set_cnt - 1] + bits[bit_loc];
    pattern_set(*pr_loc, mask[set_cnt]);
    set_cnt++;

    i = check_cnt(set_cnt);
    if (i < 0) {
	close_out_a_set();
	last_attr_type = 0;
    }
    else
	if (i + bit_loc > 16)
	    close_out_a_set();
}


/*
 * NAME:                check_cnt
 *
 * FUNCTION: verify the count of members in an attribute.
 */

check_cnt(num)
register int    num;
{
    register int    i = 0;

    do {
	i++;
	num = num / 2;
    } while (num);

    if (i > 3)			/* This statement will limit the # of
				   members in a set to 8 */
	i = -1;
    return i;
}


/*
 * NAME:                avail_attr
 *
 * FUNCTION: validate the attribute type
 */

avail_attr(attr)
int     attr;
{
    register int    type;

    switch (attr) {

	case _dREVERSE: 
	    set_str = MR;
	    type = 1;
	    break;
	case _dBOLD: 
	    set_str = MD;
	    type = 1;
	    break;
	case _dBLINK: 
	    set_str = MB;
	    type = 1;
	    break;
	case _dUNDERSCORE: 
	    set_str = US;
	    type = 1;
	    break;
        case _dTOPLINE:
	    set_str = TP;
	    type = 1;
	    break;
        case _dBOTTOMLINE:
	    set_str = BM;
	    type = 1;
	    break;
        case _dRIGHTLINE:
	    set_str = RV;
	    type = 1;
	    break;
        case _dLEFTLINE:
	    set_str = LV;
	    type = 1;
	    break;
	case _dDIM: 
	    set_str = MH;
	    type = 1;
	    break;
	case _dINVISIBLE: 
	    set_str = MK;
	    if (set_str == NULL) {	/* INVISIBLE always exists, even if */
		set_str = "";		/* we have to fake it */
		fake_invis = 1;
	    } else {
		fake_invis = 0;
	    }
	    type = 1;
	    break;
	case _dPROTECTED: 
	    set_str = MP;
	    type = 1;
	    break;
	case _dSTANDOUT: 
	    set_str = SO;
	    type = 1;
	    break;

	case _dF_BLACK: 
	    set_str = CF[0];
	    type = 2;
	    break;
	case _dF_RED: 
	    set_str = CF[1];
	    type = 2;
	    break;
	case _dF_GREEN: 
	    set_str = CF[2];
	    type = 2;
	    break;
	case _dF_BROWN: 
	    set_str = CF[3];
	    type = 2;
	    break;
	case _dF_BLUE: 
	    set_str = CF[4];
	    type = 2;
	    break;
	case _dF_MAGENTA: 
	    set_str = CF[5];
	    type = 2;
	    break;
	case _dF_CYAN: 
	    set_str = CF[6];
	    type = 2;
	    break;
	case _dF_WHITE: 
	    set_str = CF[7];
	    type = 2;
	    break;

	case _dB_BLACK: 
	    set_str = CB[0];
	    type = 3;
	    break;
	case _dB_RED: 
	    set_str = CB[1];
	    type = 3;
	    break;
	case _dB_GREEN: 
	    set_str = CB[2];
	    type = 3;
	    break;
	case _dB_BROWN: 
	    set_str = CB[3];
	    type = 3;
	    break;
	case _dB_BLUE: 
	    set_str = CB[4];
	    type = 3;
	    break;
	case _dB_MAGENTA: 
	    set_str = CB[5];
	    type = 3;
	    break;
	case _dB_CYAN: 
	    set_str = CB[6];
	    type = 3;
	    break;
	case _dB_WHITE: 
	    set_str = CB[7];
	    type = 3;
	    break;

	case _dFONT0: 
	    set_str = FO[0];
	    type = 4;
	    break;
	case _dFONT1: 
	    set_str = FO[1];
	    type = 4;
	    break;
	case _dFONT2: 
	    set_str = FO[2];
	    type = 4;
	    break;
	case _dFONT3: 
	    set_str = FO[3];
	    type = 4;
	    break;
	case _dFONT4: 
	    set_str = FO[4];
	    type = 4;
	    break;
	case _dFONT5: 
	    set_str = FO[5];
	    type = 4;
	    break;
	case _dFONT6: 
	    set_str = FO[6];
	    type = 4;
	    break;
	case _dFONT7: 
	    set_str = FO[7];
	    type = 4;
	    break;

	default: 
	    set_str = NULL;
	    type = 0;
	    break;		/* unknown */

    }				/* end switch */

    if (type == 1)
	reset_str = ME;
    else
	reset_str = NULL;

    if (type != 0 && set_str == NULL) {
	reset_str = NULL;
	type = 0;
    }
    return type;
}


/*
 * NAME:                pattern_set
 *
 * FUNCTION: set the attribute mask
 */

pattern_set(attr,key_mask)
	register int    attr;
	register ATTR key_mask;
{

    switch (attr) {

	case _dREVERSE: 
	    REVERSE = key_mask;
	    break;		/* reverse */
	case _dBOLD: 
	    BOLD = key_mask;
	    break;		/* bold */
	case _dBLINK: 
	    BLINK = key_mask;
	    break;		/* blink */
	case _dUNDERSCORE: 
	    UNDERSCORE = key_mask;
	    break;		/* underscore */
        case _dTOPLINE:
	    TOPLINE = key_mask;
	    break;		/* topline: outline(keisen) */
        case _dBOTTOMLINE:
	    BOTTOMLINE = key_mask;
	    break;		/* bottomline: outline(keisen) */
        case _dRIGHTLINE:
	    RIGHTLINE = key_mask;
	    break;		/* right line: outline(keisen) */
        case _dLEFTLINE:
	    LEFTLINE = key_mask;
	    break;		/* left line: outline(keisen) */
	case _dDIM: 
	    DIM = key_mask;
	    break;		/* dim */
	case _dINVISIBLE: 
	    INVISIBLE = key_mask;
	    break;		/* invisible */
	case _dPROTECTED: 
	    PROTECTED = key_mask;
	    break;		/* protected */
	case _dSTANDOUT: 
	    STANDOUT = key_mask;
	    break;		/* standout */

	case _dF_BLACK: 
	    F_BLACK = key_mask;
	    break;		/* f_black */
	case _dF_RED: 
	    F_RED = key_mask;
	    break;		/* f_red */
	case _dF_GREEN: 
	    F_GREEN = key_mask;
	    break;		/* f_green */
	case _dF_BROWN: 
	    F_BROWN = key_mask;
	    break;		/* f_brown */
	case _dF_BLUE: 
	    F_BLUE = key_mask;
	    break;		/* f_blue */
	case _dF_MAGENTA: 
	    F_MAGENTA = key_mask;
	    break;		/* f_magenta */
	case _dF_CYAN: 
	    F_CYAN = key_mask;
	    break;		/* f_cyan */
	case _dF_WHITE: 
	    F_WHITE = key_mask;
	    break;		/* f_white */

	case _dB_BLACK: 
	    B_BLACK = key_mask;
	    break;		/* b_black */
	case _dB_RED: 
	    B_RED = key_mask;
	    break;		/* b_red */
	case _dB_GREEN: 
	    B_GREEN = key_mask;
	    break;		/* b_green */
	case _dB_BROWN: 
	    B_BROWN = key_mask;
	    break;		/* b_brown */
	case _dB_BLUE: 
	    B_BLUE = key_mask;
	    break;		/* b_blue */
	case _dB_MAGENTA: 
	    B_MAGENTA = key_mask;
	    break;		/* b_magenta */
	case _dB_CYAN: 
	    B_CYAN = key_mask;
	    break;		/* b_cyan */
	case _dB_WHITE: 
	    B_WHITE = key_mask;
	    break;		/* b_white */

	case _dFONT0: 
	    FONT0 = key_mask;
	    break;		/* font 0 */
	case _dFONT1: 
	    FONT1 = key_mask;
	    break;		/* font 1 */
	case _dFONT2: 
	    FONT2 = key_mask;
	    break;		/* font 2 */
	case _dFONT3: 
	    FONT3 = key_mask;
	    break;		/* font 3 */
	case _dFONT4: 
	    FONT4 = key_mask;
	    break;		/* font 4 */
	case _dFONT5: 
	    FONT5 = key_mask;
	    break;		/* font 5 */
	case _dFONT6: 
	    FONT6 = key_mask;
	    break;		/* font 6 */
	case _dFONT7: 
	    FONT7 = key_mask;
	    break;		/* font 7 */

	default: 
	    break;		/* unknown */

    }				/* end switch */
}

/*
 * NAME:                bx_attr_set
 *
 * FUNCTION: determine the mask for the set of attributes specified in the
 *      description strings (termcap) Bx and/or By
 *      The string contains the termcap names for the attributes to
 *      be used with each box set. The table above pairs those names
 *      with the global variables containing the attribute masks
 */

bx_attr_set(pt)
char   *pt;			/* arg is pointer to string of */
				/* - termcap names             */

{

    char   *p1;			/* second pointer into string  */

    struct Ntb *tp;		/* pointer into table          */

    int     rt = 0;		/* hold return code            */

    for (; *pt != '\0'; pt += 2) {/* step thru string of names   */
	p1 = pt + 1;		/* calculate ptr to second char */
	for (tp = &NTB[0]; tp->c1 != '\0'; tp++) {
				/* step through the table      */
	    if (*pt == tp->c1 && *p1 == tp->c2) {
				/* if name matches in table    */
		rt |= *(tp->at);/* combine masks for this set  */
		break;		/* consider next name in string */
	    }			/* end - if match              */
	}			/* end - search table          */
    }				/* end - scan string of names  */
    return rt;			/* send back mask for attrs    */
}				/* end - bx_attr_set           */
