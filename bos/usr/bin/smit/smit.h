/* @(#)61       1.20  src/bos/usr/bin/smit/smit.h, cmdsmit, bos41J, 9515A_all 4/3/95 16:22:02 */

#ifndef _H_SMIT
#define _H_SMIT

/*
 * COMPONENT_NAME: (CMDSMIT) SMIT -- System Management Interface Tool
 *
 * FUNCTIONS: include file definitions.
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1989, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/* ----------------------------- smit alias' of asl defs ------------------- */
/* provide consistent "SM_" prefix for use by ODM object building utilities  */
/* utilizing symbolic constants                                              */

#define SM_NO                   ASL_NO
#define SM_YES                  ASL_YES

                                /* sm_cmd_opt.required (+ ASL_YES, ASL_NO)   */
#define SM_YES_NON_EMPTY        ASL_YES_NON_EMPTY
#define SM_EXCEPT_WHEN_EMPTY    ASL_EXCEPT_WHEN_EMPTY

                                /* sm_cmd_opt.op_type (field operation types)*/
#define SM_NOOP_ENTRY           ASL_NOOP_ENTRY
#define SM_RING_ENTRY           ASL_RING_ENTRY
#define SM_LIST_ENTRY           ASL_LIST_ENTRY
#define SM_NOOP_CLEAR_ENTRY     ((char) ('N'))
#define SM_RING_CLEAR_ENTRY     ((char) ('R'))
#define SM_LIST_CLEAR_ENTRY     ((char) ('L'))

                                /* sm_cmd_opt.entry_type (field entry types) */
#define SM_TEXT_ENTRY           ASL_TEXT_ENTRY
#define SM_RAW_TEXT_ENTRY       ASL_RAW_TEXT_ENTRY
#define SM_NUM_ENTRY            ASL_NUM_ENTRY
#define SM_SIGNED_NUM_ENTRY     ASL_SIGNED_NUM_ENTRY
#define SM_HEX_HEX              ASL_HEX_ENTRY
#define SM_FILE_FILE            ASL_FILE_ENTRY
#define SM_NO_ENTRY             ASL_NO_ENTRY
#define SM_INVISIBLE_ENTRY      ASL_INVISIBLE_ENTRY
/* ----------------------------- smit alias' of asl defs ------------------- */

                                /* sm_cmd_opt.multi_select values            */
                                /* SM_YES, SM_NO, and ...                    */
#define SM_YES_MULTI_PARAM      ((char) ('m'))
#define SM_YES_COMMA_SEP        ((char) (','))
#define SM_EMPTY                ((char) ('e'))

                                /* sm_menu_opt.next_type values              */
#define SM_MENU                 ((char) ('m'))
#define SM_DIALOGUE             ((char) ('d'))
#define SM_STANDARD_DIALOGUE    ((char) ('s'))
#define SM_NAME                 ((char) ('n'))
#define SM_INFO                 ((char) ('i'))

                                /* sm_name_hdr.type values                   */
#define SM_JUST_NEXT_ID         ((char) ('j'))
#define SM_CAT_RAW_NAME         ((char) ('r'))
#define SM_CAT_COOKED_NAME      ((char) ('c'))
                                /* (command to) list selected item           */
                                /* processing modes                          */
#define SM_FIRST_FIELD          ((char) ('1'))
#define SM_SECOND_FIELD         ((char) ('2'))
#define SM_ALL_FIELDS           ((char) ('a'))
#define SM_RANGE                ((char) ('r'))
#define SM_DATAFILE             ((char) ('f'))
#define SM_FIRST_DATAFILE       ((char) ('F'))
#define SM_COLOR_FIELD          ((char) ('c'))

                                /* sm_cmd_hdr.exec_mode, SM_PIPE_STDIO is    */
                                /* the default                               */
#define SM_PIPE_STDIO                   ((char) ('p'))
#define SM_PIPE_STDIO_BACKUP            ((char) ('P'))
#define SM_PIPE_STDIO_NO_SCROLL         ((char) ('n'))
#define SM_PIPE_STDIO_NO_SCROLL_BACKUP  ((char) ('N'))
#define SM_INHERIT_STDIO                ((char) ('i'))
#define SM_INHERIT_STDIO_BACKUP         ((char) ('I'))
#define SM_EXIT_EXEC                    ((char) ('e'))
#define SM_EXIT_EXEC_2                  ((char) ('E'))

#define SM_PIPE_STDIO_SEQ               ((char) ('q'))
#define SM_PIPE_STDIO_NO_SCROLL_SEQ     ((char) ('o'))
#define SM_EXIT_EXEC_SEQ                ((char) ('f'))
#define SM_INHERIT_STDIO_SEQ            ((char) ('j'))


                                /* sm_cmd_opt.disc_field_name "name_select"  */
                                /* names                                     */
#define SM_DISC_FIELD_RAWNAME           "_rawname"
#define SM_DISC_FIELD_COOKEDNAME        "_cookedname"


#endif /* _H_SMIT */
