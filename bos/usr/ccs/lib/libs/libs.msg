$ @(#)79	1.14.1.4  src/bos/usr/ccs/lib/libs/libs.msg, libs, bos411, 9428A410j 10/12/93 09:21:51
$
$   COMPONENT_NAME: LIBS
$
$   FUNCTIONS: 
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1988,1993
$   All Rights Reserved
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ Security Library Messages
$
$quote "
$set MS_LIBS
$		GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate %o %s  %1$s %2$s etc.
$ These are output formats for file names, directory
$ names, permission codes, command names, and the like,
$ depending on their context within each message. 
$ The program replaces %o,%1$o,etc with an octal number.
$ The program replaces %d,%1$d,etc with a decimal number.
$ the program replaces %.2f with a fixed-point number.
$ the program replaces %u with an unsigned integer. 
$ The program replaces %.1f%% with a percentage.
$ The program replaces %s,%1$s,etc with a string of 
$ characters.   
$ 
$ 
M_MINALPHA  "\ta minimum of %1$d alphabetic character.\n"
$ this message follows either one of M_ATTRFAIL or M_ATTREQ
$ notice "character" is singular, as in one character.
$ 
M_MINOTHER  "\ta minimum of %1$d non-alphabetic character.\n"
$ this message follows either one of M_ATTRFAIL or M_ATTREQ
$ notice "character" is singular, as in one character.
$ 
M_MINDIFF  "\ta minimum of %1$d character not found in old password.\n"
$ this message follows either one of M_ATTRFAIL or M_ATTREQ
$ notice "character" is singular, as in one character.
$ 
M_MAXREPEAT "\ta maximum of %1$d repeated character.\n"
$ this message follows either one of M_ATTRFAIL or M_ATTREQ
$ notice "character" is singular, as in one character.
$ 
M_MINLENGTH  "\ta minimum of %1$d character in length.\n"
$ this message follows either one of M_ATTRFAIL or M_ATTREQ
$ notice "character" is singular, as in one character.
$ 
M_MINAGE "\ta minimum of %1$d elapsed week between changes.\n"
$ this message follows either one of M_ATTRFAIL or M_ATTREQ
M_MINAGES "\ta minimum of %1$d elapsed weeks between changes.\n"
$ this message follows either one of M_ATTRFAIL or M_ATTREQ
$ 
M_NOMATCH "3004-600 The password entry does not match, please try again.\n"
$ 
M_INCHAR "\n3004-601 Password %s contains a character that is not valid.\n"
$ 
M_ATTREQ "\n3004-602 The required password characteristics are:\n"
$ 
M_ATTRFAIL "\n3004-603 Your password must have:\n"
$ 
M_MATCH "3004-604 Your entry does not match the old password.\n"
$ a.) this message is displayed when passwd is run and the user types
$ in the wrong password.  The user cannot do anything about it 
$ (except type in the password correctly the first time).
$ 
M_ENTPROMPT "Enter the new password again:"
$ 
M_NEWPROMPT "New password: "
$ 
M_OLDPROMPT "Old password: "
$ 
M_PROMPT "Password: "
$ 
M_PASREQ "3004-608 Passwords are required.\n\
\tPlease choose a password.\n"
$ 
M_PASEXP "3004-609 Your password has expired.\n\
\tPlease choose a new password.\n"
$
M_PASADM "3004-610 You are required to change your password.\n\
\tPlease choose a new one.\n"
$
M_OPEN "3004-611 Error opening file \"%s\" for write"
$
M_LOCK "3004-612 Error setting lock on file \"%s\".\nPlease try again later.\n"
$
M_WRITE "3004-613 Error writing to \"%s\""
$
$
M_CHDIR "3004-614 Unable to change directory to \"%s\".\n\
\tYou are in \"%s\" instead.\n"
$

M_PWNEW		"%s's New password: "
$ The %s in this message is replaced in the program by the user's name.
$ Notice that this is made possessive in English with the 's.  Make this
$ possessive in the appropriate language and place the %s in the
$ appropriate position.

