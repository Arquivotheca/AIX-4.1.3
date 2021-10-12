$ @(#)78        1.25  src/bos/usr/bin/sysdumpdev/cmddump.msg, cmddump, bos411, 9428A410j 6/14/94 13:22:27
$ COMPONENT_NAME: CMDDUMP    system dump control and format program
$
$ FUNCTIONS: .msg message catalog file for sysdumpdev and dmpfmt
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

$set 1
$quote "

$ "component" is a term that means a piece of a dump file.
$ "data area" is a term that means a piece of a component
$ "primary"   is a term that denotes the primary dump device
$ "secondary" is a term that denotes the secondary dump device
$ "dump file" is a term that means the file which contains the dump image.
$ These are special words, so remember to translate them 
$ the same throughout this file.

CAT_COMPONENT        "component"
CAT_DATA_AREA        "data area"
CAT_PRIMARY          "primary"
CAT_SECONDARY        "secondary"
CAT_DUMPFILE         "dump file"
$ These are not error messages and do not have message numbers.

CAT_DMPMAGIC        "\
0453-001 Bad magic number %1$08x at offset 0x%2$x in dump file %3$s.\n\
This can mean that the dump file is not a system dump image,\n\
or that a portion of the dump file has been corrupted.\n"
$ $ tbl.c
$ This message should not occur unless the dump file has been corrupted
$ somehow. In other words, it will be rare.
$ The term "magic number" is a unix term. It is some number that must be
$ in a particular place in the file, so that if a program does not find
$ it, it knows that the file is bad.
$ The %1$08x will be expanded to the number.
$ The 0x%2$x will be expanded to the file offset where the number was expected.
$ The %3$s  will be expanded to the name of the dump file.

CAT_ARGC1            "\
0453-002 Specify a dump file.\n"
$ $ main.c

CAT_USAGE1           "\
Usage:\n\
%1$s  -bBlai -n da_list -f file_descriptor filename\n\
Format output from system dump image.\n\
-b       batch mode. display selected component to stdout and exit.\n\
-B       batch mode. display all components to stdout and exit.\n\
-l       list data areas of component before prompt.\n\
-a       display data areas of component before prompt.\n\
-i       Take dump input from stdin.\n\
-n list  display data areas in 'list' before prompt.\n\
         ('list' is a list of numbers separated by commas)\n\
-f desc  Use file descriptor 'desc' as input. (fork/exec-ed by crash)\n\
\n"
$ '%1$s' will be expanded to the name of the program, usually sysdumpstart.
$ First line: don't translate anything that starts with '-'.
$ 'da_list' is data area list. You don't have to translate it.
$ don't translate 'file_descriptor'
$ Translate 'filename'. If it takes more than one word, connect them with
$   underscores (_).

CAT_USAGE2           "\
If a file descriptor is not specified, input is from 'filename' and\n\
   the selected component is the first.\n\
If 'filename' is not specified, dump input is taken from stdin,\n\
   interactive subcommand input is taken from /dev/tty, and\n\
   the selected component is the first one.\n\
Use the 'comp' subcommand to change components.\n\
Type '?' at the prompt for help.\n"
$ This is the second part of CAT_USAGE1 and it printed at the same
$ time that CAT_USAGE2 is. I would combine into one but the .cat file
$ cannot hold a message this big. It does not have a message number.
$ Do not translate 'comp' in Use the comp subcommand ...

CAT_HELP             "\
<number>         display selected data area (number is 1-based)\n\
list             list all data areas\n\
all              display all data areas for current component\n\
comp <number>    display all data areas for selected component\n\
comp list        list all components\n\
comp all         display all data areas for all components\n\
comp <number> -  set current component to <number>\n\
set              show current display settings\n\
set ?            show help screen for display settings\n\
quit             exit\n\
?                show this help screen\n\
!                escape to shell\n"
$ $ xtr.c
$ The left side of this message must appear exactly as it is here, except
$ for the <number>. Translate the word "number".
$ The term "1-based" means that the first number in a sequence is 1.
$    (as opposed to "0-based").

CAT_RNGCOMP          "\
0453-005 Component number %d out of range.\n"
$ %d will be expanded to the number of the component.

CAT_RNGDATAAREA      "\
0453-006 Data area number %d out of range.\n"
$ %d will be expanded to the number of the data area.

CAT_COMPNAME         "\
Component Name   %-16.16s\n\n"
$ This is not an error message. It is a descriptive header that I simply
$ want translated. This is why it does not have a message number.
$ %-16.16s will be expanded to the name of the component

CAT_DATAAREA         "\
Data Area   %1$-8.8s\n\
Address     0x%2$08x\n\
Length      0x%3$04x\n\n"
$ This is not an error message. It is a descriptive header that I simply
$ want translated. This is why it does not have a message number.
$ %1$-8.8s will be expanded to the name of the data area
$ %2$08x  will be expanded to a number
$ %3$04x  will be expanded to a number
$ Translate the right side exactly as it appears.
$ "Address" means the in-memory address of the data.
$ "Length"  means the number of bytes of this data area.

CAT_SYSDUMP          "\
0453-008 Cannot open dump device %1$s.\n    %2$s"
$ $ sysdumpdev.c
$ '%1$s' will be expanded to the name of the dump device.
$ '%2$s' will be expanded to a standard system error message.

CAT_DMPNOW           "\
0453-009 Cannot start dump to device %1$s.\n    %2$s\n"
$ '%1$s' will be expanded to the name of the dump device.
$ '%2$s' will be expanded to a standard system error message.

CAT_PROMPT           "\
0453-010 Insert volume on secondary dump device %1$s\n\
         and type <enter> when ready: "
$ This is a message asking the user insert a mountable volume
$ in the dump device and then to type the <enter> key on the keyboard
$ '%1$s' will be expanded to the name of the secondary dump device.

CAT_DMPSET           "\
0453-011 Cannot initialize dump device %1$s.\n    %2$s\n"
$ '%1$s' will be expanded to the name of the dump device.
$ '%2$s' will be expanded to a standard system error message.

CAT_IOCINFO          "\
0453-012 Cannot read dump device information from %1$s.\n    %2$s"
$ '%1$s' will be expanded to the name of the dump device.
$ '%2$s' will be expanded to a standard system error message.

CAT_USAGESTART       "\
Usage: %1$s -p | -s [-f]\n\
\n\
-p     Start a dump to the primary device.\n\
-s     Start a dump to the secondary device.\n\
-f     Force. Suppress prompt to make secondary dump device ready.\n"
$ '%1$s' will be expanded to the name of the program, usually sysdumpstart.
$ Print the Usage: line exactly as it is, Translate the word 'Usage'.
$ Preserve column alignment if you need additional lines. For example, if the
$ -f line exceeds 80 characters, start the next line underneath the word
$ 'Force' and not at the start of the line.

CAT_USAGEDEV         "\
Usage:\n\
%1$s [-P] [-p Device | -s Device]\n\
%2$s [-l | -q | -e | -z | -r Host:Path | -p Device | -s Device | -L]\n\
%3$s [-d | -D] Directory\n\
\n\
Change the primary and secondary dump device designations\n\
in a running system.\n\
\n\
-d Directory  Specify the directory where the dump is copied to at\n\
              boot time.  If the copy fails the system continues to boot.\n\
-D Directory  Specify the directory where the dump is copied to at\n\
              boot time.  If the copy fails then a menu is displayed to\n\
              allow user to copy the dump.\n\
-e            Estimate the size of the dump (in bytes) for the\n\
              current running system.\n\
-l            List the current dump device designations.\n\
-L            Display statistical information about the previous dump.\n\
-p Device     Change the primary dump device to the specified\n\
              device temporarily.\n\
-P            Make the dump device specified by -p or -s flags permanent.\n\
              Can only be used with -p or -s flags.\n\
-q            Suppress any error messages that are written\n\
              to stdout.\n\
-r Host:Path  Free the space used by the remote dump file.\n\
-s Device     Change the secondary dump device to the specified\n\
              device temporarily.\n\
-z            Write out to stdout the string containing the size \n\
              of the dump in bytes and the name of the dump device\n\
              if a new dump is present.\n\
\n\
If no flag is specified, the permanent dump device designations are used.\n"
$ '%s' will be expanded to the name of the program, usually sysdumpdev.
$ Do not translate stdout and stderr.
$ Print the Usage: line exactly as it is. Translate the word 'Usage' and the
$  words device. Connect multiple words with
$  an underscore '_'.
$ Preserve column alignment if you need additional lines. See example for
$  CAT_USAGESTART.
$ -f line exceeds 80 characters, start the next line underneath the word
$ 'Force' and not at the start of the line.

CAT_DST_BOTH         "\
0453-016 Cannot start a dump to both primary and secondary devices\n"

512          "YES"
513          "NO"
514          "Warning"
515          "Line"

CAT_LVM_HELP         "\
0453-017 lbuf <address>   show pbuf(s) corresponding to lbuf\n"
$ Do not translate lbuf and pbuf.  "address" is a memory location.
$ "show pub(s)" means that the command will display one or more addresses
$ of pbufs that are being used to fulfill the request in the lbuf.

CAT_DUP_DEV         "\
0453-018 Primary and secondary dump devices cannot be the same.\n"

CAT_DMP_NOREC         "\
0453-019 No previous dumps recorded.\n"

CAT_IOCSTAT          "\
0453-021 Cannot read dump information from %1$s.\n    %2$s"
$ '%1$s' will be expanded to the name of the dump device.
$ '%2$s' will be expanded to a standard system error message.

CAT_ODMINIT           "\
0453-022 Cannot initialize ODM.\n\
%s\n"
$ do not translate ODM.
$ '%s' will be expanded to a standard system error message.

CAT_ODM_PRIM          "\
0453-023 Cannot read primary dump device from ODM object class SWservAt.\n"
$ do not translate ODM.
$ do not translate SWservAt.

CAT_ODM_SEC          "\
0453-024 Cannot read secondary dump device from ODM object class SWservAt.\n"
$ do not translate ODM.
$ do not translate SWservAt.

CAT_HOSTUNREACH          "\
0453-025 Host %s does not respond or is not reachable.\n"
$ '%s' will be expanded to the name of the host.

CAT_DMPFMT_FILE		"\
0453-026 Specify a dmpfmt file.\n"
$ Do not translate dmpfmt

CAT_BAD_CHAR		"\
0453-027 Bad character '%c' input\n"

CAT_ODM_INIT		"\
0453-028 Cannot initialize ODM.\n"
$ do not translate ODM.

CAT_COMP_INVALID	"\
0453-029 %s not a component\n"
$ xtr.c
$ %s will be expanded to be the name of the component

CAT_INV_INPUT	"\
0453-030 The input string should be in the form of <hostname>:<pathname>\n"

CAT_CANT_CREATE_TDIR    "\
0453-031 Unable to create the temporary directory %s.\n"
$ %s will be expanded to the name of the directory

CAT_CANT_CREATE_FILE    "\
0453-032 Unable to create file %1$s on host %2$s.\n"
$ %1$s will be expanded to the name of the file
$ %2$s will be expanded to the name of the host

CAT_CANT_MOUNT    "\
0453-033 Unable to mount the directory %1$s from host %2$s\n"
$ %1$s will be expanded to the name of the directory
$ %2$s will be expanded to the name of the host

CAT_CANT_UMOUNT    "\
0453-034 Unable to umount %s.\n"
$ %s will be expanded to the name of the directory

CAT_UNABLE_TO_ACCESS    "\
0453-035 Unable to access file %1$s on host %2$s.\n"
$ %1$s will be expanded to the name of the file
$ %2$s will be expanded to the name of the host

CAT_UNABLE_TO_OPEN    "\
0453-036 Unable to open file %1$s on host %2$s.\n"
$ %1$s will be expanded to the name of the file
$ %2$s will be expanded to the name of the host

CAT_CANT_SAME    "\
0453-037 Remote host can not be the same as local host.\n"

CAT_CANT_COPY    "\
0453-038 Copy a remote dump file is not allowed.\n"

CAT_DMP_DEVINFO2       "\
0453-039\n\
\n\
Device name:         %1$s\n\
Major device number: %2$d\n\
Minor device number: %3$d\n\
Size:                %4$d bytes\n\
Date/Time:           %5$s\n\
Dump status:         %6$d\n"

CAT_DUMP_SIZE       "\
0453-041 Estimated dump size in bytes: %d\n"
$ '%d' will be expanded to the estimated size of the system dump.

$ The following messages are added for 4.1 release

CAT_PUTATTR     "\
0453-043  Unable to put %s attribute into ODM object class SWservAt.\n"
$ do not translate ODM.

CAT_GETATTR     "\
0453-044  Unable to get %s attribute from ODM object class SWservAt.\n"
$ do not translate ODM.

CAT_NOT_ROOTVG "\
0453-045 %s is not in volume group rootvg.\n"
$ do not translate rootvg

CAT_MALLOC      "\
0453-046 Unable to allocate memory for mntctl call.\n"
$ do not translate mntctl

CAT_NOT_DIR  "\
0453-047 %s is not a directory.\n"

CAT_NOT_JFS  "\
0453-048 %s is not in a journaled filesystem.\n"

CAT_MOUNT_ERR "\
0453-049 %s Error in mntctl call.\n"

CAT_INVALID_DEV "\
0453-050 %s is not a valid dump device.\n"

CAT_AUTOCOPY "\
copy directory"

CAT_FORCECOPY "\
forced copy flag"

CAT_CANT_GET_PVID "\
0453-053 Unable to get physical volume id for %s from ODM.\n"
$ do not translate ODM.

CAT_CANT_GET_PVNAME "\
0453-054 Unable to get physical volume name for %s from ODM.\n"
$ do not translate ODM.

CAT_CANT_GET_LOC "\
0453-055 Unable to get the location code for %s from ODM.\n"
$ do not translate ODM.

CAT_CANT_GETATTR "\
0453-056 Unable to get attribute %1$s for %2$s in ODM.\n"
$ do not translate ODM.

CAT_CANT_GET_LIST "\
0453-056 No attribute %1$s with the value of %2$s in ODM.\n"
$ do not translate ODM.

CAT_BAD_MOUNTPT "\
0453-057 The specified directory %s must be in one of the\n\
 	 following filesystems: /, /usr, /home/, /tmp, /var\n"

CAT_NO_PATH "\
0453-058 Enter the full pathname to the file %1$s on the remote host %2$s.\n"

CAT_COPY_FILENAME "\
Dump copy filename: %1$s\n"

CAT_BAD_HD6 "\
0453-059 The dump on %s is no longer valid.  To see if\n\
the dump was copied to a file on boot, run the following command:\n\
       sysdumpdev -L\n"

CAT_REMOTE_MP "\
0453-060 Remote dump is not supported in a multiprocessor environment.\n"
