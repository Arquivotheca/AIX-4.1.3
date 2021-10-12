$ @(#)73	1.20.5.1  src/bos/usr/bin/newgrp/tcbauth.msg, cmdsuser, bos41B, 412_41B_sync 12/19/94 16:51:01
$
$   COMPONENT_NAME: CMDSUSER
$
$   FUNCTIONS: none
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1994
$   All Rights Reserved
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "
$set MS_TCBAUTH

$ messages that do not require error numbers

M_SETSENUSAGE "Usage: setsenv [-] [\"variable=value\" ... ]\n"
M_SETGRUSAGE "Usage: setgroups [-] [[[ -a | -d ] groupset ] | [ -r [group]]]\n"
M_NEWGRUSAGE "Usage: newgrp [-] [-l] [group]\n"
M_CHUSRUSAGE "Usage: chuser \"attr=value\" ... user\n"
M_CHMEMUSAGE "Usage: chgrpmem {{ -a | -m } { + | - | = } members} group\n"
M_CHSHUSAGE "Usage: chsh [username [shell]]\n"
M_CHFNUSAGE "Usage: chfn [username]\n"
M_RMUSRUSAGE "Usage: rmuser [-p] user\n"
M_RMGRPUSAGE "Usage: rmgroup group\n"
M_LSGRPUSAGE "Usage: lsgroup [ -c | -f ] [ -a attr attr .. ] { \"ALL\" | group1,group2 ... }\n"
$ Please do not translate the word 'ALL'

M_LSUSRUSAGE "Usage: lsuser [ -c | -f ] [ -a attr attr ... ] { \"ALL\" | user1,user2 ... }\n"
$ Please do not translate the word 'ALL'

M_CHGRPUSAGE "Usage: chgroup \"attr=value\" ... group\n"
M_MKGRPUSAGE "Usage: mkgroup [-a] [-A] \"attr=value\" ... newgroup\n"
M_MKUSRUSAGE "Usage: mkuser [-a] \"attr=value\" ... newuser\n"
NOT_USED     "Usage: pwdadm [ -f flags | -q ] user\n"
M_MKPASSWD "Usage: mkpasswd [ -v ] file\n"
M_PASUSAGE "Usage: passwd [ -f | -s ] [username]\nWhere:\n\t-f\tchanges your finger information\n\t-s\tchanges your login shell.\n")
M_CURRGEK " %s's current gecos: \n\t\t\"%s\""
M_CHANGE " Change (%s) or (%s)? > "
M_TO " To?>"
M_ACURRSHELL " Current available shells:\n"
M_UCURRSHELL " %s's current login shell:\n"
M_MEMBS "\tmembers = %s\n"
M_ADMIN "\tadms = %s\n"
M_CHGPASS "Changing password for \"%s\"\n"
M_CURUSE "\tuser groups = %s\n"
M_CURPROC "\tprocess groups = %s\n"
M_TOOLONG " : Name is too long.\n"
M_PERMISSION " : You do not have permission.\n"
M_NOMEM " : Not enough memory.\n"
M_ERBADVAL " : Value is invalid.\n"
M_ERBADATTR " : Attribute is invalid.\n"
M_ERNOTERM " : Must run from tty.\n"
M_SETPASS "Setting \"%s's\" password to NULL.\n"
M_GECOS " Gecos information not changed.\n"
M_SHELL " Login shell not changed.\n"
M_ACCEXIST " : Account exists.\n"

M_DROP "3004-625 Cannot drop \"%s\" from primary group \"%s\".\n"
M_RMPRIME "3004-626 Cannot remove \"%s's\" primary group \"%s\".\n"
M_USRINFO "3004-627 Could not get user id.\n"
M_EXECL "3004-628 Error execing \"%s\".\n"
M_ISADMIN "3004-629 \"%s\" already administrator"
M_OPENERR "3004-630 Cannot open \"%s\" "
M_IDINFO "3004-631 Couldn't get ids "
M_GERRGET "3004-632 Error getting gecos for \"%s\".\n"
M_SERRGET "3004-634 Error getting shell for \"%s\".\n"
M_ERRSET "3004-635 Error setting shell for \"%s\" to \"%s\""
M_GETAT "3004-640 Error getting attribute \"%s\" for \"%s\""
M_USRNIN "3004-641 User \"%s\" not in member list.\n"
M_NAMINFO "3004-655 Could not get user name from id \"%d\""
M_GOTINT "3004-657 Terminating from signal.\n"
M_KEYWD "3004-658 Keyword \"%s\" "
M_BADFLAG "3004-660 Flag \"%s\" "
M_ERRGET "3004-661 Error getting \"%s\" value"
M_LONGNAME "3004-662 Username \"%s\" is too long.\n"
M_NOAUTH "3004-664 You are not authorized to change \"%s's\" password.\n"
M_IDNOEXIST "3004-665 User \"%lu\" does not exist.\n"

