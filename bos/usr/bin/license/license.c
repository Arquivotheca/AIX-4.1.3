static char sccsid[] = "@(#)66  1.4.1.2  src/bos/usr/bin/license/license.c, cmdlicense, bos411, 9428A410j 6/6/94 18:19:07";
/*
 * COMPONENT_NAME: CMDLICENSE 
 *
 * FUNCTIONS:
 *   change_max_users, list_max_users, open_login_file, find_users_stanza,
 *   get_stanza_line, check_for_header, change_users_stanza, write_login_file,
 *   get_maxlogins_value,  add_netls_daemon, create_netls_file, 
 *   delete_netls_file, remove_netls_daemon
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp.  1990, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <NLchar.h>
#include <ctype.h>
#include <sys/license.h>
#include "license_msg.h"

#define DEF_INITTAB     "%s: Could not update /etc/inittab.\n"
#define DEF_INVAL       "%s: Could not retrieve the licensing information.\n"

nl_catd  scmc_catd;              /* Cat descriptor for scmc conversion */
char *command_name;              /* Set in main for the messages */

/*
 *   NAME:     add_netls_daemon
 *
 *   FUNCTION: Adds the stanza for the bosnetls monitord
 *             daemon to the /etc/inittab file.
 *
 *   RETURNS:  0 if successful, -1 otherwise.
 */
int
add_netls_daemon(void)
{
        char    s[200];
        int     result;

        /*
         * See if a entry for netls exists; if it does remove it and
         * add the netls entry.
         */
        result = system("lsitab \"monitord\" >/dev/console 2>&1");
        if (result == 0)
        {
                result = system ("rmitab \"monitord\" >/dev/console 2>&1");
                if (result != 0)
                {
                        fprintf(stdout, catgets(scmc_catd, LICENSE_SET,
                                LICENSE_INITTAB, DEF_INITTAB), command_name);
                        return(-1);
                }
        }

        sprintf(s, "mkitab \"monitord:2:once:%s >/dev/console 2>&1\"",
                                                        MONITORD_PROGRAM);
        result = system(s);
        if (result != 0)
        {
                fprintf(stdout, catgets(scmc_catd, LICENSE_SET, LICENSE_INITTAB,
                                                DEF_INITTAB), command_name);
                return(-1);
        }

        return(0);
}


/*
 *   NAME:     remove_netls_daemon
 *
 *   FUNCTION: Removes the stanza for the bosnetls monitord
 *             daemon in the /etc/inittab file.
 *
 *   RETURNS:  0 if successful, -1 otherwise.
 */
int
remove_netls_daemon(void)
{
        int     result;

        /*
         * See if the daemon exists currently in the /etc/inittab file.
         * If it does, then remove the line from the file.
         */
        result = system("lsitab \"monitord\" >/dev/console 2>&1");
        if (result == 0)                /* exists so remove line */
        {
                result = system ("rmitab \"monitord\" >/dev/console 2>&1");
                if (result != 0)
                {
                        fprintf(stdout,
                                catgets(scmc_catd, LICENSE_SET, LICENSE_INITTAB,
                                                DEF_INITTAB), command_name);
                        return(-1);
                }
        }

        return(0);              /* daemon does not exist */
}



/*
 *   NAME:     create_netls_file
 *
 *   FUNCTION: Creates a file /etc/security/.netls.  The creation
 *             of this file signifies that the bosnetls is active.
 *
 *   RETURNS:  0 if successful, -1 otherwise.
 */
int
create_netls_file(void)
{
        struct  stat    buf;
        int             fd;             /*file descriptor */


        /* If /etc/security/.netls exists; there is no need to create it */

        if (stat(FLOATING_ON, &buf) != 0)
        {
                if ((fd = creat(FLOATING_ON, 0400)) < 0)
                {
                        fprintf(stdout,
                                catgets(scmc_catd, LICENSE_SET, LICENSE_ENABLE,
                                "%s: Could not enable floating licensing.\n"),
                                                                command_name);
                        return(-1);
                }
                close(fd);
        }
        return(0);
}


/*
 *   NAME:     delete_netls_file
 *
 *   FUNCTION: Deletes the file /etc/security/.netls. The deletion
 *             of this file signifies that bosnetls is non_active.
 *
 *   RETURNS:  0 if successful, -1 otherwise.
 */
