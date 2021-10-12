/* @(#)16       1.38.1.9  src/bos/kernel/sys/POWER/scdisk.h, sysxdisk, bos411, 9428A410j 11/23/93 15:39:48 */
#ifndef _H_SCDISK
#define _H_SCDISK
/*
 * COMPONENT_NAME: (SYSXDISK) SCSI Disk Device Driver Include File
 *
 * FUNCTIONS:  NONE
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************************************************************/
/* scdisk.h header dependencies                                         */
/************************************************************************/

#include <sys/types.h>
#include <sys/scsi.h>

/************************************************************************/
/* SCSI CD-ROM, Disk, Read/Write Optical Drive Ioctls 			*/
/************************************************************************/

#define DKIORDSE		0x01	/* Read and return sense data on*/
					/* error.  			*/
#define DKIOWRSE		0x02	/* Write and return sense data 	*/
					/* on error.			*/
#define DKIOCMD			0x03	/* Issue a user-defined SCSI 	*/
					/* command    			*/
#define DKEJECT			0x04    /* Eject Media from drive      	*/
#define DKPMR			0x05    /* Prevent media removal 	*/
#define DKAMR			0x06	/* Allow media removal		*/
#define DKFORMAT		0x07	/* Format media			*/
#define DKAUDIO			0x08	/* CD-ROM Play Audio Operations	*/
#define DK_CD_MODE		0x09	/* Get or Change CD-ROM data 	*/
					/* modes 			*/


/************************************************************************/
/* Structure for Play Audio Operations on CD-ROM drives 		*/
/************************************************************************/

struct cd_audio_cmd {
	char 	audio_cmds;		/* CD_ROM audio commands or	*/
					/* operations.			*/

#define	CD_PLAY_AUDIO	  	0x1	/* Plays Audio on CD-ROM drive	*/
#define CD_PAUSE_AUDIO		0x2	/* Pause the current Play Audio */
					/* operation.			*/
#define CD_RESUME_AUDIO		0x3	/* Resume the paused Play Audio */
					/* operation.			*/
#define	CD_STOP_AUDIO		0x4	/* Stop the current Play Audio  */
					/* operation.			*/
#define	CD_INFO_AUDIO		0x5	/* Get information on current	*/
					/* status of Play Audio 	*/
					/* operation (i.e. track, 	*/
					/* volume, and whether it is	*/
					/* paused or not). 		*/
#define	CD_TRK_INFO_AUDIO	0x6	/* Get number of tracks or MSF	*/
					/* (minutes, seconds,frames) of	*/
					/* last track/index on media.	*/
#define CD_GET_TRK_MSF		0x7	/* Get MSF information on the	*/
					/* specified track.		*/
#define	CD_SET_VOLUME		0x8	/* Set the the audio volume of	*/
					/* the output ports of CD-ROM	*/
					/* drive. This value  can be    */
					/* logically or'ed with:	*/
					/* CD_PLAY_AUDIO		*/
					/* CD_PAUSE_AUDIO		*/
					/* CD_RESUME_AUDIO		*/
					/* CD_STOP_AUDIO		*/
					/* CD_INFO_AUDIO		*/
					/* to create a one combined	*/
					/* command, which will set the	*/
					/* volume as well.		*/


	char	msf_flag;		/* TRUE if Minutes, Seconds,	*/
					/* and Frames (MSF) is being	*/
					/* used.  FALSE if Track/Index	*/
					/* is being used.		*/
					/* This flag is ignored for the */
					/* CD_INFO_AUDIO and 		*/
					/* CD_GET_TRK_MSF operations, 	*/
					/* since these operation use    */
					/* both the current track       */
					/* and current msf information.	*/
	union {
		struct {		/* If msf_flag is false use this*/
					/* structure to specify location*/
			uchar first_track;
			uchar first_index;
			uchar last_track;
			uchar last_index;
			uchar resvd1;
			uchar resvd2;
		} track_index; 
		struct {		/* If msf_flag is true, use this*/
					/* structure to specify location*/
		  	uchar first_mins;
			uchar first_secs;
			uchar first_frames;
			uchar last_mins;
			uchar last_secs;
			uchar last_frames;
		} msf;
		struct {		/* If the audio_cmds is  	*/
					/* CD_INFO_AUDIO, then this 	*/
					/* structure will be returned   */
					/* with the current track/index */
					/* and current msf location.	*/
			uchar current_track;
			uchar current_index;
			uchar resvd1;
		  	uchar current_mins;
			uchar current_secs;
			uchar current_frames;			
		} info_audio;
		struct {		/* If the audio_cmds is 	*/
					/* CD_GET_TRK_MSF, then this	*/
					/* structure is used. The 	*/
					/* track field must be set by   */
					/* the caller.  The mins,       */
					/* seconds, and frames are 	*/
					/* returned for the supplied	*/
					/* track.			*/
			uchar track;
			uchar resvd1;
			uchar resvd2;
		  	uchar mins;
			uchar secs;
			uchar frames;
		}track_msf;
	      } indexing;
	uchar	volume_type;		/* Type of volume control to 	*/
					/* use.          		*/

#define	CD_VOLUME_ALL		0x1	/* Control the the volume of	*/
					/* all output ports to one	*/
					/* common volume level.		*/
#define	CD_VOLUME_CHNLS		0x2	/* Control the volume of 	*/
					/* individual output ports.	*/
#define	CD_ALL_AUDIO_MUTE 	0x3	/* Mute all output ports.	*/
				
#define	CD_SET_AUDIO_CHNLS	0x4	/* Set up which audio channels	*/
					/* map to which output ports of	*/
					/* the CD-ROM drive.		*/

