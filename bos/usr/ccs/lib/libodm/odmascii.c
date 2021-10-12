static char sccsid[] = "@(#)85	1.11  src/bos/usr/ccs/lib/libodm/odmascii.c, libodm, bos411, 9428A410j 7/19/94 16:36:32";
/*
 * COMPONENT_NAME: (LIBODM) Object Data Manager library
 *
 * FUNCTIONS: get_ascii_phrase , get_value_from_string, convert_to_binary,
 *            get_one_byte_from_ascii, convert_to_hex_ascii
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#define _ILS_MACROS
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <mbstr.h>
#include <sys/types.h>
#include <ctype.h>
#include "odmtrace.h"

#define STANDARD_BUFFERLEN 512


int stanza_line_number = 0;
char hex_values[] = {
    '0','1','2','3','4','5','6','7','8','9',
    'A','B','C','D','E','F'};

/*
 * NAME: get_ascii_phrase
 *
 * FUNCTION:
 *
 *       This function looks through an ascii file and passes back a
 *       stanza.
 *
 * NOTES:
 *
 *       This function calls malloc to allocate space for the stanza.
 *       Since this space is static in this function, the routine which
 *       calls get_ascii_phrase() must NOT try to call free() on the
 *       stanza returned.
 *
 * RETURNS:
 *
 *       Returns the length (in bytes) of the stanza retrieved if
 *       successful, -1 otherwise.
 */
