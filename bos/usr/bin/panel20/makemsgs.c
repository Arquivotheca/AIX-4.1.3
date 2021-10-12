#ifdef COMPILE_SCCSID
static char sccsid[] = "@(#)93	1.2  src/bos/usr/bin/panel20/makemsgs.c, cmdhia, bos411, 9428A410j 6/15/90 17:38:55";
#endif

/*
 * COMPONENT_NAME: (CMDHIA) Messages Scanner and Mask Setter
 *
 * FUNCTIONS: main, scantext, scanargs
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1986, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include "w_dismsg.h"

FILE *infp;
FILE *outfp;
char inbuf[160];
char dlim;
char tempbuf[4];
int setnum;
int msgnum;
int firsttime = 1;

main(argc, argv)
int argc;
char *argv[];
{
	char *infile;
	char *outfile;
	int i;
	int j;
	int len;
	int wlen;

	/*
	 * makemsgs <input file> <output file>
	 * input file  - messages source file
	 * output file - .c file which initializes essential variables 
         *               for each message 
	 */
	if (argc != 3)
	{
		printf("Incorrect number of arguments.\n");
		exit(1);
	}

	infile = *++argv;
	if ((infp = fopen(infile, "r")) == NULL)
	{
		printf("Cannot open %s.\n", infile);
		exit(1);
	}

	outfile = *++argv;
	if ((outfp = fopen(outfile, "w")) == NULL)
	{
		printf("Cannot create %s.\n", outfile);
		exit(1);
	}

	fprintf(outfp, "#include <string.h>\n");
	fprintf(outfp, "#include \"w_msg.h\"\n\n");
	fprintf(outfp, "int mask;\n\n");
	fprintf(outfp, "initialize(snum, mnum)\n");
	fprintf(outfp, "int snum;\n");
	fprintf(outfp, "int mnum;\n");
	fprintf(outfp, "{\n");
	fprintf(outfp, "\tswitch(snum)\n");
	fprintf(outfp, "\t{\n");

	/* Process one message source file line at a time */
	while (fgets(inbuf, (int)sizeof(inbuf), infp) != NULL)
	{
		/* if 1st char is a $,
		   then check if we have a $quote or $set cmd */
		if (inbuf[0] == '$')
		{
			/* if we did find a $quote cmd, get the delimit char */
			if (!strncmp(inbuf, "$quote", 6))
			{
				i = 6;
				while (inbuf[i] == ' ' || inbuf[i] == '\t' )
					++i;
				if (inbuf[i] == '\n')
				{
					printf("ERROR - No delimit character \
after $quote.\n");
					exit(1);
				}
				dlim = inbuf[i];
			}

			/* if we did find a $set cmd, get the set number */
			else if (!strncmp(inbuf, "$set", 4))
			{
				i = 4;
				while (inbuf[i] == ' ' || inbuf[i] == '\t')
					++i;
				if (inbuf[i] == '\n')
				{
					printf("ERROR - No set number after \
$set.\n");
					exit(1);
				}
				strcpy(tempbuf, "   ");
				j = 0;
				do 
				{
					tempbuf[j] = inbuf[i];
					++i;
					++j;
				} while (inbuf[i] != ' ' && inbuf[i] != '\n' 
							 && inbuf[i] != '\t');
				setnum = atoi(tempbuf);
				if (!firsttime)
				{
				 	fprintf(outfp, "\t\t\t\tdefault:\n");
				 	fprintf(outfp, 
				"\t\t\t\t\tstrcpy(regmsgnum, \"000\");\n");
				 	fprintf(outfp, 
				"\t\t\t\t\tmask = %d;\n", W_MSGVI1 | W_MSGVI2);
				 	fprintf(outfp, 
				"\t\t\t\t\tcount = 2;\n");
				 	fprintf(outfp, 
				"\t\t\t\t\tvalidmsg = 0;\n");
				 	fprintf(outfp, "\t\t\t\t\tbreak;\n");
				 	fprintf(outfp, "\t\t\t}\n");
				 	fprintf(outfp, "\t\t\tbreak;\n");
				}
				firsttime = 0;
				fprintf(outfp, "\t\tcase %d:\n", setnum);
				fprintf(outfp, "\t\t\tswitch(mnum)\n");
				fprintf(outfp, "\t\t\t{\n");
			}
		}

		/* if 1st char is not a $,
		   then check if we have a new message */
		else
		{
			len = strlen(inbuf);
			wlen = strspn(inbuf, " \t\n");
			if (wlen == len) /* this is a blank line */
				continue;
			else if ((inbuf[wlen] < '0') || (inbuf[wlen] > '9'))
			{
				printf("ERROR - Message number does not \
start with a number.\n");
				exit(1);
			}
			else /* must be start of a new message
				so get the message number      */
			{
				strcpy(tempbuf, "   ");
				i = wlen;
				j = 0;
				do
				{
					tempbuf[j] = inbuf[i];
					++i;
					++j;
				} while (inbuf[i] != ' ' && inbuf[i] != '\t' 
						 	&& inbuf[i] != '\n');
				if (inbuf[i] == '\n')
				{
					printf("ERROR - No message text after \
message number.\n");
					exit(1);
				}
				msgnum = atoi(tempbuf);
				fprintf(outfp, "\t\t\t\tcase %d:\n", msgnum);
				do
				{
					++i;
				} while (inbuf[i] == ' ' || inbuf[i] == '\t');
				if (inbuf[i] != dlim)
				{
					printf("ERROR - Message text does not \
start with delimit char.\n");
					exit(1);
				}
				scantext(++i);
			}
		}
	}
	fprintf(outfp, "\t\t\t\tdefault:\n");
	fprintf(outfp, "\t\t\t\t\tstrcpy(regmsgnum, \"000\");\n");
	fprintf(outfp, "\t\t\t\t\tmask = %d;\n", W_MSGVI1 | W_MSGVI2);
	fprintf(outfp, "\t\t\t\t\tcount = 2;\n");
	fprintf(outfp, "\t\t\t\t\tvalidmsg = 0;\n");
	fprintf(outfp, "\t\t\t\t\tbreak;\n");
	fprintf(outfp, "\t\t\t}\n");
	fprintf(outfp, "\t\t\tbreak;\n");
	fprintf(outfp, "\t\tdefault:\n", setnum);
	fprintf(outfp, "\t\t\tstrcpy(regmsgnum, \"000\");\n");
	fprintf(outfp, "\t\t\tmask = %d;\n", W_MSGVI1 | W_MSGVI2);
	fprintf(outfp, "\t\t\tcount = 2;\n");
	fprintf(outfp, "\t\t\tvalidmsg = 0;\n");
	fprintf(outfp, "\t\t\tbreak;\n");
	fprintf(outfp, "\t}\n");
	fprintf(outfp, "}\n");
	fclose(infp);
	fclose(outfp);
}

