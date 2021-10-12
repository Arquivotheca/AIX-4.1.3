static char sccsid[] = "@(#)17	1.1  src/bldenv/gos/desktop/pass2/htag2.c, desktop, bos412, GOLDA411a 5/3/94 16:48:29";
/*
 *   COMPONENT_NAME: DESKTOP
 *
 *   FUNCTIONS: AssertFileIsReadable
 *		BuildIndex
 *		BuildLoids
 *		CloseFile
 *		FcloseFile
 *		FileExists
 *		FileSize
 *		FopenFile
 *		FwriteFile
 *		GetALine
 *		HandleSNB
 *		IncorporateVstructElements1149
 *		IterateLoids
 *		LocateVstructElements1029
 *		LookForTossFile
 *		MIN
 *		MakeFileNames
 *		OpenFile
 *		ReadFile
 *		RemoveSuperfluousBlocks
 *		StringToUpper
 *		WriteFile
 *		main
 *		sizeof
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>

#define xxx 1

#define MIN(a,b) (((a)<(b))?(a):(b))

#define TMPDIR   "/usr/tmp"
#define RS       '\036'
#define LINEMAX  (BUFSIZ * 8)

/* keep the following array and enum in sync */
static char *elementWithIdStrings[] = {
    "VIRPAGE",
    "BLOCK",
    "FORM",
    "P",
    "CP",
    "HEAD",
    "SUBHEAD",
    "ANCHOR",
    "SNREF",
    "GRAPHIC",
    "TEXT",
    "AUDIO",
    "VIDEO",
    "ANIMATE",
    "SCRIPT",
    "CROSSDOC",
    "MAN-PAGE",
    "TEXTFILE",
    "SYS-CMD",
    "CALLBACK",
    "SWITCH",
    0
    };

typedef enum {
    virpage,
    block,
    form,
    p,
    cp,
    head,
    subhead,
    anchor,
    snref,
    graphic,
    text,
    audio,
    video,
    animate,
    script,
    crossdoc,
    man_page,
    textfile,
    sys_cmd,
    callback,
    _switch,
    unknown,
    } ElementWithId;


typedef struct _loidsRec {
    ElementWithId     type;
    int               rlevel;
    int               offset;
    char             *rid;
    char             *rssi;
    struct _loidsRec *next;
   } LoidsRec, *LoidsPtr;
LoidsRec loids; /* the first node is a dummy to make the logic easier */


/* Keep the following array and enum in sync.  The code relies on the
 * end tags coming together at the end of the list and started by
 * eloids.  The "<VIRPAGE" is last because we stop scanning the toss
 * once we hit a virpage.
 */
char *elementOfInterestStrings[] = {
    "<LOIDS",
    "<TOSS",
    "<INDEX",
    "<LOPHRASES",
    "</LOIDS>",
    "</TOSS>",
    "</INDEX>",
    "</LOPHRASES>",
    "<VIRPAGE",
    0
    };

typedef enum {
    loidsTag,
    tossTag,
    indexTag,
    lophrasesTag,
    eloidsTag,
    etossTag,
    eindexTag,
    elophrasesTag,
    first_virpage,
    start_of_document
    } ElementOfInterest;

/* The following offsets are all into inFile.  They are zero if the
 * element doesn't exist.  The "- 2" is because we don't care about
 * the offset of the first "<virpage" nor do we need an entry for the
 * NULL terminator.  For start tags, the offset is the offset to the
 * STAGO.  For end tags, the offset is the offset to the character
 * after the ETAGC unless that character is a newline in which case
 * the offset is to the character following the newline.
 */
int offsets[(sizeof(elementOfInterestStrings) / sizeof(char *)) - 2];

static char openLoids[]  = "<LOIDS>";
static char closeLoids[] = "\n</LOIDS>\n";


char  dotSDL[]       = ".sdl";
char  dotIDX[]       = ".idx";
char  dotSNB[]       = ".snb";
char  dotTSS[]       = ".tss";

int   compression; /* = 0; set to non-zero if the user asks for compression */

char *progName;   /* the base name of this program - used in error messages */

/*
 * Names for all the input, output and intermediate files we'll need.
*/
char *inFileName;
char *outFileName;
char *idxFileName;
char *snbFileName;
char *sortedIdxFileName;
char *tossFileName;
char *loidsFileName;
char *tempFileName;
char *compFileName;
char *compZFileName;

FILE *inFile;
FILE *outFile;

int haveIndex; /* = 0;  set to non-zero if we have built an index */
int tossSize;  /* = 0;  set to actual size of toss file if exists */