int get_ascii_phrase(input_file,file_format,phrase_string)
FILE *input_file;        /* Valid file pointer containing the phrase */
int file_format;         /* Phrase type in the file (STANZA)          */
char **phrase_string;    /* Pointer to a (char *) for the phrase returned  */
{
    int quote_count = 0;        /* keep track of the quotes (") to       */

    /* determine the correct end of a record */
    char *character_pointer;


    int current_string_length; /* The number of bytes currently in the string */

    static char input_buffer[STANDARD_BUFFERLEN];

    int phrase_line_complete; /* Boolean value to indicate whether a complete */
                              /* line of the phrase was retrieved.            */

    int line_is_blank;        /* Boolean indicating if the line is made up */
    			      /* completely of whitespace characters       */

    int found_newline;        /* Boolean indicating that the newline       */
                              /* character was found.                      */

    int character_length;      /* The length in bytes of a character.       */

    int input_buffer_length;   /* The length in bytes of the input buffer   */

    static char *stanza_phrase = (char *) NULL;
    /* Static memory which will hold the stanzas */
    /* read in from the disk. It will grow to    */
    /* the size of the largest stanza.           */

    static int stanza_phrase_malloc_length = 0;
    /* The total number of bytes which have been */
    /* allocated for the stanza_phrase.          */

    static int file_record_number = 0; /* The current line in the file */

    int found_stanza_start;   /* Boolean indicating if the start of the  */
                              /* stanza has been found (not a comment line) */

    static int use_current_input_buffer = FALSE;
    /* Boolean indicating if the beginning of the */
    /* next stanza is already in the input_buffer */

    *phrase_string = (char *) NULL;
    TRC("get_ascii_phrase","looking for phrases","","","");
    TRC("get_ascii_phrase","   Geting STANZA","","","");
    /*-------------------------------------*/
    /* Make space for the phrase           */
    /*-------------------------------------*/
    if (stanza_phrase_malloc_length < STANDARD_BUFFERLEN)
      {
        stanza_phrase = (char *) malloc(STANDARD_BUFFERLEN);
        if (stanza_phrase == (char *) NULL)
          {
            stanza_phrase_malloc_length = 0;
            TRC("get_ascii_phrase","stanza_phrase malloc failed!!","","","");
            /*ODM----------------------malloc failed-------------*/
            /*------------------------*/
            /* odmerrno = ODM_MALLOC; */
            /*------------------------*/
            return(-1);
          } /* endif */
        stanza_phrase_malloc_length = STANDARD_BUFFERLEN;
      } /* endif */


    *stanza_phrase = '\0';


    current_string_length = 0;

    phrase_line_complete = TRUE;
    if (stanza_line_number == 0)
      {
        /*--------------------------------------------------------------*/
        /* Reset the file_record_number each time stanza_line_number is */
        /* zero.  This means that we are starting in a new file.        */
        /*--------------------------------------------------------------*/
        file_record_number = 0;
      } /* endif */

    stanza_line_number = file_record_number + 1;/* Store the line number so */
    						/* we know which stanza it is */

    found_stanza_start = FALSE;

    /*-----------------------------------------------------------------------*/
    /* The use_current_input_buffer variable is used to indicate that on     */
    /* the previous fgets call the beginning of a different stanza was found */
    /* and a call to fgets is not needed since the record is already in the  */
    /* input_buffer.                                                         */
    /*-----------------------------------------------------------------------*/

    while (use_current_input_buffer ||
        fgets(input_buffer,STANDARD_BUFFERLEN ,input_file)
        != (char *) NULL )

      {
        use_current_input_buffer = FALSE;

        file_record_number++;  /* Keep track of the line number in the file */

        TRC("get_ascii_phrase","Read line from file %s",input_buffer,"","");

        if ( quote_count % 2 == 0 && phrase_line_complete &&
            (input_buffer[0] == '#' || input_buffer[0] == '*') )
          {
            /*----------------------------*/
            /* This is a comment. Skip it. */
            /*-----------------------------*/
            while (strchr(input_buffer,'\n') == (char *) NULL)
              {
                /*--------------------------------------------*/
                /* If there is not a newline (\n) in the string */
                /* then we did not read a full line.  Continue */
                /* getting records until we find the return.   */
                /*---------------------------------------------*/

                if (fgets(input_buffer,STANDARD_BUFFERLEN ,input_file)
                    == (char *) NULL )
                  {
                    file_record_number++;
                    break;   /* This break will end the while( strchr() ) */
                  }

                file_record_number++;
              } /* endwhile */
            continue;
          } /* endif */


        if ( phrase_line_complete &&
            quote_count % 2 == 0 &&
            !isspace(input_buffer[0]))
          {
            /*----------------------------------------------------------*/
            /* We may have found the beginning of another stanza.       */
            /* Look through the string to determine if it has the form  */
            /*    <classname>:                                          */
            /*        or                                                */
            /*    <descriptor> = <value>                                */
            /*----------------------------------------------------------*/

            if (found_stanza_start         &&
                strchr(input_buffer,':') &&
                !strchr(input_buffer,'=')    )
              {
                /*-------------------------------------------------------*/
                /* If we already found the start of a stanza and we find */
                /* another line which has a colon character (:) but not  */
                /* an equal character (=) in it, then we have found the  */
                /* start of next stanza.  The current stanza is complete.*/
                /*-------------------------------------------------------*/
                use_current_input_buffer = TRUE;  /* Since we already have */
                /* the first line of the  */
                /* next stanza.           */

                file_record_number--;

                break;
              }
            else
              {
                /*----------------------------------------------------------*/
                /* The first time we find a line which is not a comment and */
                /* has the first character as a non-blank character set     */
                /* stanza_line_number to the current record number so we    */
                /* know where the stanza starts.                            */
                /*----------------------------------------------------------*/
                stanza_line_number = file_record_number;
                found_stanza_start = TRUE;
              } /* endif */

          } /* endif */


        /*------------------------------------------*/
        /* Count the number of quotes in the buffer */
        /* and determine if the line is made up of  */
        /* only white space.                        */
        /*------------------------------------------*/
        character_pointer = input_buffer;
        line_is_blank = TRUE;
        found_newline = FALSE;

        while ( *character_pointer != '\0')
          {
            /*--------------------------------------------------------*/
            /* Look for backslashes (\), quotes ("), and comments (*) */
            /*--------------------------------------------------------*/
            switch (*character_pointer)
              {

            case '\\':
                /*------------------------------------------*/
                /* If the character is a backslash (\) then */
                /* skip the next character.                 */
                /*------------------------------------------*/
                character_pointer++;

                /* Defect 119760 : should check for return code from mblen, */
                /*                 not the result of adding mblen to the    */
                /*                 character_pointer.                       */
 
		if ((character_length = mblen(character_pointer,MB_CUR_MAX))<0)
                   return(-1);
                else 
                   character_pointer += character_length;

                /* 
                if((character_pointer += mblen(character_pointer,MB_CUR_MAX))<0)			return(-1);
                */

                line_is_blank = FALSE;
                break;
            case '\n':
                found_newline = TRUE;
                character_pointer++;
                break;
            case '"':
                /*-----------------------------------*/
                /* Count the number of quotes found. */
                /*-----------------------------------*/
                quote_count++;
                character_pointer++;
                line_is_blank = FALSE;
                break;
            case '*':
            case '#':
                /*------------------------------*/
                /* End the string at a comment  */
                /* if the quotes are matched up */
                /*------------------------------*/
                if (quote_count % 2 == 0)
                  {
                    *character_pointer = '\n';
                    character_pointer++;
                    *character_pointer = '\0';

                  }
                else
                  {
                    character_pointer++;
                  } /* endif */

                line_is_blank = FALSE;
                break;
            default :
                /*-------------------------------------------------------*/
                /* This is an ordinary character.  Increment the pointer */
                /*-------------------------------------------------------*/

		/*-----------------------------------------------------------*/
		/* If it isn't a legitimate multi byte character then return */
		/*-----------------------------------------------------------*/
		if ((character_length = mblen(character_pointer,MB_CUR_MAX))<0)
                  return(-1);

                if ((character_length = mblen(character_pointer,MB_CUR_MAX))>1)
                  {
                    /*-------------------------*/
                    /* This is a NLS character */
                    /*-------------------------*/
                    line_is_blank = FALSE;
                  }
                else if (!isspace(*character_pointer))
                  {
                    line_is_blank = FALSE;
                  } /* endif */

                character_pointer += character_length;
              } /* endswitch */

          } /* endwhile */

        /*-----------------------------------------------*/
        /* The end of a STANZA is marked by a blank line */
        /*-----------------------------------------------*/
        if ( quote_count % 2 == 0 &&
            phrase_line_complete  &&
            line_is_blank  && found_newline)

          {
            /*------------------------------------------------------------*/
            /* If the quote_count is even then all the quotes (") have    */
            /* been matched up.                                           */
            /*------------------------------------------------------------*/
            if (current_string_length != 0)
              {
                /*-------------------------------*/
                /* Found the end of the phrase.  */
                /*-------------------------------*/
                TRC("get_ascii_phrase","Found phrase end!","","","");
                break;
              } /* endif */

            /*-------------------------------------------------*/
            /* If there are no characters in the phrase, then  */
            /* we found a blank line between phrases.  Continue */
            /* looking.                                         */
            /*--------------------------------------------------*/

          }
        else
          {
            if (phrase_line_complete)
              {
                /*---------------------------------------------*/
                /* If phrase_line_complete is TRUE then change */
                /* it to FALSE since we are now getting a      */
                /* different line in the phrase.               */
                /*---------------------------------------------*/
                phrase_line_complete = FALSE;
              } /* endif */

            /*------------------*/
            /* Save the buffer. */
            /*------------------*/

            /*------------------------------------------*/
            /* Check to see if there is enough space    */
            /*------------------------------------------*/
            input_buffer_length = strlen(input_buffer);
            if (input_buffer_length + current_string_length >=
                stanza_phrase_malloc_length)
              {
                /*----------------------------------------*/
                /* There is not enough space. Realloc.    */
                /*----------------------------------------*/
                stanza_phrase_malloc_length += STANDARD_BUFFERLEN;

                stanza_phrase = (char *) realloc(stanza_phrase,
                    stanza_phrase_malloc_length);
                if (stanza_phrase == (char *) NULL)
                  {
                    stanza_phrase_malloc_length = 0;
                    TRC("get_ascii_phrase","stanza_phrase realloc failed!!",
                        "","","");
                    /*ODM----------------------malloc failed-------------*/
                    /*------------------------*/
                    /* odmerrno = ODM_MALLOC; */
                    /*------------------------*/
                    return(-1);
                  } /* endif */

              } /* endif */

            strcpy(stanza_phrase + current_string_length,input_buffer);

            current_string_length += input_buffer_length;

            if (found_newline != NULL &&
                quote_count % 2 == 0)
              {
                /*------------------------------------------------------------*/
                /* If the buffer has a newline in it and the number of quotes */
                /* found is even, then we have found the end of a line in the */
                /* phrase.  We need to keep track of this in case the entire  */
                /* line cannot be read with a single fgets().                 */
                /*------------------------------------------------------------*/
                phrase_line_complete = TRUE;

              } /* endif */


          } /* endif input_buffer[0] == '\n'*/

      } /* endwhile fgets != NULL */

    TRC("get_ascii_phrase","final phrase is %s",stanza_phrase,
        "length %d",current_string_length);

    if (quote_count % 2 != 0)
      {
        /*----------------------------------------------*/
        /* If there is an odd number of quotes (") then */
        /* this is a bad phrase.                        */
        /*----------------------------------------------*/
        TRC("get_ascii_phrase","quote_count is odd!! %d",quote_count,"","");
        *phrase_string = (char *) NULL;
        /*ODM--------------------------bad phrase------------------*/
        /*------------------------*/
        /* odmerrno = ODM_PHRASE; */
        /*------------------------*/
        return(-1);
      } /* endif */


    *phrase_string = stanza_phrase;
    return(current_string_length);

} /* end of get_ascii_phrase */


