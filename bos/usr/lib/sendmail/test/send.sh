# @(#)00	1.1  src/bos/usr/lib/sendmail/test/send.sh, cmdsend_tc, bos411, 9428A410j 5/8/91 16:41:22
#
# COMPONENT_NAME: TCPTEST send.sh
# 
# ORIGINS: 27 28
# 
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
# Licensed Materials - Property of IBM
# 
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# 
# 
#  Notes and assumptions:
#    -The lpptest.sh test script (w/supporting files) is required
#       to run the send.sh test script.
#    -Run this test(s) as the USER defined in the testenv file.
#       send_entry() contains set-up for any preconditions.
#    -All files (e.g., send , tcptest, etc.) must exist in
#       in current directory which must be HOME of USER.

send_entry()
{
	# USER variable in testenv must be the current user.
	id | grep "root"
	if test $? -ne 0
	then
		echo "USER variable not Super User: root !  Quitting..."
		exit 1
	fi

	# Current directory must be the HOME directory of the current user.
	# (tcptest will probably bomb first)
	pwd | grep "${HOME}"
	if test $? -ne 0
	then
		echo "Current directory not correct ... changing"
		echo "to ${HOME} directory..."
		cd ${HOME}
	fi

	# Make ask from ask.c and testlist from testlist.c
	make ask
	make testlist

	# Are we root or joe-blow?
	id | grep "^uid=0(root)"
	if test $? -ne 0
	then
		SH_PROMPT="$ "
	else
		SH_PROMPT="# "
	fi

	#@EXTRA SETUP
        PID=$$
        BAK1=/usr/spool/mail/${USER}
        BAK2=/usr/spool/mqueue
        if [ -f $BAK1.orig.fvt ]; then
	{
		echo "\n"
		echo "========================================\c"
		echo "======================================="
	 	echo "\007\007$BAK1.orig.fvt exists"
	 	echo "This script will copy $BAK1 over $BAK1.orig.fvt"
		echo "Making backup copy of $BAK1.orig.fvt to \c"
		echo "/lost+found/$BAK1."$PID
		mvdir $BAK1.orig.fvt /lost+found/$BAK1.$PID
		echo "========================================\c"
		echo "======================================="
		echo "\n"
        } | tee -a ${RESULT}
		rm  $BAK1.orig.fvt >/dev/null 2>&1
	fi
        if [ -d $BAK2.orig.fvt ]; then
        {
		echo "\n"
		echo "========================================\c"
		echo "======================================="
	 	echo "\007\007$BAK2.orig.fvt exists"
	 	echo "This script will copy $BAK2 over $BAK2.orig.fvt"
		echo "Making backup copy of $BAK2.orig.fvt to \c"
		echo "/lost+found/$BAK2."$PID
		mvdir $BAK2.orig.fvt /lost+found/$BAK2.$PID
		echo "========================================\c"
		echo "======================================="
		echo "\n"
        } | tee -a ${RESULT}
		rm -r $BAK2.orig.fvt >/dev/null 2>&1
	fi
        if [ -f $BAK1 -a ! -f $BAK1.orig.fvt ]; then
           mv $BAK1 $BAK1.orig.fvt
           TT1="$?"
 	   if [ $TT1 -ne 0 ]; then
 		echo "\007\007ERROR 001: Could not backup existing $BAK1\c"
		echo " in send_entry()"
                exit 1
           fi
        fi
        if [ -d $BAK2 -a ! -d $BAK2.orig.fvt ]; then
           mvdir $BAK2 $BAK2.orig.fvt
           TT1="$?"
 	   if [ $TT1 -ne 0 ]; then
 		echo "\007\007ERROR 001: Could not backup existing $BAK2\c"
		echo " in send_entry()"
                exit 1
           fi
        fi
        SENDLIB=/usr/lib
	for i in aliases aliases.DB aliases.DBl sendmail.cf sendmail.cf sendmail.cfDB
        do
		cp $SENDLIB/$i $SENDLIB/$i.orig.fvt  >/dev/null 2>&1	
	done
        if [ -f $HOME/mbox ]
        then
  		cp $HOME/mbox $HOME/mbox.orig.fvt
       		rm -f $HOME/mbox >/dev/null 2>&1	
	else
		touch $HOME mbox
	fi
#	SENDDAEMON=`ps -ef | grep "sendmail" | grep -v "grep" | grep "\-bd"`
#       SENDPID=`echo $SENDDAEMON | awk '{print $2}'`
#	grep $SENDPID /etc/sendmail.pid
#       if [ "$?" -ne 0 ]; then
#		echo "ERROR999: sendmail is not configured properly"
#		echo "Check the sendmail configuration. QUITTING!!!"
#		exit 1
#       fi
	echo "Set-up completed!\n\n"
        #@SETUP END
}
send_001()
{
	LPTEST=send${TX}001
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v  -oeq "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send001.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send001.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}001" $?
	rm -f $HOME/dead.letter /tmp/send001.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}


send_002()
{
	LPTEST=send${TX}002
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -oee "
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send002.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send002.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}002" $?
	rm -f $HOME/dead.letter  /tmp/send002.sendm >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}

send_003()
{
	LPTEST=send${TX}003
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -of "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF > /dev/null
From: foo@machinename

This a test of the sendmail command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	grep "From: foo@machinename" /usr/spool/mail/${USER} >/dev/null 
	lputil verify "send${TX}003" $?
	rm -f $HOME/dead.letter  /tmp/send003.sendm >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	echo; echo
}

send_004()
{
	LPTEST=send${TX}004
	rm -f /tmp/${LPTEST}
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v  -og9999 "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF > /tmp/send004.out
This a test of the sendmail command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	grep "DefGid 9999" /tmp/send004.out > /dev/null 
	lputil verify "send${TX}004" $?
	rm -f $HOME/dead.letter /tmp/send004.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm /tmp/send004.out
}

send_005()
{
	LPTEST=send${TX}005
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v  -oi "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF
This is a test of the sendmail command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send005.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send005.sendm >/dev/null 2>&1
       TT1="$?"
	[ $TT1 -ne 0 ]
	lputil manual "send${TX}005" $?
	rm -f $HOME/dead.letter  /tmp/send005.sendm >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}

send_006()
{
	LPTEST=send${TX}006
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	cp /etc/syslog.conf /tmp/syslog.bak
	sync
	echo "mail.info	/tmp/send006.log" >> /etc/syslog.conf
	sync
	refresh -s syslogd 
	touch /tmp/send006.log
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -oL22 "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	sync
	grep "sendmail" /tmp/send006.log > /dev/null
	lputil verify "send${TX}006" $?
	rm -f $HOME/dead.letter  /tmp/send006.sendm >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send006.log
	sync
	mv /tmp/syslog.bak /etc/syslog.conf
	refresh -s syslogd 
	echo; echo
}

send_007()
{
	LPTEST=send${TX}007
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v  -oMxvalue "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF 
This a test of the sendmail command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	grep "${USER} (value)" /usr/spool/mail/${USER} >/dev/null
	lputil verify "send${TX}007" $?
	rm -f $HOME/dead.letter /tmp/send007.sendm  >/dev/null 
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 
}


send_008()
{
	LPTEST=send${TX}008
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -oP"${USER}"@"${LHOST}" null " 
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	sleep 5
	grep "To: ${USER}@" /usr/spool/mail/${USER} >/dev/null 
	lputil verify "send${TX}008" $?
	rm -f $HOME/dead.letter /tmp/send008.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	echo; echo
}


send_009()
{
	LPTEST=send${TX}009
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v  -oQ/tmp/send009.boggus "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: $CMD
EOF
	TT1="$?"
	[ -d /tmp/send009.boggus ]
	TT2="$?"
        if [ $TT1 -ne 0  $TT2 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send009.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send009.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}009" $?
	sendmail -oQ/usr/spool/mqueue
	rm -fr /tmp/send009.boggus  >/dev/null 2>&1
	rm -f $HOME/dead.letter /tmp/send009.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}


