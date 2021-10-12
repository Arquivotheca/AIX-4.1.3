static char sccsid[] = "@(#)86	1.4  src/bos/usr/ccs/lib/libs/checktime.c, libs, bos411, 9428A410j 3/14/94 17:09:55";
/*
 * COMPONENT_NAME: (LIBS) Security Library Functions 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <langinfo.h>
#include <sys/errno.h>
#include "checktime.h"
#include "libs.h"

static char *gettoken(char *, struct token *, int);

/******************************** GLOBALS *****************************/

/**** State transition table for DFA ****/
int Transition[NUMSTATES][NUMTOKENS]= {
/*         ,   e   -   :   !   m   j   h  */
/* S1 */ {-1, E1, -1, H2, H1, M1, J1, -1},
/* M1 */ {S1, E1, M2, H2, -1, -1, -1, -1},
/* M2 */ {-1, -1, -1, -1, -1, M3, -1, -1},
/* M3 */ {S1, E1, -1, H2, -1, -1, -1, -1},
/* H1 */ {-1, -1, -1, H2, -1, M1, J1, H3},
/* H2 */ {-1, -1, -1, -1, -1, -1, -1, H3},
/* H3 */ {-1, -1, H4, -1, -1, -1, -1, -1},
/* H4 */ {-1, -1, -1, -1, -1, -1, -1, H5},
/* H5 */ {S1, E1, -1, -1, -1, -1, -1, -1},
/* J1 */ {S1, E1, J2, H2, -1, -1, -1, -1},
/* J2 */ {-1, -1, -1, -1, -1, -1, J3, -1},
/* J3 */ {S1, E1, -1, H2, -1, -1, -1, -1},
};

/*
 * Default day and month names.  These should be in all lower case as the code
 * does not handle upper case to lower case conversions for these strings (only
 * for the strings entered by the user).
 */
char *monthnames[]={
   "january", 
   "february", 
   "march", 
   "april", 
   "may", 
   "june", 
   "july", 
   "august",
   "september", 
   "october", 
   "november", 
   "december" 
};

char *daynames[]={
   "sunday",
   "monday", 
   "tuesday", 
   "wednesday", 
   "thursday", 
   "friday", 
   "saturday"
};