/*
 * NAME: get_value_from_string
 *
 * FUNCTION:
 *
 *   This routine will go through an ascii string and find a string
 *   which ends with a character in 'stop_chars'.  It will parse out
 *   quote characters (") as necessary.
 *   Subsequent calls to this routine with 'string_with_value == NULL'
 *   will find values starting from the last value found in the string
 *
 * NOTES:
 *
 *    Since the space for the value returned is allocated in this routine
 *    and is static to this routine, the routine which calls this
 *    routine must not try to call free() on the value returned.
 *
 * RETURNS:
 *
 *    This routine will return a pointer to the first string which ended
 *    with one of the characters in the 'stop_chars' string. It also passes
 *    back the single character which ended the string
 */
char *get_value_from_string(string_with_values,stop_chars,
skip_spaces, terminating_char )
char *string_with_values;   /* The string to look through.  May be NULL */
char *stop_chars;          /* A list of characters will terminate the value */
int skip_spaces;           /* TRUE or FALSE to skip spaces                  */
char *terminating_char;    /* the character which terminated the value */
{
    static char *next_value_ptr = (char *) NULL;
    /* This is used to keep track */
    /* of the last value position */

    static char *return_value = (char *) NULL;
    /* This will hold the formated value */
    /* It is static storage which will    */
    /* become as large as the largest value */

    static char *return_value_ptr = (char *) NULL;

    char *start_of_trailing_spaces = (char *) NULL;
    /* Pointer to thvaluee beginning of the    */
    /* trailing spaces (if any)           */

    int continue_looking;              /* Boolean indicating to keep searching */
    int inside_quote;                  /* Boolean indicating inside quotes (') */


    int character_length;              /* The length in bytes of a character    */

    static int return_value_malloc_length = 0;
    /* The number of bytes which have been*/
    /* allocated to return_value_ptr      */

    /*-----------------------------------------*/
    /* Set odmerrno = 0 so that the calling    */
    /* routine can check odmerrno to determine */
    /* if there was an error if this routine   */
    /* returns NULL.                           */
    /*-----------------------------------------*/
    /*---------------*/
    /* odmerrno = 0; */
    /*---------------*/

    if (string_with_values == (char *) NULL )
      {
        if (next_value_ptr == (char *) NULL)
          {
            /*--------------------------------------------*/
            /* Fatal error!!.  Since this routine is only */
            /* called from other ODM routines, the case   */
            /* where both these pointers are NULL should  */
            /* never happen.                              */
            /*--------------------------------------------*/
            odmtrace = 1;
            /*------------------*/
            /* odmerrno = -999; */
            /*------------------*/
            TRC("get_value_from_string","both pointers are NULL!!","","","");
            return( (char *) NULL);
          } /* endif */
      }
    else
        next_value_ptr = string_with_values;
        /* endif */


    TRC("get_value_from_string","getting value at %s",next_value_ptr,
        "","");

    if (*next_value_ptr == '\0')
      {

        TRC("get_value_from_string","No more values in string","","","");
        return( (char *) NULL);
      } /* endif */

    if (skip_spaces == TRUE)
      {
        /*---------------------------------------*/
        /* If the skip_spaces is TRUE    then    */
        /* skip to the first non-space character */
        /*---------------------------------------*/
        while (isspace(*next_value_ptr))
          {
            next_value_ptr++;
          } /* endwhile */
      } /* endif */

    /*---------------------------------*/
    /* Approximate the length of this  */
    /* value.                          */
    /* If there is already enough space */
    /* then do not malloc.              */
    /*---------------------------------*/

    if (return_value_malloc_length < strlen(next_value_ptr) + 1)
      {
        return_value_malloc_length = strlen(next_value_ptr) + 1;
        if (return_value == (char *) NULL)
          {
            return_value = (char *) malloc (return_value_malloc_length);
          }
        else
          {
            return_value = (char *) realloc (return_value,
                return_value_malloc_length);
          } /* endif */

        if (return_value == (char *) NULL)
          {
            return_value_malloc_length = 0;
            return_value = (char *) NULL;

            TRC("get_value_from_string","return_value malloc failed!!",
                "","","");
            /*ODM----------------------------malloc failed----------------*/
            /*------------------------*/
            /* odmerrno = ODM_MALLOC; */
            /*------------------------*/
            return( (char *) NULL);
          } /* endif */
      } /* endif */

    continue_looking = TRUE;
    inside_quote = FALSE;
    return_value_ptr = return_value;
    start_of_trailing_spaces = return_value;


    while (continue_looking)
      {

	if ((character_length = mblen(next_value_ptr, MB_CUR_MAX))<0)
		return(-1);		

       /* character_length = NLchrlen(next_value_ptr); */

        if ( ( !inside_quote &&
            strchr(stop_chars,*next_value_ptr) != (char *) NULL )
            ||
            *next_value_ptr == '\0'  )
          {
            *return_value_ptr = '\0';

            if ( *next_value_ptr == '\0' && inside_quote )
              {
                /*-------------------------------------*/
                /* Odd number of quotes in the value!  */
                /* This is an error!!                  */
                /* Found the NULL before completing the */
                /* quoted part.                         */
                /*--------------------------------------*/
                TRC("get_value_from_string","odd number of quotes!!",
                    "","","");
                /*---------------------*/
                /* free(return_value); */
                /*---------------------*/
                /*ODM--------------bad import record---------*/
                /*------------------------*/
                /* odmerrno = ODM_PHRASE; */
                /*------------------------*/
                return( (char *) NULL);
              } /* endif */


            TRC("get_value_from_string","found  a stop character %s",
                next_value_ptr,"","");

            /*-----------------------------------------------*/
            /* Found an end of a value.                      */
            /*-----------------------------------------------*/

            /*-------------------------------------------------------*/
            /* Since this is the last value we need to stop looking. */
            /*-------------------------------------------------------*/
            continue_looking = FALSE;
            *terminating_char = *next_value_ptr;

          }
        else
          {
            switch (*next_value_ptr)
              {
            case '\"':
                /*-----------------------------------------*/
                /* Switch the quote indicator              */
                /*-----------------------------------------*/
                inside_quote = !inside_quote;

                break;
            case '\\':
                /*----------------------------------------------*/
                /* Found a backslash.  Skip this  character.    */
                /* and save the next character.                 */
                /*----------------------------------------------*/

                start_of_trailing_spaces = return_value_ptr + 1;

                next_value_ptr++;
                if (*next_value_ptr == 'n')
                  {
                    /*--------------------------------*/
                    /* Found a newline character (\n) */
                    /*--------------------------------*/
                    *return_value_ptr = '\n';
                    return_value_ptr++;
                    break;
                  }
                else if (*next_value_ptr == 't')
                  {
                    /*----------------------------*/
                    /* Found a tab character (\t) */
                    /*----------------------------*/
                    *return_value_ptr = '\t';
                    return_value_ptr++;
                    break;

                  }
                else if (*next_value_ptr == 'b')
                  {
                    /*---------------------------*/
                    /* Found backspace character */
                    /*---------------------------*/

                    *return_value_ptr = '\b';
                    return_value_ptr++;
                    break;
                  }
                else if (*next_value_ptr == 'r')
                  {
                    /*----------------------------*/
                    /* Found carriage return (\r) */
                    /*----------------------------*/

                    *return_value_ptr = '\r';
                    return_value_ptr++;
                    break;
                  }
                else if (*next_value_ptr == 'f')
                  {
                    /*----------------------*/
                    /* Found form feed (\f) */
                    /*----------------------*/

                    *return_value_ptr = '\f';
                    return_value_ptr++;
                    break;
                  }
                else if (*next_value_ptr == '\n')
                  {
                    /*-------------------------------------------------------*/
                    /* Found a backslash followed by a newline character.    */
                    /* This means this line is continued on the next line.   */
                    /* Don't save the newline character.                     */
                    /*-------------------------------------------------------*/
                    break;
                  }
                else
                  {
                    /*-----------------------------------------*/
                    /* This is some other character.  Save it. */
                    /*-----------------------------------------*/
                  } /* endif */


                if((character_length = mblen(next_value_ptr,MB_CUR_MAX))<0)
			return(-1);
                /*-----------------------------------------------------*/
                /* Fall through to the 'default' to save the character */
                /*-----------------------------------------------------*/
            default :
                /*--------------------------------------------------*/
                /* This character is not a quote. Save it.          */
                /*--------------------------------------------------*/
                if (character_length == 1)
                  {

                    *return_value_ptr = *next_value_ptr;
                    return_value_ptr++;

                    if (inside_quote || !isspace(*next_value_ptr))
                      {
                        start_of_trailing_spaces = return_value_ptr;
                      } /* endif */
                  }
                else
                  {
                    memcpy((void *) return_value_ptr,
                        (void *) next_value_ptr,(size_t) character_length );

                    return_value_ptr += character_length;
                    start_of_trailing_spaces = return_value_ptr;
                  } /* endif */

              } /* endswitch */



          } /* endif == ',' or ' ' or NULL */

        *return_value_ptr = '\0';

        if (*next_value_ptr != '\0')
          
            next_value_ptr += character_length;
         

      } /* endwhile continue_looking */



    TRC("get_value_from_string","returning %s",return_value,"","");
    TRC("get_value_from_string","start_of_trailing - return_value %d",
        start_of_trailing_spaces - return_value,"","");

    if (skip_spaces &&
        start_of_trailing_spaces != (char *) NULL)
        *start_of_trailing_spaces = '\0';

    return(return_value);



} /* end of get_value_from_string */


