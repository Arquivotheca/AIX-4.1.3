$ @(#)19\t1.5  src/bos/usr/bin/learn/learn.msg, cmdlearn, bos411, 9428A410j 3/22/93 13:19:08
$
$ COMPONENT_NAME: (CMDLEARN) provides computer-aided instruction courses
$
$ FUNCTIONS: none
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

$
$ messages for learn
$quote "         define quote character.

$len
$set MS_LEARN        Define initial set#

$ $ All message numbers for this file should begin with 0.
$ Do not translate "learn"

LTYPEBYE             "Type the following to exit the learn facility: bye\n"
$ Do not translate "bye".

LINLESSON            "The current lesson is %1$s of %2$s. The speed rating is %3$d.\n"

LHAVECOMPLETED       "%1$d out of %2$d lessons have been completed.\n"

LNOCD                "Cannot use the cd command within the learn facility.\n"

LALRDYLRN            "You are already in learn.\n"

LFAIL                "%1$s L%2$-6s fail %3$2d %4$s\n"
$ Use blanks to align variables after "pass" and "fail" are translated.

LPASS                "%1$s L%2$-6s pass %3$2d %4$s\n"
$ Use blanks to align variables after "pass" and "fail" are translated.

LDOUNIT              "dounit:  There is no %s lesson \n"

LSORRYSTILL          "\n\
That is still not correct.  Do you want to try again?  \n\
\tPress y for yes or n for no.  "
$ Leave two spaces at the end of this sentence.

LSORRY               "\n\
That is not correct.  Do you want to try again?  \n\
\tPress y for yes or n for no.  "
$ Leave two spaces at the end of this sentence.

LTRYPROBAGN          "Try the problem again.\n"

LWHENEVER            "To read the lesson again, type the following: again\n"
$ If you translate the word "again" that's at the end of the sentence,
$ you must use the SAME word for the message LAGAIN (the LAGAIN message
$ is further down in this file.

LYOUCANLEAVE         "To exit from learn at any time, type the following: bye\n"
$ Do not translate "bye".

LBYE                 "bye"

LOKTHATWAS           "\n\
The %s lesson is complete.\n"

LSKIPPING            "Skipping to the next lesson.\n\n"

LPLEASETYPE          "Please type yes, no, or bye:  "
$ Do not translate "bye". Leave two spaces at the end of the sentence.

LNOLESSON            "There is no %1$s lesson.  Try the %2$s lesson instead.\n\n"

LTHEREISNOLESSON     "There is no %s lesson.\n"

LGETLESSONCANT       "getlesson:  Cannot do %s.\n"
$ do not translate "getlesson".

LTRYINGLESSON        "Trying the %s lesson.\n\n"

LNOLESSONCONTAIN     "There is no lesson containing %s.\n"

LINTERRUPT           "Received an interrupt signal.\n\
Do you want to continue?  Press y for yes, n for no.  "
$ leave two spaces at the end of this sentence.

LEXECERROR           "learn: 0703-001 Cannot run the designated program.\n"
$ do not translate "learn".

LMAKTEE              "maktee: 0703-002 Cannot run the lrntee command.\n"
$ do not translate "lrntee" or "maktee".

LERRORTEE            "maktee: 0703-003 Cannot make a tee for copyout.\n"
$ do not translate "maktee", "tee", or "copyout".

LTOOMNYLESS          "setid: 0703-004 There are too many lessons.\n"
$ do not translate "setdid".

LLESSNMTOOLNG        "setid: 0703-005 The lesson names exceed the character limit.\n"
$ do not translate "setdid".

LLESSSEQ             "%1$3d lesson %2$7s sequence %3$3d.\n"
$ "sequence" is refering to a lesson sub-topic. This is a table heading, the
$ column alignment must stay the same.

LREADY               "ready"

LANSWER              "answer"

LPPRINT              "#print"
$ do not translate.

LPCOPYIN             "#copyin"
$ do not translate.

LPUNCOPYIN           "#uncopyin"
$ do not translate.

LPCOPYOUT            "#copyout"
$ do not translate.

LPUNCOPYOUT          "#uncopyout"
$ do not translate.

LPPIPE               "#pipe"
$ do not translate.

LPPUNPIPE            "#unpipe"
$ do not translate.

LPPSUCCEED           "#succeed"

LPFAIL               "#fail"

LCHDIR               "chdir"
$ do not translate.

LCD                  "cd"
$ do not translate.

LLEARN               "learn"
$ do not translate.

LPLOG                "#log"
$ do not translate.

LYES                 "yes"

LNO                  "no"

LAGAIN               "again"
$ See the comment above in the message LWHENEVER.

LPMV                 "#mv"
$ do not translate.

LPUSER               "#user"
$ do not translate.

LPNEXT               "#next"

LSKIP                "skip"

LWHERE               "where"
$ do not translate.

LPMATCH              "#match"

LPBAD                "#bad"

LPCREATE             "#create"
$ do not translate.

LPCMP                "#cmp"
$ do not translate.

LHINT                "hint"

LPONCE               "#once"

LEXECVFAIL           "mysys: 0703-030 The execv system call failed on %s.\n"
$ do not translate "mysys" and "execv".

LCANTCD              "selsub: 0703-031 Cannot change to a directory that is not standard.\n"
$ do not translate "selsub".

LTAKINGUP            "Beginning where you finished last time at %1$s %2$s.\n"
$ do not translate "learn".

LRMNRENETER          "%1$s\n\
  rm $HOME/.learnrc\n\
  learn %2$s\n"
$ do not translate.

LTOSTART             "  To begin this lesson again, exit the learn facility by typing:  bye.\n\
\tThen enter the following two command lines: "

LTHESEAVAIL          "The available courses are:\n"

LWANTMOREINFO        "To get more course information,\n\
or if you are using the learn command for the first time,\n\
press the Enter key.  Otherwise type the name of\n\
the course you want, and press the Enter key.\n"

LWANTMOREINFO2       ""
$ This is a null message. There should be NO blank line or text.

LWANTMOREINFO3       ""
$ This is a null message. There should be NO blank line or text.

LWANTMOREINFO4       ""
$ This is a null message. There should be NO blank line or text.

LWHICHSUB            "\nWhich subject do you want?  "

LSUBJ1               "Press the Enter key to start at the beginning of the lesson,\n\
or specify a lesson number to go directly to a different lesson.\n\
A key word can be used in place of the lesson number.\n\
When a key word is used, the learn command will begin at the first lesson\n\
which contains that word.\n"

LSUBJ2               ""
$ This is a null message. There should be NO blank line or text.

LSUBJ3               ""
$ This is a null message. There should be NO blank line or text.

LSUBJ4               ""
$ This is a null message. There should be NO blank line or text.

LSUBJ5               ""
$ This is a null message. There should be NO blank line or text.

LSUBJ6               ""
$ This is a null message. There should be NO blank line or text.

LSUBJ7               ""
$ This is a null message. There should be NO blank line or text.

LCANTCDPUB           "selsub: 0703-032 Cannot change to a public directory.\n"

LCOUNDNTPLAY         "selsub: 0703-033 Cannot make a work directory with %s.\n\
Exiting learn.\n"

LLEAVINGLRN          "Exiting learn.\n"

LNOSUBORLESS         "learn: 0703-034 There is no subject or lesson named %s.\n\
Exiting learn.\n"
$ do not translate "learn".

LWHATELSSN           "Select a lesson number or name:"

LNOSCRIPT            "selunit: 0703-035 There is no script for lesson %s.\n"
$ do not translate "selunit"

LCONGRAT             "Congratulations. You have finished this sequence.\n"

LNOALTLESS           "There are no alternative lessons at this stage.\n"

LGETHELP             ""
$ This is a null message. There should be NO blank line or text.

LSTPLYDIR            "start: Cannot open work directory."
$ do not translate "start".

LSTRTNOLESS          "start: 0703-036 There is no %s lesson.\n"
$ do not translate "start".

LTHATWASLESS         "\n\
The %s lesson is complete.\n\n"

LGOODLESSON          "\n\
Good. The %s lesson is complete.\n\n"

LCANTRM              "wrapup: 0703-037 Cannot find the rm command.\n"
$ Do not translate wrapup.

LBYEDOT              ""
$ This is a null message. There should be NO blank line or text.