/*
 * Scan for the external (registered) message number 
 * and call scanargs to scan for arguments to be 
 * inserted into the message
 */
scantext(i)
int i;
{
	int j;

	while (inbuf[i] == ' ' || inbuf[i] == '\t')
		++i;
	/* if we found the component number, 
	   get the unique 3 digit number for this message */
	if (!strncmp(&inbuf[i], "0790-", 5))
	{
		strcpy(tempbuf, "   ");
		i = i + 5;
		j = 0;
		do
		{
			tempbuf[j] = inbuf[i];
			++i;
			++j;
		} while (inbuf[i] != ' ' && inbuf[i] != '\t' 
					 && inbuf[i] != '\n');
		fprintf(outfp, "\t\t\t\t\tstrcpy(regmsgnum, \"%s\");\n", 
								tempbuf);
	}
	else /* assign blanks to the 3 digit number */
		fprintf(outfp, "\t\t\t\t\tstrcpy(regmsgnum, \"   \");\n");
	scanargs(i);
	return;
}

/*
 * Scan for arguments to be inserted into the message
 */
scanargs(i)
int i;
{
	int msgmask = 0;
	int count = 0;
	int intcnt = 0;
	int longcnt = 0;
	int strcnt = 0;
	int charcnt = 0;

do
{
	/* Process each char until end of line or 
           until the delimit char is encountered */
	while(inbuf[i] != '\0' && inbuf[i] != dlim)
	{
		/* look for %n$ argument specifications */
		if (inbuf[i] == '%')
		{
			++i;
			if (inbuf[++i] == '$')
			{
				/* its an int argument */
				if (inbuf[++i] == 'd')
				{
					/* if its the 1st int,
					   add the 1st int mask to mask for 
					   this message */
					if (intcnt == 0)
					{
						msgmask |= W_MSGVI1;
						intcnt++;
					}
					/* if its the 2nd int,
					   add the 2nd int mask to mask for 
					   this message */
					else if (intcnt == 1)
					{
						msgmask |= W_MSGVI2;
						intcnt++;
					}
					else /* only 2 ints allowed */
					{
						printf("ERROR - More \
integers than allowed.\n");
						exit(1);
					}
				}
				else if (inbuf[i] == 'l')
				{
					/* its a long int argument */
					if (inbuf[++i] == 'd')
					{
						/* if its the 1st long int,
					   	   add the 1st long int mask to
					   	   mask for this message */
						if (longcnt == 0)
						{
							msgmask |= W_MSGVL1;
							longcnt++;
						}
						/* if its the 2nd long int,
					   	   add the 2nd long int mask to
					   	   mask for this message */
						else if (longcnt == 1)
						{
							msgmask |= W_MSGVL2;
							longcnt++;
						}
						/* only 2 long ints allowed */
						else
						{
							printf("ERROR - More \
longs than allowed.\n");
							exit(1);
						}
					}
				}
				/* its a char string argument */
				else if (inbuf[i] == 's')
				{
					/* if its the 1st char string,
				   	   add the 1st char string mask to
				   	   mask for this message */
					if (strcnt == 0)
					{
						msgmask |= W_MSGVC1;
						strcnt++;
					}
					/* if its the 2nd char string,
				   	   add the 2nd char string mask to
				   	   mask for this message */
					else if (strcnt == 1)
					{
						msgmask |= W_MSGVC2;
						strcnt++;
					}
					/* if its the 3rd char string,
				   	   add the 3rd char string mask to
				   	   mask for this message */
					else if (strcnt == 2)
					{
						msgmask |= W_MSGVC3;
						strcnt++;
					}
					else /* only 3 char strings allowed */
					{
						printf("ERROR - More \
strings than allowed.\n");
						exit(1);
					}
				}
				/* its a char argument */
				else if (inbuf[i] == 'c')
				{
					/* if its the 1st char,
				   	   add the 1st char mask to
				   	   mask for this message */
					if (charcnt == 0)
					{
						msgmask |= W_MSGVCH1;
						charcnt++;
					}
					else /* only 1 char allowed */
					{
						printf("ERROR - More \
characters than allowed.\n");
						exit(1);
					}
				}
			}
		}
		++i;
	}
	if (inbuf[i] == dlim) /* end of message */
	{
		fprintf(outfp, "\t\t\t\t\tmask = %d;\n", msgmask);
		if (msgmask & W_MSGVI1)
		{
		  fprintf(outfp, "/* W_MSGVI1 */\n");
		  count++;
		}
		if (msgmask & W_MSGVI2)
		{
		  fprintf(outfp, "/* W_MSGVI2 */\n");
		  count++;
		}
		if (msgmask & W_MSGVL1)
		{
		  fprintf(outfp, "/* W_MSGVL1 */\n");
		  count++;
		}
		if (msgmask & W_MSGVL2)
		{
		  fprintf(outfp, "/* W_MSGVL2 */\n");
		  count++;
		}
		if (msgmask & W_MSGVC1)
		{
		  fprintf(outfp, "/* W_MSGVC1 */\n");
		  count++;
		}
		if (msgmask & W_MSGVC2)
		{
		  fprintf(outfp, "/* W_MSGVC2 */\n");
		  count++;
		}
		if (msgmask & W_MSGVC3)
		{
		  fprintf(outfp, "/* W_MSGVC3 */\n");
		  count++;
		}
		if (msgmask & W_MSGVCH1)
		{
		  fprintf(outfp, "/* W_MSGVCH1 */\n");
		  count++;
		}
		fprintf(outfp, "\t\t\t\t\tcount = %d;\n", count);
		fprintf(outfp, "\t\t\t\t\tvalidmsg = 1;\n");
		fprintf(outfp, "\t\t\t\t\tbreak;\n");
		return;
	}
	else /* position to 1st character */
		i = 0;
} while (fgets(inbuf, (int)sizeof(inbuf), infp) != NULL); /* get next line */
printf("ERROR - Message text does not end with delimit char.\n");
exit(1);
}
