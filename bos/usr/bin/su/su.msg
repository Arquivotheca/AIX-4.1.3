$ @(#)34	1.4  src/bos/usr/bin/su/su.msg, cmdsauth, bos411, 9428A410j 6/10/91 10:27:52
$ 
$ COMPONENT_NAME: (TCBAUTH) su.msg
$ 
$ FUNCTIONS: 
$ 
$ ORIGINS: 27
$ 
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. date 1, 1989 
$ All Rights Reserved
$ 

$quote "
$set MS_SU

M_USRNONEX "3004-500 User \"%s\" does not exist.\n"
M_NOSU "3004-501 Cannot su to \"%s\""
M_GETLOG "3004-502 Cannot get \"LOGNAME\" variable"
M_SETCRED "3004-503 Cannot set process credentials"
M_SETPRIV "3004-504 Cannot set process privilege"
M_SETPENV "3004-505 Cannot set process environment"
M_NOSPACE "3004-506 Cannot allocate space"
M_EXPIRED " : Account has expired.\n"
M_TOOLONG " : Name is too long.\n"
M_NOWAY " : Account is not accessible.\n"
M_NOPERM " : No permission.\n"
M_NOAUTH " : Authentication is denied.\n"