int
delete_netls_file(void)
{
        struct  stat    buf;
        int             tmp;

        /* If /etc/security/.netls does not exist, no need to remove it */

        if (stat(FLOATING_ON, &buf) == 0 && unlink(FLOATING_ON) < 0)
        {
                fprintf(stdout, catgets(scmc_catd, LICENSE_SET, LICENSE_DISABLE,
                "%s: Could not disable floating licensing.\n"), command_name);
                return(-1);
        }
        return(0);
}



/*
 *   NAME:     change_max_users
 *   FUNCTION: Sets the 'maxlogins' entry in the 'usw:' stanza in the
 *             login.cfg file to the value 'new_max_users'.
 *   RETURNS:  A zero if successful, -1 otherwise.
 *
 */
int change_max_users(new_max_users)
int new_max_users;                /* The new maximum number of users */
{
   struct login_file_type *login_file_info;   /* Info about the login file */
   int return_code;

   login_file_info  = open_login_file(O_RDWR);
   if (login_file_info == NULL)
     {
       /*---------------------------------------------*/
       /* Could not open and shmat the login file.    */
       /*The open_login_file has                      */
       /* already printed the error message.          */
       /*---------------------------------------------*/
       return(-1);
     } /* endif */

   /*------------------------------------------------------*/
   /* Look through the file and find the 'usw:' stanza and */
   /* the 'maxlogins' value                                */
   /*------------------------------------------------------*/
   find_users_stanza(login_file_info);

    /*----------------------------------------*/
    /* Change the new maximum number of users */
    /*----------------------------------------*/
    return_code = change_users_stanza(login_file_info,new_max_users);
    if (return_code < 0)
      {
        /*-----------------------------------------------*/
        /* Could not successfully change the login value */
        /*-----------------------------------------------*/
        return(-1);
      } /* endif */

    /*----------------------------------------*/
    /* Write out the change to the login file */
    /*----------------------------------------*/
    return_code = write_login_file(login_file_info);
    close(login_file_info->file_id);
    if (return_code < 0)
      {
        /*---------------------------------------------*/
        /* Could not successfully write the login file */
        /*---------------------------------------------*/
        return(-1);
      } /* endif */


    /*----------------------*/
    /* Return successfully. */
    /*----------------------*/
    return(0);

} /* end of change_max_users */

/*
 *   NAME:      list_max_users
 *   FUNCTION:  Display the maximum number of users which can be
 *              concurrently logged in.
 *   RETURNS:   A zero if successful, -1 otherwise.
 *
 */
int list_max_users(int do_colon_output)
{
   struct login_file_type *login_file_info;
   struct  stat    buf;
   int fixed_licenses;
   int floating;

/* Get number of users 	*/
   login_file_info  = open_login_file(O_RDONLY);
   if (login_file_info == NULL)
     {
       /*---------------------------------------------*/
       /* Could not open and shmat the login file.    */
       /*The open_login_file has                      */
       /* already printed the error message.          */
       /*---------------------------------------------*/
       return(-1);
     } /* endif */

   /*------------------------------------------------------*/
   /* Look through the file and find the 'usw:' stanza and */
   /* the 'maxlogins' value                                */
   /*------------------------------------------------------*/
   find_users_stanza(login_file_info);

   /*----------------------------------------------*/
   /* Get the maximum number of users */
   /*----------------------------------------------*/
   fixed_licenses = get_maxlogins_value(login_file_info);
   close(login_file_info->file_id);

   if (fixed_licenses < 0)
       return(-1);

        /*
         * Check if /etc/security/.netls exists.
         * Users that don't have access to /etc/security will fail.
         */
        if ((!(floating = !stat(FLOATING_ON, &buf))) && errno == EACCES)
        {
                fprintf(stdout, catgets(scmc_catd, LICENSE_SET, LICENSE_INVAL,
                                                DEF_INVAL), command_name);
                return(-1);
        }

        /*
         * Print out the max users.
        * Colon form used for smit panels - i.e., message not translatable.
         */
        if (do_colon_output == TRUE)
        {
                /*
                 * Print the value like:
                 *
                 *   #fixed:floating
                 *       24:off
                 */
                fprintf(stdout, "#fixed:floating\n");
                fprintf(stdout, "%d:%s\n", fixed_licenses,
                                                (floating) ? "on" : "off");
        }
        else
        {
                fprintf(stdout, catgets(scmc_catd, LICENSE_SET, LICENSE_TITLE,
                "Maximum number of fixed licenses is %d.\n"), fixed_licenses);

                fprintf(stdout, (floating)
                        ? catgets(scmc_catd, LICENSE_SET, LICENSE_MODE_ON,
                                        "Floating licensing is enabled.\n")
                        : catgets(scmc_catd, LICENSE_SET, LICENSE_MODE_OFF,
                                        "Floating licensing is disabled.\n"));
        }

        return(0);

} /* end of list_max_users */