M_PWEITHER	"Enter %s's password or %s's Old password: "
$ The %s in both instances are replaced in the program with the appropriate
$ users' names.  These user names are to be possessive, and in the case of
$ English, a 's has been concatenated to the name.


M_PWOLD		"%s's Old password: "
$ The %s in this message is replaced in the program by the user's name.
$ Notice that this is made possessive in English with the 's.  Make this
$ possessive in the appropriate language and place the %s in the
$ appropriate position.

M_PWPROMPT	"%s's Password: "
$ The %s in this message is replaced in the program by the user's name.
$ Notice that this is made possessive in English with the 's.  Make this
$ possessive in the appropriate language and place the %s in the
$ appropriate position.

M_MINALPHAS "\ta minimum of %1$d alphabetic characters.\n"
$ notice "characters" is plural, as in multiple characters.
$
M_MINOTHERS "\ta minimum of %1$d non-alphabetic characters.\n"
$ notice "characters" is plural, as in multiple characters.
$
M_MINDIFFS "\ta minimum of %1$d characters not found in old password.\n"
$ notice "characters" is plural, as in multiple characters.
$
M_MAXREPEATS "\ta maximum of %1$d repeated characters.\n"
$ notice "characters" is plural, as in multiple characters.
$
M_MINLENGTHS "\ta minimum of %1$d characters in length.\n"
$ notice "characters" is plural, as in multiple characters.
$
M_SETPASS "Setting \"%s's\" password to NULL.\n"
$ The %s will be replaced by the users name.  A 's is appended for possessive.

M_USRNONEX   "3004-616 User \"%s\" does not exist.\n"

M_GRAMMARERR "3004-617 An authentication error occurred on the \
grammar \"%s\".\n"
$ Authentication in general is the process of logging into the system.
$ The grammar spoken of here is a boolean expression that describes 
$ what authentication methods will be tried during login.
$ 

M_FAILLOCAL  "3004-618 User \"%s\" failed local authentication.\n"

M_FAILLOAD   "3004-619 Security method \"%s\" could not be loaded.\n"

M_NOAUTHPTR  "3004-620 Security method \"%s\" has no authentication function.\n"
$ Within security load modules there should exist a function that can 
$ authenticate the user.

M_NOPASSPTR  "3004-621 Security method \"%s\" has no password change \
function.\n"
$ Within security load modules there should exist a function that can 
$ change a user's password.

M_PUTERR     "3004-622 An error occurred updating the password database.\n"
M_NOAUTH     "3004-664 You are not authorized to change \"%s's\" password.\n"

M_BADLOGIN	"3004-300 You entered an invalid login name or password.\n"
M_USERLOCKED	"3004-301 Your account has been locked; please see the system administrator.\n"
M_EXPIRED	"3004-302 Your account has expired; please see the system administrator.\n"
M_TOOMANYBAD	"3004-303 There have been too many unsuccessful login attempts; please see\n\
\tthe system administrator.\n"
M_USERTIME	"3004-304 You are not allowed to login at this time.\n"
M_NOLOCAL	"3004-305 Local logins are not allowed for this account.\n"
M_NOREMOTE	"3004-306 Remote logins are not allowed for this account.\n"
M_NOSU		"3004-307 You are not allowed to su to this account.\n"
M_NODAEMON	"3004-308 You are not allowed to use SRC or cron.\n"
M_NOTTYS	"3004-309 You are not allowed to access the system via this terminal.\n"
M_TTYLOCKED	"3004-310 This terminal has been locked; please see the system administrator.\n"
M_TTYTIME	"3004-311 This terminal can not be used at this time.\n"
M_LICENSE	"3004-312 All available login sessions are in use.\n"
M_BADLOGTIMES	"3004-313 User %s has an invalid logintimes value of \"%s\".\n"

M_NOPASSREUSE "3004-314 Password was recently used and is not valid for reuse.\n"

M_PUTHISTERR "3004-315 The password database was successfully updated, but an \n\
\terror occurred while updating the password history database.\n"

M_HISTATTR   "3004-316 Error retrieving the password history attributes.\n"

