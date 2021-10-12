static char sccsid[] = "@(#)82	1.1  src/bos/usr/bin/bterm/help.c, libbidi, bos411, 9428A410j 8/26/93 13:34:58";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: BDHelp
 *		BDKeywordError
 *		BDListKeywords
 *		BDSyntax
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
 */
# include	<stdio.h>

static char *arg_string[] = {
"Usage: bterm [-help] [-keywords] [-nobidi] [-symmetric] [-autopush] ",
"  [-or Orientation] [-text TextType] [-nss NumShape] [-csd CharShape] ",
"  [-maps Mapdir] [-tail] [-nonulls] [-e Command] \n\n",
0
};

static char *help_string[]  = {
"The \"bterm\" command accepts the following options:\n",
"   -help           list the available option flags\n",
"   -keywords       list the keywords to be used in .Bidi-default file\n",
"   -nobidi         turns off bidi mode\n", 
"   -symmetric      turns on symmetric char swapping mode\n",
"   -autopush       turns on autopush mode\n",
"   -tail           turns on shaping of the seen family on two cells\n",
"   -nonulls        specifies the default initialization character to be \n",
"                   spaces instead of nulls.\n",
"   -or Orientation specifies default screen orientation:LTR or RTL\n",
"   -text TextType  specifies the type of data stream handled:\n",
"                   explict, implict or visual.\n",
"   -nss NumShape   specifies default shape of arabic numerals:\n",
"                   bilingual, hindi, arabic or passthru.\n",
"   -csd CharShape  specifies default shape of arabic characters:\n",
"                   automatic, passthru, isolated, initial, middle,\n",
"                   or final.\n",
"   -maps Mapdir    page codes to be used under the bterm command\n",
"   -e  Command     specifies the cmd to execute instead of a shell\n",
"                   this must be the last option on the command line\n",
0
};

static char *key_string[] = {
"The \"bterm\" command accepts the following .Bidi-defaults keywords:\n",
"\n",
"  nobidi        on,  turns off bidi mode\n",
"                off, turns on bidi mode\n\n",

"  fSCrRev       on,  the ScrRev function key is enabled\n",
"                off, the ScrRev function key is disabled\n\n",

"  fRTL          on,  the RTL function key is enabled\n",
"                off, the RTL function key is disabled\n\n",

"  fLTR          on,  the LTR function key is enabled\n",
"                off, the LTR function key is disabled\n\n",

"  fPush         on,  the Push function key is enabled\n",
"                off, the Push function key is disabled\n\n",

"  fEndPush      on,  the EndPush function key is enabled\n",
"                off, the EndPush function key is disabled\n\n",

"  fAutoPush     on,  the AutoPush function key is enabled\n",
"                off, the AutoPush function key is disabled\n\n",

"  fASD          on,  the ASD function key is enabled\n",
"                off, the ASD function key is disabled\n\n",

"  fShapeIS      on,  the ShapeIS function key is enabled\n",
"                off, the SHapeIS function key is disabled\n\n",

"  fShapeIN      on,  the ShapeIN function key is enabled\n",
"                off, the SHapeIN function key is disabled\n\n",

"  fShapeM       on,  the ShapeM function key is enabled\n",
"                off, the SHapeM function key is disabled\n\n",

"  fShapeF       on,  the ShapeF function key is enabled\n",
"                off, the SHapeF function key is disabled\n\n",

"  textType      Type of data stream handled. textType can be:\n",
"                explict, implicit or visual\n",
"                only visual text type is currently implemented\n\n",

"  orientation   LTR, left to right default screen orientation\n",
"                RTL, right to left default screen orientation\n\n",

"  symmetric     on,  Symmetric Character Swappina is enabled\n",
"                off, Symmetric Character Swapping is disabled\n\n",

"  numShape      Specifies default Numeral Shape Selection (NSS).\n",
"                NSS may only apply to Arabic and can be:\n",
"                bilingual, hindi, arabic, passthru\n\n",

"  charShape     Specifies default Character Shape Selection (CSD).\n",
"                CSD may only apply to Arabic and can be: automatic\n",
"                passthru, isolated, initial, middle or final\n\n",

"  autopush      on, turns on autopush enable\n",
"                off,turns off autopush enable\n\n",

"  tail          on, turns on seen shaping on two cells\n",
"                off,turns off seen shaping on one cell\n\n",

"  nonulls       on, turns on replacing all nulls by spaces\n",
"                off,turns off replacing all nulls by spaces\n\n",

"  maps          Specifies the page code directory to be used for keyboard\n",
"                layering, input, output and symetric character swapping\n",
0
};

BDSyntax (arg)
char *arg;
{
	register char **us = arg_string;

	fprintf(stderr, "\nUnknown option %s\n\n", arg);

	while (*us) 
		fprintf(stderr, *us++);

	fprintf(stderr, "Enter 'bterm -help' for more details\n\n");

	exit (1);
}

BDHelp ()
{
	register char **us = help_string;

	while (*us) 
		fputs(*us++, stdout);
	exit (0);
}

BDListKeywords ()
{
	register char **us = key_string;

	while (*us) 
		fputs(*us++, stdout);

	exit (0);
}

BDKeywordError(arg, val)
char *arg, *val;
{
	fprintf(stderr, "\nInvalid bidi parmeter \'%s\' = %s\n\n",
		 arg, val);

	fprintf(stderr, "Enter \"bterm -keywords\" for more details\n\n");

	exit (1);
}
