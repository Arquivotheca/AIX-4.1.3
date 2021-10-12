$ @(#)26	1.10  src/bos/usr/bin/pwdck/pwdck.msg, cmdsadm, bos411, 9428A410j 7/30/91 12:32:52
$
$  COMPONENT_NAME: (TCBADM) Trusted System Adminstration
$
$  FUNCTIONS: 
$ 
$  ORIGINS: 27
$ 
$  (C) COPYRIGHT International Business Machines Corp. 1989, 1990
$  All Rights Reserved
$  Licensed Materials - Property of IBM
$ 
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ 

$
$ Messages for the PWDCK command
$

$quote "
$set MS_PWDCK

$ strings to indicate user database errors caught by pwdck
$ 3001-401 to 3001-429

MAUTH "3001-401  The authname \"%s\" has no entry in /etc/security/passwd.\n"
MBADP "3001-402  The user \"%s\" has an invalid password field in /etc/passwd.\n"
MEPLINE "3001-403  Bad line found in /etc/passwd:\n\"%s\"\n"
MEXIST "3001-404  /etc/security/passwd does not exist.\n"
MFLG "3001-405  The user \"%s\" has invalid flags specified.\n"
MIGN "3001-406  \"%s\" ignored.\n"
MLINE "3001-407  Bad line found in /etc/security/passwd:\n\"%s\"\n"
MLST "3001-408  The user \"%s\" has an invalid lastupdate attribute.\n"
MMIS "3001-409  The user \"%s\" is missing the lastupdate attribute.\n"
MMODIF "3001-410  WARNING: The password files were modified by another process\n          while pwdck was running. The other process' changes may be lost.\n"
MNAM "3001-411  The user name \"%s\" in /etc/passwd is invalid.\n"
MNENT "3001-412  The user \"%s\" was not found in /etc/passwd.\n"
MNEW "3001-413  Adding \"%s\" stanza to /etc/security/passwd.\n"
MNFND "3001-414  The stanza for \"%s\" was not found in /etc/security/passwd.\n"
MNPW "3001-415  The user \"%s\" has an invalid password attribute.\n"
MUAUTH "3001-416  The user \"%s\" in /etc/security/passwd is not specified as an\nauthentication name in /etc/security/user.\n"
MDEL "3001-417  Deleting the stanza for \"%s\" from /etc/security/passwd.\n"
MOLDDBM "3001-418  The DBM files for /etc/passwd are out of date.\n          Use the mkpasswd command to update these files.\n"
MORDER "3001-419  Sorting /etc/security/passwd in the same order as /etc/passwd.\n"
MSORT "3001-420  /etc/security/passwd is not in the same order as /etc/passwd.\n"

$ strings used for pwdck interactive mode.
$ they all require yes/no answers.
$ 3001-430 to 3001-450

FAUTH "3001-430  add a stanza to /etc/security/passwd ? "
FBADP "3001-431  replace it with '!' and transfer original password ? "
FEPLINE "3001-432  remove it ? "
FEXIST "3001-433  create /etc/security/passwd ? "
FFLG "3001-434  remove the invalid flags ? "
FLST1 "3001-435  remove it ? "
FLST2 "3001-436  update it ? "
FMIS "3001-437  add it ? "
FMODIF "3001-438  overwrite the files anyway ? "
FNAM "3001-439  remove the entry ? "
FNPW "3001-440  make 'password = *' ? "
FUAUTH "3001-441  remove the stanza from /etc/security/passwd ? "
FSORT "3001-442  sort it ? "


$ strings to indicate system errors
$ 3001-451 to 3001-500

MACLGET "3001-451  pwdck: could not get acl of \"%s\".\n"
MACLPUT "3001-452  pwdck: could not put acl on \"%s\".\n"
MATTR "3001-453  pwdck: could not get attribute for user \"%s\".\n"
MCHOWN "3001-454  pwdck: could not chown %s.\n"
MCREAT "3001-455  pwdck: could not create /etc/security/passwd.\n"
MEPENT "3001-456  pwdck: no entries were found in /etc/passwd.\n"
MGETCONF "3001-457  pwdck: could not read password restrictions.\n"
MLOCK "3001-458  pwdck: could not lock on \"%s\". Please try again later.\n"
MMALLOC "3001-459  pwdck: could not allocate memory.\n"
MMKTEMP "3001-460  pwdck: could not make temp file name.\n"
MOPENDB "3001-461  pwdck: could not open user data base.\n"
MOPENEP "3001-462  pwdck: could not open /etc/passwd.\n"
MOPENESP "3001-463  pwdck: could not open /etc/security/passwd.\n"
MOPENTEP "3001-464  pwdck: could not open temporary /etc/passwd.\n"
MOPENTESP "3001-465  pwdck: could not open temporary /etc/security/passwd.\n"
MPCLGET "3001-466  pwdck: could not get pcl of \"%s\".\n"
MPCLPUT "3001-467  pwdck: could not put pcl on \"%s\".\n"
MPOPEN "3001-468  pwdck: could not popen to run \"%s\".\n"
MREALLOC "3001-468  pwdck: could not reallocate memory.\n"
MCOPYTEMP "3001-469  pwdck: could not copy temp files.\n"
MSTAT "3001-470  pwdck: could not stat %s.\n"
MTCBATTR "3001-471  pwdck: could not get tcb attributes for /etc/security/passwd.\n"
MTIME "3001-472  pwdck: could not get last modified time of \"%s\".\n"
MUNLNK "3001-473  pwdck: could not unlink temporary file %s.\n"
MUSG "3001-474  usage: pwdck [ -p | -y | -n | -t ] [ user1 user2 ... | ALL ]\n"
MWRITE "3001-475  pwdck: could not write into \"%s\".\n"