/*
 *   NAME:      open_login_file
 *   FUNCTION:  Opens the file /etc/security/login.cfg.  Attempts to
 *              use the 'shmat()' function to memory-map the file. If
 *              that does not work, a block of memory is malloc'd and
 *              the entire file is read in.
 *   RETURNS:   A login_file_type structure which contains a pointer to
 *              the memory-mapped file if successful, NULL otherwise.
 *
 */
struct login_file_type *open_login_file(file_mode)
int file_mode;   /* Open for read (O_RDONLY) or read/write (O_RDWR) */
{
  static struct login_file_type login_file_info;
  struct stat system_file_info;                 /* Info about the file */

  /*--------------------------*/
  /* Initialize the structure */
  /*--------------------------*/
  login_file_info.file_id        = -1;
  login_file_info.file_size      = -1;
  login_file_info.stanza_start   = NULL;
  login_file_info.maxlogins_start = NULL;
  login_file_info.had_to_malloc  = FALSE;
  login_file_info.mem_file       = NULL;

  /*---------------*/
  /* Open the file */
  /*---------------*/
  login_file_info.file_id = open(LOGIN_FILE_PATH,file_mode);
  if (login_file_info.file_id == -1)
    {
      if (file_mode == O_RDONLY)
        {
          fprintf(stderr,catgets(scmc_catd,LICENSE_SET,LICENSE_OPEN,
          "%s: Cannot open the file %s for read access.\n\
\tCheck the permissions on the file and try again.\n"),command_name,LOGIN_FILE_PATH);
          return(NULL);
        }

      /*----------------------------------------------------------------*/
      /* Check to see if the file could not be opened because it is not */
      /* created.  If that is true, then create it.                     */
      /*----------------------------------------------------------------*/
       if (errno  == ENOENT)
         {
           login_file_info.file_id = open(LOGIN_FILE_PATH,file_mode | O_CREAT,0644);
           if (login_file_info.file_id == -1)
             {
                  /*---------------------------*/
                  /* Could not create the file */
                  /*---------------------------*/
          fprintf(stderr,catgets(scmc_catd,LICENSE_SET,LICENSE_CREATE,
          "%s: Cannot create the file %s.\n\
\tCheck the permissions on the directory and try again.\n"),command_name,LOGIN_FILE_PATH);
              return(NULL);
             }
         }
       else
         {
          fprintf(stderr,catgets(scmc_catd,LICENSE_SET,LICENSE_RW,
          "%s: Cannot open the file %s for read/write access.\n\
\tCheck the permissions on the file and try again.\n"),command_name,LOGIN_FILE_PATH);
          return(NULL);
         } /* endif  errno == ENOENT */

    } /* endif  file_id == -1 */

  /*------------------------*/
  /* Find out the file size */
  /*------------------------*/
  if (stat(LOGIN_FILE_PATH,&system_file_info) == -1)
    {
      /*------------------------------------------*/
      /* Could not get information about the file */
      /*------------------------------------------*/
      fprintf(stderr,catgets(scmc_catd,LICENSE_SET,LICENSE_STAT,
      "%s: Could not retrieve information for the file %s.\n\
\tMake sure the file is accessable and try again.\n"),command_name,LOGIN_FILE_PATH);
      goto ERROR_CLEANUP_FOR_open;
    } /* endif */

   login_file_info.file_size = system_file_info.st_size;

   /*--------------------------------*/
   /* Attempt to memory map the file */
   /*--------------------------------*/
   login_file_info.mem_file = (char *) -1;
/*   login_file_info.mem_file = (char *) shmat(login_file_info.file_id,NULL,SHM_MAP);*/
   if (login_file_info.mem_file == (char *) -1)
     {
       /*----------------------------------------------------------------------*/
       /* Could not shmat the file.  Malloc a large block to put the file into */
       /* and read the whole file.                                             */
       /*----------------------------------------------------------------------*/

       login_file_info.mem_file = (char *) malloc(login_file_info.file_size +
                                                         MAX_STANZA_LENGTH);

       if (login_file_info.mem_file == NULL)
         {
           /*----------------------------------*/
           /* Could not allocate enough space. */
           /*----------------------------------*/
           fprintf(stderr,catgets(scmc_catd,LICENSE_SET,LICENSE_MALLOC,
           "%s: Could not allocate %d bytes of storage.\n\
\tTry again later or contact the systems administrator.\n"),command_name,
                            login_file_info.file_size + MAX_STANZA_LENGTH);
           goto ERROR_CLEANUP_FOR_open;
         } /* endif */

         login_file_info.had_to_malloc = TRUE;   /* So we know to free() */

        /*-------------------------------------------*/
        /* We have the storage. Now read in the file */
        /*-------------------------------------------*/
        if (read(login_file_info.file_id,login_file_info.mem_file,
                 (unsigned) login_file_info.file_size)  == -1)
          {
            /*-------------------------------*/
            /* Could not read the whole file */
            /*-------------------------------*/
           fprintf(stderr,catgets(scmc_catd,LICENSE_SET,LICENSE_READ,
           "%s: Could not read all of the file %s.\n\
\tMake sure there are no locks on the file and try again.\n"),command_name,
                            login_file_info.file_size + MAX_STANZA_LENGTH);
           goto ERROR_CLEANUP_FOR_open;
          } /* endif */

     } /* endif mem_file == -1 */

  /*-------------------------------------------------------------*/
  /* The file has been successfully opened and read into memory. */
  /*-------------------------------------------------------------*/
  return(&login_file_info);

ERROR_CLEANUP_FOR_open:

  /*------------------------------------------------*/
  /* Close the file and free() memory if necessary. */
  /*------------------------------------------------*/
  if (login_file_info.file_id > 0)
    {
      close(login_file_info.file_id);
    } /* endif */

  if (login_file_info.mem_file != NULL &&
                       login_file_info.had_to_malloc == TRUE)
    {
      free((void *) login_file_info.mem_file);
    } /* endif */

  return(NULL);

} /* end of open_login_file */

