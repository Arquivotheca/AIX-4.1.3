/* @(#)47	1.3  src/bos/usr/bin/odme/odmold.h, cmdodm, bos411, 9428A410j 6/15/90 22:36:12 */
/*
 * COMPONENT_NAME: (ODME) ODMOLD.H - migration defines for new odm
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#define ODM_H

/*----------------------------------------------------------------*/
/* Data structures and manifest constants for the object facility */
/* Version 2.2                                                    */
/*----------------------------------------------------------------*/

/*------------------------------------------*/
/* Manifest Constants                       */
/*------------------------------------------*/

/*  LOCKING CONSTANTS */

#define OBJ_LOCK_SHR 0X0111   /* ANYONE ELSE CAN READ/WRITE TO THE CLASS */
                                /* WHILE IT IS OPENED WITH THIS FLAG.      */
                                /* READ/WRITE VALID.                       */

#define OBJ_LOCK_EX 0X0011     /* NO ONE ELSE CAN ACCESS THE CLASS WHILE IT */
                                /* IS OPEN WITH THIS FLAG. READ/WRITE VALID  */

#define OBJ_LOCK_NB 0X0001    /* OPEN THE CLASS FOR READ ONLY. ANYONE      */
                                /* ELSE CAN READ/WRITE TO THE CLASS WHILE IT */
                                /* IS OPENED WITH THIS FLAG.  READ VALID     */

/* PERMISSION CONSTANTS */
#define ODMOWNR 00400                /* READ PERMISSION   TO     OWNER   */
#define ODMOWNW 00200                /* WRITE PERMISSION  TO     OWNER   */
#define ODMGRPR 00040                /* READ PERMISSION   TO     GROUP   */
#define ODMGRPW 00020                /* WRITE PERMISSION  TO     GROUP   */
#define ODMOTHR 00004                /* READ PERMISSION   TO     OTHERS  */
#define ODMOTHW 00002                /* WRITE PERMISSION  TO     OTHERS  */



/*  RANDOM CONSTANTS */

#define MAX_BINARY_LENGTH         30000
#define MAX_CHAR_LENGTH 255       /* for key columns */
#define MAX_CLASS_NAME 255                  /* including the NULL character */
#define MAX_CLASS_NODE 20
#define MAX_CLASS_OPEN 30                      /* 30 OBJECTS CAN BE OPEN */
#define MAX_CLASS_PATH 255                 /* including the NULL character */
#define MAX_CLASS_REPEAT 10
#define MAX_DESCRIP_NAME 64             /* including the NULL character */
#define MAX_EXTENSIONS 5
#define MAX_LINK_LENGTH 255
#define MAX_LONGCHAR_LENGTH       1024             /* for non-key columns */
#define MAX_METHOD_LENGTH 255
#define MAX_MOUTPUT_SIZE 4096
#define MAX_NUM_DESCRIP 100
#define MAX_OBJ_METHODS 20
#define MAX_PARM_LIST 64
#define MAX_SEARCH_LENGTH 1024
#define MAX_SORT_DESCRIP 4
#define MAX_SUBJ_VALUE 14
#define MAX_USER_AREA 200
#define MAX_USER_KEYS 10

/*  CHECKPOINT TOGGLE  */

#define NOCHECKPT    3
#define VERSIONING   1
#define TRANSACTION  2

/*  SEARCH EXPANSION TOGGLE */

#define EXPAND 2
#define NOEXPAND 4
#define DO_INHERITING 32
#define DO_TO_INHERIT 64

#define ODM_DEFAULT 0


#define FIRST 8
#define NEXT 16


/*  OBJECT DROP / OBJECT CLASS CLOSE TOGGLE */

#define FORCE 1
#define NOFORCE 2


/* CHANGE/DELETE ITERATOR CONSTANTS */
#define CHANGE_ALL -1
#define DELETE_ALL -1

/*  DESCRIPTOR INDEX CHARACTERISTICS */

#define KEY 1
#define NO_KEY 2

