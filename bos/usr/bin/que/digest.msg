$ @(#)81        1.19  src/bos/usr/bin/que/digest.msg, cmdque, bos411, 9428A410j 11/3/93 14:22:55
$
$ COMPONENT_NAME: (CMDQUE) spooling commands
$
$ FUNCTIONS:
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
$ NOTE: All messages are run through a message routine that requires the
$       messages not to have linefeeds (\n) at the end of the message string. 

$quote "
$set MS_DIGEST
MSGDFLD		"0781-001 illegal field name for device stanza."
MSGQFLD		"0781-002 illegal field name for queue stanza."
MSGFLNG		"0781-003 field value too long; limit is %d chars."
MSGFLRG		"0781-004 field value too large; maximum is %d."
MSGIVAL		"0781-005 illegal value `%s'."
MSGMISS		"0781-006 queue %1$s, device %2$d: missing backend field."
MSGULNK		"0781-007 cannot unlink %s."
MSGNOPN		"0781-008 cannot open %s for writing."
MSGEWRT		"0781-009 write error on file %s."
MSGRD		"0781-010 cannot open %s for reading."
MSGMST		"0781-011 missing device stanza."
MSGNODEV	"0781-012 No device line in queue stanza."
MSGRQ		"0781-013 Cannot have rq line without hostid line."
MSGHOST		"0781-014 Cannot have hostid line without rq line."
MSGCOLON	"0781-015 missing `:' after stanza name."
MSGBLANK	"0781-016 missing blank after field name."
MSGCNFG		"0781-017 error in config file %1$s, line %2$d."
$ /* AIX security enhancement		*/
$ /* added MSGGDGS MSGCHWN and MSGPERM	*/
MSGGDGS		"0781-018 Cannot get effective gid for digester."
MSGCHWN		"0781-019 Cannot chown %1$s.  Effective gid = %2$u."
MSGPERM		"0781-020 Cannot set permissions on %s."
MSGMEQ		"0781-280 missing `='."
MSGTC		"0781-281 devices line contains trailing comma."
MSGNARGS	"0781-282 Not enough args to digest."
$ $  Note to translators, MSGLREAD is a sanity message,
$ $  it should only be returned if there are major problems.
MSGLREAD	"0781-283 Bug in digest, lread returned trash."
MSGBADDEV	"0781-284 device `%s' wasn't specified on devices line for queue."
MSGBADNAME	"0781-285 unrecognized field name %s."
MSGNLCK		"0781-286 cannot lock %s for writing."
MSGNTRN		"0781-287 cannot truncate %s."
MSGSTAT		"0781-309 cannot obtain file status for %s."
MSGCHECK	"0781-311 queue name %s cannot be used more then once." 