/*
 * First we have some small and obvious utility routines for opening,
 * reading, writing and closing files.  Most have versions for both
 * file descriptor and stream operations.  When possible, the file
 * descriptor (unbuffered) versions are used for performance reasons.
 * Of course, when using the unbuffered versions we attempt to read in
 * BUFSIZ characters at a time.
*/
int
OpenFile(char *name, int type, int code)
{
int fd;
int oflag, mode;

mode = 0;
if ((oflag = type) == O_WRONLY)
    {
    oflag |= O_CREAT;
    mode = 0666;
    }

if ((fd = open(name, oflag, mode)) == -1)
    {
    fprintf(stderr,
	    "%s: error opening \"%s\" for %s\n",
	    progName,
	    name,
	    (type == O_RDONLY) ? "reading" : "writing");
    perror(name);
    exit(code);
    }
return fd;
}


int
ReadFile(int fd, char *name, char *buffer, int amount, int code)
{
int length;

if ((length = read(fd, buffer, amount)) == -1)
    {
    fprintf(stderr, "%s: error reading from \"%s\"\n", progName, name);
    perror(name);
    exit(code);
    }
return length;
}


int
WriteFile(int fd, char *name, char *buffer, int amount, int code)
{
int length;

if ((length = write(fd, buffer, amount)) == -1)
    {
    fprintf(stderr, "%s: error writing to \"%s\"\n", progName, name);
    perror(name);
    exit(code);
    }
return length;
}


int
FileExists(char *name)
{
if (access(name, F_OK) == 0) return 1;
return 0;
}


void
AssertFileIsReadable(char *name, int code)
{
if (access(name, R_OK) == -1)
    {
    fprintf(stderr, "%s: cannot open \"%s\" for reading\n", progName, name);
    perror(name);
    exit(code);
    }
}


int
FileSize(char *name, int code)
{
struct stat buf;

if (stat(name, &buf) == -1)
    {
    fprintf(stderr, "%s: error getting size of \"%s\"\n", progName, name);
    perror(name);
    exit(code);
    }

return buf.st_size;
}


void
CloseFile(int fd, char *name, int code)
{
if (close(fd) == -1)
    {
    fprintf(stderr, "%s: error closing \"%s\"\n", progName, name);
    perror(name);
    exit(code);
    }
}


FILE *
FopenFile(char *name, int type, int code)
{
FILE *file;
char *mode;

if (type == O_RDONLY)
    mode = "r";
else if (type == O_WRONLY)
    mode = "w";
else
    {
    fprintf(stderr,
	    "%s: bad type (%d) in opening \"%s\"",
	    progName,
	    type,
	    name);
    exit(code);
    }

if ((file = fopen(name, mode)) == NULL)
    {
    fprintf(stderr,
	    "%s: error opening \"%s\" for %s\n",
	    progName,
	    name,
	    (*mode == 'r') ? "reading" : "writing");
    perror(name);
    exit(code);
    }
return file;
}


int
GetALine(FILE *file, char *name, char *line, int max, int code)
{
char *pc;
int   length;

pc = fgets(line, max, file);
if (!pc)
    {
    if (!ferror(file))
	{
	return 0;
	}
    else
	{
	fprintf(stderr,
		"%s: error getting a line from \"%s\"\n",
		progName,
		name);
	perror(name);
	exit(code);
	}
    }

length = strlen(pc);
if ((length >= (max - 1)) && (line[max-1] != '\n'))
    {
    fprintf(stderr,
	    "%s: line longer than %d bytes in file \"%s\"\n",
	    progName,
	    max,
	    name);
    exit(code);
    }
return length;
}


void
FwriteFile(FILE *file, char *name, char *buffer, size_t amount, int code)
{
if (fwrite(buffer, 1, amount, file) != amount)
    {
    fprintf(stderr, "%s: error writing to \"%s\"\n", progName, name);
    perror(name);
    exit(code);
    }
}


void
FcloseFile(FILE *file, char *name, int code)
{
if (fclose(file) != 0)
    {
    fprintf(stderr, "%s: error closing \"%s\"\n", progName, name);
    perror(name);
    exit(code);
    }
}


int
StringToUpper(char *string)
{
char *pc, c;

pc = string;
while (c = *pc)
    {
    if (isalpha(c) && islower(c)) *pc = toupper(c);
    pc++;
    }
return pc - string;
}