send_010()
{
	LPTEST=send${TX}010
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	if [ -f /tmp/send.stat ]
	then
		rm -f /tmp/send.stat
	fi
        touch /tmp/send.stat
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -oS/tmp/send.stat "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF
	this test comes from running the command: $CMD
	EOF
	sync
	/usr/lib/mailstats -S/tmp/send.stat > /tmp/send010.out
	sync
	} | tee -a ${RESULTS}
	grep "1" /tmp/send010.out >/dev/null 
	lputil verify "send${TX}010" $?
	rm -f /tmp/send010.stat  >/dev/null 
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
        rm /tmp/send010.out
        rm /tmp/send.stat
}


send_011()
{
	LPTEST=send${TX}011
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	cp /usr/lib/aliases /usr/lib/aliases.bak
	echo "dumalinm: ${USER}@`hostname`,null@null" >> /usr/lib/aliases
	/usr/lib/sendmail -bi
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v -om "dumalinm
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF > /tmp/send011.out
This a test of the sendmail command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	grep "${USER}@`hostname`\.\.\." /tmp/send011.out 
	lputil verify "send${TX}011" $?
	rm -f $HOME/dead.letter  /tmp/send011.sendm >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm /tmp/send011.out
	mv /usr/lib/aliases.bak /usr/lib/aliases
	/usr/lib/sendmail -bi
}

send_012()
{
	LPTEST=send${TX}012
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -on -bi"
	cp /usr/lib/aliases /tmp/aliases.bak
	echo "new: (stuff" >> /usr/lib/aliases
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD > /tmp/send012.out
	sync
	} | tee -a ${RESULTS}
	grep "Unbalanced" /tmp/send012.out >/dev/null 2>&1
	lputil verify "send${TX}012" $?
	rm -f $HOME/dead.letter >/dev/null 2>&1
	rm -f /tmp/send012.out > /dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	mv /tmp/aliases.bak /usr/lib/aliases
	/usr/lib/sendmail -bi > /dev/null 2>&1
	echo; echo
}


send_013()
{
	LPTEST=send${TX}013
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -oo "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	grep "This a test of the sendmail command: $CMD" /usr/spool/mail/${USER} >/dev/null 
	lputil verify "send${TX}013" $?
	rm -f $HOME/dead.letter  /tmp/send013.sendm >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	echo; echo
}


send_014()
{
	LPTEST=send${TX}014
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -os "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
$CMD <<-EOF
This is a test of the command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	grep "This is a test of the command: $CMD" /usr/spool/mail/${USER} >/dev/null
	lputil verify "send${TX}014" $?
	rm -f /tmp/send014.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}


send_015()
{
	LPTEST=send${TX}015
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v  -ou9999 "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF > /tmp/send015.out
This a test of the sendmail command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	grep "DefUid 9999" /tmp/send015.out >/dev/null 
	lputil verify "send${TX}015" $?
	rm -f $HOME/dead.letter  /tmp/send015.sendm >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send015.out
}


send_016()
{
	LPTEST=send${TX}016
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -bm "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test of the sendmail command: $CMDSEND <flag> <address>"
	$CMD <<-EOF
This a test of the sendmail command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	grep "$CMD" /usr/spool/mail/${USER} >/dev/null 
	lputil verify "send${TX}016" $?
	rm -f $HOME/dead.letter  /tmp/send016.sendm >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	echo; echo
}

send_017()
{
	LPTEST=send${TX}017
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -ov "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF > /tmp/send017.out
This a test of the sendmail command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	grep "${USER}" /tmp/send017.out >/dev/null 
	lputil verify "send${TX}017" $?
	rm -f $HOME/dead.letter  /tmp/send017.sendm >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send017.out
}



send_018()
{
	LPTEST=send${TX}018
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v  -oY "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send018.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send018.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}018" $?
	rm -f $HOME/dead.letter  /tmp/send018.sendm >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}


send_019()
{
	LPTEST=send${TX}019
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v  -oI_ "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send019.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send019.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil manual "send${TX}019" $?
	rm -f $HOME/dead.letter /tmp/send019.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}


send_020()
{
	LPTEST=send${TX}020
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -or1 "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send020.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send020.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil manual "send${TX}020" $?
	rm -f $HOME/dead.letter /tmp/send020.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}



send_021()
{
	LPTEST=send${TX}021
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -oT1s -q "
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	/usr/lib/sendmail -odq ${USER} <<-EOF
This a test of the sendmail command: $CMD
EOF
	sync
	sleep 2
	$CMD
	sync
	sleep 2
	/usr/ucb/mailq > /tmp/send021.out
	sync
	} | tee -a ${RESULTS}
	grep "empty" /tmp/send021.out >/dev/null 
	lputil verify "send${TX}021" $?
	rm -f $HOME/dead.letter /tmp/send021.out  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}


send_022()
{
	LPTEST=send${TX}022
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v -q "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF > /tmp/send022.out
This a test of the sendmail command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	grep "Queue Directory" /tmp/send022.out >/dev/null 
	lputil verify "send${TX}022" $?
	rm -f $HOME/dead.letter /tmp/send022.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send022.out
}


send_023()
{
	LPTEST=send${TX}023
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	if [ -f /usr/lib/sendmail.cfDB ]
	then
		mv /usr/lib/sendmail.cfDB /tmp/cfDB.bak         
	fi
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: kill -1 cat /etc/sendmail.pid" 
	ps -ef | grep "sendmail" | grep "`cat /etc/sendmail.pid`" 
        if [ "$?" -ne 0 ]; then
           echo "/etc/sendmail.pid and the current sendmail pid are not equal"
	   echo "The sendmail is not set properly. Check the system "
           echo "This script is going to FAIL"
	fi
	kill -1 `cat /etc/sendmail.pid`
	sync
	sync
	} | tee -a ${RESULTS}
	test -f /usr/lib/sendmail.cfDB
	lputil verify "send${TX}023" $?
	rm -f $HOME/dead.letter /tmp/send023.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	if [ -f /tmp/cfDB.bak ]
	then
		mv /tmp/cfDB.bak /usr/lib/sendmail.cfDB
	fi
	echo; echo
}


send_024()
{
	LPTEST=send${TX}024
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v  -q88 "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	ps -ef | grep sendmail | tee -a /tmp/send024.out
	grep " -q88" /tmp/send024.out >/dev/null 
	lputil verify "send${TX}024" $?
	rm -f $HOME/dead.letter /tmp/send024.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	TEMPID=`cat /etc/sendmail.pid`
        FLAGON=0
	for i in `ps -ef | grep "/usr/lib/sendmail" | grep -v "grep" | grep "\-q88" | grep "root" | awk '{print $2}'`
        do 
          if expr $TEMPID : $i
          then
              FLAGON=1
          fi
          kill -9 $i 
        done
        if [ $FLAGON -eq 1 ]; then
          echo "`ps -ef | grep "sendmail" | grep -v "grep" | awk '{print $2}'`">/etc/sendmail.pid
	fi
	rm /tmp/send024.out
	echo; echo
}



send_025()
{
	LPTEST=send${TX}025
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v -t "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF > /tmp/send025.out
CC: null
This a test of the sendmail command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	grep "null\.\.\." /tmp/send025.out >/dev/null 
	lputil verify "send${TX}025" $?
	rm -f $HOME/dead.letter /tmp/send025.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send025.out
}


send_026()
{
	LPTEST=send${TX}026
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF > /tmp/send026.out
This a test of the sendmail command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	grep "${USER}" /tmp/send026.out > /dev/null 
	lputil verify "send${TX}026" $?
	rm -f $HOME/dead.letter /tmp/send026.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send026.out
}