M_NOPASSWD "3004-667 \"%s\" does not exist or could not be opened for reading.\n"
M_NOPWDDBM "3004-668 The DBM files for \"%s\" could not be created.\n"
M_PWDBMFAIL "3004-669 The DBM write to \"%s\" failed.\n"
M_DSTORE "3004-670 Storing user \"%s\" as UID %u.\n"
M_MKPWDFINI "3004-671 There were %d password entries, the longest was %d characters.\n"

M_ERRLOCK "3004-672 Error setting lock on file \"%s\""
M_SETPENV "3004-673 Error setting process environment"
M_MALLOC "3004-674 Error getting memory for process"
M_LONGGROUP "3004-675 Groupname \"%s\" is too long.\n"

M_GRPNINST "3004-676 \"%s\" not in current user groupset.\n"
 M_GRPIN "3004-677 \"%s\" already in current process groupset"
M_ISREEL "3004-678 Group \"%s\" is already real group"
M_GETUSRGRPS "3004-679 Error getting groups for \"%s\".\n"
M_GETPCRED "3004-680 Error getting process credentials"
M_SETPCRED "3004-681 Error setting process credentials"
M_GETPENV "3004-682 Error getting environment"
M_GRPTOLG "3004-683 Exceeds maximum number of groups \"%s\""

M_GRPNONEX "3004-686 Group \"%s\" does not exist.\n"
M_USRNONEX "3004-687 User \"%s\" does not exist.\n"
M_GEXIST "3004-688 Group \"%s\" exists.\n"
M_UEXIST "3004-689 User \"%s\" exists.\n"
M_NODEFGRP "3004-690 No default group.\n"
M_CHGONERR "3004-691 Error changing \"%s\""
M_CHGTOERR "3004-692 Error changing \"%s\" to \"%s\""
M_SETERR "3004-693 Cannot set \"%s\" to \"%s\""
M_ERRADD "3004-694 Error adding \"%s\""
M_ERRADM "3004-695 Error adding \"%s\" to \"%s\""
M_RMERR "3004-696 Error removing \"%s\""
M_LISTERR "3004-697 Attribute \"%s\" is not valid.\n"
M_COMMIT "3004-698 Error committing changes to \"%s\""
M_UNKIVAL "3004-699 \"%d\""
M_UNKSVAL "3004-700 \"%s\""
M_ERRLIST "3004-701 Error listing \"%s\""
M_SETPRIV "3004-702 Cannot set process privilege"
M_CHECK "3004-703 Check \"%s\" file.\n"
M_ERRADDADM "3004-704 Error adding administrator \"%s\""
M_USRTOGRPER "3004-705 Error adding \"%s\" to group \"%s\".\n"
M_RMKEY "3004-706 Cannot remove keyword \"%s\".\n"
M_RMADMERR "3004-707 Error removing administrator \"%s\""
M_RMADMUSR "3004-708 Cannot remove \"%s\" from \"%s\""
M_ERCHGPASS "3004-709 Error changing password for \"%s\""
M_TERMINAL "3004-710 \"%s\" must be run from a valid tty.\n"
M_YPPASSWD  "3004-711 You must use yppasswd on a NIS server.\n"
M_MASTER  "3004-712 You can only change account attributes on the NIS master.\n"
M_YESSTR "yes"
M_NOSTR "no"
M_WARN	"3004-719 Warning: %s does not update %s with the new gid.\n"

CANT_ID  "3004-713 You can only change the User ID on the name server.\n"
CANT_PGRP  "3004-714 You can only change the PRIMARY group on the name server.\n"
CANT_GROUPS  "3004-715 You can only change the Group SET on the name server.\n"
CANT_HOME  "3004-716 You can only change the HOME directory on the name server.\n"
CANT_SHELL  "3004-717 You can only change the Initial PROGRAM on the name server.\n"
CANT_GECOS  "3004-718 You can only change the User INFORMATION on the name server.\n"

M_PWADMUSAGE "Usage: pwdadm [ -f flags | -q | -c ] user\n"
M_FLAGS "flags"
$ Please do not translate the word 'flags'

M_ADMINFLAGS "ADMIN"

M_NODEFHOME "3004-720 No default home directory.\n"
M_MKUSRSYS "3004-721 Could not create user.\n"

M_LASTGRP "3004-684 Current process group set must contain at least one group.\n"
M_CHSECUSAGE "Usage: chsec -f file -s stanza -a \"attr=value\" ...\n"
M_LSSECUSAGE "Usage: lssec [-c] -f file -s stanza -a attr ...\n"
M_FILENAME "3004-722 Only one file name can be specified.\n"
M_BADFILENAME "3004-723 Invalid file name \"%s\".\n"
M_STANZA "3004-724 Only one stanza name can be specified.\n"
M_BADSTANZA "3004-725 Invalid stanza name \"%s\".\n"
M_DBM_ADD_FAIL "3004-726 Cannot update the DBM files.\n\tRun \"mkpasswd\" to correct the DBM files.\n"
M_PWDBADFORM "3004-727 Error reading file \"%s\", line %d is not in the correct format.\n"