/*
 *   NAME:       find_users_stanza
 *   FUNCTION:   Looks through the file trying to find the 'usw:' stanza
 *               and the 'maxlogins' entry in the stanza.  If they are
 *               found, their corresponding pointers in the 'login_file_info'
 *               structure are set to their location in the file.  They are
 *               set to NULL otherwise.
 *   RETURNS:    Nothing.
 *
 */
void find_users_stanza(login_file_info)
struct login_file_type *login_file_info;   /* File information */
{
  register char *search_index;
  register char *end_location;        /* Last place to look in the file */

   /*------------------------------------------------------------*/
   /* Go through the login file looking for the beginning of the */
   /* stanza which begins with "usw:".  If that exists, find the */
   /* "maxlogins" part.                                          */
   /*------------------------------------------------------------*/

  /*---------------------------------------------------------------------*/
  /* First of all, find out if the file is big enough to hold the "usw:" */
  /* phrase.                                                             */
  /*---------------------------------------------------------------------*/
  if (login_file_info->file_size < LOGIN_HEADER_LENGTH)
    {
      /*--------------------------------------------------------------*/
      /* Set the stanza pointers to NULL and return since the file is */
      /* too small to contain the stanza we are looking for.          */
      /*--------------------------------------------------------------*/
      login_file_info->stanza_start = NULL;
      login_file_info->maxlogins_start = NULL;
      return;
    } /* endif */


  search_index = login_file_info->mem_file;


  /*------------------------------------------------------------------*/
  /* Find the "usw:" part.  The 'end_location' variable indicates the */
  /* last place to look for the header.                               */
  /* First look to see if the first thing in the file is "usw:"       */
  /*------------------------------------------------------------------*/