/*
 * A routine to pre-build all the input, output and intermediate file
 * names that we'll need.
*/
void
MakeFileNames(int argc, char **argv)
{
int   argCount;
int   length;
char *cp;
char  pid[32];
char *tmpDir;

progName = strrchr(*argv, '/');
if (progName) progName++;
else progName = *argv;
argCount = 1;

if ((argc - argCount) > 0)
    {
    if (strcmp(argv[1], "-c") == 0)
	{
	compression = 1;
	argCount++;
	}
    }

if (((argc - argCount) < 1) || ((argc - argCount) > 2))
    {
    fprintf(stderr,
	    "Usage: %s [-c] <inFileName> [<outFileName>]\n",
	    progName);
    exit(1);
    }

inFileName  = argv[argCount++];
outFileName = argv[argCount];     /* may be NULL */

cp = strrchr(inFileName, '.');
if (cp)
    {
    if (strcmp(cp, dotSDL) == 0)
	{
	length = cp - inFileName;
	cp = malloc(length + 1);
	strncpy(cp, inFileName, length);
	cp[length] = '\0';
	}
    else
	{
	cp = inFileName;
	}
    }
else
    cp = inFileName;
length = strlen(cp);
inFileName = malloc(length + sizeof(dotSDL));
strcpy(inFileName, cp);
strcat(inFileName, dotSDL);
if (!outFileName)
    outFileName = inFileName;

idxFileName = malloc(length+sizeof(dotIDX));
strcpy(idxFileName, cp);
strcat(idxFileName, dotIDX);

snbFileName = malloc(length+sizeof(dotSNB));
strcpy(snbFileName, cp);
strcat(snbFileName, dotSNB);

tossFileName = malloc(length+sizeof(dotTSS));
strcpy(tossFileName, cp);
strcat(tossFileName, dotTSS);

tmpDir = getenv("TMPDIR");
if (!tmpDir)
    tmpDir = TMPDIR;
sprintf(pid, "%d", getpid());
length = sizeof(dotSDL) - 1 + strlen(tmpDir) + strlen(pid) + 1; 
tempFileName = malloc(length);
strcpy(tempFileName, tmpDir);
strcat(tempFileName, "/");
strcat(tempFileName, dotSDL + 1); /* skip the "." */
strcat(tempFileName, pid);
compFileName = malloc(length + 4);
strcpy(compFileName, tempFileName);
strcat(compFileName, "comp");
compZFileName = malloc(length + 4 + 2);
strcpy(compZFileName, compFileName);
strcat(compZFileName, ".Z");
sortedIdxFileName = malloc(length + 3);
strcpy(sortedIdxFileName, tempFileName);
strcat(sortedIdxFileName, "idx");
loidsFileName = malloc(length + 5);
strcpy(loidsFileName, tempFileName);
strcat(loidsFileName, "loids");
}


/*
 * A routine to remove occurrences of <BLOCK>\n</BLOCK>, if any, from
 * the .sdl file.  These occurrences are an artifact of translation
 * and seem to be easier to get rid of here than in the first pass.
*/
void
RemoveSuperfluousBlocks()
{
FILE *inFile, *outFile;
int   sawBlock;
char  line1[LINEMAX], line2[LINEMAX], line3[LINEMAX];
char *pThisLine, *pLastLine, *pTempLine;
int   thisLength, lastLength;

inFile  = FopenFile(tempFileName, O_RDONLY, xxx);
unlink(tempFileName);
outFile = FopenFile(tempFileName, O_WRONLY, xxx);

sawBlock  = 0;
pThisLine = line1;
pLastLine = line2;
while ((thisLength = GetALine(inFile,
			      tempFileName,
			      pThisLine,
			      LINEMAX,
			      xxx)) > 0)
    {
    strcpy(line3, pThisLine);
    StringToUpper(line3);
    if (sawBlock)
	{
	sawBlock = 0;
	if (strcmp(line3, "</BLOCK>\n") == 0)
	    continue;
	FwriteFile(outFile, tempFileName, pLastLine, lastLength, xxx);
	}
    if (strcmp(line3, "<BLOCK>\n") == 0)
	{
	sawBlock = 1;
	lastLength = thisLength;
	pTempLine = pLastLine;
	pLastLine = pThisLine;
	pThisLine = pTempLine;
	continue;
	}
    FwriteFile(outFile, tempFileName, pThisLine, thisLength, xxx);
    }
FcloseFile(inFile,  tempFileName, xxx);
FcloseFile(outFile, tempFileName, xxx);
}