/* Max daynum for a given month */
int monthdays[]={31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/***************************************************************************
*
* _checktime
*
* PARMETERS:
*    timestring: (IN) A null separated, double null
*              terminated restrictions string.  This string
*              is in 'database' form - it contains no 
*              weeknames or daynames.  example:
*              
*              w-w,mmdd-mmdd:hour-hour,!mmdd
*              1-3,0200-0313:0000-0600,!1125
*              |-| |-----------------| |---|
*               |           |            |-> DENY month 11(Dec), day25 (Xmas)
*               |           |
*               |           |-> ALLOW month 2(march, no day so first day) thru
*               |                     month 3(april) 13th, from midnight-6am.
*               |                     
*               |
*               |-> ALLOW weekday 1(mon) thru 3(wed).
*
*               Months and weekdays start with 0 while daynumbers start with 1.
*
* PURPOSE:
*    This is the main routine that calls the parser and
*    the evaluator.  This will perform a full check and
*    return whether or not the user should be allowed in.
*
* RETURNS: ALLOW if the caller is allowed to login.
*          DENY  if the caller is denied access.
*          ERROR if there was an error parsing timestring.
*
**************************************************************************/
int
_checktime (char *timestring)
{
   int              lastentry;            /* Last valid entry from parsetime */
   struct entry_rec entries[MAXENTRIES];  /* Parsed tokens from parsetime    */

   /*
   **   If timestring is null pointer or 
   **   pointer to null string, allow access
   */
   if ( !timestring || !*timestring )
      return(ALLOW);

   /*
   **   String wasn't null, so we have something
   **   we need to go parse.
   */
   if ((lastentry=parsetime(timestring, entries)) >=0) {
      /* parsetime returned >=0, indicates a good parse */
      return(evaltime(lastentry, entries));
   }
   else {       /* lastentry < 0 indicates a bad parse, reject 'em */
      return(ERROR);
   }
}

/***************************************************************************
*
* parsetime
*
* PARMETERS:
*    timestring: (IN) A restrictions string (See checktime()
*                for a desription of the format of
*                timestring) to be parsed.
*    entries:    (IN/OUT) An array of entry structures repre-
*                senting the restrictions string.  This is
*                filled in and returned to the caller.
* PURPOSE:
*    This routine parses the restriction string passed
*    in timestring.  
*
* CALLED BY: checktime
*
* CALLS: gettoken
*
* NOTES:
*    This routine is implemented as a table driven
*    finite state machine(DFA).  All of the semantics
*    for the DFA are performed in the large switch 
*    statement based upon the 'next state'.
*
* RETURNS:  Last valid entry (NOT number of valid entries), if parse OK.
*           ERROR if error in parse.
*
**************************************************************************/

static int
parsetime( char *timestring, struct entry_rec entries[] )
{

   int           state=S1;        /* Init state to the starting state      */
   struct token  token;           /* Current input token                   */
   int           curentry=0;      /* Current working entry                 */
   int	         partentry=TRUE;  /* Flag indicating partial entry         */
   int           seencolon=FALSE; /* Indicates to gettoken that we have    */
                                  /* seen a colon.                         */


   /* 
   **   Initialize entry we are building 
   */
   INITENTRY(0);

   while(TRUE) {
      /* 
      **   Get next token, NULL==error.  gettoken() consumes a token and
      **   passes back a pointer to the next character to be consumed.
      */
      if ((timestring=gettoken(timestring, &token, seencolon)) == NULL)
         return(ERROR);

      switch (Transition[state][token.type]) {
         case E1:   /* End state */
            if (partentry) 
               /* 
               **   This is a partial entry - copy start info to end 
               */
               FINISHENTRY(curentry);
            return(curentry);  /* Good parse, return curentry */
	    break;

         case S1:   /* Start state */
            /* 
            **   Got a comma, process this one and move to next entry 
            */
            if (partentry) 
               /* 
               **   This is a partial entry - copy start info to end 
               */
               FINISHENTRY(curentry);
            curentry++;
            if (curentry >= MAXENTRIES)  /* Too many entries */
               return(ERROR);

            INITENTRY(curentry);         /* Start over-reinit variables    */
            partentry=TRUE;              /* Reset entry type indicator      */
            seencolon=FALSE;             /* Reset hour vs month-day context */
            break;

         case M1:  /* Got a start month */
            entries[curentry].start.month =token.val1;
            entries[curentry].start.daynum=token.val2;
            break;

         case M2:  /* Got a HYPHEN */
            partentry=FALSE;       /* Must not be a partial entry */
            break;

         case M3:  /* Got end month */
            entries[curentry].end.month =token.val1;
            entries[curentry].end.daynum=token.val2;
            break;

         case H1: /* Got a '!' */
            entries[curentry].type=DENY;
            break;

         case H2:  
            /* 
            **   Got a COLON, indicate we've seen a colon.  This is passed
            **   to gettoken() so that it can distinguish between the end
            **   of a month/day range and the end of an hour range.
            */
            seencolon=TRUE;
            break;

         case H3: /* Got start hour */
            entries[curentry].start.hour=token.val1;
            break;

         case H4: 
            /* 
            **   Got a HYPHEN.  We don't set partentry to FALSE here
            **   since hours *MUST* be in a full pair or the parser
            **   will complain.
            */
            break;
  
         case H5: /* Got an end hour */
            /* 
            **   If the end hour < start hour, return ERROR.
            */
            if (token.val1<entries[curentry].start.hour)
               return(ERROR);
            /* Got a good hour range, set up entry */
            entries[curentry].end.hour=token.val1;
            break;

         case J1: /* Got a start day of week */
            entries[curentry].start.day=token.val1;
            break;

         case J2: /* Got a HYPHEN */
            partentry=FALSE;     /* Must not be a partial entry */
            break;

         case J3: /* Got an end day of week */
            entries[curentry].end.day=token.val1;
            break;

         case INVALID:        /* INVALID transition */
            return(ERROR) ;   /* Error in parsing   */
            break;

	 default:
            /* Should NEVER get here - if we do, Transition Table is corrupt */
            return(ERROR);
            break;
      }
      /* Finally, go to next state */
      state=Transition[state][token.type];
   }
}

/**********************************************************************
*
* gettoken
*
* PURPOSE: retrieves the next token from string pointed to by p
*
* PARAMETERS:
*    string    : (IN/OUT) Where to start the scan
*    token     : (OUT) Token passed back to caller
*    seencolon : (IN) Indicates that the parser has seen a colon in
*                the current entry.  This flag is used to distinguish 
*                the end portion of a month/day range from the end 
*                portion of a time range.
*
* CALLED BY: parsetime
*    
* CALLS: Nothing
*
* NOTES: Also 'consumes' string p by advancing p and returning it
*        to the caller.  The caller should turn around and call
*        gettoken() again with this updated pointer, p.
*
* RETURNS:  On success, pointer to next character to consume.
*           On failure, NULL
*
***********************************************************************/

static char *
gettoken(char *string, struct token *token, int seencolon)
{
   int num=0;      /* Temporary holding place for a number token       */
   int month, day; /* Temporary holding places for month and day value */
   char *start;    /* Start of token                                   */

   while (isspace(*string))        /* Skip white space */
      string++;

   if (isdigit(*string)) {         /* This must be a number                  */
      num=*string-'0';             /* Initialize num to first digit          */
      start=string++;              /* Remember our start point               */
      while (isdigit(*string)) {   /* Collect rest of number                 */
         num=(num*10)+(*string-'0');
         string++;
      }

      if ((string-start)==1) {   
         /* 
         **   If length == 1, must be a weekday (0-6) 
         */
         if ( (num>=0) && (num<=6) ) {  /* Valid weekday(0-6)? */
            token->type=DAY;
            token->val1=num;
            return(string);
         }
         else
            return(NULL);  /* Bad weekday number */
      }

      /*
      **   Maybe it's an hour or month/day spec (if length == 4).
      */
      if ((string-start)!=4)
         return(NULL); /* Or maybe not! */

      if (seencolon) {  
         /* 
         **   Must be an hour spec since we've 
         **   seen a colon in current context
         */
         token->type=HOUR;
         /* Need to check magnitude and minutes < 60 */
         if ( (num>2359) || (*(string-2)>'5') )
            return(NULL);
         token->val1=num;
      }
      else {
         /* 
         **   Must be a month/day spec since we haven't
         **   seen a colon in the current context
         */
         month=num/100;           /* Extract month number        */
         if (month>11) 
            return(NULL);         /* No month > 11 (december)    */
         day=num%100;             /* Now get day number          */
         if (day>monthdays[month]) 
            return(NULL);         /* Day must be valid for month */
         token->type=MONTHDAY;
         token->val1=month;
         token->val2=day;
      }
      return(string);
   } /* End number processing */
         
   /* 
   **   Not a number, must be a special 
   **   character or alpha (invalid)
   */
   switch (*string) {
      case '\0':
         /*
         **   A single null will be interpreted 
         **   as a comma (an entry separator).  A 
         **   double null will be treated as EOI 
         **   (end of input).
         */
         if (*(string+1)=='\0')
            token->type=EOI;
         else
            token->type=COMMA;
         break;

      case '-':
         token->type=HYPHEN;
         break;

      case ':':
         token->type=COLON;
         break;

      case '!':
         token->type=EXCLAIM;
         break;
 
      default:
         return(NULL);   /* Bad character */
         break;
   }
   return(++string);
}


/**********************************************************************
*
* evaltime
*
* PURPOSE: checks the current system time against the restriction
*          entries and determines whether or not the user should
*          be allowed access in.
*
* PARAMETERS:
*    lastentry : (IN) Number of entries.
*    entries   : (IN) Array of restrictions constructed by parsetime().
*
* CALLED BY: checktime
*
* CALLS: checkentries
*
* RETURNS:  ALLOW if the user is allowed to login
*           DENY otherwise
*
***********************************************************************/

static int
evaltime(int lastentry, struct entry_rec entries[] )
{
   int       i, tod;
   struct tm curtime, *t1;

#ifdef DEBUG
   printf("Type\tMonth\tDaynum\tDay\tHour\tMonth\tDaynum\tDay\tHour\n");
   for (i=0;i<=lastentry;i++) {
      if (entries[i].type==ALLOW)
         printf("ALLOW");
      else
         printf("DENY");
      printf("\t");
      printf("%d\t%d\t%d\t%d\t",entries[i].start.month,entries[i].start.daynum,
                                entries[i].start.day,entries[i].start.hour);
      printf("%d\t%d\t%d\t%d\t",entries[i].end.month,entries[i].end.daynum,
                                entries[i].end.day,entries[i].end.hour);
      printf("\n");
   }
#endif

   /*
   **   Get the time of day 
   */
   tod = time(0);
   t1=localtime(&tod);
   memcpy(&curtime, t1, sizeof(struct tm));

   /*
   **   First look for DENY entries.  They have
   **   priority over ALLOW entries.
   */
   if (checkentries(lastentry, entries, DENY, &curtime)==TRUE)
      /* Found a matching deny entry - so bail */
      return(DENY);

   /*
   **   If there were no DENY entries (NONE) or 
   **   no *MATCHING* deny entries (FALSE), try ALLOW
   */
   if (checkentries(lastentry, entries, ALLOW, &curtime)==FALSE)
      /* There were allow entries, but NO match */
      return(DENY);

   /*
   **   There were no ALLOW entries (NONE) or 
   **   found a matching allow entry 
   */
   else
      return(ALLOW);
}


/**********************************************************************
*
* checkentries
*
* PURPOSE: Searches the array of restrictions entries for all     	
*          entries of the specified type that match the system
*          time.
*
* PARAMETERS:
*    lastentry : (IN) Number of entries in entries array.
*    entries   : (IN) Array of restrictions constructed by parsetime().
*    type      : (IN) Specifies type of entry (ALLOW or DENY) to search for.
*    time      : (IN/THRU) System time.
*
* CALLED BY: evaltime
*
* CALLS: checkhour, checkday, checkmonth
*
* RETURNS:  TRUE  if there are entries of specified type that match 
*                 the system time.
*           FALSE if there are entries of the specified type but none
*                 match the system time.
*           NONE  if there were no entries of the specified type.
*
***********************************************************************/

static int
checkentries(int              lastentry, 
            struct entry_rec entries[], 
            int              type, 
            struct tm        *time)
{
   int i;
   int empty=TRUE;

   for (i=0;i<=lastentry;i++) {
      if (entries[i].type==type) {
         empty=FALSE;   /* Indicate we have an entry of the specified type */
         /*
         **   Is this a month/day entry?
         */
         if (entries[i].start.month!=INITVAL) {
            if (checkmonth(&entries[i],time) && checkhour(&entries[i],time))
               return(TRUE);
         }
         /*
         **   Is this a week day entry?
         */
         else if (entries[i].start.day!=INITVAL) {
            if (checkday(&entries[i],time) && checkhour(&entries[i],time))
               return(TRUE);
         }
         /*
         **   If we get here we are guaranteed to have an hour range
         */
         else if (checkhour(&entries[i],time)) {
            return(TRUE);
         }
      }  /* End processing entry of correct type */
   }
   /* 
   **   No match, check to see if we saw 
   **   an entry of specified type.  If not,
   **   return NONE.  Otherwise, we saw 
   **   something - must not have matched.  
   */
   if (empty)
      return(NONE);
   else
      return(FALSE);
}
            

/**********************************************************************
*
* checkhour
*
* PURPOSE: Checks the current system time against hour spec in the 
*          specified restrictions entry.
*
* PARAMETERS:
*    entry : (IN) Restrictions entry to check.
*    time  : (IN) System time.
*
* CALLED BY: checkentries
*
* CALLS: Nothing.
*
* RETURNS:  TRUE  if there was no hour spec in the specified entry or
*                 if the hour spec matches the passed in system time.
*           FALSE if the hour spec in the specified entry doesn't
*                 match the system time.
*
* NOTE: checkentries() *DEPENDS* on checkhour returning TRUE if there
*       is no hour spec in the specified entry.
*
***********************************************************************/

static int 
checkhour(struct entry_rec *entry, struct tm *time)
{
   int hour;

   if (entry->start.hour==INITVAL)
      return(TRUE);
   /*
   **   Unlike months and days, hour specs are 
   **   not 'circular', so we are guaranteed to 
   **   have the entry's start hour < end hour
   */

   hour=time->tm_hour*100+time->tm_min;
   if ( (hour>=entry->start.hour) &&      /* After start... */
        (hour<=entry->end.hour) )         /* and before end */
      return(TRUE);                       /* good one...    */
   else                                   /* Else, sorry... */
      return(FALSE);
}       

         
   
/**********************************************************************
*
* checkmonth
*
* PURPOSE: Checks the current system time against the month spec in the
*          specified restrictions entry.
*
* PARAMETERS:
*    entry : (IN) Restrictions entry to check.
*    time  : (IN) System time.
*
* CALLED BY: checkentries
*
* CALLS: Nothing.
*
* RETURNS:  TRUE  if the month/daynumber spec matches the passed in system time.
*           FALSE if the month/daynumber spec in the specified entry doesn't
*                 match the system time.
*
* NOTE: Both the normal case (where the start < end) and the 'circular'
*       case must be handled.
*
*       In order to more easily compare the month/day number combination,
*       convert the month/day number to a 4 digit integer by multiplying
*       the month by 100 and adding the daynumber.  This will handle
*       leap year since feb 29 (0129) will always be less that mar 1(0201).
*       The only other case we have to worry about is where the end 
*       day number wasn't specified (jan1-feb).  In this case, we use
*       99 as the day number.  
*
***********************************************************************/

static int
checkmonth(struct entry_rec *entry, struct tm *time)
{
   int startday, endday, today;

   /*
   **   Convert month and day spec to mmdd integer 
   **   for easier comparison.  This allows easy
   **   handling of leapyear:  0128 and 0129 are 
   **   always less than 0201.
   */
   startday=entry->start.month*100+entry->start.daynum;
   endday  =entry->end.month*100+entry->end.daynum;
   /*
   **   If end daynum == 0 (indicating that no end day num was 
   **   specified), set it to 99.  This will be greater than any
   **   day this month and less than any day in the the next month.
   */
   if (entry->end.daynum == 0)
      endday=endday+99;
   today   =time->tm_mon*100+time->tm_mday;

   /*
   **   Non-circular case (start<=end)
   */
   if (startday<=endday) {
      if ( (today>=startday) && (today<=endday) )
         return(TRUE);
      else
         return(FALSE);
   }

   /*
   ** Circular case (end<start)
   */
   else {
      if ( (today>=startday) || (today<=endday) )
         return(TRUE);
      else
         return(FALSE);
   }
}



/**********************************************************************
*
* checkday
*
* PURPOSE: Checks the current system time against the weekday spec 
*          in the specified restrictions entry.
*
* PARAMETERS:
*    entry : (IN) Restrictions entry to check.
*    time  : (IN) System time.
*
* CALLED BY: checkentries
*
* CALLS: Nothing.
*
* RETURNS: TRUE  if the weekday spec matches the passed in system time.
*          FALSE if the weekday spec in the specified entry doesn't
*                match the system time.
*
**********************************************************************/

static int
checkday(struct entry_rec *entry, struct tm *time)
{
   /*
   **   Non-circular case (start<=end)
   */
   if (entry->start.day<=entry->end.day) {
      if ( (time->tm_wday>=entry->start.day) &&
           (time->tm_wday<=entry->end.day) )
         return(TRUE);
      else
         return(FALSE);
   }
   /*
   **   Circular case (end<start)
   */
   else {
      if ( (time->tm_wday>=entry->start.day) ||
           (time->tm_wday<=entry->end.day) )
         return(TRUE);
      else
         return(FALSE);
   }
}

/***************************************************************************
*
* normalizeinput
* 
* PURPOSE:
*    This routine takes a user-entered restrictions string (possibly
*    containing non-English month/day names and non-English month/day
*    order) and converts it to the normalized format used by the parser.
*    This normalized string is NULL separated, double NULL terminated.
*    The memory for the normalized string, p, is allocated by this 
*    routine and should be free'd by the caller.
*
* PARAMETERS:
*    user : (IN) String to be normalized.
*    norm : (OUT) The normalized string
*
* CALLED BY: usertodb
*
* CALLS: load_name_arrays, free_name_arrays
*
* RETURNS: 0 if the routine succeeds.  The normalized string is in *norm.
*          ERROR if failure
*
* NOTES:
*    General flow is to collect all names and numbers until we see a
*    'special' character whereupon we translate the month or weekday
*    name or daynumber into the appropriate normalized form.  We leave
*    hour specs alone (they're all digits anyway).
*
**************************************************************************/

static int
normalizeinput(char *user, char **norm)
{
   char *tuser;               /* Temp pointer into user string          */
   char *tnorm;               /* Temp pointer into normalized string    */
   char name  [BIGBUFSIZ],    /* Place to build month/day name          */
        number[BIGBUFSIZ];
   char *user_months[12];     /* User representation of month names     */
   char *user_days[7];        /* User representation of day names       */
   int nump=0, namep=0;       /* Index into day and month strings       */
   int i,j;                   /* Temp holder for month or day index     */
   int seenalpha=FALSE;       /* Indicates we've seen some type of name */
   int seencolon=FALSE;       /* Holds previous special char            */
   int	clen;                 /* Length of a multibyte character        */
   char lowermbc[MB_LEN_MAX]; /* For towlower()'ed mb character         */
   wchar_t	wc;           /* Temp wchar_t so we can use iswalpha()  */

   if (load_name_arrays(user_months,user_days))
      return(ERROR);

   /* 
   **   Normalized coudn't possibly be > 2 * length 
   **   of input string. In fact, often it is much shorter.
   */
   if ( (*norm=(char *)malloc( 2 * (strlen(user)+1))) == NULL) {
      free_name_arrays(user_months,user_days);
      return(ERROR);
   }

   /* Initialize everything */
   tuser=user;
   tnorm=*norm;

   while (TRUE) {
      /* 
      **   We process what we've collected when 
      **   we encounter a 'special char'.  Otherwise,
      **   we just keep collecting characters.
      */
      switch (*tuser) {
         case '!' :        /* Special characters */
         case '-' :        /* Start processing   */
         case ',' :
         case ':' :
         case '\0':

            if (*tuser==':')   /* Indicate that we have seen a colon */
               seencolon=TRUE;

            /*
            **   NULL terminate the name and number.
            */
            name[namep]  = '\0';
            number[nump] = '\0';

            if (seenalpha) { 

               /* 
               **   Previous chunk contained a name of some sort
               **   so we must have seen a day name or a month name.  
               **   Check to see if it is a month *AND NOT* a day name.
               */
               if ( ((i=isname(name,user_months,12)) >= 0) &&
                    ((j=isname(name,user_days,7)) == NONE) ) {
                  sprintf(tnorm, "%2.2d", i);          /* If it was a month */
                  tnorm+=2;                            /* convert it to int */
                  if (*number=='\0')                   /* Process daynum    */
                     strcpy(number,"00");              /* If none, say '00' */
                  else if (strlen(number)==1) {        /* Make length 2     */
                     number[2]='\0';
                     number[1]=number[0];
                     number[0]='0';
                  }
                  if (strlen(number)!=2) {             /* If processed len  */
                     free_name_arrays(user_months,user_days);
                     free(*norm);
                     return(ERROR);                     /* != 2, get out     */
                  }
   
                  strcpy(tnorm,number);                /* Else, copy it.    */
                  tnorm+=2;
               }
               /*
               **   Check to see if it is a day *AND NOT* a month name
               */
               else if ( ((i=isname(name,user_days,7)) >=0 ) &&
                         ((j=isname(name,user_months,12)) == NONE ) ) {
                  sprintf(tnorm, "%1.1d", i);          /* Must be a day  */
                  tnorm+=1;                            /* number         */
               }
               else {
                  free_name_arrays(user_months,user_days);
                  free(*norm);
                  return(ERROR);
               }
            }
            else {   
               /* 
               **   Check for an hour range.  We must know if we've
               **   seen a colon in the current context.  Without
               **   this check, there is no way of distinguishing 
               **   between 0000-0100(jan-feb) and 0000-0100 (mid-
               **   night until 1:00am in the final string.
               */
               if (seencolon) {
                  strcpy(tnorm, number);
                  tnorm+=strlen(number);
                  if (*tuser==',' || *tuser=='\0')
                     seencolon=FALSE;
               }
               else if (*tuser!='!') {
                  free_name_arrays(user_months,user_days);
                  free(*norm);
                  return(ERROR);
               }
            }
                     
            if (*tuser=='\0') {  /* Done? If so, return good */
               *tnorm++='\0';    /* But remember to double   */
               *tnorm='\0';      /* null terminate first     */
               free_name_arrays(user_months,user_days);
               return(0);
            }
            if (*tuser==',') {
               *tnorm++='\0';
               *tuser++;
            }
            else
               *tnorm++=*tuser++;  /* Not done, so copy the special character */
                  
            namep=nump=0;
            seenalpha=FALSE;
      
            break;

         default:
            /*
            **   In the default case we just collect names
            **   and/or numbers.  These are later processed when
            **   we see a 'special' character.
            */
            if (MB_CUR_MAX==1) {
               /*
               **   Single byte code set.  Handle chars normally.
               **   Using two different paths for SBCS and MBCS (below).
               **   This is for performance for in the SBCS case.
               */
               if (isalpha(*tuser)) {   /* Collect all alphas into name */
                  name[namep++]=tolower(*tuser);
                  tuser++;
                  if (namep>=BIGBUFSIZ) {
                     free_name_arrays(user_months,user_days);
                     free(*norm);
                     return(ERROR);
                  }
                  seenalpha=TRUE;    /* And indicate we have seen an alpha */
               }
               else if (isdigit(*tuser)) {  /* Collect all digits in number */
                  number[nump++]=*tuser++;
                  if (nump>=BIGBUFSIZ) {
                     free_name_arrays(user_months,user_days);
                     free(*norm);
                     return(ERROR);
                  }
               }
               else {                   /* Some wacked out char...blow chow */
                  free_name_arrays(user_months,user_days);
                  free(*norm);
                  return(ERROR);
               }
            }   /* End SBCS case */
            else {
               /*
               **   Multibyte code set.  Must handle mb chars and 
               **   conversion to 'lower' case.
               */
               mbtowc(&wc,tuser,MB_CUR_MAX);/* Get a wchar version of char */
               if (iswalpha(wc)) {    /* Collect all alphas into name      */
                  wctomb(&lowermbc,towlower(wc)); 
                  clen=mblen(lowermbc,MB_LEN_MAX);
                  if (namep+clen>=BIGBUFSIZ) {
                     free_name_arrays(user_months,user_days);
                     free(*norm);
                     return(ERROR);
                  }
                  memcpy(name,lowermbc,clen);
                  tuser+=clen;
                  namep+=clen;
                  seenalpha=TRUE;   /* And indicate we have seen an alpha */
               }
               else if (isdigit(*tuser)) {  /* Collect all digits in number */
                  number[nump++]=*tuser++;
                  if (nump>=BIGBUFSIZ) {
                     free_name_arrays(user_months,user_days);
                     free(*norm);
                     return(ERROR);
                  }
               }
               else {                   /* Some wacked out char...blow chow */
                  free_name_arrays(user_months,user_days);
                  free(*norm);
                  return(ERROR);
               }
            }  /* End MBCS case */

            break;

      }  /* End switch */
   }  /* End while */
}

/***************************************************************************
*
* free_name_arrays
* 
* PARAMETERS:
*    months : (IN/OUT) Array of month names to be free()d.
*    days   : (IN/OUT) Array of day names to be free()d.
*
* PURPOSE:
*    This routine frees the memory allocated by load_name_arrays().
*
* RETURNS:
*    Nothing
*
* CALLED BY: normalizeinput, format, load_name_arrays
*
* CALLS: Nothing
*
**************************************************************************/

static int
free_name_arrays(char *months[], char *days[])
{
   int i;

   for (i=0;i<12;i++)
      if ( months[i] )
         free(months[i]);
   for (i=0;i<7;i++)
      if ( daynames[i] )
         free(daynames[i]);
   return;
}

/***************************************************************************
*
* load_name_arrays
* 
* PARAMETERS:
*    months : (IN/OUT) Array of month names to be loaded from message
*             catalog.
*    days   : (IN/OUT) Array of day names to be loaded from message
*             catalog.
*
* PURPOSE:
*    This routine loads the month name and day name arrays from the 
*    message catalog that was opened in the calling routine.
*
* RETURNS:
*    0 if the routine succeeds.
*    ERROR if failure
*
* CALLED BY: normalizeinput, format
*
* CALLS: free_name_arrays
*
* NOTES: 
*    This routine DEPENDS on the month names starting with M_JAN (january)
*    proceeding thru M_DEC (december) being sequential and lower case in the
*    message catalog (libs.msg).
*
*    The same is true of the day names M_SUN (sunday) thru M_SAT (saturday).
*
*    The month and day names can be anywhere in the file, however.
*
**************************************************************************/

static int
load_name_arrays(char *months[], char *days[])
{
   int i,m,d;
   char *p;

   for (i=0;i<12;i++)
      months[i] = NULL;
   for (i=0;i<7;i++)
      days[i] = NULL;
   for (i=0,m=M_JAN;i<12;i++,m++) {
      p=MSGSTR(m,monthnames[i]);
      if ( (months[i]=strdup(p)) == NULL) {
         free_name_arrays(months, days);
         return(ERROR);
      }
   }
   for (i=0,d=M_SUN;i<7;i++,d++) {
      p=MSGSTR(d,daynames[i]);
      if ( (days[i]=strdup(p)) == NULL) {
         free_name_arrays(months, days);
         return(ERROR);
      }
   }
   return(0);
}

/***************************************************************************
*
* isname
* 
* PARAMETERS:
*    s      : (IN) Name to check
*    array  : (IN) Array of day names to search
*    num    : (IN) Number of names in array
*
* PURPOSE:
*    This routine searches array for the name passed in s and returns
*    the index of the matching entry.  s may be a left substring as long
*    as it is unique within array.  For example, s may be 'jul' for an
*    array of month names.
*
* CALLED BY: normalizeinput
*
* CALLS: Nothing
*
* RETURNS:
*    Index of the matching string if there is one and only one match.
*    ERROR if there is no match or more than one match.
*
**************************************************************************/

static int
isname(char *s, char *array[],int num)
{
   int i;
   int m=NONE;

   /*
   **   We should be able to use strstr() to do the search
   **   since we know we are starting on a char or mb char
   **   boundary.  So this should work in either the SBCS
   **   or the MBCS case.  The NULL terminator (NULL, or '\0')
   **   cannot appear in the encoding of any multibyte character
   **   since it is in the unique code point range 0x00-0x3f.
   */
   for (i=0; i<num; i++) {
      if (strstr(array[i], s)==array[i]) {
         if (m==NONE)
            m=i;
         else
            return(ERROR);
      }
   }
   return(m);
}

/***************************************************************************
*
* format
* 
* PARAMETERS:
*    lastentry: (IN) Number of restrictions entries
*    entries  : (IN) Array of restrictions entries constructed by parsetime()
*    buf      : (IN/OUT) Buffer for formatted output string
*    type     : (IN) Indicates user or database formatting
*
* PURPOSE:
*    This routine takes an array of restricions entries and formats them
*    either for user output (type==USERFORMAT) or database output
*    (type==DBFORMAT).  User output is intended for human consumption while
*    database output is what is stored in /etc/security/user and 
*    /etc/security/login.cfg.
*
* CALLED BY: usertodb, dbtouser
*
* CALLS: load_name_arrays, free_name_arrays
*
* RETURNS:
*    0 on success
*    ERROR on failure
*
**************************************************************************/

static int
format(int lastentry, struct entry_rec entries[], char **buf, int format)
{
   int i;                  /* Index into entries array                */
   int bufsiz;             /* Amount of mem necessary to hold outpout */
   char *tmp;              /* Temp string                             */
   char *user_months[12];  /* User representation of month names      */
   char *user_days[7];     /* User representation of day names        */
   char *dfmt;             /* Ptr to date format string               */
   char *mfp, *dfp;        /* Ptr to month format substring and day   */
                           /*    format substring in the returned     */
                           /*    date format string(dfmt).            */
   int  monthfirst=TRUE;   /* Default order of month/day              */
   char month[BIGBUFSIZ],  /* Place to build month name and day num   */
        day[BIGBUFSIZ];

   if (load_name_arrays(user_months,user_days))
      return(ERROR);

   /* Get the approriate relative of month and day */
   dfmt=nl_langinfo(D_FMT);
   if (strstr(dfmt,"%m") > strstr(dfmt,"%d"))
      monthfirst=FALSE;

   /*
   ** The user (or database) representation of any one entry must
   ** be less than 2*longest monthname + 2*longest day name + 4 special 
   ** characters + 1 NULL or comma.  This corresponds to the longest type of 
   ** entry: !md-md:hour-hour.   
   */
   bufsiz=2*longest(user_months,12)+2*longest(user_days,7)+5;
   if ((*buf=(char *)malloc((lastentry+1)*bufsiz))==NULL) {
      free_name_arrays(user_months,user_days);
      return(ERROR);
   }

   if ((tmp=(char *)malloc(bufsiz))==NULL) {
      free_name_arrays(user_months,user_days);
      free(*buf);
      *buf = NULL;
      return(ERROR);
   }

   **buf='\0';

   for (i=0; i<=lastentry; i++) {
      if (i!=0)    /* Need to print a comma if this is the 2nd or later */
         strcat(*buf,",");

      if (entries[i].type==DENY)   /* Bang it if necessary */
         strcat(*buf,"!");

      /* Process month */
      if (entries[i].start.month!=INITVAL) {
         if (format==USERFORMAT)
            strcpy(month,user_months[entries[i].start.month]);
         else {  /* DB format */
            sprintf(tmp,"%2.2d",entries[i].start.month);
            strcat(*buf,tmp);
         }
      }

      /* Process associated day number */
      if (entries[i].start.daynum!=INITVAL) {
         if (format==USERFORMAT) {
            if (entries[i].start.daynum!=0) {
               sprintf(day,"%d",entries[i].start.daynum);
            }
            else
               *day='\0';
         }
         else {  /* DB format */
            sprintf(tmp,"%2.2d",entries[i].start.daynum);
            strcat(*buf,tmp);
         }

         /* Spit out month and day in correct order */
         if (format==USERFORMAT) {
            if (monthfirst) {
               strcat(*buf, month);
               strcat(*buf, day);
            }
            else {
               strcat(*buf, day);
               strcat(*buf, month);
            }
         }
      }

      /* Process weekday */
      if (entries[i].start.day!=INITVAL) {
         if (format==USERFORMAT)
            strcat(*buf,user_days[entries[i].start.day]);
         else {
            sprintf(tmp,"%1.1d",entries[i].start.day);
            strcat(*buf,tmp);
         }
      }
      if (!PARTIALRANGE(i)) {   
         /* 
         **   If end info != start info, need 
         **   to print the second half
         */
         strcat(*buf,"-");
         /* Process month */
         if (entries[i].end.month!=INITVAL) {
            if (format==USERFORMAT)
               strcpy(month,user_months[entries[i].end.month]);
            else { /* DB format */
               sprintf(tmp,"%2.2d",entries[i].end.month);
               strcat(*buf,tmp);
            }
         }

         /* Process associated day number */
         if (entries[i].end.daynum!=INITVAL) {
            if (format==USERFORMAT) {
               if (entries[i].end.daynum!=0) {
                  sprintf(day,"%d",entries[i].end.daynum);
               }
               else
                  *day='\0';
            }
            else { /* DB format */
               sprintf(tmp,"%2.2d",entries[i].end.daynum);
               strcat(*buf,tmp);
            }

            /* Spit out month and day in correct order */
            if (format==USERFORMAT) {
               if (monthfirst) {
                  strcat(*buf, month);
                  strcat(*buf, day);
               }
               else {
                  strcat(*buf, day);
                  strcat(*buf, month);
               }
            }
         }

         /* Process weekday */ 
         if (entries[i].end.day!=INITVAL) {
            if (format==USERFORMAT)
               strcat(*buf,user_days[entries[i].end.day]);
            else {
               sprintf(tmp,"%1.1d",entries[i].end.day);
               strcat(*buf,tmp);
            }
         }
      }

      /* Process entire hour range */
      if (entries[i].end.hour!=INITVAL) {
         sprintf(tmp,":%4.4d-",entries[i].start.hour);
         strcat(*buf,tmp);
         sprintf(tmp,"%4.4d",entries[i].end.hour);
         strcat(*buf,tmp);
      }
   }
   free(tmp);
   /* Double NULL terminate this thing */
   *(*buf+strlen(*buf)+1)='\0';
   free_name_arrays(user_months,user_days);
   return(0);
}

/***************************************************************************
*
* longest
* 
* PARAMETERS:
*    array: (IN) Array of month or day names
*    num  : (IN) Number of names in array
*
* PURPOSE:
*    This routine finds the longest name in array.
*
* CALLED BY: format
*
* CALLS: Nothing
*
* RETURNS:
*    Length of longest name in array
*
***************************************************************************/

static int
longest(char *array[], int num)
{
   int i;
   int longest=-1;
   int len;
   
   /*
   **   We should be able to user strlen() to determine the 
   **   number of *BYTES* needed to store the longest name
   **   in array[].  This is possible since NULL ('\0') is
   **   in the unique code point range (0x00-0x3f) and cannot
   **   appear in any encoding of a multibyte character.
   */
   for (i=0;i<num;i++)
      if ((len=strlen(array[i]))>longest)
         longest=len;
   return(longest);
}

/***************************************************************************
*
* dbtouser
* 
* PARAMETERS:
*    dbstring  : (IN) Database format string 
*    userstring: (OUT) User format string
*
* PURPOSE:
*    This routine converts a database format string (all digits, comma-
*    separated) to a user readable string.
*
* CALLED BY: external routines (i.e. chuser/lsuser/mkuser, etc).
*
* CALLS: parsetime, format
*
* RETURNS:
*    0 if success
*    ERROR otherwise
*
***************************************************************************/

char *
_dbtouser(char *user, char *dbstring)
{
   struct entry_rec entries[MAXENTRIES];
   int lastentry;
   char *userstring;

   if ((lastentry=parsetime(dbstring, entries))>=0) {
      format(lastentry, entries, &userstring, USERFORMAT);
      return(userstring);
   }
   return(NULL);
}

/***************************************************************************
*
* usertodb
* 
* PARAMETERS:
*    userstring: (IN) User format string 
*    dbstring  : (OUT) Database format string
*
* PURPOSE:
*    This routine converts a user format string to a database
*    format string (comma-separated). 
*
* CALLED BY: external routines (i.e. chuser/lsuser/mkuser, etc).
*
* CALLS: normalizeinput, parsetime, format
*
* RETURNS:
*    0 if success
*    ERROR otherwise
*
***************************************************************************/

int
_usertodb(char *userstring, char **dbstring)
{
   struct entry_rec entries[MAXENTRIES];
   char *normstring;
   int lastentry;

   if (!normalizeinput(userstring, &normstring)) {
      if ((lastentry=parsetime(normstring, entries))>=0) {
         format(lastentry, entries, dbstring, DBFORMAT);
         return(0);
      }
   }
   return(EINVAL);
}

/***************************************************************************
*
* dbtomkuser
*
* PARAMETERS:
*    dbstring  : (IN) Database format string
*    userstring: (OUT) User format string
*
* PURPOSE:
*    This routine converts a database format string (all digits, comma-
*    separated) to a user readable string.
*
* CALLED BY: external routines (mkuser).
*
* CALLS: parsetime, format
*
* RETURNS:
*    0 if success
*    EINVAL otherwise
*
***************************************************************************/


int
_dbtomkuser(char *dbstring, char **user)
{
   struct entry_rec entries[MAXENTRIES];
   int lastentry;

   if ((lastentry=parsetime(dbstring, entries))>=0) {
      format(lastentry, entries, user, USERFORMAT);
      *user = dbstring;
      return(0);
   }
   return(EINVAL);
}