  if (NLstrncmp(search_index,LOGIN_HEADER,LOGIN_HEADER_LENGTH) == 0)
    {
      /* Found the LOGIN_HEADER ("usw:") */
      login_file_info->stanza_start = search_index;
    }
  else
    {
      end_location =
           search_index + login_file_info->file_size - 1;

     /*-------------------------------------------------------------------*/
     /* Get lines from the stanza file as long as we have not reached the */
     /* end of the file and we have not found the LOGIN_HEADER            */
     /* The 'search_index' variable will point at the beginning of a line */
     /* in the stanza file.                                               */
     /*-------------------------------------------------------------------*/
      while ((search_index =
               get_stanza_line(search_index,end_location,GET_HEADER)) != NULL
            && NLstrncmp(search_index,LOGIN_HEADER,LOGIN_HEADER_LENGTH) != 0);

     login_file_info->stanza_start = search_index;
    } /* endif */


   /*-----------------------------------------------------------------------*/
   /* Now we have looked through the file for the LOGIN_HEADER (usw:).  If  */
   /* that was found, (login_file_info->stanza_start != NULL) then look for */
   /* the LOGIN_ENTRY (maxlogins).                                          */
   /*-----------------------------------------------------------------------*/
   if (login_file_info->stanza_start == NULL)
     {
       /*------------------------------------------------------------------------*/
       /* Could not find the beginning of the stanza.  Set the 'maxlogins_start' */
       /* value to NULL since it could not be found either.                      */
       /*------------------------------------------------------------------------*/
       login_file_info->maxlogins_start = NULL;
       return;
     } /* endif */

   /*----------------------------------------------------------------------*/
   /* Start looking for the LOGIN_ENTRY (maxlogins) after the LOGIN_HEADER */
   /*----------------------------------------------------------------------*/
   search_index += LOGIN_HEADER_LENGTH ;

   end_location =
           search_index + login_file_info->file_size - 1;
  /*-------------------------------------------------------------------*/
  /* Get lines from the stanza file as long as we have not reached the */
  /* end of the stanza and we have not found the LOGIN_ENTRY.            */
  /* The 'search_index' variable will point at the first non-blank     */
  /* character on the line.                                            */
  /*-------------------------------------------------------------------*/
   while (
           (search_index =
             get_stanza_line(search_index,end_location,GET_ENTRY)) != NULL )
        {
            if (
                  (search_index[LOGIN_ENTRY_LENGTH] == '='   ||
                     isspace((int) search_index[LOGIN_ENTRY_LENGTH])     )    &&
                 NLstrncmp(search_index,LOGIN_ENTRY,LOGIN_ENTRY_LENGTH) == 0
               )
              break;   /* found the match */

        } /* endwhile */

  login_file_info->maxlogins_start = search_index;


  /*-------------------------------------------------------------------------*/
  /* We have now looked through the file to find the stanza entries.  If the */
  /* login_file_info->maxlogins_start is not NULL, the LOGIN_ENTRY was       */
  /* found.                                                                  */
  /*-------------------------------------------------------------------------*/

  return;
} /* end find_users_stanza */

/*
 *   NAME:      get_stanza_line
 *   FUNCTION:  Look through the login.cfg file starting at 'start_from' and
 *              find the beginning of a line which is part of a stanza.
 *              If want_header is 'GET_HEADER' then find a line which might
 *              be a header line of a stanza.  If want_header is 'GET_ENTRY'
 *              then find a line which might be one of the entry lines of
 *              the stanza.
 *   RETURNS:   A pointer to the beginning of the line if successful.  If
 *              want_header is 'GET_ENTRY' then the pointer points past the
 *              leading whitespace.  If a valid line cannot be found, NULL
 *              is returned.
 *
 */