/*
 * A routine to build the index from the .idx file (if any).
*/
void
BuildIndex()
{
char  buffer[BUFSIZ+1], *pFrom, *pTo, *pRestart, thisChar;
int   length, size;
int   status;
int   remnant;
int   found;
int   sortedIdxFd;
FILE *sdlIdxFile;
static char openIndex[]  = "\n<INDEX>\n";
static char closeIndex[] = "</INDEX>\n";
static char openEntry[]  = "<ENTRY LOCS=\"";
static char closeEntry[] = "</ENTRY>\n";
char sort[BUFSIZ], text[BUFSIZ], rid[BUFSIZ];
char lastsort[BUFSIZ], lasttext[BUFSIZ], lastrid[BUFSIZ];
char locs[BUFSIZ];

/* skip the index building step if no .idx file or .idx file is empty */
if (!FileExists(idxFileName) || (FileSize(idxFileName, xxx) == 0))
    return;

AssertFileIsReadable(idxFileName, xxx);

haveIndex = 1;

sortedIdxFd = OpenFile(sortedIdxFileName, O_WRONLY, xxx);
CloseFile(sortedIdxFd, sortedIdxFileName, xxx);

sprintf(buffer, "${ODE_TOOLS}/usr/bin/sort -f %s > %s", idxFileName, sortedIdxFileName);
if (status = system(buffer))
    {
    if (status == -1)
	{
	fprintf(stderr,
		"%s: error forking to execute \"%s\"\n",
		progName,
		buffer);
	perror(buffer);
	exit(1);
	}
    else
	{
	fprintf(stderr, "%s: error executing \"%s\"\n", progName, buffer);
	exit(1);
	}
    }

sortedIdxFd = OpenFile(sortedIdxFileName, O_RDONLY, xxx);
unlink(sortedIdxFileName);
sdlIdxFile = FopenFile(sortedIdxFileName, O_WRONLY, xxx);

FwriteFile(sdlIdxFile,
	   sortedIdxFileName,
	   openIndex,
	   sizeof(openIndex)-1,
	   xxx);

lastsort[0]    = '\0';
lasttext[0]    = '\0';
lastrid[0]     = '\0';
locs[0]        = '\0';
buffer[BUFSIZ] = '\0';
size    = 0;
remnant = 0;
pFrom = buffer;
while (size ||
       ((size = ReadFile(sortedIdxFd,
			 sortedIdxFileName,
			 buffer+remnant,
			 BUFSIZ-remnant,
			 xxx)) > 0))
    {
    /* add in remnant in case we just got a new buffer load */
    size += remnant;
    remnant = 0;

    /* First split out the "sort", "text" and "rid" fields.  They are
       seperated by an ASCII Record Seperator (036) character */
    pTo = sort;
    pRestart = pFrom;
    found = 0;
    while (--size >= 0)
	{
	if ((*pTo++ = *pFrom++) == RS)
	    {
	    found = 1;
	    break;
	    }
	}
    if (!found)
	{
	size = 0;
	pTo = buffer;
	pFrom = pRestart;
	while (*pTo++ = *pFrom++);
	remnant = pTo - buffer - 1;
	pFrom = buffer;
	continue;
	}
    *(pTo-1) = '\0';

    pTo = text;
    found = 0;
    while (--size >= 0)
	{
	thisChar = *pFrom++;
	if (thisChar == RS)
	    {
	    found = 1;
	    *pTo  = '\0';
	    break;
	    }
	if ((thisChar == '<') || (thisChar == '&'))
	    *pTo++ = '&';
	*pTo++ = thisChar;
	}
    if (!found)
	{
	size = 0;
	pTo = buffer;
	pFrom = pRestart;
	while (*pTo++ = *pFrom++);
	remnant = pTo - buffer - 1;
	pFrom = buffer;
	continue;
	}

    pTo = rid;
    found = 0;
    while (--size >= 0)
	{
	if ((*pTo++ = *pFrom++) == '\n')
	    {
	    found = 1;
	    break;
	    }
	}
    if (!found)
	{
	size = 0;
	pTo = buffer;
	pFrom = pRestart;
	while (*pTo++ = *pFrom++);
	remnant = pTo - buffer - 1;
	pFrom = buffer;
	continue;
	}
    *(pTo-1) = '\0';

    /* if buffer is consumed, reset ourselves for the next buffer load */
    if (size <= 0)
	{
	size    = 0;
	remnant = 0;
	pFrom   = buffer;
	}

    if (strcmp(text, lasttext) == 0)
	{
	if (strcmp(rid, lastrid) != 0)
	    {
	    strcat(locs, " ");
	    strcat(locs, rid);
	    }
	}
    else
	{
	if (strlen(locs) != 0)
	    {
	    if (strcmp(lastsort, lasttext) != 0)
		{
		length = fprintf(sdlIdxFile,
				 "%s%s\" SORT=\"%s\">%s%s",
			         openEntry,
			         locs,
			         lastsort,
			         lasttext,
			         closeEntry);
		}
	    else
		{
		length = fprintf(sdlIdxFile,
			         "%s%s\">%s%s",
			         openEntry,
			         locs,
			         lasttext,
			         closeEntry);
		}
	    if (length < 0)
		{
		fprintf(stderr,
			"%s: error writing to \"%s\"\n",
			progName,
			sortedIdxFileName);
		perror(sortedIdxFileName);
		exit(1);
		}
	    }
	strcpy(locs, rid);
	}
    strcpy(lasttext, text);
    strcpy(lastsort, sort);
    strcpy(lastrid,  rid);
    }
if (strlen(locs) != 0)
    {
    if (strcmp(lastsort, lasttext) != 0)
	{
	length = fprintf(sdlIdxFile,
			 "%s%s\" SORT=\"%s\">%s%s",
			 openEntry,
			 locs,
			 lastsort,
			 lasttext,
			 closeEntry);
	}
    else
	{
	length = fprintf(sdlIdxFile,
			 "%s%s\">%s%s",
			 openEntry,
			 locs,
			 lasttext,
			 closeEntry);
	}
    if (length < 0)
	{
	fprintf(stderr,
		"%s: error writing to \"%s\"\n",
		progName,
		sortedIdxFileName);
	perror(sortedIdxFileName);
	exit(1);
	}
    }
close(sortedIdxFd);
FwriteFile(sdlIdxFile,
	   sortedIdxFileName,
	   closeIndex,
	   sizeof(closeIndex)-1,
	   xxx);
FcloseFile(sdlIdxFile, sortedIdxFileName, xxx);
}


