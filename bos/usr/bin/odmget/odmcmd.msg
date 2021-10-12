$ @(#)05	1.8.1.5  src/bos/usr/bin/odmget/odmcmd.msg, cmdodm, bos411, 9428A410j 4/15/94 14:17:03
$
$   COMPONENT_NAME: CMDODM
$
$   FUNCTIONS: 
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$set 1          This is the set of messages for the odmcreate command.
$               These messages are for those error return codes generated
$               by the odmcreate command proper.  The error codes are in
$               the range 5000 through 5030.  The message numbers are in
$               the range 0 through 30.
$    Do not translate odmcreate, %$s or %$d.
$quote "

1       "0518-001 odmcreate: Specify a valid descriptor type, \n\
\tline %1$d in file %2$s.\n"
$       /* 5001 = ODM_NOTYPE_ERR */

2       "0518-002 odmcreate: Specify descriptor %1$s as an array, \n\
\tline %2$d in file %3$s.\n"
$       /* 5002 = ODM_NOTARRAY_ERR */
$ The %1$s refers to a descriptor name.

3       "0518-003 odmcreate: Do not specify descriptor %1$s as an array,\n\
\tline %2$d in file %3$s.\n"
$       /* 5003 = ODM_ARRAY_ERR */
$ The %1$s refers to a descriptor name.

4       "0518-004 odmcreate: Incorrect descriptor type specified,\n\
\tline %1$d in file %2$s. Specify a vaild descriptor type.\n"
$       /* 5004 = ODM_UNKNOWNTYPE_ERR */

5       "0518-005 odmcreate: The descriptor %1$s does not exist in class %2$s,\n\
\tline %3$d in file %4$s. \n\
\tSpecify a valid descriptor name for an ODM_LINK descriptor.\n"
$       /* 5005 = ODM_COLNOTFOUND_ERR */
$ Do not translate ODM_LINK.  The %1$s refers to a descriptor name, %2$s is an
$ object class name.

6       "0518-0006 odmcreate: Specify less than 128 descriptors in \n\
\tobject class %1$s, line %2$d in file %3$s.\n"
$       /* 5006 = ODM_MAXDESC_ERR */
$ The %1$s refers to an object class name.

7       "0518-007 odmcreate: An internal problem has been detected.\n\
\tDepending upon where this product was acquired,\n\
\tcontact a service representative or the approved supplier.\n"
$       /* 5007 = ODM_INTERNAL_ERR */

8       "0518-008 usage: odmcreate [-p][-h][-c] {File . . . }\n\
\tCreates object classes based on descriptions in the file.\n"
$       /* 5008 = ODM_ARG_ERR */
$ Do not translate -p, -h, or -c.

9       "0518-009 odmcreate: Syntax error in description file: \n\
\tline %1$d in file %2$s. \n"
$       /* 5009 = ODM_SYNTAX_ERR */

10      "0518-010 odmcreate: An incorrect array dimension specified, \n\
\tline %1$d in file %2$s. Specify an integer greater than 1.\n"
$       /* 5010 = ODM_BAD_DIM_ERR */

11      "0518-011 odmcreate: The name of the object class is too long,\n\
\tline %1$d in file %2$s.  Specify a name less than %3$d characters long.\n"
$       /* 5011 = ODM_NAMELEN_ERR */
$ The %3$d refers to a number.

12      "0518-012 odmcreate: Cannot open header file %1$s.\n\
\tCheck path name and permissions.\n"
$       /* 5012 = ODM_HDRFILE_ERR */
$ The %1$s refers to an object class name.

13      "0518-013 odmcreate: Cannot open class description file %1$s.\n\
\tCheck path name and permissions.\n"
$       /* 5013 = ODM_DESCFILE_ERR */
$ The %1$s refers to a file.

14      "0518-014 odmcreate: Cannot append to file %1$s.\n\
\tCheck path name and permissions.\n"
$       /* 5014 = ODM_APPEND_ERR */
$ The %1$s refers to a file.

15      "0518-015 odmcreate: Object class %1$s is not described in \n\
\tdescription file, line %2$d in file %3$s.\n\
\tObject classes linked to must be defined in class description file.\n"
$       /* 5015 = ODM_CLASSNOTFOUND_ERR */
$ The %1$s refers to a class name.


16      "0518-016 odmcreate: Cannot run preprocessor on class description \n\
\tfile %1$s. Check file for syntax errors.\n"
$       /* 5016 = ODM_CPP_ERR */

17      "0518-017 odmcreate: Cannot access the database at this time.\n\
\tTry again later or contact the system administrator.\n"
$       /* 5017 = ODM_INIT_ERR */

18      "0518-018 odmcreate: Cannot open output source file %1$s.\n\
\tCheck path name and permissions.\n"
$       /* 5018 = ODM_SRCFILE_ERR */
$ The %1$s refers to a file.

19      "0518-019 odmcreate: Cannot allocate enough storage.\n\
\tTry again later or contact the system administrator.\n"
$       /* 5019 = ODM_MALLOC_ERR */

20      "0518-020 odmcreate: Cannot close database.\n\
\tTry again later or contact the system administrator.\n"
$       /* 5020 = ODM_TERM_ERR */


22      "0518-022 odmcreate: Cannot create object class %1$s.\n\
\tCheck path name and permissions.\n"
$       /* 5022 = ODM_CREATE_FAIL */
$ The %1$s refers to a file.

$       /* 5023 not used */

24      "0518-024 odmcreate: The C compiler must exist when \n\
\tthe -p flag is specified."
$       /* 5024 = ODM_PREREQ_ERR */

25      "0518-025 odmcreate: Too many object classes specified in \n\
\tthe file %1$s.  Specify less than 5025 classes in one file.\n"
$       /* 5025 = ODM_MAX_CLASSES */

26       "0518-000 odmcreate: Do not use clone to imbed class description, \n\
\tline %1$d in file %2$s.  Copy the description instead.\n"
$       /* 5000 = ODM_CLONE_ERR */


$set 2          This is the message set for the odmdrop command.  The messages
$               range from 5000 to 5005
$ Do not translate odmdrop, %$s or %$d.

1       "0518-101 odmdrop: Cannot access object class %1$s\n\
\tCheck path name and permissions.\n"
$       /* 5001 = DROP_ACCESS */
$ The %1$s refers to a file.

2       "0518-102 odmdrop: Object class name is too long.  \n\
\tCheck path name and permissions.\n"
$       /* 5002 = DROP_NAMLEN */

3       "0518-103 odmdrop: An object class name was not specified.\n\
\tSpecify a vaild object class.\n"
$       /* 5003 = DROP_NONAME */

4       "0518-104 odmdrop: Cannot initialize the database.\n\
\tTry again later or contact the systems administrator.\n"
$       /* 5004 = ODM_INIT_ERR */

5       "0518-105 odmdrop: Cannot delete object class %1$s\n\
\tCheck path name and permissions.\n"
$       /* 5005 = DROP_FAILED */
$ The %1$s refers to a file.

6       "0518-100 usage: odmdrop -o class_name\n\
\tDeletes object classes.\n"
$       /* 5000 = DROP_USAGE */
$ Do not translate -o


$set 3          This is the message set for the odmchange command.  The
$               messages range from 5000 to 5017
$ Do not translate odmchange, %$s or %$d.

1       "0518-201 odmchange: The search string is too long.\n\
\tSpecify a search string less than %d characters.\n"
$       /* 5001 = CHANGE_STRLEN */
$ The %d refers to a number.

2       "0518-202 usage: odmchange -o classname [-q criteria] File \n\
\tChanges objects in a given object class.\n"
$       /* 5002 = CHANGE_USAGE */
$ Do not translate -o or -q

3       "0518-203 odmchange: An object class name was not specified.\n\
\tSpecify a vaild object class.\n"
$       /* 5003 = CHANGE_NONAME */

4       "0518-204 odmchange: Description file name too long\n\
\tSpecify a file name less than %d characters.\n"
$       /* 5004 = CHANGE_FNAMLEN */
$ The %d refers to a number.

5       "0518-205 odmchange: Description file %1$s is not accessible\n\
\tCheck path name and permissions.\n"
$       /* 5005 = CHANGE_ACCESS */
$ The %1$s is a file name.

6       "0518-206 odmchange: Cannot initialize the database.\n\
\tTry again later or contact the systems administrator.\n"
$       /* 5006 = CHANGE_INIT_ERR */

7       "0518-207 odmchange: Cannot open object class %1$s\n\
\tCheck path name and permissions.\n"
$       /* 5007 = CHANGE_OPEN_ERR */
$ The %1$s refers to a file.

8       "0518-208 odmchange: Cannot find class name in stanza, stanza line: %1$d\n\
\tThe class name must exist in the description file.\n"
$       /* 5008 = CHANGE_STANZA_NOTFOUND */
$ The %1$d refers to a line number.

9       "0518-209 odmchange: Syntax error in description file, stanza line: %1$d.\n"
$       /* 5009 = CHANGE_INVALID_STANZA */

10      "0518-210 odmchange: Incorrect criteria specified: %1$s\n\
\tCorrect criteria and try again.\n"
$       /* 5010 = CHANGE_BAD_CRITERIA */
$ The %1$s refers to the criteria.

11      "0518-211 odmchange: Error occurred changing data in object class: %1$s\n\
\tCheck stanza and try again.\n"
$       /* 5011 = CHANGE_BAD_DATA */
$ The %1$s refers to a file.

12      "0518-212 odmchange: A descriptor name was not specified, stanza line: %1$d.\n\
\tA descriptor must be specifed in the stanza.\n"
$       /* 5012 = DESC_NOT_FOUND */
$ The %1$d refers to a line number.

13      "0518-213 odmchange: Descriptor %1$s does not exist in object class,\n\
\tstanza line: %2$d.  Correct descriptor name and try again.\n"
$       /* 5013 = INVALID_DESC */
$ The %1$s refers to a descriptor name.

14      "0518-214 odmchange: Descriptor value does not correspond to descriptor type,\n\
\tstanza line: %1$d.  Correct descriptor value and try again.\n"
$       /* 5014 = INVALID_VALUE */

15      "0518-215 odmchange: Could not convert binary value, stanza line: %1$d.\n\
\tBinary values should start with 0x and contain an even number of \n\
\thexadecimal digits.\n"
$       /* 5015 = INVALID_BINARY */
$ Do not translate 0x.

16      "0518-216 odmchange: Incorrect numeric value, stanza line: %1$d.\n\
\tNumeric values should contain only the digits 0-9.\n"
$       /* 5016 = INVALID_NUMERIC */

17      "0518-217 odmchange: Incorrect descriptor type in object class %1$s.\n\
\tMake sure object class is valid.\n"
$       /* 5017 = UNKNOWN_TYPE */
$ The %1$s refers to a file.

18       "0518-200 odmchange: Object class name is too long.\n\
\tSpecify an object class name less than %d characters.\n"
$       /* 5000 = CHANGE_NAMLEN */
$ The %d refers to a number.


$set 4          This is the message set for the odmdelete command.  The
$               messages range from 5000 to 5007
$ Do not translate odmdelete, %$d or %$s.

1       "0518-301 odmdelete: Criteria string too long.\n\
\tSpecify a criteria less than %d characters.\n"
$       /* 5001 = DEL_STRLEN */

2       "0518-302 usage: odmdelete -o object_class [-q criteria]\n\
\tDeletes objects from an object class.\n"
$       /* 5002 = DEL_USAGE */
$ Do not translate -o or -q.

3       "0518-303 odmdelete: No object class name was specified\n\
\tAn object class name must be specified with the odmdelete command.\n"
$       /* 5003 = DEL_NONAME */

4       "0518-304 odmdelete: Cannot initialize the database\n\
\tTry again later or contact the systems administrator.\n"
$       /* 5004 = DEL_INIT_ERR */

5       "0518-305 odmdelete: Cound not open object class %1$s\n\
\tCheck path name and permissions.\n"
$       /* 5005 = DEL_OPEN_ERR */
$ The %1$s refers to a file.

6       "0518-306 odmdelete: Could not delete objects.\n\
\tCheck criteria, path name and permissions.\n"
$       /* 5006 = DEL_FAILED */

7       "0518-307 odmdelete: %1$d objects deleted.\n"
$       /* 5007 = DEL_NUM_OBJS */
$ The %1$d refers to a number.

8       "0518-300 odmdelete: Object class name too long. \n\
\tSpecify an object class name less than %d characters.\n"
$ The %d refers to a number.
$       /* 5000 = DEL_NAMLEN */


$set 5
$  This set of error messages is for the 'odmadd' command.
$      The 'odmadd' command takes input from a file which contains
$      stanzas.  The stanzas describe an object (or record) which
$      is to be added to the object class.  The 'stanza line' in the
$      error message indicates the line in the file at which the current
$      stanza is located.  The '%$s' at the end of the message is for
$      the name of the input file.
$  Do not translate %$s, %$d, or odmadd.

1  "0518-401 odmadd: Cannot find class name in stanza, stanza line: %1$d  %2$s.\n\
\tCorrect stanza and try again.\n"
$  A valid object class name could not be found in the stanza.
$ ADD_MSG_0

2  "0518-402 odmadd:  Cannot open class:  %1$s, stanza line: %2$d  %3$s\n\
\tCheck path name and permissions.\n"
$  The indicated object class could not be opened.
$ ADD_MSG_1

3  "0518-403 odmadd: Could not allocate storage, stanza line: %1$d  %2$s.\n\
\tTry again later or contact the systems administrator.\n"
$  A sufficient amount of storage could not be allocated for the stanza.
$ ADD_MSG_2

4  "0518-404 odmadd: Could not find descriptor name, stanza line: %1$d  %2$s.\n\
\tCorrect stanza and try again.\n"
$  A line in the stanza does not contain a valid descriptor name.
$ ADD_MSG_3

5  "0518-405 odmadd: Descriptor %1$s does not exist in object class,\n\
\tstanza line:  %2$d  %3$s. Correct descriptor name and try again.\n"
$  The descriptor name is not valid for the object class.
$ The %1$s refers to a descriptor name.
$ ADD_MSG_4

6  "0518-406 odmadd: Incorrect value for descriptor, stanza line:  %1$d  %2$s\n\
\tChange value and try again.\n"
$  The value given for a descriptor is not formed correctly.
$ ADD_MSG_5

7  "0518-407 odmadd: Could not allocate storage for vchar, stanza line: %1$d  %2$s\n\
\tTry again later or contact the systems administrator.\n"
$  A sufficent amount of storage for a 'vchar' (do not translate 'vchar')
$  could not be allocated.
$ ADD_MSG_6

8  "0518-408 odmadd: Could not convert binary value, stanza line:  %1$d  %2$s\n\
\tBinary values should start with 0x and contain an even number of \n\
\thexadecimal digits.\n"
$  The value for a 'binary' (do not translate 'binary') descriptor was
$  not correctly formed.
$ ADD_MSG_7

9  "0518-409 odmadd: Incorrect long value, stanza line:  %1$d  %2$s\n\
\tLong values should contain only the digits 0-9.\n"
$  The value for a 'long' (do not translate 'long') descriptor was not
$  correctly formed.
$ ADD_MSG_8

10  "0518-410 odmadd: Incorrect double value, stanza line:  %1$d  %2$s\n\
\tDouble values should contain only the digits 0-9.\n"
$   The value for a 'double' (do not translate 'double') descriptor was
$   not correctly formed.
$ ADD_MSG_9

11  "0518-411 odmadd: Incorrect short value, stanza line:  %1$d  %2$s\n\
\tShort values should contain only the digits 0-9.\n"
$   The value for a 'short' (do not translate 'short') descriptor was
$   not correctly formed.
$ ADD_MSG_10

12  "0518-412 odmadd: Descriptor %1$s has an unknown type, stanza line: %2$d %3$s\n\
\tMake sure object class is valid.\n"
$   The type of a given descriptor could not be established.
$ The %1$s refers to a descriptor name.
$ ADD_MSG_11

13 "0518-413 odmadd: Could not add to class, ODM error number: %1$d,\n\
\tstanza line: %2$d  %3$s.\n\
\tCheck path name and permissions.\n"
$ The %1$d refers to an odm error number.  Do not translate ODM.
$  The object could not be added to the object class.
$ ADD_MSG_12

14  "0518-414 odmadd: Incorrect stanza, stanza line:  %1$d  %2$s.\n\
\tCorrect stanza and try again.\n"
$   A stanza was incorrectly formed.
$ ADD_MSG_13

15  "0518-415 usage: odmadd {File . . . |-}\n\
\tAdds objects to an object class.\n\n\
\tThe <File> should be in the form:\n\n\
\t\t<class name 1>:\n\
\t\t\t<descriptor name A> = <descriptor value>\n\
\t\t\t<descriptor name B> = <descriptor value>\n\
\t\t<class name 2>:\n\
\t\t\t<descriptor name C> = <descriptor value>\n\
\t\t\t<descriptor name D> = <descriptor value>\n\n\
\t\t . . .\n"
$ ADD_MSG_14

16  "0518-416 odmadd: Name of input file %1$s is too long.\n\
\tSpecify a name less than %2$d characters.\n"
$   The name of the input file was too long.  The %1$s refers to a
$ descriptor name, %2$d refers to a number.
$ ADD_MSG_15

17  "0518-417 odmadd: Cannot open file: %s.\n\
\tCheck path name and permissions.\n"
$   The inputfile could not be opened. The %s refers to a file name.
$ ADD_MSG_16


$set 6
$  These messages are for the 'odmget' command.

2  "0518-501 usage: odmget [-q criteria] {Classname . . . } \n\
\tRetrieves objects from an object class.\n"
$  Message 2 shows the correct syntax for the 'odmget' command.
$ Do not remove \t.
$ GET_MSG_1

3  "0518-503 odmget: Criteria is too long: %1$s\n\
\tSpecify a criteria less than %2$d characters.\n"
$  The selection criteria was too long.
$ the %1$s refers to the criteria, %2$d refers to a number.
$ GET_MSG_2

4  "0518-504 odmget:  No object class name specified\n\
\tAn object class name must be specified.\n"
$  The name of the object class was not specified.
$ GET_MSG_3

5  "0518-505 odmget: Class name too long: %1$s\n\
\tSpecify an object class name less than %2$d characters.\n"
$  The name of the object class was too long.
$ The %1$s refers to the class name, %2$d refers to a number.
$ GET_MSG_4

6  "0518-506 odmget: Cannot open object class %s\n\
\tCheck path name and permissions.\n"
$  The object class could not be opened.
$ GET_MSG_5

7 "0518-507 odmget: Could not retrieve object for %1$s, ODM error number %2$d\n"
$  The object could not be retrieved from the object class.
$ The %1$s refers to a object class name.
$ GET_MSG_6

8  "0518-508 odmget: Could not convert for binary descriptor %1$s\n\
\tin object class %2$s\n"
$  The value of a descriptor cannot be put into an ASCII form.
$ GET_MSG_7

9  "0518-509 odmget: Unknow descriptor type for %1$s in the object class %2$s\n\
\tMake sure the object class is valid and try again.\n"
$  The descriptor has an invalid type.
$ The %1$s refers to a descriptor name, %2$s refers to a file.
$ GET_MSG_8


$set 7
$ These messages are for the 'odmshow' command.
$ Do not translate odmshow, %$s or %$d.

1 "0518-601 usage: odmshow {Classname . . .}\n\
\tShows the description of an object class.\n"
$ This message gives the correct syntax for the 'odmshow' command.
$ SHOW_MSG_0

2  "0518-602 odmshow: Could not open object class %1$s, ODM error number %2$d.\n\
\tCheck path name and permissions.\n"
$ The object class could not be opened.
$ SHOW_MSG_1

3  "0518-603 odmshow: Unknown descriptor type for %1$s of %2$d.\n\
\tMake sure the object class is valid and try again.\n"
$ The descriptor type is unknown.
$ The %1$s refers to the descriptor name
$ SHOW_MSG_2

4           "\tdescriptors:    %1$d\n\
\tstructure size:     0x%2$x (%3$d) bytes\n"
$ This message gives information about the object class.  The %1$d refers
$ to the number of descriptors. Do not translate 0x%2$x.
$ Do not remove the \t.  This message is part of a larger message.
$ SHOW_MSG_3

5           "\tdata offset:    0x%x\n"
$ The 'data offset' indicates the offset into the structure for a given
$ descriptor. Do not translate 0x%x.
$ Do not remove the \t.  This message is part of a larger message.
$ SHOW_MSG_4

6        "\tCould not open object class %1$s, ODM error number %2$d\n"
$  The object class could not be opened.
$ Do not remove the \t.  This message is part of a larger message.
$ Do not translate ODM.
$ SHOW_MSG_5

7           "\tCould not read object class %1$s, ODM error number %2$d\n"
$  The object class could be opened, but not read.
$ Do not remove the \t.  This message is part of a larger message.
$ Do not translate ODM.
$ SHOW_MSG_6

8           "\tpopulation:     %1$d objects (%2$d active, %3$d deleted)\n"
$  The 'populate' indicates how many objects are in the object class.
$  The 'active' indicates how many of the objects have not been removed.
$  The 'deleted' indicates how many of the objects have been removed.
$ Do not remove the \t.  This message is part of a larger message.
$ SHOW_MSG_7

9           "\tCould not access linked object class\n"
$  The object class linked to, does not exist or cannot be found.
$ Do not remove the \t.  This message is part of a larger message.
$ SHOW_MSG_8
