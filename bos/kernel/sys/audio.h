/* @(#)22	1.5.1.1  src/bos/kernel/sys/audio.h, sysxacpa, bos411, 9428A410j 7/22/92 21:51:55 */

#ifndef _H_AUDIO
#define _H_AUDIO

/*
 * COMPONENT_NAME: SYSXACPA - Multimedia Audio Capture and Playback Adapter
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/param.h>

/* These are the defined ioctl control operations */
#define AUDIO_INIT      0               /* obtain adapter information */
#define AUDIO_STATUS    1               /* obtian current adapter status */
#define AUDIO_CONTROL   2               /* change adpater characteristics */
#define AUDIO_BUFFER    3               /* query buffer status */
#define AUDIO_LOAD      4               /* load DSP code */
#define AUDIO_WAIT      5               /* let kernel write buffers drain */

#define LOAD_PATH       MAXPATHLEN      /* max # of chars in DSP path */

/* This is the definition for information about a sampling rate. */
typedef struct _audio_init
{
	long srate;                     /* the sampling rate in Hz */
	long bits_per_sample;           /* the number of bits per sample */
	long bsize;                     /* block size for this rate */
	int mode;                       /* PCM, ADPCM, ADPCMS, etc. */
	int channels;			/* number of audio channels */
	long position_resolution;       /* smallest increment for position */
	char loadpath[LOAD_PATH];       /* path of DSP code to be loaded */
	unsigned long flags;            /* variable, fixed, etc. */
	unsigned long operation;        /* the desired operation */
	int rc;                         /* return code for requested op */
	int slot_number;                /* slot number of the adapter */
	int device_id;                  /* adapter identification number */
	void *reserved;                 /* reserved pointer */
} audio_init;

/* These are the defined values for audio_query.mode. */
#define ADPCM           1               
#define PCM             2              
#define MU_LAW          3               /* mu-law */
#define A_LAW           5
#define SOURCE_MIX      6

/* The following defines can be used as input information in */
/* audio_init.flags. */
#define FIXED                   ( 1 << 0 )	/* fixed length data */
#define LEFT_ALIGNED            ( 1 << 1 )      /* variable length data */
#define RIGHT_ALIGNED           ( 1 << 2 )      /* variable length data */
#define TWOS_COMPLEMENT		( 1 << 3 )	/* 2's complement data */
#define SIGNED                  ( 1 << 4 )      /* unsigned data */
#define BIG_ENDIAN              ( 1 << 5 )      /* high order bit is to */
						/* the left [MSB] */

/* The following flags are returned by AUDIO_INIT as output information */
/* in audio_init.flags. */
#define PITCH                   ( 1 << 20 )     /* picth is supported */
#define INPUT                   ( 1 << 21 )     /* input is supported */
#define OUTPUT                  ( 1 << 22 )     /* output is supported */
#define MONITOR                 ( 1 << 23 )     /* monitor is supported */
#define VOLUME                  ( 1 << 24 )     /* volume is supported */
#define VOLUME_DELAY            ( 1 << 25 )     /* volume delay is supported */
#define BALANCE                 ( 1 << 26 )     /* balance is supported */
#define BALANCE_DELAY           ( 1 << 27 )     /* balance delay is supported */
#define TREBLE                  ( 1 << 28 )     /* treble control is supported */
#define BASS                    ( 1 << 29 )     /* bass control is supported */
#define BESTFIT_PROVIDED        ( 1 << 30 )	/* best fit returned */
#define LOAD_CODE               ( 1 << 31 )	/* DSP load needed */

/* These are the defined values for audio.init.operation. */
#define PLAY            1
#define RECORD          2

/* These are the defined values for audio_init.rc. */
#define NO_PLAY                 1       /* DSP code can't do play requests */
#define NO_RECORD               2       /* DSP code can't do record requests */
#define INVALID_REQUEST         4       /* request was invalid */
#define CONFLICT                5       /* conflict with open's flags */
#define OVERLOADED		6	/* out of DSP MIPS or memory */

#define AUDIO_IGNORE            -1      /* ignore this field */

/* These are the defined values for audio_init.device_id. */
#define MACPA                   2       /* this is the supported adapter */

/* This is used when changing the adapter state. */
typedef struct _audio_change
{
	void *dev_info;                 /* ptr to device dependent info */
	long input;                     /* the new input source */
	long output;                    /* the new output */
	long monitor;                   /* the new monitor state */
	long volume;                    /* the new volume level */
	long volume_delay;              /* the new volume delay */
	long balance;                   /* the new balance */
	long balance_delay;             /* the new balance delay */
	long treble;                    /* the new treble state */
	long bass;                      /* the new bass state */
	long pitch;                     /* the new pitch state */
} audio_change;