void
LookForTossFile()
{
/* do we need to interpolate a <toss> element? */
if (FileExists(tossFileName))
    tossSize = FileSize(tossFileName, xxx);
if (tossSize != 0)
    AssertFileIsReadable(tossFileName, xxx);
}


void
HandleSNB()
{
int   oldOffset, newOffset;
FILE *snbFile;
int   outFd, inFd;
char  line[LINEMAX], buffer[BUFSIZ];
int   length, delta;
static char openSNB[]  = "<SNB>\n";
static char closeSNB[] = "</SNB>\n";

oldOffset = 0;
newOffset = 0;
inFd  = OpenFile(inFileName,   O_RDONLY, xxx);
unlink(tempFileName);
outFd = OpenFile(tempFileName, O_WRONLY, xxx);

if (FileExists(snbFileName))
    {
    snbFile = FopenFile(snbFileName, O_RDONLY, xxx);
    while ((length = GetALine(snbFile,
			      snbFileName,
			      line,
			      LINEMAX,
			      xxx)) > 0)
	{
	if (isdigit(*line))
	    {
	    if (newOffset != 0)
		WriteFile(outFd,
			  tempFileName,
			  closeSNB,
			  sizeof(closeSNB)-1,
			  xxx);
	    newOffset = atoi(line);
	    delta = newOffset - oldOffset;
	    while (delta)
		{
		length = ReadFile(inFd,
				  inFileName,
				  buffer,
				  MIN(BUFSIZ,delta),
				  xxx);
		WriteFile(outFd, tempFileName, buffer, length, xxx);
		delta -= length;
		}
	    WriteFile(outFd, tempFileName, openSNB, sizeof(openSNB)-1, xxx);
	    oldOffset = newOffset;
	    }
	else
	    WriteFile(outFd, tempFileName, line, length, xxx);
	}
    if (newOffset != 0)
	WriteFile(outFd, tempFileName, closeSNB, sizeof(closeSNB)-1, xxx);
    FcloseFile(snbFile, snbFileName, xxx);
    }
while (length = ReadFile(inFd, inFileName, buffer, BUFSIZ, xxx))
    WriteFile(outFd, tempFileName, buffer, length, xxx);
CloseFile(inFd, inFileName, xxx);
CloseFile(outFd, tempFileName, xxx);
}


