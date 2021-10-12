static char sccsid[] = "@(#)26	1.6  src/bos/diag/da/tok/tokfrus.c, datok, bos411, 9428A410j 8/19/93 15:22:16";
/*
 *   COMPONENT_NAME: DATOK
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#define NEW_DIAG_ASL	1
#define NEW_DIAG_CONTROLLER	1 

#include "diag/da.h"
#include "diag/dcda_msg.h"

/* fru bucket structures for the token-ring adapter */
struct fru_bucket frub710 =
  	{"", FRUB1, 0x850, 0x710, TOK_RMSG_1,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	};	

struct fru_bucket frub711 =
	{"", FRUB1, 0x850, 0x711, TOK_RMSG_1,
		{
			{70, "", "", 0, DA_NAME, EXEMPT},
			{30, "", "", 0, PARENT_NAME, EXEMPT},
		},
	};

struct fru_bucket frub712 =
  	{"", FRUB1, 0x850, 0x712, TOK_RMSG_1,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	};

struct fru_bucket frub713 =
  	{"", FRUB1, 0x850, 0x713, TOK_RMSG_1,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	};

struct fru_bucket frub714 =
  	{"", FRUB1, 0x850, 0x714, TOK_RMSG_1,
		{
			{65, "toknet", "", TOKNETMSG, NOT_IN_DB, EXEMPT},
			{10, "tokcable", "", CABLEMSG, NOT_IN_DB, EXEMPT},
			{25, "", "", 0, DA_NAME, EXEMPT},
		},
	};

struct fru_bucket frub715 =
  	{"", FRUB1, 0x850, 0x715, TOK_RMSG_1,
		{
			{90, "", "", 0, DA_NAME, EXEMPT},
			{10, "tokcable", "", CABLEMSG, NOT_IN_DB, EXEMPT},
		},
	};

struct fru_bucket frub716 =
  	{"", FRUB1, 0x850, 0x716, TOK_RMSG_1,
		{
			{75, "toknet", "", TOKNETMSG, NOT_IN_DB, EXEMPT},
			{25, "", "", 0, DA_NAME, EXEMPT},
		},
	};

struct fru_bucket frub717 =
  	{"", FRUB1, 0x850, 0x717, TOK_RMSG_1,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	};

struct fru_bucket frub720 =
  	{"", FRUB1, 0x850, 0x720, TOK_RMSG_2,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	};

struct fru_bucket frub721 =
  	{"", FRUB1, 0x850, 0x721, TOK_RMSG_2,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	};

struct fru_bucket frub722=
  	{"", FRUB1, 0x850, 0x722, TOK_RMSG_2,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	};

struct fru_bucket frub810 =
  	{"", FRUB1, 0x850, 0x810, TOK_RMSG_4,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	};	

struct fru_bucket frub811 =
  	{"", FRUB1, 0x850, 0x811, TOK_RMSG_4, 
		{
			{70, "", "", 0, DA_NAME, EXEMPT},
			{30, "", "", 0, PARENT_NAME, EXEMPT},
		},
	};

struct fru_bucket frub812 =
  	{"", FRUB1, 0x850, 0x812, TOK_RMSG_4,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	};

struct fru_bucket frub813 =
  	{"", FRUB1, 0x850, 0x813, TOK_RMSG_4,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	};

struct fru_bucket frub814 =
  	{"", FRUB1, 0x850, 0x814, TOK_RMSG_1,
		{
			{65, "toknet", "", TOKNETMSG, NOT_IN_DB, EXEMPT},
			{10, "tokcable", "", CABLEMSG, NOT_IN_DB, EXEMPT},
			{25, "", "", 0, DA_NAME, EXEMPT},
		},
	};

struct fru_bucket frub815 =
  	{"", FRUB1, 0x850, 0x815, TOK_RMSG_1,
		{
			{90, "", "", 0, DA_NAME, EXEMPT},
			{10, "tokcable", "", CABLEMSG, NOT_IN_DB, EXEMPT},
		},
	};

struct fru_bucket frub816 =
  	{"", FRUB1, 0x850, 0x816, TOK_RMSG_1,
		{
			{75, "toknet", "", TOKNETMSG, NOT_IN_DB, EXEMPT},
			{25, "", "", 0, DA_NAME, EXEMPT},
		},
	};

struct fru_bucket frub817 =
  	{"", FRUB1, 0x850, 0x817, TOK_RMSG_1,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	};

struct fru_bucket frub820 =
  	{"", FRUB1, 0x850, 0x820, TOK_RMSG_5,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	};
 
struct fru_bucket frub821 =
  	{"", FRUB1, 0x850, 0x821, TOK_RMSG_5,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	};

struct fru_bucket frub822 =
  	{"", FRUB1, 0x850, 0x822, TOK_RMSG_5,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	};

struct fru_bucket frub770 =
  	{"", FRUB1, 0x850, 0x770, TOK_RMSG_3,
		{
			{95, "", "", 0, DA_NAME, EXEMPT},
			{5, "", "", 0, PARENT_NAME, EXEMPT},
		},
	};


struct fru_bucket frub880 =
  	{"", FRUB1, 0x850, 0x880, TOK_RMSG_6,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	};

struct fru_bucket frub902 =
  	{"", FRUB1, 0x850, 0x902, TOK_RMSG_8,
		{
			{60, "", "", 0, DA_NAME, EXEMPT},
			{40, "", "", 0, PARENT_NAME, EXEMPT},
		},
	};

struct fru_bucket frub910 =
  	{"", FRUB1, 0x850, 0x910, TOK_RMSG_7,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	};

struct fru_bucket frub904 =
  	{"", FRUB1, 0x850, 0x904, TOK_RMSG_8,
		{
			{100, "", "", 0, DA_NAME, EXEMPT},
		},
	};

/*	configuration for device fails		*/
struct fru_bucket frub905 =
  	{"", FRUB1, 0x850, 0x905, TOK_RMSG_9,
		{
			{80, "", "", 0, DA_NAME, EXEMPT},
			{10, "", "", 0, PARENT_NAME, EXEMPT},
			{10, " ", "", TOK_RMSG_11, NOT_IN_DB, EXEMPT},
		},
	};

/* 	cannot open device because of EIO or ENODEV	*/
struct fru_bucket frub906 =
  	{"", FRUB1, 0x850, 0x905, TOK_RMSG_10,
		{
			{80, "", "", 0, DA_NAME, EXEMPT},
			{20, "", "", 0, PARENT_NAME, EXEMPT},
		},
	};