char *get_stanza_line(start_from,end_location,want_header)
char *start_from;      /* Where to start looking */
char *end_location;    /* Where to stop looking  */
int want_header;       /* Flag. GET_HEADER if looking for stanza header, */
                       /*       GET_ENTRY  if looking for entry in stanza. */
{
  register char *newline_location; /* Pointer to a \n character */
  int header_check;   /* Returned from check_for_header() */

  /*--------------------------------------------------------------*/
  /*  While we have not found a newline character and we are not */
  /* past the end of the file, look for a newline character.      */
  /*--------------------------------------------------------------*/
  newline_location = start_from;

  while (TRUE)
    {
      while (newline_location <= end_location && *newline_location != '\n')
          newline_location += NLchrlen(newline_location);

      /* If we could not find a newline character, return a NULL */

      if (newline_location > end_location)
             return(NULL);

      /* Increment past the newline character */
      newline_location++;

      /* If the line we found is a comment or newline, skip it */

      if (*newline_location == '*' || *newline_location == '#')
          continue;

      /* Skip beginning whitespace if we are not looking for the header */
      if (want_header == GET_ENTRY)
        {
          while (newline_location <= end_location &&
                              isspace((int) *newline_location) )
             newline_location++;

          /* If the rest of the file was whitespace, return NULL */

          if (newline_location > end_location)
                 return(NULL);
        } /* endif */

      /* Find out if this line could be a stanza header */
      header_check = check_for_header(newline_location,end_location);

      /* If this line is a header, but we are looking for an entry,        */
      /* return NULL since the beginning of the next stanza has been found */
      if (header_check == GET_HEADER && want_header == GET_ENTRY)
         return(NULL);


      if (want_header ==  header_check)
          return(newline_location);


    } /* endwhile TRUE */

} /* end of get_stanza_line */

/*
 *   NAME:     check_for_header
 *   FUNCTION: Checks the line beginning at 'start_location' to determine
 *             if it could possibly be the first line of a stanza. The
 *             first line of a stanza looks like 'word:'.
 *   RETURNS:  The value GET_HEADER if the line might be a header line
 *             of a stanza.  Return GET_ENTRY if it is not a header line.
 *
 */
int check_for_header(start_location,end_location)
register char *start_location;      /* The beginning of the line */
register char *end_location;        /* The end of the file        */
{
  /*-----------------------------------------------------------------*/
  /* Since a header of a stanza looks like "word:", see if the thing */
  /* at start_location looks like that.                              */
  /*-----------------------------------------------------------------*/
  while (start_location <= end_location &&
          *start_location != ':'        &&
          *start_location != '='        &&
          !isspace((int) *start_location)             )

                start_location += NLchrlen(start_location);

 /*------------------------------------------------------*/
 /* If we found the :, then this qualifies as a header. */
 /*------------------------------------------------------*/
  if (start_location <= end_location && *start_location == ':')
     return(GET_HEADER);


 /*-----------------------------------------------------*/
 /* This string is does not qualify as a stanza header. */
 /*-----------------------------------------------------*/
 return(GET_ENTRY);
} /* end check_for_header */

/*
 *   NAME:      change_users_stanza
 *   FUNCTION:  Does the actual modification of the login.cfg file to update
 *              the new maximum number of users.
 *   RETURNS:   A zero if successful, -1 otherwise
 *
 */