void
BuildLoids()
{
static char *attributeStrings[] = {
    "SSI",
    "ID",
    "LEVEL",
    0
    };

typedef enum {
    rssiAtt,
    ridAtt,
    rlevelAtt
    } Attributes;

int level[21];
ElementWithId type; /* the element type of current tag   */
Attributes attribute; /* the atttribute being processed  */
char   rid[65];       /* NAMELEN is 64 in the SDL DTD    */
char   rssi[BUFSIZ];  /* arbitrarily large               */
char   lbuff[32]; /* big enough to hold a number         */
char  *pl, *pc;  /* ptr into line and general char ptr   */
int    filePos; /* file offset of beginning of this line */
int    offset;  /* file offset of beginning of this tag  */
int    slen;    /* length of this tag or attribute name  */
int    length;
char **ppc;
char   line[LINEMAX], c;
FILE  *loidsFile;
LoidsPtr pLoidsEnd, pNewId;

rid[0]   = '\0';
rssi[0]  = '\0';
lbuff[0] = '\0';
for (type = virpage; type < unknown; type++) /* type == unknown at end */
    level[type] = -1;
level[virpage]  = 1;
level[block]    = 1;
level[form]     = 1;
offset          = 0;

inFile    = FopenFile(tempFileName,  O_RDONLY, xxx);
loidsFile = FopenFile(loidsFileName, O_WRONLY, xxx);

FwriteFile(loidsFile, loidsFileName, openLoids, sizeof(openLoids)-1, xxx);

loids.type = unknown; /* the first one is a dummy to make logic easier */
loids.next = NULL;
pLoidsEnd = &loids;

filePos = 0;
while ((length = GetALine(inFile, tempFileName, line, LINEMAX, xxx)) != 0)
    {
    length = StringToUpper(line);
    pl = line;
    while (*pl && (*pl++ != '<'));
    while (*pl)
	{
	ppc = elementWithIdStrings;
	while (*ppc)
	    {
	    slen = strlen(*ppc);
	    if ((strncmp(*ppc, pl, slen) == 0)
			       &&
		((c = (pl[slen] == ' ')) || (c == '\t') || (c == '>')))
		{
		type = (ElementWithId) (ppc - elementWithIdStrings);
		break;
		}
	    ppc++;
	    }
	if (*ppc)
	    {
	    if (type == virpage)
		{
		offset = filePos + (pl - line) - 1; /* back up over '<' */
		level[block] = 1;
		level[form]  = 1;
		}
	    pl += slen;
	    while (((c = *pl) == ' ') || (c == '\t')) pl++;
	    while (c != '>')
		{
		ppc = attributeStrings;
		while (*ppc)
		    {
		    slen = strlen(*ppc);
		    if (strncmp(*ppc, pl, slen) == 0)
			{
			pl += slen;
			if (((c = *pl) == '=') || (c == ' ') || (c = '\t'))
			    break;
			}
		    ppc++;
		    }
		if (*ppc)
		    {
		    attribute = (Attributes) (ppc - attributeStrings);
		    while ((c = *pl++) != '=');
		    while (((c = *pl++) != '"') && (c != '\''));
		    switch (attribute)
			{
			case rssiAtt:
			    pc = rssi;
			    break;
			case ridAtt:
			    pc = rid;
			    break;
			case rlevelAtt:
			    pc = lbuff;
			    break;
			}
		    while (*pl != c) *pc++ = *pl++;
		    *pc = '\0';
		    if (attribute == rlevelAtt)
			level[type] = atoi(lbuff);
		    }
		while (((c = *pl) != ' ') && (c != '\t') && (c != '>')) pl++;
		while (((c = *pl) == ' ') || (c == '\t')) pl++;
		}
	    if (rid[0])
		{ /* write the loids entry to the loids file */
		fprintf(loidsFile,
			"\n<ID TYPE=\"%s\" RID=\"%s\" ",
			elementWithIdStrings[type],
			rid);
		if (rssi[0])
		    fprintf(loidsFile, "RSSI=\"%s\" ", rssi);
		if (level[type] >= 0)
		    fprintf(loidsFile, "RLEVEL=\"%d\" ", level[type]);
		fprintf(loidsFile, "OFFSET=\"%d\">", offset);

		/* and save a copy in a structure for later processing */
		pNewId = malloc(sizeof(LoidsRec));
		pNewId->type   = type;
		pNewId->rlevel = level[type];
		pNewId->offset = offset;
		pNewId->rid = malloc(strlen(rid)+1);
		strcpy(pNewId->rid, rid);
		if (rssi[0])
		    {
		    pNewId->rssi = malloc(strlen(rssi)+1);
		    strcpy(pNewId->rssi, rssi);
		    }
		else
		    pNewId->rssi = NULL;
		pNewId->next = NULL;
		pLoidsEnd->next = pNewId;
		pLoidsEnd = pNewId;
		}
	    if (level[type] >= 0)
		level[type] = 1;
	    rid[0]  = '\0';
	    rssi[0] = '\0';
	    type    = unknown;
	    }
	while (*pl && (*pl++ != '<'));
	}
    filePos += length;
    }
FwriteFile(loidsFile, loidsFileName, closeLoids, sizeof(closeLoids)-1, xxx);
FcloseFile(loidsFile, loidsFileName, xxx);
}


