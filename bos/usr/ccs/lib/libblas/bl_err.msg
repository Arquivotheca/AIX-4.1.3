$ @(#)98	1.2  src/bos/usr/ccs/lib/libblas/bl_err.msg, libblas, bos411, 9428A410j 6/15/90 17:50:55
$
$ COMPONENT_NAME: (LIBBLAS) Basic Linear Algebra Subroutine Library
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$set MS_BLAS_ERROR
$ translators:  %s will be replaced by the name of
$ a routine in a floating point library.  %d will be replaced
$ with the number of the parameter with an illegal value.
FORT_FATAL1  "libblas: On entry to %s parameter number %d had an illegal value\n"
$
WARNING1  "libblas (%s): ulimit too low to create work area, performance degraded\n"
WARNING2  "libblas (%s): Not enough page space for work area, performance degraded \n"
WARNING4  "libblas (%s): Unaligned array, performance degraded \n"
$
$ translators:  the word stride should not be translated 
FAILURE1  "libblas (%s): Incorrect stride calculation\n"
$ translators:  a "scratch array" is a temporary work area in memory
FAILURE2  "libblas (%s): Unaligned scratch array, performance degraded.\n"
$
$ translators:  the words WARNING, ERROR, FAILURE and SEVERITY
$                should not be translated
WARNING_BAD "libblas (%s): Unexpected WARNING index %d \n"
ERROR_BAD   "libblas (%s): Unexpected ERROR index %d \n"
FAILURE_BAD "libblas (%s): Unexpected FAILURE index %d \n"
SEVERITY_BAD "libblas (%s): Unexpected SEVERITY index %d \n"
