mail_entry()
{
	#set the mail and bellmail prompt for "ask"
	mail_prompt="?"
	bell_prompt="?"
	test_prompt=""
	ask_loc=/tmp/mailtest/ask

	# Current directory must be the HOME directory of the current user.
	# (tcptest will probably bomb first)
	pwd | grep "${HOME}"
	if test $? -ne 0
	then
		echo "Current directory not correct ... changing"
		echo "to ${HOME} directory..."
		cd ${HOME}
	fi

	if test -f /usr/spool/mail/${USER}
		then  mv /usr/spool/mail/${USER} $HOME/mailfile.bak
	fi

	if test -f $HOME/mbox
		then mv $HOME/mbox /$HOME/mbox.bak
	fi

	if test -f $HOME/.mailrc
		then mv $HOME/.mailrc $HOME/.mailrc.bak
	fi
	
	if test -f $HOME/.forward
		then mv $HOME.forward $HOME/.forward.bak
	fi
	mv /usr/share/lib/Mail.rc /usr/share/lib/Mail.rc.bak
	echo "set ask askcc dot save keep" > /usr/share/lib/Mail.rc
	echo "ignore Received Message-Id Resent-Message-Id Status Mail-From Return-Path Via" >> /usr/share/lib/Mail.rc
	
}

mail_001()
{
	LPTEST=mail${TX}001
 	echo "Started `date` - mail TEST Number   - $LPTEST"
   	rm -f /tmp/${LPTEST}
   	{ 
   	echo "$LPTEST test of mail -v. Should produce verbose status."
	mail -v ${RUSER}@${RHOST} <<-EOF > /tmp/mail_001.out
		This is a test of the mail -v flag.
	EOF
	sync
   	} | tee -a ${RESULTS}
	grep HELO /tmp/mail_001.out > /dev/null
	TT1="$?"
	grep MAIL /tmp/mail_001.out > /dev/null
	TT2="$?"
	grep RCPT /tmp/mail_001.out > /dev/null
	TT3="$?"
	grep DATA /tmp/mail_001.out > /dev/null
	TT4="$?"
	grep QUIT /tmp/mail_001.out > /dev/null
	TT5="$?"
   	grep "Sent" /tmp/mail_001.out >/dev/null
	TT6="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] && [ $TT6 = 0 ]  
   	lputil verify "mail${TX}001" $? 
   	rm -f /tmp/mail_001.out  /usr/spool/mail/${USER}
   	echo;echo
}

mail_002()
{
	LPTEST=mail${TX}002
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
    	{ 
   	echo "$LPTEST test of mail -i. Should ignore tty interrupts."
	sync
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail -i ${USER}
	${waitch}Subject: ${waitch}mail_002.test 
	${waitch}$test_prompt${waitch}
	${waitch}$test_prompt${waitch}
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync
	} | tee -a ${RESULTS} 
	sleep 5
	grep mail_002.test /usr/spool/mail/${USER} >/dev/null 2>&1
    	lputil verify "mail${TX}002" $?
	rm -f /usr/spool/mail/${USER} 
	echo;echo
}

mail_003()
{
	LPTEST=mail${TX}003
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo '$LPTEST visual test of mail -N. Check that no header was displayed.'
	sync
	mail ${USER} <<-EOF
		send some mail so we can test mail -N flag
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail -N
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync	
	} | tee -a ${RESULTS} | tee /tmp/mail003.out
	grep "Mail [5.2 UCB] [IBM AIX 3.2]  Type ? for help." /tmp/mail003.out > /dev/null 2>&1
	TT1="$?"
	DATE=`date | awk '{ print $1, $2, $3 }'`
	grep $DATE /tmp/mail003.out >/dev/null 2>&1
	TT2="$?"
	[ $TT1 -ne 0 ] && [ $TT2 -ne 0 ]
   	lputil verify "mail${TX}003" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail003.out
	echo;echo
}

mail_004()
{
	LPTEST=mail${TX}004
	echo "Started `date` - mail TEST Number   - $LPTEST"
	rm -f /tmp/${LPTEST}
	{ 
	echo "$LPTEST test of mail -d. Should produce debug info."
	mail -d  > /tmp/mail004.out
	sync
	} | tee -a ${RESULTS}
	grep "user = ${USER}, mailname = /var/spool/mail/${USER}" /tmp/mail004.out >/dev/null 2>&1
	TT1="$?"
	grep "deadletter = $HOME/dead.letter, mailrc = $HOME/.mailrc, mbox = $HOME/mbox" /tmp/mail004.out >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}004" $? 
	rm -f /tmp/mail004.*
	echo;echo
}

mail_005()
{
	LPTEST=mail${TX}005
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of mail -s"
	mail -s "mail005 subject" ${USER} <<-EOF
		Testing mail with the -s flag
	EOF
	sync
	} | tee -a ${RESULTS}
	sleep 3
	grep "Subject: mail005 subject" /usr/spool/mail/${USER} > /dev/null 2>&1
	lputil verify "mail${TX}005" $? 
	rm -f /usr/spool/mail/${USER}
   	echo;echo
}

mail_006()
{
	LPTEST=mail${TX}006
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox
	echo "Started `date` - mail TEST Number - $LPTEST"
	{
	echo "$LPTEST test of mail -f."

	## put some mail in mbox - the default ##
	
	echo "From root Fri Feb 14 10:03:36 1992
Received: by mickey.mouse.disney (AIX 3.2/UCB 5.64/4.03)
	  id AA13448; Fri, 14 Feb 1992 10:03:36 -0600
Date: Fri, 14 Feb 1992 10:03:36 -0600
From: minnie
To: mickey
Subject: valentines

Happy Valentines day!!" > /tmp/mbox
	sync

	## now test the mail -f default ##

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail -f /tmp/mbox
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync
	} | tee -a ${RESULTS} | tee /tmp/mail006.out
	grep "\"/tmp/mbox\" complete" /tmp/mail006.out > /dev/null 2>&1
	TT1="$?"
	grep "Happy Valentines day!!" /tmp/mail006.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}006" $?
	rm -f $HOME/mbox /tmp/mail006.out /tmp/mbox
	echo;echo
}
	
mail_007()
{
	LPTEST=mail${TX}007
	rm -f /tmp/${LPTEST}
	rm -f $HOME/mbox
	echo "Started `date` - mail TEST Number - $LPTEST"
	{
	echo "$LPTEST test of mail -f."

	## put some mail in mbox - the default ##
	
	echo "From root Fri Feb 14 10:03:36 1992
Received: by mickey.mouse.disney (AIX 3.2/UCB 5.64/4.03)
	  id AA13448; Fri, 14 Feb 1992 10:03:36 -0600
Date: Fri, 14 Feb 1992 10:03:36 -0600
From: minnie
To: mickey
Subject: valentines

Happy Valentines day!!" > $HOME/mbox 
	sync

	## now test the mail -f default ##

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail -f
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync
	} | tee -a ${RESULTS} | tee /tmp/mail007.out
	grep "$HOME/mbox" /tmp/mail007.out > /dev/null 2>&1
	TT1="$?"
	grep "Happy Valentines day!!" /tmp/mail007.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}007" $?
	rm -f $HOME/mbox /tmp/mail007.out
	echo;echo
}
	
mail_008()
{
	LPTEST=mail${TX}008
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of mail -f."

	##  put some mail in a file ##
 
	echo "From root Fri Feb 14 10:03:36 1992
Received: by mickey.mouse.disney (AIX 3.2/UCB 5.64/4.03)
	  id AA13448; Fri, 14 Feb 1992 10:03:36 -0600
Date: Fri, 14 Feb 1992 10:03:36 -0600
From: minnie 
To: mickey
Subject: where's goofy        

Is goofy on vacation??"  > /tmp/mail008.out
	sync
	
	##  now test mail -f flag ##
 
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail -f /tmp/mail008.out
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}delete
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	} | tee -a ${RESULTS} | tee /tmp/mail008.out1
	grep '"/tmp/mail008.out": 1 message' /tmp/mail008.out1 >/dev/null 2>&1
	TT1="$?"
	grep "Is goofy on vacation??" /tmp/mail008.out1 >/dev/null 2>&1
	TT2="$?"
	grep "Is goofy on vacation??" /tmp/mail008.out >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 2 ]
	lputil verify "mail${TX}008" $? 
	rm -f /tmp/mail008.*
	echo;echo
}

mail_009()
{
  id | grep root > /dev/null
  if test $? -ne 0 
  then
	LPTEST=mail${TX}009
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of mail -u. (test for non root user)"
	mail -u ${USER2} > /tmp/mail009.out 2>&1
	sync	 
	} | tee -a ${RESULTS}
	grep "No mail for ${USER2}" /tmp/mail009.out > /dev/null 2>&1
	lputil verify "mail${TX}009" $? 
	rm -f /tmp/mail009.out
	echo;echo
   else echo "mail_009() must be run by a non-root user"
   fi	
}

mail_010()
{
   id | grep root > /dev/null
   if test $? = 0
   then
	LPTEST=mail${TX}010
	rm -f /tmp/${LPTEST}
	rm -f /usr/spool/mail/${USER}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of mail -u. (for root user)"

	# put some mail in the user's mail box #
	mail ${USER2} <<-EOF
		testing the mail -u flag.
		EOF
	sync
	
	# now read the user's mailbox #
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail -u ${USER2}
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync
	} | tee -a ${RESULTS} | tee /tmp/mail010.out
	grep "/var/spool/mail/${USER2}" /tmp/mail010.out >/dev/null 2>&1
	TT1="$?"
	grep -e "testing the mail -u flag" /tmp/mail010.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}010" $? 
	rm -f /tmp/mail010.out
	echo;echo
   else echo " mail_010() must be run by root"
   fi
}

mail_011()
{
  id | grep root > /dev/null
  if test $? = 0 
  then	
	LPTEST=mail${TX}011
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of mail -u. (test for root user)"
	su guest -c "echo '' | mail -u ${USER2} > /tmp/mail011.out 2>&1" > /dev/null 2>&1
	sync
	} | tee -a ${RESULTS}
	grep "No mail for ${USER2}" /tmp/mail011.out > /dev/null 2>&1
	lputil verify "mail${TX}011" $? 
	rm -f /tmp/mail011.out
	echo;echo
   else echo "mail_011() must be run by root"
   fi	
}

mail_012()
{
	LPTEST=mail${TX}012
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	{
	echo "$LPTEST test of mail -n. Should not read Mail.rc file."
	# put some mail in the mailbox #
	mail ${USER} <<-EOF
		This will test mail -n flag.     
	EOF
	sync;sync
	
	# save Mail.rc #
	cp /usr/lib/Mail.rc /usr/lib/Mail.rc.bak
	echo "set keep"  >> /usr/lib/Mail.rc
	
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail -n
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}delete
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS}
	ls /usr/spool/mail/${USER} >/dev/null 2>&1
	TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "mail${TX}012" $?
	cp /usr/lib/Mail.rc.bak /usr/lib/Mail.rc
	rm -f /usr/lib/Mail.rc.bak
	echo;echo
}

mail_013()
{
	LPTEST=mail${TX}013
	rm -f /tmp/${LPTEST}
	rm -f /usr/spool/mqueue/*
	echo "Started `date` - mail TEST Number - $LPTEST"
	{
	echo "$LPTEST test of sending mail to a remote user"
	mail ${RUSER}@${RHOST} <<-EOF
		testing mail to a remote user
		EOF
	TT1="$?"
	if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" > /tmp/mail013.out 2>&1`
	fi
	sync
	} | tee -a ${RESULTS}
	grep "Function failed: FVTERROR${LPTEST}" /tmp/mail013.out > /dev/null 2>&1
	TT1="$?"
	sleep 3
	ls /usr/spool/mqueue/qf* >/dev/null 2>&1
	TT2="$?"
	[ $TT1 -ne 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}013" $?
	rm -f /tmp/mail013.out
	echo;echo 
}

mail_014()
{
	LPTEST=mail${TX}014
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	{
	echo "$LPTEST test of sending mail to users"
	mail ${USER},${USER2} <<-EOF
		testing mail to two local addresses 
		EOF
	TT1="$?"
	if [ $TT1 -ne 0 ]; then
		`echo "Function failed: FVTERROR${LPTEST}" > /tmp/mail014.out 2>&1`
	fi
	sync
	} | tee -a ${RESULTS}
	sleep 3
	grep "Function failed: FVTERROR${LPTEST}" /tmp/mail014.out > /dev/null 2>&1
	TT1="$?"
	grep "testing mail to two local addresses" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "testing mail to two local addresses" /usr/spool/mail/${USER2} > /dev/null 2>&1
	TT3="$?"
	[ $TT1 -ne 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}014" $?
	rm -f /usr/spool/mail/${USER} /usr/spool/mail/${USER2}
	rm -f /tmp/mail014.out
	echo;echo 
}

mail_015()
{
	LPTEST=mail${TX}015
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of the mail <CR>"
	## put some mail in the mailbox so we can enter mail mode ##
	mail ${USER} <<-EOF
		testing the mail <CR> test case
		EOF
	sleep 3
	sync
	${ask} <<- EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}${test_prompt}
	${waitch}$mail_prompt${waitch}${test_prompt}
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync
	} | tee -a ${RESULTS} | tee /tmp/mail015.out
	grep "testing the mail <CR> test case" /tmp/mail015.out > /dev/null 2>&1
	TT1="$?"
	grep "At EOF" /tmp/mail015.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}015" $? 
	rm -f /tmp/mail015.out
	echo;echo
}

mail_016()
{
	LPTEST=mail${TX}016
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of mail list"
	## send some mail so we can read it and enter mail mode ##
	mail ${USER} <<-EOF
		this is a test of the mail <list> subcommand
		EOF
	sync
	echo "list" | Mail > /tmp/mail016.out
	sync;sync;sync
	} | tee -a ${RESULTS}
	grep "Commands are" /tmp/mail016.out >/dev/null 2>&1
	TT1="$?"
	grep "next, alias, print, type, Type, Print, visual, top, touch, preserve," /tmp/mail016.out > /dev/null 2>&1
	TT2="$?"
	grep "delete, dp, dt, undelete, unset, mail, mbox, more, page, More, Page," /tmp/mail016.out > /dev/null 2>&1
	TT3="$?"
	grep "unread, Unread, new, New, !, copy, Copy, chdir, cd, save, Save, source,"	/tmp/mail016.out > /dev/null 2>&1
	TT4="$?"
	grep "set, shell, pipe, |, version, group, write, from, file, folder," /tmp/mail016.out >/dev/null 2>&1
	TT5="$?"
	grep "folders, ?, z, headers, help, =, Reply, Respond, followup, Followup," /tmp/mail016.out > /dev/null 2>&1
	TT6="$?"
	grep "reply, respond, edit, echo, quit, list, local, xit, exit, size, hold," /tmp/mail016.out > /dev/null 2>&1
	TT7="$?"
	grep "if, else, endif, alternates, ignore, discard, retain, unalias, #" /tmp/mail016.out > /dev/null 2>&1
	TT8="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] && [ $TT6 = 0 ] && [ $TT7 = 0 ] && [ $TT8 = 0 ]
	lputil verify "mail${TX}016" $?  
	echo;echo
	rm -f /tmp/mail016.out
}

mail_017()
{
	LPTEST=mail${TX}017
	echo "Started `date` - mail TEST Number   - $LPTEST"
	rm -f /tmp/${LPTEST}
	{ 
	echo "$LPTEST test of mail subcommand +."
	# send some mail first #
	mail ${USER} <<-EOF
		testing mail using the "+" subcommand
	EOF
	sync;sync
	echo "+q"|mail  > /tmp/mail_017.out 
	sync;sync;sync
	} | tee -a ${RESULTS}
	grep "Referencing beyond EOF" /tmp/mail_017.out >/dev/null
	lputil verify "mail${TX}017" $? 
	rm -f /usr/spool/mail/${USER}	/tmp/mail_017.out
	echo;echo
}

mail_018()
{
	LPTEST=mail${TX}018
	echo "Started `date` - mail TEST Number - $LPTEST"
	rm -f /tmp/${LPTEST}
	echo "$LPTEST test of the mail subcommand, +."
	# send two pieces of mail #
	mail ${USER} <<-EOF
		testing mail using the "+" subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		second piece of mail for the "+" subcommand test
	EOF
	sync;sync
	{
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}+
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail018.out
	grep 'second piece of mail for the "+" subcommand test' /tmp/mail018.out >/dev/null 2>&1
	TT1="$?"
	lputil verify "mail${TX}018" $?
	rm -f /tmp/mail018.out /usr/spool/mail/${USER}
	rm -f $HOME/mbox
	echo;echo
}	

mail_019()
{
	LPTEST=mail${TX}019
	echo "Started `date` - mail TEST Number   - $LPTEST"
	rm -f /tmp/${LPTEST}
	{ 
	echo "$LPTEST test of mail subcommand -."
	# send some mail first #
	mail ${USER} <<-EOF
		testing the mail subcommand "-"
	EOF
	sleep 3
	sync;sync
	echo "-q"|mail  > /tmp/mail_019.out 
	sync;sync;sync
	} | tee -a ${RESULTS}
	grep "Referencing before 1" /tmp/mail_019.out >/dev/null
	lputil verify "mail${TX}019" $? 
	rm -f /tmp/mail_019.out /usr/spool/mail/${USER}	
	echo;echo
}

mail_020()
{
	LPTEST=mail${TX}020
	echo "Started `date` - mail TEST Number - $LPTEST"
	rm -f /tmp/${LPTEST}
	{
	echo "$LPTEST test of the mail subcommand, -."
	# send two pieces of mail #
	mail ${USER} <<-EOF
		testing mail using the "-" subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		second piece of mail for the "-" subcommand test
	EOF
	sync;sync
	
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}preserve 2
	${waitch}$mail_prompt${waitch}-
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail020.out
	grep 'testing mail using the "-" subcommand' /tmp/mail020.out >/dev/null 2>&1
	TT1="$?"
	lputil verify "mail${TX}020" $?
	rm -f /tmp/mail020.out /usr/spool/mail/${USER}
	rm -f $HOME/mbox
	echo;echo
}	

mail_021()
{
	LPTEST=mail${TX}021
	echo "Started `date` - mail TEST Number   - $LPTEST"
	rm -f /tmp/${LPTEST}
	{ 
	echo "$LPTEST test of mail subcommand ?."
	# send some mail #
	mail ${USER} <<-EOF
		testing the mail subcommand "?"
	EOF
	sync;sync
	sleep 3
	echo "?
x
" | mail > /tmp/mail_021.out
	sync;sync
	} | tee -a ${RESULTS} 
	grep "Control Commands" /tmp/mail_021.out >/dev/null
	TT1="$?"
	grep "Display Commands:" /tmp/mail_021.out >/dev/null
	TT2="$?"
	grep "Message Handling:" /tmp/mail_021.out >/dev/null
	TT3="$?"
	grep "Creating New Mail:" /tmp/mail_021.out > /dev/null
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
	lputil verify "mail${TX}021" $? 
	rm -f /tmp/mail_021.out /usr/spool/mail/${USER}
	echo;echo
}

mail_022()
{
	LPTEST=mail${TX}022
	echo "Started `date` - mail TEST Number   - $LPTEST"
	rm -f /tmp/${LPTEST}
	{ 
	echo "$LPTEST test of mail subcommand !."
	# send some mail #
	mail ${USER} <<-EOF
		testing the "!" mail subcommand
	EOF
	sleep 3
	sync;sync;sync
	echo "!echo 'MAIL TEST'" | mail  > /tmp/mail_022.out 
	sync;sync
	} | tee -a ${RESULTS}
	grep "MAIL TEST" /tmp/mail_022.out >/dev/null
	lputil verify "mail${TX}022" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_022.out
	echo;echo
}

mail_023()
{
	LPTEST=mail${TX}023
	echo "Started `date` - mail TEST Number   - $LPTEST"
	rm -f /tmp/${LPTEST}
	{ 
	echo "$LPTEST test of mail subcommand =."
	# send some mail #
	mail ${USER} <<-EOF
		testing the mail subcommand, "="
	EOF
	sync;sync
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}=
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_023.out
	grep ^1 /tmp/mail_023.out >/dev/null
	lputil verify "mail${TX}023" $? 
	rm -f /usr/spool/mail/${USER} /tmp/mail_023.out
	echo;echo
}

mail_024()
{
	LPTEST=mail${TX}024
	echo "Started `date` - mail TEST Number   - $LPTEST"
	rm -f /tmp/${LPTEST}
	{ 
	echo "$LPTEST test of mail subcommand, #"
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "#" mail subcommand
	EOF
	sync;sync
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}# !hostname
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
   	} | tee -a ${RESULTS} | tee /tmp/mail_024.out
	grep ${LHOST} /tmp/mail_024.out >/dev/null 2>&1
	TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "mail${TX}024" $?
   	rm -f /tmp/mail_024.out /usr/spool/mail/${USER}
   	echo;echo
}

mail_025()
{
	LPTEST=mail${TX}025
	echo "Started `date` - mail TEST Number   - $LPTEST"
	rm -f /tmp/${LPTEST}
	{ 
	echo "$LPTEST test of mail subcommand, alias."
	# put alias in .mailrc first #
	echo "alias disney	snow@white" > $HOME/.mailrc
	echo "alias muppets	kermit@frog" >> $HOME/.mailrc 

	# send some mail #
	mail ${USER} <<-EOF
		testing the "alias" mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}alias
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_025.out
	grep "disney" /tmp/mail_025.out >/dev/null 2>&1
	TT1="$?"
	grep "snow@white" /tmp/mail_025.out > /dev/null 2>&1
	TT2="$?"
	grep "muppets" /tmp/mail_025.out >/dev/null 2>&1
	TT3="$?"
	grep "kermit@frog" /tmp/mail_025.out > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
	lputil verify "mail${TX}025" $?
	rm -f /tmp/mail_025.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	echo;echo
}

mail_026()
{
	LPTEST=mail${TX}026
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the alias mail subcommand"
	# put alias in .mailrc file first #
	echo "alias disney snow@white" > $HOME/.mailrc
	echo "alias muppets kermit@frog" >> $HOME/.mailrc 

	# send some mail #
	mail ${USER} <<-EOF
		testing the "alias" mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}alias disney
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_026.out
	grep "disney" /tmp/mail_026.out >/dev/null 2>&1
	TT1="$?"
	grep "snow@white" /tmp/mail_026.out > /dev/null 2>&1
	TT2="$?"
	grep "muppets" /tmp/mail_026.out >/dev/null 2>&1
	TT3="$?"
	grep "kermit@frog" /tmp/mail_026.out > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ] && [ $TT4 -ne 0 ]
	lputil verify "mail${TX}026" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_026.out 
	rm -f $HOME/.mailrc
	echo;echo
}

mail_027()
{
	LPTEST=mail${TX}027
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the alias mail subcommand"
	# put an alias in the .mailrc file #
	echo "alias disney snow@white" > $HOME/.mailrc
	echo "alias muppets kermit@frog" >> $HOME/.mailrc 

	# send some mail #
	mail ${USER} <<-EOF
		testing the "alias" mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}alias disney seven@dwarfs
	${waitch}$mail_prompt${waitch}alias seuss cat@hat
	${waitch}$mail_prompt${waitch}alias
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_027.out
	grep "^disney" /tmp/mail_027.out >/dev/null 2>&1
	TT1="$?"
	grep "seven@dwarfs snow@white" /tmp/mail_027.out > /dev/null 2>&1
	TT2="$?"	
	grep "^seuss" /tmp/mail_027.out >/dev/null 2>&1
	TT3="$?"
	TT4=`grep -c cat@hat /tmp/mail_027.out` 
	grep "^muppets" /tmp/mail_027.out >/dev/null 2>&1
	TT5="$?"
	grep kermit@frog /tmp/mail_027.out > /dev/null 2>&1
	TT6="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 2 ] && [ $TT5 = 0 ] && [ $TT6 = 0 ]	
	lputil verify "mail${TX}027" $?
	rm -f /tmp/mail_027.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	echo;echo
}

mail_028()
{
	LPTEST=mail${TX}028
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the alias mail subcommand"
	# send to an alias through .mailrc file #

	# put an alias in the .mailrc file #
	echo "alias test ${USER2}" > $HOME/.mailrc

	# send some mail to alias #
	mail -v test <<-EOF > /tmp/mail_028.out
		testing the "alias" mail subcommand
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS}
	grep 'testing the "alias" mail subcommand' /usr/spool/mail/${USER2} >/dev/null 2>&1
	TT1="$?"
	grep "${USER2}... Connecting to .local..." /tmp/mail_028.out >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}028" $?
	rm -f /usr/spool/mail/${USER2} /tmp/mail_028.out
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_029()
{
	LPTEST=mail${TX}029
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the alias mail subcommand"
	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the alias mail subcommand
	EOF
	sync;sync
	# issue an alias and send to it in mail mode #
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}alias test cinderella 
	${waitch}$mail_prompt${waitch}mail test
	${waitch}Subject: ${waitch}test
	${waitch}$test_prompt${waitch}hicinderella
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} 
	sleep 3
	grep "550 cinderella... User unknown" /usr/spool/mail/${USER} >/dev/null 2>&1
	lputil verify "mail${TX}029" $?
	rm -f /tmp/mail_029.out /usr/spool/mail/${USER}
	echo;echo
}

mail_030()
{
	LPTEST=mail${TX}030
	echo "Started `date` - mail TEST Number   - $LPTEST"
	rm -f /tmp/${LPTEST}
	{ 
	echo "$LPTEST test of mail subcommand, alternates."
	echo "alternates ${RUSER}@${RHOST}" > $HOME/.mailrc

	# put some mail in system mailbox as though it were resolved #
	# through an alias by another machine #

	echo "From root Wed Mar 18 02:44:00 1992
Received: from ${RHOST} by ${LHOST} (AIX 3.2/UCB 5.64/4.03)
          id AA09071; Wed, 18 Mar 1992 02:43:59 -0600
Received: by ${RHOST} (AIX 3.2/UCB 5.64/4.03)
          id AA39671; Wed, 18 Mar 92 14:44:39 -0600
Date: Wed, 18 Mar 92 14:44:39 -0600
From: root@${RHOST}
Message-Id: <9203182044.AA39671@${RHOST}>
To: ${RUSER}@${RHOST}
Subject: testing alternates
Status: R
	
testing the alternates mail subcommand
" > /usr/spool/mail/${USER}

	# get in mail mode, and issue "reply" subcommand #
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}reply
	${waitch}$test_prompt${waitch}testing
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_030.out
	grep "To: ${USER}@${RHOST}"  /tmp/mail_030.out >/dev/null 2>&1
	TT1="$?"
	grep "Subject: Re:  testing alternates" /tmp/mail_030.out >/dev/null 2>&1
	TT2="$?"
	grep "To: ${RUSER}@${RHOST} ${USER}@${RHOST}" /tmp/mail_030.out >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ] 
	lputil verify "mail${TX}030" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_030.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_031()
{
	LPTEST=mail${TX}031
	echo "Started `date` - mail TEST Number   - $LPTEST"
	rm -f /tmp/${LPTEST}
	{ 
	echo "$LPTEST test of mail subcommand, alternates."
	# the alternates command is used in $HOME/.mailrc file #
	echo "alternates snow@white" > $HOME/.mailrc
	
	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the alternates mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}alternates
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_031.out
	grep ^snow@white /tmp/mail_031.out >/dev/null 2>&1
	lputil verify "mail${TX}031" $?
	rm -f /tmp/mail_031.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	echo;echo
}

mail_032()
{
	LPTEST=mail${TX}032
	echo "Started `date` - mail TEST Number   - $LPTEST"
	rm -f /tmp/${LPTEST}
	{ 
	echo "$LPTEST test of mail subcommand, alternates."
	# issue alternates command while in mail mode #
	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the alternates mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}alternates seven@dwarfs
	${waitch}$mail_prompt${waitch}alternates
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_032.out
	grep ^seven@dwarfs /tmp/mail_032.out >/dev/null 2>&1
	lputil verify "mail${TX}032" $?
	rm -f /tmp/mail_032.out /usr/spool/mail/${USER}
	echo;echo
}

mail_033()
{
	LPTEST=mail${TX}033
	echo "Started `date` - mail TEST Number   - $LPTEST"
	rm -f /tmp/${LPTEST}
	{ 
	echo "$LPTEST test of mail subcommand, alternates."
	# test alternates subcommand in mail mode #
	# put some mail in system mailbox as though it were resolved #
	# through an alias by another machine #

	echo "From root Wed Mar 18 02:44:00 1992
Received: from disney.land by doctor.seuss (AIX 3.2/UCB 5.64/4.03)
          id AA09071; Wed, 18 Mar 1992 02:43:59 -0600
Received: by disney.land (AIX 3.2/UCB 5.64/4.03)
          id AA39671; Wed, 18 Mar 92 14:44:39 -0600
Date: Wed, 18 Mar 92 14:44:39 -0600
From: root@disney.land
Message-Id: <9203182044.AA39671@disney.land>
To: mickey@disney.land 
Subject: testing alternates
Status: R
	
testing the alternates mail subcommand
" > /usr/spool/mail/${USER}

	# get in mail mode, and issue "reply" subcommand #
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}alternates mickey@disney.land
	${waitch}$mail_prompt${waitch}reply
	${waitch}$test_prompt${waitch}testing
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_033.out
	grep "To: ${USER}@disney.land"  /tmp/mail_033.out >/dev/null 2>&1
	TT1="$?"
	grep "Subject: Re:  testing alternates" /tmp/mail_033.out >/dev/null 2>&1
	TT2="$?"
	grep "To: mickey@disney.land ${USER}@disney.land" /tmp/mail_033.out >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ] 
	lputil verify "mail${TX}033" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_033.out
	echo;echo
}

mail_034()
{
	LPTEST=mail${TX}034
	echo "Started `date` - mail TEST Number   - $LPTEST"
	rm -f /tmp/${LPTEST}
	{ 
	echo "$LPTEST test of mail subcommand, group."
	# put group in .mailrc first #
	echo "group disney snow@white" > $HOME/.mailrc
	echo "group muppets kermit@frog" >> $HOME/.mailrc 

	# send some mail #
	mail ${USER} <<-EOF
		testing the "group" mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}group
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_034.out
	grep "disney" /tmp/mail_034.out >/dev/null 2>&1
	TT1="$?"
	grep "snow@white" /tmp/mail_034.out > /dev/null 2>&1
	TT2="$?"
	grep "muppets" /tmp/mail_034.out >/dev/null 2>&1
	TT3="$?"
	grep "kermit@frog" /tmp/mail_034.out > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] 
	lputil verify "mail${TX}034" $?
	rm -f /tmp/mail_034.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	echo;echo
}

mail_035()
{
	LPTEST=mail${TX}035
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the group mail subcommand"
	# put group in .mailrc file first #
	echo "group disney snow@white" > $HOME/.mailrc
	echo "group muppets kermit@frog" >> $HOME/.mailrc 

	# send some mail #
	mail ${USER} <<-EOF
		testing the "group" mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}group disney
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_035.out
	grep "disney" /tmp/mail_035.out >/dev/null 2>&1
	TT1="$?"
	grep "snow@white" /tmp/mail_035.out > /dev/null 2>&1
	TT2="$?"
	grep "muppets" /tmp/mail_035.out >/dev/null 2>&1
	TT3="$?"
	grep "kermit@frog" /tmp/mail_035.out > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ] && [ $TT4 -ne 0 ]
	lputil verify "mail${TX}035" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_035.out 
	rm -f $HOME/.mailrc
	echo;echo
}

mail_036()
{
	LPTEST=mail${TX}036
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the group mail subcommand"
	# put a group in the .mailrc file #
	echo "group disney snow@white" > $HOME/.mailrc
	echo "group muppets kermit@frog" >> $HOME/.mailrc 

	# send some mail #
	mail ${USER} <<-EOF
		testing the "group" mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}group disney seven@dwarfs
	${waitch}$mail_prompt${waitch}group seuss cat@hat
	${waitch}$mail_prompt${waitch}group
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_036.out
	grep "^disney" /tmp/mail_036.out >/dev/null 2>&1
	TT1="$?"
	grep "snow@white seven@dwarfs" /tmp/mail_036.out >/dev/null 2>&1
	TT2="$?"
	grep "^seuss" /tmp/mail_036.out >/dev/null 2>&1
	TT3="$?"
	TT4=`grep -c "cat@hat" /tmp/mail_036.out > /dev/null 2>&1`
	grep "^muppets" /tmp/mail_036.out >/dev/null 2>&1
	TT5="$?"
	grep "kermit@frog" /tmp/mail_036.out > /dev/null 2>&1
	TT6="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 2 ] && [ $TT5 = 0 ] && [ $TT6 = 0 ]	
	lputil verify "mail${TX}036" $?
	rm -f /tmp/mail_036.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	echo;echo
}

mail_037()
{
	LPTEST=mail${TX}037
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the group mail subcommand"
	# send to a group through .mailrc file #

	# put a group in the .mailrc file #
	echo "group test ${USER2}" > $HOME/.mailrc

	# send some mail to group #
	mail -v test <<-EOF > /tmp/mail_037.out
		testing the "group" mail subcommand
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS}
	grep 'testing the "group" mail subcommand' /usr/spool/mail/${USER2} >/dev/null 2>&1
	TT1="$?"
	grep "${USER2}... Connecting to .local..." /tmp/mail_037.out >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}037" $?
	rm -f /usr/spool/mail/${USER2} /tmp/mail_037.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_038()
{
	LPTEST=mail${TX}038
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the group mail subcommand"
	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the group mail subcommand
	EOF
	sync;sync
	# issue a group and send to it in mail mode #
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}group test cinderella 
	${waitch}$mail_prompt${waitch}mail test
	${waitch}Subject: ${waitch}test
	${waitch}$test_prompt${waitch}hicinderella
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} 
	sleep 3
	grep "550 cinderella... User unknown" /usr/spool/mail/${USER} >/dev/null 2>&1
	lputil verify "mail${TX}038" $?
	rm -f /tmp/mail_038.out /usr/spool/mail/${USER}
	echo;echo
}

mail_039()
{
	LPTEST=mail${TX}039
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the chdir mail subcomand" 
	# send some mail to get in mail mode #
	mail ${USER} <<-EOF
		testing the "chdir" mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}chdir /usr/bin
	${waitch}$mail_prompt${waitch}!pwd
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_039.out
	grep ^/usr/bin /tmp/mail_039.out >/dev/null 2>&1
	lputil verify "mail${TX}039" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_039.out
	echo;echo
}

mail_040()
{
	LPTEST=mail${TX}040
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the chdir mail subcomand" 
	# send some mail to get in mail mode #
	mail ${USER} <<-EOF
		testing the "chdir" mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}chdir 
	${waitch}$mail_prompt${waitch}!pwd
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_040.out
	grep ^$HOME /tmp/mail_040.out >/dev/null 2>&1
	lputil verify "mail${TX}040" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_040.out
	echo;echo
}

mail_041()
{
	LPTEST=mail${TX}041
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the cd mail subcomand" 
	# send some mail to get in mail mode #
	mail ${USER} <<-EOF
		testing the "cd" mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}cd /usr/bin
	${waitch}$mail_prompt${waitch}!pwd
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_041.out
	grep ^/usr/bin /tmp/mail_041.out >/dev/null 2>&1
	lputil verify "mail${TX}041" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_041.out
	echo;echo
}

mail_042()
{
	LPTEST=mail${TX}042
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the cd mail subcomand" 
	# send some mail to get in mail mode #
	mail ${USER} <<-EOF
		testing the "cd" mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}cd 
	${waitch}$mail_prompt${waitch}!pwd
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_042.out
	grep ^$HOME /tmp/mail_042.out >/dev/null 2>&1
	lputil verify "mail${TX}042" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_042.out
	echo;echo
}

mail_043()
{
	LPTEST=mail${TX}043
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the copy mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "copy" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		testing the "copy" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}copy 1 2 /tmp/mail_043.out1
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync 
	} | tee -a ${RESULTS} | tee /tmp/mail_043.out
	grep '"/tmp/mail_043.out1" \[New file\] 16/626' /tmp/mail_043.out > /dev/null 2>&1
	TT1="$?"
	grep "Saved 2 messages in $HOME/mbox" /tmp/mail_043.out >/dev/null 2>&1
	TT2="$?"

	# nothing but the Status field should be different in mbox and #
	# mail_043.out1. If TT4 = 0, then our copy was successful #
	# and nothing was deleted since it was put in $HOME/mbox  #
	diff /tmp/mail_043.out1 $HOME/mbox | grep -v Status | grep "<" > /tmp/mail_043.out2
	TT3=`ls -l /tmp/mail_043.out2 | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] 
	lputil verify "mail${TX}043" $?
	rm -f /tmp/mail_043*  /usr/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_044()
{
	LPTEST=mail${TX}044
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the copy mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the mail subcommand, "copy".
	EOF
	sync;sync	
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}copy /tmp/mail_044.out1
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_044.out
	grep '"/tmp/mail_044.out1" \[New file\] 8/315' /tmp/mail_044.out > /dev/null 2>&1
	TT1="$?"
	grep "Saved 1 message in $HOME/mbox" /tmp/mail_044.out >/dev/null 2>&1
	TT2="$?"
	# nothing but the Status field should be different in mbox and #
	# mail_044.out1. If TT4 = 0, then our copy was successful #
	# and nothing was deleted since it was put in $HOME/mbox  #
	diff /tmp/mail_044.out1 $HOME/mbox | grep -v Status | grep "<" > /tmp/mail_044.out2
	TT3=`ls -l /tmp/mail_044.out2 | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}044" $?
	rm -f /tmp/mail_044*  /usr/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_045()
{
	LPTEST=mail${TX}045
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the copy mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the mail subcommand, "copy".
	EOF
	sync;sync	
	mail ${USER} <<-EOF
		testing the mail subcommand, "copy"
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}copy 1 /tmp/mail_045.out1
	${waitch}$mail_prompt${waitch}copy 2 /tmp/mail_045.out1
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_045.out
	grep '"/tmp/mail_045.out1" \[New file\] 8/315' /tmp/mail_045.out > /dev/null 2>&1
	TT1="$?"
	grep "Saved 2 messages in $HOME/mbox" /tmp/mail_045.out >/dev/null 2>&1
	TT2="$?"
	grep '"/tmp/mail_045.out1" \[Appended\] 8/314' /tmp/mail_045.out > /dev/null 2>&1 	
	TT3="$?"
	# nothing but the Status field should be different in mbox and #
	# mail_045.out1. If TT4 = 0, then our copy was successful #
	# and nothing was deleted since it was put in $HOME/mbox  #
	diff /tmp/mail_045.out1 $HOME/mbox | grep -v Status | grep "<" > /tmp/mail_045.out2
	TT4=`ls -l /tmp/mail_045.out2 | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] 
	lputil verify "mail${TX}045" $?
	rm -f /tmp/mail_045*  /usr/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_046()
{
	LPTEST=mail${TX}046
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the co (copy) mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the mail subcommand, "co" (copy)
	EOF
	sync;sync	
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}co /tmp/mail_046.out1
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_046.out
	grep '"/tmp/mail_046.out1" \[New file\] 8/319' /tmp/mail_046.out > /dev/null 2>&1
	TT1="$?"
	grep "Saved 1 message in $HOME/mbox" /tmp/mail_046.out >/dev/null 2>&1
	TT2="$?"
	# nothing but the Status field should be different in mbox and #
	# mail_046.out1. If TT3 = 0, then our copy was successful #
	# and nothing was deleted since it was put in $HOME/mbox  #
	diff /tmp/mail_046.out1 $HOME/mbox | grep -v Status | grep "<" > /tmp/mail_046.out2
	TT3=`ls -l /tmp/mail_046.out2 | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] 
	lputil verify "mail${TX}046" $?
	rm -f /tmp/mail_046*  /usr/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_047()
{
	LPTEST=mail${TX}047
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the c (copy) mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the mail subcommand, "c" (copy)
	EOF
	sync;sync	
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}c /tmp/mail_047.out1
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_047.out
	grep '"/tmp/mail_047.out1" \[New file\] 8/318' /tmp/mail_047.out > /dev/null 2>&1
	TT1="$?"
	grep "Saved 1 message in $HOME/mbox" /tmp/mail_047.out >/dev/null 2>&1
	TT2="$?"
	# nothing but the Status field should be different in mbox and #
	# mail_047.out1. If TT3 = 0, then our copy was successful #
	# and nothing was deleted since it was put in $HOME/mbox  #
	diff /tmp/mail_047.out1 $HOME/mbox | grep -v Status | grep "<" > /tmp/mail_047.out2
	TT3=`ls -l /tmp/mail_047.out2 | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}047" $?
	rm -f /tmp/mail_047*  /usr/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_048()
{
	LPTEST=mail${TX}048
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the delete mail subcommand" 
	# first ensure $HOME/mbox is clean #
	rm -f $HOME/mbox
	# now send some mail #
	mail ${USER} <<-EOF
		testing the "delete" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}delete
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS}
	test -f $HOME/mbox
	TT1="$?"
	grep 'testing the "delete" mail subcommand' /usr/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	[ $TT1 -ne 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}048" $?
	echo;echo
}

mail_049()
{
	LPTEST=mail${TX}049
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the delete mail subcommand" 
	# first ensure $HOME/mbox is clean #
	rm -f $HOME/mbox
	# now send some mail #
	mail ${USER} <<-EOF
		test1 for testing the "delete" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "delete" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test3 for testing the "delete" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}delete 1-2
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS}
	test -f $HOME/mbox
	TT1="$?"
	grep test1 /usr/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	grep test2 /usr/spool/mail/${USER} >/dev/null 2>&1
	TT3="$?"
	grep test3 /usr/spool/mail/${USER} >/dev/null 2>&1
	TT4="$?"
	[ $TT1 -ne 0 ] && [ $TT2 -ne 0 ] && [ $TT3 -ne 0 ] && [ $TT4 = 0 ]
	lputil verify "mail${TX}049" $?
	rm -f /usr/spool/mail/${USER}
	echo;echo
}

mail_050()
{
	LPTEST=mail${TX}050
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the d (delete) mail subcommand" 
	# first ensure $HOME/mbox is clean #
	rm -f $HOME/mbox
	# now send some mail #
	mail ${USER} <<-EOF
		testing the "d" (delete) mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}d
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS}
	test -f $HOME/mbox
	TT1="$?"
	grep 'testing the "d" (delete) mail subcommand' /usr/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	[ $TT1 -ne 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}050" $?
	rm -f /usr/spool/mail/${USER}
	echo;echo
}

mail_051()
{
	LPTEST=mail${TX}051
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the dp mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		test1 for testing the "dp" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "dp" mail subcommand	
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}dp
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_051.out
	grep "Saved 1 message in $HOME/mbox" /tmp/mail_051.out >/dev/null 2>&1
	TT1="$?"
	grep test2 $HOME/mbox >/dev/null 2>&1
	TT2="$?"
	grep ^test2 /tmp/mail_051.out >/dev/null 2>&1
	TT3="$?"
	grep '"dp"' /usr/spool/mail/${USER} >/dev/null 2>&1
	TT4="$?"	
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 -ne 0 ]
	lputil verify "mail${TX}051" $?
	rm -f $HOME/mbox /tmp/mail_051.out
	echo;echo
}

mail_052()
{
	LPTEST=mail${TX}052
	rm -f /tmp/${LPTEST} 
	rm -f $HOME/mbox
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the dp mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "dp" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}dp
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_052.out
	grep "No more messages" /tmp/mail_052.out >/dev/null 2>&1
	TT1="$?"
	grep '"dp"' /usr/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	test -f $HOME/mbox
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}052" $?
	rm -f $HOME/mbox /tmp/mail_052.out
	echo;echo
}

mail_053()
{
	LPTEST=mail${TX}053
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the dt mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		test1 for testing the "dt" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "dt" mail subcommand	
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}dt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_053.out
	grep "Saved 1 message in $HOME/mbox" /tmp/mail_053.out >/dev/null 2>&1
	TT1="$?"
	grep test2 $HOME/mbox >/dev/null 2>&1
	TT2="$?"
	grep ^test2 /tmp/mail_053.out >/dev/null 2>&1
	TT3="$?"
	grep '"dt"' /usr/spool/mail/${USER} >/dev/null 2>&1
	TT4="$?"	
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 -ne 0 ]
	lputil verify "mail${TX}053" $?
	rm -f $HOME/mbox /tmp/mail_053.out
	echo;echo
}

mail_054()
{
	LPTEST=mail${TX}054
	rm -f /tmp/${LPTEST} 
	rm -f $HOME/mbox
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the dt mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "dt" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}dt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_054.out
	grep "No more messages" /tmp/mail_054.out >/dev/null 2>&1
	TT1="$?"
	grep '"dt"' /usr/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	test -f $HOME/mbox
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}054" $?
	rm -f $HOME/mbox /tmp/mail_054.out
	echo;echo
}

mail_055()
{
	LPTEST=mail${TX}055
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the echo mail subcommand" 
	# send some mail so we can get in mail mode #
	mail  ${USER} <<-EOF
		testing the "echo" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}echo "testing the echo subcommand"
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_055.out
	grep "^testing the echo subcommand" /tmp/mail_055.out >/dev/null 2>&1
	lputil verify "mail${TX}055" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_055.out
	echo;echo
}

mail_056()
{
	LPTEST=mail${TX}056
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the exit mail subcommand" 
	# first send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "exit" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}delete
	${waitch}$mail_prompt${waitch}exit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} 
	grep 'testing the "exit" mail subcommand' /usr/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	lputil verify "mail${TX}056" $?
	rm -f /usr/spool/mail/${USER}
	echo;echo
}

mail_057()
{
	LPTEST=mail${TX}057
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the ex (exit) mail subcommand" 
	# first send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "ex" (exit) mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}delete
	${waitch}$mail_prompt${waitch}ex
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} 
	grep 'testing the "ex" (exit) mail subcommand' /usr/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	lputil verify "mail${TX}057" $?
	rm -f /usr/spool/mail/${USER}
	echo;echo
}

mail_058()
{
	LPTEST=mail${TX}058
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the x (exit) mail subcommand" 
	# first send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "x" (exit) mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}delete
	${waitch}$mail_prompt${waitch}x
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} 
	grep 'testing the "x" (exit) mail subcommand' /usr/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	lputil verify "mail${TX}058" $?
	rm -f /usr/spool/mail/${USER}
	echo;echo
}

mail_059()
{
	LPTEST=mail${TX}059
	rm -f /tmp/${LPTEST} 
	EOF="(EOF):"
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the more mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "more" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}more
	${waitch}$EOF${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_059.out
	grep "^(EOF):"	/tmp/mail_059.out >/dev/null 2>&1
	TT1="$?"
	grep '^testing the "more" mail subcommand' /tmp/mail_059.out > /dev/null 2<&1 
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}059"  $?
	rm -f /tmp/mail_059.out /usr/spool/mail/${USER}
	echo;echo
}

mail_060()
{
	LPTEST=mail${TX}060
	rm -f /tmp/${LPTEST} 
	EOF="(EOF):"
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the more mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		test1 for testing the "more" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "more" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}more 1-2
	${waitch}$EOF${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_060.out
	grep "^(EOF):"	/tmp/mail_060.out >/dev/null 2>&1
	TT1="$?"
	grep "^test1" /tmp/mail_060.out >/dev/null 2>&1
	TT2="$?"
	grep "^test2" /tmp/mail_060.out >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}060"  $?
	rm -f /tmp/mail_060.out /usr/spool/mail/${USER}
	echo;echo
}

mail_061()
{
	LPTEST=mail${TX}061
	rm -f /tmp/${LPTEST} 
	EOF="(EOF):"
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the mo (more) mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "mo" (more) mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}mo
	${waitch}$EOF${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_061.out
	grep "^(EOF):"	/tmp/mail_061.out >/dev/null 2>&1
	TT1="$?"
	grep '^testing the "mo" (more) mail subcommand' /tmp/mail_061.out > /dev/null 2<&1 
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}061"  $?
	rm -f /tmp/mail_061.out /usr/spool/mail/${USER}
	echo;echo
}

mail_062()
{
	LPTEST=mail${TX}062
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the More mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		test1 for testing the "More" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "More" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}ignore Status
	${waitch}$mail_prompt${waitch}More 1-2
	${waitch}:${waitch}$test_prompt
	${waitch}$EOF${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_062.out
	grep "^(EOF):"	/tmp/mail_062.out >/dev/null 2>&1
	TT1="$?"
	grep "test1" /tmp/mail_062.out >/dev/null 2>&1
	TT2="$?"
	grep "test2" /tmp/mail_062.out >/dev/null 2>&1
	TT3="$?"
	grep Status /tmp/mail_062.out >/dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
	lputil verify "mail${TX}062"  $?
	rm -f /tmp/mail_062.out /usr/spool/mail/${USER}
	echo;echo
}
	
mail_063()
{
	LPTEST=mail${TX}063
	rm -f /tmp/${LPTEST} 
	EOF="(EOF):"
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the More mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "More" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}More
	${waitch}$EOF${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_063.out
	grep "^(EOF):"	/tmp/mail_063.out >/dev/null 2>&1
	TT1="$?"
	grep '^testing the "More" mail subcommand' /tmp/mail_063.out > /dev/null 2<&1 
	TT2="$?"
	grep Status /tmp/mail_063.out >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}063"  $?
	rm -f /tmp/mail_063.out /usr/spool/mail/${USER}
	echo;echo
}

mail_064()
{
	LPTEST=mail${TX}064
	rm -f /tmp/${LPTEST} 
	EOF="(EOF):"
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the Mo (More) mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "Mo" (More) mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}Mo
	${waitch}$EOF${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_064.out
	grep "^(EOF):"	/tmp/mail_064.out >/dev/null 2>&1
	TT1="$?"
	grep '^testing the "Mo" (More) mail subcommand' /tmp/mail_064.out > /dev/null 2<&1 
	TT2="$?"
	grep Status /tmp/mail_064.out >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}064"  $?
	rm -f /tmp/mail_064.out /usr/spool/mail/${USER}
	echo;echo
}

mail_065()
{
	LPTEST=mail${TX}065
	rm -f /tmp/${LPTEST} 
	EOF="(EOF):"
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the page mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "page" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}page
	${waitch}$EOF${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_065.out
	grep "^(EOF):"	/tmp/mail_065.out >/dev/null 2>&1
	TT1="$?"
	grep '^testing the "page" mail subcommand' /tmp/mail_065.out > /dev/null 2<&1 
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}065"  $?
	rm -f /tmp/mail_065.out /usr/spool/mail/${USER}
	echo;echo
}

mail_066()
{
	LPTEST=mail${TX}066
	rm -f /tmp/${LPTEST} 
	EOF="(EOF):"
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the page mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		test1 for testing the "page" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "page" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}page 1-2
	${waitch}$EOF${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_066.out
	grep "^(EOF):"	/tmp/mail_066.out >/dev/null 2>&1
	TT1="$?"
	grep "^test1" /tmp/mail_066.out >/dev/null 2>&1
	TT2="$?"
	grep "^test2" /tmp/mail_066.out >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}066"  $?
	rm -f /tmp/mail_066.out /usr/spool/mail/${USER}
	echo;echo
}

mail_067()
{
	LPTEST=mail${TX}067
	rm -f /tmp/${LPTEST} 
	EOF="(EOF):"
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the pa (page) mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "pa" (page) mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}pa
	${waitch}$EOF${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_067.out
	grep "^(EOF):"	/tmp/mail_067.out >/dev/null 2>&1
	TT1="$?"
	grep '^testing the "pa" (page) mail subcommand' /tmp/mail_067.out > /dev/null 2<&1 
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}067"  $?
	rm -f /tmp/mail_067.out /usr/spool/mail/${USER}
	echo;echo
}

mail_068()
{
	LPTEST=mail${TX}068
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the Page mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		test1 for testing the "Page" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "Page" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}ignore Status
	${waitch}$mail_prompt${waitch}Page 1-2
	${waitch}:${waitch}$test_prompt
	${waitch}$EOF${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_068.out
	grep "^(EOF):"	/tmp/mail_068.out >/dev/null 2>&1
	TT1="$?"
	grep "test1" /tmp/mail_068.out >/dev/null 2>&1
	TT2="$?"
	grep "test2" /tmp/mail_068.out >/dev/null 2>&1
	TT3="$?"
	grep Status /tmp/mail_068.out >/dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
	lputil verify "mail${TX}068"  $?
	rm -f /tmp/mail_068.out /usr/spool/mail/${USER}
	echo;echo
}
	
mail_069()
{
	LPTEST=mail${TX}069
	rm -f /tmp/${LPTEST} 
	EOF="(EOF):"
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the Page mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "Page" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}Page
	${waitch}$EOF${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_069.out
	grep "^(EOF):"	/tmp/mail_069.out >/dev/null 2>&1
	TT1="$?"
	grep '^testing the "Page" mail subcommand' /tmp/mail_069.out > /dev/null 2<&1 
	TT2="$?"
	grep Status /tmp/mail_069.out >/dev/null 2>&1
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}069"  $?
	rm -f /tmp/mail_069.out /usr/spool/mail/${USER}
	echo;echo
}

mail_070()
{
	LPTEST=mail${TX}070
	rm -f /tmp/${LPTEST} 
	EOF="(EOF):"
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the Pa (Page) mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "Pa" (Page) mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}Pa
	${waitch}$EOF${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_070.out
	grep "^(EOF):"	/tmp/mail_070.out >/dev/null 2>&1
	TT1="$?"
	grep '^testing the "Pa" (Page) mail subcommand' /tmp/mail_070.out > /dev/null 2<&1 
	TT2="$?"
	grep Status /tmp/mail_070.out >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}070"  $?
	rm -f /tmp/mail_070.out /usr/spool/mail/${USER}
	echo;echo
}

mail_071()
{
	LPTEST=mail${TX}071
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the file mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		testing the "file" mail subcommand
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}file
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit  
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS}  | tee /tmp/mail_071.out
	TT1=`grep -c /var/spool/mail/${USER} /tmp/mail_071.out`
	[ $TT1 = 3 ]
	lputil verify "mail${TX}071" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_071.out
	echo;echo
}

mail_072()
{
	LPTEST=mail${TX}072
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the file mail subcommand" 
	# put some mail in a mailbox other than the system's #
       echo "From maryg Tue Oct 09 20:50 CDT 1990
Received: by v31lab3.austin.aic.com (AIX 1.3/4.03)
          id AA16587; Tue, 9 Oct 90 20:49:51 -0500
Date: Tue, 9 Oct 90 20:49:51 -0500
From: maryg
Message-Id: <9010100149.AA16587@v31lab3.austin.aic.com>
To: maryg
Subject: testing_file_name
Status: RO

This is a msg for testing the file<arg> mail subcommand 
" > /tmp/mail_072.out1

	# send some mail #
	mail ${USER} <<-EOF
		testing the "file<arg>" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}delete
	${waitch}$mail_prompt${waitch}file /tmp/mail_072.out1
	${waitch}$mail_prompt${waitch}type 1
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_072.out
	grep '"/tmp/mail_072.out1": 1 message'  /tmp/mail_072.out >/dev/null 2>&1
	TT1="$?"
	grep "^This is a msg for testing the file<arg> mail subcommand" /tmp/mail_072.out >/dev/null 2>&1
	TT2="$?"
	TT3=`ls -l /var/spool/mail/${USER} | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}072" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_072*
	echo;echo
}

mail_073()
{
	LPTEST=mail${TX}073
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the file(#) mail subcommand" 
	# put some mail in a mailbox other than the system's #
	echo "From maryg Tue Oct 09 20:50 CDT 1990
Received: by v31lab3.austin.aic.com (AIX 1.3/4.03)
          id AA16587; Tue, 9 Oct 90 20:49:51 -0500
Date: Tue, 9 Oct 90 20:49:51 -0500
From: maryg
Message-Id: <9010100149.AA16587@v31lab3.austin.aic.com>
To: maryg
Subject: testing_file_name
Status: RO

This is a msg for testing the file(#) mail subcommand 
" > /tmp/mail_073.out1

	# send some mail #
	mail ${USER} <<-EOF
		testing the "file #" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}file #  
	${waitch}$mail_prompt${waitch}file /tmp/mail_073.out1
	${waitch}$mail_prompt${waitch}file #
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_073.out
	grep "^No previous file" /tmp/mail_073.out >/dev/null 2>&1
	TT1="$?"
	TT2=`grep -c /var/spool/mail/${USER}  /tmp/mail_073.out`
	TT3=`ls -l /var/spool/mail/${USER} | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 = 3 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}073" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_073*
	echo;echo
}

mail_074()
{
	LPTEST=mail${TX}074
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the file(%) mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		testing the "file %" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}file %  
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_074.out
	TT1=`grep -c /var/spool/mail/${USER}  /tmp/mail_074.out`
	grep "1 message 1 unread" /tmp/mail_074.out >/dev/null 2>&1
	TT2="$?"
	TT3=`ls -l /var/spool/mail/${USER} | awk '{ print $5 }'`
	[ $TT1 = 4 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}074" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_074*
	echo;echo
}
	
mail_075()
{
    id | grep root >/dev/null
    if test $? = 0  
    then 
	LPTEST=mail${TX}075
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the file(%user) mail subcommand" 
	# send some mail to another user and yourself #
	mail bugs <<-EOF
		test1 for testing the "file %user" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "file %user" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}file %bugs
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_075.out
	grep '"/var/spool/mail/bugs": 1 message 1 new' /tmp/mail_075.out >/dev/null 2>&1
	TT1="$?"
	grep 'test1 for testing the "file %user" mail subcommand' /tmp/mail_075.out >/dev/null 2>&1
	TT2="$?"
	TT3=`ls -l /var/spool/mail/${USER} | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}075" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_075*
	echo;echo
    else echo "mail_075() must be run by root"
    fi
}

mail_076()
{
    id | grep root >/dev/null
    if test $? -ne 0  
    then 
	LPTEST=mail${TX}076
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the file(%user) mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		test2 for testing the "file %user" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}file %bugs
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_076.out
	grep "Permission denied" /tmp/mail_076.out >/dev/null 2>&1
	TT1="$?"
	TT2=`ls -l /usr/spool/mail/${USER} | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}076" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_076*
	echo;echo
    else echo "mail_076() must be run by a non-root user"
    fi
}

mail_077()
{
	LPTEST=mail${TX}077
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the file(&) mail subcommand" 
	# put some mail in $HOME/mbox # 
	echo "From maryg Tue Oct 09 20:50 CDT 1990
Received: by v31lab3.austin.aic.com (AIX 1.3/4.03)
          id AA16587; Tue, 9 Oct 90 20:49:51 -0500
Date: Tue, 9 Oct 90 20:49:51 -0500
From: maryg
Message-Id: <9010100149.AA16587@v31lab3.austin.aic.com>
To:${USER}
Subject: testing_file_name
Status: RO

This is a msg for testing the file(&) mail subcommand 
" > $HOME/mbox

	# send some mail #
	mail ${USER} <<-EOF
		testing the "file &" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}file &  
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_077.out
	grep "$HOME/mbox" /tmp/mail_077.out > /dev/null 2>&1
	TT1="$?"
	grep "This is a msg for testing the file(&) mail subcommand" /tmp/mail_077.out >/dev/null 2>&1
	TT2="$?" 
	TT3=`ls -l /usr/spool/mail/${USER} | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}077" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_077*
	echo;echo
}

mail_078()
{
	LPTEST=mail${TX}078
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the file(+Name) mail subcommand" 
	# first set your folder directory in .mailrc #
	echo "set folder=/tmp/mail_letter" > $HOME/.mailrc
	
	# put some mail in a file in your folder directory #
	mkdir /tmp/mail_letter > /dev/null
       	echo "From maryg Tue Oct 09 20:50 CDT 1990
Received: by v31lab3.austin.aic.com (AIX 1.3/4.03)
          id AA16587; Tue, 9 Oct 90 20:49:51 -0500
Date: Tue, 9 Oct 90 20:49:51 -0500
From: maryg
Message-Id: <9010100149.AA16587@v31lab3.austin.aic.com>
To: maryg
Subject: test
Status: RO

This is a msg for testing the file(+Name) mail subcommand 
" > /tmp/mail_letter/mail_078.out1

	# now send some mail #
	mail ${USER} <<-EOF
		testing the file(+Name) mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}file +mail_078.out1
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_078.out
	grep '"+mail_078.out1": 1 message' /tmp/mail_078.out >/dev/null 2>&1
	TT1="$?"
	grep "^This is a msg for testing the file(+Name) mail subcommand" /tmp/mail_078.out >/dev/null 2>&1
	TT2="$?"
	TT3=`ls -l /usr/spool/mail/${USER} | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}078" $?
	rm -f $HOME/.mailrc
	rm -f /usr/spool/mail/${USER} /tmp/mail_078.out 
	rm -r /tmp/mail_letter >/dev/null 2>&1
	echo;echo
}

mail_079()
{
	LPTEST=mail${TX}079
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the fi (file) mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		testing the "fi" (file) mail subcommand
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}fi
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit  
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS}  | tee /tmp/mail_079.out
	TT1=`grep -c /var/spool/mail/${USER} /tmp/mail_079.out`
	[ $TT1 = 3 ]
	lputil verify "mail${TX}079" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_079.out
	echo;echo
}

mail_080()
{
	LPTEST=mail${TX}080
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the folder mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		testing the "folder" mail subcommand
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}folder
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit  
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS}  | tee /tmp/mail_080.out
	TT1=`grep -c /var/spool/mail/${USER} /tmp/mail_080.out`
	[ $TT1 = 3 ]
	lputil verify "mail${TX}080" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_080.out
	echo;echo
}

mail_081()
{
	LPTEST=mail${TX}081
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the folder mail subcommand" 
	# put some mail in a mailbox other than the system's #
       echo "From maryg Tue Oct 09 20:50 CDT 1990
Received: by v31lab3.austin.aic.com (AIX 1.3/4.03)
          id AA16587; Tue, 9 Oct 90 20:49:51 -0500
Date: Tue, 9 Oct 90 20:49:51 -0500
From: maryg
Message-Id: <9010100149.AA16587@v31lab3.austin.aic.com>
To: maryg
Subject: testing_folder_name
Status: RO

This is a msg for testing the folder<arg> mail subcommand 
" > /tmp/mail_081.out1

	# send some mail #
	mail ${USER} <<-EOF
		testing the "folder<arg>" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}delete
	${waitch}$mail_prompt${waitch}folder /tmp/mail_081.out1
	${waitch}$mail_prompt${waitch}type 1
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_081.out
	grep '"/tmp/mail_081.out1": 1 message'  /tmp/mail_081.out >/dev/null 2>&1
	TT1="$?"
	grep "^This is a msg for testing the folder<arg> mail subcommand" /tmp/mail_081.out >/dev/null 2>&1
	TT2="$?"
	TT3=`ls -l /usr/spool/mail/${USER} | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}081" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_081*
	echo;echo
}

mail_082()
{
	LPTEST=mail${TX}082
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the folder(#) mail subcommand" 
	# put some mail in a mailbox other than the system's #
	echo "From maryg Tue Oct 09 20:50 CDT 1990
Received: by v31lab3.austin.aic.com (AIX 1.3/4.03)
          id AA16587; Tue, 9 Oct 90 20:49:51 -0500
Date: Tue, 9 Oct 90 20:49:51 -0500
From: maryg
Message-Id: <9010100149.AA16587@v31lab3.austin.aic.com>
To: maryg
Subject: testing_folder_name
Status: RO

This is a msg for testing the folder(#) mail subcommand 
" > /tmp/mail_082.out1

	# send some mail #
	mail ${USER} <<-EOF
		testing the "folder #" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}folder #  
	${waitch}$mail_prompt${waitch}folder /tmp/mail_082.out1
	${waitch}$mail_prompt${waitch}folder #
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_082.out
	grep "^No previous file" /tmp/mail_082.out >/dev/null 2>&1
	TT1="$?"
	TT2=`grep -c /var/spool/mail/${USER}  /tmp/mail_082.out`
	TT3=`ls -l /var/spool/mail/${USER} | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 = 3 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}082" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_082*
	echo;echo
}

mail_083()
{
	LPTEST=mail${TX}083
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the folder(%) mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		testing the "folder %" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}folder %  
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_083.out
	TT1=`grep -c /var/spool/mail/${USER}  /tmp/mail_083.out`
	grep "1 message 1 unread" /tmp/mail_083.out >/dev/null 2>&1
	TT2="$?"
	TT3=`ls -l /var/spool/mail/${USER} | awk '{ print $5 }'`
	[ $TT1 = 4 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}083" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_083*
	echo;echo
}
	
mail_084()
{
    id | grep root >/dev/null
    if test $? = 0  
    then 
	LPTEST=mail${TX}084
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the folder(%user) mail subcommand" 
	# send some mail to another user and yourself #
	mail bugs <<-EOF
		test1 for testing the "folder %user" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "folder %user" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}folder %bugs
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_084.out
	grep '"/var/spool/mail/bugs": 1 message 1 new' /tmp/mail_084.out >/dev/null 2>&1
	TT1="$?"
	grep 'test1 for testing the "folder %user" mail subcommand' /tmp/mail_084.out >/dev/null 2>&1
	TT2="$?"
	TT3=`ls -l /var/spool/mail/${USER} | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}084" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_084*
	echo;echo
    else echo "mail_084() must be run by root"
    fi
}

mail_085()
{
    id | grep root >/dev/null
    if test $? -ne 0  
    then 
	LPTEST=mail${TX}085
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the folder(%user) mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		test2 for testing the "folder %user" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}folder %bugs
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_085.out
	grep "Permission denied" /tmp/mail_085.out >/dev/null 2>&1
	TT1="$?"
	TT2=`ls -l /usr/spool/mail/${USER} | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}085" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_085*
	echo;echo
    else echo "mail_085() must be run by a non-root user"
    fi
}

mail_086()
{
	LPTEST=mail${TX}086
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the folder(&) mail subcommand" 
	# put some mail in $HOME/mbox # 
	echo "From maryg Tue Oct 09 20:50 CDT 1990
Received: by v31lab3.austin.aic.com (AIX 1.3/4.03)
          id AA16587; Tue, 9 Oct 90 20:49:51 -0500
Date: Tue, 9 Oct 90 20:49:51 -0500
From: maryg
Message-Id: <9010100149.AA16587@v31lab3.austin.aic.com>
To:${USER}
Subject: testing_folder_name
Status: RO

This is a msg for testing the folder(&) mail subcommand 
" > $HOME/mbox

	# send some mail #
	mail ${USER} <<-EOF
		testing the "folder &" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}folder &  
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_086.out
	grep "$HOME/mbox" /tmp/mail_086.out > /dev/null 2>&1
	TT1="$?"
	grep "This is a msg for testing the folder(&) mail subcommand" /tmp/mail_086.out >/dev/null 2>&1
	TT2="$?" 
	TT3=`ls -l /usr/spool/mail/${USER} | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}086" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_086*
	echo;echo
}

mail_087()
{
	LPTEST=mail${TX}087
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the folder(+Name) mail subcommand" 
	# first set your folder directory in .mailrc #
	echo "set folder=/tmp/mail_letter" > $HOME/.mailrc
	
	# put some mail in a file in your folder directory #
	mkdir /tmp/mail_letter > /dev/null
       	echo "From maryg Tue Oct 09 20:50 CDT 1990
Received: by v31lab3.austin.aic.com (AIX 1.3/4.03)
          id AA16587; Tue, 9 Oct 90 20:49:51 -0500
Date: Tue, 9 Oct 90 20:49:51 -0500
From: maryg
Message-Id: <9010100149.AA16587@v31lab3.austin.aic.com>
To: maryg
Subject: test
Status: RO

This is a msg for testing the folder(+Name) mail subcommand 
" > /tmp/mail_letter/mail_087.out1

	# now send some mail #
	mail ${USER} <<-EOF
		testing the folder(+Name) mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}folder +mail_087.out1
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_087.out
	grep '"+mail_087.out1": 1 message' /tmp/mail_087.out >/dev/null 2>&1
	TT1="$?"
	grep "^This is a msg for testing the folder(+Name) mail subcommand" /tmp/mail_087.out >/dev/null 2>&1
	TT2="$?"
	TT3=`ls -l /usr/spool/mail/${USER} | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}087" $?
	rm -f $HOME/.mailrc
	rm -f /usr/spool/mail/${USER} /tmp/mail_087.out 
	rm -r /tmp/mail_letter >/dev/null 2>&1
	echo;echo
}

mail_088()
{
	LPTEST=mail${TX}088
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the fo (folder) mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		testing the "fo" (folder) mail subcommand
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}fo
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit  
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS}  | tee /tmp/mail_088.out
	TT1=`grep -c /var/spool/mail/${USER} /tmp/mail_088.out`
	[ $TT1 = 3 ]
	lputil verify "mail${TX}088" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_088.out
	echo;echo
}

mail_089()
{
	LPTEST=mail${TX}089
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the folders mail subcommand" 
	echo "set folder=/tmp/mail_letter" > $HOME/.mailrc
	
	# put some files in your folder directory #
	mkdir /tmp/mail_letter >/dev/null
	touch /tmp/mail_letter/letter1 /tmp/mail_letter/letter2 
	touch /tmp/mail_letter/letter3
	 
	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "folders" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}folders
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_089.out
	grep "^letter1 letter2 letter3" /tmp/mail_089.out >/dev/null 2>&1
	TT1="$?"
	lputil verify "mail${TX}089" $?
	rm -f $HOME/.mailrc
	rm -r /tmp/mail_letter >/dev/null 2>&1
	rm -f /usr/spool/mail/${USER} /tmp/mail_089.out		
	echo;echo
}

mail_090()
{
	LPTEST=mail${TX}090
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the next mail subcommand" 
	# send a few pieces of mail #
	mail ${USER} <<-EOF
		test1 for testing the "next" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "next" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test3 for testing the "next" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}next 2-3
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_090.out
	grep '^test2 for testing the "next" mail subcommand' /tmp/mail_090.out >/dev/null 2>&1
	TT1="$?"
	grep '^test1 for testing the "next" mail subcommand' /tmp/mail_090.out >/dev/null 2>&1
	TT2="$?"
	grep '^test3 for testing the "next" mail subcommand' /tmp/mail_090.out >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}090" $?
	rm -f /tmp/mail_090.out /usr/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_091()
{
	LPTEST=mail${TX}091
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the next mail subcommand" 
	# send a few pieces of mail #
	mail ${USER} <<-EOF
		test1 for testing the "next" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "next" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}next 
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_091.out
	grep '^test1 for testing the "next" mail subcommand' /tmp/mail_091.out >/dev/null 2>&1
	TT1="$?"
	grep '^test2 for testing the "next" mail subcommand' /tmp/mail_091.out >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] 
	lputil verify "mail${TX}091" $?
	rm -f /tmp/mail_091.out /usr/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_092()
{
	LPTEST=mail${TX}092
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the n (next) mail subcommand" 
	# send a few pieces of mail #
	mail ${USER} <<-EOF
		test1 for testing the "n" (next) mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "n" (next) mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}n
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_092.out
	grep '^test1 for testing the "n" (next) mail subcommand' /tmp/mail_092.out >/dev/null 2>&1
	TT1="$?"
	grep '^test2 for testing the "n" (next) mail subcommand' /tmp/mail_092.out >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] 
	lputil verify "mail${TX}092" $?
	rm -f /tmp/mail_092.out /usr/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_093()
{
	LPTEST=mail${TX}093
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (new) mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		testing the "new" mail subcommand
	EOF
	sync;sync
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}new
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_093.out
	grep "Held 1 message in /var/spool/mail/${USER}" /tmp/mail_093.out >/dev/null 2>&1
	TT1="$?"
	grep 'testing the "new" mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}093" $?
	rm -f /tmp/mail_093.out /var/spool/mail/${USER}
	echo;echo
}

mail_094()
{
	LPTEST=mail${TX}094
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (new) mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		test1 for testing the "new" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "new" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type 1-2
	${waitch}$mail_prompt${waitch}new 1-2
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_094.out
	grep "Held 2 messages in /var/spool/mail/${USER}" /tmp/mail_094.out >/dev/null 2>&1
	TT1="$?"
	grep 'test1 for testing the "new" mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	grep 'test2 for testing the "new" mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}094" $?
	rm -f /tmp/mail_094.out /var/spool/mail/${USER}
	echo;echo
}

mail_095()
{
	LPTEST=mail${TX}095
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (New) mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		testing the "New" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}New
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_095.out
	grep "Held 1 message in /var/spool/mail/${USER}" /tmp/mail_095.out >/dev/null 2>&1
	TT1="$?"
	grep 'testing the "New" mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}095" $?
	rm -f /tmp/mail_095.out /var/spool/mail/${USER}
	echo;echo
}

mail_096()
{
	LPTEST=mail${TX}096
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (New) mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		test1 for testing the "New" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "New" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type 1-2
	${waitch}$mail_prompt${waitch}New 1-2
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_096.out
	grep "Held 2 messages in /var/spool/mail/${USER}" /tmp/mail_096.out >/dev/null 2>&1
	TT1="$?"
	grep 'test1 for testing the "New" mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	grep 'test2 for testing the "New" mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}096" $?
	rm -f /tmp/mail_096.out /var/spool/mail/${USER}
	echo;echo
}

mail_097()
{
	LPTEST=mail${TX}097
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (unread) mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		testing the "unread" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}unread
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_097.out
	grep "Held 1 message in /var/spool/mail/${USER}" /tmp/mail_097.out >/dev/null 2>&1
	TT1="$?"
	grep 'testing the "unread" mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}097" $?
	rm -f /tmp/mail_097.out /var/spool/mail/${USER}
	echo;echo
}

mail_098()
{
	LPTEST=mail${TX}098
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (unread) mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		test1 for testing the "unread" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "unread" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type 1-2
	${waitch}$mail_prompt${waitch}unread 1-2
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_098.out
	grep "Held 2 messages in /var/spool/mail/${USER}" /tmp/mail_098.out >/dev/null 2>&1
	TT1="$?"
	grep 'test1 for testing the "unread" mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	grep 'test2 for testing the "unread" mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}098" $?
	rm -f /tmp/mail_098.out /var/spool/mail/${USER}
	echo;echo
}

mail_099()
{
	LPTEST=mail${TX}099
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the U (unread) mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		testing the "U" (unread) mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}U
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_099.out
	grep "Held 1 message in /var/spool/mail/${USER}" /tmp/mail_099.out >/dev/null 2>&1
	TT1="$?"
	grep 'testing the "U" (unread) mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}099" $?
	rm -f /tmp/mail_099.out /var/spool/mail/${USER}
	echo;echo
}

mail_100()
{
	LPTEST=mail${TX}100
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (Unread) mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		testing the "Unread" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}Unread
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_100.out
	grep "Held 1 message in /var/spool/mail/${USER}" /tmp/mail_100.out >/dev/null 2>&1
	TT1="$?"
	grep 'testing the "Unread" mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}100" $?
	rm -f /tmp/mail_100.out /var/spool/mail/${USER}
	echo;echo
}

mail_101()
{
	LPTEST=mail${TX}101
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (Unread) mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		test1 for testing the "Unread" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "Unread" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type 1-2
	${waitch}$mail_prompt${waitch}Unread 1-2
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_101.out
	grep "Held 2 messages in /var/spool/mail/${USER}" /tmp/mail_101.out >/dev/null 2>&1
	TT1="$?"
	grep 'test1 for testing the "Unread" mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	grep 'test2 for testing the "Unread" mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}101" $?
	rm -f /tmp/mail_101.out /usr/spool/mail/${USER}
	echo;echo
}

mail_102()
{
	LPTEST=mail${TX}102
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (headers) mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		testing the "headers" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}headers
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_102.out
	TT1=`grep -c "N  1 ${USER}" /tmp/mail_102.out`
	[ $TT1 = 2 ]
	lputil verify "mail${TX}102" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_102.out
	echo;echo
}

mail_103()
{
	LPTEST=mail${TX}103
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the h (headers) mail subcommand" 
	# send some mail #
	mail ${USER} <<-EOF
		testing the "h" (headers) mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}h
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_103.out
	TT1=`grep -c "N  1 ${USER}" /tmp/mail_103.out`
	[ $TT1 = 2 ]
	lputil verify "mail${TX}103" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_103.out
	echo;echo
}

mail_104()
{
	LPTEST=mail${TX}104
	echo "Started `date` - mail TEST Number   - $LPTEST"
	rm -f /tmp/${LPTEST}
	{ 
	echo "$LPTEST test of (help) mail subcommand"
	# send some mail #
	mail ${USER} <<-EOF
		testing the "help" mail subcommand 
	EOF
	sync;sync
	echo "help
x
" | mail > /tmp/mail_104.out
	sync;sync
	} | tee -a ${RESULTS} 
	grep "Control Commands" /tmp/mail_104.out >/dev/null
	TT1="$?"
	grep "Display Commands:" /tmp/mail_104.out >/dev/null
	TT2="$?"
	grep "Message Handling:" /tmp/mail_104.out >/dev/null
	TT3="$?"
	grep "Creating New Mail:" /tmp/mail_104.out > /dev/null
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
	lputil verify "mail${TX}104" $? 
	rm -f /tmp/mail_104.out /var/spool/mail/${USER}
	echo;echo
}

mail_105()
{
	LPTEST=mail${TX}105
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of (hold) mail subcommand"
	# send some mail #
	mail ${USER} <<-EOF
		testing the "hold" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}hold
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS}  | tee /tmp/mail_105.out
	grep "Held 1 message in /var/spool/mail/${USER}" /tmp/mail_105.out >/dev/null
	TT1="$?"
	grep 'testing the "hold" mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}105" $?
	rm -f /tmp/mail_105.out /var/spool/mail/${USER}
	echo;echo
}

mail_106()
{
	LPTEST=mail${TX}106
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of (hold) mail subcommand"
	# send some mail #
	mail ${USER} <<-EOF
		test1 for testing the "hold" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "hold" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type 1-2
	${waitch}$mail_prompt${waitch}hold 1-2
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS}  | tee /tmp/mail_106.out
	grep "Held 2 messages in /var/spool/mail/${USER}" /tmp/mail_106.out >/dev/null
	TT1="$?"
	grep 'test1 for testing the "hold" mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	grep 'test2 for testing the "hold" mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}106" $?
	rm -f /tmp/mail_106.out /var/spool/mail/${USER}
	echo;echo
}

mail_107()
{
	LPTEST=mail${TX}107
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of ho (hold) mail subcommand"
	# send some mail #
	mail ${USER} <<-EOF
		testing the "ho" (hold) mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}ho
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS}  | tee /tmp/mail_107.out
	grep "Held 1 message in /var/spool/mail/${USER}" /tmp/mail_107.out >/dev/null
	TT1="$?"
	grep 'testing the "ho" (hold) mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}107" $?
	rm -f /tmp/mail_107.out /var/spool/mail/${USER}
	echo;echo
}

mail_108()
{
	LPTEST=mail${TX}108
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of (preserve) mail subcommand"
	# send some mail #
	mail ${USER} <<-EOF
		testing the "preserve" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}preserve
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS}  | tee /tmp/mail_108.out
	grep "Held 1 message in /var/spool/mail/${USER}" /tmp/mail_108.out >/dev/null
	TT1="$?"
	grep 'testing the "preserve" mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}108" $?
	rm -f /tmp/mail_108.out /var/spool/mail/${USER}
	echo;echo
}

mail_109()
{
	LPTEST=mail${TX}109
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of (preserve) mail subcommand"
	# send some mail #
	mail ${USER} <<-EOF
		test1 for testing the "preserve" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "preserve" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type 1-2
	${waitch}$mail_prompt${waitch}preserve 1-2
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS}  | tee /tmp/mail_109.out
	grep "Held 2 messages in /var/spool/mail/${USER}" /tmp/mail_109.out >/dev/null
	TT1="$?"
	grep 'test1 for testing the "preserve" mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	grep 'test2 for testing the "preserve" mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}109" $?
	rm -f /tmp/mail_109.out /var/spool/mail/${USER}
	echo;echo
}

mail_110()
{
	LPTEST=mail${TX}110
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of pre (preserve) mail subcommand"
	# send some mail #
	mail ${USER} <<-EOF
		testing the "pre" (preserve) mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}pre
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS}  | tee /tmp/mail_110.out
	grep "Held 1 message in /var/spool/mail/${USER}" /tmp/mail_110.out >/dev/null
	TT1="$?"
	grep 'testing the "pre" (preserve) mail subcommand' /var/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}110" $?
	rm -f /tmp/mail_110.out /var/spool/mail/${USER}
	echo;echo
}

mail_111()
{
	LPTEST=mail${TX}111
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of if-endif mail subcommand"
	# send some mail #
	mail ${USER} <<-EOF
		testing the if-endif  mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}if receive
	${waitch}$mail_prompt${waitch}list
	${waitch}$mail_prompt${waitch}endif
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync 
	} | tee -a ${RESULTS} | tee /tmp/mail_111.out
	grep "^Commands are:" /tmp/mail_111.out >/dev/null 2>&1
	lputil verify "mail${TX}111" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_111.out
	echo;echo
}

mail_112()
{
	LPTEST=mail${TX}112
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of if-else-endif mail subcommand"
	# send some mail #
	mail ${USER} <<-EOF
		testing the if-else-endif mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail	
	${waitch}$mail_prompt${waitch}if send 
	${waitch}$mail_prompt${waitch}echo "the then part is true"
	${waitch}$mail_prompt${waitch}else 
	${waitch}$mail_prompt${waitch}echo "the else part is true"
	${waitch}$mail_prompt${waitch}endif
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync 
	} | tee -a ${RESULTS} | tee /tmp/mail_112.out
	grep "^the then part is true" /tmp/mail_112.out >/dev/null 2>&1
	TT1="$?"
	grep "^the else part is true" /tmp/mail_112.out >/dev/null 2>&1
	TT2="$?"
	[ $TT1 -ne 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}112" $?
	rm -f /usr/spool/mail/${USER} $HOME/mbox /tmp/mail_112.out
	echo;echo
}

mail_113()
{
	LPTEST=mail${TX}113
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of ignore mail subcommand"
	# set the ignore command in $HOME/.mailrc #
	echo "ignore Received Message-Id Resent-Message-Id Mail-From Return-Path" > $HOME/.mailrc

	# send some mail #
	mail ${USER} <<-EOF
		testing the "ignore" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}ignore
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync      
	} | tee -a ${RESULTS} | tee /tmp/mail_113.out
	grep "^mail-from" /tmp/mail_113.out >/dev/null 2>&1
	TT1="$?"
	grep "^message-id" /tmp/mail_113.out >/dev/null 2>&1
	TT2="$?"
	grep "^received" /tmp/mail_113.out >/dev/null 2>&1
	TT3="$?"
	grep "^resent-message-id" /tmp/mail_113.out >/dev/null 2>&1
	TT4="$?"
	grep "^return-path" /tmp/mail_113.out >/dev/null 2>&1
	TT5="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] 	
	lputil verify "mail${TX}113" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_113.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_114()
{
	LPTEST=mail${TX}114
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of ignore mail subcommand"

	# send some mail #
	mail ${USER} <<-EOF
		testing the "ignore" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}ignore To
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_114.out
	grep "^To:" /tmp/mail_114.out >/dev/null 2>&1
	TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "mail${TX}114" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_114.out
	echo;echo
}
	
mail_115()
{
	LPTEST=mail${TX}115
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of discard mail subcommand"
	# set the discard command in $HOME/.mailrc #
	echo "discard Received Message-Id Resent-Message-Id Mail-From Return-Path" > $HOME/.mailrc

	# send some mail #
	mail ${USER} <<-EOF
		testing the "discard" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}discard
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync      
	} | tee -a ${RESULTS} | tee /tmp/mail_115.out
	grep "^mail-from" /tmp/mail_115.out >/dev/null 2>&1
	TT1="$?"
	grep "^message-id" /tmp/mail_115.out >/dev/null 2>&1
	TT2="$?"
	grep "^received" /tmp/mail_115.out >/dev/null 2>&1
	TT3="$?"
	grep "^resent-message-id" /tmp/mail_115.out >/dev/null 2>&1
	TT4="$?"
	grep "^return-path" /tmp/mail_115.out >/dev/null 2>&1
	TT5="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] 	
	lputil verify "mail${TX}115" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_115.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_116()
{
	LPTEST=mail${TX}116
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of discard mail subcommand"

	# send some mail #
	mail ${USER} <<-EOF
		testing the "discard" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}discard To
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_116.out
	grep "^To:" /tmp/mail_116.out >/dev/null 2>&1
	TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "mail${TX}116" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_116.out
	echo;echo
}

mail_117()
{
	LPTEST=mail${TX}117
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of di (discard) mail subcommand"
	# set the discard command in $HOME/.mailrc #
	echo "di Received Message-Id Resent-Message-Id Mail-From Return-Path" > $HOME/.mailrc

	# send some mail #
	mail ${USER} <<-EOF
		testing the "di" (discard) mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}di
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync      
	} | tee -a ${RESULTS} | tee /tmp/mail_117.out
	grep "^mail-from" /tmp/mail_117.out >/dev/null 2>&1
	TT1="$?"
	grep "^message-id" /tmp/mail_117.out >/dev/null 2>&1
	TT2="$?"
	grep "^received" /tmp/mail_117.out >/dev/null 2>&1
	TT3="$?"
	grep "^resent-message-id" /tmp/mail_117.out >/dev/null 2>&1
	TT4="$?"
	grep "^return-path" /tmp/mail_117.out >/dev/null 2>&1
	TT5="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] 	
	lputil verify "mail${TX}117" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_117.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_118()
{
	LPTEST=mail${TX}118
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of from mail subcommand"

	# send some mail #
	mail ${USER} <<-EOF
		testing the "from" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "from" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test3 for testing the "from" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}from 2-3
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_118.out
	TT1=`grep -c "N  1 ${USER}" /tmp/mail_118.out`
	TT2=`grep -c "N  2 ${USER}" /tmp/mail_118.out`
	TT3=`grep -c "N  3 ${USER}" /tmp/mail_118.out`
	[ $TT1 = 1 ] && [ $TT2 = 2 ] && [ $TT3 = 2 ]
	lputil verify "mail${TX}118" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_118.out
	echo;echo
}

mail_119()
{
	LPTEST=mail${TX}119
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of from mail subcommand"

	# send some mail #
	mail ${USER} <<-EOF
		test1 for testing the "from" mail subcommand
	EOF
	sync;sync
	# send some mail from a different user #
	/usr/lib/sendmail -f"cinderella" ${USER} <<-EOF
		test2 for testing the "from" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test3 for testing the "from" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}from cinderella
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_119.out
	TT1=`grep -c "N  1 ${USER}" /tmp/mail_119.out`
	TT2=`grep -c "N  2 cinderella" /tmp/mail_119.out`
	TT3=`grep -c "N  3 ${USER}" /tmp/mail_119.out`
	[ $TT1 = 1 ] && [ $TT2 = 2 ] && [ $TT3 = 1 ]
	lputil verify "mail${TX}119" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_119.out
	echo;echo
}

mail_120()
{
	LPTEST=mail${TX}120
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of f (from) mail subcommand"

	# send some mail #
	mail ${USER} <<-EOF
		test1 for testing the "f" (from) mail subcommand
	EOF
	sync;sync
	# send some mail from a different user #
	/usr/lib/sendmail -f"cinderella" ${USER} <<-EOF
		test2 for testing the "f" (from) mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test3 for testing the "f" (from) mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}f cinderella
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_120.out
	TT1=`grep -c "N  1 ${USER}" /tmp/mail_120.out`
	TT2=`grep -c "N  2 cinderella" /tmp/mail_120.out`
	TT3=`grep -c "N  3 ${USER}" /tmp/mail_120.out`
	[ $TT1 = 1 ] && [ $TT2 = 2 ] && [ $TT3 = 1 ]
	lputil verify "mail${TX}120" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_120.out
	echo;echo
}

mail_121()
{
	LPTEST=mail${TX}121
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of mbox mail subcommand"

	# send some mail #
	mail ${USER} <<-EOF
		testing the "mbox" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}mbox 
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_121.out
	grep "Saved 1 message in $HOME/mbox" /tmp/mail_121.out >/dev/null 2>&1
	TT1="$?"
	grep 'testing the "mbox" mail subcommand' $HOME/mbox >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}121" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_121.out
	echo;echo
}

mail_122()
{
	LPTEST=mail${TX}122
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST test of mbox mail subcommand"

	# send some mail #
	mail ${USER} <<-EOF
		test1 for testing the "mbox" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "mbox" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}mbox 1-2
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_122.out
	grep "Saved 2 messages in $HOME/mbox" /tmp/mail_122.out >/dev/null 2>&1
	TT1="$?"
	grep 'testing the "mbox" mail subcommand' $HOME/mbox >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}122" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_122.out
	echo;echo
}

mail_124()
{
	LPTEST=mail${TX}124
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the type mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "type" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_124.out
	grep '^testing the "type" mail subcommand' /tmp/mail_124.out > /dev/null 2<&1 
	lputil verify "mail${TX}124"  $?
	rm -f /tmp/mail_124.out /usr/spool/mail/${USER}
	echo;echo
}

mail_125()
{
	LPTEST=mail${TX}125
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the type mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		test1 for testing the "type" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "type" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type 1-2
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_125.out
	grep "^test1" /tmp/mail_125.out >/dev/null 2>&1
	TT1="$?"
	grep "^test2" /tmp/mail_125.out >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] 
	lputil verify "mail${TX}125"  $?
	rm -f /tmp/mail_125.out /usr/spool/mail/${USER}
	echo;echo
}

mail_126()
{
	LPTEST=mail${TX}126
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the t (type) mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "t" (type) mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}t
	${waitch}$EOF${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_126.out
	grep '^testing the "t" (type) mail subcommand' /tmp/mail_126.out > /dev/null 2<&1 
	lputil verify "mail${TX}126"  $?
	rm -f /tmp/mail_126.out /usr/spool/mail/${USER}
	echo;echo
}

mail_127()
{
	LPTEST=mail${TX}127
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the Type mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		test1 for testing the "Type" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "Type" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}ignore Status
	${waitch}$mail_prompt${waitch}Type 1-2
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_127.out
	grep "test1" /tmp/mail_127.out >/dev/null 2>&1
	TT1="$?"
	grep "test2" /tmp/mail_127.out >/dev/null 2>&1
	TT2="$?"
	grep Status /tmp/mail_127.out >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] 
	lputil verify "mail${TX}127"  $?
	rm -f /tmp/mail_127.out /usr/spool/mail/${USER}
	echo;echo
}
	
mail_128()
{
	LPTEST=mail${TX}128
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the Type mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "Type" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}Type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_128.out
	grep '^testing the "Type" mail subcommand' /tmp/mail_128.out > /dev/null 2<&1 
	TT1="$?"
	grep Status /tmp/mail_128.out >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}128"  $?
	rm -f /tmp/mail_128.out /usr/spool/mail/${USER}
	echo;echo
}

mail_129()
{
	LPTEST=mail${TX}129
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the T (Type) mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "T" (Type) mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}T
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_129.out
	grep '^testing the "T" (Type) mail subcommand' /tmp/mail_129.out > /dev/null 2<&1 
	TT1="$?"
	grep Status /tmp/mail_129.out >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}129"  $?
	rm -f /tmp/mail_129.out /usr/spool/mail/${USER}
	echo;echo
}

mail_130()
{
	LPTEST=mail${TX}130
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the print mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "print" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}print
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_130.out
	grep '^testing the "print" mail subcommand' /tmp/mail_130.out > /dev/null 2<&1 
	lputil verify "mail${TX}130"  $?
	rm -f /tmp/mail_130.out /usr/spool/mail/${USER}
	echo;echo
}

mail_131()
{
	LPTEST=mail${TX}131
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the print mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		test1 for testing the "print" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "print" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}print 1-2
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_131.out
	grep "^test1" /tmp/mail_131.out >/dev/null 2>&1
	TT1="$?"
	grep "^test2" /tmp/mail_131.out >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] 
	lputil verify "mail${TX}131"  $?
	rm -f /tmp/mail_131.out /usr/spool/mail/${USER}
	echo;echo
}

mail_132()
{
	LPTEST=mail${TX}132
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the p (print) mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "p" (print) mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}p
	${waitch}$EOF${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_132.out
	grep '^testing the "p" (print) mail subcommand' /tmp/mail_132.out > /dev/null 2<&1 
	lputil verify "mail${TX}132"  $?
	rm -f /tmp/mail_132.out /usr/spool/mail/${USER}
	echo;echo
}

mail_133()
{
	LPTEST=mail${TX}133
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the Print mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		test1 for testing the "Print" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "Print" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}ignore Status
	${waitch}$mail_prompt${waitch}Print 1-2
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_133.out
	grep "test1" /tmp/mail_133.out >/dev/null 2>&1
	TT1="$?"
	grep "test2" /tmp/mail_133.out >/dev/null 2>&1
	TT2="$?"
	grep Status /tmp/mail_133.out >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] 
	lputil verify "mail${TX}133"  $?
	rm -f /tmp/mail_133.out /usr/spool/mail/${USER}
	echo;echo
}
	
mail_134()
{
	LPTEST=mail${TX}134
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the Print mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "Print" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}Print
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_134.out
	grep '^testing the "Print" mail subcommand' /tmp/mail_134.out > /dev/null 2<&1 
	TT1="$?"
	grep Status /tmp/mail_134.out >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}134"  $?
	rm -f /tmp/mail_134.out /usr/spool/mail/${USER}
	echo;echo
}

mail_135()
{
	LPTEST=mail${TX}135
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the P (Print) mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "P" (Print) mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}P
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_135.out
	grep '^testing the "P" (Print) mail subcommand' /tmp/mail_135.out > /dev/null 2<&1 
	TT1="$?"
	grep Status /tmp/mail_135.out >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}135"  $?
	rm -f /tmp/mail_135.out /usr/spool/mail/${USER}
	echo;echo
}

mail_136()
{
	LPTEST=mail${TX}136
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the retain mail subcommand" 
	# send some mail first #
	echo "retain To" > $HOME/.mailrc
	mail ${USER} <<-EOF
		testing the "retain" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}retain 
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_136.out
	grep "to" /tmp/mail_136.out > /dev/null 2<&1 
	lputil verify "mail${TX}136"  $?
	rm -f /tmp/mail_136.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	echo;echo
}

mail_137()
{
	LPTEST=mail${TX}137
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the retain mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "retain" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}retain To
	${waitch}$mail_prompt${waitch}t 
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_137.out
	grep "^To: ${USER}" /tmp/mail_137.out > /dev/null 2<&1 
	TT1="$?"
	grep "Date:" /tmp/mail_137.out > /dev/null 2>&1
	TT2="$?"
	grep "From:" /tmp/mail_137.out >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 -ne 0 ] 
	lputil verify "mail${TX}137"  $?
	rm -f /tmp/mail_137.out /usr/spool/mail/${USER}
	echo;echo
}

mail_138()
{
	LPTEST=mail${TX}138
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the retain mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "retain" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}retain To
	${waitch}$mail_prompt${waitch}p 
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_138.out
	grep "^To: ${USER}" /tmp/mail_138.out > /dev/null 2<&1 
	TT1="$?"
	grep "Date:" /tmp/mail_138.out > /dev/null 2>&1
	TT2="$?"
	grep "From:" /tmp/mail_138.out >/dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 -ne 0 ] 
	lputil verify "mail${TX}138"  $?
	rm -f /tmp/mail_138.out /usr/spool/mail/${USER}
	echo;echo
}

mail_139()
{
	LPTEST=mail${TX}139
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the save mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		test1 for testing the "save" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "save" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}save 1-2 /tmp/mail_139.out1
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync 
	} | tee -a ${RESULTS} | tee /tmp/mail_139.out
	grep "\"/tmp/mail_139.out1\" \[New file\] 16/646" /tmp/mail_139.out > /dev/null 2>&1 
	TT1="$?"
	grep 'test1 for testing the "save" mail subcommand' /tmp/mail_139.out1 >/dev/null 2>&1
	TT2="$?"
	grep 'test2 for testing the "save" mail subcommand' /tmp/mail_139.out1 >/dev/null 2>&1
	TT3="$?"
	TT4=`grep -c "Received:" /tmp/mail_139.out1`
	TT5=`grep -c "Date:" /tmp/mail_139.out1`
	TT6=`grep -c "From:" /tmp/mail_139.out1`
	TT7=`grep -c "Message-Id:" /tmp/mail_139.out1`
	test -f $HOME/mbox
	TT8="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 2 ] && [ $TT5 = 2 ] && [ $TT6 = 2 ] && [ $TT7 = 2 ] && [ $TT8 -ne 0 ]
	lputil verify "mail${TX}139" $?
	rm -f /tmp/mail_139*  /usr/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_140()
{
	LPTEST=mail${TX}140
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the save mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the mail subcommand, "save".
	EOF
	sync;sync	
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}save /tmp/mail_140.out1
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_140.out
	grep "\"/tmp/mail_140.out1\" \[New file\] 8/315" /tmp/mail_140.out > /dev/null 2>&1 
	TT1="$?"
	test -f $HOME/mbox
	TT2="$?"
	TT3=`grep -c "Received:" /tmp/mail_140.out1`
	TT4=`grep -c "Date:" /tmp/mail_140.out1`
	TT5=`grep -c "From:" /tmp/mail_140.out1`
	TT6=`grep -c "Message-Id:" /tmp/mail_140.out1`
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 = 1 ] && [ $TT4 = 1 ] && [ $TT5 = 1 ] && [ $TT6 = 1 ] 
	lputil verify "mail${TX}140" $?
	rm -f /tmp/mail_140*  /usr/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_141()
{
	LPTEST=mail${TX}141
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the save mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the mail subcommand, "save".
	EOF
	sync;sync	
	mail ${USER} <<-EOF
		testing the mail subcommand, "save"
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}save 1 /tmp/mail_141.out1
	${waitch}$mail_prompt${waitch}save 2 /tmp/mail_141.out1
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_141.out
	grep "\"/tmp/mail_141.out1\" \[New file\] 8/315" /tmp/mail_141.out > /dev/null 2>&1 
	TT1="$?"
	grep "\"/tmp/mail_141.out1\" \[Appended\] 8/314" /tmp/mail_141.out > /dev/null 2>&1
	TT2="$?"
	test -f $HOME/mbox
	TT3="$?"
	TT4=`grep -c "Received:" /tmp/mail_141.out1`
	TT5=`grep -c "Date:" /tmp/mail_141.out1`
	TT6=`grep -c "From:" /tmp/mail_141.out1`
	TT7=`grep -c "Message-Id:" /tmp/mail_141.out1`
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ] && [ $TT4 = 2 ] && [ $TT5 = 2 ] && [ $TT6 = 2 ] && [ $TT7 = 2 ] 
	lputil verify "mail${TX}141" $?
	rm -f /tmp/mail_141*  /usr/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_142()
{
	LPTEST=mail${TX}142
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the s (save) mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the mail subcommand, "s" (save)
	EOF
	sync;sync	
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}s /tmp/mail_142.out1
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_142.out
	grep "\"/tmp/mail_142.out1\" \[New file\] 8/318" /tmp/mail_142.out > /dev/null 2>&1 
	TT1="$?"
	test -f $HOME/mbox
	TT2="$?"
	TT3=`grep -c "Received:" /tmp/mail_142.out1`
	TT4=`grep -c "Date:" /tmp/mail_142.out1`
	TT5=`grep -c "From:" /tmp/mail_142.out1`
	TT6=`grep -c "Message-Id:" /tmp/mail_142.out1`
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 = 1 ] && [ $TT4 = 1 ] && [ $TT5 = 1 ] && [ $TT6 = 1 ]
	lputil verify "mail${TX}142" $?
	rm -f /tmp/mail_142*  /usr/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_143()
{
	LPTEST=mail${TX}143
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the quit mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the mail subcommand, quit 
	EOF
	sync;sync	
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_143.out
	grep  "testing the mail subcommand, quit" /tmp/mail_143.out > /dev/null 2>&1 
	TT1="$?"
	grep "Saved 1 message in $HOME/mbox" /tmp/mail_143.out >/dev/null 2>&1
	TT2="$?"
	grep "testing the mail subcommand, quit" $HOME/mbox > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}143" $?
	rm -f /tmp/mail_143.out  /usr/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_144()
{
	LPTEST=mail${TX}144
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the quit mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the mail subcommand, quit 
	EOF
	sync;sync	
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}delete
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_144.out
	grep "Saved 1 message in $HOME/mbox" /tmp/mail_144.out >/dev/null 2>&1
	TT1="$?"
	grep "testing the mail subcommand, quit" $HOME/mbox > /dev/null 2>&1
	TT2="$?"
	[ $TT1 -ne 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}144" $?
	rm -f /tmp/mail_144.out  /usr/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_145()
{
	LPTEST=mail${TX}145
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the quit mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the mail subcommand, quit 
	EOF
	sync;sync	
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_145.out
	grep  "testing the mail subcommand, quit" /tmp/mail_145.out > /dev/null 2>&1 
	TT1="$?"
	grep "Saved 1 message in $HOME/mbox" /tmp/mail_145.out >/dev/null 2>&1
	TT2="$?"
	grep "testing the mail subcommand, quit" $HOME/mbox > /dev/null 2>&1
	TT3="$?"
	grep "testing the mail subcommand, quit" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT4="$?"
	[ $TT1 -ne 0 ] && [ $TT2 -ne 0 ] && [ $TT3 -ne 0 ] && [ $TT4 = 0 ]
	lputil verify "mail${TX}145" $?
	rm -f /tmp/mail_145.out  /usr/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_146()
{
	LPTEST=mail${TX}146
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the q (quit) mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the mail subcommand, q (quit)
	EOF
	sync;sync	
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}q
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_146.out
	grep  "testing the mail subcommand, q (quit)" /tmp/mail_146.out > /dev/null 2>&1 
	TT1="$?"
	grep "Saved 1 message in $HOME/mbox" /tmp/mail_146.out >/dev/null 2>&1
	TT2="$?"
	grep "testing the mail subcommand, q (quit)" $HOME/mbox > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}146" $?
	rm -f /tmp/mail_146.out  /usr/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_147()
{
	LPTEST=mail${TX}147
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	echo "$LPTEST FVT testing the Reply mail subcommand" 
	# send some mail first #
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail ${USER}
	${waitch}Subject: ${waitch}test
	${waitch}$test_prompt${waitch}testing the Reply mail subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}bugs
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	{ 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}Reply
	${waitch}$test_prompt${waitch}test2
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sleep 3
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_147.out
	grep "To: ${USER}" /tmp/mail_147.out > /dev/null 2>&1
	TT1="$?"
	grep test2 /usr/spool/mail/${USER} >/dev/null 2>&1	
	TT2="$?"
	grep "Cc: bugs" /tmp/mail_147.out > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}147" $?
	rm -f /tmp/mail_147.out  /usr/spool/mail/${USER} $HOME/mbox
	rm -f /usr/spool/mail/bugs
	echo;echo
}

mail_148()
{
	LPTEST=mail${TX}148
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	echo "$LPTEST FVT testing the R (Reply) mail subcommand" 
	# send some mail first #
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail ${USER}
	${waitch}Subject: ${waitch}test
	${waitch}$test_prompt${waitch}testing the R (Reply) mail subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}bugs
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	{
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}R
	${waitch}$test_prompt${waitch}test2
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sleep 3
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_148.out
	grep "To: ${USER}" /tmp/mail_148.out > /dev/null 2>&1
	TT1="$?"
	grep test2 /usr/spool/mail/${USER} >/dev/null 2>&1	
	TT2="$?"
	grep "Cc: bugs" /tmp/mail_148.out > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}148" $?
	rm -f /tmp/mail_148.out  /usr/spool/mail/${USER} $HOME/mbox
	rm -f /usr/spool/mail/bugs
	echo;echo
}

mail_149()
{
	LPTEST=mail${TX}149
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	echo "$LPTEST FVT testing the reply mail subcommand" 
	# send some mail first #
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail ${USER}
	${waitch}Subject: ${waitch}test
	${waitch}$test_prompt${waitch}testing the reply mail subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}bugs
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	{
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}reply
	${waitch}$test_prompt${waitch}test2
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	sleep 3
	} | tee -a ${RESULTS} | tee /tmp/mail_149.out
	grep "To: ${USER}" /tmp/mail_149.out > /dev/null 2>&1
	TT1="$?"
	grep test2 /usr/spool/mail/${USER} >/dev/null 2>&1	
	TT2="$?"
	grep "Cc: bugs" /tmp/mail_149.out > /dev/null 2>&1
	TT3="$?"
	grep test2 /usr/spool/mail/bugs > /dev/null 2>&1
	TT4="$?" 
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
	lputil verify "mail${TX}149" $?
	rm -f /tmp/mail_149.out  /usr/spool/mail/${USER} $HOME/mbox
	rm -f /usr/spool/mail/bugs
	echo;echo
}

mail_150()
{
	LPTEST=mail${TX}150
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	echo "$LPTEST FVT testing the r (reply) mail subcommand" 
	# send some mail first #
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail ${USER}
	${waitch}Subject: ${waitch}test
	${waitch}$test_prompt${waitch}testing the r (reply) mail subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}bugs
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	{
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}r
	${waitch}$test_prompt${waitch}test2
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	sleep 3
	} | tee -a ${RESULTS} | tee /tmp/mail_150.out
	grep "To: ${USER}" /tmp/mail_150.out > /dev/null 2>&1
	TT1="$?"
	grep test2 /var/spool/mail/${USER} >/dev/null 2>&1	
	TT2="$?"
	grep "Cc: bugs" /tmp/mail_150.out > /dev/null 2>&1
	TT3="$?"
	grep test2 /var/spool/mail/bugs > /dev/null 2>&1
	TT4="$?" 
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
	lputil verify "mail${TX}150" $?
	rm -f /tmp/mail_150.out  /var/spool/mail/${USER} $HOME/mbox
	rm -f /var/spool/mail/bugs
	echo;echo
}

mail_151()
{
	LPTEST=mail${TX}151
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	echo "$LPTEST FVT testing the Respond mail subcommand" 
	# send some mail first #
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail ${USER}
	${waitch}Subject: ${waitch}test
	${waitch}$test_prompt${waitch}testing the Respond mail subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}bugs
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	{ 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}Respond
	${waitch}$test_prompt${waitch}test2
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sleep 3
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_151.out
	grep "To: ${USER}" /tmp/mail_151.out > /dev/null 2>&1
	TT1="$?"
	grep test2 /usr/spool/mail/${USER} >/dev/null 2>&1	
	TT2="$?"
	grep "Cc: bugs" /tmp/mail_151.out > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}151" $?
	rm -f /tmp/mail_151.out  /usr/spool/mail/${USER} $HOME/mbox
	rm -f /usr/spool/mail/bugs
	echo;echo
}

mail_152()
{
	LPTEST=mail${TX}152
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	echo "$LPTEST FVT testing the respond mail subcommand" 
	# send some mail first #
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail ${USER}
	${waitch}Subject: ${waitch}test
	${waitch}$test_prompt${waitch}testing the respond mail subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}bugs
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	{
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}respond
	${waitch}$test_prompt${waitch}test2
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	sleep 3
	} | tee -a ${RESULTS} | tee /tmp/mail_152.out
	grep "To: ${USER}" /tmp/mail_152.out > /dev/null 2>&1
	TT1="$?"
	grep test2 /usr/spool/mail/${USER} >/dev/null 2>&1	
	TT2="$?"
	grep "Cc: bugs" /tmp/mail_152.out > /dev/null 2>&1
	TT3="$?"
	grep test2 /usr/spool/mail/bugs > /dev/null 2>&1
	TT4="$?" 
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
	lputil verify "mail${TX}152" $?
	rm -f /tmp/mail_152.out  /usr/spool/mail/${USER} $HOME/mbox
	rm -f /usr/spool/mail/bugs
	echo;echo
}

mail_154()
{
	LPTEST=mail${TX}154
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the undelete mail subcommand" 
	# send some mail first so we can get into mail mode #
	mail ${USER} <<-EOF
		testing the "undelete" mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}delete
	${waitch}$mail_prompt${waitch}t 1
	${waitch}$mail_prompt${waitch}undelete
	${waitch}$mail_prompt${waitch}t 1
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_154.out
	grep "1: Inappropriate message" /tmp/mail_154.out > /dev/null 2>&1
	TT1="$?"
	grep 'testing the "undelete" mail subcommand' /tmp/mail_154.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] 
	lputil verify "mail${TX}154" $?
	rm -f  /usr/spool/mail/${USER} $HOME/mbox /tmp/mail_154.out
	echo;echo
}

mail_155()
{
	LPTEST=mail${TX}155
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the u (undelete) mail subcommand" 
	# send some mail first so we can get into mail mode #
	mail ${USER} <<-EOF
		testing the "u" (undelete) mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}delete
	${waitch}$mail_prompt${waitch}t 1
	${waitch}$mail_prompt${waitch}u
	${waitch}$mail_prompt${waitch}t 1
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_155.out
	grep "1: Inappropriate message" /tmp/mail_155.out > /dev/null 2>&1
	TT1="$?"
	grep 'testing the "u" (undelete) mail subcommand' /tmp/mail_155.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] 
	lputil verify "mail${TX}155" $?
	rm -f  /usr/spool/mail/${USER} $HOME/mbox /tmp/mail_155.out
	echo;echo
}

mail_156()
{
	LPTEST=mail${TX}156
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the shell mail subcommand" 
	# send some mail first so we can get into mail mode #
	mail ${USER} <<-EOF
		testing the "shell" mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}shell
	${waitch}${sh_prompt}${waitch}uname
	${waitch}${sh_prompt}${waitch}exit
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_156.out
	grep "AIX" /tmp/mail_156.out > /dev/null 2>&1
	lputil verify "mail${TX}156" $?
	rm -f  /usr/spool/mail/${USER} $HOME/mbox /tmp/mail_156.out
	echo;echo
}
	
mail_157()
{
	LPTEST=mail${TX}157
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the sh (shell) mail subcommand" 
	# send some mail first so we can get into mail mode #
	mail ${USER} <<-EOF
		testing the "sh"(shell) mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}sh
	${waitch}${sh_prompt}${waitch}uname
	${waitch}${sh_prompt}${waitch}exit
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_157.out
	grep "AIX" /tmp/mail_157.out > /dev/null 2>&1
	lputil verify "mail${TX}157" $?
	rm -f  /usr/spool/mail/${USER} $HOME/mbox /tmp/mail_157.out
	echo;echo
}
	
mail_158()
{
	LPTEST=mail${TX}158
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the size mail subcommand" 
	# send some mail first so we can get into mail mode #
	mail ${USER} <<-EOF
		testing the "size" mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}size
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_158.out
	grep "1: 10/313" /tmp/mail_158.out > /dev/null 2>&1
	lputil verify "mail${TX}158" $?
	rm -f  /var/spool/mail/${USER} $HOME/mbox /tmp/mail_158.out
	echo;echo
}
	
mail_159()
{
	LPTEST=mail${TX}159
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the touch mail subcommand" 
	# send some mail first so we can get into mail mode #
	mail ${USER} <<-EOF
		testing the "touch" mail subcommand
	EOF
	sync;sync;sync
	sleep
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}touch
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} 
	grep 'testing the "touch" mail subcommand' $HOME/mbox > /dev/null 2>&1 
	lputil verify "mail${TX}159" $?
	rm -f  /var/spool/mail/${USER} $HOME/mbox
	echo;echo
}
	
	
mail_160()
{
	LPTEST=mail${TX}160
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the source mail subcommand" 
	# set up a source file with a few mail commands #
	echo "size"  > /tmp/mail_160.run
	echo "help" >> /tmp/mail_160.run
 
	# send some mail so we can get into mail mode #
	mail ${USER} <<-EOF
		testing the "source" mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}source /tmp/mail_160.run
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_160.out 
	grep "1: 10/315" /tmp/mail_160.out > /dev/null 2>&1
	TT1="$?"
	grep "Control Commands:" /tmp/mail_160.out > /dev/null 2>&1
	TT2="$?"
	grep "Display Commands:" /tmp/mail_160.out > /dev/null 2>&1
	TT3="$?"
	grep "Message Handling:" /tmp/mail_160.out > /dev/null 2>&1
	TT4="$?"
	grep "Creating New Mail:" /tmp/mail_160.out > /dev/null 2>&1
	TT5="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ]
	lputil verify "mail${TX}160" $?
	rm -f  /usr/spool/mail/${USER}
	rm -f /tmp/mail_160.*
	echo;echo
}
	
mail_161()
{
	LPTEST=mail${TX}161
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the so (source) mail subcommand" 
	# set up a source file with a few mail commands #
	echo "size"  > /tmp/mail_161.run
	echo "help" >> /tmp/mail_161.run

	# send some mail so we can get into mail mode #
	mail ${USER} <<-EOF
		testing the "so" (source) mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}so /tmp/mail_161.run
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_161.out 
	grep "1: 10/320" /tmp/mail_161.out > /dev/null 2>&1
	TT1="$?"
	grep "Control Commands:" /tmp/mail_161.out > /dev/null 2>&1
	TT2="$?"
	grep "Display Commands:" /tmp/mail_161.out > /dev/null 2>&1
	TT3="$?"
	grep "Message Handling:" /tmp/mail_161.out > /dev/null 2>&1
	TT4="$?"
	grep "Creating New Mail:" /tmp/mail_161.out > /dev/null 2>&1
	TT5="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ]
	lputil verify "mail${TX}161" $?
	rm -f  /usr/spool/mail/${USER} 
	rm -f /tmp/mail_161.*
	echo;echo
}

mail_162()
{
	LPTEST=mail${TX}162
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the top mail subcommand" 
	# send some mail so we can get into mail mode #
	mail ${USER} <<-EOF
		testing the "top" mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}top
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_162.out
	grep "Message  1:" /tmp/mail_162.out > /dev/null 2>&1
	TT1="$?"
	grep "From root"  /tmp/mail_162.out > /dev/null 2>&1
	TT2="$?"
	grep "Received:"  /tmp/mail_162.out > /dev/null 2>&1
	TT3="$?"
	grep "Date:"  /tmp/mail_162.out > /dev/null 2>&1
	TT4="$?"
	grep "From:" /tmp/mail_162.out > /dev/null 2>&1
	TT5="$?"
	grep "To:" /tmp/mail_162.out > /dev/null 2>&1
	TT6="$?"
	grep "Subject:" /tmp/mail_162.out > /dev/null 2>&1
	TT7="$?"
	grep 'testing the "top" mail subcommand' > /dev/null 2>&1
	TT8="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] && [ $TT6 -ne 0 ] && [ $TT7 -ne 0 ] && [ $TT8 -ne 0 ]
	lputil verify "mail${TX}162" $?
	rm -f  /usr/spool/mail/${USER}
	rm -f /tmp/mail_162.out
	echo;echo
}
	
mail_163()
{
	LPTEST=mail${TX}163
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the version mail subcommand" 
	# send some mail so we can get into mail mode #
	mail ${USER} <<-EOF
		testing the "version" mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}version
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_163.out
	grep "Version \[5.2 UCB\] \[AIX 4.1\]" /tmp/mail_163.out >/dev/null 2>&1
	TT1="$?"
	[ $TT1 = 0 ]
	lputil verify "mail${TX}163" $?
	rm -f  /var/spool/mail/${USER}
	rm -f /tmp/mail_163.out
	echo;echo
}
	
mail_164()
{
	LPTEST=mail${TX}164
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the ve (version) mail subcommand" 
	# send some mail so we can get into mail mode #
	mail ${USER} <<-EOF
		testing the "ve" (version) mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}ve
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_164.out
	grep "Version \[5.2 UCB\] \[AIX 4.1\]" /tmp/mail_164.out >/dev/null 2>&1
	TT1="$?"
	lputil verify "mail${TX}164" $?
	rm -f  /usr/spool/mail/${USER}
	rm -f /tmp/mail_164.out
	echo;echo
}
	
mail_165()
{
	LPTEST=mail${TX}165
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the write mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		test1 for testing the "write" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "write" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}write 1-2 /tmp/mail_165.out1
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync 
	} | tee -a ${RESULTS} | tee /tmp/mail_165.out
	grep "\"/tmp/mail_165.out1\" \[New file\]" /tmp/mail_165.out >/dev/null 2>&1 
	TT1="$?"
	grep 'test1 for testing the "write" mail subcommand' /tmp/mail_165.out1 >/dev/null 2>&1
	TT2="$?"
	grep 'test2 for testing the "write" mail subcommand' /tmp/mail_165.out1 >/dev/null 2>&1
	TT3="$?"
	TT4=`grep -c "Received:" /tmp/mail_165.out1`
	TT5=`grep -c "Date:" /tmp/mail_165.out1`
	TT6=`grep -c "From:" /tmp/mail_165.out1`
	TT7=`grep -c "Message-Id:" /tmp/mail_165.out1`
	test -f $HOME/mbox
	TT8="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] && [ $TT6 = 0 ] && [ $TT7 = 0 ] && [ $TT8 -ne 0 ]
	lputil verify "mail${TX}165" $?
	rm -f /tmp/mail_165*  /usr/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_166()
{
	LPTEST=mail${TX}166
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the write mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the mail subcommand, "write".
	EOF
	sync;sync	
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}write /tmp/mail_166.out1
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_166.out
	grep "\"/tmp/mail_166.out1\" \[New file\]" /tmp/mail_166.out > /dev/null 2>&1
	TT1="$?"
	test -f $HOME/mbox
	TT2="$?"
	TT3=`grep -c "Received:" /tmp/mail_166.out1`
	TT4=`grep -c "Date:" /tmp/mail_166.out1`
	TT5=`grep -c "From:" /tmp/mail_166.out1`
	TT6=`grep -c "Message-Id:" /tmp/mail_166.out1`
	grep "testing the mail subcommand, \"write\"." /tmp/mail_166.out1 >/dev/null 2>&1
	TT7="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] && [ $TT6 = 0 ] && [ $TT7 = 0 ] 
	lputil verify "mail${TX}166" $?
	rm -f /tmp/mail_166*  /var/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_167()
{
	LPTEST=mail${TX}167
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the w (write) mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the mail subcommand, "w" (write).
	EOF
	sync;sync	
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}w /tmp/mail_167.out1
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_167.out
	grep "\"/tmp/mail_167.out1\" \[New file\]" /tmp/mail_167.out > /dev/null 2>&1 
	TT1="$?"
	test -f $HOME/mbox
	TT2="$?"
	TT3=`grep -c "Received:" /tmp/mail_167.out1`
	TT4=`grep -c "Date:" /tmp/mail_167.out1`
	TT5=`grep -c "From:" /tmp/mail_167.out1`
	TT6=`grep -c "Message-Id:" /tmp/mail_167.out1`
	grep "testing the mail subcommand, \"w\" (write)." /tmp/mail_167.out1 > /dev/null 2>&1 
	TT7="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] && [ $TT6 = 0 ] && [ $TT7 = 0 ]
	lputil verify "mail${TX}167" $?
	rm -f /tmp/mail_167*  /usr/spool/mail/${USER} $HOME/mbox
	echo;echo
}

mail_168()
{
	LPTEST=mail${TX}168
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the xit mail subcommand" 
	# first send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "xit" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}delete
	${waitch}$mail_prompt${waitch}xit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} 
	grep 'testing the "xit" mail subcommand' /usr/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	lputil verify "mail${TX}168" $?
	rm -f /usr/spool/mail/${USER}
	echo;echo
}

mail_169()
{
	LPTEST=mail${TX}169
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the z+ mail subcommand" 
	# first send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "z+" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}z+ 
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_169.out
	grep "On last screenful of messages" /tmp/mail_169.out > /dev/null 2>&1
	lputil verify "mail${TX}169" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_169.out
	echo;echo
}

mail_170()
{
	LPTEST=mail${TX}170
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the z- mail subcommand" 
	# first send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "z-" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}z- 
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_170.out
	grep "On first screenful of messages" /tmp/mail_170.out > /dev/null 2>&1
	lputil verify "mail${TX}170" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_170.out
	echo;echo
}

mail_171()
{
	LPTEST=mail${TX}171
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the z mail subcommand" 
	# first send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "z" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}z 
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_171.out
	grep "On last screenful of messages" /tmp/mail_171.out > /dev/null 2>&1
	lputil verify "mail${TX}171" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_171.out
	echo;echo
}

mail_172()
{
	LPTEST=mail${TX}172
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set mail subcommand" 
	echo "set ask askcc dot" > $HOME/.mailrc

	# first send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "set" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}set
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_172.out
	grep ^ask /tmp/mail_172.out > /dev/null 2>&1
	TT1="$?"
	grep ^askcc /tmp/mail_172.out > /dev/null 2>&1
	TT2="$?"
	grep ^dot /tmp/mail_172.out > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}172" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_172.out
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_173()
{
	LPTEST=mail${TX}173
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set append) mail subcommand" 

	echo "set append" > $HOME/.mailrc

	# first send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "set append" subcommand 
	EOF
	sync;sync;sync

	# put some mail in $HOME/mbox #
	echo "From root Fri Feb 14 10:03:36 1992
Received: by mickey.mouse.disney (AIX 3.2/UCB 5.64/4.03)
	  id AA13448; Fri, 14 Feb 1992 10:03:36 -0600
Date: Fri, 14 Feb 1992 10:03:36 -0600
From: minnie
To: mickey
Subject: valentines

Happy Valentines day!!" > $HOME/mbox 
	sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}t1
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_173.out
	set `grep -n "Happy Valentines day" $HOME/mbox | sed 'y/:/ /'` ; TT1=$1
	set `grep -n "testing the" $HOME/mbox | sed 'y/:/ /'` ; TT2=$1
	[ $TT1 -le $TT2 ]
	lputil verify "mail${TX}173" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_173.out
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_174()
{
	LPTEST=mail${TX}174
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set ask) mail subcommand" 

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail ${USER}
	${waitch}Subject: ${waitch}mail_test
	${waitch}$test_prompt${waitch}testing the set ask subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_174.out
	# the fact that the above passed verifies that "set ask" works
	#  but let's do some double checking
	grep "Subject: " /tmp/mail_174.out > /dev/null 2>&1
	lputil verify "mail${TX}174" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_174.out
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_175()
{
	LPTEST=mail${TX}175
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set askcc) mail subcommand" 

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail ${USER}
	${waitch}Subject: ${waitch}mail_test
	${waitch}$test_prompt${waitch}testing the set askcc subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_175.out
	# the fact that the above passed verifies that "set askcc" works
	# but let's do some double checking
	grep "Cc: " /tmp/mail_175.out > /dev/null 2>&1
	lputil verify "mail${TX}175" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_175.out
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_176()
{
	LPTEST=mail${TX}176
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set autoprint) mail subcommand" 
	echo "set autoprint" > $HOME/.mailrc

	# send a few pieces of mail
	mail ${USER} <<-EOF
		test1 for testing the "set autoprint" subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF 
		test2 for testing the "set autoprint" subcommand
	EOF
	sync;sync
	sleep 3

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail 
	${waitch}$mail_prompt${waitch}delete
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_176.out
	grep 'test1 for testing the "set autoprint" subcommand' /tmp/mail_176.out > /dev/null 2>&1
	TT1="$?"
	grep 'test2 for testing the "set autoprint" subcommand' /tmp/mail_176.out > /dev/null 2>&1
	TT2="$?"
	grep "Saved 1 message in $HOME/mbox" /tmp/mail_176.out > /dev/null 2>&1
	TT3="$?"
	[ $TT1 -ne 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}176" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_176.out
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_177()
{
	LPTEST=mail${TX}177
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set debug) mail subcommand" 
	echo "set debug" > $HOME/.mailrc
	
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail ${USER}
	${waitch}Subject: ${waitch}mail_test
	${waitch}$test_prompt${waitch}testing the set debug subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_177.out
	grep "uid = 0, user = ${USER}, mailname = /var/spool/mail/${USER}" /tmp/mail_177.out > /dev/null 2>&1
	TT1="$?"
	grep "deadletter = $HOME/dead.letter, mailrc = $HOME/.mailrc, mbox = $HOME/mbox" /tmp/mail_177.out > /dev/null 2>&1
	TT2="$?"
	grep "Recipients of message:" /tmp/mail_177.out > /dev/null 2>&1
	TT3="$?"
	grep '"send-mail" "-i" "-x" "root"' /tmp/mail_177.out > /dev/null 2>&1
	TT4="$?"
	grep "testing the set debug subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT5="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 -ne 0 ]
	lputil verify "mail${TX}177" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_177.out
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_178()
{
	LPTEST=mail${TX}178
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set dot) mail subcommand" 

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail ${USER}
	${waitch}Subject: ${waitch}mail_test
	${waitch}$test_prompt${waitch}testing the set dot subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	sleep 3
	} | tee -a ${RESULTS} | tee /tmp/mail_178.out
	# the fact that the above passed verifies that "set dot" works
	grep "testing the set dot subcommand" /tmp/mail_178.out > /dev/null 2>&1
	lputil verify "mail${TX}178" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_178.out
	echo;echo
}

mail_179()
{
	LPTEST=mail${TX}179
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set hold) mail subcommand" 
	echo "set hold" > $HOME/.mailrc
	mail ${USER} <<-EOF
		testing the set hold subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_179.out
	grep "Held 1 message in /var/spool/mail/${USER}" /tmp/mail_179.out > /dev/null 2>&1
	TT1="$?"
	grep "testing the set hold subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}179" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_179.out $HOME/.mailrc
	echo;echo
}
	
	
mail_180()
{
	LPTEST=mail${TX}180
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set ignore) mail subcommand" 
	echo "set ignore" > $HOME/.mailrc
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail ${USER}
	${waitch}Subject: ${waitch}mail_test
	${waitch}$test_prompt${waitch}testing the set ignore subcommand
	${waitch}$test_prompt${waitch}
	${waitch}$test_prompt${waitch}
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_180.out
	grep "Interrupt -- one more to kill letter" /tmp/mail_180.out > /dev/null 2>&1
	TT1="$?"
	grep "testing the set ignore subcommand" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	[ $TT1 -ne 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}180" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_180.out
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_181()
{
	LPTEST=mail${TX}181
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set keep) mail subcommand" 
	mail ${USER} <<-EOF
		testing the set keep subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS}
	TT1=`ls -l /usr/spool/mail/${USER} | awk '{ print $5 }'`
	[ $TT1 = 0 ]
	lputil verify "mail${TX}181" $?
	rm -f /usr/spool/mail/${USER} 
	echo;echo
}
	
mail_182()
{
	LPTEST=mail${TX}182
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set keepsave) mail subcommand" 
	echo "set keepsave" > $HOME/.mailrc
	mail ${USER} <<-EOF
		testing the set keepsave subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}s /tmp/mail_182.output
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_182.out
	grep "Held 1 message in /var/spool/mail/${USER}" /tmp/mail_182.out > /dev/null 2>&1
	TT1="$?"
	grep  "testing the set keepsave subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep  "testing the set keepsave subcommand" /tmp/mail_182.output >/dev/null 2>&1 
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}182" $?
	rm -f /usr/spool/mail/${USER} $HOME/mbox /tmp/mail_182.*
	rm -f $HOME/.mailrc
	echo;echo
}

mail_183()
{
	LPTEST=mail${TX}183
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set keepsave) mail subcommand" 
	echo "set keepsave" > $HOME/.mailrc
	mail ${USER} <<-EOF
		testing the set keepsave subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}w /tmp/mail_183.output
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_183.out
	grep "Held 1 message in /var/spool/mail/${USER}" /tmp/mail_183.out > /dev/null 2>&1
	TT1="$?"
	grep  "testing the set keepsave subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep  "testing the set keepsave subcommand" /tmp/mail_183.output > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}183" $?
	rm -f /usr/spool/mail/${USER} $HOME/mbox /tmp/mail_183.*
	rm -f $HOME/.mailrc
	echo;echo
}

mail_184()
{
	LPTEST=mail${TX}184
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set metoo) mail subcommand" 
	echo "set metoo" > $HOME/.mailrc
	echo "alias tester ${USER} ${USER2}" >> $HOME/.mailrc
	mail tester <<-EOF
		testing the set metoo subcommand
	EOF
	sync;sync
	} | tee -a ${RESULTS} 
	grep "testing the set metoo subcommand" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "testing the set metoo subcommand" /usr/spool/mail/${USER2} > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}184" $?
	rm -f /usr/spool/mail/${USER} /usr/spool/mail/${USER2} 
	rm -f $HOME/.mailrc
	echo;echo
}

mail_185()
{
	LPTEST=mail${TX}185
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set noheader) mail subcommand" 
	echo "set noheader" > $HOME/.mailrc
	mail ${USER} <<-EOF
		testing the set noheader subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_185.out
	grep "5.2 UCB" /tmp/mail_185.out > /dev/null 2>&1
	TT1="$?"
	grep "1 message 1 new" /tmp/mail_185.out > /dev/null 2>&1
	TT2="$?"
	grep "N  1 ${USER}" /tmp/mail_185.out > /dev/null 2>&1
	TT3="$?"
	[ $TT1 -ne 0 ] && [ $TT2 -ne 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}185" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_185.out
	rm -f $HOME/.mailrc
	echo;echo
}


mail_186()
{
	LPTEST=mail${TX}186
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set quiet) mail subcommand" 
	echo "set quiet" > $HOME/.mailrc
	mail ${USER} <<-EOF
		testing the set quiet subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_186.out
	grep "5.2 UCB" /tmp/mail_186.out > /dev/null 2>&1
	TT1="$?"
	grep "IBM AIX 3.2" /tmp/mail_186.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 -ne 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}186" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_186.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_187()
{
	LPTEST=mail${TX}187
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set Replyall) mail subcommand" 
	echo "set Replyall" > $HOME/.mailrc
	mail ${USER} ${USER2} <<-EOF
		testing the set Replyall subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}Reply
	${waitch}$test_prompt${waitch}test2 for the set Replyall subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_187.out
	sleep 3
	grep "test2 for the set Replyall subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "test2 for the set Replyall subcommand" /var/spool/mail/${USER2} > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] 
	lputil verify "mail${TX}187" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_187.out /var/spool/mail/${USER2}
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_188()
{
	LPTEST=mail${TX}188
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set Replyall) mail subcommand" 
	echo "set Replyall" > $HOME/.mailrc
	mail ${USER} ${USER2} <<-EOF
		testing the set Replyall subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}reply
	${waitch}$test_prompt${waitch}test2 for the set Replyall subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_188.out
	grep "test2 for the set Replyall subcommand" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "test2 for the set Replyall subcommand" /usr/spool/mail/${USER2} > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}188" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_188.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_189()
{
	LPTEST=mail${TX}189
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set verbose) mail subcommand" 
	echo "set verbose" > $HOME/.mailrc
	mail ${USER} <<-EOF > /tmp/mail_189.out
		testing the set verbose subcommand
	EOF
	sync;sync
	} | tee -a ${RESULTS} 
	grep "${USER}... Connecting to .local..." /tmp/mail_189.out > /dev/null 2>&1
	TT1="$?"
	grep "${USER}... Sent" /tmp/mail_189.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}189" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_189.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_190()
{
	LPTEST=mail${TX}190
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set record) mail subcommand" 
	echo "set record=/tmp/mail_190.out" > $HOME/.mailrc
	mail ${USER2} <<-EOF
		testing the set record subcommand
	EOF
	sync;sync
	} | tee -a ${RESULTS} 
	grep "testing the set record subcommand" /tmp/mail_190.out > /dev/null 2>&1
	lputil verify "mail${TX}190" $?
	rm -f /usr/spool/mail/${USER2} /tmp/mail_190.out
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_191()
{
	LPTEST=mail${TX}191
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (set screen) mail subcommand" 
	echo "set screen=2" > $HOME/.mailrc

	# send a few pieces of mail 
	for i in 1 2 3 ; do mail -s test$i ${USER} < /dev/null >/dev/null; done
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_191.out
	grep "test1" /tmp/mail_191.out > /dev/null 2>&1
	TT1="$?"
	grep "test2" /tmp/mail_191.out > /dev/null 2>&1
	TT2="$?"
	grep "test3" /tmp/mail_191.out > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}191" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_191.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_192()
{
	LPTEST=mail${TX}192
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the set toplines mail subcommand" 
	# send some mail so we can get into mail mode #

	echo "set toplines=3" > $HOME/.mailrc

	mail ${USER} <<-EOF
		testing the set toplines subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}top
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_192.out
	grep "Message  1:" /tmp/mail_192.out > /dev/null 2>&1
	TT1="$?"
	grep "From root"  /tmp/mail_192.out > /dev/null 2>&1
	TT2="$?"
	grep "Received:"  /tmp/mail_192.out > /dev/null 2>&1
	TT3="$?"
	grep "Date:"  /tmp/mail_192.out > /dev/null 2>&1
	TT4="$?"
	grep "From:" /tmp/mail_192.out > /dev/null 2>&1
	TT5="$?"
	grep "To:" /tmp/mail_192.out > /dev/null 2>&1
	TT6="$?"
	grep "Subject:" /tmp/mail_192.out > /dev/null 2>&1
	TT7="$?"
	grep 'testing the "top" mail subcommand' /tmp/mail_192.out> /dev/null 2>&1
	TT8="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 -ne 0 ] && [ $TT5 -ne 0 ] && [ $TT6 -ne 0 ] && [ $TT7 -ne 0 ] && [ $TT8 -ne 0 ]
	lputil verify "mail${TX}192" $?
	rm -f  /usr/spool/mail/${USER} $HOME/.mailrc /tmp/mail_192.out
	echo;echo
}

mail_193()
{
	LPTEST=mail${TX}193
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the set SHELL mail subcommand" 
	# send some mail first so we can get into mail mode #

	echo "set SHELL=/bin/sh" > $HOME/.mailrc

	mail ${USER} <<-EOF
		testing the set SHELL subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}shell
	${waitch}${sh_prompt}${waitch}set -o vi
	${waitch}${sh_prompt}${waitch}exit
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_193.out
	grep -e "-o: bad option(s)" /tmp/mail_193.out > /dev/null 2>&1
	lputil verify "mail${TX}193" $?
	rm -f  /usr/spool/mail/${USER} $HOME/mbox /tmp/mail_193.out
	echo;echo
}
	
mail_194()
{
	LPTEST=mail${TX}194
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the set PAGER mail subcommand" 
	# send some mail first so we can get into mail mode #

	echo "set PAGER=/usr/bin/more" > $HOME/.mailrc

	mail ${USER} <<-EOF
		testing the set PAGER subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}page
	${waitch}$mail_prompt${waitch} quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_194.out
	grep "testing the set PAGER subcommand" /tmp/mail_194.out > /dev/null 2>&1
	TT1="$?"
	#fgrep [H[J /tmp/mail_194.out > /dev/null 2>&1
	fgrep "stdin: END" /tmp/mail_194.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}194"  $?
	rm -f /tmp/mail_194.out /usr/spool/mail/${USER}
	echo;echo
}

mail_195()
{
	LPTEST=mail${TX}195
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (unset append) mail subcommand" 

	echo "set append" > $HOME/.mailrc

	# first send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "unset append" subcommand 
	EOF
	sync;sync;sync

	# put some mail in $HOME/mbox #
	echo "From root Fri Feb 14 10:03:36 1992
Received: by mickey.mouse.disney (AIX 3.2/UCB 5.64/4.03)
	  id AA13448; Fri, 14 Feb 1992 10:03:36 -0600
Date: Fri, 14 Feb 1992 10:03:36 -0600
From: minnie
To: mickey
Subject: valentines

Happy Valentines day!!" > $HOME/mbox 
	sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}unset append
	${waitch}$mail_prompt${waitch}t1
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_195.out
	set `grep -n "Happy Valentines day" $HOME/mbox | sed 'y/:/ /'` ; TT1=$1
	set `grep -n "testing the" $HOME/mbox | sed 'y/:/ /'` ; TT2=$1
	[ $TT1 -ge $TT2 ]
	lputil verify "mail${TX}195" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_195.out
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_196()
{
	LPTEST=mail${TX}196
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (unset askcc) mail subcommand" 
	echo "unset askcc" > $HOME/.mailrc 

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail ${USER}
	${waitch}Subject: ${waitch}mail_test
	${waitch}$test_prompt${waitch}testing the unset askcc subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_196.out
	# the fact that the above passed verifies that "unset askcc" works
	# but let's do some double checking
	grep "Cc: " /tmp/mail_196.out > /dev/null 2>&1
	TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "mail${TX}196" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_196.out
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_197()
{
	LPTEST=mail${TX}197
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (unset autoprint) mail subcommand" 
	echo "set autoprint" > $HOME/.mailrc

	# send a few pieces of mail
	mail ${USER} <<-EOF
		test1 for testing the "set autoprint" subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF 
		test2 for testing the "set autoprint" subcommand
	EOF
	sync;sync
	sleep 3

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail 
	${waitch}$mail_prompt${waitch}unset autoprint
	${waitch}$mail_prompt${waitch}delete
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_197.out
	grep 'test1 for testing the "set autoprint" subcommand' /tmp/mail_197.out > /dev/null 2>&1
	TT1="$?"
	grep 'test2 for testing the "set autoprint" subcommand' /tmp/mail_197.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 -ne 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}197" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_197.out
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_198()
{
	LPTEST=mail${TX}198
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (unset debug) mail subcommand" 
	echo "set debug" > $HOME/.mailrc
	# send some mail to get in mail mode
	echo "From root Fri Feb 14 10:03:36 1992
Received: by mickey.mouse.disney (AIX 3.2/UCB 5.64/4.03)
	  id AA13448; Fri, 14 Feb 1992 10:03:36 -0600
Date: Fri, 14 Feb 1992 10:03:36 -0600
From: minnie 
To: mickey
Subject: where's goofy        

Is goofy on vacation??"  >  $HOME/mbox
	sync
	
	##  now test mail -f flag ##
	sleep 3	
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail -f
	${waitch}$mail_prompt${waitch}unset debug
	${waitch}$mail_prompt${waitch}mail ${USER}
	${waitch}Subject: ${waitch}mail_test
	${waitch}$test_prompt${waitch}testing the unset debug subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_198.out
	grep "Recipients of message:" /tmp/mail_198.out > /dev/null 2>&1
	TT1="$?"
	grep '"send-mail" "-i" "-x" "root"' /tmp/mail_198.out > /dev/null 2>&1
	TT2="$?"
	grep "testing the unset debug subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	[ $TT1 -ne 0 ] && [ $TT2 -ne 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}198" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_198.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_199()
{
	LPTEST=mail${TX}199
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (unset hold) mail subcommand" 
	echo "set hold" > $HOME/.mailrc
	mail ${USER} <<-EOF
		testing the unset hold subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}unset hold
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_199.out
	grep "Saved 1 message in $HOME/mbox" /tmp/mail_199.out > /dev/null 2>&1
	TT1="$?"
	grep "testing the unset hold subcommand" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "testing the unset hold subcommand" $HOME/mbox > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}199" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_199.out $HOME/mbox
	rm -f $HOME/.mailrc
	echo;echo
}
	
	
mail_200()
{
	LPTEST=mail${TX}200
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (unset ignore) mail subcommand" 
	echo "set ignore" > $HOME/.mailrc
	# send some mail to get in mail mode
	mail ${USER} <<-EOF
		testing the unset ignore subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}unset ignore
	${waitch}$mail_prompt${waitch}mail bugs 
	${waitch}Subject: ${waitch}mail_test
	${waitch}$test_prompt${waitch}testing the unset ignore mail subcommand
	${waitch}$test_prompt${waitch}
	${waitch}$test_prompt${waitch}
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_200.out
	grep "Interrupt -- one more to kill letter" /tmp/mail_200.out > /dev/null 2>&1
	TT1="$?"
	grep  "testing the unset ignore mail subcommand" /tmp/mail_200.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}200" $?
	rm -f /var/spool/mail/bugs /tmp/mail_200.out
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_201()
{
	LPTEST=mail${TX}201
	rm -f /tmp/${LPTEST} 
	rm -f /usr/spool/mail/${USER}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (unset keep) mail subcommand" 
	mail ${USER} <<-EOF
		testing the unset keep subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}unset keep
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS}
	ls -l /usr/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	[ $TT1 = 2 ]
	lputil verify "mail${TX}201" $?
	rm -f /usr/spool/mail/${USER} 
	echo;echo
}
	
mail_202()
{
	LPTEST=mail${TX}202
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (unset keepsave) mail subcommand" 
	echo "set keepsave" > $HOME/.mailrc
	mail ${USER} <<-EOF
		testing the unset keepsave subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}unset keepsave
	${waitch}$mail_prompt${waitch}s /tmp/mail_202.output
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_202.out
	grep "Saved 1 message in $HOME/mbox" /tmp/mail_202.out > /dev/null 2>&1
	TT1="$?"
	grep  "testing the unset keepsave subcommand" $HOME/mbox > /dev/null 2>&1
	TT2="$?"
	[ $TT1 -ne 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}202" $?
	rm -f /usr/spool/mail/${USER}  /tmp/mail_202.*
	rm -f $HOME/.mailrc
	echo;echo
}

mail_203()
{
	LPTEST=mail${TX}203
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (unset metoo) mail subcommand" 
	echo "set metoo" > $HOME/.mailrc
	echo "alias tester ${USER} ${USER2}" >> $HOME/.mailrc

	# send some mail to get in mail mode #
	mail ${USER} <<-EOF
		testing the unset metoo subcommand
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}unset metoo
	${waitch}$mail_prompt${waitch}mail tester
	${waitch}Subject: ${waitch}mail_test
	${waitch}$test_prompt${waitch}test2 for the unset metoo subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} 
	grep "test2 for the unset metoo subcommand" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "test2 for the unset metoo subcommand" /usr/spool/mail/${USER2} > /dev/null 2>&1
	TT2="$?"
	[ $TT1 -ne 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}203" $?
	rm -f /usr/spool/mail/${USER} /usr/spool/mail/${USER2} 
	rm -f $HOME/.mailrc
	echo;echo
}

mail_204()
{
	LPTEST=mail${TX}204  
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (unset noheader) mail subcommand" 
	cp /usr/share/lib/Mail.rc /usr/share/lib/Mail.rc.bak2
	echo "set noheader" >> /usr/share/lib/Mail.rc
	echo "unset noheader" > $HOME/.mailrc
	mail ${USER} <<-EOF
		testing the unset noheader subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_204.out
	grep "5.2 UCB" /tmp/mail_204.out > /dev/null 2>&1
	TT1="$?"
	grep "1 message 1 new" /tmp/mail_204.out > /dev/null 2>&1
	TT2="$?"
	grep "N  1 ${USER}" /tmp/mail_204.out > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}204" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_204.out
	rm -f $HOME/.mailrc
	cp /usr/share/lib/Mail.rc.bak2 /usr/share/lib/Mail.rc
	rm -f /usr/share/lib/Mail.rc.bak2
	echo;echo
}


mail_205()
{
	LPTEST=mail${TX}205
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (unset quiet) mail subcommand" 
	cp /usr/share/lib/Mail.rc /usr/share/lib/Mail.rc.bak2
	echo "set quiet" >> /usr/share/lib/Mail.rc
	echo "unset quiet" > $HOME/.mailrc
	mail ${USER} <<-EOF
		testing the unset quiet subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_205.out
	grep "\[5.2 UCB\] \[AIX 4.1\]" /tmp/mail_205.out > /dev/null 2>&1
	TT1="$?"
	[ $TT1 = 0 ] 
	lputil verify "mail${TX}205" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_205.out
	rm -f $HOME/.mailrc
	cp /usr/share/lib/Mail.rc.bak2 /usr/share/lib/Mail.rc
	rm -f /usr/share/lib/Mail.rc.bak2
	echo;echo
}

mail_206()
{
	LPTEST=mail${TX}206
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (unset Replyall) mail subcommand" 
	cp /usr/share/lib/Mail.rc /usr/share/lib/Mail.rc.bak2
	echo "set Replyall" >> /usr/share/lib/Mail.rc
	echo "unset Replyall" > $HOME/.mailrc
	mail ${USER} ${USER2} <<-EOF
		testing the unset Replyall subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}reply
	${waitch}$test_prompt${waitch}test2 for the set Replyall subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_206.out
	sleep 3
	grep "test2 for the set Replyall subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "test2 for the set Replyall subcommand" /var/spool/mail/${USER2} > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] 
	lputil verify "mail${TX}206" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_206.out /var/spool/mail/${USER2}
	rm -f $HOME/.mailrc
	cp /usr/share/lib/Mail.rc.bak /usr/share/lib/Mail.rc
	rm -f /usr/share/lib/Mail.rc.bak2
	echo;echo
}
	
mail_207()
{
	LPTEST=mail${TX}207
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (unset Replyall) mail subcommand" 
	cp /usr/share/lib/Mail.rc /usr/share/lib/Mail.rc.bak2
	echo "set Replyall" >> /usr/share/lib/Mail.rc
	echo "unset Replyall" > $HOME/.mailrc
	mail ${USER} ${USER2} <<-EOF
		testing the unset Replyall subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}Reply
	${waitch}$test_prompt${waitch}test2 for the set Replyall subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_207.out
	grep "test2 for the set Replyall subcommand" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "test2 for the set Replyall subcommand" /usr/spool/mail/${USER2} > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}207" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_207.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_208()
{
	LPTEST=mail${TX}208
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (unset verbose) mail subcommand" 
	cp /usr/share/lib/Mail.rc /usr/share/lib/Mail.rc.bak2
	echo "set verbose" >> usr/share/lib/Mail.rc
	echo "unset verbose" > $HOME/.mailrc
	mail ${USER} <<-EOF > /tmp/mail_208.out
		testing the unset verbose subcommand
	EOF
	sync;sync
	} | tee -a ${RESULTS} 
	grep "${USER}... Connecting to .local..." /tmp/mail_208.out > /dev/null 2>&1
	TT1="$?"
	grep "${USER}... Sent" /tmp/mail_208.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 -ne 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}208" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_208.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_209()
{
	LPTEST=mail${TX}209
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (unset record) mail subcommand" 
	cp /usr/share/lib/Mail.rc /usr/share/lib/Mail.rc.bak2
	echo "set record=/tmp/mail_209.out" >> /usr/share/lib/Mail.rc
	echo "unset record" > $HOME/.mailrc
	mail ${USER2} <<-EOF
		testing the unset record subcommand
	EOF
	sync;sync
	} | tee -a ${RESULTS} 
	grep "testing the unset record subcommand" /tmp/mail_209.out > /dev/null 2>&1
	TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "mail${TX}209" $?
	rm -f /usr/spool/mail/${USER2} 
	rm -f /tmp/mail_209.out
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_210()
{
	LPTEST=mail${TX}210
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the (unset screen) mail subcommand" 
	cp /usr/share/lib/Mail.rc /usr/share/lib/Mail.rc.bak2
	echo "set screen=2" >> /usr/share/lib/Mail.rc
	echo "unset screen" > $HOME/.mailrc

	# send a few pieces of mail 
	for i in 1 2 3 4; do mail -s test$i ${USER} < /dev/null >/dev/null; done
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_210.out
	grep "test1" /tmp/mail_210.out > /dev/null 2>&1
	TT1="$?"
	grep "test2" /tmp/mail_210.out > /dev/null 2>&1
	TT2="$?"
	grep "test3" /tmp/mail_210.out > /dev/null 2>&1
	TT3="$?"
	grep "test4" /tmp/mail_210.out > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
	lputil verify "mail${TX}210" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_210.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_211()
{
	LPTEST=mail${TX}
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the unset toplines mail subcommand" 
	# send some mail so we can get into mail mode #

	echo "set toplines=3" > $HOME/.mailrc

	mail ${USER} <<-EOF
		testing the unset toplines subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}unset toplines
	${waitch}$mail_prompt${waitch}top
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_211.out
	grep "Message  1:" /tmp/mail_211.out > /dev/null 2>&1
	TT1="$?"
	grep "From root"  /tmp/mail_211.out > /dev/null 2>&1
	TT2="$?"
	grep "Received:"  /tmp/mail_211.out > /dev/null 2>&1
	TT3="$?"
	grep "Date:"  /tmp/mail_211.out > /dev/null 2>&1
	TT4="$?"
	grep "From:" /tmp/mail_211.out > /dev/null 2>&1
	TT5="$?"
	grep "To:" /tmp/mail_211.out > /dev/null 2>&1
	TT6="$?"
	grep "Subject:" /tmp/mail_211.out > /dev/null 2>&1
	TT7="$?"
	grep 'testing the "top" mail subcommand' /tmp/mail_211.out > /dev/null 2>&1
	TT8="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] && [ $TT6 -ne 0 ] && [ $TT7 -ne 0 ] && [ $TT8 -ne 0 ]
	lputil verify "mail${TX}211" $?
	rm -f  /usr/spool/mail/${USER} $HOME/.mailrc /tmp/mail_211.out
	echo;echo
}

mail_212()
{
	LPTEST=mail${TX}212
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the unset SHELL mail subcommand" 
	# send some mail first so we can get into mail mode #

	echo "set SHELL=/bin/sh" > $HOME/.mailrc

	mail ${USER} <<-EOF
		testing the unset SHELL subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}${mail_prompt}${waitch}shell
	${waitch}${sh_prompt}${waitch}set -o vi
	${waitch}${sh_prompt}${waitch}exit
	${waitch}${mail_prompt}${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_212.out
	grep -e "-o: bad option(s)" /tmp/mail_212.out > /dev/null 2>&1
	lputil verify "mail${TX}212" $?
	rm -f  /usr/spool/mail/${USER} $HOME/mbox /tmp/mail_212.out
	echo;echo
}
	
mail_213()
{
	LPTEST=mail${TX}213
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the unset PAGER mail subcommand" 
	# send some mail first so we can get into mail mode #

	echo "set PAGER=/usr/bin/more" > $HOME/.mailrc

	mail ${USER} <<-EOF
		testing the unset PAGER subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}unset PAGER
	${waitch}$mail_prompt${waitch}page
	${waitch}$EOF${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_213.out
	grep "^(EOF):"	/tmp/mail_213.out >/dev/null 2>&1
	TT1="$?"
	grep "testing the unset PAGER subcommand" /tmp/mail_213.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}213"  $?
	rm -f /tmp/mail_213.out /usr/spool/mail/${USER}
	echo;echo
}

mail_214()
{
	LPTEST=mail${TX}214
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the unset folder mail subcommand" 
	echo "set folder=/tmp/mail_letter" > $HOME/.mailrc
	
	# put some files in your folder directory #
	mkdir /tmp/mail_letter >/dev/null
	touch /tmp/mail_letter/letter1 /tmp/mail_letter/letter2 
	touch /tmp/mail_letter/letter3
	 
	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "unset folder" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}unset folder
	${waitch}$mail_prompt${waitch}folders
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_214.out
	grep 'No value set for "folder"' /tmp/mail_214.out >/dev/null 2>&1
	TT1="$?"
	lputil verify "mail${TX}214" $?
	rm -f $HOME/.mailrc
	rm -r /tmp/mail_letter >/dev/null 2>&1
	rm -f /usr/spool/mail/${USER} /tmp/mail_214.out		
	echo;echo
}

mail_215()
{
	LPTEST=mail${TX}215
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number   - $LPTEST"
	echo "$LPTEST FVT testing the bellmail command" 
	# send some mail with bellmail #
	bellmail ${USER} <<-EOF
		testing the bellmail command
	EOF
	sync;sync
	grep "testing the bellmail command" /usr/spool/mail/${USER} >/dev/null 2>&1
	lputil verify "mail${TX}215" $?
	rm -f /usr/spool/mail/${USER}
	echo;echo
}

mail_216()
{
	LPTEST=mail${TX}216
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number  - $LPTEST"
	echo "$LPTEST FVT testing the bellmail -d flag"
	mkdir /tmp/mail_216.out > /dev/null 2>&1
	bellmail -d /tmp/mail_216.out ${USER} <<-EOF
		testing the bellmail -d flag
	EOF
	sync;sync
	grep "testing the bellmail -d flag" /tmp/mail_216.out/${USER} > /dev/null 2>&1
	lputil verify "mail${TX}216" $?
	rm -r /tmp/mail_216.out
	echo;echo
}

mail_217()  
{
set -x
	LPTEST=mail${TX}217
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the bellmail -e flag"
	# remove the mailbox so that there is no mail #
	rm -f /usr/spool/mail/${USER}
	bellmail -e > /tmp/mail_217.out
	TT1="$?"

	# send some mail now #
	bellmail ${USER} <<-EOF
		testing the bellmail -e flag
	EOF
	sleep 2
	bellmail -e
	TT2="$?"
	grep "No mail" /tmp/mail_217.out > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 1 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ] 
	lputil verify "mail${TX}217" $?
	rm -f /tmp/mail_217.out
	echo;echo
}

mail_218()
{
	LPTEST=mail${TX}218
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	{
	echo "$LPTEST FVT testing the bellmail -f flag"
	# send some mail to the specified file first #
	mkdir /tmp/mail_218.out > /dev/null 2>&1
	bellmail -d /tmp/mail_218.out ${USER} <<-EOF
		testing the bellmail -f flag
	EOF
	sync;sync
	sleep 2
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail -f /tmp/mail_218.out/root
	${waitch}$bell_prompt${waitch}$test_prompt
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_218.out1
	grep "testing the bellmail -f flag" /tmp/mail_218.out1 > /dev/null 2>&1
	TT1="$?"
	[ $TT1 = 0 ] 
	lputil verify "mail${TX}218" $?
	rm -r /tmp/mail_218.out
	rm -f  /tmp/mail_218.out1
	echo;echo
}
	
mail_219()
{
	LPTEST=mail${TX}219
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	{
	echo "$LPTEST FVT testing the bellmail -p flag"
	# send some mail first #
	bellmail ${USER} <<-EOF
		test1 for testing the bellmail -p flag
	EOF
	sync;sync
	bellmail ${USER} <<-EOF
		test2 for testing the bellmail -p flag
	EOF
	sync;sync
	sleep 2
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail -p
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_219.out
	grep "test1 for testing the bellmail -p flag" /tmp/mail_219.out > /dev/null 2>&1
	TT1="$?"
	grep "test2 for testing the bellmail -p flag" /tmp/mail_219.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}219" $?
	rm -f /tmp/mail_219.out /usr/spool/mail/${USER}	
	echo;echo
}

mail_220()
{
	LPTEST=mail${TX}220
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the bellmail -q flag"
	# send some mail first #
	bellmail ${USER} <<-EOF
		test1 for testing the bellmail -q flag
	EOF
	sync;sync
	bellmail ${USER} <<-EOF
		test2 for testing the bellmail -q flag
	EOF
	{
	sync;sync
	sleep 2
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail   
	${waitch}$bell_prompt${waitch}
	${waitch}$bell_prompt${waitch}
	${waitch}$bell_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	} | tee /tmp/mail_220.out1
	sync;sync
	{
	sleep 2
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail -q
	${waitch}$bell_prompt${waitch}
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee /tmp/mail_220.out2
	# the fact that the above worked pretty much verifies the -q flag
	# but let's do some extra checking
	grep " quit" /tmp/mail_220.out1 > /dev/null 2>&1
	TT1="$?"
	grep " quit" /tmp/mail_220.out2 > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}220" $?
	rm -f /tmp/mail_220.* /usr/spool/mail/${USER}
	echo;echo
}

mail_221()
{
	LPTEST=mail${TX}221
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the bellmail -r flag"
	# send some mail first #
	bellmail ${USER} <<-EOF
		test1 for testing the bellmail -r flag
	EOF
	sync;sync
	bellmail ${USER} <<-EOF
		test2 for testing the bellmail -r flag
	EOF
	sync;sync
	sleep 2
	bellmail -r -p > /tmp/mail_221.out

	set `grep -n "test1 for testing the bellmail -r flag" /tmp/mail_221.out | sed 'y/:/ /'` ; TT1=$1
	set `grep -n "test2 for testing the bellmail -r flag" /tmp/mail_221.out | sed 'y/:/ /'`; TT2=$1
	[ $TT1 -le $TT2 ]
	lputil verify "mail${TX}221" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_221.out
	echo;echo
}

mail_222() 
{
	LPTEST=mail${TX}222
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the bellmail -t flag"
	# send some mail first #
	bellmail -t ${USER} ${USER2} <<-EOF
		test1 for testing the bellmail -t flag
	EOF
	sync;sync
	bellmail -p > /tmp/mail_222.out1
	grep "To: ${USER} ${USER2}" /tmp/mail_222.out1 > /dev/null 2>&1
	TT1="$?"

	# let's do some extra verification here by just sending mail
	# and ensuring that without the -t flag nothing is prefixed to message
	rm -f /usr/spool/mail/${USER} /usr/spool/mail/${USER2}
	bellmail ${USER} ${USER2} <<-EOF
		test2 for testing the bellmail -t flag
	EOF
	bellmail -p > /tmp/mail_222.out2
	grep "To: ${USER} ${USER2}" /tmp/mail_222.out2 > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}222" $?   
	rm -f /tmp/mail_222.* /usr/spool/mail/${USER} /usr/spool/mail/${USER2}
	echo;echo
}

mail_223()
{
	LPTEST=mail${TX}223
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with unknown address"
	bellmail cinderella <<-EOF > /tmp/mail_223.out  2>&1
		testing bellmail with an unknown user
	EOF
	sync;sync
	grep "bellmail: can't send to cinderella" /tmp/mail_223.out > /dev/null 2>&1
	TT1="$?"
	grep "Mail saved in dead.letter" /tmp/mail_223.out > /dev/null 2>&1
	TT2="$?"
	grep "testing bellmail with an unknown user" dead.letter > /dev/null 2>$1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}223" $?
	rm -f /tmp/mail_223.out  dead.letter
	echo;echo
}

mail_224()
{
	LPTEST=mail${TX}224
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (+) subcommand"
	# send some mail 
	bellmail ${USER} <<-EOF
		test1 for testing the bellmail (+) subcommand
	EOF
	sync;sync
	bellmail ${USER} <<-EOF
		test2 for testing the bellmail (+) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}+
	${waitch}$bell_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_224.out
	grep "test1 for testing the bellmail (+) subcommand" /tmp/mail_224.out > /dev/null 2>&1
	TT1="$?"
	grep "test2 for testing the bellmail (+) subcommand" /tmp/mail_224.out > /dev/null 2>&1
	TT2="$?"  
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}224" $?
	rm -f /tmp/mail_224.out /usr/spool/mail/${USER}
	echo;echo
}

mail_225()
{
	LPTEST=mail${TX}225
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (-) subcommand"
	# send some mail 
	bellmail ${USER} <<-EOF
		test1 for testing the bellmail (-) subcommand
	EOF
	sync;sync
	bellmail ${USER} <<-EOF
		test2 for testing the bellmail (-) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}+            
	${waitch}$bell_prompt${waitch}-
	${waitch}$bell_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_225.out
	TT1=`grep -c "test2 for testing the bellmail (-) subcommand" /tmp/mail_225.out`
	[ $TT1 = 2 ]
	lputil verify "mail${TX}225" $?
	rm -f /tmp/mail_225.out /usr/spool/mail/${USER}
	echo;echo
}

mail_226()
{
	LPTEST=mail${TX}226
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (-) subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		test1 for testing the bellmail (-) subcommand
	EOF
	sync;sync
	bellmail ${USER} <<-EOF
		test2 for testing the bellmail (-) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}-
	${waitch}$bell_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_226.out
	TT1=`grep -c "test2 for testing the bellmail (-) subcommand" /tmp/mail_226.out`
	grep "test1 for testing the bellmail (-) subcommand" /tmp/mail_226.out >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 2 ] && [ $TT2 -ne 0 ] 
	lputil verify "mail${TX}226" $?
	rm -f /tmp/mail_226.out /usr/spool/mail/${USER}
	echo;echo
}

mail_227()
{
	LPTEST=mail${TX}227
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (!) subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		testing the bellmail (!) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}!uname
	${waitch}$bell_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_227.out
	grep "^AIX" /tmp/mail_227.out > /dev/null 2>&1
	lputil verify "mail${TX}227" $?
	rm -f /tmp/mail_227.out /usr/spool/mail/${USER}
	echo;echo
} 

mail_228()  
{
	LPTEST=mail${TX}228
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (*) subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		testing the bellmail (*) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}*
	${waitch}$bell_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_228.out
	grep "usage" /tmp/mail_228.out >/dev/null 2>&1  
	TT1="$?"
	grep "q		quit" /tmp/mail_228.out >/dev/null 2>&1 
	TT2="$?"
	grep "x		exit without changing mail" /tmp/mail_228.out  >/dev/null 2>&1
	TT3="$?"
	grep "p		print" /tmp/mail_228.out  >/dev/null 2>&1
	TT4="$?"
	grep "s \[file\]	save (default mbox)" /tmp/mail_228.out  >/dev/null 2>&1 
	TT5="$?"
	grep "w \[file\]	same without header" /tmp/mail_228.out  >/dev/null 2>&1
	TT6="$?"
	grep -e "-		print previous" /tmp/mail_228.out  >/dev/null 2>&1
	TT7="$?"
	grep "d		delete" /tmp/mail_228.out  >/dev/null 2>&1 
	TT8="$?"
	grep "+		next (no delete)" /tmp/mail_228.out >/dev/null 2>&1 
	TT9="$?"
	grep "m \[user\]	mail to user" /tmp/mail_228.out >/dev/null 2>&1 
	TT10="$?"
	grep "! cmd		execute cmd" /tmp/mail_228.out >/dev/null 2>&1
	TT11="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] && [ $TT6 = 0 ] && [ $TT7 = 0 ] && [ $TT8 = 0 ] && [ $TT9 = 0 ] && [ $TT10 = 0 ] && [ $TT11 = 0 ]
	lputil verify "mail${TX}228" $?
	rm -f /tmp/mail_228.out /usr/spool/mail/${USER}
	echo;echo
}	
	
mail_229()  
{
	LPTEST=mail${TX}229
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (?) subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		testing the bellmail (?) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}?
	${waitch}$bell_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_229.out
	grep "q		quit" /tmp/mail_229.out >/dev/null 2>&1 
	TT2="$?"
	grep "x		exit without changing mail" /tmp/mail_229.out  >/dev/null 2>&1
	TT3="$?"
	grep "p		print" /tmp/mail_229.out  >/dev/null 2>&1
	TT4="$?"
	grep "s \[file\]	save (default mbox)" /tmp/mail_229.out  >/dev/null 2>&1 
	TT5="$?"
	grep "w \[file\]	same without header" /tmp/mail_229.out  >/dev/null 2>&1
	TT6="$?"
	grep -e "-		print previous" /tmp/mail_229.out  >/dev/null 2>&1
	TT7="$?"
	grep "d		delete" /tmp/mail_229.out  >/dev/null 2>&1 
	TT8="$?"
	grep "+		next (no delete)" /tmp/mail_229.out >/dev/null 2>&1 
	TT9="$?"
	grep "m \[user\]	mail to user" /tmp/mail_229.out >/dev/null 2>&1 
	TT10="$?"
	grep "! cmd		execute cmd" /tmp/mail_229.out >/dev/null 2>&1
	TT11="$?"
	[ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] && [ $TT6 = 0 ] && [ $TT7 = 0 ] && [ $TT8 = 0 ] && [ $TT9 = 0 ] && [ $TT10 = 0 ] && [ $TT11 = 0 ]
	lputil verify "mail${TX}229" $?
	rm -f /tmp/mail_229.out /usr/spool/mail/${USER}
	echo;echo
}	
	
mail_230()
{
	LPTEST=mail${TX}230
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (n) subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		test1 for testing the bellmail (n) subcommand
	EOF
	sync;sync
	bellmail ${USER} <<-EOF
		test2 for testing the bellmail (n) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}n
	${waitch}$bell_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_230.out
	grep "test1 for testing the bellmail (n) subcommand" /tmp/mail_230.out > /dev/null 2>&1
	lputil verify "mail${TX}230" $?
	rm -f /tmp/mail_230.out /usr/spool/mail/${USER}
	echo;echo
}

mail_231()
{
	LPTEST=mail${TX}231
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (<CR>) subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		test1 for testing the bellmail (<CR>) subcommand
	EOF
	sync;sync
	bellmail ${USER} <<-EOF
		test2 for testing the bellmail (<CR>) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}$test_prompt
	${waitch}$bell_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_231.out
	grep "test1 for testing the bellmail (<CR>) subcommand" /tmp/mail_231.out > /dev/null 2>&1
	lputil verify "mail${TX}231" $?
	rm -f /tmp/mail_231.out /usr/spool/mail/${USER}
	echo;echo
}

mail_232()
{
	LPTEST=mail${TX}232
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (^) subcommand"
	# send some mail 
	bellmail ${USER} <<-EOF
		test1 for testing the bellmail (^) subcommand
	EOF
	sync;sync
	bellmail ${USER} <<-EOF
		test2 for testing the bellmail (^) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}+            
	${waitch}$bell_prompt${waitch}^
	${waitch}$bell_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_232.out
	TT1=`grep -c "test2 for testing the bellmail (^) subcommand" /tmp/mail_232.out`
	[ $TT1 = 2 ]
	lputil verify "mail${TX}232" $?
	rm -f /tmp/mail_232.out /usr/spool/mail/${USER}
	echo;echo
}

mail_233()
{
	LPTEST=mail${TX}233
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (^) subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		test1 for testing the bellmail (^) subcommand
	EOF
	sync;sync
	bellmail ${USER} <<-EOF
		test2 for testing the bellmail (^) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}^
	${waitch}$bell_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_233.out
	TT1=`grep -c "test2 for testing the bellmail (^) subcommand" /tmp/mail_233.out`
	grep "test1 for testing the bellmail (^) subcommand" /tmp/mail_233.out >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 2 ] && [ $TT2 -ne 0 ] 
	lputil verify "mail${TX}233" $?
	rm -f /tmp/mail_233.out /usr/spool/mail/${USER}
	echo;echo
}

mail_234()
{
	LPTEST=mail${TX}234
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (d) subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		test1 for testing the bellmail (d) subcommand
	EOF
	sync;sync
	bellmail ${USER} <<-EOF
		test2 for testing the bellmail (d) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}d
	${waitch}$bell_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_234.out
	grep "test1 for testing the bellmail (d) subcommand" /tmp/mail_234.out > /dev/null 2>&1
	TT1="$?"
	grep "test2 for testing the bellmail (d) subcommand" /tmp/mail_234.out > /dev/null 2>&1
	TT2="$?"
	grep "test1 for testing the bellmail (d) subcommand" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	grep "test2 for testing the bellmail (d) subcommand" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 -ne 0 ]
	lputil verify "mail${TX}234" $?
	rm -f /tmp/mail_234.out /usr/spool/mail/${USER}
	echo;echo
}

mail_235()
{
	LPTEST=mail${TX}235
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (m) subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		testing the bellmail (m) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}m bugs 
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS}  
	grep "testing the bellmail (m) subcommand" /usr/spool/mail/bugs >/dev/null 2>&1
	lputil verify "mail${TX}235" $?
	rm -f /usr/spool/mail/${USER} /usr/spool/mail/bugs
	echo;echo
}

mail_236()
{
	LPTEST=mail${TX}236
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (p) subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		testing the bellmail (p) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}p
	${waitch}$bell_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	} | tee -a ${RESULTS} | tee /tmp/mail_236.out
	TT1=`grep -c "testing the bellmail (p) subcommand" /tmp/mail_236.out`
	[ $TT1 = 2 ]
	lputil verify "mail${TX}236" $?
	rm -f /tmp/mail_236.out /usr/spool/mail/${USER}
	echo;echo
}

mail_237() 
{
	LPTEST=mail${TX}237
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (q) subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		test1 for testing the bellmail (q) subcommand
	EOF
	sync;sync
	bellmail ${USER} <<-EOF
		test2 for testing the bellmail (q) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}d
	${waitch}$bell_prompt${waitch}q
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} 
	grep "test1 for testing the bellmail (q) subcommand" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "test2 for testing the bellmail (q) subcommand" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}237" $? 
	rm -f /usr/spool/mail/${USER}
	echo;echo
}

mail_238()
{
	LPTEST=mail${TX}238
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (s) subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		testing the bellmail (s) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}s
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} 
	grep "testing the bellmail (s) subcommand" $HOME/mbox > /dev/null 2>&1
	TT1="$?"
	grep "From" $HOME/mbox > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}238" $?       
	rm -f $HOME/mbox 
	echo;echo
}

mail_239()
{
	LPTEST=mail${TX}239
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (s file) subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		testing the bellmail (s file) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}s /tmp/mail_239.out
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} 
	grep "testing the bellmail (s file) subcommand" /tmp/mail_239.out > /dev/null 2>&1
	TT1="$?"
	grep "From" /tmp/mail_239.out > /dev/null 2>&1  
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] 
	lputil verify "mail${TX}239" $?       
	rm -f $HOME/mbox  /tmp/mail_239.out
	echo;echo
}

mail_240()
{
	LPTEST=mail${TX}240
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (w) subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		testing the bellmail (w) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}w
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS}
	grep "testing the bellmail (w) subcommand" $HOME/mbox >/dev/null 2>&1
	TT1="$?"
	grep "From" $HOME/mbox > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}240" $?
	rm -f $HOME/mbox
	echo;echo
}

mail_241()
{
	LPTEST=mail${TX}241
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (w file) subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		testing the bellmail (w file) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}w /tmp/mail_241.out
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS}
	grep "testing the bellmail (w file) subcommand" /tmp/mail_241.out >/dev/null 2>&1
	TT1="$?"
	grep "From" /tmp/mail_241.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}241" $?
	rm -f $HOME/mbox /tmp/mail_241.out
	echo;echo
}

	
mail_242() 
{
	LPTEST=mail${TX}242
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (x) subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		test1 for testing the bellmail (x) subcommand
	EOF
	sync;sync
	bellmail ${USER} <<-EOF
		test2 for testing the bellmail (x) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}d
	${waitch}$bell_prompt${waitch}x
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} 
	grep "test1 for testing the bellmail (x) subcommand" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "test2 for testing the bellmail (x) subcommand" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}242" $? 
	rm -f /usr/spool/mail/${USER}
	echo;echo
}

mail_243()
{
	LPTEST=mail${TX}243
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (y) subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		testing the bellmail (y) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}y
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS}
	grep "testing the bellmail (y) subcommand" $HOME/mbox >/dev/null 2>&1
	TT1="$?"
	grep "From" $HOME/mbox > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}243" $?
	rm -f $HOME/mbox
	echo;echo
}

mail_244()
{
	LPTEST=mail${TX}244
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with (y file) subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		testing the bellmail (y file) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}y /tmp/mail_244.out
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS}
	grep "testing the bellmail (y file) subcommand" /tmp/mail_244.out >/dev/null 2>&1
	TT1="$?"
	grep "From" /tmp/mail_244.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}244" $?
	rm -f $HOME/mbox /tmp/mail_244.out
	echo;echo
}

mail_245()
{
	LPTEST=mail${TX}245
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing bellmail with an incorrect subcommand"
	# send some mail
	bellmail ${USER} <<-EOF
		testing bellmail with an incorrect flag
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}bellmail
	${waitch}$bell_prompt${waitch}a
	${waitch}$bell_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_245.out
	grep "usage" /tmp/mail_245.out >/dev/null 2>&1  
	TT1="$?"
	grep "q		quit" /tmp/mail_245.out >/dev/null 2>&1 
	TT2="$?"
	grep "x		exit without changing mail" /tmp/mail_245.out  >/dev/null 2>&1
	TT3="$?"
	grep "p		print" /tmp/mail_245.out  >/dev/null 2>&1
	TT4="$?"
	grep "s \[file\]	save (default mbox)" /tmp/mail_245.out  >/dev/null 2>&1 
	TT5="$?"
	grep "w \[file\]	same without header" /tmp/mail_245.out  >/dev/null 2>&1
	TT6="$?"
	grep -e "-		print previous" /tmp/mail_245.out  >/dev/null 2>&1
	TT7="$?"
	grep "d		delete" /tmp/mail_245.out  >/dev/null 2>&1 
	TT8="$?"
	grep "+		next (no delete)" /tmp/mail_245.out >/dev/null 2>&1 
	TT9="$?"
	grep "m \[user\]	mail to user" /tmp/mail_245.out >/dev/null 2>&1 
	TT10="$?"
	grep "! cmd		execute cmd" /tmp/mail_245.out >/dev/null 2>&1
	TT11="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] && [ $TT6 = 0 ] && [ $TT7 = 0 ] && [ $TT8 = 0 ] && [ $TT9 = 0 ] && [ $TT10 = 0 ] && [ $TT11 = 0 ]
	lputil verify "mail${TX}245" $?
	rm -f /tmp/mail_245.out /usr/spool/mail/${USER}
	echo;echo
}	
	
mail_246()
{
	LPTEST=mail${TX}246
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the fmt command"
	echo "aaaaaaaaaaaaaaaaaaaa" > /tmp/mail_246.out
	echo "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbb" >> /tmp/mail_246.out
	echo "cccccccccc" >> /tmp/mail_246.out
	echo "ddddd" >> /tmp/mail_246.out

	fmt /tmp/mail_246.out > /tmp/mail_246.out1
	grep "aaaaaaaaaaaaaaaaaaaa bbbbbbbbbbbbbbbbbbbbbbbbbbbbbb cccccccccc ddddd" /tmp/mail_246.out1 > /dev/null 2>&1
	lputil verify "mail${TX}246" $?
	rm -f /tmp/mail_246.*
	echo;echo
}	
	
mail_247()
{
	LPTEST=mail${TX}247
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the fmt -width command"
	echo "aaaaaaaaaaaaaaaaaaaa" > /tmp/mail_247.out
	echo "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbb" >> /tmp/mail_247.out
	echo "cccccccccc" >> /tmp/mail_247.out
	echo "ddddd" >> /tmp/mail_247.out
	echo "eeeee" >> /tmp/mail_247.out

	fmt  -35 /tmp/mail_247.out > /tmp/mail_247.out1
	grep aaaaaaaaaaaaaaaaaaaa /tmp/mail_247.out1 > /dev/null 2>&1
	TT1="$?"
	grep  bbbbbbbbbbbbbbbbbbbbbbbbbbbbbb /tmp/mail_247.out1 >/dev/null 2>&1
	TT2="$?"
	grep "cccccccccc ddddd eeeee" /tmp/mail_247.out1 > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}247" $?
	rm -f /tmp/mail_247.*
	echo;echo
}

mail_248()
{
	LPTEST=mail${TX}248
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the vacation -I  command"
	# initialize the $HOME/.vacation.pag and $HOME/.vacation.dir
	# files with the vacation -I flag
	vacation -I
	ls $HOME/.vacation.pag > /dev/null 2>&1
	TT1="$?"
	ls $HOME/.vacation.dir > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}248" $?
	rm -f $HOME/.vacation.*      
	echo;echo
}

mail_249()
{
	LPTEST=mail${TX}249
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the vacation -I  command"
	# initialize the $HOME/.vacation.pag and $HOME/.vacation.dir
	# files with the vacation -I flag
	vacation -I
	ls $HOME/.vacation.pag > /dev/null 2>&1
	TT1="$?"
	ls $HOME/.vacation.dir > /dev/null 2>&1
	TT2="$?"

	# create the $HOME/.vacation.msg file
	echo "From: tester@mail.sh.file
Subject: testing the vacation command

I am on vacation until sometime next year when I feel like
coming back. If you have any questions, please contact my next of kin.

See ya,
root " > $HOME/.vacation.msg

	# set up the $HOME/.forward file
	echo "\0134${USER},\"|vacation ${USER}\"" > $HOME/.forward
	
	# now send yourself some mail 
	mail ${USER} <<-EOF
		test1 for testing the vacation command
	EOF
	sync;sync
	sleep 3
	
	# now let's see what's in our system mailbox 
	grep "test1 for testing the vacation command" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	grep "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT4="$?"
	
	# send some mail again because we want to be sure that it 
	# only sends the mail message once a week 
	
	mail ${USER} <<-EOF
		test2 for testing the vacation command
	EOF
	sync;sync
	sleep 3
	
	# make sure we only have one copy of the vacation message
	# in our system mailbox
	
	TT5=`grep -c "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER}`
	
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 1 ] 
	lputil verify "mail${TX}249" $?
	rm -f $HOME/.vacation.*  /usr/spool/mail/${USER} $HOME/.forward
	echo;echo
}
	
mail_250()
{
	LPTEST=mail${TX}250
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the enroll command"
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}enroll
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	ls /usr/spool/secretmail/${USER}.key > /dev/null 2>&1
	lputil verify "mail${TX}250" $?
	rm -f /usr/spool/secretmail/${USER}.key
	echo;echo
}

mail_251()
{
	LPTEST=mail${TX}251
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the xsend command"
	# send some secret mail
	xsend ${USER} > /tmp/mail_251.out 2>&1
	sync;sync
	grep "specified recipient is not enrolled (they must use the enroll command)" /tmp/mail_251.out > /dev/null 2>&1
	lputil verify "mail${TX}251" $?
	rm -f /tmp/mail_251.out
	echo;echo
}
	
mail_252()
{
	LPTEST=mail${TX}252
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the xsend command"
	# send some secret mail, but first enroll
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}enroll
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	xsend ${USER} <<-EOF
		testing the xsend command for sending secret mail
	EOF
	sync;sync
	sleep 2
	ls /usr/spool/secretmail/${USER}.0 > /dev/null 2>&1
	TT1="$?"
	grep "Your secret mail can be read on host `hostname` using the \"xget\" command" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}252" $?
	rm -f /usr/spool/mail/${USER} /usr/spool/secretmail/${USER}.*
	echo;echo
}

mail_253()
{
	LPTEST=mail${TX}253
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the xget (?) command"
	# send some secret mail, but first enroll
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}enroll
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	xsend ${USER} <<-EOF
		testing the xget (?) subcommand 
	EOF
	sync;sync
	sleep 5
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}xget 
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}?${waitch}?
	${waitch}?${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a {RESULTS} | tee /tmp/mail_253.out
	sleep 3
	grep "Commands are:" /tmp/mail_253.out > /dev/null 2>&1
	TT1="$?"
	grep "q       quit, leaving unread messages" /tmp/mail_253.out > /dev/null 2>&1
	TT2="$?"
	grep "n       delete current message and display next" /tmp/mail_253.out > /dev/null 2>&1
	TT3="$?"
	grep "d       same as above" /tmp/mail_253.out > /dev/null 2>&1
	TT4="$?"
	grep "<CR>    same as above" /tmp/mail_253.out > /dev/null 2>&1
	TT5="$?"
	grep "!       execute shell command" /tmp/mail_253.out > /dev/null 2>&1
	TT6="$?"
	grep "s       save message in the named file or mbox" /tmp/mail_253.out > /dev/null 2>&1
	TT7="$?"
	grep "w       same as above" /tmp/mail_253.out > /dev/null 2>&1
	TT8="$?"
	grep "?       print this list" /tmp/mail_253.out > /dev/null 2>&1
	TT9="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] && [ $TT6 = 0 ] && [ $TT7 = 0 ] && [ $TT8 = 0 ] && [ $TT9 = 0 ]
	lputil verify "mail${TX}253" $?
	rm -f /tmp/mail_253.out
	rm -f /usr/spool/secretmail/${USER}.* /usr/spool/mail/${USER}
	echo;echo 
}

mail_254()
{
	LPTEST=mail${TX}254
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the xget (!) command"
	# send some secret mail, but first enroll
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}enroll
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	xsend ${USER} <<-EOF
		testing the xget (!) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}xget
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}$bell_prompt${waitch}!uname
	${waitch}$bell_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_254.out
	grep ^AIX /tmp/mail_254.out > /dev/null 2>&1
	lputil verify "mail${TX}254" $?       
	rm -f /tmp/mail_254.out /usr/spool/mail/${USER}
	rm -f /usr/spool/secretmail/${USER}.*
	echo;echo
}

mail_255()
{
	LPTEST=mail${TX}255
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the xget (q) command"
	# send some secret mail, but first enroll
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}enroll
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	xsend ${USER} <<-EOF
		testing the xget (q) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}xget
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}$bell_prompt${waitch}q
	${waitch}${sh_prompt}${waitch}exit
	EOF
	} | tee -a ${RESULTS} | tee /tmp/mail_255.out
	ls /usr/spool/secretmail/root.0 > /dev/null 2>&1
	lputil verify "mail${TX}255" $?
	rm -f /usr/spool/secretmail/${USER}.* /usr/spool/mail/${USER}
	rm -f /tmp/mail_255.out
	echo;echo
}

mail_256()
{
	LPTEST=mail${TX}256
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the xget (n) command"
	# send some secret mail, but first enroll
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}enroll
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	xsend ${USER} <<-EOF
		test1 for testing the xget (n) subcommand
	EOF
	sync;sync
	xsend ${USER} <<-EOF
		test2 for testing the xget (n) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}xget
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}$bell_prompt${waitch}n
	${waitch}$bell_prompt${waitch}n
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_256.out
	grep "test1 for testing the xget (n) subcommand" /tmp/mail_256.out > /dev/null 2>&1
	TT1="$?"
	grep "test2 for testing the xget (n) subcommand" /tmp/mail_256.out > /dev/null 2>&1
	TT2="$?"
	ls /usr/spool/secretmail/root.[0-1] > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}256" $?
	rm -f /tmp/mail_256.out /usr/spool/mail/${USER}
	echo;echo
}

mail_257()
{
	LPTEST=mail${TX}257
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the xget (d) command"
	# send some secret mail, but first enroll
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}enroll
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	xsend ${USER} <<-EOF
		test1 for testing the xget (d) subcommand
	EOF
	sync;sync
	xsend ${USER} <<-EOF
		test2 for testing the xget (d) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}xget
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}$bell_prompt${waitch}d
	${waitch}$bell_prompt${waitch}d
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_257.out
	grep "test1 for testing the xget (d) subcommand" /tmp/mail_257.out > /dev/null 2>&1
	TT1="$?"
	grep "test2 for testing the xget (d) subcommand" /tmp/mail_257.out > /dev/null 2>&1
	TT2="$?"
	ls /usr/spool/secretmail/root.[0-1] > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}257" $?
	rm -f /tmp/mail_257.out /usr/spool/mail/${USER}
	echo;echo
}


mail_258()
{
	LPTEST=mail${TX}258
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the xget (<CR>) command"
	# send some secret mail, but first enroll
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}enroll
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	xsend ${USER} <<-EOF
		test1 for testing the xget (<CR>) subcommand
	EOF
	sync;sync
	xsend ${USER} <<-EOF
		test2 for testing the xget (<CR>) subcommand
	EOF
	sync;sync
	sleep 2
	{
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}xget
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}$bell_prompt${waitch}$test_prompt
	${waitch}$bell_prompt${waitch}$test_prompt
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_258.out
	grep "test1 for testing the xget (<CR>) subcommand" /tmp/mail_258.out > /dev/null 2>&1
	TT1="$?"
	grep "test2 for testing the xget (<CR>) subcommand" /tmp/mail_258.out > /dev/null 2>&1
	TT2="$?"
	ls /usr/spool/secretmail/root.[0-1] > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}258" $?
	rm -f /tmp/mail_258.out /usr/spool/mail/${USER}
	echo;echo
}

mail_259()
{
	LPTEST=mail${TX}259
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the xget (s) command"
	# send some secret mail, but first enroll
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}enroll
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	xsend ${USER} <<-EOF
		testing the xget (s) subcommand
	EOF
	sync;sync
	sleep 5
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}xget
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}$bell_prompt${waitch}s
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	grep "testing the xget (s) subcommand" mbox > /dev/null 2>&1
	TT1="$?"
	ls /usr/spool/secretmail/${USER}.0 > /dev/null 2>&1
	TT2="$?"
	grep "From" mbox > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}259" $?
	rm -f mbox  /usr/spool/mail/${USER}
	echo;echo
}

mail_260()
{
	LPTEST=mail${TX}260
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the xget (s file) command"
	# send some secret mail, but first enroll
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}enroll
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	xsend ${USER} <<-EOF
		testing the xget (s file) subcommand
	EOF
	sync;sync
	sleep 2
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}xget
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}$bell_prompt${waitch}s /tmp/mail_260.out
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	grep "testing the xget (s file) subcommand" /tmp/mail_260.out > /dev/null 2>&1
	TT1="$?"
	ls /usr/spool/secretmail/${USER}.0 > /dev/null 2>&1
	TT2="$?"
	grep "From" /tmp/mail_260.out > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}260" $?
	rm -f mbox  /usr/spool/mail/${USER} /tmp/mail_260.out
	echo;echo
}

mail_261()
{
	LPTEST=mail${TX}261
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the xget (w) command"
	# send some secret mail, but first enroll
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}enroll
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	xsend ${USER} <<-EOF
		testing the xget (w) subcommand
	EOF
	sync;sync
	sleep 2
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}xget
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}$bell_prompt${waitch}w
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	grep "testing the xget (w) subcommand" mbox > /dev/null 2>&1
	TT1="$?"
	ls /usr/spool/secretmail/${USER}.0 > /dev/null 2>&1
	TT2="$?"
	grep "From" mbox > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}261" $?
	rm -f mbox /usr/spool/mail/${USER}
	echo;echo
}

mail_262()
{
	LPTEST=mail${TX}262
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the xget (w file) command"
	# send some secret mail, but first enroll
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}enroll
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	xsend ${USER} <<-EOF
		testing the xget (w file) subcommand
	EOF
	sync;sync
	sleep 2
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}xget
	${waitch}Enter encryption key: ${waitch}abcdefg
	${waitch}$bell_prompt${waitch}w /tmp/mail_262.out
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	grep "testing the xget (w file) subcommand" /tmp/mail_262.out > /dev/null 2>&1
	TT1="$?"
	ls /usr/spool/secretmail/${USER}.0 > /dev/null 2>&1
	TT2="$?"
	grep "From" /tmp/mail_262.out > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}262" $?
	rm -f mbox  /usr/spool/mail/${USER} /tmp/mail_262.out
	echo;echo
}

mail_263()
{
	LPTEST=mail${TX}263
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the from command"
	# send some mail from various users
	/usr/lib/sendmail -f sleepy ${USER} <<-EOF
		Hi, snow white!
	EOF
	sync;sync
	/usr/lib/sendmail -f doc ${USER} <<-EOF
		Hi, snow white!
	EOF
	sync;sync
	sleep 2
	from > /tmp/mail_263.out
	grep "From: sleepy" /tmp/mail_263.out > /dev/null 2>&1
	TT1="$?"
	grep "From: doc" /tmp/mail_263.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] 
	lputil verify "mail${TX}263" $?
	rm -f /tmp/mail_263.out /usr/spool/mail/${USER}
	echo;echo
}

mail_264()
{
	LPTEST=mail${TX}264
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the from -s command"
	# send some mail from various users
	/usr/lib/sendmail -f sleepy ${USER} <<-EOF
		Hi, snow white!
	EOF
	sync;sync
	/usr/lib/sendmail -f doc ${USER} <<-EOF
		Hi, snow white!
	EOF
	sync;sync
	from -s sleepy > /tmp/mail_264.out1
	from -s doc > /tmp/mail_264.out2
	from -s grumpy > /tmp/mail_264.out3
	grep "From: sleepy" /tmp/mail_264.out1 > /dev/null 2>&1
	TT1="$?"
	grep "From: doc" /tmp/mail_264.out1 >/dev/null 2>&1
	TT2="$?"
	grep "From: sleepy" /tmp/mail_264.out2 > /dev/null 2>&1
	TT3="$?"
	grep "From: doc" /tmp/mail_264.out2 > /dev/null 2>&1
	TT4="$?"
	TT5=`ls -l /tmp/mail_264.out3 | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 -ne 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ]
	lputil verify "mail${TX}264" $?
	rm -f /tmp/mail_264.*  /usr/spool/mail/${USER}
	echo;echo
}

mail_265()
{
	LPTEST=mail${TX}265
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the from <user> command"
	# send some mail from various users
	/usr/lib/sendmail -f sleepy ${USER} <<-EOF
		Hi, snow white!
	EOF
	sync;sync
	/usr/lib/sendmail -f doc ${USER2} <<-EOF
		Hi, snow white!
	EOF
	sync;sync
	from ${USER} > /tmp/mail_265.out1
	from ${USER2} > /tmp/mail_265.out2
	grep "From: sleepy" /tmp/mail_265.out1 > /dev/null 2>&1
	TT1="$?"
	grep "From: doc" /tmp/mail_265.out1 > /dev/null 2>&1
	TT2="$?"
	grep "From: sleepy" /tmp/mail_265.out2 > /dev/null 2>&1
	TT3="$?"
	grep "From: doc" /tmp/mail_265.out2 > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 -ne 0 ] && [ $TT4 = 0 ]
	lputil verify "mail${TX}265" $?
	rm -f /tmp/mail_265.* /usr/spool/mail/${USER} /usr/spool/mail/${USER2}
	echo;echo
}

mail_266()
{
	LPTEST=mail${TX}266
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the from -d command"
	# send some mail from various users to a different mail spool
	# directory
	mkdir /tmp/mail_266.out > /dev/null 2>&1
	bellmail -d /tmp/mail_266.out ${USER} <<-EOF
		From: cinderella
		testing the from -d flag
	EOF
	sync;sync
	from -d /tmp/mail_266.out > /tmp/mail_266.out1
	grep "From: cinderella" /tmp/mail_266.out1 > /dev/null 2>&1
	lputil verify "mail${TX}266" $?
	rm -r /tmp/mail_266.out   > /dev/null 2>&1
	rm -f /tmp/mail_266.out1
	echo;echo
}

mail_267()
{
	LPTEST=mail${TX}267
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the bugfiler command"
	# set up the alias file
	cp /etc/aliases /etc/aliases.bak
	echo "bugs:\"|/usr/lib/bugfiler\"" >> /etc/aliases
	/usr/lib/sendmail -bi >/dev/null 2>&1
	
	#send bug report
	mail bugs < bugreport
	sleep 5
	grep "Received bug report." /usr/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "Subject: Short summary of the problem" /u/bugs/mail/games/1 > /dev/null 2>&1
	TT2="$?"
	grep "games/1    <source file> AIX4.1"	/u/bugs/mail/summary > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}267" $?
	rm -r /u/bugs/mail > /dev/null 2>&1
	rm -f /usr/spool/mail/${USER}
	cp /etc/aliases.bak /etc/aliases
	/usr/lib/sendmail -bi > /dev/null 2>&1
	rm /etc/aliases.bak
	echo;echo
}

mail_268()
{
	LPTEST=mail${TX}268
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the bugfiler <maildir> command"
	# set up the alias file
	cp /etc/aliases /etc/aliases.bak
	echo "bugs:\"|/usr/lib/bugfiler /u/bugs/bugsmail\"" >> /etc/aliases
	/usr/lib/sendmail -bi >/dev/null 2>&1
	
	#send bug report
	mail bugs < bugreport
	sleep 5
	grep "Received bug report." /usr/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "Subject: Short summary of the problem" /u/bugs/bugsmail/games/1 > /dev/null 2>&1
	TT2="$?"
	grep "games/1    <source file> AIX4.1"	/u/bugs/bugsmail/summary > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}268" $?
	rm -r /u/bugs/bugsmail > /dev/null 2>&1
	rm -f /usr/spool/mail/${USER}
	cp /etc/aliases.bak /etc/aliases
	/usr/lib/sendmail -bi > /dev/null 2>&1
	rm /etc/aliases.bak
	echo;echo
}

mail_269()
{
	LPTEST=mail${TX}269
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the bugfiler -m command"
	# set up the alias file
	cp /etc/aliases /etc/aliases.bak
	echo "bugs:\"|/usr/lib/bugfiler -m 0666\"" >> /etc/aliases
	/usr/lib/sendmail -bi >/dev/null 2>&1
	
	#send bug report
	mail bugs < bugreport
	sleep 5
	grep "Received bug report." /usr/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "Subject: Short summary of the problem" /u/bugs/mail/games/1 > /dev/null 2>&1
	TT2="$?"
	grep "games/1    <source file> AIX4.1"	/u/bugs/mail/summary > /dev/null 2>&1
	TT3="$?"
	ls -l /u/bugs/mail/games/1 | awk '{ print $1 }' > /tmp/mail_269.out
	grep -e "-rw-rw-rw-" /tmp/mail_269.out > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
	lputil verify "mail${TX}269" $?
	rm -r /u/bugs/mail > /dev/null 2>&1
	rm -f /usr/spool/mail/${USER} /tmp/mail_269.out
	cp /etc/aliases.bak /etc/aliases
	/usr/lib/sendmail -bi > /dev/null 2>&1
	rm /etc/aliases.bak
	echo;echo
}

mail_270()
{
	LPTEST=mail${TX}270
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the bugfiler -b command"
	# set up the alias file
	cp /etc/aliases /etc/aliases.bak
	echo "bugs:\"|/usr/lib/bugfiler -b ${USER2}\"" >> /etc/aliases
	/usr/lib/sendmail -bi >/dev/null 2>&1
	
	#send bug report
	mail bugs < bugreport
	sleep 5
	grep "Received bug report." /usr/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "Subject: Short summary of the problem" /u/${USER2}/mail/games/1 > /dev/null 2>&1
	TT2="$?"
	grep "games/1    <source file> AIX4.1"	/u/${USER2}/mail/summary > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] 
	lputil verify "mail${TX}270" $?
	rm -r /u/${USER2}/mail > /dev/null 2>&1
	rm -f /usr/spool/mail/${USER}
	cp /etc/aliases.bak /etc/aliases
	/usr/lib/sendmail -bi > /dev/null 2>&1
	rm /etc/aliases.bak
	echo;echo
}

mail_271()
{
        LPTEST=mail${TX}271
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number   - $LPTEST"
        {
        echo "$LPTEST FVT testing the set mailbox mail subcommand"
	echo "set mailbox=/tmp/mail_271.mbox" > $HOME/.mailrc
	

        echo "From root Fri Feb 14 10:03:36 1992
Received: by mickey.mouse.disney (AIX 3.2/UCB 5.64/4.03)
          id AA13448; Fri, 14 Feb 1992 10:03:36 -0600
Date: Fri, 14 Feb 1992 10:03:36 -0600
From: minnie
To: mickey
Subject: valentines

Happy Valentines day!!" > /tmp/mail_271.mbox 
	sync;sync

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee /tmp/mail_271.out
	grep "Happy Valentines day!!" /tmp/mail_271.out > /dev/null 2>&1
        lputil verify "mail${TX}271" $?
        rm -f $HOME/.mailrc /tmp/mail_271.mbox /tmp/mail_271.out
        echo;echo
}

mail_272()
{
        LPTEST=mail${TX}272
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number   - $LPTEST"
        {
        echo "$LPTEST FVT testing the set mailbox mail subcommand"

	export mailbox=/tmp/mail_272.mbox

        echo "From root Fri Feb 14 10:03:36 1992
Received: by mickey.mouse.disney (AIX 3.2/UCB 5.64/4.03)
          id AA13448; Fri, 14 Feb 1992 10:03:36 -0600
Date: Fri, 14 Feb 1992 10:03:36 -0600
From: minnie
To: mickey
Subject: valentines

Happy Valentines day!!" > /tmp/mail_272.mbox 
	sync;sync

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee /tmp/mail_272.out
	grep "Happy Valentines day!!" /tmp/mail_272.out > /dev/null 2>&1
        lputil verify "mail${TX}272" $?
        rm -f /tmp/mail_272.mbox /tmp/mail_272.out
	unset mailbox
        echo;echo
}

mail_273()
{
        LPTEST=mail${TX}273
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number   - $LPTEST"
        {
        echo "$LPTEST FVT testing the set mailbox mail subcommand"

	export MAILBOX=/tmp/mail_273.mbox

        echo "From root Fri Feb 14 10:03:36 1992
Received: by mickey.mouse.disney (AIX 3.2/UCB 5.64/4.03)
          id AA13448; Fri, 14 Feb 1992 10:03:36 -0600
Date: Fri, 14 Feb 1992 10:03:36 -0600
From: minnie
To: mickey
Subject: valentines

Happy Valentines day!!" > /tmp/mail_273.mbox 
	sync;sync

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee /tmp/mail_273.out
	grep "Happy Valentines day!!" /tmp/mail_273.out > /dev/null 2>&1
        lputil verify "mail${TX}273" $?
        rm -f /tmp/mail_273.mbox /tmp/mail_273.out
	unset MAILBOX
        echo;echo
}

mail_274()
{
        LPTEST=mail${TX}274
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number   - $LPTEST"
        {
        echo "$LPTEST FVT testing the unset mailbox mail subcommand"
	echo "set mailbox=/tmp/mail_274.mbox" > $HOME/.mailrc

        echo "From root Fri Feb 14 10:03:36 1992
Received: by mickey.mouse.disney (AIX 3.2/UCB 5.64/4.03)
          id AA13448; Fri, 14 Feb 1992 10:03:36 -0600
Date: Fri, 14 Feb 1992 10:03:36 -0600
From: minnie
To: mickey
Subject: valentines

Happy Valentines day!!" > /tmp/mail_274.mbox 
	sync;sync

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}unset mailbox
	${waitch}$mail_prompt${waitch}file %
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee /tmp/mail_274.out
	grep "/var/spool/mail/${USER}: No such file or directory" /tmp/mail_274.out > /dev/null 2>&1
        lputil verify "mail${TX}274" $?
        rm -f /tmp/mail_274.mbox /tmp/mail_274.out
	rm -f $HOME/.mailrc
        echo;echo
}


mail_275()
{
        LPTEST=mail${TX}275
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number   - $LPTEST"
        {
        echo "$LPTEST FVT testing the set mailbox mail subcommand"


	# send some mail to get in mail mode #
	mail -s "mailbox test" ${USER} <<-EOF
		testing the set mailbox command
	EOF
	sync;sync

	# put some mail in the alternative mailbox #

        echo "From root Fri Feb 14 10:03:36 1992
Received: by mickey.mouse.disney (AIX 3.2/UCB 5.64/4.03)
          id AA13448; Fri, 14 Feb 1992 10:03:36 -0600
Date: Fri, 14 Feb 1992 10:03:36 -0600
From: minnie
To: mickey
Subject: valentines

Happy Valentines day!!" > /tmp/mail_275.mbox 
	sync;sync

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}set mailbox=/tmp/mail_275.mbox
	${waitch}$mail_prompt${waitch}file %
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee /tmp/mail_275.out
	grep '"/tmp/mail_275.mbox": 1 message' /tmp/mail_275.out > /dev/null 2>&1
	TT1="$?"
	grep "Happy Valentines day!!" /tmp/mail_275.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]	
        lputil verify "mail${TX}275" $?
        rm -f /tmp/mail_275.mbox /tmp/mail_275.out
        echo;echo
}


mail_276()
{
        LPTEST=mail${TX}276
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number   - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~v editing subcommand"

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER}
	*Subject: *testing mail
	*$test_prompt*~v
	*~*i hello joy 
	* *nothing
	* * :wq
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "hello joy" /var/spool/mail/${USER} > /dev/null 2>&1
        lputil verify "mail${TX}276" $?
        rm -f /usr/spool/mail/${USER}
        echo;echo
}

mail_277()
{
        LPTEST=mail${TX}277
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number   - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~v editing subcommand"

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER}
	*Subject: *testing mail
	*$test_prompt*~v
	*~*i hello joy 
	* *nothing
	* * :wq
	* *testing the ~v editing subcommand
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "hello joy" /var/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "testing the ~v editing subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
        lputil verify "mail${TX}277" $?
        rm -f /var/spool/mail/${USER}
        echo;echo
}

mail_278()
{
        LPTEST=mail${TX}278
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~q editing subcommand"
	# put something in dead.letter, so we can later test to ensure #
	# that the contents of dead.letter is overwritten when using ~q #
	echo "some stuff in dead.letter" > $HOME/dead.letter

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER}
	*Subject: *testing mail
	*$test_prompt*testing the ~q editing subcommand
	*$test_prompt*~q
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_278.out
	grep "(Last Interrupt -- letter saved in $HOME/dead.letter)" /tmp/mail_278.out> /dev/null 2>&1
	TT1="$?"
	grep "testing the ~q editing subcommand" $HOME/dead.letter >/dev/null 2>&1
	TT2="$?"
	grep "some stuff in dead.letter" $HOME/dead.letter > /dev/null 2>&1
	TT3="$?"
	grep "testing the ~q editing subcommand" /var/spool/mail/${USER} >/dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ] && [ $TT4 -ne 0 ]
        lputil verify "mail${TX}278" $?
        rm -f /var/spool/mail/${USER}
	rm -f /tmp/mail_278.out $HOME/dead.letter
        echo;echo
}

mail_279()
{
        LPTEST=mail${TX}279
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~q editing subcommand"
	echo "set nosave" > $HOME/.mailrc

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER}
	*Subject: *testing mail
	*$test_prompt*testing the ~q editing subcommand
	*$test_prompt*~q
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_279.out
	grep "(Last Interrupt -- letter saved in dead.letter)" /tmp/mail_279.out> /dev/null 2>&1
	TT1="$?"
	grep "testing the ~q editing subcommand" $HOME/dead.letter >/dev/null 2>&1
	TT2="$?"
	grep "testing the ~q editing subcommand" /var/spool/mail/${USER} >/dev/null 2>&1
	TT3="$?"
	[ $TT1 -ne 0 ] && [ $TT2 -ne 0 ] && [ $TT3 -ne 0 ]
        lputil verify "mail${TX}279" $?
        rm -f /var/spool/mail/${USER}
	rm -f /tmp/mail_279.out $HOME/.mailrc
        echo;echo
}

mail_280()
{
        LPTEST=mail${TX}280
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~p editing subcommand"
	echo "this subcommand prints message entered so far" >/tmp/mail_280.file

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER}
	*Subject: *testing mail
	*$test_prompt*~r /tmp/mail_280.file
	*$test_prompt*testing the ~p editing subcommand
	*$test_prompt*~p
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_280.out
	grep "this subcommand prints message entered so far" /tmp/mail_280.out > /dev/null 2>&1
	TT1="$?"
	TT2=`grep -c "testing the ~p editing subcommand" /tmp/mail_280.out`
	grep "To: ${USER}" /tmp/mail_280.out > /dev/null 2>&1
	TT3="$?"
	TT4=`grep -c "Subject: testing mail" /tmp/mail_280.out`
	[ $TT1 = 0 ] && [ $TT2 = 3 ] && [ $TT3 = 0 ] && [ $TT4 = 2 ]
        lputil verify "mail${TX}280" $?
        rm -f /var/spool/mail/${USER}
	rm -f /tmp/mail_280.out /tmp/mail_280.file
        echo;echo
}

mail_281()
{
        LPTEST=mail${TX}281
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~:<mail command> editing subcommand"

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER}
	*Subject: *testing mail
	*$test_prompt*~:folder 
	*$test_prompt*testing the ~: editing subcommand
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_281.out
	grep "\"/var/spool/mail/${USER}\": 0 messages" /tmp/mail_281.out > /dev/null 2>&1
	TT1="$?"
	grep "\"/var/spool/mail/${USER}\": 0 messages" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "testing the ~: editing subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 = 0 ]
        lputil verify "mail${TX}281" $?
        rm -f /var/spool/mail/${USER}
	rm -f /tmp/mail_281.out 
        echo;echo
}

mail_282()
{
        LPTEST=mail${TX}282
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~h editing subcommand"

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail bu   
	*Subject: *testing mail
	*$test_prompt*~h 
	*To: bu*gs
	*Subject: testing mail* subcommands
	*Cc: *${USER}
	*Bcc: *$test_prompt
	*$test_prompt*test for the ~h editing subcommand
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_282.out
	grep "To: bugs" /var/spool/mail/bugs >/dev/null 2>&1
	TT1="$?"
	grep "Subject: testing mail subcommands" /var/spool/mail/bugs >/dev/null 2>&1
	TT2="$?" 
	grep "test for the ~h editing subcommand" /var/spool/mail/${USER} >/dev/null 2>&1
	TT3="$?"
	grep "test for the ~h editing subcommand" /var/spool/mail/bugs >/dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
        lputil verify "mail${TX}282" $?
        rm -f /var/spool/mail/${USER} /var/spool/mail/bugs
	rm -f /tmp/mail_282.out 
        echo;echo
}

mail_283()
{
        LPTEST=mail${TX}283
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~h editing subcommand"

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail buggs
	*Subject: *testing mail
	*$test_prompt*~h 
	*To: buggs*s
	*Subject: testing mail*no mail
	*Cc: *$test_prompt
	*Bcc: *$test_prompt
	*$test_prompt*test for the ~h editing subcommand
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_283.out
	grep "To: bugs" /var/spool/mail/bugs >/dev/null 2>&1
	TT1="$?"
	grep "Subject: testing no mail" /var/spool/mail/bugs >/dev/null 2>&1
	TT2="$?" 
	grep "test for the ~h editing subcommand" /var/spool/mail/bugs >/dev/null 2>&1
	TT3="$?"
	grep "test for the ~h editing subcommand" /var/spool/mail/bugs >/dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
        lputil verify "mail${TX}283" $?
        rm -f /var/spool/mail/bugs
	rm -f /tmp/mail_283.out 
        echo;echo
}

mail_284()
{
        LPTEST=mail${TX}284
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~h editing subcommand"

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail buggs
	*Subject: *testing mail
	*$test_prompt*~h 
	*To: buggs*s
	*Subject: testing mail*no mail
	*Cc: *$test_prompt
	*Bcc: *$test_prompt
	*$test_prompt*test for the ~h editing subcommand
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} 
	grep "To: bugs" /var/spool/mail/bugs >/dev/null 2>&1
	TT1="$?"
	grep "Subject: testing no mail" /var/spool/mail/bugs >/dev/null 2>&1
	TT2="$?" 
	[ $TT1 = 0 ] && [ $TT2 = 0 ] 
        lputil verify "mail${TX}284" $?
        rm -f /var/spool/mail/bugs
        echo;echo
}

mail_285()
{
        LPTEST=mail${TX}285
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~t editing subcommand"

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*~t bugs
	*$test_prompt*test for the ~t editing subcommand
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "test for the ~t editing subcommand" /var/spool/mail/bugs >/dev/null 2>&1
	TT1="$?"
	grep "test for the ~t editing subcommand" /var/spool/mail/${USER} >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
        lputil verify "mail${TX}285" $?
        rm -f /var/spool/mail/bugs
        echo;echo
}

mail_286()
{
        LPTEST=mail${TX}286
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~s editing subcommand"

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*~s test editing subcommand
	*$test_prompt*test for the ~t editing subcommand
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "test for the ~t editing subcommand" /var/spool/mail/${USER} >/dev/null 2>&1
	TT1="$?"
	grep "Subject: test editing subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
        lputil verify "mail${TX}286" $?
        rm -f /var/spool/mail/${USER}
        echo;echo
}

mail_287()
{
        LPTEST=mail${TX}287
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~c editing subcommand"

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*test for the ~c editing subcommand
	*$test_prompt*~c bugs
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "test for the ~c editing subcommand" /var/spool/mail/${USER} >/dev/null 2>&1
	TT1="$?"
	grep "test for the ~c editing subcommand" /var/spool/mail/bugs >/dev/null 2>&1
	TT2="$?"
	grep "Cc: bugs" /var/spool/mail/bugs > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] 
        lputil verify "mail${TX}287" $?
        rm -f /var/spool/mail/${USER} /var/spool/mail/bugs
        echo;echo
}

mail_288()
{
        LPTEST=mail${TX}288
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~b editing subcommand"

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*test for the ~b editing subcommand
	*$test_prompt*~b bugs
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "test for the ~b editing subcommand" /var/spool/mail/${USER} >/dev/null 2>&1
	TT1="$?"
	grep "test for the ~b editing subcommand" /var/spool/mail/bugs >/dev/null 2>&1
	TT2="$?"
	grep "Bcc: bugs" /var/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	grep "Bcc: bugs" /var/spool/mail/bugs > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ] && [ $TT4 -ne 0 ]
        lputil verify "mail${TX}288" $?
        #rm -f /var/spool/mail/${USER} /var/spool/mail/bugs
        echo;echo
}

mail_289()
{
        LPTEST=mail${TX}289
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~d editing subcommand"
	echo "this is the contents of dead.letter" > $HOME/dead.letter

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*test for the ~d editing subcommand
	*$test_prompt*~d
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "test for the ~d editing subcommand" /var/spool/mail/${USER} >/dev/null 2>&1
	TT1="$?"
	grep "this is the contents of dead.letter" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "end of letter" /var/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] 
        lputil verify "mail${TX}289" $?
        rm -f /var/spool/mail/${USER}
        echo;echo
}

mail_290()
{
        LPTEST=mail${TX}290
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~r editing subcommand"
	echo "this is the contents of /tmp/mail_290.file" > /tmp/mail_290.file 

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*test for the ~r editing subcommand
	*$test_prompt*~r /tmp/mail_290.file
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "test for the ~r editing subcommand" /var/spool/mail/${USER} >/dev/null 2>&1
	TT1="$?"
	grep "this is the contents of /tmp/mail_290.file" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "end of letter" /var/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] 
        lputil verify "mail${TX}290" $?
        rm -f /var/spool/mail/${USER}
        echo;echo
}

mail_291()
{
        LPTEST=mail${TX}291
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~f editing subcommand"
	# first send a few pieces of mail #
	echo "ignore Subject" > $HOME/.mailrc
	mail -s test_291 ${USER} <<-EOF
		test1 for testing the ~f editing subcommand
	EOF
	sync;sync
	sleep 2

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail 
	*$mail_prompt*mail bugs
	*Subject: *testing mail
	*$test_prompt*test for the ~f editing subcommand
	*$test_prompt*~f 
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*$mail_prompt*quit
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "test for the ~f editing subcommand" /var/spool/mail/bugs >/dev/null 2>&1
	TT1="$?"
	grep "test1 for testing the ~f editing subcommand" /var/spool/mail/bugs > /dev/null 2>&1
	TT2="$?"
	grep "end of letter" /var/spool/mail/bugs > /dev/null 2>&1
	TT3="$?"
	grep "Subject: test_291" /var/spool/mail/bugs > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 -ne 0 ]
        lputil verify "mail${TX}291" $?
        rm -f /var/spool/mail/${USER} /var/spool/mail/bugs $HOME/.mailrc
        echo;echo
}

mail_292()
{
        LPTEST=mail${TX}292
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~f editing subcommand"
	# first send a few pieces of mail #
	echo "ignore Subject" > $HOME/.mailrc
	mail -s test_292 ${USER} <<-EOF
		test1 for testing the ~f editing subcommand
	EOF
	sync;sync
	mail -s test_292_2 ${USER} <<-EOF
		test2 for testing the ~f editing subcommand
	EOF
	sync;sync
	sleep 2

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail 
	*$mail_prompt*mail bugs
	*Subject: *testing mail
	*$test_prompt*test for the ~f editing subcommand
	*$test_prompt*~f 1-2 
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*$mail_prompt*quit
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "test for the ~f editing subcommand" /var/spool/mail/bugs >/dev/null 2>&1
	TT1="$?"
	grep "test1 for testing the ~f editing subcommand" /var/spool/mail/bugs > /dev/null 2>&1
	TT2="$?"
	grep "test2 for testing the ~f editing subcommand" /var/spool/mail/bugs > /dev/null 2>&1
	TT3="$?"
	grep "end of letter" /var/spool/mail/bugs > /dev/null 2>&1
	TT4="$?"
	grep "Subject: test_292" /var/spool/mail/bugs > /dev/null 2>&1
	TT5="$?"
	grep "Subject: test_292_2" /var/spool/mail/bugs > /dev/null 2>&1
	TT6="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 -ne 0 ] && [ $TT6 -ne 0 ]
        lputil verify "mail${TX}292" $?
        rm -f /var/spool/mail/${USER} /var/spool/mail/bugs $HOME/.mailrc
        echo;echo
}

mail_293()
{
        LPTEST=mail${TX}293
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~m editing subcommand"
	# first send a few pieces of mail #
	echo "set indentprefix=>>>\nignore Subject" > $HOME/.mailrc
	mail -s test_293 ${USER} <<-EOF
		test1 for testing the ~m editing subcommand
	EOF
	sync;sync
	sleep 2

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail 
	*$mail_prompt*mail bugs
	*Subject: *testing mail
	*$test_prompt*test for the ~m editing subcommand
	*$test_prompt*~m
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*$mail_prompt*quit
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "^test for the ~m editing subcommand" /var/spool/mail/bugs >/dev/null 2>&1
	TT1="$?"
	grep "^>>>test1 for testing the ~m editing subcommand" /var/spool/mail/bugs > /dev/null 2>&1
	TT2="$?"
	grep "^end of letter" /var/spool/mail/bugs > /dev/null 2>&1
	TT3="$?"
	grep "^>>>Subject: test_293" /var/spool/mail/bugs > /dev/null 2>&1
	TT4="$?" 
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 -ne 0 ]
        lputil verify "mail${TX}293" $?
        rm -f /var/spool/mail/${USER} /var/spool/mail/bugs $HOME/.mailrc
        echo;echo
}

mail_294()
{
        LPTEST=mail${TX}294
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~m editing subcommand"
	# first send a few pieces of mail #
	mail ${USER} <<-EOF
		test1 for testing the ~m editing subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the ~m editing subcommand
	EOF
	sync;sync
	sleep 2

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail 
	*$mail_prompt*mail bugs
	*Subject: *testing mail
	*$test_prompt*test for the ~m editing subcommand
	*$test_prompt*~m 1-2 
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*$mail_prompt*quit
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "^test for the ~m editing subcommand" /var/spool/mail/bugs >/dev/null 2>&1
	TT1="$?"
	grep "^	test1 for testing the ~m editing subcommand" /var/spool/mail/bugs > /dev/null 2>&1
	TT2="$?"
	grep "^	test2 for testing the ~m editing subcommand" /var/spool/mail/bugs > /dev/null 2>&1
	TT3="$?"
	grep "end of letter" /var/spool/mail/bugs > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
        lputil verify "mail${TX}294" $?
	rm -f /var/spool/mail/${USER} /var/spool/mail/bugs > /dev/null 2>&1
	echo;echo
}

mail_295()
{
        LPTEST=mail${TX}295
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~w editing subcommand"

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*test for the ~w editing subcommand
	*$test_prompt*~w  /tmp/mail_295.file
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "test for the ~w editing subcommand" /var/spool/mail/${USER} >/dev/null 2>&1
	TT1="$?"
	grep "end of letter" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "test for the ~w editing subcommand" /tmp/mail_295.file > /dev/null 2>&1
	TT3="$?"
	grep "end of letter" /tmp/mail_295.file > /dev/null 2>&1
	TT4="$?" 
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 -ne 0 ]
        lputil verify "mail${TX}295" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_295.file
	echo;echo
}

mail_296()
{
        LPTEST=mail${TX}296
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~! editing subcommand"

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*test for the ~! editing subcommand
	*$test_prompt*~!uname
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_296.out
	UNAME=`uname`
	grep "$UNAME" /tmp/mail_296.out > /dev/null 2>&1
	TT1="$?"
	grep "test for the ~! editing subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "end of letter" /var/spool/mail/${USER}
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
        lputil verify "mail${TX}296" $?
#	rm -f /var/spool/mail/${USER} /tmp/mail_296.out
	echo;echo
}

mail_297()
{
        LPTEST=mail${TX}297
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~| editing subcommand"

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*test for the ~| editing subcommand
	*$test_prompt*aaaaaaaaaa
	*$test_prompt*bbbbbbbbbb
	*$test_prompt*~| fmt
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} 
	grep "test for the ~| editing subcommand aaaaaaaaaa bbbbbbbbbb" /var/spool/mail/${USER} > /dev/null 2>&1
        lputil verify "mail${TX}297" $?
	rm -f /var/spool/mail/${USER} 
	echo;echo
}

mail_298()
{
        LPTEST=mail${TX}298
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~~ editing subcommand"

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*test for the ~~ editing subcommand
	*$test_prompt*~~r
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} 
	grep "test for the ~~ editing subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "^~r" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
        lputil verify "mail${TX}298" $?
	rm -f /var/spool/mail/${USER} 
	echo;echo
}

mail_299()
{
        LPTEST=mail${TX}299
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~? editing subcommand"

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*test for the ~? editing subcommand
	*$test_prompt*~?
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_299.out
	grep "test for the ~? editing subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "Control Commmands:
   <EOT>           Send message (Ctrl-D on many terminals)
   ~q              Quit editor without saving or sending message.
   ~p              Display the contents of the message buffer.
   ~: <mcmd>       Run a mailbox command, <mcmd>.
Add to Heading:
   ~h              Add to lists for To:, Subject:, Cc: and Bcc:.
   ~t <addrlist>   Add user addresses in <addrlist> to the To: list.
   ~s <subject>    Set the Subject: line to the string specified by <subject>.
   ~c <addrlist>   Add user addresses in <addrlist> to Cc: (copy to) list.
   ~b <addrlist>   Add user addresses in <addrlist> to Bcc: (blind copy) list.
Add to Message:
   ~d              Append the contents of dead.letter to the message.
   ~r <filename>   Append the contents of <filename> to the message.
   ~f <numlist>    Append the contents of message numbers <numlist>.
   ~m <numlist>    Append/indent the contents of message numbers <numlist>.
Change Message:
   ~e              Edit the message using an external editor (default is e).
   ~v              Edit the message using an external editor (default is vi).
   ~w <filename>   Write the message to <filename>.
   ~! <command>    Start a shell, run <command>, and return to the editor.
   ~| <command>    Pipe the message to standard input of <command>; REPLACE
                   the message with the standard output from that command.
================ Mail Editor Commands  (continue on next line) ================" /tmp/mail_299.out > /dev/null 2>&1
        lputil verify "mail${TX}299" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_299.out
	echo;echo
}

mail_300()
{
        LPTEST=mail${TX}300
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number   - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~e editing subcommand"
	export EDITOR=/usr/bin/vi
	
	$ask_loc -w* <<-EOF
	*${sh_prompt}*mail ${USER}
	*Subject: *testing mail
	*$test_prompt*~e
#	*~*i hello joy 
#	* *nothing
#	* * :wq
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "hello joy" /var/spool/mail/${USER} > /dev/null 2>&1
        lputil verify "mail${TX}300" $?
        rm -f /usr/spool/mail/${USER}
	unset EDITOR
        echo;echo
}

mail_301()
{
        LPTEST=mail${TX}301
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number   - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~v editing subcommand"
	export EDITOR=/usr/bin/vi

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER}
	*Subject: *testing mail
	*$test_prompt*~v
	*~*i hello joy 
	* *nothing
	* * :wq
	* *testing the ~v editing subcommand
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "hello joy" /var/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "testing the ~v editing subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
        lputil verify "mail${TX}301" $?
        rm -f /var/spool/mail/${USER}
	unset EDITOR
        echo;echo
}

mail_302()
{
	LPTEST=mail${TX}302
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the vacation -I  command"
	# initialize the $HOME/.vacation.pag and $HOME/.vacation.dir
	# files with the vacation -I flag
	vacation -I -f
	ls $HOME/.vacation.pag > /dev/null 2>&1
	TT1="$?"
	ls $HOME/.vacation.dir > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}302" $?
	rm -f $HOME/.vacation.*      
	echo;echo
}

mail_303()
{
	LPTEST=mail${TX}303
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the vacation -I  command"
	# initialize the $HOME/.vacation.pag and $HOME/.vacation.dir
	# files with the vacation -I flag
	vacation -f -I > /tmp/mail_303.out 2>&1
	grep "vacation: Usage: vacation \[-f interval\] username (or) vacation -I" /tmp/mail_303.out
	TT1="$?"
	ls $HOME/.vacation.pag > /dev/null 2>&1
	TT2="$?"
	ls $HOME/.vacation.dir > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 2 ] && [ $TT3 = 2 ]
	lputil verify "mail${TX}303" $?
	rm -f $HOME/.vacation.*      
	rm -f /tmp/mail_303.out
	echo;echo
}

mail_304()
{
	LPTEST=mail${TX}304
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the vacation -f  command"
	vacation -f > /tmp/mail_304.out 2>&1
	grep "vacation: Usage: vacation \[-f interval\] username (or) vacation -I" /tmp/mail_304.out
	TT1="$?"
	grep "vacation: option requires an argument -- f" /tmp/mail_304.out
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] 
	lputil verify "mail${TX}304" $?
	rm -f /tmp/mail_304.out
	echo;echo
}

mail_305()
{
	LPTEST=mail${TX}305
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the vacation -f  command"
	vacation -f username > /tmp/mail_305.out 2>&1
	grep "vacation: Usage: vacation \[-f interval\] username (or) vacation -I" /tmp/mail_305.out
	TT1="$?"
	[ $TT1 = 0 ]  
	lputil verify "mail${TX}305" $?
	rm -f /tmp/mail_305.out
	echo;echo
}

mail_306()
{
	LPTEST=mail${TX}306
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the vacation -f  command"
	vacation -f 3 username > /tmp/mail_306.out 2>&1
	grep "vacation: Unknown user username" /tmp/mail_306.out
	TT1="$?"
	[ $TT1 = 0 ]  
	lputil verify "mail${TX}306" $?
	rm -f /tmp/mail_306.out
	echo;echo
}

mail_307()
{
	LPTEST=mail${TX}307
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the vacation -f  command"
	vacation -f 3h  > /tmp/mail_307.out 2>&1
	grep "vacation: Unknown user 3h" /tmp/mail_307.out
	TT1="$?"
	[ $TT1 = 0 ]  
	lputil verify "mail${TX}307" $?
	rm -f /tmp/mail_307.out
	echo;echo
}


mail_308()
{
	LPTEST=mail${TX}308
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the vacation -f  command"
	vacation -f3h  > /tmp/mail_308.out 2>&1
	grep "vacation: Usage: vacation \[-f interval\] username (or) vacation -I" /tmp/mail_308.out
	TT1="$?"
	[ $TT1 = 0 ]  
	lputil verify "mail${TX}308" $?
	rm -f /tmp/mail_308.out
	echo;echo
}

mail_309()
{
	LPTEST=mail${TX}309
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the vacation -f  command"
	vacation -f3h  username > /tmp/mail_309.out 2>&1
	grep "vacation: Unknown user username" /tmp/mail_309.out
	TT1="$?"
	[ $TT1 = 0 ]  
	lputil verify "mail${TX}309" $?
	rm -f /tmp/mail_309.out
	echo;echo
}

mail_310()
{
	LPTEST=mail${TX}310
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the vacation -f  command"
	vacation -f3x  username > /tmp/mail_310.out 2>&1
	grep "vacation: Usage: vacation \[-f interval\] username (or) vacation -I" /tmp/mail_310.out
	TT1="$?"
	[ $TT1 = 0 ]  
	lputil verify "mail${TX}310" $?
	rm -f /tmp/mail_310.out
	echo;echo
}


mail_311()
{
	LPTEST=mail${TX}311
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the vacation -f  command"
	vacation -f0m  username > /tmp/mail_311.out 2>&1
	grep "vacation: Usage: vacation \[-f interval\] username (or) vacation -I" /tmp/mail_311.out
	TT1="$?"
	[ $TT1 = 0 ]  
	lputil verify "mail${TX}311" $?
	rm -f /tmp/mail_311.out
	echo;echo
}


mail_312()
{
	LPTEST=mail${TX}312
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the vacation -f  command"
	# initialize the $HOME/.vacation.pag and $HOME/.vacation.dir
	# files with the vacation -I flag
	vacation -I
	ls $HOME/.vacation.pag > /dev/null 2>&1
	TT1="$?"
	ls $HOME/.vacation.dir > /dev/null 2>&1
	TT2="$?"

	# create the $HOME/.vacation.msg file
	echo "From: tester@mail.sh.file
Subject: testing the vacation command

I am on vacation until sometime next year when I feel like
coming back. If you have any questions, please contact my next of kin.

See ya, " > $HOME/.vacation.msg

	# set up the $HOME/.forward file
	echo "\0134${USER},\"|vacation -f2w ${USER}\"" > $HOME/.forward
	
	# now send yourself some mail 
	mail ${USER} <<-EOF
		test1 for testing the vacation command
	EOF
	sync;sync
	sleep 5
	
	# now let's see what's in our system mailbox 
	grep "test1 for testing the vacation command" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	grep "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT4="$?"
	
	# send some mail again because we want to be sure that it 
	# only sends the mail message once every two weeks

	sleep 5
	
	mail ${USER} <<-EOF
		test2 for testing the vacation command
	EOF
	sync;sync
	sleep 5
	
	# make sure we only have one copy of the vacation message
	# in our system mailbox
	
	TT5=`grep -c "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER}`
	
	# let's set the day ahead by a week and see what happens

	DATE=`date +"%m%d7/27/94M"`  
	DATE1=`date +"%d"` 
	NEWDAY=`echo "$DATE1 + 7" | bc`
	if [ $NEWDAY -gt 30 ]; then
		NEWDAY=`echo "$DATE + 00070000" | bc`
		NEW_DATE=`echo "$NEWDAY - 00300000" | bc`
		NEW_DATE=`echo "$NEW_DATE + 01000000" | bc`
	else
		NEW_DATE=`echo "$DATE + 00070000" | bc`
	fi
	# now set the new date which will be ahead by 7 days and a few seconds
	# "bc" may have thrown out the leading zero if the month is before
	# October, so see if we need to insert a leading zero
	if [ $NEW_DATE -gt 10000000 ]; then
		date ${NEW_DATE}
	else	date 0${NEW_DATE}
	fi
	
	# send some more mail
	mail ${USER} <<-EOF
		test3 for testing the vacation command
	EOF
	sync;sync
	sleep 3

	# check and see if a vacation notice was sent , only one message 
	# should be in the mailbox from the first time mail was sent.

	TT6=`grep -c "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER}`
	
	# let's set the date back to the original date, it may be off by 
	# a few seconds
	date $DATE

	# make sure the message sent was put in the user's mailbox
	grep "test3 for testing the vacation command" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT7="$?"

	# now let's check with the date set ahead by two weeks

	DATE=`date +"%m%d7/27/94M"`  
	DATE1=`date +"%d"` 
	NEWDAY=`echo "$DATE1 + 14" | bc`
	if [ $NEWDAY -gt 30 ]; then
		NEWDAY=`echo "$DATE + 00140000" | bc`
		NEW_DATE=`echo "$NEWDAY - 00300000" | bc`
		NEW_DATE=`echo "$NEW_DATE + 01000000" | bc`
	else
		NEW_DATE=`echo "$DATE + 00140000" | bc`
	fi
	# now set the new date which will be ahead by 14 days and a few seconds
	# "bc" may have thrown out the leading zero if the month is before
	# October, so see if we need to insert a leading zero
	if [ $NEW_DATE -gt 10000000 ]; then
		date ${NEW_DATE}
	else	date 0${NEW_DATE}
	fi
	
	# send some more mail
	mail ${USER} <<-EOF
		test4 for testing the vacation command
	EOF
	sync;sync
	sleep 3
	
	# let's set the date back to the original date, it may be off by 
	# a few seconds
	date $DATE

	# we should now have two copies of the vacation message
	# in our system mailbox
	
	TT8=`grep -c "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER}`
	
	# make sure the mail sent was put in the user's mailbox
	grep "test4 for testing the vacation command" /usr/spool/mail/${USER} >/dev/null 2>&1
	TT9="$?"
	
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 1 ] && [ $TT6 = 1 ] && [ $TT7 = 0 ] && [ $TT8 = 2 ] && [ $TT9 = 0 ]
	lputil verify "mail${TX}312" $?
	rm -f $HOME/.vacation.*  $HOME/.forward
	rm -f /usr/spool/mail/${USER}
	echo;echo
}
	
mail_313()
{
	LPTEST=mail${TX}313
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the vacation -f  command"
	# initialize the $HOME/.vacation.pag and $HOME/.vacation.dir
	# files with the vacation -I flag
	vacation -I
	ls $HOME/.vacation.pag > /dev/null 2>&1
	TT1="$?"
	ls $HOME/.vacation.dir > /dev/null 2>&1
	TT2="$?"

	# create the $HOME/.vacation.msg file
	echo "From: tester@mail.sh.file
Subject: testing the vacation command

I am on vacation until sometime next year when I feel like
coming back. If you have any questions, please contact my next of kin.

See ya, " > $HOME/.vacation.msg

	# set up the $HOME/.forward file
	echo "\0134${USER},\"|vacation -f4d ${USER}\"" > $HOME/.forward
	
	# now send yourself some mail 
	mail ${USER} <<-EOF
		test1 for testing the vacation command
	EOF
	sync;sync
	sleep 5
	
	# now let's see what's in our system mailbox 
	grep "test1 for testing the vacation command" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	grep "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT4="$?"
	
	# send some mail again because we want to be sure that it 
	# only sends the mail message once every 4 days

	sleep 5
	
	mail ${USER} <<-EOF
		test2 for testing the vacation command
	EOF
	sync;sync
	sleep 5
	
	# make sure we only have one copy of the vacation message
	# in our system mailbox
	
	TT5=`grep -c "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER}`
	
	# let's set the time ahead by 2 days and see what happens

	DATE=`date +"%m%d7/27/94M"`  
	DATE1=`date +"%d"` 
	NEWDAY=`echo "$DATE1 + 2" | bc`
	if [ $NEWDAY -gt 30 ]; then
		NEWDAY=`echo "$DATE + 00020000" | bc`
		NEW_DATE=`echo "$NEWDAY - 00300000" | bc`
		NEW_DATE=`echo "$NEWDAY - 00300000" | bc`
		NEW_DATE=`echo "$NEW_DATE + 01000000" | bc`
	else
		NEW_DATE=`echo "$DATE + 00020000" | bc`
	fi
	# now set the new date which will be ahead by 2 days and a few seconds
	# "bc" may have thrown out the leading zero if the month is before
	# October, so see if we need to insert a leading zero
	if [ $NEW_DATE -gt 10000000 ]; then
		date ${NEW_DATE}
	else	date 0${NEW_DATE}
	fi
	
	# send some more mail
	mail ${USER} <<-EOF
		test3 for testing the vacation command
	EOF
	sync;sync
	sleep 3

	# check and see if a vacation notice was sent , only one message 
	# should be in the mailbox from the first time mail was sent.

	TT6=`grep -c "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER}`
	
	# let's set the date back to the original date, it may be off by 
	# a few seconds
	date $DATE

	# make sure the message sent was put in the user's mailbox
	grep "test3 for testing the vacation command" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT7="$?"

	# now let's check with the date set ahead by 4 days 

	DATE=`date +"%m%d7/27/94M"`  
	DATE1=`date +"%d"` 
	NEWDAY=`echo "$DATE1 + 4" | bc`
	if [ $NEWDAY -gt 30 ]; then
		NEWDAY=`echo "$DATE + 00040000" | bc`
		NEW_DATE=`echo "$NEWDAY - 00300000" | bc`
		NEW_DATE=`echo "$NEW_DATE + 01000000" | bc`
	else
		NEW_DATE=`echo "$DATE + 00040000" | bc`
	fi
	# now set the new date which will be ahead by 4 days and a few seconds
	# "bc" may have thrown out the leading zero if the month is before
	# October, so see if we need to insert a leading zero
	if [ $NEW_DATE -gt 10000000 ]; then
		date ${NEW_DATE}
	else	date 0${NEW_DATE}
	fi
	
	# send some more mail
	mail ${USER} <<-EOF
		test4 for testing the vacation command
	EOF
	sync;sync
	sleep 3
	
	# let's set the date back to the original date, it may be off by 
	# a few seconds
	date $DATE

	# we should now have two copies of the vacation message
	# in our system mailbox
	
	TT8=`grep -c "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER}`
	
	# make sure the mail sent was put in the user's mailbox
	grep "test4 for testing the vacation command" /usr/spool/mail/${USER} >/dev/null 2>&1
	TT9="$?"
	
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 1 ] && [ $TT6 = 1 ] && [ $TT7 = 0 ] && [ $TT8 = 2 ] && [ $TT9 = 0 ]
	lputil verify "mail${TX}313" $?
	rm -f $HOME/.vacation.*  $HOME/.forward
	rm -f /usr/spool/mail/${USER}
	echo;echo
}
	
mail_314()
{
	LPTEST=mail${TX}314
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the vacation -f  command"
	# initialize the $HOME/.vacation.pag and $HOME/.vacation.dir
	# files with the vacation -I flag
	vacation -I
	ls $HOME/.vacation.pag > /dev/null 2>&1
	TT1="$?"
	ls $HOME/.vacation.dir > /dev/null 2>&1
	TT2="$?"

	# create the $HOME/.vacation.msg file
	echo "From: tester@mail.sh.file
Subject: testing the vacation command

I am on vacation until sometime next year when I feel like
coming back. If you have any questions, please contact my next of kin.

See ya, " > $HOME/.vacation.msg

	# set up the $HOME/.forward file
	echo "\0134${USER},\"|vacation -f96h ${USER}\"" > $HOME/.forward
	
	# now send yourself some mail 
	mail ${USER} <<-EOF
		test1 for testing the vacation command
	EOF
	sync;sync
	sleep 5
	
	# now let's see what's in our system mailbox 
	grep "test1 for testing the vacation command" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	grep "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT4="$?"
	
	# send some mail again because we want to be sure that it 
	# only sends the mail message once every 96 hours (or 4 days)

	sleep 5
	
	mail ${USER} <<-EOF
		test2 for testing the vacation command
	EOF
	sync;sync
	sleep 5
	
	# make sure we only have one copy of the vacation message
	# in our system mailbox
	
	TT5=`grep -c "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER}`
	
	# let's set the time ahead by 2 days and see what happens

	DATE=`date +"%m%d7/27/94M"`  
	DATE1=`date +"%d"` 
	NEWDAY=`echo "$DATE1 + 2" | bc`
	if [ $NEWDAY -gt 30 ]; then
		NEWDAY=`echo "$DATE + 00020000" | bc`
		NEW_DATE=`echo "$NEWDAY - 00300000" | bc`
		NEW_DATE=`echo "$NEW_DATE + 01000000" | bc`
	else
		NEW_DATE=`echo "$DATE + 00020000" | bc`
	fi
	# now set the new date which will be ahead by 2 days and a few seconds
	# "bc" may have thrown out the leading zero if the month is before
	# October, so see if we need to insert a leading zero
	if [ $NEW_DATE -gt 10000000 ]; then
		date ${NEW_DATE}
	else	date 0${NEW_DATE}
	fi
	
	# send some more mail
	mail ${USER} <<-EOF
		test3 for testing the vacation command
	EOF
	sync;sync
	sleep 3

	# check and see if a vacation notice was sent , only one message 
	# should be in the mailbox from the first time mail was sent.

	TT6=`grep -c "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER}`
	
	# let's set the date back to the original date, it may be off by 
	# a few seconds
	date $DATE

	# make sure the message sent was put in the user's mailbox
	grep "test3 for testing the vacation command" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT7="$?"

	# now let's check with the date set ahead by 4 days 

	DATE=`date +"%m%d7/27/94M"`  
	DATE1=`date +"%d"` 
	NEWDAY=`echo "$DATE1 + 4" | bc`
	if [ $NEWDAY -gt 30 ]; then
		NEWDAY=`echo "$DATE + 00040000" | bc`
		NEW_DATE=`echo "$NEWDAY - 00300000" | bc`
		NEW_DATE=`echo "$NEW_DATE + 01000000" | bc`
	else
		NEW_DATE=`echo "$DATE + 00040000" | bc`
	fi
	# now set the new date which will be ahead by 4 days and a few seconds
	# "bc" may have thrown out the leading zero if the month is before
	# October, so see if we need to insert a leading zero
	if [ $NEW_DATE -gt 10000000 ]; then
		date ${NEW_DATE}
	else	date 0${NEW_DATE}
	fi
	
	# send some more mail
	mail ${USER} <<-EOF
		test4 for testing the vacation command
	EOF
	sync;sync
	sleep 3
	
	# let's set the date back to the original date, it may be off by 
	# a few seconds
	date $DATE

	# we should now have two copies of the vacation message
	# in our system mailbox
	
	TT8=`grep -c "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER}`
	
	# make sure the mail sent was put in the user's mailbox
	grep "test4 for testing the vacation command" /usr/spool/mail/${USER} >/dev/null 2>&1
	TT9="$?"
	
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 1 ] && [ $TT6 = 1 ] && [ $TT7 = 0 ] && [ $TT8 = 2 ] && [ $TT9 = 0 ]
	lputil verify "mail${TX}314" $?
	rm -f $HOME/.vacation.*  $HOME/.forward
	rm -f /usr/spool/mail/${USER}
	echo;echo
}
	
mail_315()
{
	LPTEST=mail${TX}315
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the vacation -f  command"
	# initialize the $HOME/.vacation.pag and $HOME/.vacation.dir
	# files with the vacation -I flag
	vacation -I
	ls $HOME/.vacation.pag > /dev/null 2>&1
	TT1="$?"
	ls $HOME/.vacation.dir > /dev/null 2>&1
	TT2="$?"

	# create the $HOME/.vacation.msg file
	echo "From: tester@mail.sh.file
Subject: testing the vacation command

I am on vacation until sometime next year when I feel like
coming back. If you have any questions, please contact my next of kin.

See ya, " > $HOME/.vacation.msg

	# set up the $HOME/.forward file
	echo "\0134${USER},\"|vacation -f1440m ${USER}\"" > $HOME/.forward
	
	# now send yourself some mail 
	mail ${USER} <<-EOF
		test1 for testing the vacation command
	EOF
	sync;sync
	sleep 5
	
	# now let's see what's in our system mailbox 
	grep "test1 for testing the vacation command" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	grep "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT4="$?"
	
	# send some mail again because we want to be sure that it 
	# only sends the mail message once every 1440 minutes (1 day)

	sleep 5
	
	mail ${USER} <<-EOF
		test2 for testing the vacation command
	EOF
	sync;sync
	sleep 5
	
	# make sure we only have one copy of the vacation message
	# in our system mailbox
	
	TT5=`grep -c "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER}`
	
	# now let's check with the date set ahead by 1440 minutes (1 day) 

	DATE=`date +"%m%d7/27/94M"`  
	DATE1=`date +"%d"` 
	NEWDAY=`echo "$DATE1 + 1" | bc`
	if [ $NEWDAY -gt 30 ]; then
		NEWDAY=`echo "$DATE + 00010000" | bc`
		NEW_DATE=`echo "$NEWDAY - 00300000" | bc`
		NEW_DATE=`echo "$NEW_DATE + 01000000" | bc`
	else
		NEW_DATE=`echo "$DATE + 00010000" | bc`
	fi
	# now set the new date which will be ahead by  1 day and a few seconds
	# "bc" may have thrown out the leading zero if the month is before
	# October, so see if we need to insert a leading zero
	if [ $NEW_DATE -gt 10000000 ]; then
		date ${NEW_DATE}
	else	date 0${NEW_DATE}
	fi
	
	# send some more mail
	mail ${USER} <<-EOF
		test3 for testing the vacation command
	EOF
	sync;sync
	sleep 3
	
	# let's set the date back to the original date, it may be off by 
	# a few seconds
	date $DATE

	# we should now have two copies of the vacation message
	# in our system mailbox
	
	TT6=`grep -c "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER}`
	
	# make sure the mail sent was put in the user's mailbox
	grep "test3 for testing the vacation command" /usr/spool/mail/${USER} >/dev/null 2>&1
	TT7="$?"
	
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 1 ] && [ $TT6 = 2 ] && [ $TT7 = 0 ] 
	lputil verify "mail${TX}315" $?
	rm -f $HOME/.vacation.*  $HOME/.forward
	rm -f /usr/spool/mail/${USER}
	echo;echo
}
	
mail_316()
{
	LPTEST=mail${TX}316
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the vacation -f  command"
	# initialize the $HOME/.vacation.pag and $HOME/.vacation.dir
	# files with the vacation -I flag
	vacation -I
	ls $HOME/.vacation.pag > /dev/null 2>&1
	TT1="$?"
	ls $HOME/.vacation.dir > /dev/null 2>&1
	TT2="$?"

	# create the $HOME/.vacation.msg file
	echo "From: tester@mail.sh.file
Subject: testing the vacation command

I am on vacation until sometime next year when I feel like
coming back. If you have any questions, please contact my next of kin.

See ya, " > $HOME/.vacation.msg

	# set up the $HOME/.forward file
	echo "\0134${USER},\"|vacation -f 10s ${USER}\"" > $HOME/.forward
	
	# now send yourself some mail 
	mail ${USER} <<-EOF
		test1 for testing the vacation command
	EOF
	sync;sync
	sleep 5
	
	# now let's see what's in our system mailbox 
	grep "test1 for testing the vacation command" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	grep "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT4="$?"
	
	# let's wait 10 seconds and then try again.
	sleep 10	
	
	# send some more mail
	mail ${USER} <<-EOF
		test2 for testing the vacation command
	EOF
	sync;sync

	sleep 3

	# we should now have two copies of the vacation message
	# in our system mailbox
	
	TT5=`grep -c "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER}`
	
	# make sure the mail sent was put in the user's mailbox
	grep "test2 for testing the vacation command" /usr/spool/mail/${USER} >/dev/null 2>&1
	TT6="$?"
	
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 2 ] && [ $TT6 = 0 ] 
	lputil verify "mail${TX}316" $?
	rm -f $HOME/.vacation.*  $HOME/.forward
	rm -f /usr/spool/mail/${USER}
	echo;echo
}

mail_317()
{
	LPTEST=mail${TX}317
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the vacation -f  command"
	# initialize the $HOME/.vacation.pag and $HOME/.vacation.dir
	# files with the vacation -I flag
	vacation -I
	ls $HOME/.vacation.pag > /dev/null 2>&1
	TT1="$?"
	ls $HOME/.vacation.dir > /dev/null 2>&1
	TT2="$?"

	# create the $HOME/.vacation.msg file
	echo "From: tester@mail.sh.file
Subject: testing the vacation command

I am on vacation until sometime next year when I feel like
coming back. If you have any questions, please contact my next of kin.

See ya, " > $HOME/.vacation.msg

	# set up the $HOME/.forward file
	echo "\0134${USER},\"|vacation -f4 ${USER}\"" > $HOME/.forward
	
	# now send yourself some mail 
	mail ${USER} <<-EOF
		test1 for testing the vacation command
	EOF
	sync;sync
	sleep 5
	
	# now let's see what's in our system mailbox 
	grep "test1 for testing the vacation command" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	grep "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT4="$?"
	
	# send some mail again because we want to be sure that it 
	# only sends the mail message once every 4 days

	sleep 5
	
	mail ${USER} <<-EOF
		test2 for testing the vacation command
	EOF
	sync;sync
	sleep 5
	
	# make sure we only have one copy of the vacation message
	# in our system mailbox
	
	TT5=`grep -c "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER}`
	
	# let's set the time ahead by 2 days and see what happens

	DATE=`date +"%m%d7/27/94M"`  
	DATE1=`date +"%d"` 
	NEWDAY=`echo "$DATE1 + 2" | bc`
	if [ $NEWDAY -gt 30 ]; then
		NEWDAY=`echo "$DATE + 00020000" | bc`
		NEW_DATE=`echo "$NEWDAY - 00300000" | bc`
		NEW_DATE=`echo "$NEWDAY - 00300000" | bc`
		NEW_DATE=`echo "$NEW_DATE + 01000000" | bc`
	else
		NEW_DATE=`echo "$DATE + 00020000" | bc`
	fi
	# now set the new date which will be ahead by 2 days and a few seconds
	# "bc" may have thrown out the leading zero if the month is before
	# October, so see if we need to insert a leading zero
	if [ $NEW_DATE -gt 10000000 ]; then
		date ${NEW_DATE}
	else	date 0${NEW_DATE}
	fi
	
	# send some more mail
	mail ${USER} <<-EOF
		test3 for testing the vacation command
	EOF
	sync;sync
	sleep 3

	# check and see if a vacation notice was sent , only one message 
	# should be in the mailbox from the first time mail was sent.

	TT6=`grep -c "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER}`
	
	# let's set the date back to the original date, it may be off by 
	# a few seconds
	date $DATE

	# make sure the message sent was put in the user's mailbox
	grep "test3 for testing the vacation command" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT7="$?"

	# now let's check with the date set ahead by 4 days 

	DATE=`date +"%m%d7/27/94M"`  
	DATE1=`date +"%d"` 
	NEWDAY=`echo "$DATE1 + 4" | bc`
	if [ $NEWDAY -gt 30 ]; then
		NEWDAY=`echo "$DATE + 00040000" | bc`
		NEW_DATE=`echo "$NEWDAY - 00300000" | bc`
		NEW_DATE=`echo "$NEW_DATE + 01000000" | bc`
	else
		NEW_DATE=`echo "$DATE + 00040000" | bc`
	fi
	# now set the new date which will be ahead by 4 days and a few seconds
	# "bc" may have thrown out the leading zero if the month is before
	# October, so see if we need to insert a leading zero
	if [ $NEW_DATE -gt 10000000 ]; then
		date ${NEW_DATE}
	else	date 0${NEW_DATE}
	fi
	
	# send some more mail
	mail ${USER} <<-EOF
		test4 for testing the vacation command
	EOF
	sync;sync
	sleep 3
	
	# let's set the date back to the original date, it may be off by 
	# a few seconds
	date $DATE

	# we should now have two copies of the vacation message
	# in our system mailbox
	TT8=`grep -c "I am on vacation until sometime next year when I feel like" /usr/spool/mail/${USER}`
	
	# make sure the mail sent was put in the user's mailbox
	grep "test4 for testing the vacation command" /usr/spool/mail/${USER} >/dev/null 2>&1
	TT9="$?"
	
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 1 ] && [ $TT6 = 1 ] && [ $TT7 = 0 ] && [ $TT8 = 2 ] && [ $TT9 = 0 ]
	lputil verify "mail${TX}317" $?
	rm -f $HOME/.vacation.*  $HOME/.forward
	rm -f /usr/spool/mail/${USER}
	echo;echo
}
	
mail_318()
{
	LPTEST=mail${TX}318
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the mail -c command"
	mail -c  > /tmp/mail_318.out 2>&1
	grep "Address required with -c" /tmp/mail_318.out > /dev/null 2>&1
	lputil verify "mail${TX}318" $?
	rm -f /tmp/mail_318.out
	echo;echo
}


mail_319()
{
	LPTEST=mail${TX}319
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the mail -c command"
	echo "this is a test for test case mail_319" > /tmp/mail_319.file
	mail -c ${USER} < /tmp/mail_319.file > /tmp/mail_319.out 2>&1
	grep "The flags you gave make no sense since you're not sending mail" /tmp/mail_319.out > /dev/null 2>&1
	TT1="$?"
	[ $TT1 = 0 ]  
	lputil verify "mail${TX}319" $?
	rm -f /tmp/mail_319.out /tmp/mail_319.file
	echo;echo
}

mail_320()
{
	LPTEST=mail${TX}320
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the mail -c command"
	echo "this is a test for the testcase mail_320." > /tmp/mail_320.file
	mail -c "${USER} ${USER2}" bugs < /tmp/mail_320.file 
	sleep 3
	grep "this is a test for the testcase mail_320." /usr/spool/mail/${USER} > /dev/null 2>&1	
	TT1="$?"
	grep "Cc: ${USER2}, ${USER}" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "this is a test for the testcase mail_320." /usr/spool/mail/${USER2} > /dev/null 2>&1
	TT3="$?"
	grep "Cc: ${USER2}, ${USER}" /usr/spool/mail/${USER2} > /dev/null 2>&1
	TT4="$?"
	grep "this is a test for the testcase mail_320." /usr/spool/mail/bugs > /dev/null 2>&1
	TT5="$?"
	grep "Cc: ${USER2}, ${USER}" /usr/spool/mail/bugs > /dev/null 2>&1
	TT6="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] && [ $TT6 = 0 ]
	lputil verify "mail${TX}320" $?
	rm -f /tmp/mail_320.out /tmp/mail_320.file
	rm -f /var/spool/mail/${USER} /var/spool/mail/${USER2} /var/spool/mail/bugs
	echo;echo
}

mail_321()
{
	LPTEST=mail${TX}321
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	echo "$LPTEST FVT testing the mail -c command"
	echo "this is a test for the testcase mail_321." > /tmp/mail_321.file
	mail -c "${USER}" ${USER2} < /tmp/mail_321.file 
	sleep 3
	grep "this is a test for the testcase mail_321." /usr/spool/mail/${USER} > /dev/null 2>&1	
	TT1="$?"
	grep "Cc: ${USER}" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "this is a test for the testcase mail_321." /usr/spool/mail/${USER2} > /dev/null 2>&1
	TT3="$?"
	grep "Cc: ${USER}" /usr/spool/mail/${USER2} > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ]  && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
	lputil verify "mail${TX}321" $?
	rm -f /tmp/mail_321.out /tmp/mail_321.file
	rm -f /var/spool/mail/${USER} /var/spool/mail/${USER2}
	echo;echo
}

mail_322()
{
	LPTEST=mail${TX}322
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the mail -c command"
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail -c ${USER} ${USER2}
	${waitch}Subject: ${waitch}mail_test
	${waitch}$test_prompt${waitch}testing the mail -c flag
	${waitch}$test_prompt${waitch}.
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_322.out
	grep "Cc:" /tmp/mail_322.out > /dev/null 2>&1
	TT1="$?"
	grep "testing the mail -c flag" /usr/spool/mail/${USER} >/dev/null 2>&1	
	TT2="$?"
	grep "Cc: ${USER}" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	grep "testing the mail -c flag" /usr/spool/mail/${USER2} > /dev/null 2>&1
	TT4="$?"
	grep "Cc: ${USER}" /usr/spool/mail/${USER2} > /dev/null 2>&1
	TT5="$?"
	[ $TT1 -ne 0 ]  && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ]
	lputil verify "mail${TX}322" $?
	rm -f /tmp/mail_322.out /tmp/mail_322.file
	rm -f /var/spool/mail/${USER} /var/spool/mail/${USER2}
	echo;echo
}


mail_323()
{
	LPTEST=mail${TX}323
	rm -f /tmp/${LPTEST}
	echo "Started `date` - mail TEST Number - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the mail -c command"
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail -c "${USER} ${USER2}" bugs 
	${waitch}Subject: ${waitch}mail_test
	${waitch}$test_prompt${waitch}testing the mail -c flag
	${waitch}$test_prompt${waitch}.
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_323.out
	sleep 3
	grep "Cc:" /tmp/mail_323.out > /dev/null 2>&1
	TT1="$?"
	grep "testing the mail -c flag" /usr/spool/mail/${USER} >/dev/null 2>&1	
	TT2="$?"
	grep "Cc: ${USER2}, ${USER}" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	grep "testing the mail -c flag" /usr/spool/mail/${USER2} > /dev/null 2>&1
	TT4="$?"
	grep "Cc: ${USER2}, ${USER}" /usr/spool/mail/${USER2} > /dev/null 2>&1
	TT5="$?"
	grep "testing the mail -c flag" /usr/spool/mail/bugs > /dev/null 2>&1
	TT6="$?"
	grep "Cc: ${USER2}, ${USER}" /usr/spool/mail/bugs > /dev/null 2>&1
	TT7="$?"
	[ $TT1 -ne 0 ]  && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] && [ $TT6 = 0 ] && [ $TT7 = 0 ]
	lputil verify "mail${TX}323" $?
	rm -f /tmp/mail_323.out /tmp/mail_323.file
	rm -f /var/spool/mail/${USER} /var/spool/mail/${USER2} /var/spool/mail/bugs
	echo;echo
}

mail_324()
{
        LPTEST=mail${TX}324
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the mail -c flag"

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail -s "testing mail" -c somebody buggs
	*$test_prompt*~h 
	*To: buggs*s
	*Subject: testing mail*no mail
	*Cc: somebody*
	*Bcc: *$test_prompt
	*$test_prompt*test for the ~h editing subcommand
	*$test_prompt*.
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee /tmp/mail_324.out
	grep "Cc: somebody" /usr/spool/mail/bugs >/dev/null 2>&1
	TT1="$?"
	[ $TT1 -ne 0 ] 
        lputil verify "mail${TX}324" $?
        rm -f /var/spool/mail/bugs
        echo;echo
}

mail_325()
{
	LPTEST=mail${TX}325
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set showto mail subcommand" 
	echo "set showto" > $HOME/.mailrc

	# first send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "set showto" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_325.out
	grep "1 To ${USER}" /tmp/mail_325.out > /dev/null 2>&1
	lputil verify "mail${TX}325" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_325.out
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_326()
{
	LPTEST=mail${TX}326
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set showto mail subcommand" 

	# first send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "set showto" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}set showto
	${waitch}$mail_prompt${waitch}headers
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_326.out
	grep "1 To ${USER}" /tmp/mail_326.out > /dev/null 2>&1
	lputil verify "mail${TX}326" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_326.out
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_327()
{
	LPTEST=mail${TX}327
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set showto mail subcommand" 

	echo "set showto" > $HOME/.mailrc 
	# first send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "set showto" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}set noshowto
	${waitch}$mail_prompt${waitch}headers
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_327.out
	grep "1 To ${USER}" /tmp/mail_327.out > /dev/null 2>&1
	TT1="$?"
	grep "1 ${USER}" /tmp/mail_327.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}327" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_327.out
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_328()
{
	LPTEST=mail${TX}328
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set showto mail subcommand" 
	echo "set showto" > $HOME/.mailrc 
	
	# first send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "set showto" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}unset showto
	${waitch}$mail_prompt${waitch}headers
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_328.out
	grep "1 To ${USER}" /tmp/mail_328.out > /dev/null 2>&1
	TT1="$?"
	grep "1 ${USER}" /tmp/mail_328.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}328" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_328.out
	rm -f $HOME/.mailrc
	echo;echo
}
	
mail_329()
{
	LPTEST=mail${TX}329
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set LISTER mail subcommand" 
	echo 'set "LISTER=ls -l"' > $HOME/.mailrc 

	# set up a folders directory containing some folders #	
	mkdir /tmp/mail_folder
	touch /tmp/mail_folder/letter1
	echo "set folder=/tmp/mail_folder" >> $HOME/.mailrc

	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "set LISTER" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}folders
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_329.out
	grep "rw-r--r--" /tmp/mail_329.out > /dev/null 2>&1
	lputil verify "mail${TX}329" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_329.out
	rm -r /tmp/mail_folder > /dev/null 2>&1
	rm -f $HOME/.mailrc
	echo;echo
}

mail_330()
{
	LPTEST=mail${TX}330
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set LISTER mail subcommand" 

	# set up a folders directory containing some folders #	
	mkdir /tmp/mail_folder
	touch /tmp/mail_folder/letter1
	echo "set folder=/tmp/mail_folder" > $HOME/.mailrc

	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "set LISTER" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}set "LISTER=ls -l" 
	${waitch}$mail_prompt${waitch}folders
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_330.out
	grep "rw-r--r--" /tmp/mail_330.out > /dev/null 2>&1
	lputil verify "mail${TX}330" $?
	rm -f /tmp/mail_330.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	rm -r /tmp/mail_folder > /dev/null 2>&1
	echo;echo
}

mail_331()
{
	LPTEST=mail${TX}331
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set LISTER mail subcommand" 

	# set up a folders directory containing some folders #	
	mkdir /tmp/mail_folder
	touch /tmp/mail_folder/letter1
	echo "set folder=/tmp/mail_folder" > $HOME/.mailrc

	# set LISTER as an environment variable
	export LISTER="ls -l"

	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "set LISTER" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}folders
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_331.out
	grep "rw-r--r--" /tmp/mail_331.out > /dev/null 2>&1
	lputil verify "mail${TX}331" $?
	rm -f /tmp/mail_331.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	rm -r /tmp/mail_folder > /dev/null 2>&1
	echo;echo
}

mail_332()
{
	LPTEST=mail${TX}332
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the unset LISTER mail subcommand" 
	echo 'set "LISTER=ls -l"' > $HOME/.mailrc

	# set up a folders directory containing some folders #	
	mkdir /tmp/mail_folder
	touch /tmp/mail_folder/letter1
	echo "set folder=/tmp/mail_folder" >> $HOME/.mailrc

	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "unset LISTER" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}unset LISTER
	${waitch}$mail_prompt${waitch}folders
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_332.out
	grep "rw-r--r--" /tmp/mail_332.out > /dev/null 2>&1
	TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "mail${TX}332" $?
	rm -f /tmp/mail_332.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	rm -r /tmp/mail_folder > /dev/null 2>&1
	echo;echo
}

mail_333()
{
	LPTEST=mail${TX}333
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set cmd mail subcommand" 
	echo 'set "cmd=grep From"' > $HOME/.mailrc

	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "set cmd" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}pipe
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_333.out
	grep "From: ${USER}" /tmp/mail_333.out > /dev/null 2>&1
	lputil verify "mail${TX}333" $?
	rm -f /tmp/mail_333.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	echo;echo
}

mail_334()
{
	LPTEST=mail${TX}334
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set cmd mail subcommand" 

	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "set cmd" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}set "cmd=grep From"
	${waitch}$mail_prompt${waitch}pipe
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_334.out
	grep "From: ${USER}" /tmp/mail_334.out > /dev/null 2>&1
	lputil verify "mail${TX}334" $?
	rm -f /tmp/mail_334.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	echo;echo
}

mail_335()
{
	LPTEST=mail${TX}335
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the unset cmd mail subcommand" 
	echo 'set "cmd=grep From"' > $HOME/.mailrc

	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "unset cmd" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}unset cmd
	${waitch}$mail_prompt${waitch}pipe
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_335.out
	grep "No shell command specified" /tmp/mail_335.out > /dev/null 2>&1
	lputil verify "mail${TX}335" $?
	rm -f /tmp/mail_335.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	echo;echo
}

mail_336()
{
	LPTEST=mail${TX}336
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set nocmd mail subcommand" 
	echo 'set "cmd=grep From"' > $HOME/.mailrc

	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "set nocmd" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}set nocmd
	${waitch}$mail_prompt${waitch}pipe
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_336.out
	grep "No shell command specified" /tmp/mail_336.out > /dev/null 2>&1
	lputil verify "mail${TX}336" $?
	rm -f /tmp/mail_336.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	echo;echo
}

mail_337()
{
	LPTEST=mail${TX}337
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set prompt mail subcommand" 
	echo "set prompt=*" > $HOME/.mailrc
	new_prompt=*

	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "set prompt" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$new_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_337.out
	grep "*quit" /tmp/mail_337.out > /dev/null 2>&1
	lputil verify "mail${TX}337" $?
	rm -f /tmp/mail_337.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	echo;echo
}

mail_338()
{
	LPTEST=mail${TX}338
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the unset prompt mail subcommand" 
	echo "set prompt=*" > $HOME/.mailrc
	new_prompt=*

	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "unset prompt" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$new_prompt${waitch}unset prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_338.out
	grep "$mail_prompt quit" /tmp/mail_338.out > /dev/null 2>&1
	lputil verify "mail${TX}338" $?
	rm -f /tmp/mail_338.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	echo;echo
}

mail_339()
{
	LPTEST=mail${TX}339
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set prompt mail subcommand" 
	new_prompt=*

	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "set prompt" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}set prompt=*
	${waitch}$new_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_339.out
	grep "*Held 1 message in /var/spool/mail/${USER}" /tmp/mail_339.out > /dev/null 2>&1
	lputil verify "mail${TX}339" $?
	rm -f /tmp/mail_339.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	echo;echo
}

mail_340()
{
	LPTEST=mail${TX}340
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set noprompt mail subcommand" 
	echo "set prompt=*" > $HOME/.mailrc
	new_prompt=*

	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the "set noprompt" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$new_prompt${waitch}set noprompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_340.out
	grep "$mail_prompt quit" /tmp/mail_340.out > /dev/null 2>&1
	lputil verify "mail${TX}340" $?
	rm -f /tmp/mail_340.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	echo;echo
}

mail_341()
{
	LPTEST=mail${TX}341
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set outfolder mail subcommand" 
	echo "set record=myfile" > $HOME/.mailrc
	echo "set folder=/tmp/mail_341" >> $HOME/.mailrc
	echo "set outfolder" >> $HOME/.mailrc
	mkdir /tmp/mail_341 > /dev/null 2>&1
	
	# send some mail #
	mail ${USER} <<-EOF
		testing the "set outfolder" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	} | tee -a ${RESULTS} 
	grep 'testing the "set outfolder" mail subcommand' /tmp/mail_341/myfile > /dev/null 2>&1
	lputil verify "mail${TX}341" $?
	rm -f /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	rm -r /tmp/mail_341 > /dev/null 2>&1
	echo;echo
}

mail_342()
{
	LPTEST=mail${TX}342
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set outfolder mail subcommand" 
	echo "set record=myfile" > $HOME/.mailrc
	echo "set folder=/tmp/mail_342" >> $HOME/.mailrc
	mkdir /tmp/mail_342 > /dev/null 2>&1
	
	# send some mail #
	mail -s "mail_342 test" ${USER} <<-EOF
		testing the "set outfolder" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}set outfolder
	${waitch}$mail_prompt${waitch}Reply
	${waitch}$test_prompt${waitch}testing outfolder
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}$sh_prompt${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_342.out
	grep "testing outfolder" /tmp/mail_342/myfile >/dev/null 2>&1
	lputil verify "mail${TX}342" $?
	rm -f /tmp/mail_342.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	rm -r /tmp/mail_342 >/dev/null 2>&1
	echo;echo
}

mail_343()
{
	LPTEST=mail${TX}343
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the unset outfolder mail subcommand" 
	echo "set record=myfile" > $HOME/.mailrc
	echo "set folder=/tmp/mail_343" >> $HOME/.mailrc
	echo "set outfolder" >> $HOME/.mailrc
	mkdir /tmp/mail_343 > /dev/null 2>&1
	
	# send some mail #
	mail -s "mail_343 test" ${USER} <<-EOF
		testing the "unset outfolder" mail subcommand
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}unset outfolder
	${waitch}$mail_prompt${waitch}Reply
	${waitch}$test_prompt${waitch}testing outfolder
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}$sh_prompt${waitch}exit
	EOF
        sync;sync;sync
        } | tee -a ${RESULTS} | tee /tmp/mail_343.out
	grep "testing outfolder" $HOME/myfile > /dev/null 2>&1
	TT1="$?"
	grep "testing outfolder" /tmp/mail_343/myfile > /dev/null 2>&1
	TT2="$?"
	grep "unset outfolder" /tmp/mail_343/myfile > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}343" $?
	rm -f /usr/spool/mail/${USER} $HOME/.mailrc
	rm -f $HOME/myfile
	rm -r /tmp/mail_343 > /dev/null 2>&1
	echo;echo
}

mail_344()
{
	LPTEST=mail${TX}344
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set nooutfolder mail subcommand" 
	echo "set record=myfile" > $HOME/.mailrc
	echo "set folder=/tmp/mail_344" >> $HOME/.mailrc
	echo "set outfolder" >> $HOME/.mailrc
	mkdir /tmp/mail_344 > /dev/null 2>&1
	
	# send some mail #
	mail -s "mail_344 test" ${USER} <<-EOF
		testing the "set nooutfolder" mail subcommand
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}set nooutfolder
	${waitch}$mail_prompt${waitch}Reply
	${waitch}$test_prompt${waitch}testing nooutfolder
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}$sh_prompt${waitch}exit
	EOF
        sync;sync;sync
        } | tee -a ${RESULTS} | tee /tmp/mail_344.out
	grep "testing nooutfolder" $HOME/myfile > /dev/null 2>&1
	TT1="$?"
	grep "testing nooutfolder" /tmp/mail_344/myfile > /dev/null 2>&1
	TT2="$?"
	grep "set nooutfolder" /tmp/mail_344/myfile > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}344" $?
	rm -f /usr/spool/mail/${USER} $HOME/.mailrc
	rm -f $HOME/myfile
	rm -r /tmp/mail_344 > /dev/null 2>&1
	echo;echo
}

mail_345()
{
	LPTEST=mail${TX}345
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set outfolder mail subcommand with an absolute pathname" 
	echo "set record=/tmp/myfile" > $HOME/.mailrc
	echo "set folder=/tmp/mail_345" >> $HOME/.mailrc
	echo "set outfolder" >> $HOME/.mailrc
	mkdir /tmp/mail_345 > /dev/null 2>&1
	
	# send some mail #
	mail ${USER} <<-EOF
		testing the "set outfolder" mail subcommand 
	EOF
	sync;sync 
	sleep 3
	} | tee -a ${RESULTS} 
	grep 'testing the "set outfolder" mail subcommand' /tmp/mail_345/myfile > /dev/null 2>&1
	TT1="$?"
	grep 'testing the "set outfolder" mail subcommand' /tmp/myfile > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 2 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}345" $?
	rm -f /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc /tmp/myfile
	rm -r /tmp/mail_345 > /dev/null 2>&1
	echo;echo
}

mail_346()
{
	LPTEST=mail${TX}346
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set sendwait mail subcommand" 
	echo "set sendwait" > $HOME/.mailrc

	# send some mail #
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail cinderella < $HOME/.mailrc
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
        } | tee -a ${RESULTS} | tee /tmp/mail_346.out
	grep "^cinderella... User unknown" /tmp/mail_346.out > /dev/null 2>&1
	lputil verify "mail${TX}346" $?
	rm -f /usr/spool/mail/${USER} $HOME/.mailrc
	rm -f /tmp/mail_346.out
	echo;echo
}

mail_347()
{
	LPTEST=mail${TX}347
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set sendwait mail subcommand" 

	# send some mail #
	mail ${USER} <<-EOF
		testing set sendwait mail subcommand
	EOF
	sync;sync
	sleep 2

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}set sendwait
	${waitch}$mail_prompt${waitch}mail cinderella 
	${waitch}Subject: ${waitch}testing sendwait
	${waitch}$test_prompt${waitch}testing set sendwait 
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
        } | tee -a ${RESULTS} | tee /tmp/mail_347.out
	grep "^cinderella... User unknown" /tmp/mail_347.out > /dev/null 2>&1
	lputil verify "mail${TX}347" $?
	rm -f /usr/spool/mail/${USER} 
	rm -f /tmp/mail_347.out
	echo;echo
}

mail_348()
{
	LPTEST=mail${TX}348
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the unset sendwait mail subcommand" 
	echo "set sendwait" > $HOME/.mailrc

	# send some mail #
	mail ${USER} <<-EOF
		testing unset sendwait mail subcommand
	EOF
	sync;sync
	sleep 2

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}unset sendwait
	${waitch}$mail_prompt${waitch}mail cinderella 
	${waitch}Subject: ${waitch}testing sendwait
	${waitch}$test_prompt${waitch}testing unset sendwait 
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
        } | tee -a ${RESULTS} | tee /tmp/mail_348.out
	grep "^cinderella... User unknown" /tmp/mail_348.out > /dev/null 2>&1
	TT1="$?"
	[ $TT1 -ne 0 ] 
	lputil verify "mail${TX}348" $?
	rm -f /usr/spool/mail/${USER} $HOME/.mailrc
	rm -f /tmp/mail_348.out
	echo;echo
}

mail_349()
{
	LPTEST=mail${TX}349
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the set nosendwait mail subcommand" 
	echo "set nosendwait" > $HOME/.mailrc

	# send some mail #
	mail ${USER} <<-EOF
		testing set nosendwait mail subcommand
	EOF
	sync;sync
	sleep 2

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}set nosendwait
	${waitch}$mail_prompt${waitch}mail cinderella 
	${waitch}Subject: ${waitch}testing nosendwait
	${waitch}$test_prompt${waitch}testing set nosendwait 
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
        } | tee -a ${RESULTS} | tee /tmp/mail_349.out
	grep "^cinderella... User unknown" /tmp/mail_349.out > /dev/null 2>&1
	TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "mail${TX}349" $?
	rm -f /usr/spool/mail/${USER} $HOME/.mailrc
	rm -f /tmp/mail_349.out
	echo;echo
}

mail_350()
{
	LPTEST=mail${TX}350
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the MBOX environment variable" 

	export MBOX=mail_350.out1
	# send some mail #
	mail ${USER} <<-EOF
		testing the MBOX environment variable 
	EOF
	sync;sync
	sleep 2

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
        } | tee -a ${RESULTS} | tee /tmp/mail_350.out
	grep "Saved 1 message in mail_350.out1" /tmp/mail_350.out > /dev/null 2>&1
	TT1="$?"
	grep "testing the MBOX environment variable" mail_350.out1 > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}350" $?
	rm -f /usr/spool/mail/${USER} 
	rm -f /tmp/mail_350.out mail_350.out1
	echo;echo
}

mail_351()
{
	LPTEST=mail${TX}351
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the MBOX environment variable" 

	export MBOX=$HOME/mail_351.out1
	# send some mail #
	mail ${USER} <<-EOF
		testing the MBOX environment variable 
	EOF
	sync;sync
	sleep 2

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
        } | tee -a ${RESULTS} | tee /tmp/mail_351.out
	grep "Saved 1 message in $HOME/mail_351.out1" /tmp/mail_351.out > /dev/null 2>&1
	TT1="$?"
	grep "testing the MBOX environment variable" $HOME/mail_351.out1 > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}351" $?
	rm -f /usr/spool/mail/${USER} 
	rm -f /tmp/mail_351.out $HOME/mail_351.out1
	unset MBOX
	echo;echo
}

mail_352()
{
	LPTEST=mail${TX}352
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the MBOX environment variable" 

	export MBOX=$HOME/mail_352.out1
	# send some mail #
	mail ${USER} <<-EOF
		testing the MBOX environment variable 
	EOF
	sync;sync
	sleep 2

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}exit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
        } | tee -a ${RESULTS} | tee /tmp/mail_352.out
	grep "Saved 1 message in $HOME/mail_352.out1" /tmp/mail_352.out > /dev/null 2>&1
	TT1="$?"
	grep "testing the MBOX environment variable" $HOME/mail_352.out1 > /dev/null 2>&1
	TT2="$?"
	grep "testing the MBOX environment variable" /usr/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	[ $TT1 -ne 0 ] && [ $TT2 -ne 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}352" $?
	rm -f /usr/spool/mail/${USER} $HOME/.mailrc
	rm -f /tmp/mail_352.out /tmp/mail_352.out1
	echo;echo
}

mail_353()
{
	LPTEST=mail${TX}353
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the MBOX environment variable" 

	export MBOX=$HOME/mail_353.out1
	# send some mail #
	mail ${USER} <<-EOF
		testing the MBOX environment variable 
	EOF
	sync;sync
	sleep 2

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}s /tmp/mail_353.out2
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
        } | tee -a ${RESULTS} | tee /tmp/mail_353.out
	grep "Saved 1 message in $HOME/myfile" /tmp/mail_353.out > /dev/null 2>&1
	TT1="$?"
	grep "testing the MBOX environment variable" $HOME/mail_353.out1 > /dev/null 2>&1
	TT2="$?"
	grep "testing the MBOX environment variable"  /tmp/mail_353.out2
	TT3="$?"
	[ $TT1 -ne 0 ] && [ $TT2 -ne 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}353" $?
	rm -f /usr/spool/mail/${USER} $HOME/.mailrc
	rm -f /tmp/mail_353.out /tmp/mail_353.out1 /tmp/mail_353.out2
	echo;echo
}

mail_354()
{
	LPTEST=mail${TX}354
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the DEAD environment variable with ~d" 

	export DEAD=saved.letter
	echo "this is the contents of saved.letter" > saved.letter

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*testing DEAD
	*$test_prompt*~d
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "testing DEAD" /var/spool/mail/${USER} >/dev/null 2>&1
	TT1="$?"
	grep "this is the contents of saved.letter" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "end of letter" /var/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] 
        lputil verify "mail${TX}354" $?
        rm -f /var/spool/mail/${USER} saved.letter
	unset DEAD
        echo;echo
}

mail_355()
{
        LPTEST=mail${TX}355
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the DEAD environment variable"
	
	export DEAD=/tmp/mail_355.out2

	# put something in dead.letter, so we can later test to ensure #
	# that the contents of dead.letter is overwritten when using ~q #
	echo "some stuff in dead.letter" > /tmp/mail_355.out2	

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER}
	*Subject: *testing mail
	*$test_prompt*testing the DEAD environment variable 
	*$test_prompt*~q
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_355.out
	grep "(Last Interrupt -- letter saved in /tmp/mail_355.out2)" /tmp/mail_355.out> /dev/null 2>&1
	TT1="$?"
	grep "testing the DEAD environment variable" /tmp/mail_355.out2 >/dev/null 2>&1
	TT2="$?"
	grep "some stuff in dead.letter" /tmp/mail_355.out2 > /dev/null 2>&1
	TT3="$?"
	grep "testing the ~q editing subcommand" /var/spool/mail/${USER} >/dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ] && [ $TT4 -ne 0 ]
        lputil verify "mail${TX}355" $?
        rm -f /var/spool/mail/${USER}
	rm -f /tmp/mail_355.out 
	rm -f /tmp/mail_355.out2
	unset DEAD
        echo;echo
}

mail_356()
{
        LPTEST=mail${TX}356
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number   - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~e editing subcommand"
	echo "set EDITOR=/usr/bin/vi" > $HOME/.mailrc
	
	$ask -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER}
	*Subject: *testing mail
	*$test_prompt*~e   
	*~*i hello joy
	* *nothing
	* * :wq
	* *$test_prompt
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	sleep 3
	grep "hello joy" /var/spool/mail/${USER} > /dev/null 2>&1
        lputil verify "mail${TX}356" $?
	rm -f /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
        echo;echo
}

mail_357()
{
        LPTEST=mail${TX}357
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number   - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~v editing subcommand"
	echo "set EDITOR=/usr/bin/vi"

	$ask -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER}
	*Subject: *testing mail
	*$test_prompt*~v
	*~*i hello joy 
	* *nothing
	* * :wq
	* *testing the ~v editing subcommand
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "hello joy" /var/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "testing the ~v editing subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
        lputil verify "mail${TX}357" $?
        rm -f /var/spool/mail/${USER}
	rm -f $HOME/.mailrc
        echo;echo
}

mail_358()
{
        LPTEST=mail${TX}358
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number   - $LPTEST"
        {
        echo "$LPTEST FVT testing the HOME environment variable"
	export HOME=/tmp
	
	# send some mail #
	mail ${USER} <<-EOF
		testing the HOME environment variable 
	EOF
	sync;sync
	sleep 2

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_358.out
	grep "testing the HOME environment variable" /tmp/mbox > /dev/null 2>&1
        lputil verify "mail${TX}358" $?
	rm -f $HOME/mbox /tmp/mail_358.out
	unset HOME
        echo;echo
}
	
	
mail_359()
{
        LPTEST=mail${TX}359
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number   - $LPTEST"
        echo "$LPTEST FVT testing the MAILRC environment variable"
	export MAILRC=/tmp/mail_359.out
	echo "set record=/tmp/mail_359.out2" > /tmp/mail_359.out
	
	# send some mail #
	mail ${USER} <<-EOF
		testing the MAILRC environment variable 
	EOF
	sync;sync
	sleep 2
	grep "testing the MAILRC environment variable" /tmp/mail_359.out2 >/dev/null 2>&1 
        lputil verify "mail${TX}359" $?
	rm -f /tmp/mail_359.out /tmp/mail_359.out2
	rm -f /var/spool/mail/${USER}
	unset MAILRC
	echo;echo
} 
	
mail_360()
{
	LPTEST=mail${TX}360
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the PAGER environment variable" 
	# send some mail first so we can get into mail mode #

	export PAGER=/usr/bin/more

	mail ${USER} <<-EOF
		testing the PAGER environment variable
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}page
	${waitch}$mail_prompt${waitch} quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_360.out
	grep "testing the PAGER environment variable" /tmp/mail_360.out > /dev/null 2>&1
	TT1="$?"
	#fgrep [H[JMessage /tmp/mail_360.out > /dev/null 2>&1
	fgrep "stdin: END" /tmp/mail_360.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}360"  $?
	rm -f /tmp/mail_360.out /usr/spool/mail/${USER}
	unset PAGER
	echo;echo
}

mail_361()
{
	LPTEST=mail${TX}361
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the PAGER environment variable" 
	# send some mail first so we can get into mail mode #

	echo "set crt=5" > $HOME/.mailrc
	export PAGER=/usr/bin/more

	mail ${USER} <<-EOF
		testing the PAGER environment variable
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_361.out
	grep "testing the PAGER environment variable" /tmp/mail_361.out > /dev/null 2>&1
	TT1="$?"
	fgrep [H[JMessage /tmp/mail_361.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}361"  $?
	rm -f /tmp/mail_361.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	unset PAGER
	echo;echo
}

mail_362()
{
	LPTEST=mail${TX}362
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the PAGER environment variable" 
	# send some mail first so we can get into mail mode #

	echo "set crt=20" > $HOME/.mailrc
	export PAGER=/usr/bin/more

	mail ${USER} <<-EOF
		testing the PAGER environment variable
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}type
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_362.out
	grep "testing the PAGER environment variable" /tmp/mail_362.out > /dev/null 2>&1
	TT1="$?"
	fgrep [H[JMessage /tmp/mail_362.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}362"  $?
	rm -f /tmp/mail_362.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	unset PAGER
	echo;echo
}

mail_363()
{
	LPTEST=mail${TX}363
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the SHELL environment variable" 

	# save the value that SHELL is currently set to.
	echo $SHELL > /tmp/mail_363.out2
	
	export SHELL=/bin/sh

	# send some mail first so we can get into mail mode #
	mail ${USER} <<-EOF
		testing the SHELL environment variable 
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}shell
	${waitch}${sh_prompt}${waitch}set -o vi
	${waitch}${sh_prompt}${waitch}exit
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_363.out
	grep -e "-o: bad option(s)" /tmp/mail_363.out > /dev/null 2>&1
	lputil verify "mail${TX}363" $?
	rm -f  /usr/spool/mail/${USER} $HOME/mbox /tmp/mail_363.out
	#set the SHELL back to it's original value
	export SHELL=`cat /tmp/mail_363.out2`
	rm -f /tmp/mail_363.out2
	echo;echo
}
	
mail_364()
{
        LPTEST=mail${TX}364
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number   - $LPTEST"
        {
        echo "$LPTEST FVT testing the VISUAL environment variable"
	export VISUAL=/usr/bin/vi

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER}
	*Subject: *testing mail
	*$test_prompt*~v
	*~*i hello joy 
	* *nothing
	* * :wq
	* *testing the VISUAL environment variable 
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	sleep 3
	grep "hello joy" /var/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "testing the VISUAL environment variable" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
        lputil verify "mail${TX}364" $?
        rm -f /var/spool/mail/${USER}
	unset VISUAL
        echo;echo
}

mail_365()
{
	LPTEST=mail${TX}365
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"

	echo "$LPTEST FVT testing the -e flag" 

	mail -e
	TT1="$?"	

	mail ${USER} <<-EOF
		testing the -e flag 
	EOF
	sleep 3
	mail -e
	TT2="$?"
	[ $TT1 = 1 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}365" $?
	rm -f  /usr/spool/mail/${USER} 
	echo;echo
}

mail_366()
{
	LPTEST=mail${TX}366
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"

	echo "$LPTEST FVT testing the -H flag" 

	mail ${USER} <<-EOF
		testing the -H flag 
	EOF
	sleep 3
	mail -H > /tmp/mail_366.out
	grep "Mail \[5.2 UCB\] \[AIX 4.1\]  Type ? for help." /tmp/mail_366.out >/dev/null 2>&1
	TT1="$?"
	grep '"/var/spool/mail/root": 1 message 1 new' /tmp/mail_366.out >/dev/null 2>&1
	TT2="$?"
	grep ">N  1 ${USER}" /tmp/mail_366.out > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}366" $?
	rm -f  /usr/spool/mail/${USER}  /tmp/mail_366.out
	echo;echo
}

mail_367()
{
	LPTEST=mail${TX}367
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"

	echo "$LPTEST FVT testing the -F flag" 

	mail -F ${USER} <<-EOF
		testing the -F flag 
	EOF
	sleep 3
	grep "testing the -F flag" $HOME/${USER}
	TT1="$?"
	lputil verify "mail${TX}367" $?
	rm -f  /var/spool/mail/${USER} $HOME/${USER}
	echo;echo
}

mail_368()
{
	LPTEST=mail${TX}368
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"

	echo "$LPTEST FVT testing the -F flag" 

	mail -F ${RUSER}@${RHOST} ${USER} <<-EOF
		testing the -F flag 
	EOF
	sleep 3
	grep "testing the -F flag" $HOME/${RUSER}@${RHOST} > /dev/null 2>&1
	TT1="$?"
	[ $TT1 = 0 ]
	lputil verify "mail${TX}368" $?
	rm -f  /var/spool/mail/${USER} $HOME/${RUSER}@${RHOST}
	echo;echo
}

mail_369()
{
	LPTEST=mail${TX}369
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"

	echo "$LPTEST FVT testing the -F flag" 
	echo "set record=/tmp/mail_369.out" > $HOME/.mailrc

	mail -F ${RUSER}@${RHOST} ${USER} <<-EOF
		testing the -F flag 
	EOF
	sleep 3
	grep "testing the -F flag" $HOME/${RUSER}@${RHOST} > /dev/null 2>&1
	TT1="$?"
	grep "testing the -F flag" /tmp/mail_369.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ]
	lputil verify "mail${TX}369" $?
	rm -f /var/spool/mail/${USER} $HOME/${RUSER}@${RHOST} > /dev/null 2>&1
	rm -f $HOME/.mailrc
	echo;echo
}

mail_370()
{
	LPTEST=mail${TX}370
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the askbcc subcommand" 
	echo "set askbcc" > $HOME/.mailrc

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail bugs
	${waitch}Subject: ${waitch} testing askbcc
	${waitch}$test_prompt${waitch}testing the askbcc subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}Bcc: ${waitch}$test_prompt
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync
	} | tee -a ${RESULTS} | tee /tmp/mail_370.out
	# the fact that the above happened says the askbcc command works
	# but let's double check since "ask" does go crazy often
	grep "Bcc:" /tmp/mail_370.out > /dev/null 2>&1
	lputil verify "mail${TX}370" $?
	rm -f  /usr/spool/mail/bugs $HOME/.mailrc /tmp/mail_370.out
	echo;echo
}

mail_371()
{
	LPTEST=mail${TX}371
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the askbcc subcommand" 
	echo "set askbcc" > $HOME/.mailrc

	# send some mail to get in mail mode
	mail ${USER} <<-EOF
		testing the set noaskbcc subcommand
	EOF
	sync;sync
	sleep 3

	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail 
	${waitch}$mail_prompt${waitch}set noaskbcc
	${waitch}$mail_prompt${waitch}mail bugs 
	${waitch}Subject: ${waitch} testing askbcc
	${waitch}$test_prompt${waitch}testing the askbcc subcommand
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync
	} | tee -a ${RESULTS} | tee /tmp/mail_371.out
	# the fact that the above happened says the askbcc command works
	# but let's double check since "ask" does go crazy often
	grep "Bcc:" /tmp/mail_371.out > /dev/null 2>&1
	TT1="$?"
	[ $TT1 -ne 0 ]
	lputil verify "mail${TX}371" $?
	rm -f  /usr/spool/mail/bugs $HOME/.mailrc /tmp/mail_371.out
	echo;echo
}

mail_372()
{
	LPTEST=mail${TX}372
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	echo "$LPTEST FVT testing the flipr mail subcommand" 

	echo "set flipr" > $HOME/.mailrc
	# send some mail first #
	mail -s test -c ${USER2} ${USER} <<-EOF
		testing the flipr mail subcommand
	EOF
	sync;sync;sync
	{ 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}respond
	${waitch}$test_prompt${waitch}test2
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sleep 3
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_372.out
	grep "To: ${USER}" /tmp/mail_372.out > /dev/null 2>&1
	TT1="$?"
	grep test2 /usr/spool/mail/${USER} >/dev/null 2>&1	
	TT2="$?"
	grep "Cc: ${USER2}" /tmp/mail_372.out > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}372" $?
	rm -f /tmp/mail_372.out  /usr/spool/mail/${USER} 
	rm -f /usr/spool/mail/${USER2} $HOME/.mailrc
	echo;echo
}

mail_373()
{
	LPTEST=mail${TX}373
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	echo "$LPTEST FVT testing the flipr mail subcommand" 
	echo "set flipr" > $HOME/.mailrc
	# send some mail first #
	mail -s test -c ${USER2} ${USER} <<-EOF
		testing the flipr mail subcommand 
	EOF
	sync;sync;sync
	{
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}Respond
	${waitch}$test_prompt${waitch}test2
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	sleep 3
	} | tee -a ${RESULTS} | tee /tmp/mail_373.out
	grep "To: ${USER}" /tmp/mail_373.out > /dev/null 2>&1
	TT1="$?"
	grep test2 /usr/spool/mail/${USER} >/dev/null 2>&1	
	TT2="$?"
	grep "Cc: ${USER2}" /tmp/mail_373.out > /dev/null 2>&1
	TT3="$?"
	grep test2 /usr/spool/mail/${USER2} > /dev/null 2>&1
	TT4="$?" 
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
	lputil verify "mail${TX}373" $?
	rm -f /tmp/mail_373.out  /usr/spool/mail/${USER} 
	rm -f /usr/spool/mail/${USER2} $HOME/.mailrc
	echo;echo
}

mail_374()
{
	LPTEST=mail${TX}374
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	echo "$LPTEST FVT testing the set noflipr mail subcommand" 
	echo "set flipr" > $HOME/.mailrc
	# send some mail first #
	mail -s test -c ${USER2} ${USER} <<-EOF
		testing the set noflipr mail subcommand 
	EOF
	sync;sync;sync
	{
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}set noflipr
	${waitch}$mail_prompt${waitch}respond
	${waitch}$test_prompt${waitch}test2
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	sleep 3
	} | tee -a ${RESULTS} | tee /tmp/mail_374.out
	grep "To: ${USER}" /tmp/mail_374.out > /dev/null 2>&1
	TT1="$?"
	grep test2 /usr/spool/mail/${USER} >/dev/null 2>&1	
	TT2="$?"
	grep "Cc: ${USER2}" /tmp/mail_374.out > /dev/null 2>&1
	TT3="$?"
	grep test2 /usr/spool/mail/${USER2} > /dev/null 2>&1
	TT4="$?" 
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
	lputil verify "mail${TX}374" $?
	rm -f /tmp/mail_374.out  /usr/spool/mail/${USER} 
	rm -f /usr/spool/mail/${USER2} $HOME/.mailrc
	echo;echo
}

mail_375()
{
	LPTEST=mail${TX}375
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	echo "$LPTEST FVT testing the bang mail subcommand" 
	echo "set bang" > $HOME/.mailrc
	# send some mail first #
	mail ${USER} <<-EOF
		testing the bang mail subcommand 
	EOF
	sync;sync;sync
	{
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}! uname 
	${waitch}$mail_prompt${waitch}! !
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	sleep 3
	} | tee -a ${RESULTS} | tee /tmp/mail_375.out
	grep "^!  uname" /tmp/mail_375.out > /dev/null 2>&1
	TT1="$?"
	UNAME=`uname`
	TT2=`grep  -c "$UNAME" /tmp/mail_375.out` 
	[ $TT1 = 0 ] && [ $TT2 = 2 ]
	lputil verify "mail${TX}375" $?
	rm -f /tmp/mail_375.out  /usr/spool/mail/${USER} 
	rm -f $HOME/.mailrc
	echo;echo
}

mail_376()
{
	LPTEST=mail${TX}376
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	echo "$LPTEST FVT testing the set nobang mail subcommand" 
	echo "set bang" > $HOME/.mailrc
	# send some mail first #
	mail ${USER} <<-EOF
		testing the bang mail subcommand 
	EOF
	sync;sync;sync
	{
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}set nobang
	${waitch}$mail_prompt${waitch}! uname 
	${waitch}$mail_prompt${waitch}! !
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	sleep 3
	} | tee -a ${RESULTS} | tee /tmp/mail_376.out
	grep "^! uname" /tmp/mail_376.out > /dev/null 2>&1
	TT1="$?"
	grep "/bin/ksh: syntax error at line 1 :" /tmp/mail_376.out > /dev/null 2>&1
	TT2="$?"
	UNAME=`uname`
	TT3=`grep  -c "$UNAME" /tmp/mail_376.out` 
	[ $TT1 -ne 0 ] && [ $TT2 = 0 ] && [ $TT3 = 1 ]
	lputil verify "mail${TX}376" $?
	rm -f /tmp/mail_376.out  /usr/spool/mail/${USER} 
	rm -f $HOME/.mailrc
	echo;echo
}

mail_377()
{
        LPTEST=mail${TX}377
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the bang mail subcommand"

	echo "set bang" > $HOME/.mailrc

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*test for the bang mail subcommand
	*$test_prompt*~!uname
	*$test_prompt*~! !
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_377.out
	UNAME=`uname`
	TT1=`grep -c "$UNAME" /tmp/mail_377.out` 
	grep "^! uname" /tmp/mail_377.out > /dev/null 2>&1
	TT2="$?"
	[ $TT1 = 2 ] && [ $TT2 = 0 ] 
        lputil verify "mail${TX}377" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_377.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_378()
{
        LPTEST=mail${TX}378
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the bang mail subcommand default"

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*test for the bang mail subcommand
	*$test_prompt*~!uname
	*$test_prompt*~! !
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_378.out
	UNAME=`uname`
	TT1=`grep -c "$UNAME" /tmp/mail_378.out` 
	grep "^! uname" /tmp/mail_378.out > /dev/null 2>&1
	TT2="$?"
	grep "/bin/ksh: syntax error at line 1 :" /tmp/mail_378.out > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 1 ] && [ $TT2 -ne 0 ] && [ $TT3 = 0 ]
        lputil verify "mail${TX}378" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_378.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_379()
{
        LPTEST=mail${TX}379
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the indentprefix subcommand"
	echo "set indentprefix=>" > $HOME/.mailrc
	#send some mail first to get in mail mode
	mail ${USER} <<-EOF
		testing the indentprefix subcommand
	EOF
	sync;sync
	sleep 3
	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail 
	*$mail_prompt*mail bugs
	*Subject: *testing mail
	*$test_prompt*testing 
	*$test_prompt*~m
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*$mail_prompt*quit
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "^>testing the indentprefix subcommand" /var/spool/mail/bugs > /dev/null 2>&1
	TT1="$?"
	grep "^>From" /var/spool/mail/bugs > /dev/null 2>&1
	TT2="$?"
	grep "^>Date: " /var/spool/mail/bugs > /dev/null 2>&1
	TT3="$?"
	grep "^>From: " /var/spool/mail/bugs > /dev/null 2>&1
	TT4="$?"
	grep "^>To: " /var/spool/mail/bugs > /dev/null 2>&1
	TT5="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ]  
        lputil verify "mail${TX}379" $?
	rm -f /var/spool/mail/bugs /tmp/mail_379.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_380()
{
        LPTEST=mail${TX}380
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the set noindentprefix subcommand"
	echo "set indentprefix=>" > $HOME/.mailrc
	#send some mail first to get in mail mode
	mail ${USER} <<-EOF
		testing the set noindentprefix subcommand
	EOF
	sync;sync
	sleep 3
	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail 
	*$mail_prompt*set noindentprefix
	*$mail_prompt*mail bugs
	*Subject: *testing mail
	*$test_prompt*testing 
	*$test_prompt*~m
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*$mail_prompt*quit
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "^	testing the set noindentprefix subcommand" /var/spool/mail/bugs > /dev/null 2>&1
	TT1="$?"
	grep "^	From" /var/spool/mail/bugs > /dev/null 2>&1
	TT2="$?"
	grep "^	Date: " /var/spool/mail/bugs > /dev/null 2>&1
	TT3="$?"
	grep "^	From: " /var/spool/mail/bugs > /dev/null 2>&1
	TT4="$?"
	grep "^	To: " /var/spool/mail/bugs > /dev/null 2>&1
	TT5="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ]  
        lputil verify "mail${TX}380" $?
	rm -f /var/spool/mail/bugs /tmp/mail_380.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_381()
{
        LPTEST=mail${TX}381
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the set sign subcommand"
	echo 'set sign="Snow White and Dwarfs"' > $HOME/.mailrc

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*testing sign subcommand
	*$test_prompt*~a
	*$test_prompt*the end
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_381.out
	grep "^testing sign subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "^Snow White and Dwarfs" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "^the end" /var/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	grep "^Snow White and Dwarfs" /tmp/mail_381.out > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
        lputil verify "mail${TX}381" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_381.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_382()
{
        LPTEST=mail${TX}382
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the set sign subcommand"
	echo 'set sign="Snow White\\n\\tand\\nThe Seven Dwarfs"' > $HOME/.mailrc

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*testing sign subcommand
	*$test_prompt*~a
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_382.out
	grep "^testing sign subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "^Snow White" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "	and" /var/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	grep "^The Seven Dwarfs" /var/spool/mail/${USER} > /dev/null 2>&1
	TT4="$?"
	grep "^Snow White" /tmp/mail_382.out > /dev/null 2>&1
	TT5="$?"
	grep "	and" /tmp/mail_382.out > /dev/null 2>&1
	TT6="$?"
	grep "^The Seven Dwarfs" /tmp/mail_382.out > /dev/null 2>&1
	TT7="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] && [ $TT6 = 0 ] && [ $TT7 = 0 ]
        lputil verify "mail${TX}382" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_382.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_383()
{
        LPTEST=mail${TX}383
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the set sign subcommand"
	echo 'set sign="Snow White\\b\\land\\nThe Seven Dwarfs"' > $HOME/.mailrc

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*testing sign subcommand
	*$test_prompt*~a
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_383.out
	grep "^testing sign subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	fgrep "Snow White\b\land" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "^The Seven Dwarfs" /var/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	fgrep "Snow White\b\land" /tmp/mail_383.out > /dev/null 2>&1
	TT4="$?"
	grep "^The Seven Dwarfs" /tmp/mail_383.out > /dev/null 2>&1
	TT5="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] 
        lputil verify "mail${TX}383" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_383.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_384()
{
        LPTEST=mail${TX}384
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the set sign subcommand"

	# send some mail first to get in mail mode
	mail ${USER} <<-EOF
		testing the set sign subcommand
	EOF
	sync;sync
	sleep 3

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail 
	*$mail_prompt*set sign="Snow White\b\l\tand\nThe Seven Dwarfs"
	*$mail_prompt*mail bugs
	*Subject: *testing mail
	*$test_prompt*testing sign subcommand
	*$test_prompt*~a
	*$test_prompt*.
	*Cc: *$test_prompt
	*$mail_prompt*quit
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_384.out
	grep "^testing sign subcommand" /var/spool/mail/bugs > /dev/null 2>&1
	TT1="$?"
	fgrep "Snow White\b\l	and" /var/spool/mail/bugs > /dev/null 2>&1
	TT2="$?"
	grep "^The Seven Dwarfs" /var/spool/mail/bugs > /dev/null 2>&1
	TT3="$?"
	fgrep "Snow White\b\l	and" /tmp/mail_384.out > /dev/null 2>&1
	TT4="$?"
	grep "^The Seven Dwarfs" /tmp/mail_384.out > /dev/null 2>&1
	TT5="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] 
        lputil verify "mail${TX}384" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_384.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_385()
{
        LPTEST=mail${TX}385
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the set nosign subcommand"
	echo set sign="Snow White" > $HOME/.mailrc

	# send some mail first to get in mail mode
	mail ${USER} <<-EOF
		testing the set nosign subcommand
	EOF
	sync;sync
	sleep 3

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail 
	*$mail_prompt*set nosign
	*$mail_prompt*mail bugs
	*Subject: *testing mail
	*$test_prompt*testing nosign subcommand
	*$test_prompt*~a
	*$test_prompt*.
	*Cc: *$test_prompt
	*$mail_prompt*quit
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_385.out
	grep "testing nosign subcommand" /var/spool/mail/bugs > /dev/null 2>&1
	TT1="$?"
	grep "^Snow White" /var/spool/mail/bugs > /dev/null 2>&1
	TT2="$?"
	grep "^Snow White" /tmp/mail_385.out > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 -ne 0 ]
        lputil verify "mail${TX}385" $?
	rm -f /var/spool/mail/bugs /tmp/mail_385.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_386()
{
        LPTEST=mail${TX}381
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the set Sign subcommand"
	echo 'set Sign="Snow White and Dwarfs"' > $HOME/.mailrc

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*testing Sign subcommand
	*$test_prompt*~A
	*$test_prompt*the end
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_386.out
	grep "^testing Sign subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "^Snow White and Dwarfs" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "^the end" /var/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	grep "^Snow White and Dwarfs" /tmp/mail_386.out > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
        lputil verify "mail${TX}386" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_386.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_387()
{
        LPTEST=mail${TX}387
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the set Sign subcommand"
	echo 'set Sign="Snow White\\n\\tand\\nThe Seven Dwarfs"' > $HOME/.mailrc

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*testing Sign subcommand
	*$test_prompt*~A
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_387.out
	grep "^testing Sign subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "^Snow White" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "	and" /var/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	grep "^The Seven Dwarfs" /var/spool/mail/${USER} > /dev/null 2>&1
	TT4="$?"
	grep "^Snow White" /tmp/mail_387.out > /dev/null 2>&1
	TT5="$?"
	grep "	and" /tmp/mail_387.out > /dev/null 2>&1
	TT6="$?"
	grep "^The Seven Dwarfs" /tmp/mail_387.out > /dev/null 2>&1
	TT7="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] && [ $TT6 = 0 ] && [ $TT7 = 0 ]
        lputil verify "mail${TX}387" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_387.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_388()
{
        LPTEST=mail${TX}388
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the set Sign subcommand"
	echo 'set Sign="Snow White\\b\\land\\nThe Seven Dwarfs"' > $HOME/.mailrc

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*testing Sign subcommand
	*$test_prompt*~A
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_388.out
	grep "^testing Sign subcommand" /var/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	fgrep "Snow White\b\land" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "^The Seven Dwarfs" /var/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	fgrep "Snow White\b\land" /tmp/mail_388.out > /dev/null 2>&1
	TT4="$?"
	grep "^The Seven Dwarfs" /tmp/mail_388.out > /dev/null 2>&1
	TT5="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] 
        lputil verify "mail${TX}388" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_388.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_389()
{
        LPTEST=mail${TX}389
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the set Sign subcommand"

	# send some mail first to get in mail mode
	mail ${USER} <<-EOF
		testing the set Sign subcommand
	EOF
	sync;sync
	sleep 3

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail 
	*$mail_prompt*set Sign="Snow White\b\l\tand\nThe Seven Dwarfs"
	*$mail_prompt*mail bugs
	*Subject: *testing mail
	*$test_prompt*testing Sign subcommand
	*$test_prompt*~A
	*$test_prompt*.
	*Cc: *$test_prompt
	*$mail_prompt*quit
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_389.out
	grep "^testing Sign subcommand" /var/spool/mail/bugs > /dev/null 2>&1
	TT1="$?"
	fgrep "Snow White\b\l	and" /var/spool/mail/bugs > /dev/null 2>&1
	TT2="$?"
	grep "^The Seven Dwarfs" /var/spool/mail/bugs > /dev/null 2>&1
	TT3="$?"
	fgrep "Snow White\b\l	and" /tmp/mail_389.out > /dev/null 2>&1
	TT4="$?"
	grep "^The Seven Dwarfs" /tmp/mail_389.out > /dev/null 2>&1
	TT5="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] 
        lputil verify "mail${TX}389" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_389.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_390()
{
        LPTEST=mail${TX}390
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the set noSign subcommand"
	echo 'set Sign="Snow White"' > $HOME/.mailrc

	# send some mail first to get in mail mode
	mail ${USER} <<-EOF
		testing the set noSign subcommand
	EOF
	sync;sync
	sleep 3

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail 
	*$mail_prompt*set noSign
	*$mail_prompt*mail bugs
	*Subject: *testing mail
	*$test_prompt*testing noSign subcommand
	*$test_prompt*~A
	*$test_prompt*.
	*Cc: *$test_prompt
	*$mail_prompt*quit
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_390.out
	grep "^testing noSign subcommand" /var/spool/mail/bugs > /dev/null 2>&1
	TT1="$?"
	grep "^Snow White" /var/spool/mail/bugs > /dev/null 2>&1
	TT2="$?"
	grep "^Snow White" /tmp/mail_390.out > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 -ne 0 ]
        lputil verify "mail${TX}390" $?
	rm -f /var/spool/mail/bugs /tmp/mail_390.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_391()
{
        LPTEST=mail${TX}391
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~i"
	echo 'set sign="Snow White and Dwarfs"' > $HOME/.mailrc

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*testing tilde i 
	*$test_prompt*~i sign
	*$test_prompt*the end
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_391.out
	grep "^testing tilde i" /var/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "^Snow White and Dwarfs" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "^the end" /var/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	grep "^Snow White and Dwarfs" /tmp/mail_391.out > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
        lputil verify "mail${TX}391" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_391.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_392()
{
        LPTEST=mail${TX}392
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~i"
	echo 'set Sign="Snow White and Dwarfs"' > $HOME/.mailrc

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*testing tilde i 
	*$test_prompt*~i Sign
	*$test_prompt*the end
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_392.out
	grep "^testing tilde i" /var/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "^Snow White and Dwarfs" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "^the end" /var/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	grep "^Snow White and Dwarfs" /tmp/mail_392.out > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
        lputil verify "mail${TX}392" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_392.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_393()
{
        LPTEST=mail${TX}393
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~i"
	echo 'set sign="Snow White and Dwarfs"' > $HOME/.mailrc

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*testing tilde i 
	*$test_prompt*~i 
	*$test_prompt*the end
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_393.out
	grep "^testing tilde i" /var/spool/mail/${USER} > /dev/null 2>&1
	TT1="$?"
	grep "^Snow White and Dwarfs" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "^the end" /var/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	grep "^Snow White and Dwarfs" /tmp/mail_393.out > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 = 0 ] && [ $TT4 -ne 0 ]
        lputil verify "mail${TX}393" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_393.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_394()
{
        LPTEST=mail${TX}394
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~i"
	echo 'set sign="Snow White and Dwarfs"' > $HOME/.mailrc

	#send some mail to get in mail mode
	mail ${USER} <<-EOF
		testing the tilde i
	EOF
	sync;sync
	sleep 3

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail 
	*$mail_prompt*unset sign
	*$mail_prompt*mail bugs
	*Subject: *testing mail
	*$test_prompt*testing tilde i 
	*$test_prompt*~i sign
	*$test_prompt*the end
	*$test_prompt*.
	*Cc: *$test_prompt
	*$mail_prompt*quit
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_394.out
	grep "^testing tilde i" /var/spool/mail/bugs > /dev/null 2>&1
	TT1="$?"
	grep "^Snow White and Dwarfs" /var/spool/mail/bugs > /dev/null 2>&1
	TT2="$?"
	grep "^the end" /var/spool/mail/bugs > /dev/null 2>&1
	TT3="$?"
	grep "^Snow White and Dwarfs" /tmp/mail_394.out > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 = 0 ] && [ $TT4 -ne 0 ]
        lputil verify "mail${TX}394" $?
	rm -f /var/spool/mail/${USER} /tmp/mail_394.out
	rm -f $HOME/.mailrc
	echo;echo
}

mail_395()
{
        LPTEST=mail${TX}395
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~F editing subcommand"
	echo "ignore Subject" > $HOME/.mailrc
	# first send a few pieces of mail #
	mail -s test_395 ${USER} <<-EOF
		test1 for testing the ~F editing subcommand
	EOF
	sync;sync
	sleep 2

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail 
	*$mail_prompt*mail bugs
	*Subject: *testing mail
	*$test_prompt*test for the ~F editing subcommand
	*$test_prompt*~F 
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*$mail_prompt*quit
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "test for the ~F editing subcommand" /var/spool/mail/bugs >/dev/null 2>&1
	TT1="$?"
	grep "^test1 for testing the ~F editing subcommand" /var/spool/mail/bugs > /dev/null 2>&1
	TT2="$?"
	grep "end of letter" /var/spool/mail/bugs > /dev/null 2>&1
	TT3="$?"
	grep "Subject: test_395" /var/spool/mail/bugs >/dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
        lputil verify "mail${TX}395" $?
        rm -f /var/spool/mail/${USER} /var/spool/mail/bugs $HOME/.mailrc
        echo;echo
}

mail_396()
{
        LPTEST=mail${TX}396
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~F editing subcommand"
	echo "ignore Subject" > $HOME/.mailrc
	# first send a few pieces of mail #
	mail -s test_396 ${USER} <<-EOF
		test1 for testing the ~F editing subcommand
	EOF
	sync;sync
	mail -s test_396_2  ${USER} <<-EOF
		test2 for testing the ~F editing subcommand
	EOF
	sync;sync
	sleep 2

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail 
	*$mail_prompt*mail bugs
	*Subject: *testing mail
	*$test_prompt*test for the ~F editing subcommand
	*$test_prompt*~F 1-2 
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*$mail_prompt*quit
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "test for the ~F editing subcommand" /var/spool/mail/bugs >/dev/null 2>&1
	TT1="$?"
	grep "^test1 for testing the ~F editing subcommand" /var/spool/mail/bugs > /dev/null 2>&1
	TT2="$?"
	grep "^test2 for testing the ~F editing subcommand" /var/spool/mail/bugs > /dev/null 2>&1
	TT3="$?"
	grep "end of letter" /var/spool/mail/bugs > /dev/null 2>&1
	TT4="$?"
	grep "Subject: test_396" /var/spool/mail/bugs > /dev/null 2>&1
	TT5="$?"
	grep "Subject: test_396_2" /var/spool/mail/bugs > /dev/null 2>&1
	TT6="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] && [ $TT6 = 0 ] 
        lputil verify "mail${TX}396" $?
        rm -f /var/spool/mail/${USER} /var/spool/mail/bugs $HOME/.mailrc
        echo;echo
}

mail_397()
{
        LPTEST=mail${TX}397
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~M editing subcommand"
	echo "set indentprefix=>>>\nignore Subject" > $HOME/.mailrc
	# first send a few pieces of mail #
	mail -s test_397 ${USER} <<-EOF
		test1 for testing the ~M editing subcommand
	EOF
	sync;sync
	sleep 2

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail 
	*$mail_prompt*mail bugs
	*Subject: *testing mail
	*$test_prompt*test for the ~M editing subcommand
	*$test_prompt*~M
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*$mail_prompt*quit
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "^test for the ~M editing subcommand" /var/spool/mail/bugs >/dev/null 2>&1
	TT1="$?"
	grep ">>>test1 for testing the ~M editing subcommand" /var/spool/mail/bugs > /dev/null 2>&1
	TT2="$?"
	grep "^end of letter" /var/spool/mail/bugs > /dev/null 2>&1
	TT3="$?"
	grep ">>>Subject: test_397" /var/spool/mail/bugs > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ]
        lputil verify "mail${TX}397" $?
        rm -f /var/spool/mail/${USER} /var/spool/mail/bugs $HOME/.mailrc
        echo;echo
}

mail_398()
{
        LPTEST=mail${TX}398
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~M editing subcommand"
	# first send a few pieces of mail #
	mail -s test_398 ${USER} <<-EOF
		test1 for testing the ~M editing subcommand
	EOF
	sync;sync
	mail -s test_398_2 ${USER} <<-EOF
		test2 for testing the ~M editing subcommand
	EOF
	sync;sync
	sleep 2

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail 
	*$mail_prompt*mail bugs
	*Subject: *testing mail
	*$test_prompt*test for the ~M editing subcommand
	*$test_prompt*~M 1-2 
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*$mail_prompt*quit
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "^test for the ~M editing subcommand" /var/spool/mail/bugs >/dev/null 2>&1
	TT1="$?"
	grep "	test1 for testing the ~M editing subcommand" /var/spool/mail/bugs > /dev/null 2>&1
	TT2="$?"
	grep "^	test2 for testing the ~M editing subcommand" /var/spool/mail/bugs > /dev/null 2>&1
	TT3="$?"
	grep "end of letter" /var/spool/mail/bugs > /dev/null 2>&1
	TT4="$?"
	grep "	Subject: test_398" /var/spool/mail/bugs >/dev/null 2>&1
	TT5="$?"
	grep "	Subject: test_398_2" /var/spool/mail/bugs >/dev/null 2>&1
	TT6="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] && [ $TT5 = 0 ] && [ $TT6 = 0 ]
        lputil verify "mail${TX}398" $?
	rm -f /var/spool/mail/${USER} /var/spool/mail/bugs $HOME/.mailrc
	echo;echo
}

mail_399()
{
        LPTEST=mail${TX}399
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~< editing subcommand"
	echo "this is the contents of /tmp/mail_399.file" > /tmp/mail_399.file 

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*test for the ~< editing subcommand
	*$test_prompt*~< /tmp/mail_399.file
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "test for the ~< editing subcommand" /var/spool/mail/${USER} >/dev/null 2>&1
	TT1="$?"
	grep "this is the contents of /tmp/mail_399.file" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "end of letter" /var/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] 
        lputil verify "mail${TX}399" $?
        rm -f /var/spool/mail/${USER}
        echo;echo
}

mail_400()
{
        LPTEST=mail${TX}400
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~<! editing subcommand"
	echo "this is the contents of /tmp/mail_400.file" > /tmp/mail_400.file 

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER} 
	*Subject: *testing mail
	*$test_prompt*test for the ~<! editing subcommand
	*$test_prompt*~<! cat /tmp/mail_400.file
	*$test_prompt*end of letter 
	*$test_prompt*.
	*Cc: *$test_prompt
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS}
	grep "test for the ~<! editing subcommand" /var/spool/mail/${USER} >/dev/null 2>&1
	TT1="$?"
	grep "this is the contents of /tmp/mail_400.file" /var/spool/mail/${USER} > /dev/null 2>&1
	TT2="$?"
	grep "end of letter" /var/spool/mail/${USER} > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] 
        lputil verify "mail${TX}400" $?
        rm -f /var/spool/mail/${USER}
        echo;echo
}

mail_401()
{
        LPTEST=mail${TX}401
        rm -f /tmp/${LPTEST}
        echo "Started `date` - mail TEST Number - $LPTEST"
        {
        echo "$LPTEST FVT testing the ~x editing subcommand"

	$ask_loc -w* -t5 <<-EOF
	*${sh_prompt}*mail ${USER}
	*Subject: *testing mail
	*$test_prompt*testing the ~x editing subcommand
	*$test_prompt*~x
	*${sh_prompt}*exit
	EOF
        sync;sync
        } | tee -a ${RESULTS} | tee -a /tmp/mail_401.out
	grep "(Last Interrupt -- letter saved in dead.letter)" /tmp/mail_401.out> /dev/null 2>&1
	TT1="$?"
	grep "testing the ~x editing subcommand" $HOME/dead.letter >/dev/null 2>&1
	TT2="$?"
	grep "testing the ~x editing subcommand" /var/spool/mail/${USER} >/dev/null 2>&1
	TT3="$?"
	[ $TT1 -ne 0 ] && [ $TT2 -ne 0 ] && [ $TT3 -ne 0 ]
        lputil verify "mail${TX}401" $?
        rm -f /var/spool/mail/${USER} /tmp/mail_401.out
        echo;echo
}

mail_402()
{
	LPTEST=mail${TX}402
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the Copy mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the "Copy" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		testing the "Copy" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}Copy 1-2
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	} | tee -a ${RESULTS} | tee /tmp/mail_402.out
	grep "\"$HOME/${USER}\" \[New file\]" /tmp/mail_402.out > /dev/null 2>&1
	TT1="$?"
	grep "Saved 2 messages in $HOME/mbox" /tmp/mail_402.out >/dev/null 2>&1
	TT2="$?"

	# nothing but the Status field should be different in mbox and #
	# mail_402.out1. If TT4 = 0, then our copy was successful #
	# and nothing was deleted since it was put in $HOME/mbox  #
	diff $HOME/${USER} $HOME/mbox | grep -v Status | grep "<" > /tmp/mail_402.out2
	TT3=`ls -l /tmp/mail_402.out2 | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] 
	lputil verify "mail${TX}402" $?
	rm -f /tmp/mail_402*  /usr/spool/mail/${USER} $HOME/mbox
	rm -f $HOME/${USER}
	echo;echo
}

mail_403()
{
	LPTEST=mail${TX}403
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the Copy mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the mail subcommand, "Copy".
	EOF
	sync;sync	
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}Copy 
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_403.out
	grep "\"$HOME/${USER}\" \[New file\]" /tmp/mail_403.out > /dev/null 2>&1
	TT1="$?"
	grep "Saved 1 message in $HOME/mbox" /tmp/mail_403.out >/dev/null 2>&1
	TT2="$?"
	# nothing but the Status field should be different in mbox and #
	# mail_403.out1. If TT4 = 0, then our copy was successful #
	# and nothing was deleted since it was put in $HOME/mbox  #
	diff $HOME/${USER} $HOME/mbox | grep -v Status | grep "<" > /tmp/mail_403.out2
	TT3=`ls -l /tmp/mail_403.out2 | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ]
	lputil verify "mail${TX}403" $?
	rm -f /tmp/mail_403*  /usr/spool/mail/${USER} $HOME/mbox
	rm -f $HOME/${USER}
	echo;echo
}

mail_404()
{
	LPTEST=mail${TX}404
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the copy mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the mail subcommand, "Copy".
	EOF
	sync;sync	
	mail ${USER} <<-EOF
		testing the mail subcommand, "Copy"
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}Copy 1 
	${waitch}$mail_prompt${waitch}Copy 2 
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_404.out
	grep "\"$HOME/${USER}\" \[New file\]" /tmp/mail_404.out > /dev/null 2>&1
	TT1="$?"
	grep "Saved 2 messages in $HOME/mbox" /tmp/mail_404.out >/dev/null 2>&1
	TT2="$?"
	grep "\"$HOME/${USER}\" \[Appended\]" /tmp/mail_404.out >/dev/null 2>&1
	TT3="$?"
	# nothing but the Status field should be different in mbox and #
	# mail_404.out1. If TT4 = 0, then our copy was successful #
	# and nothing was deleted since it was put in $HOME/mbox  #
	diff $HOME/${USER} $HOME/mbox | grep -v Status | grep "<" > /tmp/mail_404.out2
	TT4=`ls -l /tmp/mail_404.out2 | awk '{ print $5 }'`
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] 
	lputil verify "mail${TX}404" $?
	rm -f /tmp/mail_404*  /usr/spool/mail/${USER} $HOME/mbox
	rm -f $HOME/${USER}
	echo;echo
}

mail_405()
{
	LPTEST=mail${TX}405
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the Save mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		test1 for testing the "Save" mail subcommand
	EOF
	sync;sync
	mail ${USER} <<-EOF
		test2 for testing the "Save" mail subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}Save 1-2 
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync 
	} | tee -a ${RESULTS} | tee /tmp/mail_405.out
	grep "\"$HOME/${USER}\" \[New file\]" /tmp/mail_405.out > /dev/null 2>&1 
	TT1="$?"
	grep 'test1 for testing the "Save" mail subcommand' $HOME/${USER} >/dev/null 2>&1
	TT2="$?"
	grep 'test2 for testing the "Save" mail subcommand' $HOME/${USER} >/dev/null 2>&1
	TT3="$?"
	TT4=`grep -c "Received:" $HOME/${USER}`
	TT5=`grep -c "Date:" $HOME/${USER}`
	TT6=`grep -c "From:" $HOME/${USER}`
	TT7=`grep -c "Message-Id:" $HOME/${USER}`
	test -f $HOME/mbox
	TT8="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 2 ] && [ $TT5 = 2 ] && [ $TT6 = 2 ] && [ $TT7 = 2 ] && [ $TT8 -ne 0 ]
	lputil verify "mail${TX}405" $?
	rm -f /tmp/mail_405*  /usr/spool/mail/${USER} $HOME/mbox
	rm -f $HOME/${USER}
	echo;echo
}

mail_406()
{
	LPTEST=mail${TX}406
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the Save mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the mail subcommand, "Save".
	EOF
	sync;sync	
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}Save
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	} | tee -a ${RESULTS} | tee /tmp/mail_406.out
	grep "\"$HOME/${USER}\" \[New file\]" /tmp/mail_406.out >/dev/null 2>&1
	TT1="$?"
	test -f $HOME/mbox
	TT2="$?"
	TT3=`grep -c "Received:" $HOME/${USER}`
	TT4=`grep -c "Date:" $HOME/${USER}`
	TT5=`grep -c "From:" $HOME/${USER}`
	TT6=`grep -c "Message-Id:" $HOME/${USER}`
	[ $TT1 = 0 ] && [ $TT2 -ne 0 ] && [ $TT3 = 1 ] && [ $TT4 = 1 ] && [ $TT5 = 1 ] && [ $TT6 = 1 ] 
	lputil verify "mail${TX}406" $?
	rm -f /tmp/mail_406*  /usr/spool/mail/${USER} $HOME/mbox
	rm -f $HOME/${USER}
	echo;echo
}

mail_407()
{
	LPTEST=mail${TX}407
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the Save mail subcommand" 
	# send some mail first #
	mail ${USER} <<-EOF
		testing the mail subcommand, "Save".
	EOF
	sync;sync	
	mail ${USER} <<-EOF
		testing the mail subcommand, "Save"
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}Save 1 
	${waitch}$mail_prompt${waitch}Save 2 
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_407.out
	grep "\"$HOME/${USER}\" \[New file\]" /tmp/mail_407.out >/dev/null 2>&1 
	TT1="$?"
	grep "\"$HOME/${USER}\" \[New file\]" /tmp/mail_407.out > /dev/null 2>&1
	TT2="$?"
	test -f $HOME/mbox
	TT3="$?"
	TT4=`grep -c "Received:" $HOME/${USER}`
	TT5=`grep -c "Date:" $HOME/${USER}`
	TT6=`grep -c "From:" $HOME/${USER}`
	TT7=`grep -c "Message-Id:" $HOME/${USER}`
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ] && [ $TT4 = 2 ] && [ $TT5 = 2 ] && [ $TT6 = 2 ] && [ $TT7 = 2 ] 
	lputil verify "mail${TX}407" $?
	rm -f /tmp/mail_407*  /usr/spool/mail/${USER} $HOME/mbox
	rm -f $HOME/${USER}
	echo;echo
}

mail_408()
{
	LPTEST=mail${TX}408
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the followup subcommand" 
	# send some mail first #
	mail -s test_408 -c bugs root <<-EOF
		testing the followup subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}followup
	${waitch}$test_prompt${waitch}test2
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	sleep 3
	} | tee -a ${RESULTS}
	grep test2 /var/spool/mail/${USER}> /dev/null 2>&1
	TT1="$?"
	grep test2 $HOME/${USER} >/dev/null 2>&1	
	TT2="$?"
	grep test2 /var/spool/mail/bugs > /dev/null 2>&1
	TT3="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ]
	lputil verify "mail${TX}408" $?
	rm -f /usr/spool/mail/${USER} $HOME/mbox
	rm -f /usr/spool/mail/bugs $HOME/${USER}
	echo;echo
}

mail_409()
{
	LPTEST=mail${TX}409
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the followup subcommand" 
	echo "set record=/tmp/mail_409.out" > $HOME/.mailrc
	# send some mail first #
	mail -s test_409 -c bugs root <<-EOF
		testing the followup subcommand
	EOF
	sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}followup
	${waitch}Subject: Re:  test_409${waitch}test2
	${waitch}$test_prompt${waitch}test2
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sleep 3
	} | tee -a ${RESULTS}
	grep test2 /usr/spool/mail/${USER}> /dev/null 2>&1
	TT1="$?"
	grep test2 $HOME/${USER} >/dev/null 2>&1	
	TT2="$?"
	grep test2 /usr/spool/mail/bugs > /dev/null 2>&1
	TT3="$?"
	grep test2 /tmp/mail_409.out > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 -ne 0 ] && [ $TT4 -ne 0 ]
	lputil verify "mail${TX}409" $?
	rm -f /var/spool/mail/${USER} $HOME/mbox
	rm -f /var/spool/mail/bugs $HOME/${USER} /tmp/mail_409.out
	echo;echo
}

mail_410()
{
	LPTEST=mail${TX}410
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{
	echo "$LPTEST FVT testing the Followup subcommand" 

	# put some mail from different users in /var/spool/mail/${USER}

	echo "From root Fri Feb 14 10:03:36 1992
Received: by ${LHOST}.${LIPADDR} (AIX 3.2/UCB 5.64/4.03)
	  id AA13448; Fri, 14 Feb 1992 10:03:36 -0600
Date: Fri, 14 Feb 1992 10:03:36 -0600
From: bugs 
To: ${USER} 
Subject: test_410        

This is the message body for test_410"  >  /var/spool/mail/${USER}
	sync
	
	echo "\nFrom root Fri Feb 14 10:03:36 1992
Received: by ${LHOST}.${LIPADDR} (AIX 3.2/UCB 5.64/4.03)
	  id AA13448; Fri, 14 Feb 1992 10:03:36 -0600
Date: Fri, 14 Feb 1992 10:03:36 -0600
From: ${USER2} 
To: ${USER} 
Subject: test_2        

This is the message body for test_2" >>  /var/spool/mail/${USER}
	sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}Followup 1-2
	${waitch}$test_prompt${waitch}testing Followup
	${waitch}$test_prompt${waitch}.
	${waitch}Cc: ${waitch}$test_prompt
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	sleep 3
	} | tee -a ${RESULTS} | tee -a /tmp/mail_410.out
	grep "testing Followup" /var/spool/mail/bugs > /dev/null 2>&1
	TT1="$?"
	grep "testing Followup" /var/spool/mail/${USER2} > /dev/null 2>&1
	TT2="$?"
	grep "To: bugs ${USER2}" /tmp/mail_410.out > /dev/null 2>&1
	TT3="$?"
	grep "Subject: Re:  test_410" /tmp/mail_410.out > /dev/null 2>&1
	TT4="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ] && [ $TT3 = 0 ] && [ $TT4 = 0 ] 
	lputil verify "mail${TX}410" $?
	rm -f /usr/spool/mail/${USER} $HOME/mbox /tmp/mail_410.out
	rm -f /usr/spool/mail/bugs $HOME/${USER}
	echo;echo
}

mail_411()
{
	LPTEST=mail${TX}411
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the | (pipe) subcommand" 
	echo 'set "cmd=grep From"' > $HOME/.mailrc

	# send some mail so we can get in mail mode #
	mail ${USER} <<-EOF
		testing the | (pipe) subcommand 
	EOF
	sync;sync 
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}|
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_411.out
	grep "From: ${USER}" /tmp/mail_411.out > /dev/null 2>&1
	lputil verify "mail${TX}411" $?
	rm -f /tmp/mail_411.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	echo;echo
}

mail_412()
{
	LPTEST=mail${TX}412
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the | (pipe) subcommand" 

	echo "From root Fri Feb 14 10:03:36 1992
Received: by ${LHOST}.${LIPADDR} (AIX 3.2/UCB 5.64/4.03)
	  id AA13448; Fri, 14 Feb 1992 10:03:36 -0600
Date: Fri, 14 Feb 1992 10:03:36 -0600
From: bugs 
To: ${USER} 
Subject: test_412        

This is the message body for test_412"  >  /var/spool/mail/${USER}
	sync
	
	echo "\nFrom root Fri Feb 14 10:03:36 1992
Received: by ${LHOST}.${LIPADDR} (AIX 3.2/UCB 5.64/4.03)
	  id AA13448; Fri, 14 Feb 1992 10:03:36 -0600
Date: Fri, 14 Feb 1992 10:03:36 -0600
From: ${USER2} 
To: ${USER} 
Subject: test_2        

This is the message body for test_2" >>  /var/spool/mail/${USER}
	sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}set cmd="grep From"
	${waitch}$mail_prompt${waitch}| 1-2 
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_412.out
	grep "From: bugs" /tmp/mail_412.out > /dev/null 2>&1
	TT1="$?"
	grep "From: ${USER2}" /tmp/mail_412.out >/dev/null 2>&1
	TT2="$?"
	[ $TT1 = 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}412" $?
	rm -f /tmp/mail_412.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	echo;echo
}

mail_413()
{
	LPTEST=mail${TX}413
	echo "Started `date` - mail TEST Number   - $LPTEST"
	rm -f /tmp/${LPTEST}
	{ 
	echo "$LPTEST test of mail subcommand, unalias."
	# put alias in .mailrc first #
	echo "alias disney snow@white" > $HOME/.mailrc
	echo "alias muppets kermit@frog" >> $HOME/.mailrc 

	# send some mail #
	mail ${USER} <<-EOF
		testing the "unalias" mail subcommand
	EOF
	sync;sync;sync
	sleep 3
	${ask} <<-EOF
	${waitch}${sh_prompt}${waitch}mail
	${waitch}$mail_prompt${waitch}unalias disney
	${waitch}$mail_prompt${waitch}alias
	${waitch}$mail_prompt${waitch}quit
	${waitch}${sh_prompt}${waitch}exit
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_413.out
	grep "snow@white" /tmp/mail_413.out >/dev/null 2>&1
	TT1="$?"
	grep "kermit@frog" /tmp/mail_413.out >/dev/null 2>&1
	TT2="$?"
	[ $TT1 -ne 0 ] && [ $TT2 = 0 ]
	lputil verify "mail${TX}413" $?
#	rm -f /tmp/mail_413.out /usr/spool/mail/${USER}
	rm -f $HOME/.mailrc
	echo;echo
}

mail_414()
{
	LPTEST=mail${TX}414
	rm -f /tmp/${LPTEST} 
	echo "Started `date` - mail TEST Number   - $LPTEST"
	{ 
	echo "$LPTEST FVT testing the unalias mail subcommand"
	# put alias in .mailrc file first #
	echo "alias disney snow@white" > $HOME/.mailrc
	echo "alias muppets kermit@frog" >> $HOME/.mailrc 
	echo "unalias disney" >> $HOME/.mailrc

	# send some mail #
	mail disney <<-EOF
		testing the "unalias" mail subcommand
	EOF
	sync;sync
	} | tee -a ${RESULTS} | tee /tmp/mail_414.out
	grep "User unknown" /tmp/mail_414.out >/dev/null 2>&1
	lputil verify "mail${TX}414" $?
	rm -f /usr/spool/mail/${USER} /tmp/mail_414.out 
	rm -f $HOME/.mailrc
	echo;echo
}

mail_exit()
{
	if test -f $HOME/.mailrc.bak
		then mv $HOME/.mailrc.bak $HOME/.mailrc
	fi

	if test -f /usr/share/lib/Mail.rc.bak
		then mv /usr/share/lib/Mail.rc.bak /usr/share/lib/Mail.rc
	fi
}