void
LocateVstructElements()
{
int   filePos, offset, length, slen;
char  c, *lp, **ppc;
ElementOfInterest tag;
char  line[LINEMAX];

rewind(inFile);
filePos = 0;

tag = start_of_document;
while ((length = GetALine(inFile, tempFileName, line, LINEMAX, xxx)) != 0)
    {
    length = StringToUpper(line);
    lp = line;
    while (*lp && (*lp != '<')) lp++;
    while (*lp)
	{
	ppc = elementOfInterestStrings;
	while (*ppc)
	    {
	    slen = strlen(*ppc);
	    if (strncmp(*ppc, lp, slen) == 0)
		{
		tag = (ElementOfInterest) (ppc - elementOfInterestStrings);
		if (tag >= eloidsTag)
		    break;
		if (((c = lp[slen]) == '>') || (c == ' ') || (c == '\t'))
		    break;
		}
	    ppc++;
	    }
	if (tag == first_virpage)
	    break;
	if (*ppc)
	    {
	    offset = filePos + (lp - line);
	    if (tag >= eloidsTag)
		{
		offset += slen;
		if (lp[slen] == '\n') offset++;
		}
	    offsets[tag] = offset;
	    }
	lp++;
	while (*lp && (*lp != '<')) lp++;
	}
    if (tag == first_virpage)
	break;
    filePos += length;
    }
FcloseFile(inFile, tempFileName, xxx);
}


void
IterateLoids()
{
int       replacing, length, incr, tmp;
FILE     *loidsFile;
LoidsPtr  pLoids;

/* compute how much we are deleting from and adding to the <vstruct> */
replacing = offsets[eloidsTag] - offsets[loidsTag];
if (haveIndex)
    {
    if (offsets[indexTag] != 0)
	replacing += (offsets[eindexTag] - offsets[indexTag]);
    replacing -=  FileSize(sortedIdxFileName, xxx);
    }
if (tossSize != 0)
    {
    if (offsets[tossTag] != 0)
	replacing += (offsets[etossTag] - offsets[tossTag]);
    replacing -= tossSize;
    }
incr = length = FileSize(loidsFileName, xxx) - replacing;

/* Iterate over the <loids>, updating the offsets by "incr" until no
 * change in the size of the <loids>.  The "incr" may be positive or
 * negative based on original sizes of the elements and their
 * replacements.
*/
while (incr)
    {
    loidsFile = FopenFile(loidsFileName, O_WRONLY, xxx);
    FwriteFile(loidsFile,
	       loidsFileName,
	       openLoids,
	       sizeof(openLoids)-1,
	       xxx);
    pLoids = loids.next; /* first one is a dummy (simplified some logic) */
    while (pLoids)
	{
	pLoids->offset += incr;
	fprintf(loidsFile,
		"\n<ID TYPE=\"%s\" RID=\"%s\" ",
		elementWithIdStrings[pLoids->type],
		pLoids->rid);
	if (pLoids->rssi)
	    fprintf(loidsFile, "RSSI=\"%s\" ", pLoids->rssi);
	if (pLoids->rlevel >= 0)
	    fprintf(loidsFile, "RLEVEL=\"%d\" ", pLoids->rlevel);
	fprintf(loidsFile, "OFFSET=\"%d\">", pLoids->offset);
	pLoids = pLoids->next;
	}
    FwriteFile(loidsFile,
	       loidsFileName,
	       closeLoids,
	       sizeof(closeLoids)-1,
	       xxx);
    FcloseFile(loidsFile, loidsFileName, xxx);
    tmp = FileSize(loidsFileName, xxx) - replacing;
    incr = tmp - length;
    length = tmp;
    }
}