M_HISTDBOPEN "3004-317 Error opening the password history database.\n"

M_BADUSER	"3004-318 Error obtaining the user's password information.\n"
M_PWATTRIBUTE	"3004-319 Error retrieving the \"%s\" attribute.\n"
$ The %s will be replaced with one of the attribute names contained in
$ the /etc/security/usr file.

M_ADMINONLY	"3004-320 Only the system administrator can change this password.\n"
M_SEEADMIN	"3004-321 Please see the system administrator to change your password.\n"
M_BADDATA	"3004-322 \"%s\" defined by your \"%s\" password\n\
attribute could not be executed.  Please see the system administrator.\n"
$ The first  %s will be replaced by the value of an attribute.
$ The second %s will be replaced by the name of the attribute.

M_CHKFAIL	"3004-323 Password composition failure from \"%s=%s\".\n"
$ The first %s will be replaced by the name of an attribute.
$ The second %s will be replaced by the value of the attribute.

M_CHKADMFAIL	"3004-324 Password administration failure from \"%s=%s\".\n"
$ The first %s will be replaced by the name of an attribute.
$ The second %s will be replaced by the value of the attribute.

M_CHKERR	"3004-325 Password internal error from \"%s=%s\".\n"
$ The first %s will be replaced by the name of an attribute.
$ The second %s will be replaced by the value of the attribute.

M_BADSTATE	\
"3004-326 Your password has expired, but your password has not reached\n\
its minimum age requirement.\n"
M_ADMINEXP	"3004-327 Your password has been expired for too long.\n"
M_EXPWARN	"3004-328 Your password will expire: %s\n"
$ The %s will be replaced by a time and date like "Thu Oct 07 20:12:49 1993".

M_BADCRY	"3004-329 Your encrypted password is invalid.\n\
\tPlease choose a new password.\n"
M_BADCRYPT	"3004-330 Your encrypted password is invalid.\n"
M_PASREQED	"3004-331 Passwords are required.\n"
M_PASEXPED	"3004-332 Your password has expired.\n"
M_PASADMIN	"3004-333 A password change is required.\n"
M_DICTERR	"3004-334 Error accessing password dictionary \"%s\".\n"
$ The %s will be replaced by a file name.

M_DICTMATCH	"3004-335 Passwords must not match words in the dictionary.\n"
M_PWRFAIL	"3004-336 Your password does not meet the password requirements.\n\
\tPlease try again.\n"
M_PWRADMFAIL	"3004-337 A password change is not allowed at this time.\n"
M_PWRERR	"3004-338 Security method \"%s\" detected an internal error.\n"
$ The %s is a method name.

M_SLOGIN	"Last login: %s on %s\n"
M_SLOGINHT	"Last login: %s on %s from %s\n"
M_FLOGIN	"Last unsuccessful login: %s on %s\n"
M_FLOGINHT	"Last unsuccessful login: %s on %s from %s\n"
M_FCOUNT	"1 unsuccessful login attempt since last login.\n"
M_FCOUNTS	"%d unsuccessful login attempts since last login.\n"

$ -------------------------------------------------------------------
$ >>>>>>>>>>>>>>> NOTES TO TRANSLATORS AND CODE OWNERS <<<<<<<<<<<<<<
$
$   1. The monthnames and weekday names that appear below MUST remain
$      ALL lower case!  Do NOT capitalize them.
$
$   2. The order of the month and weekday names CANNOT be altered.
$ -------------------------------------------------------------------

$ Week day names - do *NOT* alter capitalization or order

M_SUN "sunday"
M_MON "monday"
M_TUE "tuesday"
M_WED "wednesday"
M_THU "thursday"
M_FRI "friday"
M_SAT "saturday"

$ Month names - do *NOT* alter capitalization or order

M_JAN "january"
M_FEB "february"
M_MAR "march"
M_APR "april"
M_MAY "may"
M_JUN "june"
M_JUL "july"
M_AUG "august"
M_SEP "september"
M_OCT "october"
M_NOV "november"
M_DEC "december"