/*
 * NAME: convert_to_binary
 *
 * FUNCTION:
 *
 *    This routine will convert an arbitrary length ascii string which
 *    represents a binary value into an actual binary value.
 *
 *    Example:
 *        hex_string = "0xABCDEF1234567890"  (string of characters)
 *           is converted to
 *        binary_value = ABCDEF1234567890  (array of binary values)
 *
 *
 * RETURNS:
 *
 *    Returns 0 if successful, -1 otherwise.
 */
int convert_to_binary(hex_string,binary_value,max_length)
char *hex_string;        /* String of characters       */
char *binary_value;      /* Place to put binary values */
int max_length;          /* maximum length of binary_value */
{
    int index;
    int hex_length;               /* Number of bytes in the hex string   */
    int returnstatus;

    char *binary_ptr;                   /* Pointer into binary value */
    char *hex_ptr;                      /* Pointer into ASCII value  */


    if (hex_string == (char *) NULL ||
        hex_string[0] != '0'        ||
        (hex_string[1] != 'x'  && hex_string[1] != 'X')          )
      {
        TRC("convert_to_binary","Invalid hex value ","","","");
        return(-1);
      } /* endif */


    hex_length = strlen(hex_string) - 2 ;
    if (hex_length & 00000001)
      {
        /*------------------------------------------------------------------*/
        /* If the hex_length is odd (the first bit is set) then this is     */
        /* an invalid hex string since there MUST be two hex characters for */
        /* each byte.                                                       */
        /*------------------------------------------------------------------*/
        TRC("convert_to_binary","Odd number of hex characters","","","");
        return(-1);
      } /* endif */

    if (binary_value == (char *) NULL ||
        max_length < hex_length / 2)
      {
        /*---------------------------------------------------------------*/
        /* Since we use the space allocated by the calling routine, this */
        /* value MUST be non null.  It is up to the calling routine to   */
        /* determine if there is enough space for the value.             */
        /*---------------------------------------------------------------*/
        return(-1);
      } /* endif */

    hex_ptr = hex_string + 2;   /* The '+ 2' is for the "0x" in front */

    binary_ptr = binary_value;


    for (index = 0 ; index < hex_length / 2 ; index++, hex_ptr += 2)
      {

        returnstatus = get_one_byte_from_ascii(hex_ptr);
        if (returnstatus == -1)
          {
            return(-1);

          } /* endif */

        *binary_ptr++ =  (char) returnstatus;

      } /* endfor */


    return(0);

} /* end of convert_to_binary */