void
IncorporateVstructElements()
{
char      buffer[BUFSIZ], *pc;
int       length, delta;
int       inFd, idxFd, tossFd;
FILE     *outFile;
LoidsPtr  pLoids;

inFd = OpenFile(tempFileName, O_RDONLY, xxx);
pc = outFileName;
if ((*outFileName == '-') && (outFileName[1] == '\0'))
    {
    outFile = stdout;
    outFileName = "<stdout>";
    }
else
    outFile = FopenFile(outFileName, O_WRONLY, xxx);

/* copy inFd to outFile up to start of <loids> */
delta = offsets[loidsTag];
while (length = ReadFile(inFd, inFileName, buffer, MIN(BUFSIZ,delta), xxx))
    {
    FwriteFile(outFile, outFileName, buffer, length, xxx);
    delta -= length;
    }

/* write out the start-tag for <loids> */
FwriteFile(outFile,
	   outFileName,
	   openLoids,
	   sizeof(openLoids)-1,
	   xxx);

/* and write out the new <loids> */
pLoids = loids.next; /* first one is a dummy (simplified some logic) */
while (pLoids)
    {
    fprintf(outFile,
	    "\n<ID TYPE=\"%s\" RID=\"%s\" ",
	    elementWithIdStrings[pLoids->type],
	    pLoids->rid);
    if (pLoids->rssi)
	fprintf(outFile, "RSSI=\"%s\" ", pLoids->rssi);
    if (pLoids->rlevel >= 0)
	fprintf(outFile, "RLEVEL=\"%d\" ", pLoids->rlevel);
    fprintf(outFile, "OFFSET=\"%d\">", pLoids->offset + delta);
    pLoids = pLoids->next;
    }

/* followed by the end-tag for <loids> */
FwriteFile(outFile,
	   outFileName,
	   closeLoids,
	   sizeof(closeLoids)-1,
	   xxx);
unlink(loidsFileName);

/* and skip over the old <loids> in inFd */
lseek(inFd, offsets[eloidsTag], SEEK_SET);

delta = offsets[etossTag];
if (tossSize == 0) /* we don't have a new <toss> and ... */
    {
    if (delta) /* an old <toss> is in the document, copy it to outFile */
	{
	delta -= offsets[eloidsTag];
	while (length = ReadFile(inFd,
				 inFileName,
				 buffer,
				 MIN(BUFSIZ,delta),
				 xxx))
	    {
	    FwriteFile(outFile, outFileName, buffer, length, xxx);
	    delta -= length;
	    }
	}
    }
else /* we have a new <toss> and ... */
    {
    if (delta) /* an old <toss> is in the document, blow it away */
	lseek(inFd, delta, SEEK_SET);

    /* and copy over the new <toss> */
    tossFd = OpenFile(tossFileName, O_RDONLY, xxx);
    while (length = ReadFile(tossFd, tossFileName, buffer, BUFSIZ, xxx))
	FwriteFile(outFile, outFileName, buffer, length, xxx);
    CloseFile(tossFd, tossFileName, xxx);
    }

/* if we have an <lophrases>, copy it over to outFile */
if (offsets[elophrasesTag])
    {
    delta = offsets[elophrasesTag] - offsets[lophrasesTag];
    while (length = ReadFile(inFd,
			     inFileName,
			     buffer,
			     MIN(BUFSIZ,delta),
			     xxx))
	{
	FwriteFile(outFile, outFileName, buffer, length, xxx);
	delta -= length;
	}
    }

/* if we have a new index, write it out to outFile */
if (haveIndex)
    {
    idxFd = OpenFile(sortedIdxFileName, O_RDONLY, xxx);
    while (length = ReadFile(idxFd, sortedIdxFileName, buffer, BUFSIZ, xxx))
	FwriteFile(outFile, outFileName, buffer, length, xxx);
    unlink(sortedIdxFileName);
    }

/* if we have a new index and there was an old index, blow it away */
if (offsets[eindexTag] && haveIndex)
    lseek(inFd, offsets[eindexTag], SEEK_SET);


/* copy the remainder of inFd to outFile */
while (length = ReadFile(inFd, tempFileName, buffer, BUFSIZ, xxx))
    FwriteFile(outFile, outFileName, buffer, length, xxx);

CloseFile(inFd, tempFileName, xxx);
FcloseFile(outFile, outFileName, xxx);
unlink(tempFileName);
}


main(int argc, char **argv)
{
MakeFileNames(argc, argv);
BuildIndex();
LookForTossFile();
HandleSNB();
RemoveSuperfluousBlocks();
BuildLoids();
LocateVstructElements();
IterateLoids();
IncorporateVstructElements();
return 0;
}
