#!/usr/bin/ksh
# @(#)07        1.5  src/bldenv/lockname/namedb2h.sh, ade_build, bos41J 4/26/95 12:22:19
#
# COMPONENT_NAME: LOCKSTAT
#
#
# FUNCTIONS: namedb2h
#
# ORIGINS: 83 27
#
# (C) COPYRIGHT International Business Machines Corp. 1984, 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME: namedb2h

#
# LEVEL 1,  5 Years Bull Confidential Information
#

# FUNCTION: Generates sys/lockname.h
#           from src/bos/usr/lpp/perfagent/lockstat/lockname.db

ECHO=$ODE_TOOLS/usr/bin/echo
GREP=$ODE_TOOLS/usr/bin/grep
SED=$ODE_TOOLS/usr/bin/sed
AWK=$ODE_TOOLS/usr/bin/awk
CAT=$ODE_TOOLS/usr/bin/cat
CC=$ODE_TOOLS/usr/bin/cc
DIFF=/usr/bin/diff
SORT=$ODE_TOOLS/usr/bin/sort
RM=$ODE_TOOLS/usr/bin/rm
#
# cleanup temp files
#
cleanup()
{
     $RM -f /tmp/loaa$$ /tmp/loab$$ /tmp/loac$$
}
#
# to cleanup in case of problems
#
trap "cleanup;exit 2" 2
trap "cleanup;exit 3" 3
#
# Skip comment lines, and keep just lock_family_names from lockname.db
# while we are at it, check size of subsystem name (<= 6 chars)
#
AATEMPNAM=/tmp/loaa$$
export AATEMPNAM
$GREP -v "^\#" lockname.db | $SED -e 's/\$/ /' | $AWK '
BEGIN { "echo $AATEMPNAM" | getline tempname ;
}
{ if (NF){
     if (length ($1) > 6)
          printf ("WARNING: subsystem name: %s  name too long (%d c.)\n",$1,length($1));
     printf("%s\n",  $2) > tempname ;
     }
}'
#
# search for duplicate: sort the lock_family_names without and with suppression
# of duplicates and see if the results are different
#
$SORT </tmp/loaa$$ >/tmp/loab$$
$SORT -u </tmp/loaa$$ >/tmp/loac$$
$DIFF /tmp/loab$$ /tmp/loac$$  >/tmp/loaa$$
if [ $? -ne 0 ]
then
#
# if so, there must be duplicates; print them and exit
#
        $ECHO "DUPLICATE: "
        $GREP '\<' /tmp/loaa$$ | $AWK '
                { printf("      %s\n",$2); }'
     cleanup;
     exit 1
fi
cleanup;
#
# Otherwise build lockname.h from lockname.db:
#     - copy CMVC header (SCCS string, copyrights, ...
#     - strip the component name (eg PROC$)
#     - check for names too long (> 20 chars)
#     - insert the "#define " at the beginning
#
$RM -f ./lockname.h
$SED -e '
s/# @(#)/\/\* @(#)/
s/^#/ \*/
s/\$/ /
s/lockname\.db/lockname.h/g
' < lockname.db | $AWK '
BEGIN {
	inheader = 1;
}
/ lock symbolic name/ {
	printf (" */\n\n\n") >> "./lockname.h";
	printf ("/*\n") >> "./lockname.h";
	printf (" * WARNING: This header file is for debug purposes only.\n") >> "./lockname.h";
	printf (" *          The labels and values herein might change with\n") >> "./lockname.h";
	printf (" *          each release of AIX\n */\n\n") >> "./lockname.h";
	printf ("#ifndef _H_LOCKNAME\n") >> "./lockname.h";
	printf ("#define _H_LOCKNAME\n\n") >> "./lockname.h";
	printf ("/*\n") >> "./lockname.h";
	print >> "./lockname.h";
	printf (" *\n */\n\n\n") >> "./lockname.h";
	inheader = 0;
	next;
	}
{
	if (inheader)   {
		print >> "./lockname.h";
		next;
	} else if ($1 == "#")
		next;

        if (NF == 3) {
          lockname=$2;
          if (length($2) > 20)
               printf ("WARNING: lockname: %s  name too long (%d c.)\n",$2,length($2));
          for (i=length($2);i<=34;i++)
               lockname = lockname " " ;
               printf ("#define %s %6d\n",lockname,int($3)) >>"./lockname.h" ;
     }
}
END { printf("\n#endif /* _H_LOCKNAME */\n") >> "./lockname.h";
}'
cleanup;
exit 0;

