$ @(#)69	1.6.1.4  src/bos/usr/bin/que/aix/qprt.msg, cmdque, bos411, 9428A410j 1/5/94 12:15:43
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
$set MS_QPRT
MSGUSE1 "[-cCnr] [-Bnn|-Bna|-Bng|-Ban|-Baa|-Bag|-Bgn|-Bga|-Bgg]"
MSGUSE2 "[-aPreviewOpt] [-ALevel] [-bBottomMargin] [-dInputType]"
MSGUSE3 "[-DUser] [-eEmpasizedOpt] [-EDblHigh] [-fFilter]"
MSGUSE4 "[-FFontFile] [-gBegin] [-GCoord] [-hHeader] [-HHostname]"
MSGUSE5 "[-iIndent] [-IFontID] [-jInit] [-JRestore] [-kColor]"
MSGUSE6 "[-K] [-lLength] [-LLineWrap] [-mMessage] [-QValue]"
MSGUSE7 "[-MMessageFile] [-NNumberCopies] [-OPaperHandl] [-PPrinter]"
MSGUSE8 "[-pPitch] [-qQuality] [-r] [-RPriority] [-sNameType] [-SSpeed]"
MSGUSE9 "[-tTopMargin] [-TTitle] [-uPaperSrc] [-UDirectional]"
MSGUSE10 "[-vLinesPerIn] [-VVertical] [-wPageWidth] [-WDblWide]"
MSGUSE11 "[-xLineFeed] [-XCodePage] [-yDblStrike] [-YDuplex]"
MSGUSE12 "[-zRotate] [-ZFormFeed] [-#h] [-#j] [-#v] File ..."
MSGUSE13 "Starts a print job."
MSGFOERR "%1$s: Error in opening the file '%2$s'\n"
$ do not translate "qprt" or the flags cCnrBhbDmMNoPRT
