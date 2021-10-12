*
* @(#)89        1.23  src/bos/usr/bin/errlg/bos.sysmgt.serv_aid.err.S, cmderrlg, bos41J, 9520A_a 5/16/95 17:59:42
*
*   COMPONENT_NAME: CMDERRLG
*
*   FUNCTIONS: error description stanza file
*
*   ORIGINS: 27
*
*
*   (C) COPYRIGHT International Business Machines Corp. 1989
*   All Rights Reserved
*   Licensed Materials - Property of IBM
*   US Government Users Restricted Rights - Use, duplication or
*   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*


*
* error logging template file
*
* errupdate reads this file at system installation time
*  and fills in the template data base.
*
* The message corresponding to each codepoint is in codepoint.desc
*

* !!!!!!!!!!!!  IMPORTANT - PLEASE READ THIS IF MODIFYING THIS FILE !!!!!!!!!!!
*
* - If you are making a change to a template that already exists in this file,
*   certain considerations must be made:
*
*   1. The crcid (error id) will change if any one of the following fields
*      are modified or (where applicable) added to: Class, Err_Desc, Err_Type,  
*      Fail_Actions, Fail_Causes, Inst_Actions, Inst_Causes, Prob_Causes,
*      User_Actions, User_Causes, and Detail_Data.  So if you are changing a
*      template in such a way that the crcid also changes, make sure to do
*      the following:
*      - Check the errnotify.add file to see if the original crcid is being
*      used for concurrent error notification.  If it is, the errnotify.add
*      file has to be changed to use the new crcid instead of the original
*      one. 
*      - If the template with the original crcid has already gone out with
*      a published release, for the purpose of a migration install,
*      put a "-<original crcid>:" at the top of this file.  This will prevent
*      the template repository from ending up with two different templates 
*      with duplicate labels.
*  2. The crcid will not change if the Label is changed or if the Comment, Log,
*     Report, or Alert fields of the template are changed.  In these cases,
*     DO NOT add a "-<original crcid>:" to the top of the file - even though the
*     raw template input in this file looks different than it did before - as 
*     this will have the effect of deleting the template entirely from the
*     resulting template repository.
*
* - If you are deleting a template that has already gone out with a published
*   release, for the purpose of a migration install, put a 
*   "-<crcid of template being deleted>:" at the top of the file, and then 
*   delete the template in this file.
*
* - If you are modifying, adding, or deleting a template, be sure to make the
*   corresponding changes to the errids.desc file.  The errids.desc file will 
*   be used to generate the errids.h file, so it should look exactly like this 
*   err.S file, except for the deletions of old templates (the "-<crcid>:" 
*   lines at the top of this file).
*
* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!






* delete templates that have been altered in 4.1

-1805B3B1:

-3DDB0A69:

-8564E6B1:
 
-D7BDE2AD:

-0703026B:

-0E47005F:

-19A13B0A:

-DEE5AA95:

-8BF58AD9:

-76866F0F:

-4E21B571:

-0F27AAE5:

-E64EC259:

-CFC1A4DD:

-20FAED7F:

-1581762B:

-21F54B38:

-4A29D32A:

+ OPMSG:
        Err_Type = TEMP
        Class = O
        Report = TRUE
        Log    = TRUE
        Alert  = FALSE
        Comment = "alertable operator message"
        Err_Desc = B000
        Prob_Causes = FFFF
	User_Causes = E805
	User_Actions= 2000
        Fail_Causes =  FFFF
        Fail_Actions=  FFFF
        Detail_Data = 230, 8117 ,ALPHA
 
+ SYSLOG:
        Err_Type = UNKN
        Class = S
        Report = TRUE
        Log    = TRUE
        Alert  = FALSE
        Comment = "syslog msg routed to error log"
        Err_Desc = E846
        Prob_Causes = FFFF
	User_Causes = E806
	User_Actions= 2000
        Inst_Causes =  FFFF
        Inst_Actions=  FFFF
        Detail_Data = 230, 8004 ,ALPHA
 
+ CHECKSTOP:
        Err_Type = TEMP
        Class = H
        Report = TRUE
        Log    = TRUE
        Alert  = FALSE
        Comment = "checkstop has occurred"
        Err_Desc = E840
        Prob_Causes = 0000
        Fail_Causes = 0000 
        Fail_Actions=  0000,3302,3000
        Detail_Data = 1,  8002, DECIMAL
        Detail_Data = 57, 8003, ALPHA
 
+ DISKETTE_ERR1:
        User_Causes = ffff
        Class = H
        Err_Type = TEMP
        Report = TRUE
        Log = TRUE
        Alert = FALSE
        Comment = "diskette device overrun error"
        Err_Desc = 1322
        Prob_Causes = 5
        User_Actions = f011,3301,0000
        Fail_Causes = 5
        Fail_Actions = ffff
        Detail_Data = 64, 11, HEX
 
+ DISKETTE_ERR2:
        User_Causes = ffff
        Class = H
        Err_Type = UNKN
        Report = TRUE
        Log = TRUE
        Alert = FALSE
        Comment = "diskette timeout error"
        Err_Desc = 1321
        Prob_Causes = 3100
        User_Actions = 0000
        Fail_Causes = 3306
        Fail_Actions = ffff
        Detail_Data = 64, 11, HEX
 
+ DISKETTE_ERR3:
        User_Causes = ffff
        Class = H
        Err_Type = PERM
        Report = TRUE
        Log = TRUE
        Alert = FALSE
        Comment = "diskette media error"
        Err_Desc = e802
        Prob_Causes = 5000
        User_Actions = 1603,3301,e801,0000
        Fail_Causes = 5000
        Fail_Actions = ffff
        Detail_Data = 64, 11, HEX
 
+ DISKETTE_ERR4:
        User_Causes = ffff
        Class = H
        Err_Type = UNKN
        Report = TRUE
        Log = TRUE
        Alert = FALSE
        Comment = "diskette unknown error"
        Err_Desc = 1322
        Prob_Causes = 5000,3100,6311
        User_Actions = 0000
        Fail_Causes = 3306
        Fail_Actions = ffff
        Detail_Data = 64, 11, HEX
 
+ DISKETTE_ERR5:
        User_Causes = ffff
        Class = H
        Err_Type = TEMP
        Report = TRUE
        Log = TRUE
        Alert = FALSE
        Comment = "temporary diskette PIO error"
        Err_Desc = E874
        Prob_Causes = 5
        User_Actions = f011,3301,0000
        Fail_Causes = 5
        Fail_Actions = ffff
 
+ DISKETTE_ERR6:
        User_Causes = ffff
        Class = H
        Err_Type = PERM
        Report = TRUE
        Log = TRUE
        Alert = FALSE
        Comment = "permanent diskette PIO error"
        Err_Desc = E874
        Prob_Causes = 5
        User_Actions = 0000
        Fail_Causes = 5
        Fail_Actions = ffff
 
+ MSLA_CLOSE:
	Comment = "Msla close error"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc =   2000
	Prob_Causes =  1000
	Fail_Causes =  1022
	Fail_Actions = 1301,1205
	Detail_Data =  0032,00D0,ALPHA
	Detail_Data =  0004,000F,DEC
	Detail_Data =  0004,0052,HEX
 
+ MSLA_PROTOCOL:
	Comment =  "Msla protocol error"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = 3000
	Prob_Causes = 3111
	Fail_Causes = 3111,3400,3500
	Fail_Actions = 0000
	Detail_Data =  0032,00D0,ALPHA
	Detail_Data =  0004,000F,DEC
	Detail_Data =  0004,0052,HEX
 
+ MSLA_START:
	Comment =  "Msla start error"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = 5004
	Prob_Causes = 0101
	Fail_Causes = 0101
	Fail_Actions = 1012
	Detail_Data =  0032,00D0,ALPHA
	Detail_Data =  0004,000F,DEC
	Detail_Data =  0004,0052,HEX
 
+ MSLA_IOCTL:
	Comment =  "Msla ioctl error"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = 1010
	Prob_Causes = 3300
	Fail_Causes = 3300
	Fail_Actions = 0001,1300
	Detail_Data =  0032,00D0,ALPHA
	Detail_Data =  0004,000F,DEC
	Detail_Data =  0004,0052,HEX
 
+ MSLA_INTR:
	Comment =  "Msla intr error"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = 3000
	Prob_Causes = 1021,E00A
	Fail_Causes =  2000,3500
	Fail_Actions = 2000,0000
	Detail_Data =  0032,00D0,ALPHA
	Detail_Data =  0004,000F,DEC
	Detail_Data =  0004,0052,HEX
 
+ MSLA_WRITE:
	Comment =  "Msla write error"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = 1010
	Prob_Causes = 3330
	Fail_Causes = 3300
	Fail_Actions = 0001,1300
	Detail_Data =  0032,00D0,ALPHA
	Detail_Data =  0004,000F,DEC
	Detail_Data =  0004,0052,HEX
 
+ MSLA_ADAPTER:
	Comment =  "Msla hardware error"
	Class = H
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = 1010
	Prob_Causes = 3302
	Fail_Causes = 3302
	Fail_Actions = 3100
	Detail_Data =  0032,00D0,ALPHA
	Detail_Data =  0004,000F,DEC
	Detail_Data =  0004,0052,HEX
 
+ SCSI_ERR1:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "permanent SCSI adapter hardware error"
    Err_Desc = 1010
    Prob_Causes = 3330, 3400, 3461, 6000
    Fail_Causes = 3300, 3400, 6000
    Fail_Actions = 0000, 0301
    Detail_Data = 108, 11, HEX
 
+ SCSI_ERR2:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "temporary SCSI adapter hardware error"
    Err_Desc = 1010
    Prob_Causes = 3330, 3400, 3461, 6000
    Fail_Causes = 3300, 3400, 6000
    Fail_Actions = 0000, 0301
    Detail_Data = 108, 11, HEX
 
+ SCSI_ERR3:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "permanent unknown SCSI adapter sfw error"
    Err_Desc = 6100
    Prob_Causes = 3331
    Fail_Causes = 3300
    Fail_Actions = 0000, 3301, 3000
    Detail_Data = 108, 11, HEX
 
+ SCSI_ERR4:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "temporary unknown SCSI adapter sfw error"
    Err_Desc = 6100
    Prob_Causes = 3331
    Fail_Causes = 3300
    Fail_Actions = 0000, 3301, 3000
    Detail_Data = 108, 11, HEX
 
+ SCSI_ERR5:
    Class = S
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "permanent unknown driver error"
    Err_Desc = 2100
    Prob_Causes = 1000
    Fail_Causes = 1000
    Fail_Actions = 3301, 3000
    Detail_Data = 108, 11, HEX
 
+ SCSI_ERR6:
    Class = S
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "temporary unknown driver error"
    Err_Desc = 2100
    Prob_Causes = 1000
    Fail_Causes = 1000
    Fail_Actions = 3301, 3000
    Detail_Data = 108, 11, HEX
 
+ SCSI_ERR7:
    Class = H
    Err_Type = UNKN
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "permanent unknown system error"
    Err_Desc = FE00
    Prob_Causes = 1000
    Fail_Causes = 1000
    Fail_Actions = 0000, 3301, 3000
    Detail_Data = 108, 11, HEX
 
+ SCSI_ERR8:
    Class = H
    Err_Type = UNKN
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "temporary unknown system error"
    Err_Desc = FE00
    Prob_Causes = 1000
    Fail_Causes = 1000
    Fail_Actions = 0000, 3301, 3000
    Detail_Data = 108, 11, HEX

+ SCSI_ERR9:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "potential data loss condition"
    Err_Desc = E849
    Prob_Causes = E87E
    Fail_Causes = EA10
    Fail_Actions = 0000
 
+ SCSI_ERR10:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "temporary SCSI bus error"
    Err_Desc = E942
    Prob_Causes = 3400, 3461, 6000, 3300
    Fail_Causes = 3400, 6000, 3300
    Fail_Actions = 0000, 0301
    Detail_Data = 108, 11, HEX

+ DISK_ERR2:
    Comment = "CD-ROM, disk, or read/write optical hardware error"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = 1312
    Prob_Causes = 6310
    Fail_Causes = 6310, 6330
    Fail_Actions = 0000
    Detail_Data = 156, 11, HEX
 
+ DISK_ERR3:
    Comment = "adapter-detected CD-ROM, disk, or read/write optical failure"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = 1312
    Prob_Causes = 6310, 3452
    Fail_Causes = 6310, 6330, 3452
    Fail_Actions = 0000
    Detail_Data = 156, 11, HEX
 
+ DISK_ERR5:
    Comment = "unknown CD-ROM, disk, or read/write optical failure"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = FE00
    Prob_Causes = 6310, 5000, 3300
    Fail_Causes = FFFF
    Fail_Actions = 1601,0000
    Detail_Data = 156, 11, HEX
 
+ BADISK_ERR1:
    Comment = "badisk recovered soft read error"
    Class = H
    Report = TRUE
    Alert = FALSE
    Log = TRUE
    Err_Type = TEMP
    Err_Desc = 1312
    Prob_Causes = 5001, 6310
    User_Causes = 5100
    User_Actions = ffff
    Fail_Causes = 5001
    Fail_Actions = 0000
    Detail_Data = 22, 11, HEX
 
+ BADISK_ERR2:
    Comment = "badisk hard read error"
    Class = H
    Report = TRUE
    Alert = FALSE
    Log = TRUE
    Err_Type = PERM
    Err_Desc = 1312
    Prob_Causes = 5001, 6310
    User_Causes = 5100
    User_Actions = ffff
    Fail_Causes = 5001
    Fail_Actions = 0000
    Detail_Data = 22, 11, HEX
 
+ BADISK_ERR3:
    Comment = "badisk soft equipment check"
    Class = H
    Report = TRUE
    Alert = FALSE
    Log = TRUE
    Err_Type = TEMP
    Err_Desc = 1312
    Prob_Causes = 6310
    User_Causes = ffff
    User_Actions = 3301,1002
    Fail_Causes = 6310
    Fail_Actions = 0000
    Detail_Data = 22, 11, HEX
 
+ BADISK_ERR4:
    Comment = "badisk hard equipment check"
    Class = H
    Report = TRUE
    Alert = FALSE
    Log = TRUE
    Err_Type = PERM
    Err_Desc = 1312
    Prob_Causes = 5001
    User_Causes = 5100
    User_Actions = 3301,1002
    Fail_Causes = 5001
    Fail_Actions = 0000
    Detail_Data = 22, 11, HEX
 
+ BADISK_ERR5:
    Comment = "badisk attachment error"
    Class = H
    Report = TRUE
    Alert = FALSE
    Log = TRUE
    Err_Type = PERM
    Err_Desc = 1312
    Prob_Causes = 0005,0441,1020,6310
    User_Causes = ffff
    User_Actions = 3301,1002,E801,1800
    Fail_Causes = 0005,0501,6310
    Fail_Actions = 0000
    Detail_Data = 22, 11, HEX
 
+ BADISK_ERR8:
    Comment = "badisk seek fault"
    Class = H
    Report = TRUE
    Alert = FALSE
    Log = TRUE
    Err_Type = PERM
    Err_Desc = 1312
    Prob_Causes = 6310
    User_Causes = ffff
    User_Actions = 3301,1002
    Fail_Causes = 6310
    Fail_Actions = 0000
    Detail_Data = 22, 11, HEX
 
+ CDROM_ERR1:
        Comment = "cdrom permanent media error"
        Class = H
        Report = TRUE
        Log    = TRUE
        Alert  = FALSE
        Err_Type = PERM
        Err_Desc = E801
        Prob_Causes = 5004
        User_Causes = 5100
        User_Actions = 1601, 0000
        Fail_Causes = E800, 6312
        Fail_Actions = 1601, 0000
        Detail_Data = 156, 11 ,HEX
 
+ CDROM_ERR2:
        Comment = "cdrom temporary media error"
        Class = H
        Report = TRUE
        Log    = TRUE
        Alert  = FALSE
        Err_Type = TEMP
        Err_Desc = E801
        Prob_Causes = 5004
        User_Causes = 5100
        User_Actions = 1601, 0000
        Fail_Causes = E800, 6312
        Fail_Actions = 1601, 0000
        Detail_Data = 156, 11 ,HEX
 
+ CDROM_ERR3:
        Comment = "cdrom permanent hardware error"
        Class = H
        Report = TRUE
        Log    = TRUE
        Alert  = FALSE
        Err_Type = PERM
        Err_Desc = E800
        Prob_Causes = 6312
        Fail_Causes = 6312
        Fail_Actions = 0000
        Detail_Data = 156, 11 ,HEX
 
+ CDROM_ERR4:
        Comment = "cdrom temporary hardware error"
        Class = H
        Report = TRUE
        Log    = TRUE
        Alert  = FALSE
        Err_Type = TEMP
        Err_Desc = E800
        Prob_Causes = 6312
        Fail_Causes = 6312
        Fail_Actions = 0000
        Detail_Data = 156, 11 ,HEX
 
+ CDROM_ERR5:
        Comment = "adapter-detected permanent cdrom error"
        Class = H
        Report = TRUE
        Log    = TRUE
        Alert  = FALSE
        Err_Type = PERM
        Err_Desc = E800
        Prob_Causes = 6312,3451
        Fail_Causes = 6312
        Fail_Actions = 0000
        Detail_Data = 156, 11 ,HEX
 
+ CDROM_ERR6:
        Comment = "adapter-detected temporary cdrom error"
        Class = H
        Report = TRUE
        Log    = TRUE
        Alert  = FALSE
        Err_Type = TEMP
        Err_Desc = E800
        Prob_Causes = 6312,3451
        Fail_Causes = 6312
        Fail_Actions = 0000
        Detail_Data = 156, 11 ,HEX
 
+ CDROM_ERR7:
        Comment = "cdrom device recovered error"
        Class = H
        Report = TRUE
        Log    = TRUE
        Alert  = FALSE
        Err_Type = TEMP
        Err_Desc = E800
        Prob_Causes = 5004, 6312
        User_Causes = 5100
        User_Actions = 1601, 0000
        Fail_Causes = E800, 6312
        Fail_Actions = 1601, 0000
        Detail_Data = 156, 11 ,HEX
 
+ CDROM_ERR8:
        Comment = "cdrom unknown error"
        Class = H
        Report = TRUE
        Log    = TRUE
        Alert  = FALSE
        Err_Type = UNKN
        Err_Desc = FE00
        Prob_Causes = 5004, 6312, 3300
        Fail_Causes = FFFF
        Fail_Actions = 0000
        Detail_Data = 156, 11 ,HEX
 
+ MEMORY:
        Report = TRUE
        Log    = TRUE
        Alert  = FALSE
        Err_Desc = E845
        Prob_Causes = E823,E826
        Fail_Causes = E8E8,E8E9
        Fail_Actions = 0000
        Detail_Data = 4, A3 ,HEX
        Class  = H
        Err_Type = PERM
        Comment = "memory parity error"
 
 
+ TAPE_ERR1:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "tape media error"
    Err_Desc = 1332
    Prob_Causes = 5003
    User_Causes = 5100,7401
    User_Actions = 1601,0000
    Detail_Data = 156, 11, HEX
 
+ TAPE_ERR2:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "tape hardware/aborted command error"
    Err_Desc = 1331
    Prob_Causes = 6314
    Fail_Causes = 5003, 6314
    Fail_Actions = 0000
    Detail_Data = 156, 11, HEX
 
+ TAPE_ERR3:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "recovered error threshold exceeded"
    Err_Desc = 1331
    Prob_Causes = 5003,6314
    User_Causes = 5100, 7401
    User_Actions = 1601, 0000
    Fail_Causes = 5003, 6314
    Fail_Actions = 0000
    Detail_Data = 156, 11, HEX
 
+ TAPE_ERR4:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "tape SCSI adapter detected error"
    Err_Desc = 1331
    Prob_Causes = 3300, 6314
    Fail_Causes = 3300, 6314
    Fail_Actions = 0000
    Detail_Data = 156, 11, HEX
 
+ TAPE_ERR5:
    Class = H
    Err_Type = UNKN
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "tape unknown error"
    Err_Desc = FE00
    Prob_Causes = 5003, 6314, 3300
    Fail_Causes = FFFF
    Fail_Actions = 0000
    Detail_Data = 156, 11, HEX
 
+ TAPE_ERR6:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "tape drive needs cleaning"
    Err_Desc = 1332
    Prob_Causes = 6314
    User_Causes = 7401
    User_Actions = 0000
    Fail_Causes = 6314
    Fail_Actions = 0000
    Detail_Data = 156, 11, HEX
 
+ DMA_ERR:
    Class = H
    Err_Type = UNKN
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "DMA unknown error, see device's error log"
    Err_Desc = FE00
    Prob_Causes = 0005, 1000, 3300, 6000
    Fail_Causes = FFFF
    Fail_Actions = 0000
    Detail_Data = 4,65,HEX           * Virtual Memory Handle (IOCC)
    Detail_Data = 4,50,HEX           * Channel
    Detail_Data = 4,07,HEX           * Channel Status Register
 
+ INTR_ERR:
    Class = H
    Err_Type = UNKN
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "INTERRUPT unknown error"
    Err_Desc = FE00
    Prob_Causes = 0005, 1000, 3300, 6000
    Fail_Causes = FFFF
    Fail_Actions = 0000
    Detail_Data = 4,65,HEX           * Virtual Memory Handle (IOCC)
    Detail_Data = 4,818B,HEX         * Interrupt Level
    Detail_Data = 4,8188,HEX         * EIS0 register
    Detail_Data = 4,8189,HEX         * EIS1 register
 
+ MISC_ERR:
    Class = H
    Err_Type = UNKN
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "INTERRUPT:IO bus timeout or Channel Check "
    Err_Desc = E856
    Prob_Causes = 3300, 6300
    Fail_Causes = 3300, 6300
    Fail_Actions = 0000
    Detail_Data = 4,8119,HEX           * IO Bus(IOCC) number
    Detail_Data = 4,811A,HEX           * Bus Status Register
    Detail_Data = 4,811B,HEX           * Miscellaneous Interrupt Register
 
+ EPOW_SUS:
    Class = H
    Err_Type = UNKN
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "EPOW: Loss of primary power."
    Err_Desc = 1400
    Prob_Causes = 200, 201
    Fail_Causes =  200
    Fail_Actions = 200,3302,0000
    Detail_Data = 4,8009,HEX         * Power status register
 
+ EPOW_RES:
    Class = H
    Err_Type = UNKN
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "EPOW: Resumption of primary power."
    Err_Desc = E842
    Prob_Causes = 200, 201
    Fail_Causes =  200
    Fail_Actions = 200,3302,0000
    Detail_Data = 4,8009,HEX         * Power status register
 
*
* error record template for logging SRC messages
*
+ SRC:
        Comment = "System Resource Controler"
        Class = S
        Log = TRUE
        Report = TRUE
        Alert = false
        Err_Type = PERM
        Err_Desc = 0x2100               * software program error
        Prob_Causes = 0x1001            * application program
        Fail_Causes = 0x1000            * software program error
        Fail_Actions = 0x1000           * perform problem recovery procedures
        Detail_Data = 4,14,DECIMAL      * sympthom code
        Detail_Data = 4,12,DECIMAL      * software error code
        Detail_Data = 4,07,DECIMAL      * error code
        Detail_Data = 40,A2,ALPHA       * detecting module
        Detail_Data = 20,A3,ALPHA       * failing mudule/subsystem
 
*
* Rendering Context Manager
*
+ RCMERR:
        Comment = "Rendering Context Mgr error"
        Class = S
        Log = TRUE
        Report = TRUE
        Alert = FALSE
        Err_Type = PERM
	Err_Desc = 2100               * software program error
	Prob_Causes = 1000            * software program
	Fail_Causes = 1000            * software program error
	Fail_Actions = 2000           * review detailed data
        Detail_Data = 80,804B,ALPHA     * Detecting,failing,rc,rcm err,unique
 
*
* Console Device Driver
*
 
+ CONSOLE:
	Comment = "Console device driver error"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = 2100               * software program error
	Prob_Causes = 1000            * software program
	Fail_Causes = 1000            * software program error
	Fail_Actions = 2000           * review detailed data
	Detail_Data = 10,A2,ALPHA     * detecting module
	Detail_Data = 10,A3,ALPHA     * failing module
	Detail_Data =  4,0F,DECIMAL   * return code from failing module
	Detail_Data =  4,07,DECIMAL   * error code
 
+ DUMP:
        Err_Type = TEMP
        Class = H
        Report = TRUE
        Log    = TRUE
        Alert  = FALSE
        Comment = "error during init of primary dump device "
        Err_Desc = E841
        Prob_Causes = 6310,6314
        User_Causes =  6001,6010,3300
        User_Actions=  0000,1503
        Detail_Data = 4, 8005, hex            * major/minor device number
        Detail_Data = 4, 07,   hex            * error code
 
+ PPRINTER_ERR1:
	Class = H
	Err_Type = UNKN
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "pprinter unknown error"
	Err_Desc = 1201
	Prob_Causes = 0005
	Fail_Causes = 0005
	Fail_Actions = 3300, 3000
	Detail_Data = 4, 54, HEX
	Detail_Data = 4, 8031, HEX
	Detail_Data = 4, 51, HEX
	Detail_Data = 4, 70, HEX
 
*	Multiprotocol Quad Port (MPQP) error template
*
+ MPQP_QUE:
  	Comment		= "MPQP Queue Unavailable"
	Class		= H
	Report		= TRUE
	Log		= TRUE
	Alert		= FALSE
	Err_Type	= TEMP
	Err_Desc	= E970
	Prob_Causes	= 1022 , E883
	Fail_Causes	= 1022 , EA13
	Fail_Actions	= 1301 , 0000
	Detail_Data = 32 , D0 , ALPHA

+ MPQP_XFTO:
        Comment         = "Transmit Failsafe Timeout "
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = TRUE
        Err_Type        = PERM
        Err_Desc        = 1010
        Prob_Causes     = 3330
        Fail_Causes     = 3302
        Fail_Actions    = 0000 , 1402
        Detail_Data = 32, D0, ALPHA

+ MPQP_X21TO:
        Comment         = "X.21 Timeout"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 3310
        Prob_Causes     = 2005 , 3600
        Inst_Causes     = 1501 , 3500
        Inst_Actions    = 0000 , 0301
        Fail_Causes     = 2005 , 3400 , 3302
        Fail_Actions    = 0000 , 0301 , 1200
        Detail_Data = 32, D0, ALPHA
	Detail_Data = 4, 8144, HEX
  
+ MPQP_DSRTO:
        Comment         = "DRS Going On Timeout"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 3302
        Prob_Causes     = 3600 , 3330
        User_Causes     = 3400
        User_Actions    = 0301
        Inst_Causes     = 3400 , 3500
        Inst_Actions    = 0301
        Fail_Causes     = 3600 , 3302
        Fail_Actions    = 0403 , 0000
        Detail_Data = 32, D0, ALPHA
 
+ MPQP_DSROFFTO:
        Comment         = " DSR already on for switched line "
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 3302
        Prob_Causes     = 3600 , 3330 , 3601
        User_Causes     = 3400
        User_Actions    = 0301
        Inst_Causes     = 3400 , 3500
        Inst_Actions    = 0301
        Fail_Causes     = 3600 , 3302
        Fail_Actions    = 0403 , 0000
        Detail_Data = 32, D0, ALPHA
 
+ MPQP_CTSTO:
        Comment         = " CTS going On Timeout "
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 3000
        Prob_Causes     = 3600 , 3302
        User_Causes     = 3400
        User_Actions    = 0301 , 1503
        Inst_Causes     = 3400 , 3500
        Inst_Actions    = 0301
        Fail_Causes     = 3600 , 3302
        Fail_Actions    = 0403 , 0000
        Detail_Data = 32, D0, ALPHA

+ MPQP_DSRDRP:
        Comment         = " DSR Dropped During Transmit "
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 3000
        Prob_Causes     = 3600 , 3302 , 3603
        User_Causes     = 3400
        User_Actions    = 0301 , 1503
        Inst_Causes     = 3400 , 3500
        Inst_Actions    = 0301
        Fail_Causes     = 3600 , 3302
        Fail_Actions    = 0403 , 0000
        Detail_Data = 32, D0, ALPHA
 
+ MPQP_CTSDRP:
        Comment         = " CTS Dropped During Transmit "
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 3000
        Prob_Causes     = 3600 , 3302 , 3603
        User_Causes     = 3400
        User_Actions    = 0301 , 1503
        Inst_Causes     = 3400 , 3500
        Inst_Actions    = 0301
        Fail_Causes     = 3600 , 3302
        Fail_Actions    = 0403 , 0000
        Detail_Data = 32, D0, ALPHA
 
+ MPQP_RCVERR:
        Comment         = " MPQP Error On Receive Data"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 3000
        Prob_Causes     = 2000 , 3600
        Inst_Causes     = 3500 , 3400
        Inst_Actions    = 0301 , 1503
        Fail_Causes     = 3600 , 3400
        Fail_Actions    = 0403 , 1503
        Detail_Data = 32, D0, ALPHA
	Detail_Data = 4, 8145, HEX
 
+ MPQP_ASWCHK:
        Comment         = " Adapter Software Checksum Error "
        Class           = S
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 6100
        Prob_Causes     = 3302
        Fail_Causes     = 1022
        Fail_Actions    = 1504
        Detail_Data = 32, D0, ALPHA
 
+ MPQP_ADPERR:
        Comment         = " Adapter Not Present or not functioning "
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 1010
        Prob_Causes     = 3302
        Inst_Causes     = 3500
        Inst_Actions    = 1503
        Fail_Causes     = 33C1, 33C2
        Fail_Actions    = 0000
        Detail_Data = 32, D0, ALPHA
	Detail_Data = 4, 60, HEX
	Detail_Data = 4, A5, HEX
 
+ MPQP_IPLTO:
        Comment         = "Adapter IPL Timeout "
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 1010
        Prob_Causes     = 3302
        Inst_Causes     = 3500 , 1400
        Inst_Actions    = 1503
        Fail_Causes     = 3302
        Fail_Actions    = 0000
        Detail_Data = 32, D0, ALPHA
 
+ MPQP_X21CECLR:
        Comment         = " X.21 Unexpected Clear During Call Establishment "
        Class           = S
        Report          = TRUE
        Log             = TRUE
        Alert           = TRUE
        Err_Type        = PERM
        Err_Desc        = 3310
        Prob_Causes     = 2005
        Fail_Causes     = 3400 , 3600 , 2005
        Fail_Actions    = 0301 , 0403 , 0000
        Detail_Data = 32, D0, ALPHA
 
+ MPQP_X21DTCLR:
        Comment         = " X.21 Unexpected Clear During Data Transfer "
        Class           = S
        Report          = TRUE
        Log             = TRUE
        Alert           = TRUE
        Err_Type        = PERM
        Err_Desc        = 3310
        Prob_Causes     = 2005 , 3300
        Fail_Causes     = 3400 , 3600 , 2005
        Fail_Actions    = 0301 , 0403 , 0000
        Detail_Data = 32, D0, ALPHA
 
+ MPQP_X21CPS:
        Comment         = " X.21 Call Progress Signal "
        Class           = S
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 3310
        Prob_Causes     = 3330 , 2005
        Fail_Causes     = 2005 , 3400 , 3600
        Fail_Actions    = 0403 , 1200 , 0000
        Detail_Data = 32, D0, ALPHA
	Detail_Data = 4, 8146, HEX 
 
+ MPQP_BFR:
        Comment         = " MPQP -  Memory Resources Unavailable "
        Class           = S
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERF
        Err_Desc        = 5004
        Prob_Causes     = FE00
        Fail_Causes     = E932
        Fail_Actions    = E837,E835
        Detail_Data = 32, D0, ALPHA
 
+ MPQP_XMTUND:
        Comment         = " Transmit Underrun "
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERF
        Err_Desc        = 5010
        Prob_Causes     = 3330
        Fail_Causes     = 3302 , 1022
        Fail_Actions    = 0000
        Detail_Data = 32, D0, ALPHA
 
+ MPQP_RCVOVR:
        Comment         = " Receive Overrun   "
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERF
        Err_Desc        = 5011
        Prob_Causes     = 3330
        Fail_Causes     = 3302 , 1022
        Fail_Actions    = 0000
        Detail_Data = 32, D0, ALPHA
 
+ CMDLVM:
        Comment         = "Logical Volume Manager "
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERF
        Err_Desc        = 1312
        Prob_Causes     = E855
	User_Causes     = E802
	User_Actions	= FFFF
	Inst_Causes     = E805, E806
	Inst_Actions	= FFFF
        Fail_Causes     = E924, E8E4
        Fail_Actions    = 2000
        Detail_Data     = 16, 8043, ALPHA
 
+ COM_PERM_PIO:
	Comment = "permanent PIO error detected by driver"
	Class = H
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E874
	Prob_Causes = FFFF
	Fail_Causes = E938
	Fail_Actions = E858, 0000
	Detail_Data = 4, 8011, HEX
	Detail_Data = 4, 8012, HEX
	Detail_Data = 4, 8013, HEX
	Detail_Data = 4, 8014, HEX
	Detail_Data = 4, 02, HEX
	Detail_Data = 4, 8031, HEX
 
+ COM_TEMP_PIO:
	Comment = "temporary PIO error detected by driver"
	Class = H
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = E874
	Prob_Causes = FFFF
	Fail_Causes = E938
	Fail_Actions = E858, 0000
	Detail_Data = 4, 8011, HEX
	Detail_Data = 4, 8012, HEX
	Detail_Data = 4, 8013, HEX
	Detail_Data = 4, 8014, HEX
	Detail_Data = 4, 02, HEX
	Detail_Data = 4, 8031, HEX
 
+ TTY_BADINPUT:
	Comment = "ttyinput routine returned with error code"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = E921
	Prob_Causes = FFFF
	Fail_Causes = E951, E952, E953
	Fail_Actions = E838, E839, E825
	Detail_Data = 4, 8015, DECIMAL

+ TTY_PARERR:
	Comment = "Parity/Framing error on input"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = E904
	Prob_Causes = FFFF
	Fail_Causes = E954, E952, E953
	Fail_Actions = E83B, 1503, E83C, E825
 
+ TTY_OVERRUN:
	Comment = "Receiver over-run on input"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = E905
	Prob_Causes = FFFF
	Fail_Causes = 0020
	Fail_Actions = E837, E83A
 
+ TTY_TTYHOG:
	Comment = "ttyhog over-run"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = E906
	Prob_Causes = FFFF
	Fail_Causes = 0020
	Fail_Actions = E837, E83A
 
+ RS_PROG_SLIH:
	Comment = "Unconfiguration process could not find slih structure"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = UNKN
	Err_Desc = E876
	Prob_Causes = FFFF
	Fail_Causes = E930, E983
	Fail_Actions = 0000, 0600, E825
 
+ TTY_PROG_PTR:
	Comment = "Pointer to private structure is null"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = UNKN
	Err_Desc = E877
	Prob_Causes = FFFF
	Fail_Causes = E930, E983
	Fail_Actions = 0000, 0600, E825
 
+ RS_PROG_IOCC:
	Comment = "IOCC is not configured at unconfig time."
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = UNKN
	Err_Desc = E878
	Prob_Causes = FFFF
	Fail_Causes = E930, E983
	Fail_Actions = 0000, 0600, E825
 
+ COM_CFG_ADPT:
	Comment = "adapter already configured"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E885
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Fail_Causes = E986
	Fail_Actions = E845
	Detail_Data = 4, 8015, DECIMAL
 
+ COM_CFG_NADP:
	Comment = "adapter not present"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E886
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Fail_Causes = E987
	Fail_Actions = E846, E825
	Detail_Data = 4, 8015, DECIMAL
 
+ COM_CFG_BUSI:
	Comment = "bad bus id"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E887
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Fail_Causes = E988
	Fail_Actions = E847
	Detail_Data = 4, 07, DECIMAL
 
+ COM_CFG_BUST:
	Comment = "bad bus type"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E888
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Fail_Causes = E989
	Fail_Actions = E848
	Detail_Data = 4, 07, DECIMAL
 
+ COM_CFG_IFLG:
	Comment = "bad interrupt flags"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E889
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Fail_Causes = E98A
	Fail_Actions = E849
	Detail_Data = 4, 07, DECIMAL
 
+ COM_CFG_ILVL:
	Comment = "bad interrupt level"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E88A
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Fail_Causes = E98B
	Fail_Actions = E84A
	Detail_Data = 4, 07, DECIMAL
 
+ COM_CFG_INTR:
	Comment = "bad interrupt priority"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E88B
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Fail_Causes = E98C
	Fail_Actions = E84B
	Detail_Data = 4, 07, DECIMAL
 
+ COM_CFG_MNR:
	Comment = "bad minor number"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E88C
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Fail_Causes = E98D
	Fail_Actions = E84C
	Detail_Data = 4, 07, DECIMAL
 
+ COM_CFG_BUSID:
	Comment = "bus id out of range"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E88D
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Fail_Causes = E98E
	Fail_Actions = E84D
	Detail_Data = 4, 65, HEX
 
+ COM_CFG_DMA:
	Comment = "dma level conflict"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E88E
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Fail_Causes = E98F
	Fail_Actions = E84E
	Detail_Data = 4, 07, DECIMAL
 
+ COM_CFG_DEVA:
	Comment = "devswadd failed"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E88F
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Detail_Data = 4, 8015, DECIMAL
 
+ COM_CFG_PIN:
	Comment = "pincode failed"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E890
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Detail_Data = 4, 8015, DECIMAL
 
+ COM_CFG_DEVD:
	Comment = "devswdel failed"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E896
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Detail_Data = 4, 8015, DECIMAL
 
+ COM_CFG_UNPIN:
	Comment = "unpincode failed"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E897
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Detail_Data = 4, 8015, DECIMAL
 
+ COM_CFG_SLIH:
	Comment = "i_init of slih failed"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E891
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Fail_Causes = E990
	Fail_Actions = E84F, E854
 
+ COM_CFG_PORT:
	Comment = "port already configured"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E892
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Fail_Causes = E991
	Fail_Actions = FFFF
 
+ COM_CFG_RESID:
	Comment = "resid not correct"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E893
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Detail_Data = 4, 8015, DECIMAL
	Fail_Causes = E992
	Fail_Actions = E851, 3000
	Detail_Data = 4, 07, DECIMAL
 
+ COM_CFG_UIO:
	Comment = "uiomove failed"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E893
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Fail_Causes = E993
	Fail_Actions = E851, 3000
	Detail_Data = 4, 8015, DECIMAL
 
+ COM_CFG_UNK:
	Comment = "Unknown adapater type"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E895
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Fail_Causes = E994
	Fail_Actions = E850, 3000
	Detail_Data = 4, 35, HEX
 
+ RS_MEM_EDGE:
	Comment = "can't allocate edge structure"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E879
	Prob_Causes = FFFF
	Fail_Causes = E931
	Fail_Actions = 0007, 0600, E825
 
+ RS_MEM_IOCC:
	Comment = "can't allocate iocc structure"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E87A
	Prob_Causes = FFFF
	Fail_Causes = E931
	Fail_Actions = 0007, 0600, E825
 
+ RS_MEM_EDGEV:
	Comment = "can't allocate edge vector"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E87B
	Prob_Causes = FFFF
	Fail_Causes = E931
	Fail_Actions = 0007, 0600, E825
 
+ COM_MEM_SLIH:
	Comment = "can't allocate slih structure"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E87C
	Prob_Causes = FFFF
	Fail_Causes = E931
	Fail_Actions = 0007, 0600, 3000
 
+ RS_MEM_PVT:
	Comment = "can't allocate private structure"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E87D
	Prob_Causes = FFFF
	Fail_Causes = E931
	Fail_Actions = 0007, 0600, 3000
 
+ RS_PIN_EDGE:
	Comment = "can't pin edge structure"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E87F
	Prob_Causes = FFFF
	Fail_Causes = E932
	Fail_Actions = 0007, 1013, 0600, 3000
 
+ RS_PIN_EDGEV:
	Comment = "can't pin edge vector"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E880
	Prob_Causes = FFFF
	Fail_Causes = E932
	Fail_Actions = 0007, 1013, 0600, 3000
 
+ RS_PIN_IOCC:
	Comment = "can't pin iocc structure"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E881
	Prob_Causes = FFFF
	Fail_Causes = E932
	Fail_Actions = 0007, 1013, 0600, 3000
 
+ COM_PIN_SLIH:
	Comment = "can't pin slih structure"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E882
	Prob_Causes = FFFF
	Fail_Causes = E932, E982
	Fail_Actions = 1013, 0000, 0600, 3000
 
+ RS_8_16_ARB:
	Comment = "8/16 arbitration register failure"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E872
	Prob_Causes = FFFF
	Fail_Causes = E984, E938
	Fail_Actions = 0000
	Detail_Data = 4, 03, HEX
 
+ RS_BAD_INTER:
	Comment = "interrupt from non-existant port"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E873
	Prob_Causes = FFFF
	Fail_Causes = E937, E938
	Fail_Actions = 0000
	Detail_Data = 4, 60, DECIMAL
 
+ EU_CFG_HERE:
	Comment = "adapter already configured"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E885
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Fail_Causes = E995
	Fail_Actions = FFFF
 
+ EU_CFG_GONE:
	Comment = "adapter already unconfigured"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E898
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Fail_Causes = E995
	Fail_Actions = FFFF
 
+ EU_CFG_BUSY:
	Comment = "in use, can't uninitialized"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E899
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Fail_Causes = E996
	Fail_Actions = E852
 
+ EU_CFG_NADP:
	Comment = "adapter not present"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E886
	Prob_Causes = FFFF
	Inst_Causes = E808
	Inst_Actions = E844
	Fail_Causes = E987
	Fail_Actions = E846, 3000
	Detail_Data = 4, 35, HEX
 
+ EU_CFG_NPLN:
	Comment = "can't access planar"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E886
	Prob_Causes = 0203
	Inst_Causes = 3400
	Inst_Actions = 0200
	Fail_Causes = 3400
	Fail_Actions = 0302, 0000
	Detail_Data = 4, 35, HEX
 
+ EU_DIAG_ACC:
	Comment = "in use, no destructive diagnostics"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E883
	Prob_Causes = FFFF
	Fail_Causes = E996
	Fail_Actions = E853
 
+ EU_DIAG_MEM:
	Comment = "can't allocate diagnostic buffer"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E87E
	Prob_Causes = FFFF
	Fail_Causes = E932, E982
	Fail_Actions = 1013, 0000, 0600, 3000
 
+ LION_BOX_DIED:
	Comment = "64port concentrator died"
	Class = H
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E902
	Prob_Causes = FFFF
	Fail_Causes = E939, E93A, E93B
	Fail_Actions = 0200, 0301, 0000
 
+ LION_HRDWRE:
	Comment = "Failure in adapter hardware"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E903
	Prob_Causes = FFFF
	Fail_Causes = E937, E938
	Fail_Actions = 0000, E836
 
+ LION_MEM_LIST:
	Comment = "can't allocate ttyp_t list"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E91A
	Prob_Causes = FFFF
	Fail_Causes = E931
	Fail_Actions = E835, 0600, 3000
 
+ LION_MEM_ADAP:
	Comment = "can't allocate adap structure"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E919
	Prob_Causes = FFFF
	Fail_Causes = E931
	Fail_Actions = E835, 0600, 3000
 
+ LION_PIN_ADAP:
	Comment = "can't pin ADAP structure"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E91F
	Prob_Causes = FFFF
	Fail_Causes = E932
	Fail_Actions = E835, E837, 0600, 3000
 
+ LION_BUFFERO:
	Comment = "Buffer over-run on input"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = E930
	Prob_Causes = FFFF
	Fail_Causes = 0020
	Fail_Actions = E837, E83A
 
+ LION_UNKCHUNK:
	Comment = "Unknown error code in chunk"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = E931
	Prob_Causes = FFFF
	Fail_Causes = E983
	Fail_Actions = E846
	Detail_Data = 4, 07, HEX
 
+ LION_CHUNKNUMC:
	Comment = "Bad value in chunk numc"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = E932
	Prob_Causes = FFFF
	Fail_Causes = E983
	Fail_Actions = E846
	Detail_Data = 4, 92, HEX
 
+ C327_START:
        Comment         = " C327 Start Error "
        Class           = S
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = E82C
        Prob_Causes     = 3400, 2000, 3111
        Fail_Causes     = 3400, 2000, 3111
        Fail_Actions    = 0000, 2000
        Detail_Data     = 4, 0E, HEX
        Detail_Data     = 4, 52, DECIMAL

+ C327_INTR:
        Comment         = " C327 Interrupt Error "
        Class           = S
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = E82B
        Prob_Causes     = 3400, 2000
        Fail_Causes     = 3400, 2000
        Fail_Actions    = 0000, 2000
        Detail_Data     = 4, 0E, DECIMAL
        Detail_Data     = 4, 52, DECIMAL
 
+ TOK_ADAP_ERR:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Potential data loss condition"
    Err_Desc = E849
    Prob_Causes = E87F
    Fail_Causes = EA11
    Fail_Actions = 0000
 
+ X25_ALERT5:
        Comment         = "Modem Failure detected, DCD and/or DSR error"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE808
        Prob_Causes     = 0x3601, 0x3401
        User_Causes     = 0x0201, 0x3400
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0x1202, 0x3401
        Inst_Actions    = 0x1500
        Fail_Causes     = 0x3601, 0x3400, 0xF035
        Fail_Actions    = 0x0301
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT7:
        Comment         = "Local modem auto call unit is not responding"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE809
        Prob_Causes     = 0x3601
        User_causes     = 0x2300
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0x1202
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x3601
        Fail_Actions    = 0x0403
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT8:
        Comment         = "X.21 failure detected in the X.25 subsystem"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE80A
        Prob_Causes     = 0x2005, 0x2300
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x2300, 0xF050
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT9:
        Comment         = "Frame type w received"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 0xE80B
        Prob_Causes     = 0x2006, 0x2106
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x20C1, 0xF01A
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT10:
        Comment         = "Frame type x received"
        Class           = H
        Report          = FALSE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 0xE80C
        Prob_Causes     = 0x2006, 0x2106
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x20C1, 0xF01A
        Fail_Actions    = 0x2000 
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT11:
        Comment         = "Frame type y received"
        Class           = H
        Report          = FALSE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 0xE80D
        Prob_Causes     = 0x2006, 0x2106
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x20C1, 0xF01A
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT12:
        Comment         = "Frame type z received"
        Class           = H
        Report          = FALSE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 0xE80E
        Prob_Causes     = 0x2006, 0x2106
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x20C1, 0xF01A
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT13:
        Comment         = "Frame type w sent"
        Class           = H
        Report          = FALSE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 0xE80F
        Prob_Causes     = 0x2006, 0x2106
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x20C1, 0xF01A
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT14:
        Comment         = "Frame type x sent"
        Class           = H
        Report          = FALSE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 0xE810
        Prob_Causes     = 0x2006, 0x2106
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x20C1, 0xF01A
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT15:
        Comment         = "Frame type y sent"
        Class           = H
        Report          = FALSE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 0xE811
        Prob_Causes     = 0x2006, 0x2106
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x20C1, 0xF01A
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT16:
        Comment         = "Frame type z sent"
        Class           = H
        Report          = FALSE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 0xE812
        Prob_Causes     = 0x2006, 0x2106
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x20C1, 0xF01A
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT17:
        Comment         = "Frame retry limit reached"
        Class           = H
        Report          = FALSE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE813
        Prob_Causes     = 0x2106
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x20A5
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT18:
        Comment         = "Unexpected disconnect frame received"
        Class           = H
        Report          = FALSE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE814
        Prob_Causes     = 0x2006, 0x2106
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x20C1, 0xF01A
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT19:
        Comment         = "Disc mode frame received during link activation"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE815
        Prob_Causes     = 0x2006, 0x2106
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x23C1, 0xF01A
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT31:
        Comment         = "A RESET_INDICATION packet was received by the DTE"
        Class           = H
        Report          = FALSE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE820
        Prob_Causes     = 0x2006, 0x2106
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x20C1, 0xF020
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT21:
        Comment         = "A CLEAR_INDICATION packet was received by the DTE"
        Class           = H
        Report          = FALSE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE816
        Prob_Causes     = 0x2006, 0x2106
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x20C1, 0xF056
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT22:
        Comment         = "A RESTART_INDICATION packet was received by the DTE"
        Class           = H
        Report          = FALSE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE817
        Prob_Causes     = 0x2006, 0x2106
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x2006
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT23:
        Comment         = "A RESET_REQUEST packet was sent to the network"
        Class           = H
        Report          = FALSE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE818
        Prob_Causes     = 0x2080
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0xFFFF
        Fail_Actions    = 0xFFFF
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT24:
        Comment         = "A CLEAR_REQUEST packet was sent to the network"
        Class           = H
        Report          = FALSE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE819
        Prob_Causes     = 0x0503, 0x1001
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x3302
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT25:
        Comment         = "A START_REQUEST packet was sent to the network"
        Class           = H
        Report          = FALSE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE81A
        Prob_Causes     = 0x2080
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x3302
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT26:
        Comment         = "T20 expired prior to receipt of RESTART_CONFIRM"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE81B
        Prob_Causes     = 0x2006, 0x2106
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x20A4, 0x2200
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT27:
        Comment         = "T20 expired prior to receipt of RESET_CONFIRM"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE81C
        Prob_Causes     = 0x2006, 0x2106
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x20A4
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT28:
        Comment         = "T21 expired prior to CALL_CONNECT or CLEAR_INDICATE"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE81D
        Prob_Causes     = 0x2006, 0x2106, 0x2300
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x2006, 0xE952, 0x2201
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT29:
        Comment         = "T23 expired prior to receipt of CLEAR_CONFIRMATION"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE81E
        Prob_Causes     = 0x2006, 0x2106
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x2006, 0xE952, 0x2201
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT30:
        Comment         = "DIAGNOSTIC packet received-DTE,protocol violation"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE81F
        Prob_Causes     = 0x0503, 0x1022
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x2006, 0x0503
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT32:
        Comment         = "CLEAR_INDICATE packet was sent by DCE"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE821
        Prob_Causes     = 0x3541
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x3541, 0x2006
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT33:
        Comment         = "RESET_INDICATION packet was sent by DCE"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE822
        Prob_Causes     = 0x3542, 0x3541
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x3541
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT34:
        Comment         = "RESTART_INDICATION packet was sent by DCE"
        Class           = H
        Report          = FALSE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 0xE823
        Prob_Causes     = 0x3541, 0x2037
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x32D3, 0x3302
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT35:
        Comment         = "RESTART_REQUEST was received by the DCE"
        Class           = H
        Report          = FALSE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 0xE824
        Prob_Causes     = 0x0503
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x2006, 0x3302
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT36:
        Comment         = "T10 expired (DCE) prior to receipt RESTART_CONFIRM"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 0xE825
        Prob_Causes     = 0x0503
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x2006, 0x0503
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT37:
        Comment         = "T12 expired (DTE) prior to receipt RESET_CONFIRM"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 0xE826
        Prob_Causes     = 0x1010, 0x1022, 0x0503
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x1022, 0x0503
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT38:
        Comment         = "T11 expired (DTE) prior to receipt CALL_ACCEPTED"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 0xE827
        Prob_Causes     = 0x1001, 0x0503
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x1022, 0x0503
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ALERT39:
        Comment         = "T13 expired (DCE) prior to receipt CLEAR_CONFIRM"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 0xE828
        Prob_Causes     = 0x1001, 0x0503
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Fail_Causes     = 0x1022, 0x32D1
        Fail_Actions    = 0x2000
        Detail_Data     = 40, 11, HEX
 
+ X25_ADAPT:
        Comment         = "X.25 adapter card is not present "
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0x1010
        Prob_Causes     = 0x3302
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0x3500
        Inst_Actions    = 0x1503
        Fail_Causes     = 0x3302
        Fail_Actions    = 0x0000
 
+ X25_IPL:
        Comment         = "X.25 adapter time-out during IPL"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0x1010
        Prob_Causes     = 0x3302
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0x3500, 0x1400
        Inst_Actions    = 0x1503
        Fail_Causes     = 0x3302
        Fail_Actions    = 0x1803
 
+ X25_CONFIG:
        Comment         = "X.25 configuration error"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE829
        Prob_Causes     = 0x0503
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0x1503
        Inst_Causes     = 0xFFFF
        Inst_Actions    = 0xFFFF
        Fail_Causes     = 0xFFFF
        Fail_Actions    = 0x2000
        Detail_Data     = 80, 11, HEX
 
+ X25_UCODE:
        Comment         = "X.25 microcode error"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = 0xE82A
        Prob_Causes     = 0x3331
        User_causes     = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        User_Actions    = 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
        Inst_Causes     = 0xFFFF
        Inst_Actions    = 0xFFFF
        Fail_Causes     = 0xE923, 0x3302
        Fail_Actions    = 0x1301, 0x1803
        Detail_Data     = 80, 12, HEX
 
+ WHP0001:
              Comment = "Memory Allocation error"
              Class = S
              Log = True
              Report = True
              Alert = False
              Err_Type = TEMP
              Err_Desc = 2100
              Prob_Causes = E83C
              Fail_Causes = E930
              Fail_Actions = 3302, 3000
              Detail_Data = 32,D0,ALPHA
 
+ WHP0002:
              Comment = "Message Queue Recieve Error"
              Class = S
              Log = True
              Report = True
              Alert = False
              Err_Type = TEMP
              Err_Desc = 2100
              Prob_Causes = E830
              Fail_Causes = E8EA
              Fail_Actions = E811
              Detail_Data = 32,D0,ALPHA
              Detail_Data = 4,8057,DEC
 
+ WHP0003:
              Comment = "IPC Message Queue Send Error"
              Class = S
              Log = True
              Report = True
              Alert = False
              Err_Type = TEMP
              Err_Desc = 2100
              Prob_Causes = E831
              Fail_Causes = E8EB
              Fail_Actions = E811
              Detail_Data = 32,D0,ALPHA
              Detail_Data = 4,8057,DEC
 
+ WHP0004:
              Comment = "Hcon IPC Message Queue Creation Error"
              Class = S
              Log = True
              Report = True
              Alert = False
              Err_Type = TEMP
              Err_Desc = 2100
              Prob_Causes = E832
              Fail_Causes = E8EC
              Fail_Actions = E812
              Detail_Data = 32,D0,ALPHA
 
+ WHP0005:
              Comment = "Hcon IPC Message Queue Creation Error"
              Class = S
              Log = True
              Report = True
              Alert = False
              Err_Type = TEMP
              Err_Desc = 2100
              Prob_Causes = E832
              Fail_Causes = E8ED
              Fail_Actions = E813,E814
              Detail_Data = 32,D0,ALPHA
 
+ WHP0006:
              Comment = "Hcon IPC Message Queue Stat Error"
              Class = S
              Log = True
              Report = True
              Alert = False
              Err_Type = TEMP
              Err_Desc = 2100
              Prob_Causes = E833
              Fail_Causes = E8EE
              Fail_Actions = E811
              Detail_Data = 32,D0,ALPHA
              Detail_Data = 4,8057,DEC
 
+ WHP0007:
              Comment = "Hcon IPC Message Queue Set Error"
              Class = S
              Log = True
              Report = True
              Alert = False
              Err_Type = TEMP
              Err_Desc = 2100
              Prob_Causes = E834
              Fail_Causes = E8EF
              Fail_Actions = E811
              Detail_Data = 32,D0,ALPHA
 
+ WHP0008:
              Comment = "Hcon IPC Message Queue Remove Error"
              Class = S
              Log = True
              Report = True
              Alert = False
              Err_Type = TEMP
              Err_Desc = 2100
              Prob_Causes = E835
              Fail_Causes = E8F0
              Fail_Actions = E816
              Detail_Data = 32,D0,ALPHA
 
+ WHP0009:
              Comment = "Hcon IPC Shared Segment Attachment Error"
              Class = S
              Log = True
              Report = True
              Alert = False
              Err_Type = TEMP
              Err_Desc = 2100
              Prob_Causes = E836
              Fail_Causes = E8F1
              Fail_Actions = E815
              Detail_Data = 32,D0,ALPHA
              Detail_Data = 4,8057,DEC
 
+ WHP0010:
              Comment = "Hcon IPC Shared Segment Detachment Error"
              Class = S
              Log = True
              Report = True
              Alert = False
              Err_Type = TEMP
              Err_Desc = 2100
              Prob_Causes = E837
              Fail_Causes = E8F2
              Fail_Actions = E815
              Detail_Data = 32,D0,ALPHA
              Detail_Data = 4,8057,DEC
 
+ WHP0011:
              Comment = "Hcon IPC Shared Segment Allocation Error"
              Class = S
              Log = True
              Report = True
              Alert = False
              Err_Type = TEMP
              Err_Desc = 2100
              Prob_Causes = E838
              Fail_Causes = E8F3
              Fail_Actions = E816
              Detail_Data = 32,D0,ALPHA
 
+ WHP0012:
              Comment = "Hcon IPC Shared Segment Stat Error"
              Class = S
              Log = True
              Report = True
              Alert = False
              Err_Type = TEMP
              Err_Desc = 2100
              Prob_Causes = E839
              Fail_Causes = E8F4
              Fail_Actions = E815
              Detail_Data = 32,D0,ALPHA
              Detail_Data = 4,8057,DEC
 
+ WHP0013:
              Comment = "Hcon IPC Shared Segment Set Error"
              Class = S
              Log = True
              Report = True
              Alert = False
              Err_Type = TEMP
              Err_Desc = 2100
              Prob_Causes = E83A
              Fail_Causes = E8F5
              Fail_Actions = E815
              Detail_Data = 32,D0,ALPHA
              Detail_Data = 4,8057,DEC
 
+ WHP0014:
              Comment = "Hcon IPC Shared Segment Remove Error"
              Class = S
              Log = True
              Report = True
              Alert = False
              Err_Type = TEMP
              Err_Desc = 2100
              Prob_Causes = E83B
              Fail_Causes = E8F6
              Fail_Actions = E816
              Detail_Data = 32,D0,ALPHA
 
+ ISI_PROC:
	Err_Type = PERM
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "instuction storage interrupt : processor"
	Err_Desc = E837
	Prob_Causes = 1000
	Fail_Causes = 1000
	Fail_Actions = 3301,3000
	Detail_Data = 4, 8037, hex
	Detail_Data = 4, 8038, hex
	Detail_Data = 4, 8035, hex
	Detail_Data = 4, 8039, hex
 
+ DSI_PROC:
	Err_Type = PERM
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "data storage interrupt : processor"
	Err_Desc = E831
	Prob_Causes = 1000
	Fail_Causes = 1000
	Fail_Actions = 3301,3000
	Detail_Data = 4, 8032, hex
	Detail_Data = 4, 8035, hex
	Detail_Data = 4, 8033, hex
	Detail_Data = 4, 8039, hex
 
+ DSI_SCU:
	Err_Type = PERM
	Class = H
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "data storage interrupt : storage control unit"
	Err_Desc = E832
	Prob_Causes = E820, 1000
	Fail_Causes = E8E5, 1000
	Fail_Actions = 0000,3301,3000
	Detail_Data = 4, 8032, hex
	Detail_Data = 4, 8033, hex
	Detail_Data = 4, 8034, hex
	Detail_Data = 4, 8035, hex
 
+ DSI_IOCC:
	Err_Type = PERM
	Class = H
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "data storage interrupt : IOCC"
	Err_Desc = E833
	Prob_Causes = E821, 1000
	Fail_Causes = E8E6, 1000
	Fail_Actions = 0000,3301,3000
	Detail_Data = 4, 8032, hex
	Detail_Data = 4, 8033, hex
	Detail_Data = 4, 8035, hex
	Detail_Data = 4, 8036, hex
 
+ DSI_SLA:
	Err_Type = PERM
	Class = H
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "data storage interrupt : SLA"
	Err_Desc = E834
	Prob_Causes = E822, 1000
	Fail_Causes = E8E7, 1000
	Fail_Actions = 0000,3301,3000
	Detail_Data = 4, 8032, hex
	Detail_Data = 4, 8033, hex
	Detail_Data = 4, 8035, hex
 
 
+ MACHINECHECK:
        Report = TRUE
        Log    = TRUE
        Alert  = FALSE
        Err_Desc = E830
        Prob_Causes = E823, E820, 0005
        Fail_Causes = E8E8, E8E5, 0005
        Fail_Actions = 0000
        Class  = H
        Err_Type = PERM
    	Detail_Data = 1, 8051, HEX
    	Detail_Data = 1, 8052, ALPHA
    	Detail_Data = 2, 8060, HEX
    	Detail_Data = 4, 8053, HEX
    	Detail_Data = 4, 8054, HEX
    	Detail_Data = 4, 8055, HEX
    	Detail_Data = 4, 8056, HEX
        Comment = "machine check"

+ MACHINE_CHECK_604:
        Comment = "Machine check interrupt (604)"
        Class = H
        Log = True
        Report = True
        Alert = False
        Err_Type = PEND
        Err_Desc = E830
        Prob_Causes = EC50, EC51, EC52
        Fail_Causes = EB45, EB46, EB47
        Fail_Actions = EC1D, EA02
        Detail_Data = 4, EB2D, HEX
        Detail_Data = 4, 8040, HEX
        Detail_Data = 4, 8041, HEX

+ PEGA_PARITY_ERR:
        Comment = "Single/multiple bit parity error"
        Class = H
        Log = True
        Report = True
        Alert = False
        Err_Type = PEND
        Err_Desc = E845
        Prob_Causes = E860, E861
        Fail_Causes = E920, E921
        Fail_Actions = EC1C, EA02
        Detail_Data = 2, 0013, HEX
        Detail_Data = 1, 005B, HEX
        Detail_Data = 2, EC36, HEX
        Detail_Data = 4, EC37, HEX
 
 
+ KERNEL_PANIC:
	Err_Type = TEMP
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "Kernel Panic!"
	Err_Desc = 2000
	Prob_Causes = FFFF
	Fail_Causes = FFFF
	Fail_Actions = 0000
	Detail_Data = 50, 8141, ALPHA
	Detail_Data = 50, 8140, ALPHA
 
+ DOUBLE_PANIC:
	Err_Type = TEMP
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "Double Panic!"
	Err_Desc = 2000
	Prob_Causes = FFFF
	Fail_Causes = FFFF
	Fail_Actions = 0000
	Detail_Data = 50, 8140, ALPHA
	Detail_Data = 50, 8140, ALPHA
 
+ EXCHECK_DMA:
	Err_Type = PERM
	Class = H
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "external check - dma error"
	Err_Desc = E835
	Prob_Causes = E821,0000, 1000
	Fail_Causes = E8E6, 0000, 1000
	Fail_Actions = 0000, 3301, 3000
	Detail_Data = 4, 8113, hex
	Detail_Data = 4, 8114, hex
	Detail_Data = 4, 8115, hex
	Detail_Data = 32, 8116, hex
 
+ EXCHECK_SCRUB:
	Err_Type = PERM
	Class = H
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "external check - memory scrub error"
	Err_Desc = B000
	Prob_Causes = 0101, 0000
	Fail_Causes = 0101, 0000
	Fail_Actions = 2000, 0000
	Detail_Data = 4, 8113, hex
	Detail_Data = 4, 8114, hex
	Detail_Data = 4, 8115, hex
	Detail_Data = 32, 8116, hex
 
+ NB1:
	Comment       = "RUN OUT OF PSB IN INTERRUPT HANDLER"
	Class  	      = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = TEMP
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E960
        Fail_Actions  = 3000
 
+ NB2:
	Comment       = "UNKNOWN ROUTER COMMAND OPTION"
	Class  	      = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = TEMP
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E961
        Fail_Actions  = 2200
	Detail_Data   = 4, AA03, HEX
 	
+ NB3:
	Comment       = "OUT OF IOB"
	Class  	      = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = TEMP
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E962
        Fail_Actions  = 3000
 	
+ NB4:
	Comment       = "DEVIOCTL - DISABLE SAP FAILED"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = PERM
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E963
        Fail_Actions  = 3301,3103
	Detail_Data   = 4, 0F, HEX
 	
+ NB5:
	Comment       = "SAP ALREADY CLOSED"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = TEMP
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E964
        Fail_Actions  = 3301,3103
 	
+ NB6:
	Comment       = "BUFFER OVERFLOW"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = TEMP
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E965
        Fail_Actions  = 3301,3103
 	
+ NB7:
	Comment       = "INACTIVITY WITHOUT TERMINATION"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = TEMP
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E966
        Fail_Actions  = 2000
 	
+ NB8:
	Comment       = "INACTIVITY ENDED"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = TEMP
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E967
        Fail_Actions  = 2000
 	
+ NB9:
	Comment       = "UNDEFINED RESULT INDICATOR"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = TEMP
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E968
        Fail_Actions  = 2000
	Detail_Data   = 4, AA12, HEX
 	
+ NB10:
	Comment       = "OUT OF IOB IN OPEN LINK STATION"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = TEMP
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E969
        Fail_Actions  = 3301,3103
 	
+ NB11:
	Comment       = "OUT OF IOB IN OPEN LINK STATION COMPLETION"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = TEMP
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E96A
        Fail_Actions  = 3301,3103
 	
+ NB12:
	Comment       = "DEVIOCTL (OPEN LINK STATION) FAILED"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = PERM
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E96B
        Fail_Actions  = 3301,3103
	Detail_Data   = 4, 0F, HEX
 	
+ NB13:
	Comment       = "OUT OF IOB IN CONNECT LINK STATION"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = PERM
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E96C
        Fail_Actions  = 3301,3103
 	
+ NB14:
	Comment       = "DEVIOCTL-CONTACT FAILED"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = PERM
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E96D
        Fail_Actions  = 3301,3103
	Detail_Data   = 4, 0F, HEX
 
+ NB15:
	Comment       = "CONNECT LINK STATION OUT OF RETRIES"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = PERM
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E96E
        Fail_Actions  = 3301,3103
 	
+ NB16:
	Comment       = "CONNECT LINK STATION FAILED, RETRY"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = TEMP
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E96F
        Fail_Actions  = 3301,3103
 	
 	
+ NB17:
	Comment       = "DEVIOCTL (HALT LINK STATION) FAILED"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = PERM
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E970
        Fail_Actions  = 2000
	Detail_Data   = 4, 0F, HEX
 	
+ NB18:
	Comment       = "OUT OF IOB IN ENABLE SAP"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = PERM
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E971
        Fail_Actions  = 3301,3103
 	
+ NB19:
	Comment       = "DEVIOCTL (ENABLE SAP) FAILED"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = PERM
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E972
        Fail_Actions  = 3301,3103
	Detail_Data   = 4, 0F, HEX
 	
+ NB20:
	Comment       = "DEVIOCTL-WRITE FAILED"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = PERM
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E973
        Fail_Actions  = 2000
	Detail_Data   = 4, 0F, HEX
 	
+ NB21:
	Comment       = "ERROR OPERATION RESULT FROM LLC"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = PERM
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E974
        Fail_Actions  = 2000
	Detail_Data   = 4, AA12, HEX
 	
+ NB22:
	Comment       = "UNKNOWN USER-SAP-CORRELATOR RECEIVED IN INTERRUPT HANDLER"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = PERM
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E975
        Fail_Actions  = 2000
	Detail_Data   = 4, AA27, HEX
 	
+ NB23:
	Comment       = "UNKNOWN USER-SAP-CORRELATOR IN RECEIVE-FUNCTION"
	Class         = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = PERM
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E976
        Fail_Actions  = 2000
	Detail_Data   = 4, AA27, HEX
 	
+ NB24:
	Comment       = "INVALID CORRELATOR FROM OPEN SAP/LS RESPONSE"
	Class  	      = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = TEMP
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E977
        Fail_Actions  = 2000
	Detail_Data   = 4, AA30, HEX
 	
+ NB25:
	Comment       = "DEVSWADD SVC FAILED"
	Class  	      = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = PERM
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E978
        Fail_Actions  = 3301,3103
	Detail_Data   = 4, 0F, HEX
 	
+ NB26:
	Comment       = "DEVSWDEL SVC FAILED"
	Class  	      = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = PERM
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E979
        Fail_Actions  = 3301,3103
	Detail_Data   = 4, 0F, HEX
 	
+ NB27:
	Comment       = "OPEN PENDING ON TERMINATION"
	Class  	      = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = TEMP
	Err_Desc      = 2100
	Prob_Causes   = E86B
        Fail_Actions  = 3301,3103
	Fail_Causes   = E97A
 	
+ NB28:
	Comment       = "UNKNOWN CONFIG OPTIONS"
	Class  	      = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = TEMP
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E97B
        Fail_Actions  = 2000
	Detail_Data   = 4, AA35, HEX
 	
+ NB29:
	Comment       = "MALLOC SVC FAILED"
	Class  	      = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = PERM
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E97C
        Fail_Actions  = 3301,3103
 	
+ NB30:
	Comment       = "PALLOC SVC FAILED"
	Class  	      = S
	Report        = TRUE
	Log           = TRUE
	Err_type      = PERM
	Err_Desc      = 2100
	Prob_Causes   = E86B
	Fail_Causes   = E97D
        Fail_Actions  = 3301,3103
 
+ ATE_ERR1:
	comment = "no pacing character received"
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc =  3000
	Prob_Causes = 1023
	User_Causes = FFFF
	User_Actions = FFFF
	Inst_Causes = FFFF
	Inst_Actions = FFFF
	Fail_Causes = FFFF
	Fail_Actions = FFFF
 
+ ATE_ERR2:
	comment = "too many transmission errors"
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc =  3000
	Prob_Causes = 1023
	User_Causes =  FFFF
	User_Actions = FFFF
	Inst_Causes = FFFF
	Inst_Actions =  FFFF
	Fail_Causes = FFFF
	Fail_Actions = FFFF
 
+ ATE_ERR3:
	comment = "no acknowledgement from receiving site"
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc =  3000
	Prob_Causes =  1023
	User_Causes =  FFFF
	User_Actions = FFFF
	Inst_Causes = FFFF
	Inst_Actions = FFFF
	Fail_Causes = FFFF
	Fail_Actions = FFFF
 
+ ATE_ERR4:
	comment = "receiving site is not ready"
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc =  3000
	Prob_Causes = 1023
	User_Causes = FFFF
	User_Actions =  FFFF
	Inst_Causes = FFFF
	Inst_Actions =  FFFF
	Fail_Causes = FFFF
	Fail_Actions = FFFF
 
+ ATE_ERR5:
	comment = "sending site is not sending"
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc =  3000
	Prob_Causes = 1023
	User_Causes = FFFF
	User_Actions = FFFF
	Inst_Causes = FFFF
	Inst_Actions = FFFF
	Fail_Causes = FFFF
	Fail_Actions = FFFF
 
+ ATE_ERR6:
	comment = "no carrier signal"
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc =  3000
	Prob_Causes = 2303
	User_Causes = FFFF
	User_Actions = FFFF
	Inst_Causes = FFFF
	Inst_Actions =  FFFF
	Fail_Causes = FFFF
	Fail_Actions = FFFF
 
+ ATE_ERR7:
	comment = "checksum error"
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc =  3000
	Prob_Causes = 1022
	User_Causes = FFFF
	User_Actions = FFFF
	Inst_Causes = FFFF
	Inst_Actions = FFFF
	Fail_Causes = FFFF
	Fail_Actions = FFFF
 
+ ATE_ERR8:
	comment = "sector received twice"
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc =  3000
	Prob_Causes = 1022
	User_Causes = FFFF
	User_Actions = FFFF
	Inst_Causes = FFFF
	Inst_Actions = FFFF
	Fail_Causes = FFFF
	Fail_Actions = FFFF
 
+ ATE_ERR9:
	comment = "incorrect sector received"
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc =  3000
	Prob_Causes = 1022
	User_Causes = FFFF
	User_Actions = FFFF
	Inst_Causes = FFFF
	Inst_Actions = FFFF
	Fail_Causes = FFFF
	Fail_Actions = FFFF
 
+ ATE_ERR10:
	comment = "sector could not be verified"
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc =  3000
	Prob_Causes = 1022
	User_Causes = FFFF
	User_Actions = FFFF
	Inst_Causes = FFFF
	Inst_Actions =  FFFF
	Fail_Causes = FFFF
	Fail_Actions = FFFF
 
+ PROGRAM_INT:
	Err_Type = PERM
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "program interrupt"
	Err_Desc = E836
	Prob_Causes = 1000
	Fail_Causes = 1000
	Fail_Actions = 3301, 3000
	Detail_Data = 4, 8035, hex
	Detail_Data = 4, 8040, hex
	Detail_Data = 4, 8041, hex
	Detail_Data = 4, 8042, hex
 
+ FLPT_UNAVAIL:
	Err_Type = PERM
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "floating point unavailable"
	Err_Desc = B000
	Prob_Causes = E83C, 0000
	Fail_Causes = E930, 0000
	Fail_Actions = 2000, 0000
	Detail_Data = 4, 8035, hex
	Detail_Data = 4, 8040, hex
	Detail_Data = 4, 8042, hex
 
+ PSLA001:
	      Comment = "PSLA graphic unit or cable error "
	      Class = H
	      Log = True
	      Report = True
	      Alert = False
	      Err_Type = TEMP
	      Err_Desc = 1002
	      Prob_Causes = 3451
	      Fail_Causes = 3451
	      Fail_Actions = 0302, 0000
	      Detail_Data = 8,A2,ALPHA
	      Detail_Data = 8,A3,ALPHA
	      Detail_Data = 4,8005,HEX
	      Detail_Data = 4,07,HEX
	      Detail_Data = 4,0E,HEX
	      Detail_Data = 4,36,HEX

+ PSLA002:
	      Comment = "PSLA device driver or microcode error "
	      Class = S
	      Log = True
	      Report = True
	      Alert = False
	      Err_Type = TEMP
	      Err_Desc = 2100
	      Prob_Causes = 1000
	      Fail_Causes = 1000
	      Fail_Actions = 0000, 3301, 3000
	      Detail_Data = 8,A2,ALPHA
	      Detail_Data = 8,A3,ALPHA
	      Detail_Data = 4,8005,HEX
	      Detail_Data = 4,07,HEX
	      Detail_Data = 4,0E,HEX
	      Detail_Data = 4,36,HEX
	      Detail_Data = 24,11,HEX

+ PSLA003:
	      Comment = "PSLA host link error "
	      Class = H
	      Log = True
	      Report = True
	      Alert = False
	      Err_Type = TEMP
	      Err_Desc = 3300
	      Prob_Causes = 2080
	      Fail_Causes = EA0F
	      Fail_Actions = 0302, 0000, 3301, 3000
	      Detail_Data = 8,A2,ALPHA
	      Detail_Data = 8,A3,ALPHA
	      Detail_Data = 4,8005,HEX
	      Detail_Data = 4,07,HEX
	      Detail_Data = 4,0E,HEX
	      Detail_Data = 4,36,HEX


+ CORE_DUMP:
	      Comment = "Core dump caused by signal"
	      Class = S
	      Report = True
	      Log = True
	      Alert = False
	      Err_Type = PERM
	      Err_Desc = 2000
	      Prob_Causes = 1000
	      User_Causes = E810
	      User_Actions = 1300
	      Fail_Causes = 1000
	      Fail_Actions = 1205, 3301, 3000
	      Detail_Data = 4, 8050, DEC
	      Detail_Data = 4, 8030, DEC
	      Detail_Data = 4, 8175, DEC
	      Detail_Data = 4, 8176, DEC
	      Detail_Data = 33, EB14, ALPHA

+ ACCT_OFF:
        Err_Type = TEMP
        Class = O
        Report = TRUE
        Log    = TRUE
        Alert  = FALSE
        Comment = "Accounting off"
        Err_Desc = EC26
        Prob_Causes = E90D
	User_Causes = 0100
	User_Actions = EB8A,EB8B
        Fail_Causes =  FFFF
        Fail_Actions=  FFFF
 
+ EU_BAD_ADPT:
	      Comment = "Illegal adapter in expansion unit"
	      Class = H
	      Report = True
	      Log = True
	      Alert = False
	      Err_Type = PERM
	      Err_Desc = E847
	      Prob_Causes = E83D
	      Inst_Causes = E807
	      Inst_Actions = 1500
	      Fail_Causes = E8F6
	      Fail_Actions = 0000
	      Detail_Data = 4, 8118, DEC

+ LVM_SWREL:
    Comment = "SW relocation successful"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = E850
    Prob_Causes = E840
    Fail_Causes = E902
    Fail_Actions = E820, E821
    Detail_Data = 4, 8005, HEX
    Detail_Data = 4, 8130, DEC
    Detail_Data = 4, 8131, DEC

+ LVM_HWREL:
    Comment = "HW relocation successful"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = E851
    Prob_Causes = E840
    Fail_Causes = E902
    Fail_Actions = E820, E821
    Detail_Data = 4, 8005, HEX
    Detail_Data = 4, 8130, DEC

+ LVM_HWFAIL:
    Comment = "HW relocation failed"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = E852
    Prob_Causes = E841, 6310
    Fail_Causes = E903, 5001, 6310, 6330
    Fail_Actions = E822, 0000
    Detail_Data = 4, 8005, HEX
    Detail_Data = 4, 8130, DEC

+ LVM_BBFAIL:
    Comment = "Error during BB relocation"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = E853
    Prob_Causes = E842
    Fail_Causes = 6310, 6330, 3452
    Fail_Actions = 0000
    Detail_Data = 4, 8005, HEX
    Detail_Data = 4, 8130, DEC
    Detail_Data = 1, 8015, DEC

+ LVM_BBRELMAX:
    Comment = "Max retries during BB relocation"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = E853
    Prob_Causes = E843
    Fail_Causes = 6310, 6330, 3452
    Fail_Actions = 0000
    Detail_Data = 4, 8005, HEX
    Detail_Data = 4, 8130, DEC

+ LVM_BBEPOOL:
    Comment = "BB relocation pool is empty"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = E853
    Prob_Causes = E844
    Fail_Causes = 5001
    Fail_Actions = 1600
    Detail_Data = 4, 8005, HEX

+ LVM_BBDIRERR:
    Comment = "Error during BB dir IO op"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = E853
    Prob_Causes = E845
    Fail_Causes = 6310, 6330, 3452
    Fail_Actions = 0000
    Detail_Data = 4, 8005, HEX
    Detail_Data = 1, 8015, DEC
    Detail_Data = 1, 8134, DEC

+ LVM_BBDIRBAD:
    Comment = "BB directory corrupted"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = E853
    Prob_Causes = E846
    Fail_Causes = E904, E905
    Fail_Actions = 0000
    Detail_Data = 4, 8005, HEX
    Detail_Data = 4, 8130, DEC

+ LVM_BBDIRFUL:
    Comment = "BB directory is full"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = E854
    Prob_Causes = E847
    Fail_Causes = 5001
    Fail_Actions = 1600
    Detail_Data = 4, 8005, HEX

+ LVM_BBDIR90:
    Comment = "BB directory is over 90% full"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = E855
    Prob_Causes = 5001
    Fail_Causes = 5001
    Fail_Actions = 1600
    Detail_Data = 4, 8005, HEX
    Detail_Data = 2, 8132, HEX
    Detail_Data = 2, 8133, HEX

+ CFGMGR_OPTION:   
    Comment = "Configuration manager invalid option"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = E860
    Prob_Causes = E852,1000
    Fail_Causes = E910,E912,1000
    Fail_Actions = E832,3000
    User_Causes = E814
    User_Actions = E833
    Detail_Data = 130,8120,ALPHA

+ CFGMGR_FATAL_DB:
    Comment = "Configuration manager fatal database problem"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = E861
    Prob_Causes = E851,1000
    Fail_Causes = E911,E913,E914,E915
    Fail_Actions = E832,3000
    User_Causes = E811
    User_Actions = E832,3000
    Detail_Data = 130,8121,ALPHA

+ CFGMGR_NONFATAL_DB:
    Comment = "Configuration manager nonfatal database problem"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = E862
    Prob_Causes = E851,E850,1000
    Fail_Causes = E911,E913,E916,E915
    Fail_Actions = E832,3000
    User_Causes = E811
    User_Actions = E832,3000
    Detail_Data = 130,8121,ALPHA

+ CFGMGR_MEMORY:
    Comment = "Configuration manager not enough memory"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = E863
    Prob_Causes = E823,1000
    Fail_Causes = E8E8,1000,E932
    Fail_Actions = 1300,3000
    Detail_Data = 130,8121,ALPHA

+ CFGMGR_PROGRAM_NF:
    Comment = "Configuration manager program not found"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = E864
    Prob_Causes = E851,E850,1000
    Fail_Causes = E916,E913,1000
    Fail_Actions = E832,3000
    User_Causes = E811
    User_Actions = E832,3000
    Detail_Data = 130,8122,ALPHA

+ CFGMGR_LOCK:
    Comment = "Configuration manager could not acquire lock"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = E865
    Prob_Causes = E853,1000
    Fail_Causes = E917,1000
    Fail_Actions = 3000
    User_Causes = E813
    User_Actions = E834
    Detail_Data = 130,8121,ALPHA

+ CFGMGR_CHILD:
    Comment = "Configuration manager child process error"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = E866
    Prob_Causes = 1000
    Fail_Causes = E919,1000
    Fail_Actions = E830,E831,3000
    User_Causes = E811
    User_Actions = E830,E831,E832
    Detail_Data = 130,8122,ALPHA

+ DUMP_STATS:
    Comment = "System dump statistics"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = E870
    Prob_Causes = E865
    Fail_Causes = E925
    Fail_Actions = 0000
    User_Causes = E818
    User_Actions = 0000
    Detail_Data = 128,8137,ALPHA
    Detail_Data =  2,8058,DEC
    Detail_Data =  2,8059,DEC
    Detail_Data =  4,8138,DEC
    Detail_Data = 32,0091,ALPHA
    Detail_Data =  4,8139,DEC
    Detail_Data =  4,813A,DEC

+ SYSDUMP_SYMP:
	Comment = "Symptom data for a system dump"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = UNKN
	Err_Desc = E870
	Prob_Causes = E865
	Fail_Causes = E925
	Fail_Actions = 0000
	User_Causes = E818
	User_Actions = 0000
	Detail_Data = 230,813A,ALPHA

+ MEM1:
    Comment = "Memory card pair failure"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = E845
    Prob_Causes = E861,E862
    Fail_Causes = E921,E922
    Inst_Causes = E804
    Inst_Actions= 0000,3000
    Fail_Actions = 0000,3000
    Detail_Data = 1,3D,ALPHA
    Detail_Data = 1,3D,ALPHA

+ MEM2:
    Comment = "Memory SIMM failure"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = E845
    Prob_Causes = E860,E861
    Fail_Causes = E920,E921
    Inst_Causes = FFFF
    Inst_Actions = 0000,3000
    Fail_Actions = 0000,3000
    Detail_Data = 4,3D,ALPHA
    Detail_Data = 4,8125,DEC

+ MEM3:
    Comment = "Memory card failure"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = E845
    Prob_Causes = E861
    Fail_Causes = E921
    Fail_Actions = 0000,3000
    Inst_Causes = FFFF
    Inst_Actions= FFFF
    Detail_Data = 1,3D,ALPHA

+ PGSP_KILL:
    Comment = "Process killed due to low paging space"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = 2000
    Prob_Causes = E870
    Fail_Causes = E980,E981
    Fail_Actions = E840,E841
    Inst_Causes = FFFF
    Inst_Actions= FFFF
    Detail_Data = 32,A6,ALPHA
    Detail_Data = 4,8030,DEC
    Detail_Data = 4,8150,DEC

+ NLS_MAP:
    Comment = "NLS mapping problem"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = 2100
    Prob_Causes = 1000
    Fail_Causes = 1000
    Fail_Actions = E846,2001
    Detail_Data = 4,07,HEX

+ NLS_BADMAP:
    Comment = "bad NLS map removed"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = E838
    Prob_Causes = 1000
    Fail_Causes = E83C
    Fail_Actions = E846,2001
    Detail_Data = 4,07,HEX
    Detail_Data = 4,8044,HEX
    Detail_Data = 4,8045,HEX
    Detail_Data = 32,8046,ALPHA

 
+ LVM_MISSPVADDED:
    Comment = "A physical volume has been defined as missing"
    Class = S
    Report = TRUE
    Alert = FALSE
    Log = TRUE
    Err_Type = UNKN
    Err_Desc = E948
    Prob_Causes = E84B
    User_Causes = ffff
    User_Actions = ffff
    Fail_Causes = ffff
    Fail_Actions = ffff
    Detail_Data = 4,8005,HEX
 
+ LVM_MISSPVRET:
    Comment = "Physical volume is now active."
    Class = S
    Report = FALSE
    Alert = FALSE
    Log = FALSE
    Err_Type = UNKN
    Err_Desc = E949
    Prob_Causes = ffff
    User_Causes = ffff
    User_Actions = ffff
    Fail_Causes = ffff
    Fail_Actions = ffff
    Detail_Data = 4,8005,HEX
 
+ LVM_SA_WRTERR:
    Comment = "Failed to write Volume Group Status area"
    Class = H
    Report = TRUE
    Alert = FALSE
    Log = TRUE
    Err_Type = UNKN
    Err_Desc = E94A
    Prob_Causes = E84B
    User_Causes = ffff
    User_Actions = ffff
    Fail_Causes = ffff
    Fail_Actions = ffff
    Detail_Data = 4,8005,HEX
    Detail_Data = 4,8015,DEC

+ LVM_SA_PVMISS:
    Comment = "A physical volume has been defined as missing."
    Class = H
    Report = TRUE
    Alert = FALSE
    Log = TRUE
    Err_Type = UNKN
    Err_Desc = E943
    Prob_Causes = E84B
    User_Causes = ffff
    User_Actions = ffff
    Fail_Causes = ffff
    Fail_Actions = ffff
    Detail_Data = 4,8005,HEX

+ LVM_SA_STALEPP:
    Comment = "Physical partition marked stale."
    Class = S
    Report = TRUE
    Alert = FALSE
    Log = TRUE
    Err_Type = UNKN
    Err_Desc = E944
    Prob_Causes = ffff
    User_Causes = ffff
    User_Actions = ffff
    Fail_Causes = ffff
    Fail_Actions = ffff
    Detail_Data = 4,8161,HEX
    Detail_Data = 4,8160,DEC
    Detail_Data = 4,8162,HEX

+ LVM_SA_FRESHPP:
    Comment = "Physical partition marked active."
    Class = S
    Report = TRUE
    Alert = FALSE
    Log = TRUE
    Err_Type = UNKN
    Err_Desc = E945
    Prob_Causes = ffff
    User_Causes = ffff
    User_Actions = ffff
    Fail_Causes = ffff
    Fail_Actions = ffff
    Detail_Data = 4,8161,HEX
    Detail_Data = 4,8160,DEC
    Detail_Data = 4,8162,HEX

+ LVM_SA_QUORCLOSE:
    Comment = "Quorum lost, volume group closing."
    Class = H
    Report = TRUE
    Alert = FALSE
    Log = TRUE
    Err_Type = UNKN
    Err_Desc = E946
    Prob_Causes = E84C
    User_Causes = ffff
    User_Actions = ffff
    Fail_Causes = ffff
    Fail_Actions = ffff
    Detail_Data = 4,8005,HEX
    Detail_Data = 4,8163,DEC
    Detail_Data = 4,8164,DEC

+ LVM_MWCWFAIL:
    Comment = "Mirror write consistency cache write failed."
    Class = H
    Report = TRUE
    Alert = FALSE
    Log = TRUE
    Err_Type = UNKN
    Err_Desc = E947
    Prob_Causes = ffff
    User_Causes = ffff
    User_Actions = ffff
    Fail_Causes = ffff
    Fail_Actions = ffff
    Detail_Data = 4,8005,HEX
    Detail_Data = 4,8130,DEC
    Detail_Data = 4,8015,DEC

+ SDM_ERR1:
   Comment = "Software or microcode errors"
   Class = H
   Report = TRUE
   Log = TRUE
   Alert = FALSE
   Err_type = PERM
   Err_Desc = 6100
   Prob_Causes = 0421
   Inst_Causes = 1500, 1600
   Inst_Actions = 0104
   Fail_Causes = 0421
   Fail_Actions = 0000
   Detail_Data = 156,11,HEX

+ SDC_ERR1:
   Comment = "Controller/DASD Link Error"
   Class = H
   Report = TRUE
   Log = TRUE
   Alert = TRUE
   Err_type = PERM
   Err_Desc = 3300
   Prob_Causes = 0400, 3460, 6310
   User_Causes = 0220
   User_Actions = 0200
   Fail_Causes = 0400, 3460, 6310
   Fail_Actions = 0000
   Detail_Data = 20,11,HEX
   Detail_Data = 44,11,HEX
   Detail_Data = 44,11,HEX
   Detail_Data = 40,11,HEX
   Detail_Data = 8,11,HEX

+ SDC_ERR2:
   Comment = "Controller Hardware Error"
   Class = H
   Report = TRUE
   Log = TRUE
   Alert = TRUE
   Err_type = PERM
   Err_Desc = 1601
   Prob_Causes = 0400
   Fail_Causes = 0400
   Fail_Actions = 0000
   Detail_Data = 20,11,HEX
   Detail_Data = 44,11,HEX
   Detail_Data = 44,11,HEX
   Detail_Data = 40,11,HEX
   Detail_Data = 8,11,HEX

+ SDC_ERR3:
   Comment = "Recovered Controller Hardware Error"
   Class = H
   Report = TRUE
   Log = TRUE
   Alert = FALSE
   Err_type = TEMP
   Err_Desc = 1601
   Prob_Causes = 0400
   Fail_Causes = 0411
   Fail_Actions = 0000
   Detail_Data = 156,11,HEX

+ SDA_ERR1:
   Comment = "Adapter Hardware Error Condition"
   Class = H
   Report = TRUE
   Log = TRUE
   Alert = TRUE
   Err_type = PERM
   Err_Desc = 1601
   Prob_Causes = 3300
   Fail_Causes = 3300
   Fail_Actions = 0000
   Detail_Data = 36,11,HEX


+ SDA_ERR2:
   Comment = "Recovered Adapter Hardware Error"
   Class = H
   Report = TRUE
   Log = TRUE
   Alert = FALSE
   Err_type = TEMP
   Err_Desc = 1601
   Prob_Causes = 3300
   Fail_Causes	= 3300
   Fail_Actions = 0000
   Detail_Data = 36,11,HEX

+ SDA_ERR3:
   Comment = "permanent unknown system error"
   Class = H
   Err_Type = PERM
   Report = TRUE
   Log = TRUE
   Alert = FALSE
   Err_Desc = FE00
   Prob_Causes = 1000
   Fail_Causes = 1000
   Fail_Actions	= 0000, 3301, 3000
   Detail_Data = 36,11,HEX

+ SDA_ERR4:
   Comment = "temporary unknown system error"
   Class = H
   Err_Type = TEMP
   Report = TRUE
   Log = TRUE
   Alert = FALSE
   Err_Desc = FE00
   Prob_Causes = 1000
   Fail_Causes = 1000
   Fail_Actions	= 0000, 3301, 3000
   Detail_Data = 36,11,HEX

+ SYS_RESET:
   Comment = "system reset button pushed"
   Class = S
   Err_Type = TEMP
   Report = TRUE
   Log = TRUE
   Alert = FALSE
   Err_Desc = E960
   Prob_Causes = E86E
   Fail_Causes = FFFF 
   Fail_Actions	= FFFF
   Detail_Data = 8,8170,ALPHA
   Detail_Data = 8,8171,ALPHA

+ REBOOT_ID:
   Comment = "system shutdown by user"
   Class = S
   Err_Type = TEMP
   Report = TRUE
   Log = TRUE
   Alert = FALSE
   Err_Desc = E961
   Prob_Causes = E86D
   Fail_Causes = FFFF
   Fail_Actions	= FFFF 
   Detail_Data = 4,8172,DEC
   Detail_Data = 4,8173,DEC
   Detail_Data = 4,8174,DEC

+ TMSCSI_READ_ERR:
   Comment = "non-retriable hdw err receiving data"
   Class = H
   Log = TRUE
   Report = TRUE
   Alert = FALSE
   Err_Type = TEMP
   Err_Desc = E940
   Prob_Causes = E810, 3451
   Fail_Causes = E998, 3451
   Fail_Actions = E862, E860, 3001
   Detail_Data = 156, 11, HEX

+ TMSCSI_CMD_ERR:
   Comment = "non-retriable hdw err sending cmd"
   Class = H
   Log = TRUE
   Report = TRUE
   Alert = FALSE
   Err_Type = TEMP
   Err_Desc = E941
   Prob_Causes = E811, 3451
   Fail_Causes = E999, 3451
   Fail_Actions = E861, E860, 3001
   Detail_Data = 156, 11, HEX

+ TMSCSI_UNRECVRD_ERR:
   Comment = "unrecovered error sending command"
   Class = H
   Log = TRUE
   Report = TRUE
   Alert = FALSE
   Err_Type = PERM
   Err_Desc = E941
   Prob_Causes = E811, 3451
   Fail_Causes = E999, 3451
   Fail_Actions = E861, E860, 3001
   Detail_Data = 156, 11, HEX

+ TMSCSI_RECVRD_ERR:
   Comment = "recovered error sending command"
   Class = H
   Log = TRUE
   Report = TRUE
   Alert = FALSE
   Err_Type = TEMP
   Err_Desc = E941
   Prob_Causes = E811, 3451
   Fail_Causes = E999, 3451
   Fail_Actions = 0301, E861, E860, 3001
   Detail_Data = 156, 11, HEX

+ TMSCSI_UNKN_SFW_ERR:
   Comment = "device driver detected sfw err"
   Class = S
   Log = TRUE
   Report = TRUE
   Alert = FALSE
   Err_Type = UNKN
   Err_Desc = 2100
   Prob_Causes = 1000
   Fail_Causes = 1000
   Fail_Actions = 3001
   Detail_Data = 156, 11, HEX

+ CORRECTED_SCRUB:
   Comment = "Corrected memory via scrubbing"
   Class = H
   Log = TRUE
   Report = TRUE
   Alert = FALSE
   Err_Type = TEMP
   Err_Desc = E871
   Prob_Causes = 0101
   Fail_Causes = FFFF
   Fail_Actions = FFFF
   Detail_Data = 4,8177,HEX
   Detail_Data = 4,8178,HEX
   Detail_Data = 4,8179,HEX

+ ACPA_INITZ:
    Comment = "ACPA Host Independent Initialization failed"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP 
    Err_Desc = E966
    Prob_Causes = 1000 
    Fail_Causes = EA07 
    Fail_Actions = E846 
    Detail_Data = 5, 8180, ALPHA 
    Detail_Data = 5, 8184, ALPHA
    Detail_Data = 25, 8181, ALPHA 
    Detail_Data = 25, 8182, ALPHA
    Detail_Data = 5, 8183, ALPHA

+ ACPA_LOAD:
    Comment = "Failed loading microcode"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP 
    Err_Desc = E965 
    Prob_Causes =  1000
    Fail_Causes = EA08 
    Fail_Actions = E867, E868 
    Detail_Data = 5, 8180, ALPHA 
    Detail_Data = 5, 8184, ALPHA
    Detail_Data = 25, 8181, ALPHA 
    Detail_Data = 25, 8182, ALPHA
    Detail_Data = 5, 8183, ALPHA
    Detail_Data = 30, 8185, ALPHA 
    Detail_Data = 30, 8186, ALPHA

+ ACPA_MEM:
    Comment = "Failed pinning memory"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP 
    Err_Desc = E967
    Prob_Causes = 1000 
    Fail_Causes = EA09 
    Fail_Actions = E86A 
    Detail_Data = 5, 8180, ALPHA 
    Detail_Data = 5, 8184, ALPHA
    Detail_Data = 25, 8181, ALPHA 
    Detail_Data = 25, 8182, ALPHA
    Detail_Data = 5, 8183, ALPHA

+ ACPA_INTR1:
    Comment = "Interrupt handler registration failed"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP 
    Err_Desc = E968
    Prob_Causes = 1000 
    Fail_Causes = EA09 
    Fail_Actions = E846 
    Detail_Data = 5, 8180, ALPHA 
    Detail_Data = 5, 8184, ALPHA
    Detail_Data = 25, 8181, ALPHA 
    Detail_Data = 25, 8182, ALPHA
    Detail_Data = 5, 8183, ALPHA

+ ACPA_INTR2:
    Comment = "Unexpected interrupt"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP 
    Err_Desc = E969
    Prob_Causes = 1000 
    Fail_Causes = 1000 
    Fail_Actions = E846 
    Detail_Data = 5, 8180, ALPHA 
    Detail_Data = 5, 8184, ALPHA
    Detail_Data = 25, 8181, ALPHA 
    Detail_Data = 25, 8182, ALPHA

+ ACPA_INTR3:
    Comment = "Invalid interrupt"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP 
    Err_Desc = E96A
    Prob_Causes = 1000 
    Fail_Causes = EA0A 
    Fail_Actions = E846 
    Detail_Data = 5, 8180, ALPHA 
    Detail_Data = 5, 8184, ALPHA
    Detail_Data = 25, 8181, ALPHA 
    Detail_Data = 25, 8182, ALPHA

+ ACPA_INTR4:
    Comment = "Interrupt timed out"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP 
    Err_Desc = E96B
    Prob_Causes = 1000 
    Fail_Causes = EA0B 
    Fail_Actions = E846 
    Detail_Data = 5, 8180, ALPHA
    Detail_Data = 5, 8184, ALPHA
    Detail_Data = 25, 8181, ALPHA 
    Detail_Data = 25, 8182, ALPHA
    Detail_Data = 5, 8183, ALPHA

+ ACPA_IOCTL1:
    Comment = "Unsupported audio control request"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM 
    Err_Desc = E96C
    Prob_Causes = 1001 
    User_Causes = E819 
    User_Actions = E86B 
    Detail_Data = 5, 8180, ALPHA
    Detail_Data = 5, 8184, ALPHA
    Detail_Data = 25, 8181, ALPHA 
    Detail_Data = 25, 8182, ALPHA
    Detail_Data = 5, 8183, ALPHA

+ ACPA_IOCTL2:
    Comment = "Invalid ACPA ioctl request" 
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP 
    Err_Desc = E96C
    Prob_Causes = 1001 
    User_Causes = E81A 
    User_Actions = E86C 
    Detail_Data = 5, 8180, ALPHA
    Detail_Data = 5, 8184, ALPHA
    Detail_Data = 25, 8181, ALPHA 
    Detail_Data = 25, 8182, ALPHA
    Detail_Data = 5, 8183, ALPHA

+ ACPA_UCODE:
    Comment = "ACPA failed loading microcode"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP 
    Err_Desc = E96D
    Prob_Causes = 1000 
    Fail_Causes = 1000 
    Fail_Actions = E869 
    Detail_Data = 5, 8180, ALPHA
    Detail_Data = 5, 8184, ALPHA
    Detail_Data = 25, 8181, ALPHA 
    Detail_Data = 25, 8182, ALPHA
    Detail_Data = 5, 8183, ALPHA

+ EXCHECK_RSC:
	Err_Type = PERM
	Class = H
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "external check - dma error"
	Err_Desc = E835
	Prob_Causes = E821,0000, 1000
	Fail_Causes = E8E6, 0000, 1000
	Fail_Actions = 0000, 3301, 3000
	Detail_Data = 4, 8113, hex
	Detail_Data = 4, 8114, hex
	Detail_Data = 4, 818A, hex
	Detail_Data = 4, 8125, hex
 
+ CAT_ERR1:
	Err_Type = PERM
	Class = H
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "microcode abnormally terminated"
	Err_Desc = 6000
	Prob_Causes = 3330,3331
	Fail_Causes = EA05
	Fail_Actions = 0000,1200,0104,1504
 
+ CAT_ERR2:
	Err_Type = PERM
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "microcode load failed"
	Err_Desc = 6100
	Prob_Causes = 3330,3331
	Fail_Causes = EA05
	Fail_Actions = 0000,1200,0104,1504
 
+ CAT_ERR3:
	Err_Type = TEMP
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "no mbuf available"
	Err_Desc = FE01
	Prob_Causes = 1020
	Fail_Causes = 1000
	Fail_Actions = 0000
 
+ CAT_ERR4:
	Err_Type = TEMP
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "no pinned memory available"
	Err_Desc = FE01
	Prob_Causes = E823
	Fail_Causes = 1000
	Fail_Actions = 0000
 
+ CAT_ERR5:
	Err_Type = TEMP
	Class = H
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "permanent channel adapter hardware error"
	Err_Desc = 1010
	Prob_Causes = 3330,3331
	Fail_Causes = EA05
	Fail_Actions = 0000,0400
 
+ CAT_ERR6:
	Err_Type = TEMP
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "370 Parallel Channel Adapter system resource problem"
	Err_Desc = FE01
	Prob_Causes = 1020
	Fail_Causes = 1000
	Fail_Actions = 0000
 
+ CAT_ERR7:
	Err_Type = TEMP
	Class = S
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "370 Parallel Channel Adapter driver resource problem"
	Err_Desc = FE01
	Prob_Causes = 1012
	Fail_Causes = 1000
	Fail_Actions = 0000
 
+ CAT_ERR8:
	Err_Type = TEMP
	Class = H
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "370 Parallel Channel Adapter detected a PIO error"
	Err_Desc = 1010
	Prob_Causes = 0005,E87A
	Fail_Causes = 0005,EA05
	Fail_Actions = 0000,0400
 
+ REPLACED_FRU:
	Err_Type = PERM
	Class = H
	Report = TRUE
	Log = TRUE
	Alert = FALSE
	Comment = "FRU replacement in the field"
	Err_Desc = E96E
	Prob_Causes = FFFF
	User_Causes = E81B
	User_Actions = E810
 
+ VCA_INITZ:
    Comment = "VCA Host independent initialization failed"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = E966
    Prob_Causes = 1000
    Fail_Causes = EA07
    Fail_Actions = E846
    Detail_Data = 5, 8184, ALPHA
    Detail_Data = 25, 8181, ALPHA
    Detail_Data = 25, 8182, ALPHA
    Detail_Data = 5, 8183, ALPHA

+ VCA_MEM:
    Comment = "failed pinning memory"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = E967
    Prob_Causes = 1000
    Fail_Causes = EA09
    Fail_Actions = E86A
    Detail_Data = 5, 8184, ALPHA
    Detail_Data = 25, 8181, ALPHA
    Detail_Data = 25, 8182, ALPHA
    Detail_Data = 5, 8183, ALPHA

+ VCA_INTR1:
    Comment = "interrupt handler registration failed"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = E968
    Prob_Causes = 1000
    Fail_Causes = EA09
    Fail_Actions = E846
    Detail_Data = 5, 8184, ALPHA
    Detail_Data = 25, 8181, ALPHA
    Detail_Data = 25, 8182, ALPHA
    Detail_Data = 5, 8183, ALPHA

+ VCA_INTR2:
    Comment = "Unexpected interrupt"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = E969
    Prob_Causes = 1000
    Fail_Causes = 1000
    Fail_Actions = E846
    Detail_Data = 5, 8184, ALPHA
    Detail_Data = 25, 8181, ALPHA
    Detail_Data = 25, 8182, ALPHA
    Detail_Data = 5, 8183, ALPHA

+ VCA_INTR3:
    Comment = "Invalid interrupt"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = E96A
    Prob_Causes = 1000
    Fail_Causes = EA0A
    Fail_Actions = E846
    Detail_Data = 5, 8184, ALPHA
    Detail_Data = 25, 8181, ALPHA
    Detail_Data = 25, 8182, ALPHA
    Detail_Data = 5, 8183, ALPHA

+ VCA_INTR4:
    Comment = "interrupt timed out"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = E96B
    Prob_Causes = 1000
    Fail_Causes = EA0B
    Fail_Actions = E846
    Detail_Data = 5, 8184, ALPHA
    Detail_Data = 25, 8181, ALPHA
    Detail_Data = 25, 8182, ALPHA
    Detail_Data = 5, 8183, ALPHA

+ VCA_IOCTL1:
    Comment = "Unsupported video control request"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = E96C
    Prob_Causes = 1001
    User_Causes = E819
    User_Actions = E86E
    Detail_Data = 5, 8184, ALPHA
    Detail_Data = 25, 8181, ALPHA
    Detail_Data = 25, 8182, ALPHA
    Detail_Data = 5, 8183, ALPHA

+ VCA_IOCTL2:
    Comment = "Invalid VCA ioctl request"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = E96C
    Prob_Causes = 1001
    User_Causes = E81A
    User_Actions = E86F
    Detail_Data = 5, 8184, ALPHA
    Detail_Data = 25, 8181, ALPHA
    Detail_Data = 25, 8182, ALPHA
    Detail_Data = 5, 8183, ALPHA

+ TTY_INTR_HOG:
	Comment = "temporary interrupt hog detected by driver"
	Class = H
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = E874
	Prob_Causes = 3451, 3500, 3300
	Fail_Causes = E952, E953, E938
	Fail_Actions = E836, E83B, E83C, 1803
	Detail_Data = 4, 8005, HEX	* major minor number
	Detail_Data = 4, 8104, DECIMAL  * retry count

+ PSIGDELIVERY:
	Comment = "Program Interrupt During Signal Delivery"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E859
	Prob_Causes = EA30
	Fail_Causes = 1001
	Fail_Actions = 1340
	Detail_Data = 4, 8035, HEX
	Detail_Data = 4, 8052, HEX
	Detail_Data = 4, 8042, HEX
 
*
* FFDC Errors used when no error log record is given.
*

+ HARDWARE_SYMPTOM:
	Comment = "Symptom data reported by hardware"
	Class = H
	Log = TRUE
	Report = TRUE
	Alert = TRUE
	Err_Type = PERM
	Err_Desc = 1002
	Prob_Causes = 6000
	Fail_Causes = 6000
	Fail_Actions = 2000, 3000
 
+ SOFTWARE_SYMPTOM:
	Comment = "Symptom data reported by software"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = TRUE
	Err_Type = UNKN
	Err_Desc = 2100
	Prob_Causes = 1000
	Fail_Causes = 1000
	Fail_Actions = 105E, 3000
 
+ INIT_RAPID:
	Comment = "init command respawning too rapidly "
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = 2100
	Prob_Causes = 1000
	User_Causes = 4000
	User_Actions = 2000
	Detail_Data = 80, 12, ALPHA   * init message
	Detail_Data = 80, A5, ALPHA   * init message

+ INIT_UTMP:
	Comment = "init failed write of utmp entry"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = 2100
	Prob_Causes = 1000
	User_Causes = 4001
	User_Actions = 2000
	Detail_Data = 80, 12, ALPHA   * init message

+ INIT_CREATE:
	Comment = "init cannot create utmp"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = 2100
	Prob_Causes = 1000,0501
	User_Causes = 4000
	User_Actions = 2000
	Detail_Data = 80, 12, ALPHA   * init message

+ INIT_OPEN:
	Comment = "init cannot open"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = 2100
	Prob_Causes = 0501
	User_Causes = 4000
	User_Actions = 2000
	Detail_Data = 80, 12, ALPHA   * init message

+ INIT_UNKNOWN:
	Comment = "init unknown err logged -- check detail data"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = TEMP
	Err_Desc = 2100
	Prob_Causes = 1000
	User_Causes = 4000
	User_Actions = 2000
	Detail_Data = 80, 12, ALPHA   * init message

+ LOG_PRE_WRAP:
	Comment="Error log file premature wrap"
	Class=		S
	Log=		True
	Report=		True
	Alert=		False
	Err_Type=	INFO
	Err_Desc=	E83E
	Prob_Causes=1064
	User_Causes=EB21
	User_Actions=EB8A, EB67, 0018
	Detail_Data=4, EC03, DEC
	Detail_Data=4, EC04, DEC

+ LOG_REG_WRAP:
	Comment="Error log file regular wrap"
	Class=		S
	Log=		True
	Report=		True
	Alert=		False
	Err_Type=	INFO
	Err_Desc=	E83F
	Prob_Causes=E840
	User_Causes=EB27
	User_Actions=E810

+ LOG_NVRAM:
	Comment="Error demon nvram processing failures"
	Class=		S
	Log=		True
	Report=		True
	Alert=		False
	Err_Type=	TEMP
	Err_Desc=	EA10
	Prob_Causes=1000
	Fail_Causes=1000
	Fail_Actions=E800, E825
	Detail_Data=32, EC05, ALPHA
	Detail_Data=32, EC06, ALPHA
	Detail_Data=32, EC06, ALPHA

+ INTRPPC_ERR:
    Class = H
    Err_Type = UNKN
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "INTERRUPT unknown error"
    Err_Desc = FE00
    Prob_Causes = 0005, 1000, 3300, 6000
    Fail_Causes = FFFF
    Fail_Actions = 0000
    Detail_Data = 4,65,HEX           * Virtual Memory Handle (IOCC)
    Detail_Data = 4,818B,HEX         * Interrupt Level

+ GRAPHICS:
        Comment = "Graphics subsystem errors"
        Class = S
        Log = TRUE
        Report = TRUE
        Alert = FALSE
        Err_Type = PERM
        Err_Desc = 2100                  * software program error
        Prob_Causes = 1000               * software program
        Fail_Causes = 1000               * software program error
        Fail_Actions = 3302, 3000, 2001  * actions on failure
        Detail_Data = 80,804B,ALPHA      * detecting,failing,rc,err,unique


+ LOST_EVENTS:
        Err_Type = TEMP
        Class = S
        Report = TRUE
        Log    = TRUE
        Alert  = FALSE
        Comment = "error logging buffer lost events error"
        Err_Desc = EB85
        Prob_Causes = EA20
        Inst_Causes = EB35
        Inst_Actions= EA20
        Fail_Causes =  EA70
        Fail_Actions=  121, 1340
        Detail_Data = 8, EB20 ,ALPHA
        Detail_Data = 8, EB21 ,ALPHA
        Detail_Data = 10, EB22 ,ALPHA

+ BAD_BUF_ENTRY:
        Comment="Error demon annunciate bad buffer entry"
        Class=          S
        Log=            True
        Report=         True
        Alert=          False
        Err_Type=       INFO
        Err_Desc=       EB86
        Prob_Causes=1000
        Fail_Causes=1000
        Fail_Actions=E825
        Detail_Data=4, EB2B, HEX
        Detail_Data=4, EB2C, HEX
        Detail_Data=4, EB2D, HEX
        Detail_Data=16, EB2E, HEX

+ ODM_CHANGE_SWSERVAT:
        Comment="Errdemon cannont change SWservAt in ODM"
        Class=          S
        Log=            True
        Report=         True
        Alert=          False
        Err_Type=       INFO
        Err_Desc=       EB87
        Prob_Causes=1000
        Fail_Causes=1000
        Fail_Actions=E825
        Detail_Data=10, EB32, ALPHA
        Detail_Data=14, EB30, ALPHA
        Detail_Data=4, EB31, DEC

+ LOW_MBUFS:
        Comment="Low Mbufs"
        Class=          S
        Log=            True
        Report=         True
        Alert=          False
        Err_Type=       PERF
        Err_Desc=       5004
        Prob_Causes=	0107
        Fail_Causes=	E8F7
        Fail_Actions=	E817

+ MID_UCODE:
        Comment="Microcode CRC test failed"
        Report =        TRUE
        Log=            TRUE
        Alert=          FALSE
        Err_Type=       PERM
        Class=          H
        Err_Desc=       6100
        Prob_Causes=    0032, EC10, EC11, 0005
        Fail_Causes= 	0032, EA40
        Fail_Actions= 	00E1, 3000
        Detail_Data = 	10, 003D, ALPHA

+ MID_PIPE1:
	Comment="PIPE processor 1 not responding"
  	Report=		TRUE
  	Log=		TRUE
  	Alert=		FALSE
  	Err_Type=	PERM
  	Class=		H
  	Err_Desc=	1010
  	Prob_Causes=	1000, EC12, EC13
  	Fail_Causes=	EA40, EA41
  	Fail_Actions=	00E1, 3000
  	Detail_Data=	10, 003D, ALPHA

+ MID_PIPE2:
	Comment="PIPE processor 2 not responding"
	Report=		TRUE
	Log=		TRUE
	Alert=		FALSE
	Err_Type=	PERM
	Class=		H
	Err_Desc=	1010
	Prob_Causes=	EC13, EC14
	Fail_Causes=	EA41
	Fail_Actions=	00E1, 3000
	Detail_Data=	10, 003D, ALPHA

+ MID_PIPE3:
	Comment="PIPE processor 3 not responding"
	Report=		TRUE
	Log=		TRUE
	Alert=		FALSE
	Err_Type=	PERM
	Class=		H
	Err_Desc=	1010
	Prob_Causes=	EC14, EC15
	Fail_Causes=	EA41
	Fail_Actions=	00E1, 3000
	Detail_Data=	10, 003D, ALPHA

+ MID_PIPE4:
	Comment="PIPE processor 4 not responding"
	Report=		TRUE
	Log=		TRUE
	Alert=		FALSE
	Err_Type=	PERM
	Class=		H
	Err_Desc=	1010
	Prob_Causes=	EC15, EC16
	Fail_Causes=	EA41
	Fail_Actions=	00E1, 3000
	Detail_Data=	10, 003D, ALPHA

+ MID_BLAST:
	Comment="BLAST processor not responding"
	Report=		TRUE
	Log=		TRUE
	Alert=		FALSE
	Err_Type=	PERM
	Class=		H
	Err_Desc=	1010
	Prob_Causes=	EC16, EC17
	Fail_Causes=	EA41, EA40
	Fail_Actions=	00E1, 3000
	Detail_Data=	10, 003D, ALPHA

+ MID_BLASTB:
	Comment="BLAST (backend) processor not responding"
	Report=		TRUE
	Log=		TRUE
	Alert=		FALSE
	Err_Type=	PERM
	Class=		H
	Err_Desc=	1010
	Prob_Causes=	EC19, EC18
	Fail_Causes=	EA44, EA42, EA43, EA40
	Fail_Actions=	00E1, 3000
	Detail_Data=	10, 003D, ALPHA

+ MID_SW:
	Comment="Datastream (software) error"
	Report=		TRUE
	Log=		TRUE
	Alert=		FALSE
	Err_Type=	TEMP
	Class=		S
	Err_Desc=	2100
	Prob_Causes=	1000
	Fail_Causes=	1000
	Fail_Actions=	1504, 3000

+ BUMP_ERROR_TABLES:
        Err_Type = UNKN
        Class = H
        Report = TRUE
        Log    = TRUE
        Alert  = FALSE
        Comment = "bump errors detected"
        Err_Desc = EA11
        Prob_Causes = EB03
        Fail_Causes = E94C
        Fail_Actions=  EA21,EA22,3000
        Detail_Data = 4,  EC08, DECIMAL
        Detail_Data = 57, EC09, ALPHA

+ BUMP_INT:
        Err_Type = TEMP
        Class = H
        Report = TRUE
        Log    = TRUE
        Alert  = FALSE
        Comment = "Recoverable memory parity error"
        Err_Desc = EC56
        Prob_Causes = E823
        Fail_Causes =  E8E8
        Fail_Actions=  3302, E802

+ SSA_DISK_ERR4:
        Comment      = "physical volume hardware error"
        Class        = H
        Report       = True
        Log          = True
        Alert        = True
        Err_type     = Perm
        Err_Desc     = 1312
        Prob_Causes  = 6310
        Fail_Causes  = 6310,6330
        Fail_Actions = 0000
        Detail_Data  = 156,0011,hex

+ DISK_ERR4:
        Comment      = "physical volume recovered error"
        Class        = H
        Report       = True
        Log          = True
        Alert        = False
        Err_type     = Temp
        Err_Desc     = 1312
        Prob_Causes  = 5001,6310
        Fail_Causes  = 5001,6310
        Fail_Actions = 0000
        Detail_Data  = 156,0011,hex

+ SSA_DISK_ERR1:
        Comment      = "DASD Detected Software Error"
        Class        = H
        Report       = True
        Log          = True
        Alert        = True
        Err_type     = Perm
        Err_Desc     = 6100
        Prob_Causes  = 3331,1012
        Fail_Causes  = 3300,6310
        Fail_Actions = 0000
        Detail_Data  = 156,0011,hex

+ SSA_DISK_ERR2:
        Comment      = "DASD statistical data"
        Class        = H
        Report       = False
        Log          = True
        Alert        = False
        Err_type     = Temp
        Err_Desc     = 1312
        Prob_Causes  = 6310
        Fail_Causes  = 6310
        Fail_Actions = 0700
        Detail_Data  = 156,0011,hex

+ DISK_ERR1:
        Comment      = "physical volume media error"
        Class        = H
        Report       = True
        Log          = True
        Alert        = False
        Err_type     = Perm
        Err_Desc     = 1312
        Prob_Causes  = 5001
        Fail_Causes  = 5001
        Fail_Actions = 0000
        Detail_Data  = 156,0011,hex

+ SSA_DISK_ERR3:
        Comment      = "Recovered SSA Media Error"
        Class        = H
        Report       = False
        Log          = True
        Alert        = False
        Err_type     = Temp
        Err_Desc     = 1312
        Prob_Causes  = 5001
        Fail_Causes  = 5001
        Fail_Actions = 0000
        Detail_Data  = 156,0011,hex

+ SSA_LINK_ERROR:
        Comment      = "SSA serial link failures"
        Class        = H
        Report       = False
        Log          = True
        Alert        = False
        Err_type     = Temp
        Err_Desc     = 3300
        Prob_Causes  = 3452
        Fail_Causes  = 3452,6310,3300
        Fail_Actions = 0000
        Detail_Data  = 156,0007,hex

+ SSA_LINK_OPEN:
        Comment      = "SSA serial link open"
        Class        = H
        Report       = True
        Log          = True
        Alert        = False
        Err_type     = Perm
        Err_Desc     = EC63
        Prob_Causes  = EC2B
        User_Causes  = EC04
        User_Actions = EC19
        Inst_Causes  = EB36
        Inst_Actions = EC1A
        Fail_Causes  = 3452,6310,3300
        Fail_Actions = 0000
        Detail_Data  = 156,0007,hex


+ SSA_DETECTED_ERROR:
        Comment      = "SSA detected failures"
        Class        = H
        Report       = True
        Log          = True
        Alert        = False
        Err_type     = Perm
        Err_Desc     = EC60
        Prob_Causes  = 0201,6310
        Fail_Causes  = 6310,0201,0204
        Fail_Actions = 0000
        Detail_Data  = 156,0007,hex

+ SSA_DEVICE_ERROR:
        Comment      = "SSA device failures"
        Class        = H
        Report       = True
        Log          = True
        Alert        = True
        Err_type     = Perm
        Err_Desc     = 1312
        Prob_Causes  = 6310
        Fail_Causes  = 6310
        Fail_Actions = 0000
        Detail_Data  = 156,0007,hex

+ SSA_DEGRADED_ERROR:
        Comment      = "SSA Degraded Condition"
        Class        = H
        Report       = True
        Log          = True
        Alert        = False
        Err_type     = Perf
        Err_Desc     = EC61
        Prob_Causes  = 3300
        Fail_Causes  = 3300
        Fail_Actions = 0000
        Detail_Data  = 156,0007,hex

+ SSA_HDW_ERROR:
        Comment      = "SSA Hardware Error Condition"
        Class        = H
        Report       = True
        Log          = True
        Alert        = True
        Err_type     = Perm
        Err_Desc     = 1010
        Prob_Causes  = 3300
        Fail_Causes  = 3300
        Fail_Actions = 0000
        Detail_Data  = 156,0007,hex

+ SSA_HDW_RECOVERED:
        Comment      = "Recovered SSA Hardware Error"
        Class        = H
        Report       = False
        Log          = True
        Alert        = False
        Err_type     = Temp
        Err_Desc     = 1010
        Prob_Causes  = 3300
        Fail_Causes  = 3300
        Fail_Actions = 0000
        Detail_Data  = 156,0007,hex

+ SSA_SOFTWARE_ERROR:
        Comment      = "SSA Software or microcode errors"
        Class        = S
        Report       = True
        Log          = True
        Alert        = True
        Err_type     = Perm
        Err_Desc     = 2100
        Prob_Causes  = 1012,3331
        Inst_Causes  = 1500,1600
        Inst_Actions = 0104
        Fail_Causes  = 3300,6310
        Fail_Actions = 0000
        Detail_Data  = 156,0007,hex

+ SSA_LOGGING_ERROR:
        Comment      = "Unable to log an error against a pdisk"
        Class        = S
        Report       = True
        Log          = True
        Alert        = False
        Err_type     = Perm
        Err_Desc     = EC62
        Prob_Causes  = EC2A
        User_Causes  = E802
        User_Actions = EC18
        Detail_Data  = 156,0007,hex

+FC_PERM_DMA_ERROR:
        Class = H
        Err_Type = PERM
        Report = TRUE
        Log = TRUE
        Alert = FALSE
        Comment = "Permanent Adapter-detected DMA error"
        Err_Desc = EB75
        Prob_Causes = EB0A
        Fail_Causes = EA5A
        Fail_Actions = F011, E800
        Detail_Data = 40, D0, ALPHA
        Detail_Data = 104, 11, HEX


+FC_TEMP_DMA_ERROR:
        Class = H
        Err_Type = TEMP
        Report = TRUE
        Log = TRUE
        Alert = FALSE
        Comment = "Adapter-detected DMA error"
        Err_Desc = EB75
        Prob_Causes = EB0A
        Fail_Causes = EA5A
        Fail_Actions = F011
        Detail_Data = 40, D0, ALPHA
        Detail_Data = 104, 11, HEX

+FC_LINK_DOWN:
        Class = H
        Err_Type = TEMP
        Report = TRUE
        Log = TRUE
        Alert = FALSE
        Comment = "Link Down"
        Err_Desc = EB76
        Prob_Causes = 3400, EB0B, 3330
        Fail_Causes = EA03, 3400, EA5B, 3330
        Fail_Actions = 0302
        Detail_Data = 40, D0, ALPHA
        Detail_Data = 104, 11, HEX

+FC_LINK_UP:
        Class = H
        Err_Type = TEMP
        Report = TRUE
        Log = TRUE
        Alert = FALSE
        Comment = "Link Up"
        Err_Desc = EB77
        Prob_Causes = 3400, EB0B, 3330
        Fail_Causes = EA03, 3400, EA5B, 3330
        Fail_Actions = F011
        Detail_Data = 40, D0, ALPHA
        Detail_Data = 104, 11, HEX