	uchar	all_channel_vol;        /* This specifies the common	*/
					/* volume of all output ports	*/
					/* when the CD_VOLUME_ALL 	*/
					/* volume type is used.		*/
	uchar	status;			/* This field returns the status*/
					/* for the CD_INFO_AUDIO	*/
					/* operation.			*/

#define	CD_NO_AUDIO		0x9	/* No Play Audio in progress.	*/
#define CD_COMPLETED		0xa	/* Play operation successfully	*/
					/* completed.			*/
#define CD_STATUS_ERROR		0xb	/* Either invalid audio status  */
					/* or play operation stopped due*/
					/* to an error.			*/
#define CD_NOT_VALID		0xc	/* Audio status is not valid or */
					/* not supported.		*/


	uchar	resvd1;			/* reserved for future expansion*/
	uchar	resvd2;			/* reserved for future expansion*/
	uchar	resvd3;			/* reserved for future expansion*/
	uchar	resvd4;			/* reserved for future expansion*/
	uint	resvd5;			/* reserved for future expansion*/
	uint	resvd6;			/* reserved for future expansion*/


	uchar	out_port_0_vol;		/* Controls the volume of output*/
					/* port 0 of the CD-ROM drive.	*/
	uchar	out_port_1_vol;		/* Controls the volume of output*/
					/* port 1 of the CD-ROM drive.	*/
	uchar	out_port_2_vol;		/* Controls the volume of output*/
					/* port 2 of the CD-ROM drive.	*/
	uchar	out_port_3_vol;		/* Controls the volume of output*/
					/* port 3 of the CD-ROM drive.	*/

	uchar	out_port_0_sel;		/* Specifies which audio channel*/
					/* maps to output port 0.	*/
	uchar	out_port_1_sel;		/* Specifies which audio channel*/
					/* maps to output port 1.	*/
	uchar	out_port_2_sel;		/* Specifies which audio channel*/
					/* maps to output port 2.	*/
	uchar	out_port_3_sel;		/* Specifies which audio channel*/
					/* maps to output port 3.	*/

/* The following defines are used for the above out_port selections */

#define	CD_MUTE_PORT		0x0	/* Mute output port.		*/
#define CD_AUDIO_CHNL_0		0x1	/* Use audio channel 0 for this	*/
					/* output port.			*/
#define CD_AUDIO_CHNL_1		0x2	/* Use audio channel 1 for this	*/
					/* output port.			*/
#define CD_AUDIO_CHNL_2		0x4	/* Use audio channel 2 for this	*/
					/* output port.			*/
#define CD_AUDIO_CHNL_3		0x8	/* Use audio channel 3 for this	*/
					/* output port.			*/



};

/************************************************************************/
/* Structure for CD-ROM Data Modes 					*/
/************************************************************************/

struct mode_form_op {
  	uchar 	action;			/* Determines if this operation */
					/* is to get the current CD-ROM */
					/* data mode or	to change it to */
					/* the values specified	 below.	*/

#define	CD_GET_MODE		0x1	/* Get current CD-ROM data mode.*/
#define	CD_CHG_MODE		0x2	/* Change CD-ROM data mode.	*/

	uchar	cd_mode_form;		/* Specifies the CD-ROM data	*/
					/* mode.			*/

#define	CD_MODE1		0x1	/* CD-ROM Data Mode 1.		*/
#define	CD_MODE2_FORM1		0x2	/* CD-ROM XA Data Mode 2 Form 1	*/
					/* The device block size used	*/
					/* for this mode is 2048 bytes	*/
					/* per block.			*/
#define	CD_MODE2_FORM2		0x3	/* CD-ROM XA Data Mode 2 Form 2	*/
					/* The device block size used	*/
					/* for this mode is 2336 bytes	*/
					/* per block.			*/
#define CD_DA			0x4	/* CD-DA. The device block size */
					/* used	for this mode is 2352 	*/
					/* bytes per block.		*/

	uchar	resvd1;			/* reserved for future expansion*/
	uchar	resvd2;			/* reserved for future expansion*/
	uchar	resvd3;			/* reserved for future expansion*/
	uchar	resvd4;			/* reserved for future expansion*/
	uchar	resvd5;			/* reserved for future expansion*/
	uchar	resvd6;			/* reserved for future expansion*/

};
#endif /* _H_SCDISK */