#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif

/* These are the values defined for audio_change.input. */
#define HIGH_GAIN_MIKE          0
#define LINE_1                  1
#define LINE_2                  2
#define LINES_1AND2             3
#define LOW_GAIN_MIKE           4
#define ALL_LINES               0xFFFFFFF

/* These are the values defined for audio_change.output. */
#define EXTERNAL_SPEAKER        1       /* external speaker */
#define INTERNAL_SPEAKER        2       /* internal speaker */
#define OUTPUT_1                4       /* first output */

/* These are the values defined for audio_change.monitor. */
#define MONITOR_OFF             0       /* no monitoring */
#define MONITOR_UNCOMPRESSED    1       /* uncompressed monitoring */
#define MONITOR_COMPRESSED      2       /* compressed monitoring */

/* This is the definition for information about the adapter's current state. */
typedef struct _audio_status
{
	long srate;                     /* the sampling rate in Hz */
	long bits_per_sample;           /* the number of bits per sample */
	long bsize;                     /* block size for this rate */
	int mode;                       /* PCM, ADPCM, MU_LAW, etc. */
	int channels;			/* number of audio channels */
	unsigned long flags;            /* various flags */
	unsigned long operation;        /* the current operation in progress */
	audio_change change;            /* status of input, output, etc. */
} audio_status;

/* These are the values defined for operation. */
#define STOPPED                 0       /* no operation is in progress */
#define PLAYING                 1       /* playback mode is on */
#define RECORDING               2       /* record mode is on */
#define UNINITIALIZED           0xFFFFFFFF     /* adapter is uninitialized */

/* This is the definition that defines an AUDIO_CONTROL request. */
typedef struct _audio_control
{
	unsigned int ioctl_request;     /* the desired ioctl request */
	void *request_info;             /* the request specifics */
	unsigned long position;         /* # of units before executing it */
	int return_code;                /* contains any error code */
} audio_control;

/* These are the values defined for audio_control.ioctl_request. */
#define AUDIO_CHANGE    0               /* change adapter characteristics */
#define AUDIO_START     1               /* start new operation */
#define AUDIO_STOP      2               /* stop current operation */
#define AUDIO_PAUSE     3               /* suspend the current operation */
#define AUDIO_RESUME    4               /* resume a suspended operation */

/* This is the definition for information about the buffer state. */
typedef struct _audio_buffer
{
	unsigned long flags;            /* indicates whether error occurred */
	unsigned long read_buf_size;    /* # of bytes in read queue */
	unsigned long write_buf_size;   /* # of bytes in write queue */
	unsigned long read_buf_time;    /* # of milliseconds in read queue */
	unsigned long write_buf_time;   /* # of milliseconds in write queue */
	unsigned long read_buf_max;     /* max # of bytes ever in read queue */
	unsigned long write_buf_max;    /* max # of bytes ever in write queue */
	unsigned long position;		/* # of milliseconds since last START */
	unsigned long position_type;    /* type of unit associated with */
					/* the position field */
	long read_buf_cap;		/* max capacity of read queue */
	long write_buf_cap;		/* max capacity of write queue */
	long request_buf_cap;		/* max number of requests that can be */
					/* enqueued */
} audio_buffer;

/* These are the possible error states that can be flagged in */
/* audio_buffer.flags. */
#define AUDIO_UNDERRUN  ( 1 << 0 )      /* indicates underflow occurred */
#define AUDIO_OVERRUN   ( 1 << 1 )      /* indicates overflow occurred */

/* These are the possible meanings for audio_buffer.position_type. */
#define POS_MSECS       0               /* position value is milliseconds */

/* This is the structure used in conjunction with the AUDIO_LOAD command. */
/* When used, the application loads the file specified by the device */
/* driver into the memory buffer described by the audio_load structure. */
typedef struct _audio_load
{
	char *buffer;                   /* ptr to buffer where code has */
					/* been loaded */
	unsigned long size;             /* number of bytes in buffer */
	unsigned long flags;            /* contains flags about buffer */
} audio_load;

#define LOAD_START      1               /* 1st part of buffer to load */
#define LOAD_END        2               /* last part of buffer to load */

#endif /* _H_AUDIO */
