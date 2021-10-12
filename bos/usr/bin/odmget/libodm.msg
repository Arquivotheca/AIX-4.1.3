$ @(#)20	1.9.1.1   src/bos/usr/bin/odmget/libodm.msg, cmdodm, bos411, 9430C411a 7/12/94 18:52:50 
$
$
$  COMPONENT_NAME: LIBODM - Message Catalog for odm_err_msg()
$
$  FUNCTIONS: libodm.msg
$
$  ORIGIN: 27
$
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1989
$  All Rights Reserved
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$
$       This file contains the corresponding messages for the ODM-generated
$       odmerrno variable.  The "odmerrno" variable is returned to an
$       application when an error has occurred in the ODM subroutine called
$       by the application.
$
$       Messages do not automatically appear when the ODM encounters an
$       error condition.  The ODM simply returns an error value, odmerrno,
$       to the calling application.
$
$       A subroutine is provided, odm_err_msg(), which allows an application
$       to retrieve a message string if it so desires.  The application
$       will pass as input to the odmerror() subroutine the odmerrno, and
$       the address of a string pointer.  The odmerror() subroutine will
$       then retrieve the message corresponding to the odmerrno, and copy
$       the message string into the input address.


$set 1          This is the first set of messages for the ODM.
$               The error codes are in the range 5900 through 5929.
$               The message numbers are in the range 0 through 29.
$ Do not translate libodm

$quote "

1   "0519-001 libodm: Cannot allocate sufficent storage.\n\
\tTry again later or contact the system administrator.\n"
$   /* ODMI_MALLOC_ERR            5901 */

2  "0519-002 libodm: The CLASS_SYMBOL does not identify a valid object class.\n\
\tCheck parameters, path name and permissions.\n"
$   /* ODMI_MAGICNO_ERR           5902 */
$ Do not translate CLASS_SYMBOL

3  "0519-003 libodm: The specified object identifier did not refer to \n\
\ta valid object.  The object identifier must be an integer greater than\n\
\tzero.\n"
$   /* ODMI_NO_OBJECT             5903 */

4  "0519-004 libodm: The specified search criteria is incorrectly formed.\n\
\tMake sure the criteria contains only valid descriptor names and \n\
\tthe search values are correct.\n"
$   /* ODMI_BAD_CRIT              5904 */

5  "0519-005 libodm: An internal consistency problem occurred. \n\
\tMake sure the object class is valid or contact the system administrator.\n"
$   /* ODMI_INTERNAL_ERR          5905 */

6  "0519-006 libodm: Too many object classes have been accessed.\n\
\tAn application can only access less than 1024 object classes.\n"
$   /* ODMI_TOOMANYCLASSES        5906 */

7  "0519-007 libodm: The object class which is linked to could not be opened.\n\
\tMake sure the object class is accessable and try again.\n"
$   /* ODMI_LINK_NOT_FOUND        5907 */

8  "0519-008 libodm: The specified file is not a valid object class.\n\
\tCheck path name and permissions.\n"
$   /* ODMI_INVALID_CLASS         5908 */

9  "0519-009 libodm: The specified object class already exists.\n\
\tAn object class must not exist when it is created.\n"
$   /* ODMI_CLASS_EXISTS          5909 */

10  "0519-010 libodm: The specified object class does not exist.\n\
\tCheck path name and permissions.\n"
$   /* ODMI_CLASS_DNE             5910 */

11  "0519-011 libodm: The object class name specified does not match the\n\
\tobject class name in the file.  Check path name and permissions.\n"
$   /* ODMI_BAD_CLASSNAME         5911 */

12  "0519-012 libodm: Cannot remove the object class from the file system.\n\
\tCheck path name and permissions.\n"
$   /* ODMI_UNLINKCLASS_ERR       5912 */

13  "0519-013 libodm: Cannot remove the object class collection from \n\
\tthe file system.  Check path name and permissions.\n"
$   /* ODMI_UNLINKCLXN_ERR        5913 */

14  "0519-014 libodm: Either the specified collection is not a valid \n\
\tobject class collection or the collection does not contain \n\
\tconsistent data.  Check path name and permissions or contact the \n\
\tsystem administrator.\n"
$   /* ODMI_INVALID_CLXN          5914 */

15  "0519-015 libodm: The specified collection is not a valid object \n\
\tclass collection.  Check path name and permissions.\n"
$   /* ODMI_CLXNMAGICNO_ERR       5915 */

16  "0519-016 libodm: The collection name specified does not match the\n\
\tcollection name in the file.  Check path name and permissions.\n"
$   /* ODMI_BAD_CLXNNAME          5916 */

17  "0519-017 libodm: The object class cannot be opened because of the \n\
\tfile permissions. \n"
$   /* ODMI_CLASS_PERMS           5917 */

18  "0519-018 libodm: The timeout value was not valid.\n\
\tSpecify a timeout value as a positive integer.\n"
$   /* ODMI_BAD_TIMEOUT           5918 */

19  "0519-019 libodm: Cannot create or open the lock file.\n\
\tCheck path name and permissions.\n"
$   /* ODMI_BAD_TOKEN             5919 */

20  "0519-020 libodm: Cannot grant the lock. \n\
\tAnother process already has the lock. Try again later.\n"
$   /* ODMI_LOCK_BLOCKED          5920 */

21  "0519-021 libodm: Cannot retrieve or set the lock environment variable.\n\
\tRemove some environment variables and try again.\n"
$   /* ODMI_LOCK_ENV               5921 */

22  "0519-022 libodm: Cannot unlock the lock file.\n\
\tMake sure the file exists and try again.\n"
$   /* ODMI_UNLOCK                 5922 */

23  "0519-023 libodm: Cannot set a lock on the file.\n\
\tCheck path name and permissions.\n"
$  /* ODMI_BAD_LOCK                5923 */

24  "0519-024 libodm: The lock identifier does not refer to a valid lock.\n\
\tThe lock identifer must be the same as was passed back \n\
\tfrom the odm_lock() routine.\n"
$  /* ODMI_LOCK_ID                 5924 */
$ Do not translate odm_lock()

25  "0519-025 libodm: The parameters passed to the routine were not\n\
\tcorrect.  Make sure there are the correct number of parameters \n\
\tand that they are valid.\n"
$ /* ODMI_PARMS                   5925 */

26  "0519-026 libodm: Cannot open a pipe to a child process.\n\
\tMake sure the child process is executable and try again.\n"
$ /* ODMI_OPEN_PIPE               5926 */

27  "0519-027 libodm: Cannot read from the pipe of the child.\n\
\tMake sure the child process is executable and try again.\n"
$ /* ODMI_READ_PIPE               5927 */

28  "0519-028 libodm: Cannot fork the child process.\n\
\tMake sure the child process is executable and try again.\n"
$ /* ODMI_OPEN_PIPE               5928 */

29  "0519-029 libodm: The specified path does not exist on the filesystem.\n\
\tMake sure the path is accessable on the filesystem.\n"
$ /* ODMI_INVALID_PATH            5929 */

30 "0519-030 libodm: The specified object class is opened as read-only\n\
\tand cannot be modified.\n"
$ /* ODMI_READ_ONLY                 5930 */

31 "0519-000 libodm: Cannot open the object class.\n\
\tCheck path name and permissions.\n"
$   /* ODMI_OPEN_ERR              5900 */

32 "0519-031 libodm: The specified object class cannot be modified\n\
\tbecause the filesystem is full.\n"
$  /* ODMI_NO_SPACE                 5931 */

$set 2          This is the second set of messages for the ODM.
$               They are in the range 5800 throuth 5819.
$               The message numbers are in the range 0 through 19.

$quote "

1   "0519-101 libodm: Cannot add to the object class collection.\n\
\tCheck path name and permissions.\n"
$   /* VCHAR_ADD_ERR                   5801 */

2  "0519-102 libodm: The object class collection could not be found.\n\
\tMake sure the file exists and check path name and permissions.\n"
$   /* VCHAR_CLASS_DNE                 5802 */

3  "0519-103 libodm: The offset into the object class collection does not\n\
\tpoint to a valid character string.  \n\
\tMake sure the collection file is correct.\n"
$   /* VCHAR_BADSTRINGADDR             5803 */

4 "0519-104 libodm: The object class collection cannot be opened because\n\
\tof the file permissions. Fix permissions and try again.\n"
$   /* VCHAR_CLASS_PERMS                       5804 */

5   "0519-100 libodm: Cannot open the object class collection file.\n\
\tCheck path name and permissions.\n"
$   /* VCHAR_OPEN_ERR                  5800 */