/*
 * NAME: get_one_byte_from_ascii
 *
 * FUNCTION:
 *
 *      Looks through a string and creates a single byte for the two
 *      ascii hex characters in the string 'hex_ptr'
 *
 *      Example:
 *          hex_ptr = "A1CD03" will return the single byte containing 0xA1
 *
 *
 * RETURNS:
 *
 *      Returns a single byte if successful, -1 otherwise.
 */
int get_one_byte_from_ascii(hex_ptr)
char *hex_ptr; /* Pointer to an array of ascii hex characters */
{
    int index;
    int return_value = 0;
    char hex_digit;

    for (index = 0; index < 2; index++, hex_ptr++)
      {
        switch (*hex_ptr)
          {
        case '0':
            hex_digit = 0;
            break;
        case '1':
            hex_digit = 1;
            break;
        case '2':
            hex_digit = 2;
            break;
        case '3':
            hex_digit = 3;
            break;
        case '4':
            hex_digit = 4;
            break;
        case '5':
            hex_digit = 5;
            break;
        case '6':
            hex_digit = 6;
            break;
        case '7':
            hex_digit = 7;
            break;
        case '8':
            hex_digit = 8;
            break;
        case '9':
            hex_digit = 9;
            break;
        case 'A':
        case 'a':
            hex_digit = 0x0a;
            break;
        case 'B':
        case 'b':
            hex_digit = 0x0b;
            break;
        case 'C':
        case 'c':
            hex_digit = 0x0c;
            break;
        case 'D':
        case 'd':
            hex_digit = 0x0d;
            break;
        case 'E':
        case 'e':
            hex_digit = 0x0e;
            break;
        case 'F':
        case 'f':
            hex_digit = 0x0f;
            break;
        default :
            TRC("get_one_byte_from_ascii","Invalid character in string! %c",
                *hex_ptr,"","");
            return(-1);
          } /* endswitch */

        if (index == 0)
          {
            return_value = hex_digit;
          }
        else
          {
            return_value = (return_value << 4) + hex_digit;
          } /* endif */

      } /* endfor */

    return(return_value);

} /* end of get_one_byte */