int change_users_stanza(login_file_info,new_max_users)
struct login_file_type *login_file_info;   /* Login file information */
int new_max_users;                         /* The new number of max users */
{
  char new_buffer[100];          /* To hold the new (part) of the stanza */
  char *new_location;
  char *newline_character;
  char *end_location;
  char *blank_ptr;

  /* Set up the buffer to hold the new value */
  if (login_file_info->stanza_start == NULL &&
      login_file_info->maxlogins_start == NULL)
    {
      /* Need to add the whole stanza */
      sprintf(new_buffer,"\n%s\n\t%s = %d\n",LOGIN_HEADER,LOGIN_ENTRY,new_max_users);

    }
  else if (login_file_info->maxlogins_start == NULL)
    {
      /* Need to add the line for maxlogins only */
      sprintf(new_buffer,"\n\t%s = %d",LOGIN_ENTRY,new_max_users);
    }
  else
    {
      /* Need to add the value only */
      sprintf(new_buffer," %d",new_max_users);
    } /* endif */


  /*------------------------------------------------------------------------*/
  /* Now update the file with the new value.  Depending on what part of the */
  /* stanza was found, we may need to add all or part of the stanza         */
  /*------------------------------------------------------------------------*/
  end_location = login_file_info->mem_file + login_file_info->file_size -1 ;
  if (login_file_info->stanza_start == NULL &&
      login_file_info->maxlogins_start == NULL)
    {
      /* Add the whole stanza since it was not found in the file */
      new_location = end_location + 1;

      login_file_info->file_size += NLstrlen(new_buffer); /* update file size */

      NLstrcpy(new_location,new_buffer);

      /* Set up the stanza pointers */
      login_file_info->stanza_start = new_location + 1;
      login_file_info->maxlogins_start = new_location + 1 +
                                            NLstrlen(LOGIN_HEADER) + 2;


    }
  else if (login_file_info->maxlogins_start == NULL)
    {
      /* Add the line for maxlogins only since the stanza was found but */
      /* the maxlogins entry was not.                                   */

      new_location = login_file_info->stanza_start + LOGIN_HEADER_LENGTH;

      if (new_location <= end_location)
        {
          /* Make room in the file */

          memcpy(new_location + NLstrlen(new_buffer),new_location ,
                              login_file_info->file_size -
                                 (int) (new_location - login_file_info->mem_file));

          login_file_info->file_size +=
                        NLstrlen(new_buffer); /* update file size */
        } /* endif */


      /* Put the entry in the stanza */
      NLstrncpy(new_location,new_buffer,NLstrlen(new_buffer));

      /* Update the stanza pointer */
      login_file_info->maxlogins_start = new_location + 2;

    }
  else
    {
      /* Add the value only.   */

      /* First find the '=' part of the entry */
      new_location = login_file_info->maxlogins_start + LOGIN_ENTRY_LENGTH;
      while (new_location <= end_location &&
             *new_location != '='         &&
             *new_location != '\n'           )
        {
          new_location += NLchrlen(new_location);
        } /* endwhile */

      /* If we're not pointing at a '=' then there is an  error */
      if (*new_location != '=' || new_location > end_location)
        {
         fprintf(stderr,catgets(scmc_catd,LICENSE_SET,LICENSE_EQ,
         "%s: Error in the file %s. The %s entry \n\
\tmust have an = symbol. Contact the systems administrator.\n"),
                                command_name,LOGIN_FILE_PATH,LOGIN_ENTRY);
          return(-1);
        } /* endif */

      /* Go past the '=' to put the new value */
      new_location++;

      if (new_location <= end_location)
        {
          /* Find the end of the line */
          newline_character = new_location;
          while (newline_character + NLchrlen(newline_character)
                                                   <= end_location &&
                 *newline_character != '\n'           )
            {
              newline_character += NLchrlen(newline_character);
            } /* endwhile */

          if (*newline_character != '\n')
            {
              /* Since we did not find a newline in the stanza, add one */
              /* to the end of new_buffer                               */
              NLstrcat(new_buffer,"\n");
            } /* endif */


          /* Blank out the space after the new_location */
          blank_ptr = new_location;
          while (blank_ptr < newline_character)
               *(blank_ptr++) = ' ';

          /* See how much space is between new_location and newline_character */
          /* If there is not enough, we will have to make room in the file.   */

          if (newline_character - new_location < NLstrlen(new_buffer))
            {
              /* Make room in the file */

              memcpy(new_location + NLstrlen(new_buffer) -
                            (newline_character - new_location),
                     new_location ,
                     login_file_info->file_size -
                             (int) (new_location - login_file_info->mem_file));

              login_file_info->file_size +=
                            NLstrlen(new_buffer) -
                            (newline_character - new_location); /* update file size */
            } /* endif */

        } /* endif new_location <= end_location*/

      /* Put the value into the stanza */
      NLstrncpy(new_location,new_buffer,NLstrlen(new_buffer));

    } /* endif */

  /* The change was a success! Return */
  return(0);

}  /* end of change_users_stanza */

/*
 *   NAME:     write_login_file
 *   FUNCTION: If the whole file was read in, this routine will write it
 *             back out.  If the initial 'shmat()' call was successful, this
 *             routine will 'shmdt()' the file.
 *   RETURNS:  A zero if successful, -1 otherwise.
 *
 */
int write_login_file(login_file_info)
struct login_file_type *login_file_info;   /* Login file information */
{
  if (login_file_info->had_to_malloc == FALSE)
    {
      /* All we have to do is detach the file */
      shmdt(login_file_info->mem_file);
      ftruncate(login_file_info->file_id,login_file_info->file_size);
      return(0);
    } /* endif */

  /* Since we read the whole file in, write the whole file out */

