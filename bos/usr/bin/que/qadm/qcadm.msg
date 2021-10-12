$ @(#)74	1.22  src/bos/usr/bin/que/qadm/qcadm.msg, cmdque, bos411, 9428A410j 3/18/94 09:33:10
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ NOTE: All messages are run through a message routine that requires the
$       messages not to have linefeeds (\n) at the end of the message string.

$quote "
$set MS_QCADM
MSGASSG		"0781-176 Assignment statement found before queue name."
MSGDEVC		"0781-177 Device stanzas do not match \"device =\" clause."
MSGDEVD		"0781-178 Illegal \"device =\" clause in device stanza."
MSGEWRT		"0781-179 Failure writing to file."
MSGEXST		"0781-180 The queue, %s, already exists in qconfig file."
MSGNXST		"0781-181 The queue, %s, does not exist in qconfig file."
MSGLINE		"0781-182 Problem with line %1$d in %2$s:"
MSGNDEL		"0781-183 Cannot delete %s: Queue contains devices."
MSGNDEV		"0781-184 Devices clause for queue: %s contains no devices."
MSGNDVA		"0781-185 Illegal attempt to add/change device clause."
MSGNODV		"0781-186 Illegal attempt to change device clause."
MSGOUTS		"0781-187 Insufficient memory."
MSGQCON		"0781-188 Problem with %s:"
MSGQDND		"0781-189 Queue:device, %1$s:/%2$s: not found in qconfig file. Not deleted."
MSGQDNS		"0781-190 Queue:device, %1$s:/%2$s: not found in qconfig file. Not printed."
MSGQDXS		"0781-191 The queue:device, %1$s:%2$s, does not exist in qconfig file."
MSGQUND		"0781-192 Queue %s: not found in qconfig file. Not deleted."
MSGQUNP		"0781-193 Queue %s: not found in qconfig file. Not printed."
MSGTBAD		"0781-194 Syntax error in qconfig file."
MSGDXST         "0781-195 The device, %s, already exists in the qconfig file."
MSGUSE1		"[-c] -qName ..."
MSGUSE2		"[-c] -qQueue -dDevice"
MSGUSE3		"-D -q<queue> -a<clause1> [-a<clause2> ...]"
MSGUSE4		"-q<queue> -d<device> -a<clause1> [-a<clause2> ...]"
MSGUSE5		"-q<queue> ..."
MSGUSE6		"-q<queue> -d<device> ..."
MSGUSE7A	"-n<newname> -q<queue>"
MSGUSE7B	"-a<clause1> [-a<clause2> ...]"
MSGUSE8A	"-n<newname> -q<queue> -d<device>"
MSGUSE8B	"-a<clause1> [-a<clause2> ...]"
MSGUSEQ		"[-c]"
MSGUSED		"[-c] -q<queue>"
MSGOPERM	"0781-258 Permission denied to open %s."
MSGORD		"0781-259 Permission denied to open %s for reading."
MSGOWR		"0781-260 Permission denied to open %s for writing."
MSGOAPP		"0781-261 Permission denied to open %s for appending."
MSGORDWR	"0781-262 Permission denied to open %s for reading or writing."
MSGWOPN		"0781-263 Unable to fopen(%1$s, %2$s).  Errno = %3$d."
MSGNOPNR	"0781-268 Cannot open %s for writing." 
MSGNOPNW	"0781-269 Cannot open %s for reading."
MSGNLCKW	"0781-270 Cannot lock %s for writing."
MSGNLCKR	"0781-271 Cannot lock %s for reading."
MSGEXCQ		"0781-272 Failed to exec %s."
MSGPIPE		"0781-273 Digest pipe error."
MSGFORK		"0781-274 Cannot fork for %s."
MSGEXEC		"0781-275 Cannot exec digester %s."
MSGWPID		"0781-276 Redigest wait: wrong pid."
MSGDIGE		"0781-277 Error from digester %1$s, status = %2$d, rv = %3$d."
MSGULNK		"0781-278 Cannot unlink old %s file",
MSGINST		"0781-279 Cannot install new %s file",
MSGBADST	"0781-315 Stat system call failed.  Errno = %d."
MSGUSE7C	"-q<queue>"
MSGUSE8C	"-q<queue> -d<device>"
MSGNDIR         "0781-316 Failure to read queue directory."
MSGNEMP         "0781-317 Queue not empty cannot rename."
MSGPAL		"0781-318 The queue, %s, already used by another spooler." 
MSGATTR		"0781-319 Attribute value missing: \'%s\'."