/*
 * NAME:  convert_to_hex_ascii
 *
 * FUNCTION:
 *
 *       Converts an arbitrary length array of binary values into
 *       an array of ascii hex characters which represent the binary value.
 *
 * NOTES:
 *
 *       This routine calls malloc() to allocate storage for the ascii
 *       representation of the binary value.  The calling routine must not
 *       try to free the string returned since the space is static to
 *       this routine.
 *
 * RETURNS:
 *
 *       An ascii string of hex characters which represent the binary value.
 */

char *convert_to_hex_ascii(binary_value,length)
char *binary_value;     /* Pointer to an array of binary values */
int length;             /* The length of the binary value       */
{
    int index;
    int malloc_length = 0;              /* Number of bytes malloc'd for hex */

    char *hex_string = (char *) NULL;   /* Hex value in ASCII        */
    char *binary_ptr;                   /* Pointer into binary value */
    char *hex_ptr;                      /* Pointer into ASCII value  */

    if (binary_value == (char *) NULL )
      {
        TRC("convert_to_hex_ascii","Null binary value ","","","");
        return((char *) NULL);
      } /* endif */

    /*-------------------------------------------------------------------*/
    /* See if there is enough space already malloc'd.  We need two times */
    /* the length of the binary value plus 2 for the "0x" and 1 for the  */
    /* final NULL.                                                       */
    /*-------------------------------------------------------------------*/

    if (malloc_length < 2*length + 3)
      {
        /*-----------------------------*/
        /* Need to allocate more space */
        /*-----------------------------*/
        malloc_length += 2*length + 3;

        if (hex_string == (char *) NULL)
          {
            hex_string = (char *) malloc(malloc_length);
          }
        else
          {
            hex_string = (char *) realloc(hex_string,malloc_length);
          } /* endif */

        if (hex_string == (char *) NULL)
          {
            TRC("convert_to_hex_ascii"," hex string malloc failed! %d",
                malloc_length, "","");
            malloc_length = 0;
            hex_string = (char *) NULL;
            return((char *) NULL);
          } /* endif */

      } /* endif */


    hex_ptr    = hex_string;
    binary_ptr = binary_value;

    *hex_ptr++ = '0';
    *hex_ptr++ = 'x';

    for (index = 0 ; index < length; index++)
      {
        *hex_ptr++ = hex_values[(*binary_ptr >> 4) & 0x0f ];
        *hex_ptr++ = hex_values[(*binary_ptr++ & 0x0f) ];

      } /* endfor */

    *hex_ptr = '\0';

    return(hex_string);

} /* end of convert_to_hex_ascii */