  if (lseek(login_file_info->file_id,(off_t) 0,SEEK_SET) != 0)
    {
      /* Could not reset the file pointer! */
     fprintf(stderr,catgets(scmc_catd,LICENSE_SET,LICENSE_SEEK,
     "%s: Could not set the file pointer in the file %s.\n\
\tContact the systems administrator with errno %d.\n"),command_name,LOGIN_FILE_PATH,errno);
     return(-1);
    } /* end if */

  if (write(login_file_info->file_id,
           login_file_info->mem_file,
           (unsigned) login_file_info->file_size) != login_file_info->file_size)
    {
      /* Could not write out the file! */
     fprintf(stderr,catgets(scmc_catd,LICENSE_SET,LICENSE_WRITE,
     "%s: Could not update the file %s.\n\
\tContact the systems administrator with errno %d.\n"),command_name,LOGIN_FILE_PATH,errno);
     return(-1);
    } /* endif */

  free((void *) login_file_info->mem_file);

  /* Successfully wrote the file */
  return(0);
} /* write_login_file */

/*
 *   NAME:     get_maxlogins_value
 *   FUNCTION: Determine the tier in which the maxlogins value belongs.
 *             The login_file_info->maxlogins_start value should already
 *             be initialized when this routine is called.
 *   RETURNS:  The tier for the maxlogins value if successful, -1 otherwise.
 *
 */
int get_maxlogins_value(login_file_info)
struct login_file_type *login_file_info;  /* Login file information */
{
  char *start_location;
  char *end_location;
  char *end_of_number;
  char number_buffer[100];
  int number_of_users;

  /* If we could not find the maxlogins start, return */

  if (login_file_info->maxlogins_start == NULL)
    {
      /* Could not find the maxlogins entry! */
      fprintf(stderr,catgets(scmc_catd,LICENSE_SET,LICENSE_MAXL,
      "%s: Could not find the %s entry in the file %s.\n\
\tUse the command chlicense to set the number of users.\n"),
                      command_name,LOGIN_ENTRY,LOGIN_FILE_PATH);
      return(-1);
    } /* endif */

  end_location = login_file_info->mem_file + login_file_info->file_size -1 ;
  /* First find the '=' part of the entry */
  start_location = login_file_info->maxlogins_start + LOGIN_ENTRY_LENGTH;
  while (start_location <= end_location &&
         *start_location != '='         &&
         *start_location != '\n'           )
    {
      start_location += NLchrlen(start_location);
    } /* endwhile */

  /* If we're not pointing at a '=' then there is an  error */
  if (*start_location != '=' || start_location > end_location)
    {
      fprintf(stderr,catgets(scmc_catd,LICENSE_SET,LICENSE_EQ,
      "%s: Error in the file %s. The %s entry \n\
\tmust have an = symbol. Contact the systems administrator.\n"),
                                command_name,LOGIN_FILE_PATH,LOGIN_ENTRY);
      return(-1);
    } /* endif */


  /* Go past the '=' symbol to look for the number */
  start_location++;

  /* Skip leading blanks */
  while (start_location <= end_location && isspace((int) *start_location))
      start_location++;

  /* The 'start_location' should be pointing at the beginning of the */
  /* number.  Go until we find a non-numeric character               */
  end_of_number = start_location;
  while (end_of_number <= end_location && isdigit((int) *end_of_number) )
             end_of_number++;

  if (end_of_number > end_location || start_location == end_of_number)
    {
      /* Could not find the number in the file! */
      fprintf(stderr,catgets(scmc_catd,LICENSE_SET,LICENSE_FNUM,
      "%s: Could not find the %s value in the file %s.\n\
\tRun the command chlicense to correctly set the number of users.\n"),
                 command_name,LOGIN_ENTRY,LOGIN_FILE_PATH);
      return(-1);
    } /* endif */

  /* Save the number found in the file */
  number_buffer[0] = '\0';
  NLstrncat(number_buffer,start_location,end_of_number - start_location);

  number_of_users = atoi(number_buffer);

  if (number_of_users < 0)
    {
      /* The number is not valid */
      fprintf(stderr,catgets(scmc_catd,LICENSE_SET,LICENSE_BNUM,
      "%s: The %s value which is in the %s file\n\
\tis not valid. Set the number of users with the chlicense command.\n"),
          command_name,LOGIN_ENTRY,LOGIN_FILE_PATH);
      return(-1);
    } /* endif */


  /* Return the number of users */
  return(number_of_users);


} /* end get_maxlogins_value */