send_027()
{
	LPTEST=send${TX}027
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/ucb/newaliases"
	CMD=$CMDSEND""
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	TEMP1=`istat /usr/lib/aliasesDBl | grep updated | awk '{print $6}'`
	$CMD 
	TT1="$?"
	TEMP2=`istat /usr/lib/aliasesDBl | grep updated | awk '{print $6}'`
        expr "$TEMP1" : "$TEMP2"
	TT2="$?"
        if [ $TT1 -ne 0 -o $TT2 -ne 1 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send027.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send027.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}027" $?
	rm -f $HOME/dead.letter /tmp/send027.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}


send_028()
{
	LPTEST=send${TX}028
	rm -f /tmp/${LPTEST}
        rm -rf /tmp/send028.mailq >/dev/null 2>&1
        mkdir /tmp/send028.mailq >/dev/null 2>&1
        cp  /usr/spool/mqueue/* /tmp/send028.mailq >/dev/null 2>&1
	rm -f /usr/spool/mqueue/*
	CMDSEND="/usr/ucb/mailq"
	CMD=$CMDSEND""
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	/usr/lib/sendmail -odq ${USER} <<-EOF
This mail insures that there will be mail in the queue
EOF
	sync
	$CMD > /tmp/send028.out
	sync
	} | tee -a ${RESULTS}
	grep "request" /tmp/send028.out >/dev/null 
	lputil verify "send${TX}028" $?
        cp  /tmp/send028.mailq/* /usr/spool/mqueue >/dev/null 2>&1
	rm -f /tmp/send028.sendm  >/dev/null 2>&1
        rm -rf /tmp/send028.mailq >/dev/null 2>&1
	rm -f /tmp/send028.out
        echo; echo
}


send_029()
{
	LPTEST=send${TX}029
	rm -f /tmp/${LPTEST}
        rm -rf /tmp/send029.mailq >/dev/null 2>&1
        mkdir /tmp/send029.mailq >/dev/null 2>&1
        mv  /usr/spool/mqueue/* /tmp/send029.mailq >/dev/null 2>&1
	rm -f /usr/spool/mqueue/* >/dev/null 2>&1
	CMDSEND="/usr/ucb/mailq"
	CMD=$CMDSEND" -v "
	sync
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test of the sendmail command: $CMD"
	/usr/lib/sendmail -odq ${USER} <<-EOF
This mail insures that there will be a message in the queue
EOF
	sync
	$CMD >/tmp/send029.out
	sync
	sync
	} | tee -a ${RESULTS}
	grep "Priority" /tmp/send029.out >/dev/null 
	lputil verify "send${TX}029" $?
        mv  /tmp/send029.mailq/* /usr/spool/mqueue >/dev/null 2>&1
	rm -f /tmp/send029.sendm  >/dev/null 2>&1
        rm -rf /tmp/send029.mailq >/dev/null 2>&1
	rm -f /tmp/send029.out
        echo; echo
}


send_030()
{
	LPTEST=send${TX}030
	rm -f /tmp/${LPTEST}
	CMDSEND="/usr/lib/edconfig"
	CMD=$CMDSEND" /usr/lib/sendmail.cf"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD >/tmp/send030.mail <<-EOF
   	`sleep 10`
	1
	EOF
	TT1="$?"
	`grep "Classes:" /tmp/send030.mail`
        sync
        TT2="$?"
        if [ $TT1 -ne 0 -o $TT2 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send030.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send030.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}030" $?
	rm -f /tmp/send030.mail  >/dev/null 2>&1
        echo; echo
}


send_031()
{
	LPTEST=send${TX}031
	rm -f /tmp/${LPTEST}
	CMDSEND="/usr/lib/mailstats"
	CMD=$CMDSEND""
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	/usr/lib/sendmail ${USER} <<-EOF
This is mail to insure that statistics exist.
EOF
	sync
	$CMD > /tmp/send031.out 
	sync
	} | tee -a ${RESULTS}
	grep "Sendmail statistics" /tmp/send031.out >/dev/null 
	lputil verify "send${TX}031" $?
	rm -f /tmp/send031.out
	echo; echo
}


send_032()
{
	LPTEST=send${TX}032
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
        cp /usr/lib/sendmail.st /usr/lib/sendmail.st.or >/dev/null 2>&1
        rm /usr/lib/sendmail.st >/dev/null 2>&1
        echo " " > /usr/lib/sendmail.st
	CMDSEND="/usr/lib/mailstats"
	CMD=$CMDSEND" -z"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	/usr/lib/sendmail ${USER} <<-EOF
This is mail to insure that statistics are present to begin with
EOF
	sync
	$CMD
	sync
	$CMDSEND > /tmp/send032.out
	sync
	} | tee -a ${RESULTS}
        grep "No statistics" /tmp/send032.out > /dev/null
	lputil verify "send${TX}032" $?
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send032.out  >/dev/null 
	echo; echo
}



send_033()
{
	LPTEST=send${TX}033
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	if [ -f /tmp/send033.stat ]
	then
		rm /tmp/send033.stat
	fi
	touch /tmp/send033.stat
	CMDSEND="/usr/lib/mailstats"
	CMD=$CMDSEND" -S /tmp/send033.stat"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD >  /tmp/send033.out
	sync
	} | tee -a ${RESULTS}
        grep "No statistics" /tmp/send033.out > /dev/null
	lputil verify "send${TX}033" $?
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send033.stat  >/dev/null 
	rm -f /tmp/send033.out  >/dev/null 
	echo; echo
}


send_034()
{
	LPTEST=send${TX}034
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	if [ -d /usr/spool/mqueue ]
	then
		mkdir /tmp/send034.q
		mv /usr/spool/mqueue/* /tmp/send034.q > /dev/null 2>&1
	fi
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -bp "
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test of the sendmail command: $CMDSEND <flag>"
	$CMD <<-EOF > /tmp/send034.out
This a test of the sendmail command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	grep "empty" /tmp/send034.out >/dev/null 
	lputil verify "send${TX}034" $?
	rm -f $HOME/dead.letter  /tmp/send034.sendm >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send034.out > /dev/null 2>&1
	if [ -d /tmp/send034.q ]
	then
		rm -r /usr/spool/mqueue > /dev/null 2>&1
		mkdir /usr/spool/mqueue 
		mv /tmp/send034.q/* /usr/spool/mqueue > /dev/null 2>&1
		rm -r /tmp/send034.q > /dev/null 2>&1 
	fi
	echo; echo
}


send_035()
{
	LPTEST=send${TX}035
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -$  "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test generating error message EX_USAGE: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 64 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send035.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send035.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}035" $?
	rm -f $HOME/dead.letter /tmp/send035.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}


send_036()
{
	LPTEST=send${TX}036
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v  boggus@`hostname`"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test generating error message EX_NOUSER: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 67 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send036.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send036.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}036" $?
	rm -f $HOME/dead.letter /tmp/send036.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}


send_037()
{
	LPTEST=send${TX}037
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v root@boggus "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test generating error message EX_NOHOST: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 68 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send037.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send037.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}037" $?
	rm -f $HOME/dead.letter /tmp/send037.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}


send_038()
{
	LPTEST=send${TX}038
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMD="su guest -c /usr/lib/sendmail -oL"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test generating error message EX_NOPERM: $CMD"
	su guest -c "echo '' | /usr/lib/sendmail -oL"
	TT1="$?"
        if [ $TT1 -ne 77 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send038.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send038.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}038" $?
	rm -f $HOME/dead.letter /tmp/send038.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}


send_039()
{
	LPTEST=send${TX}039
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v  -oA/usr/lib/boggus "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test generating error message EX_DB: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 79 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send039.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send039.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}039" $?
	rm -f $HOME/dead.letter /tmp/send039.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}

send_040()
{
	LPTEST=send${TX}040
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -ok"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
$CMD >  /tmp/send040.sendm <<-EOF
This is a test of the command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send040.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send040.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}040" $?
	rm -f /tmp/send040.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send040.sendm  >/dev/null 2>&1
}


send_041()
{
	LPTEST=send${TX}041
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -oKMB"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
$CMD >  /tmp/send041.sendm <<-EOF
This is a test of the command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send041.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send041.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}041" $?
	rm -f /tmp/send041.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send041.sendm  >/dev/null 2>&1
}


send_042()
{
	LPTEST=send${TX}042
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -oJ_ "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
$CMD <<-EOF
This is a test of the command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	grep "This is a test of the command: $CMD" /usr/spool/mail/${USER} >/dev/null 
	lputil verify "send${TX}042" $?
	rm -f /tmp/send042.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	echo; echo
}

send_043()
{
	LPTEST=send${TX}043
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	CMDSEND="/usr/lib/sendmail -d passwd"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: this command listed as not testable in test matrix"
	$CMD <<-EOF>/dev/null
		This a test of the sendmail command: $CMDSEND
		EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send043.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send043.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil notrun "send${TX}043" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send043.sendm  /tmp/mailfile.bak
        echo; echo
}


send_044()
{
	LPTEST=send${TX}044
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -ow "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
$CMD >  /tmp/send044.sendm <<-EOF
This is a test of the command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send044.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send044.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}044" $?
	rm -f /tmp/send044.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send044.sendm  >/dev/null 2>&1
}


send_045()
{
	LPTEST=send${TX}045
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v -rnull "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test of the sendmail command: $CMD"
$CMD <<-EOF > /tmp/send045.out
This is a test of the command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	grep "null\.\.\. setsender" /tmp/send045.out >/dev/null 2>&1
	lputil verify "send${TX}045" $?
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send045.out  >/dev/null 
	echo; echo
}


send_046()
{
	LPTEST=send${TX}046
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -s "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
$CMD <<-EOF
This is a test of the command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	grep "This is a test of the command: $CMD" /usr/spool/mail/${USER} >/dev/null
	lputil verify "send${TX}046" $?
	rm -f /tmp/send046.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}


send_047()
{
	LPTEST=send${TX}047
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -x "
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
$CMD >  /tmp/send047.sendm <<-EOF
This is a test of the command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send047.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send047.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}047" $?
	rm -f /tmp/send047.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send047.sendm  >/dev/null 2>&1
}



send_048()
{
	LPTEST=send${TX}048
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -oX0"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
$CMD >  /tmp/send048.sendm <<-EOF
This is a test of the command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send048.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send048.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}048" $?
	rm -f /tmp/send048.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send048.sendm  >/dev/null 2>&1
}

send_049()
{
	LPTEST=send${TX}049
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -ox0"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
$CMD >  /tmp/send049.sendm <<-EOF
This is a test of the command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send049.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send049.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}049" $?
	rm -f /tmp/send049.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send049.sendm  >/dev/null 2>&1
}

send_050()
{
	LPTEST=send${TX}050
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -oy0"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
$CMD >  /tmp/send050.sendm <<-EOF
This is a test of the command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send050.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send050.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}050" $?
	rm -f /tmp/send050.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send050.sendm  >/dev/null 2>&1
}

send_051()
{
	LPTEST=send${TX}051
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -oZ0"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
$CMD >  /tmp/send051.sendm <<-EOF
This is a test of the command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send051.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send051.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}051" $?
	rm -f /tmp/send051.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send051.sendm  >/dev/null 2>&1
}

send_052()
{
	LPTEST=send${TX}052
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -oz0"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
$CMD >  /tmp/send052.sendm <<-EOF
This is a test of the command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send052.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send052.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}052" $?
	rm -f /tmp/send052.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send052.sendm  >/dev/null 2>&1
}

send_053()
{
	LPTEST=send${TX}053
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -T1s -q "
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	/usr/lib/sendmail -odq ${USER} <<-EOF
This a test of the sendmail command: $CMD
EOF
	sync
	sleep 2
	$CMD
	sync
	sleep 2
	/usr/ucb/mailq > /tmp/send053.out
	sync
	} | tee -a ${RESULTS}
	grep "empty" /tmp/send053.out >/dev/null 
	lputil verify "send${TX}053" $?
	rm -f $HOME/dead.letter /tmp/send053.out  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}

send_054()
{
	LPTEST=send${TX}054
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	if [ -d /usr/spool/mqueue ]
	then
		mvdir /usr/spool/mqueue /usr/spool/mqueue.bak
		mkdir /usr/spool/mqueue
	fi
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -odq -oq88888 "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test of the sendmail command: $CMD"
$CMD <<-EOF
This is a test of the command: $CMD
EOF
	sync
	/usr/ucb/mailq -v > /tmp/send054.out
	sync
	} | tee -a ${RESULTS}
	grep "1 request" /tmp/send054.out >/dev/null 
	lputil verify "send${TX}054" $?
	rm -f /tmp/send054.out  >/dev/null 
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	if [ -d /usr/spool/mqueue.bak ]
	then
		rm -r /usr/spool/mqueue
		mvdir /usr/spool/mqueue.bak /usr/spool/mqueue
	fi
	echo; echo
}

send_055()
{
	LPTEST=send${TX}055
	rm -f /tmp/${LPTEST}
	mv /etc/services /tmp/services.bak
	sync
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" "${USER}
        ERR_RET_CODE="EX_OK"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test generating return code $ERR_RET_CODE: $CMD"
	$CMD <<-EOF 
This is a test of the sendmail command
EOF
	echo "$?" > /tmp/send055.out
	sync
	} | tee -a ${RESULTS}
	grep "0" /tmp/send055.out > /dev/null
	lputil verify "send${TX}055" $?
	mv /tmp/services.bak /etc/services
	rm -f $HOME/dead.letter >/dev/null 2>&1
	rm -f send055.out > /dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/send055.out
}

send_056()
{
	LPTEST=send${TX}056
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/bin/startsrc"
	CMD=$CMDSEND" -s sendmail -a -bd"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST test of the sendmail command: $CMD"
	stopsrc -s sendmail > /dev/null
	$CMD > /tmp/send056.out
	sync
	} | tee -a ${RESULTS}
	grep "started" /tmp/send056.out >/dev/null 
	lputil verify "send${TX}056" $?
	rm -f /tmp/send056.out >/dev/null 
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}

send_057()
{
	LPTEST=send${TX}057
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/bin/lssrc -l -s sendmail"
	CMD=$CMDSEND""
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST test of the sendmail command: $CMD"
	/usr/lib/sendmail ${USER} <<-EOF
This is a message to insure that statistics exist.
EOF
	sync
	$CMD > /tmp/send057.out
	sync
	} | tee -a ${RESULTS}
	grep "Sendmail statistics" /tmp/send057.out >/dev/null 
	lputil verify "send${TX}057" $?
	rm -f /tmp/send057.out  >/dev/null 
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}


send_058()
{
	LPTEST=send${TX}058
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/bin/stopsrc"
	CMD=$CMDSEND" -s sendmail"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST test of the sendmail command: $CMD"
	/bin/startsrc -s sendmail -a -bd > /dev/null
	sync
	$CMD > /tmp/send058.out
	sync
	} | tee -a ${RESULTS}
	grep "successfully" /tmp/send058.out >/dev/null 
	lputil verify "send${TX}058" $?
	/bin/startsrc -s sendmail -a -bd > /dev/null
	rm -f /tmp/send058.out  >/dev/null 
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	echo; echo
}

send_059()
{
	LPTEST=send${TX}059
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/bin/refresh -s sendmail"
	CMD=$CMDSEND""
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST test of the sendmail command: $CMD"
	$CMD > /tmp/send059.out
	sync
	} | tee -a ${RESULTS}
	grep "successfully" /tmp/send059.out >/dev/null 
	lputil verify "send${TX}059" $?
	rm -f /tmp/send059.out  >/dev/null 
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	echo; echo
}

send_060()
{
	LPTEST=send${TX}060
	rm -f /tmp/${LPTEST}
	sync
	if [ -f /usr/lib/sendmail.st ]
	then
		mv /usr/lib/sendmail.st /tmp/stat.bak
	fi
	sync
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/mailstats"
	CMD=$CMDSEND" "
        ERR_RET_CODE="EX_NOINPUT"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test generating return code $ERR_RET_CODE: $CMD"
	$CMD > /dev/null 2>&1
	echo "$?" > /tmp/send060.out
	sync
	} | tee -a ${RESULTS}
	grep "66" /tmp/send060.out > /dev/null
	lputil verify "send${TX}060" $?
	sync
	if [ -f /tmp/stat.bak ]
	then
		mv /tmp/stat.bak /usr/lib/sendmail.st
	fi
	rm -f $HOME/dead.letter >/dev/null 2>&1
	rm -f send060.out > /dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}


send_061()
{
	LPTEST=send${TX}061
	rm -f /tmp/${LPTEST}
	OLDHOST=`hostname`
	hostname test > /dev/null 2>&1
	refresh -s sendmail > /dev/null 2>&1
	sync
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v "${USER}@$OLDHOST
        ERR_RET_CODE="EX_UNAVAILABLE"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test generating error message $ERR_RET_CODE: $CMD"
	$CMD <<-EOF > /tmp/out1
This is a test of the sendmail command
EOF
	echo "$?" > /tmp/out2
	sync
	} | tee -a ${RESULTS}
	sync
	grep "69" /tmp/out2 > /dev/null
	TT1="$?"
	sync
	grep "Service unavailable" /tmp/out1 > /dev/null
	TT2="$?"
	[ $TT1 -eq 0 -a $TT2 -eq 0 ]
	lputil verify "send${TX}061" $?
	hostname $OLDHOST > /dev/null 2>&1
	sync
	refresh -s sendmail > /dev/null 2>&1 
	rm -f $HOME/dead.letter >/dev/null 2>&1
	rm -f /tmp/out1 > /dev/null 2>&1
	rm -f /tmp/out2 > /dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}


send_062()
{
	LPTEST=send${TX}062
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v "${USER}
        ERR_RET_CODE="EX_PROTOCAL"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
        echo "************* THIS SUB_COMMAND IS NOT TESTABLE *****************"
	#This error return code is listed as not testable in the test matrix 
	echo "$LPTEST FVT  test generating error message $ERR_RET_CODE: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: Return code $ERR_RET_CODE
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send062.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send062.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil notrun "send${TX}062" $?
	rm -f $HOME/dead.letter /tmp/send062.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	echo; echo
}


send_063()
{
	LPTEST=send${TX}063
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -ol "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test for flag: $CMD"
	echo "\nAt the suggestion of the client a skeleton for a test of this flag has been"
	echo "provided and has been coded as NOT-RUN.\n"
	$CMD <<-EOF > /tmp/send063.out
This a test of the sendmail command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	grep "Specify" /tmp/send063.out >/dev/null 2>&1
	lputil notrun "send${TX}063" $?
	rm -f $HOME/dead.letter /tmp/send063.out  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	echo; echo
}


send_064()
{
	LPTEST=send${TX}064
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/semutil"
	CMD=$CMDSEND" "
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test for flag: $CMD"
	$CMD > /tmp/send064.out 2>&1
	sync
	} | tee -a ${RESULTS}
	grep "usage" /tmp/send064.out >/dev/null 2>&1
	lputil verify "send${TX}064" $?
	rm -f $HOME/dead.letter /tmp/send064.out  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	echo; echo
}


send_065()
{
	LPTEST=send${TX}065
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	mkdir /usr/spool/mqueue > /dev/null 2>&1
	touch /usr/spool/mqueue/Lockfile > /dev/null 2>&1
	touch /tmp/lok 
	sync; sync; sync
	CMDSEND="/usr/lib/semutil"
	CMD=$CMDSEND" /tmp/lok"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test for flag: $CMD"
	$CMD > /tmp/send065.out 2>&1
	sync
	} | tee -a ${RESULTS}
	grep "key" /tmp/send065.out >/dev/null 2>&1
	lputil verify "send${TX}065" $?
	rm -f $HOME/dead.letter /tmp/send065.out  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/lok
	echo; echo
}


send_066()
{
	LPTEST=send${TX}066
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -s "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST test of the sendmail command: $CMD"
	$CMD <<-EOF
This is a test of the command: $CMD
EOF
	sync
	} | tee -a ${RESULTS}
	grep "This is a test of the sendmail command: $CMD" /usr/spool/mail/${USER} >/dev/null 
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}066" $?
	rm -f /tmp/send066.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	echo; echo
}


send_067()
{
	LPTEST=send${TX}067
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	mkdir /usr/spool/mqueue > /dev/null 2>&1
	touch /usr/spool/mqueue/Lockfile > /dev/null 2>&1
	sync; sync; sync
	CMDSEND="/usr/lib/semutil"
	CMD=$CMDSEND" 999"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test for flag: $CMD"
	$CMD > /tmp/send067.out 2>&1
	sync
	} | tee -a ${RESULTS}
	grep "key" /tmp/send067.out >/dev/null 2>&1
	lputil verify "send${TX}067" $?
	rm -f $HOME/dead.letter /tmp/send067.out  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	echo; echo
}



send_068()
{
	LPTEST=send${TX}068
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v "${USER}
        ERR_RET_CODE="EX_CANTCREAT"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
        echo "************* THIS SUB_COMMAND IS NOT TESTABLE *****************"
	#This error return code is difficult to generate in a script
	echo "\n$LPTEST FVT  test generating error message $ERR_RET_CODE: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: Return code $ERR_RET_CODE
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send068.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send068.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil notrun "send${TX}068" $?
	rm -f $HOME/dead.letter /tmp/send068.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}




send_069()
{
	LPTEST=send${TX}069
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" "${USER}
        ERR_RET_CODE="EX_CONFIG"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
        echo "************* THIS SUB_COMMAND IS NOT TESTABLE *****************"
	#This error return code is difficult to generate in a script
	echo "\n$LPTEST FVT  test generating error message $ERR_RET_CODE: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: Return code $ERR_RET_CODE
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send069.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send069.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil notrun "send${TX}069" $?
	rm -f $HOME/dead.letter /tmp/send069.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}




send_070()
{
	LPTEST=send${TX}070
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" "${USER}
        ERR_RET_CODE="EX_IOERR"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
        echo "************* THIS SUB_COMMAND IS NOT TESTABLE *****************"
	#This error return code is difficult to generate in a script
	echo "\n$LPTEST FVT  test generating error message $ERR_RET_CODE: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: Return code $ERR_RET_CODE
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send070.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send070.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil notrun "send${TX}070" $?
	rm -f $HOME/dead.letter /tmp/send070.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}



send_071()
{
	LPTEST=send${TX}071
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" "${USER}
        ERR_RET_CODE="EX_OSERR"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
        echo "************* THIS SUB_COMMAND IS NOT TESTABLE *****************"
	#This error return code is difficult to generate in a script
	echo "\n$LPTEST FVT  test generating error message $ERR_RET_CODE: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: Return code $ERR_RET_CODE
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send071.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send071.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil notrun "send${TX}071" $?
	rm -f $HOME/dead.letter /tmp/send071.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}





send_072()
{
	LPTEST=send${TX}072
	rm -f /tmp/${LPTEST}
	mv /etc/services /tmp/services.bak
	sync
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -v "${USER}@${RHOST}
        ERR_RET_CODE="EX_OSFILE"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test generating error message $ERR_RET_CODE: $CMD"
	$CMD <<-EOF > /tmp/send072.out1
This is a test of the sendmail command
EOF
	echo "$?" > /tmp/send072.out2
	sync
	} | tee -a ${RESULTS}
	grep "72" /tmp/send072.out2 > /dev/null
	TT1="$?"
	grep "System file missing" /tmp/send072.out1 > /dev/null
	TT2="$?"
	[ $TT1 -eq 0 -a $TT2 -eq 0 ]
	lputil verify "send${TX}072" $?
	mv /tmp/services.bak /etc/services
	rm -f $HOME/dead.letter >/dev/null 2>&1
	rm -f /tmp/send072.out1 > /dev/null 2>&1
	rm -f /tmp/send072.out2 > /dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}




send_073()
{
	LPTEST=send${TX}073
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	touch /tmp/mailq
	CMDSEND="/usr/lib/sendmail -odq -oQ/tmp/mailq"
	CMD=$CMDSEND" "${USER}
        ERR_RET_CODE="EX_SOFTWARE"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test generating error message $ERR_RET_CODE: $CMD"
	$CMD <<-EOF > /dev/null 2>&1
This a test of the sendmail command: Return code $ERR_RET_CODE
EOF
        echo "$?" > /tmp/send073.out
	sync
	} | tee -a ${RESULTS}
	grep "70" /tmp/send073.out > /dev/null 2>&1
	lputil verify "send${TX}073" $?
	rm -f $HOME/dead.letter >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	rm -f /tmp/mailq
	rm -f /tmp/send073.out
	echo; echo
}




send_074()
{
	LPTEST=send${TX}074
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" "${USER}
        ERR_RET_CODE="EX_TEMPFAIL"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
        echo "************* THIS SUB_COMMAND IS NOT TESTABLE *****************"
	#This error return code is listed as untestable in the test matrix. 
	echo "\n$LPTEST FVT  test generating error message $ERR_RET_CODE: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: Return code $ERR_RET_CODE
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send074.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send074.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil notrun "send${TX}074" $?
	rm -f $HOME/dead.letter /tmp/send074.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}


send_075()
{
	LPTEST=send${TX}075
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	mkdir /usr/spool/mqueue > /dev/null 2>&1
	touch /usr/spool/mqueue/Lockfile > /dev/null 2>&1
	sync; sync; sync
	CMDSEND="/usr/lib/semutil"
	CMD=$CMDSEND" remove"
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test for flag: $CMD"
	$CMD > /tmp/send075.out 2>&1
	sync
	} | tee -a ${RESULTS}
	grep "key" /tmp/send075.out >/dev/null 2>&1
	lputil verify "send${TX}075" $?
	rm -f $HOME/dead.letter /tmp/send075.out  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	echo; echo
}

send_076()
{
	LPTEST=send${TX}076
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	sync
	CMDSEND="/usr/lib/sendmail"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Receives formatted text and routes to users"
	$CMD <<-EOF 
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "This a test of the sendmail command: $CMDSEND" /usr/spool/mail/${USER} >/dev/null 2>&1
	lputil verify "send${TX}076" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/mailfile.bak
        echo; echo
}

send_077()
{
	LPTEST=send${TX}077
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	CMDSEND="/usr/lib/sendmail -ba"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Starts the sendmail in ARPANET mode"
	$CMD <<-EOF
		This a test of the sendmail command: $CMDSEND
		EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send077.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send077.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}077" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send077.sendm  /tmp/mailfile.bak
        echo; echo
}

send_078()
{
	LPTEST=send${TX}078
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	CMDSEND="/usr/lib/sendmail -bi" 
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Builds the alias database"
	$CMD <<-EOF > /tmp/send078.out
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "alias" /tmp/send078.out >/dev/null 
	lputil verify "send${TX}078" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send078.out  /tmp/mailfile.bak
        echo; echo
}

send_079()
{
	LPTEST=send${TX}079
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	CMDSEND="/usr/lib/sendmail -bm"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Delivers mail in the usual way (default)"
	$CMD <<-EOF >/dev/null
		This a test of the sendmail command: $CMDSEND
		EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send079.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send079.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}079" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send079.sendm  /tmp/mailfile.bak
        echo; echo
}

send_080()
{
	LPTEST=send${TX}080
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	CMDSEND="/usr/lib/sendmail -bn"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Builds the compiled vers. of NLS config. file"
	$CMD <<-EOF >/tmp/send080.sendm 
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "valid sendmail" /tmp/send080.sendm >/dev/null 2>&1
	lputil verify "send${TX}080" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send080.sendm  /tmp/mailfile.bak
        echo; echo
}

send_081()
{
	LPTEST=send${TX}081
	echo "Started `date` - send  TEST - $LPTEST"
	rm -f /tmp/${LPTEST}
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
        cp -r /usr/spool/mqueue /tmp/que.bak
        rm -rf /usr/spool/mqueue* 
	CMDSEND="/usr/lib/sendmail -bp"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Prints a listing of the mail queue"
	$CMD <<-EOF >/tmp/send081.sendm 
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "Mail queue is empty" /tmp/send081.sendm >/dev/null 2>&1
	lputil verify "send${TX}081" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
        cp -r /tmp/que.bak /usr/spool/mqueue
        rm -r /tmp/que.bak 
	rm -f /tmp/send081.sendm  /tmp/mailfile.bak 
        echo; echo
}

send_082()
{
	LPTEST=send${TX}082
	echo "Started `date` - send  TEST - $LPTEST"
	rm -f /tmp/${LPTEST}
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	CMDSEND="/usr/lib/sendmail -br"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Starts sendmail in address ISO-8859 test mode"
	$CMD <<-EOF >/tmp/send082.sendm 
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "8859 ADDRESS TEST" /tmp/send082.sendm >/dev/null 2>&1
	lputil verify "send${TX}082" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send082.sendm  /tmp/mailfile.bak
        echo; echo
}

send_083()
{
	LPTEST=send${TX}083
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	CMDSEND="/usr/lib/sendmail -v -bs" 
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Uses the simple mail transfer protocol (SMTP)"
	$CMD <<-EOF > /tmp/send083.out
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "Sendmail" /tmp/send083.out >/dev/null 
	lputil verify "send${TX}083" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send083.out  /tmp/mailfile.bak
        echo; echo
}

send_084()
{
	LPTEST=send${TX}084
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	CMDSEND="/usr/lib/sendmail -bt"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Starts the sendmail in address test mode"
	$CMD <<-EOF >/tmp/send084.out
From: null
EOF
	sync
	} | tee -a ${RESULTS}
	grep "ADDRESS TEST MODE" /tmp/send084.out >/dev/null 2>&1
	lputil verify "send${TX}084" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send084.out  /tmp/mailfile.bak
        echo; echo
}

send_085()
{
	LPTEST=send${TX}085
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	CMDSEND="/usr/lib/sendmail -bv"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Starts sendmail w/request to verify user"
	$CMD <<-EOF >/tmp/send085.out
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "deliverable" /tmp/send085.out >/dev/null 
	lputil verify "send${TX}085" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send085.out  /tmp/mailfile.bak
        echo; echo
}

send_086()
{
	LPTEST=send${TX}086
	echo "Started `date` - send  TEST - $LPTEST"
	rm -f /tmp/${LPTEST}
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	CMDSEND="/usr/lib/sendmail -bw"
	CMD="$CMDSEND"
	{ 
	echo "$LPTEST $CMDSEND: "
	$CMD <<-EOF >/tmp/send086.sendm 
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "no associated password" /tmp/send086.sendm >/dev/null 2>&1
	lputil verify "send${TX}086" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send086.sendm  /tmp/mailfile.bak
        echo; echo
}

send_087()
{
	LPTEST=send${TX}087
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	mv /usr/lib/sendmail.cf /usr/lib/sendmail.cf.bak
	sync
	CMDSEND="/usr/lib/sendmail -bz"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Builds compiled vers. of config file /usr/lib/sendmail.cf" 
	$CMD > /tmp/send087.out
	sync
	} | tee -a ${RESULTS}
	grep "configuration file" /tmp/send087.out >/dev/null 
	lputil verify "send${TX}087" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send087.out  /tmp/mailfile.bak
	mv /usr/lib/sendmail.cf.bak /usr/lib/sendmail.cf
	sync
	/usr/lib/sendmail -bz
        echo; echo
}

send_088()
{
	LPTEST=send${TX}088
	echo "Started `date` - send  TEST - $LPTEST"
	rm -f /tmp/${LPTEST}
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	CMDSEND="/usr/lib/sendmail -C"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Starts sendmail using alternate config. file"
	$CMD <<-EOF >/tmp/send088.sendm 
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "Configuration file" /tmp/send088.sendm >/dev/null 2>&1
	lputil verify "send${TX}088" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send088.sendm  /tmp/mailfile.bak
        echo; echo
}

send_089()
{
	LPTEST=send${TX}089
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	if [ -d /usr/spool/mqueue ]
	then
		mkdir /tmp/send089.q
		mv /usr/spool/mqueue/* /tmp/send089.q
	fi
	mkdir /usr/spool/mqueue > /dev/null 2>&1
	touch /usr/spool/mqueue/Lockfile
	CMDSEND="/usr/lib/sendmail -c"
	CMD=$CMDSEND" root@"${RHOST} 
	{ 
	echo "$LPTEST $CMDSEND: Msgs are que'd but not sent until costs are lower"
	$CMD <<-EOF 
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	/usr/ucb/mailq > /tmp/send089.out
	sync
	} | tee -a ${RESULTS}
	grep "1 request" /tmp/send089.out >/dev/null 2>&1
	lputil verify "send${TX}089" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send089.out  /tmp/mailfile.bak
	rm -r /usr/spool/mqueue/*
	if [ -d /tmp/send089.q ]
	then
		mv /tmp/send089.q/* /usr/spool/mqueue
		rm -r /tmp/send089.q
	fi
        echo; echo
}

send_090()
{
	LPTEST=send${TX}090
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	CMDSEND="/usr/lib/sendmail -d21.5"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Sets debugging value"
	$CMD <<-EOF>/dev/null
		This a test of the sendmail command: $CMDSEND
		EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send090.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send090.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}090" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send090.sendm  /tmp/mailfile.bak
        echo; echo
}

send_091()
{
	LPTEST=send${TX}091
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        mv /usr/spool/mail/$USER /tmp/mailfile.bak > /dev/null
	CMDSEND="/usr/lib/sendmail -em" 
	CMD="$CMDSEND"
	{ 
	echo "$LPTEST $CMDSEND: Mails error message to users mailbox"
	$CMD <<-EOF > /dev/null 
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "Recipient names must be specified" /usr/spool/mail/${USER} >/dev/null 
	lputil verify "send${TX}091" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/mailfile.bak
        echo; echo
}

send_092()
{
	LPTEST=send${TX}092
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        mv /usr/spool/mail/$USER /tmp/mailfile.bak > /dev/null 2>&1
	CMDSEND="/usr/lib/sendmail -ew"
	CMD="$CMDSEND unlikelyuser"
	{ 
	echo "$LPTEST $CMDSEND: writes error message to the terminal"
	$CMD <<-EOF 
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	sync
	grep "This a test of the sendmail command: $CMDSEND" /usr/spool/mail/${USER} >/dev/null 
	lputil verify "send${TX}092" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/mailfile.bak
        echo; echo
}

send_093()
{
	LPTEST=send${TX}093
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        mv /usr/spool/mail/$USER /tmp/mailfile.bak > /dev/null 2>&1
	CMDSEND="/usr/lib/sendmail -ep"
	CMD="$CMDSEND unlikelyuser"
	{ 
	echo "$LPTEST $CMDSEND: Sends error message to the terminal"
	$CMD <<-EOF > /tmp/send093.out
		This a test of the sendmail command: $CMDSEND
		EOF
	sync; sync
	} | tee -a ${RESULTS}
	grep "User unknown" /tmp/send093.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}093" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send093.sendm  /tmp/mailfile.bak
        echo; echo
}

send_094()
{
	LPTEST=send${TX}094
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -eq "${USER}
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "\n$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send094.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send094.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}094" $?
	rm -f $HOME/dead.letter /tmp/send094.sendm  >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}

send_095()
{
	LPTEST=send${TX}095
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
	CMDSEND="/usr/lib/sendmail"
	CMD=$CMDSEND" -oee "
	echo "Started `date` - send  TEST - $LPTEST"
	{ 
	echo "$LPTEST FVT  test of the sendmail command: $CMD"
	$CMD <<-EOF
This a test of the sendmail command: $CMD
EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send095.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send095.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}095" $?
	rm -f $HOME/dead.letter  /tmp/send095.sendm >/dev/null 2>&1
	rm -f $HOME/mbox /usr/spool/mail/${USER} >/dev/null 2>&1
}

send_096()
{
	LPTEST=send${TX}096
	echo "Started `date` - send  TEST - $LPTEST"
	rm -f /tmp/${LPTEST}
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	CMDSEND="/usr/lib/sendmail -Fname"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Sets the full name of the sender"
	$CMD <<-EOF 
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "(name)" /usr/spool/mail/${USER} >/dev/null
	lputil verify "send${TX}096" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f $HOME/dead.letter /tmp/mailfile.bak
        echo; echo
}

send_097()
{
	LPTEST=send${TX}097
	echo "Started `date` - send  TEST - $LPTEST"
	rm -f /tmp/${LPTEST}
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	CMDSEND="/usr/lib/sendmail -v -fname"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Sets the name of the sender of the mail"
	$CMD <<-EOF >/tmp/send097.out 
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "name\.\.\. setsender" /tmp/send097.out > /dev/null 
	lputil verify "send${TX}097" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f $HOME/dead.letter /tmp/send097.out  /tmp/mailfile.bak
        echo; echo
}

send_098()
{
	LPTEST=send${TX}098
	echo "Started `date` - send  TEST - $LPTEST"
	rm -f /tmp/${LPTEST}
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	CMDSEND="/usr/lib/sendmail -h"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Sets the hop count to value specified"
	$CMD <<-EOF >/tmp/send098.sendm 
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "no associated hop count" /tmp/send098.sendm >/dev/null 2>&1
	lputil verify "send${TX}098" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f $HOME/dead.letter /tmp/send098.sendm  /tmp/mailfile.bak
        echo; echo
}

send_099()
{
	LPTEST=send${TX}099
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	CMDSEND="/usr/lib/sendmail -i"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Delivers interactively (synchronously)"
	$CMD <<-EOF >/dev/null
		This a test of the sendmail command: $CMDSEND
		EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send099.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send099.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil manual "send${TX}099" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send099.sendm  /tmp/mailfile.bak
        echo; echo
}

send_100()
{
	LPTEST=send${TX}100
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        mv /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	CMDSEND="/usr/lib/sendmail -m"
	CMD="$CMDSEND unlikelyuser"
	{ 
	echo "$LPTEST $CMDSEND: Mails the error msg to user's mailbox"
	$CMD <<-EOF >/dev/null
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "Subject: Returned mail: User unknown" /usr/spool/mail/${USER} >/dev/null 
	lputil verify "send${TX}100" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/mailfile.bak
        echo; echo
}

send_101()
{
	LPTEST=send${TX}101
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        cp /usr/spool/mail/$USER /tmp/mailfile.bak > /dev/null 2>&1
	cp /usr/lib/aliases /usr/lib/aliases.bak
	echo "newalinm: ${USER}@`hostname`" >> /usr/lib/aliases
	/usr/lib/sendmail -bi > /dev/null
	CMDSEND="/usr/lib/sendmail -n"
	CMD="$CMDSEND newalinm"
	{ 
	echo "$LPTEST $CMDSEND: Prevents sendmail cmd fr. interpreting aliases"
	$CMD <<-EOF > /tmp/send101.out
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "unknown" /tmp/send101.out >/dev/null 
	lputil verify "send${TX}101" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send101.sendm  /tmp/mailfile.bak
	mv /usr/lib/aliases.bak /usr/lib/aliases
	/usr/lib/sendmail -bi > /dev/null
        echo; echo
}

send_102()
{
	LPTEST=send${TX}102
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        cp /usr/spool/mail/$USER /tmp/mailfile.bak
	echo "newalinm: ${USER}@`hostname`" > /tmp/aliases.test
	sync
	/usr/lib/sendmail -oA/tmp/aliases.test -bi > /dev/null
	CMDSEND="/usr/lib/sendmail -v -oA/tmp/aliases.test"
	CMD="$CMDSEND newalinm"
	{ 
	echo "$LPTEST $CMDSEND: Sets the option variable A"
	$CMD <<-EOF > /tmp/send102.out
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "newalinm\.\.\. aliased to" /tmp/send102.out >/dev/null 2>&1
	lputil verify "send${TX}102" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send102.out /tmp/mailfile.bak
	rm -f /tmp/aliases.test
	/usr/lib/sendmail -bi > /dev/null
        echo; echo
}

send_103()
{
	LPTEST=send${TX}103
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        mv /usr/spool/mail/$USER /tmp/mailfile.bak > /dev/null 2>&1
	CMDSEND="/usr/lib/sendmail -oBc"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Sets the option variable to Bc"
	$CMD <<-EOF >/dev/null
		This a test of the sendmail command: $CMDSEND
		EOF
	TT1="$?"
        if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" >/tmp/send103.sendm 2>&1`
        fi
	sync
	} | tee -a ${RESULTS}
	grep "FVTERROR${LPTEST}" /tmp/send103.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil manual "send${TX}103" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send103.sendm  /tmp/mailfile.bak
        echo; echo
}

send_104()
{
	LPTEST=send${TX}104
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        cp /usr/spool/mail/$USER /tmp/mailfile.bak 2>/dev/null
	if [ -d /usr/spool/mqueue ]
	then
		mkdir /tmp/send104.q
		mv /usr/spool/mqueue/* /tmp/send104.q
	fi
	mkdir /usr/spool/mqueue > /dev/null 2>&1
	touch /usr/spool/mqueue/Lockfile
	CMDSEND="/usr/lib/sendmail -oc"
	CMD=$CMDSEND" root@"${RHOST} 
	{ 
	echo "$LPTEST $CMDSEND: Msgs are que'd but not sent until costs are lower"
	$CMD <<-EOF 
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	/usr/ucb/mailq > /tmp/send104.out
	sync
	} | tee -a ${RESULTS}
	grep "1 request" /tmp/send104.out >/dev/null 2>&1
	lputil verify "send${TX}104" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send104.out  /tmp/mailfile.bak
	rm -r /usr/spool/mqueue/*
	if [ -d /tmp/queue ]
	then
		mv /tmp/send104.q/* /usr/spool/mqueue
		sync
		rm -r /tmp/send104.q
	fi
        echo; echo
}

send_105()
{
	LPTEST=send${TX}105
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        mv /usr/spool/mail/$USER /tmp/mailfile.bak > /dev/null 2>&1
	CMDSEND="/usr/lib/sendmail -odi"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Delivers mail interactively"
	$CMD <<-EOF 
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "This a test of the sendmail command: $CMDSEND" /usr/spool/mail/${USER} >/dev/null 
	lputil verify "send${TX}105" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/mailfile.bak
        echo; echo
}

send_106()
{
	LPTEST=send${TX}106
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        mv /usr/spool/mail/$USER /tmp/mailfile.bak > /dev/null 2>&1
	CMDSEND="/usr/lib/sendmail -odb"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Starts sendmail in the background as an SMTP mail router"
	$CMD <<-EOF >/dev/null
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "This a test of the sendmail command: $CMDSEND" /usr/spool/mail/${USER} >/dev/null 2>&1
	lputil verify "send${TX}106" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/mailfile.bak
        echo; echo
}

send_107()
{
	LPTEST=send${TX}107
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        cp /usr/spool/mail/$USER /tmp/mailfile.bak
	CMDSEND="/usr/lib/sendmail -v -odq"
	CMD="$CMDSEND ${USER}"
	{ 
	echo "$LPTEST $CMDSEND: Queues message only and delivers during queue run"
	$CMD <<-EOF > /tmp/send107.out
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "${USER}\.\.\. queued" /tmp/send107.out >/dev/null 
	lputil verify "send${TX}107" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send107.out  /tmp/mailfile.bak
        echo; echo
}

send_108()
{
	LPTEST=send${TX}108
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        cp /usr/spool/mail/$USER /tmp/mailfile.bak
	CMDSEND="/usr/lib/sendmail -oem" 
	CMD="$CMDSEND"
	{ 
	echo "$LPTEST $CMDSEND: Mails error message to users mailbox"
	$CMD <<-EOF > /dev/null 
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "Recipient names must be specified" /usr/spool/mail/${USER} >/dev/null 
	lputil verify "send${TX}108" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/mailfile.bak
        echo; echo
}

send_109()
{
	LPTEST=send${TX}109
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        mv /usr/spool/mail/$USER /tmp/mailfile.bak > /dev/null 2>&1
	CMDSEND="/usr/lib/sendmail -oew"
	CMD="$CMDSEND unlikelyuser"
	{ 
	echo "$LPTEST $CMDSEND: writes error message to the terminal"
	$CMD <<-EOF 
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	sync
	grep "This a test of the sendmail command: $CMDSEND" /usr/spool/mail/${USER} >/dev/null 
	lputil verify "send${TX}109" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/mailfile.bak
        echo; echo
}

send_110()
{
	LPTEST=send${TX}110
	rm -f /tmp/${LPTEST}
	echo "Started `date` - send  TEST - $LPTEST"
        mv /usr/spool/mail/$USER /tmp/mailfile.bak > /dev/null 2>&1
	CMDSEND="/usr/lib/sendmail -oep"
	CMD="$CMDSEND unlikelyuser"
	{ 
	echo "$LPTEST $CMDSEND: Sends error message to the terminal"
	$CMD <<-EOF > /tmp/send110.out
		This a test of the sendmail command: $CMDSEND
		EOF
	sync
	} | tee -a ${RESULTS}
	grep "User unknown" /tmp/send110.sendm >/dev/null 2>&1
        TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "send${TX}110" $?
        cat /tmp/mailfile.bak > /usr/spool/mail/$USER 2>/dev/null
	rm -f /tmp/send110.sendm  /tmp/mailfile.bak
        echo; echo
}

send_exit()
{
	if [ -d $HOME/Mail.orig.fvt ]
	then
		rm -rf /tmp/Mail.temp >/dev/null 2>&1
                mvdir $HOME/Mail /tmp/Mail.temp
		rm -r $HOME/Mail >/dev/null 2>&1
		mvdir $HOME/Mail.orig.fvt $HOME/Mail
		TT1="$?"
                if [ $TT1 -ne 0 ]; then
			echo "Did not succeed in restoring the original \c"
			echo "$HOME/Mail"
                        exit 1
		else
			rm -fr /tmp/Mail.temp
                fi
		cp $HOME/.mh_profile.orig.fvt  $HOME/.mh_profile
		TT1="$?"
                if [ $TT1 -ne 0 ]; then
			echo "Did not succeed in restoring the original \c"
			echo "$HOME/.mh_profile"
                        exit 1
		else
			rm -fr $HOME/.mh_profile.orig.fvt
                fi
	fi
}


