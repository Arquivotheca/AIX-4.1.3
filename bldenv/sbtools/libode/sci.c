/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: ATOI
 *		alloc_comment_leader5192
 *		bcreate_undo
 *		check_path
 *		confirm_alloc
 *		copy_file
 *		defined
 *		full_set_name
 *		get_bmerge_action1877
 *		get_full_setname2332
 *		get_src_and_org_paths
 *		get_str
 *		getenv_user
 *		is_in_error
 *		log_error
 *		makedir
 *		match_comment_leader5253
 *		merge_elem
 *		new_set_cleanup
 *		new_set_insert
 *		real_check_in_file3530
 *		remove_working_file1230
 *		sci_add_to_list
 *		sci_add_to_list_as_is2657
 *		sci_all_list
 *		sci_ancestor_list3210
 *		sci_ancestor_update_list1082
 *		sci_ancestor_update_list21117
 *		sci_check_in_elem3309
 *		sci_check_in_file3594
 *		sci_check_in_list3441
 *		sci_check_in_list23482
 *		sci_config_lookup_list4206
 *		sci_create_files4643
 *		sci_delete_files4830
 *		sci_diff_rev_with_file4300
 *		sci_diff_rev_with_rev4368
 *		sci_edit_files
 *		sci_elem_cnt
 *		sci_first
 *		sci_get_comment_leader1385
 *		sci_has_log
 *		sci_init
 *		sci_init2
 *		sci_init3
 *		sci_is_branch
 *		sci_lock_list
 *		sci_lookup_ancestor_rev_list3040
 *		sci_lookup_latest_rev_list2993
 *		sci_lookup_leader_list2806
 *		sci_lookup_merge_rev_list3148
 *		sci_lookup_rev_list3088
 *		sci_lookup_user_rev_list2924
 *		sci_merge_list
 *		sci_new_list
 *		sci_next
 *		sci_outdate_list4043
 *		sci_outdate_list_p14113
 *		sci_outdate_list_p24154
 *		sci_read_file
 *		sci_read_files
 *		sci_real_fast_lookup_latest_rev_list5811
 *		sci_real_fast_lookup_user_rev_list5653
 *		sci_rm_submit
 *		sci_select_not_exists4999
 *		sci_set_cmt_leader_list2834
 *		sci_set_comment_leader5158
 *		sci_set_symbol_list4411
 *		sci_show_log_list4230
 *		sci_submit
 *		sci_trackfile
 *		sci_update_build_file3979
 *		sci_update_build_list4012
 *		set_and_log_kludge2867
 *		set_cleanup
 *		set_create
 *		set_delete
 *		set_exists
 *		set_file_pathname5312
 *		set_fopen
 *		set_fread
 *		set_insert
 *		set_log_pathname5322
 *		set_lookup
 *		set_path_pathname5332
 *		set_remove
 *		set_source_info
 *		setup_bcstemp
 *		simple_cmp_func
 *		src_ctl_config_lookup1521
 *		src_ctl_merge
 *		src_ctl_prep_merge1464
 *		src_ctl_set_remove2120
 *		src_ctl_setup_merge1450
 *		tail_pathname
 *		temp_func
 *		tmpfile_base
 *		tmpfile_create
 *		tmpfile_delete
 *		tmpfile_name
 *		track_insert
 *		trunk_revision
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *   OBJECT CODE ONLY SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 *  
 * Copyright (c) 1992 Carnegie Mellon University 
 * All Rights Reserved. 
 *  
 * Permission to use, copy, modify and distribute this software and its 
 * documentation is hereby granted, provided that both the copyright 
 * notice and this permission notice appear in all copies of the 
 * software, derivative works or modified versions, and any portions 
 * thereof, and that both notices appear in supporting documentation. 
 *  
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR 
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE. 
 *  
 * Carnegie Mellon requests users of this software to return to 
 *  
 * Software Distribution Coordinator  or  Software_Distribution@CS.CMU.EDU 
 * School of Computer Science 
 * Carnegie Mellon University 
 * Pittsburgh PA 15213-3890 
 *  
 * any improvements or extensions that they make and grant Carnegie Mellon 
 * the rights to redistribute these changes. 
 */
/*
 * HISTORY
 * $Log: sci.c,v $
 * Revision 1.5.7.21  1993/12/02  22:01:44  damon
 * 	CR 882. changed leader to sci_ptr -> leader
 * 	[1993/12/02  22:01:28  damon]
 *
 * Revision 1.5.7.20  1993/12/02  21:17:18  damon
 * 	CR 882. Check for NULL before doing strcmp() on leader
 * 	[1993/12/02  21:15:25  damon]
 * 
 * Revision 1.5.7.19  1993/11/12  18:10:24  damon
 * 	CR 789. sci_create_files() prints files as they are created
 * 	[1993/11/12  18:10:03  damon]
 * 
 * Revision 1.5.7.18  1993/11/10  19:17:09  root
 * 	CR 463. Pedantic changes
 * 	[1993/11/10  19:16:24  root]
 * 
 * Revision 1.5.7.17  1993/11/05  23:18:45  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/05  23:18:19  damon]
 * 
 * Revision 1.5.7.16  1993/11/05  22:43:24  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/05  22:41:29  damon]
 * 
 * Revision 1.5.7.15  1993/11/05  20:34:28  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/05  20:33:42  damon]
 * 
 * Revision 1.5.7.14  1993/11/03  20:58:18  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:57:46  damon]
 * 
 * Revision 1.5.7.13  1993/11/03  20:40:56  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:38:24  damon]
 * 
 * Revision 1.5.7.12  1993/11/03  00:08:32  damon
 * 	Merged with changes from 1.5.7.11
 * 	[1993/11/03  00:08:12  damon]
 * 
 * 	CR 739. Complain if revision of file is not present
 * 	[1993/11/03  00:04:36  damon]
 * 
 * Revision 1.5.7.11  1993/11/02  23:24:04  marty
 * 	CR # 757 - bdiff now prints out an error message if an invalid revision number is specified.
 * 	[1993/11/02  23:23:40  marty]
 * 
 * Revision 1.5.7.10  1993/10/29  12:51:26  damon
 * 	CR 766. Renamed #odexmXXXXXX to #srvtmpXXXXXX
 * 	[1993/10/29  12:47:32  damon]
 * 
 * Revision 1.5.7.9  1993/10/29  12:26:19  damon
 * 	CR 698. No files in set was still VFATAL. Changed to VWARN
 * 	[1993/10/29  12:26:06  damon]
 * 
 * Revision 1.5.7.8  1993/10/29  12:21:19  damon
 * 	CR 751. Give more explicit msg if file writeable at bco
 * 	[1993/10/29  12:11:07  damon]
 * 
 * Revision 1.5.7.7  1993/10/26  22:49:55  damon
 * 	CR 752. Give more detail
 * 	[1993/10/26  22:49:37  damon]
 * 
 * Revision 1.5.7.6  1993/10/26  21:33:43  damon
 * 	CR 141. Complain if file is defunct
 * 	[1993/10/26  21:33:25  damon]
 * 
 * Revision 1.5.7.5  1993/10/07  20:18:28  damon
 * 	CR 733. Properly set expansion mode depending on leader
 * 	[1993/10/07  20:18:11  damon]
 * 
 * Revision 1.5.7.4  1993/10/07  19:53:33  damon
 * 	CR 722. Complain when comment leader has no space at end
 * 	[1993/10/07  19:52:43  damon]
 * 
 * Revision 1.5.7.3  1993/10/06  23:51:33  damon
 * 	CR 697
 * 	Properly handle being in bottom link of backing chain.
 * 	Complain if trying to submit from same.
 * 	[1993/10/06  23:51:19  damon]
 * 
 * Revision 1.5.7.2  1993/10/06  22:19:23  damon
 * 	CR 717. Removed references to BCSTEMP env var
 * 	[1993/10/06  22:17:30  damon]
 * 
 * Revision 1.5.7.1  1993/10/06  21:57:16  damon
 * 	CR 729. Properly initialize tcp_service_number
 * 	[1993/10/06  21:57:02  damon]
 * 
 * Revision 1.5.5.18  1993/10/01  19:05:38  damon
 * 	CR 698. Made message warning of no files in set more palatable
 * 	[1993/10/01  19:03:24  damon]
 * 
 * Revision 1.5.5.17  1993/09/29  15:54:24  damon
 * 	CR 707. Handles mixed submission of defunct and regular files
 * 	[1993/09/29  15:53:31  damon]
 * 
 * Revision 1.5.5.16  1993/09/29  14:30:07  root
 * 	rios porting errors
 * 	[1993/09/29  14:22:15  root]
 * 
 * Revision 1.5.5.15  1993/09/24  18:24:01  damon
 * 	CR 692. Fixed length mismatch for tmpdir2
 * 	[1993/09/24  18:21:46  damon]
 * 
 * Revision 1.5.5.14  1993/09/24  17:44:01  marty
 * 	CR # 686 - Fix double # # during bcreate.
 * 	[1993/09/24  17:43:35  marty]
 * 
 * Revision 1.5.5.13  1993/09/24  17:23:20  damon
 * 	CR 687. Fixed COPYRIGHT NOTICE checking
 * 	[1993/09/24  17:22:49  damon]
 * 
 * Revision 1.5.5.12  1993/09/24  17:14:43  marty
 * 	CR # 685 - fix sci_create_files() to check for check_copyrights before processing.
 * 	[1993/09/24  17:14:15  marty]
 * 
 * Revision 1.5.5.11  1993/09/23  14:29:19  damon
 * 	CR 656. Use raw copyrights instead of valid ones
 * 	[1993/09/23  14:28:21  damon]
 * 
 * Revision 1.5.5.10  1993/09/21  21:21:43  marty
 * 	Cr # 670 - sci_create_files() now supports named copyrights.
 * 	[1993/09/21  21:21:16  marty]
 * 
 * Revision 1.5.5.9  1993/09/16  17:19:16  damon
 * 	Added COPYRIGHT NOTICE handling
 * 	[1993/09/16  17:18:03  damon]
 * 
 * Revision 1.5.5.8  1993/09/07  16:34:22  damon
 * 	CR 625. Fixed bsubmit -info
 * 	[1993/09/07  16:33:30  damon]
 * 
 * Revision 1.5.5.7  1993/09/02  20:44:54  damon
 * 	CR 604. Handle ci return values properly
 * 	[1993/09/02  20:44:32  damon]
 * 
 * Revision 1.5.5.6  1993/09/02  17:22:46  damon
 * 	CR 648. sci_read_files() now prints each file processed
 * 	[1993/09/02  17:22:32  damon]
 * 
 * Revision 1.5.5.5  1993/09/02  15:55:26  damon
 * 	CR 631. Enabled abort option in bci
 * 	[1993/09/02  15:54:53  damon]
 * 
 * Revision 1.5.5.4  1993/09/01  19:44:58  marty
 * 	CR # 646 - Use strdup() on (char *) variables instead of strcpy().
 * 	[1993/09/01  19:44:37  marty]
 * 
 * Revision 1.5.5.3  1993/08/31  21:25:25  damon
 * 	CR 641. Take care of case where all files are being defuncted
 * 	[1993/08/31  21:24:56  damon]
 * 
 * Revision 1.5.5.2  1993/08/31  18:15:55  damon
 * 	CR 636. call okmesg more intelligently
 * 	[1993/08/31  18:14:13  damon]
 * 
 * Revision 1.5.5.1  1993/08/30  19:12:07  damon
 * 	CR 633. Restored writing to file for src_ctl_diff_rev_with_file
 * 	[1993/08/30  19:11:13  damon]
 * 
 * Revision 1.5.3.1  1993/07/09  21:09:33  damon
 * 	CR 601. -read checks out files read only
 * 	[1993/07/09  21:09:10  damon]
 * 
 * Revision 1.5.1.75  1993/06/30  18:57:40  marty
 * 	CR # 586 - Print out a little info if revision info gets out of sync.
 * 	[1993/06/30  18:56:56  marty]
 * 
 * Revision 1.5.1.74  1993/06/30  18:47:22  marty
 * 	CR # 586 - If  rcsstat does not return the exact list of files requested
 * 	(by the *real_fast* revision lookup routines) then return a status
 * 	ERROR.
 * 	[1993/06/30  18:46:59  marty]
 * 
 * Revision 1.5.1.73  1993/06/18  18:32:01  marty
 * 	CR # 593 - sci_submit should call enter().
 * 	[1993/06/18  18:31:38  marty]
 * 
 * Revision 1.5.1.72  1993/06/10  21:22:31  damon
 * 	CR 585. do not skip in sci_real_fast_lookup_user_rev_list()
 * 	[1993/06/10  21:22:19  damon]
 * 
 * Revision 1.5.1.71  1993/06/08  19:28:14  marty
 * 	CR # 575 - Get rid of all calls to src_ctl_config*() routines.
 * 	[1993/06/08  19:27:52  marty]
 * 
 * Revision 1.5.1.70  1993/06/08  19:21:23  damon
 * 	CR 579. Use File is defunct message for new branch when defuncting
 * 	[1993/06/08  19:20:41  damon]
 * 
 * Revision 1.5.1.69  1993/06/08  19:00:13  marty
 * 	Merged with changes from 1.5.1.68
 * 	[1993/06/08  18:59:55  marty]
 * 
 * 	CR # 476 - bcreate_undo() now makes a separate call to remove empty soruce control files.
 * 	[1993/06/08  18:56:06  marty]
 * 
 * Revision 1.5.1.68  1993/06/08  18:16:01  damon
 * 	CR 577. Run makedir() before renaming file from temp dir
 * 	[1993/06/08  18:15:43  damon]
 * 
 * Revision 1.5.1.67  1993/06/07  18:10:41  marty
 * 	CR # 508 - Remove #odexm directory after calling sci_*_real_fast_*() routines.
 * 	[1993/06/07  18:10:15  marty]
 * 
 * Revision 1.5.1.66  1993/06/07  16:14:05  marty
 * 	CR # 554 - Tools run outside a sandbox in the wrong directory exit gracefully.
 * 	[1993/06/07  16:13:32  marty]
 * 
 * Revision 1.5.1.65  1993/06/04  20:30:42  damon
 * 	CR 553. Init tcp_service_number to NULL
 * 	[1993/06/04  20:30:30  damon]
 * 
 * Revision 1.5.1.64  1993/06/03  18:21:30  marty
 * 	CR # 567 - sci_edit_files() checks to see if file exists before stating it.
 * 	[1993/06/03  18:21:10  marty]
 * 
 * Revision 1.5.1.63  1993/06/03  17:23:31  marty
 * 	CR # 69 - Roll back submission.
 * 	[1993/06/03  17:22:54  marty]
 * 
 * Revision 1.5.1.62  1993/06/03  17:01:28  marty
 * 	CR # 69 - bdiff now works on symbolic links with the "-r" AND "-R" options.
 * 	[1993/06/03  17:01:05  marty]
 * 
 * Revision 1.5.1.61  1993/06/02  19:58:19  marty
 * 	CR # 566 - Make src_path global.
 * 	[1993/06/02  19:58:00  marty]
 * 
 * Revision 1.5.1.60  1993/06/02  19:33:48  damon
 * 	CR 559. alloc_comment_leader now returns NULL if no match
 * 	[1993/06/02  19:32:46  damon]
 * 
 * Revision 1.5.1.59  1993/06/02  18:27:18  damon
 * 	CR 565. Fixed check for writeable file to deal with links
 * 	[1993/06/02  18:26:19  damon]
 * 
 * Revision 1.5.1.58  1993/06/02  17:50:30  damon
 * 	CR 563. No longer enter Initial Revision marker during submit of new file
 * 	[1993/06/02  17:49:57  damon]
 * 
 * Revision 1.5.1.57  1993/06/02  13:52:08  damon
 * 	CR 517. Cleaned up subprojects wrt sb.conf and sc.conf
 * 	[1993/06/02  13:50:52  damon]
 * 
 * Revision 1.5.1.56  1993/05/27  19:49:27  marty
 * 	CR # 558 - clean up for rios_aix build
 * 	[1993/05/27  19:49:03  marty]
 * 
 * 	CR # 558 - get it building on rios_aix
 * 	[1993/05/27  19:16:28  marty]
 * 
 * Revision 1.5.1.55  1993/05/27  14:37:53  marty
 * 	CR # 556 - No longer hangs, and doesn't create *.tmp file with comment leader
 * 	BIN or NONE.
 * 	[1993/05/27  14:37:25  marty]
 * 
 * Revision 1.5.1.54  1993/05/26  21:02:57  damon
 * 	CR 545. Use stat() instead of access() for sci_edit_files
 * 	[1993/05/26  20:46:45  damon]
 * 
 * Revision 1.5.1.53  1993/05/26  18:07:56  damon
 * 	CR 553. Get tcp_service_number from sc.conf
 * 	[1993/05/26  17:18:43  damon]
 * 
 * Revision 1.5.1.52  1993/05/26  01:15:39  damon
 * 	CR 452. Added -q to co in sci_read_file()
 * 	[1993/05/26  01:14:58  damon]
 * 
 * Revision 1.5.1.51  1993/05/24  19:16:07  marty
 * 	CR # 535 - Remove links when checking out a file.
 * 	[1993/05/24  19:15:36  marty]
 * 
 * Revision 1.5.1.50  1993/05/20  16:17:20  marty
 * 	CR # 455 - Print out a better message when detecting bad history section.
 * 	[1993/05/20  16:15:29  marty]
 * 
 * Revision 1.5.1.49  1993/05/17  14:59:16  marty
 * 	CR # 516 - Fix "fast" routines for looking up revision numbers.
 * 	[1993/05/17  14:58:56  marty]
 * 
 * Revision 1.5.1.48  1993/05/14  21:40:43  damon
 * 	CR 527. Fixed creation of org_path for null case
 * 	[1993/05/14  21:40:25  damon]
 * 
 * Revision 1.5.1.47  1993/05/14  16:51:34  damon
 * 	CR 518. Changed prj_read and prj_write to take full sb path
 * 	[1993/05/14  16:51:18  damon]
 * 
 * Revision 1.5.1.46  1993/05/13  16:48:35  marty
 * 	CR # 516 - sci_real_fast_lookup_user_rev_list() needs to handle defunct files.
 * 	[1993/05/13  16:48:15  marty]
 * 
 * Revision 1.5.1.45  1993/05/13  16:07:33  marty
 * 	CR # 516 - Fixes to sci_real_fast_lookup_user_rev_list().
 * 	[1993/05/13  16:07:02  marty]
 * 
 * 	CR # 516 - Debugging sci_real_fast_lookup_user_list().
 * 	[1993/05/13  15:32:02  marty]
 * 
 * Revision 1.5.1.44  1993/05/12  19:55:34  damon
 * 	CR 517. Merged with martys code
 * 	[1993/05/12  19:55:13  damon]
 * 
 * 	CR 517. Added subprojects
 * 	 *
 * 	Revision 1.5.1.43  1993/05/12  19:41:20  marty
 * 	CR # 480 - More support for "bcs -r"
 * 	[1993/05/12  16:25:40  marty]
 * 	 *
 * 	Revision 1.5.1.42  1993/05/11  21:12:01  damon
 * 	CR 468. Made file locking customizeable
 * 	[1993/05/11  21:11:01  damon]
 * 	 *
 * 	Revision 1.5.1.41  1993/05/11  17:10:39  marty
 * 	CR # 480, add "av[i]=NULL;" to sci_set_comment_leader().
 * 	[1993/05/11  17:10:19  marty]
 * 
 * Revision 1.5.1.40  1993/05/10  19:51:09  damon
 * 	CR 507. Removed bogus call to track_insert
 * 	[1993/05/10  19:50:39  damon]
 * 
 * Revision 1.5.1.39  1993/05/07  17:46:51  damon
 * 	CR 498. bci/bco now print file being processed
 * 	[1993/05/07  17:46:25  damon]
 * 
 * Revision 1.5.1.38  1993/05/07  14:43:47  marty
 * 	Update print statement, needs a carrage return.
 * 	[1993/05/07  14:43:29  marty]
 * 
 * Revision 1.5.1.37  1993/05/07  14:38:04  damon
 * 	CR 436. dont need to outdate outdated file
 * 	[1993/05/07  14:37:50  damon]
 * 
 * Revision 1.5.1.36  1993/05/06  19:30:32  damon
 * 	CR 482. Added sci_an*_up*_list2 to convert .BCSconfig to ancestry
 * 	[1993/05/06  19:30:13  damon]
 * 
 * Revision 1.5.1.35  1993/05/06  17:49:48  damon
 * 	CR 477. Made ancestry buffers dynamic
 * 	[1993/05/06  17:49:35  damon]
 * 
 * Revision 1.5.1.34  1993/05/06  14:46:34  marty
 * 	Rearrange order of include files for pmax_ultrix.
 * 	[1993/05/06  14:46:14  marty]
 * 
 * Revision 1.5.1.33  1993/05/05  19:16:54  damon
 * 	CR 479. sci_submit was using /tmp/_LOG_. Now uses opentemp() dir
 * 	[1993/05/05  19:16:40  damon]
 * 
 * Revision 1.5.1.32  1993/05/05  18:42:05  damon
 * 	CR 489. Added sci_select_not_exists()
 * 	[1993/05/05  18:41:26  damon]
 * 
 * Revision 1.5.1.31  1993/05/05  15:16:08  marty
 * 	Added include file sys/stat.h for hpux.
 * 	[1993/05/05  15:15:49  marty]
 * 
 * Revision 1.5.1.30  1993/05/05  14:18:14  damon
 * 	CR 491. Added sci_elem_cnt()
 * 	[1993/05/05  14:17:52  damon]
 * 
 * Revision 1.5.1.29  1993/05/04  21:02:26  damon
 * 	CR 483. getancestor now returns OE_ANCESTOR when no ancestor
 * 	[1993/05/04  21:02:08  damon]
 * 
 * Revision 1.5.1.28  1993/05/04  18:13:18  marty
 * 	Removed debugging code.
 * 	[1993/05/04  18:12:59  marty]
 * 
 * Revision 1.5.1.27  1993/05/04  17:47:06  marty
 * 	Changed call to rcsstat_f to rcsstat.
 * 	[1993/05/04  17:46:16  marty]
 * 
 * Revision 1.5.1.26  1993/04/30  21:58:26  marty
 * 	Set submitting to FALSE
 * 	[1993/04/30  21:58:02  marty]
 * 
 * Revision 1.5.1.25  1993/04/30  20:41:40  damon
 * 	CR 467. Check and set r/w permissions for bci/bco correctly
 * 	[1993/04/30  20:41:19  damon]
 * 
 * Revision 1.5.1.24  1993/04/29  20:54:49  marty
 * 	Added sci_real_fast_*() functions
 * 	[1993/04/29  20:54:21  marty]
 * 
 * Revision 1.5.1.23  1993/04/29  17:54:40  damon
 * 	CR 135. Fixed sci_diff_rev_with_file
 * 	[1993/04/29  17:54:05  damon]
 * 
 * Revision 1.5.1.22  1993/04/29  15:45:27  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/29  15:44:31  damon]
 * 
 * Revision 1.5.1.21  1993/04/29  14:25:35  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/29  14:25:10  damon]
 * 
 * Revision 1.5.1.20  1993/04/28  14:35:27  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/28  14:34:28  damon]
 * 
 * Revision 1.5.1.19  1993/04/26  19:23:34  damon
 * 	CR 428. CR 411. CR 135. ODE 2.2.1 patches
 * 	[1993/04/26  19:21:46  damon]
 * 
 * Revision 1.5.1.18  1993/04/26  16:24:15  damon
 * 	CR 407. Made rco/co rdiff/diff explainations better
 * 	[1993/04/26  16:24:01  damon]
 * 
 * Revision 1.5.1.17  1993/04/26  16:02:01  damon
 * 	CR 401. Removed varargs
 * 	[1993/04/26  16:01:39  damon]
 * 
 * Revision 1.5.1.16  1993/04/26  15:26:00  damon
 * 	CR 436. Merged with martys changes
 * 	[1993/04/26  15:25:10  damon]
 * 
 * 	CR 436. Now using ode2.3_server_base directory for sc.conf
 * 	 *
 * 	Revision 1.5.1.15  1993/04/21  21:48:51  marty
 * 	Cleaning up memory leaks.
 * 	[1993/04/21  21:46:08  marty]
 * 	 *
 * 	Revision 1.5.1.14  1993/04/16  15:39:55  damon
 * 	CR 436. Synching create_branch code
 * 	[1993/04/16  15:39:37  damon]
 * 	 *
 * 	Revision 1.5.1.13  1993/04/09  17:22:45  damon
 * 	CR 446. Remove warnings with added includes
 * 	[1993/04/09  17:21:54  damon]
 * 
 * Revision 1.5.1.12  1993/04/09  14:25:16  damon
 * 	Merged with Martys changes
 * 	[1993/04/09  14:24:43  damon]
 * 
 * 	CR 432. sci_outdate_list_p1 now calls src_ctl_outdate
 * 	[1993/04/09  14:22:09  damon]
 * 
 * 	CR 446. Clean up include files
 * 
 * Revision 1.5.1.11  1993/04/06  23:48:49  marty
 * 	Cleanup real_check_in() routine to free up unneeded allocated space.
 * 	[1993/04/06  23:44:24  marty]
 * 
 * Revision 1.5.1.10  1993/04/01  17:05:01  marty
 * 	Remove extra odexm_open() from sci_fast_lookup_revision().
 * 	[1993/04/01  17:04:40  marty]
 * 
 * Revision 1.5.1.9  1993/03/31  19:06:29  damon
 * 	CR 443. opentemp now just creates directory, no file
 * 	[1993/03/31  19:05:53  damon]
 * 
 * Revision 1.5.1.8  1993/03/30  20:44:38  damon
 * 	CR 436. Made ancestry update for bmerge better
 * 	[1993/03/30  20:44:08  damon]
 * 
 * Revision 1.5.1.7  1993/03/26  18:02:16  marty
 * 	Fix bug with argument passing to src_ctl_fast_lookup_revision().
 * 	[1993/03/26  18:00:41  marty]
 * 
 * Revision 1.5.1.6  1993/03/25  21:19:07  marty
 * 	Added sci_lookup_fast_rev_list() for quick revision
 * 	lookup of a large group of files.
 * 	[1993/03/25  21:18:44  marty]
 * 
 * Revision 1.5.1.5  1993/03/24  20:49:42  damon
 * 	CR 436. Added remove and resub options
 * 	[1993/03/24  20:38:16  damon]
 * 
 * Revision 1.5.1.4  1993/03/22  22:17:34  marty
 * 	Added comment_leader to all calls of src_ctl_branch_create().  Removed
 * 	call to sci_set_comment_leader() in sci_create_files().
 * 	[1993/03/22  22:14:08  marty]
 * 
 * Revision 1.5.1.3  1993/03/19  15:51:56  marty
 * 	Added NULL as last argument in odexm commands in sci_outdate_list()
 * 	and sci_set_comment_leader() to work with client/server model.
 * 	[1993/03/19  15:51:15  marty]
 * 
 * Revision 1.5.1.2  1993/03/17  21:15:05  damon
 * 	Making sure merge to trunk worked
 * 	[1993/03/17  21:14:45  damon]
 * 
 * Revision 1.5.1.1  1993/03/17  21:11:23  devrcs
 * 	Move to trunk for speed step 2
 * 
 * Revision 1.5  1993/03/17  21:10:10  devrcs
 * 	Move to trunk for speed step 1
 * 
 * Revision 1.4.5.35  1993/03/17  20:41:52  damon
 * 	CR 446. Fixed include files/ forward decls.
 * 	[1993/03/17  20:41:22  damon]
 * 
 * Revision 1.4.5.34  1993/03/17  19:40:45  damon
 * 	CR 432. Added src_ctl_set_ancestry for merges
 * 	[1993/03/17  19:39:21  damon]
 * 
 * 	CR 436. Tidy up temp file creation
 * 
 * Revision 1.4.5.33  1993/03/17  16:13:12  damon
 * 	CR 436. Tidy up temp file creation
 * 	[1993/03/17  16:12:52  damon]
 * 
 * Revision 1.4.5.32  1993/03/17  16:06:31  marty
 * 	Change alloc_comment_leader() so that gmatch() is called
 * 	with a filename, not a pathname.  This is done by using
 * 	path().
 * 	[1993/03/17  16:06:01  marty]
 * 
 * Revision 1.4.5.31  1993/03/16  21:26:54  marty
 * 	Allow src_ctl_diff_rev_with_file to operate on regular files (weed
 * 	out the symbolic links) for OT# 69
 * 	[1993/03/16  21:26:26  marty]
 * 
 * 	Revision 1.4.5.30  1993/03/16  19:04:34  marty
 * 	Removed getenv() call, replaced it with call to get_rc_value().
 * 	Also type cast alot of function calls to get rid of warnings.
 * 	[1993/03/16  18:59:02  marty]
 * 
 * Revision 1.4.5.29  1993/03/15  21:41:59  marty
 * 	Change alloc_comment_leader to match project defined comment
 * 	leaders defined in the COMMENT_LEADERS variable.
 * 	[1993/03/15  21:39:23  marty]
 * 
 * Revision 1.4.5.28  1993/03/15  19:26:48  marty
 * 	Ensure that the user has a branch on the file before trying
 * 	to "undo" it.
 * 	[1993/03/15  19:25:52  marty]
 * 
 * Revision 1.4.5.27  1993/03/15  19:12:12  marty
 * 	When "undo"ing a bcreate, indicate that no lock was set.
 * 	[1993/03/15  19:10:44  marty]
 * 
 * Revision 1.4.5.26  1993/03/15  18:13:56  damon
 * 	CR 436. Changed call to read_legal_copyrights
 * 	[1993/03/15  18:13:41  damon]
 * 
 * Revision 1.4.5.25  1993/03/15  18:10:20  damon
 * 	CR 436. Added sci_submit
 * 	[1993/03/15  18:09:49  damon]
 * 
 * Revision 1.4.5.23  1993/03/15  15:53:09  marty
 * 	Change sequence of calls inscrc_ctl_merge() when using
 * 	hst_* routines.
 * 	[1993/03/15  15:52:38  marty]
 * 
 * Revision 1.4.5.22  1993/03/05  16:04:26  marty
 * 	Check status of src_ctl_check_in() before updating history
 * 	of the file.  On error don't update history.
 * 	[1993/03/05  16:00:45  marty]
 * 
 * Revision 1.4.5.21  1993/03/04  21:45:43  damon
 * 	CR 436. Added sci_add_to_list_as_is
 * 	[1993/03/04  20:02:03  damon]
 * 
 * Revision 1.4.5.20  1993/03/04  20:25:11  marty
 * 	Added calls to history manipulation routines for src_ctl_merge and
 * 	real_check_in_file().
 * 	[1993/03/04  20:24:28  marty]
 * 
 * 	Added routines split_files and join_files.
 * 	[1993/02/24  15:00:18  marty]
 * 
 * Revision 1.4.5.19  1993/02/19  22:42:00  marty
 * 	Remove lock_sb() call from sci_init().
 * 	[1993/02/19  22:40:16  marty]
 * 	Working Version 1
 * 
 * Revision 1.4.5.18  1993/02/19  17:35:50  damon
 * 	CR 193
 * 	sci_edit_files assumes sci_lookup_user_rev_list has
 * 	already been called.
 * 	[1993/02/19  17:31:30  damon]
 * 
 * Revision 1.4.5.17  1993/02/11  21:07:12  damon
 * 	CR 390. Changed rcs -o- to rcs -o:
 * 	[1993/02/11  21:06:48  damon]
 * 
 * Revision 1.4.5.16  1993/02/11  19:56:25  damon
 * 	CR 432. Now calls outdate instead of rcs -o
 * 	[1993/02/11  19:56:03  damon]
 * 
 * Revision 1.4.5.15  1993/02/10  18:34:44  damon
 * 	CR 432. Tracks simple ancestry
 * 	[1993/02/10  18:34:11  damon]
 * 
 * Revision 1.4.5.14  1993/02/06  21:20:47  damon
 * 	CR 429. Added sci_undo_submit
 * 	[1993/02/06  21:18:46  damon]
 * 
 * Revision 1.4.5.13  1993/02/04  21:18:32  damon
 * 	CR 230. Changed rename back to copy_file in sci_update_build_list
 * 	[1993/02/04  21:16:04  damon]
 * 
 * Revision 1.4.5.12  1993/02/04  21:02:12  damon
 * 	Fixing submission problem
 * 	[1993/02/04  21:01:54  damon]
 * 
 * Revision 1.4.5.11  1993/02/03  15:50:32  damon
 * 	CR 230
 * 	BCSTEMP now defaults to <sandbox base><sandbox>/tmp.
 * 	Movement of files from the tmp directory into the
 * 	sandbox is now handled by rename instead of copy_file.
 * 	[1993/02/03  15:50:07  damon]
 * 
 * Revision 1.4.5.10  1993/02/01  21:54:55  damon
 * 	CR 417. Uses sc.conf file instead of local and shared
 * 	[1993/02/01  21:53:45  damon]
 * 
 * Revision 1.4.5.9  1993/01/26  16:34:42  damon
 * 	CR 396. Conversion to err_log
 * 	[1993/01/26  16:34:04  damon]
 * 
 * Revision 1.4.5.8  1993/01/25  21:28:19  damon
 * 	CR 396. Converted history.c to err_log
 * 	[1993/01/25  21:26:51  damon]
 * 
 * Revision 1.4.5.7  1993/01/21  20:36:13  damon
 * 	CR 197. Fixed logic for -ko in sci_set_comment_leader
 * 	[1993/01/21  20:35:37  damon]
 * 
 * Revision 1.4.5.6  1993/01/21  00:06:23  damon
 * 	CR 382. Removed cruft
 * 	[1993/01/21  00:04:36  damon]
 * 
 * Revision 1.4.5.5  1993/01/20  22:21:27  damon
 * 	CR 376. Moved more code out from sci.c
 * 	[1993/01/20  22:16:11  damon]
 * 
 * Revision 1.4.5.4  1993/01/18  20:39:47  damon
 * 	CR 197. Fixed keyword expansion for BIN/NONE
 * 	[1993/01/18  20:35:55  damon]
 * 
 * Revision 1.4.5.3  1993/01/15  16:13:18  damon
 * 	CR 376. Renamed from sci_rcs.c
 * 	[1993/01/15  15:50:50  damon]
 * 
 * Revision 1.4.5.2  1993/01/13  17:52:03  damon
 * 	CR 196. Removed rcs locking
 * 	[1993/01/13  17:51:09  damon]
 * 
 * Revision 1.4.1.19  1992/12/18  18:58:46  damon
 * 
 * 	ODE 2.2
 * 	CR 365. Does not call okmesg if NONE or BIN
 * 	CR 370. Added -common functionality
 * 	CR 362. bci -m works without -auto
 * 	CR 361. Now warns about checking out defunct file
 * 	CR 357 356. Now creates missing directories for bcreate/bco
 * 	CR 333. fixed call to copy_file
 * 	CR 345. Removed ability to check-out defunct file
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	CR 342. Added sci_set_symbol_list
 * 	CR 331. Fixed src_ctl_show_log for blog
 * 	CR 335. Added checks for NONE and BIN to remove
 * 	CR 333. Fixed calls to copy_file
 * 	Replaced comparison with NULL to '\0'
 * 	CR 291. Fixed renumbered av array in create_leaderless_log
 * 	CR 328. Removed extraneous debug code
 * 	CR 329. Made more portable
 * 	CR 296. Added a forward decl. for src_ctl_config_remove
 * 	CR 119. Now handles multiple locks
 * 	CR 319. Added checks for *_relay
 * 	CR 177. Fixed ordering of MA_LEADER and MA_HELP
 * 	CR 321. Added sandbox locking
 * 	CR 299. Converted create_leaderless_log to odexm
 * 	CR 238. Adjusted for bco
 * 	CR 238. Fixed sci_create_files for bcreate
 * 	CR 238. Added diff functions
 * 	CR282: Made more portable to non-BSD systems.
 * 	CR 238. Removed some debugging statements.
 * 	CR 238. Cleaned up function declarations
 * 	CR 240. Added odexm initialization
 * 	DCE OT defect 2341.
 * 	CR 238. Fixed sci_show_log_list
 * 	CR 191 checklogstate and supporting functions are
 * 	       now only in libode.
 * 
 * Revision 1.3.2.4  1992/06/15  19:23:43  damon
 * 	Synched with DCE changes
 * 	[1992/06/15  19:21:03  damon]
 * 
 * 	Copied from ODE latest
 * 	Used to skip looking up latest revisions during resubmissions which
 * 	caused (null) to be used for a revision if a submission was
 * 	interrupted during the build update stage. This affected the SNAPSHOT
 * 	file and the *log files.
 * 
 * 	Now closes files when there is a problem with the logs
 * 	[1992/03/09  19:12:00  damon]
 * 
 * Revision 1.3  1991/12/17  21:01:18  devrcs
 * 	Ported to hp300_hpux
 * 	[1991/12/17  14:31:54  damon]
 * 
 * Revision 1.2  1991/12/05  21:12:59  devrcs
 * 	New file for ODE 2.1. Currently only supports bsubmit.
 * 	Will be split into more files later.
 * 
 * 	History extraction is now less picky about comment leaders as below
 * 
 * 	checklogstate now strips any trailing whitespace from the comment leader
 * 
 * 	Made /tmp into DEF_TMPDIR
 * 
 * 	Added checks for binary file (if leader is "BIN").  If
 * 	binary, do not do merge.
 * 
 * 	Added canonicalization to deal with /..//./ stuff in paths.
 * 
 * 	Now uses rlog -h instead of rlog -i to search for locks.
 * 	No longer tries to generate log messages for defuncted files.
 * 	Removed duplicate declaration of BCSBBASE.
 * 
 * 	Cleaned up error handling. Added log_error and is_in_error to
 * 	handle special cases where ERROR and OK can't be returned
 * 	directly.
 * 
 * $EndLog$
 */
/*
 * This module provides the following functions:
 *
 * sci_init
 * sci_lookup_leader_list
 * sci_all_list
 * sci_lookup_user_rev_list
 * sci_lookup_latest_rev_list
 * sci_ancestor_list
 * sci_locked_list
 * sci_is_branch
 * sci_check_in_list
 * sci_lock_list
 * sci_update_build_list
 * sci_outdate_list
 * sci_add_to_list
 * sci_trackfile
 */
#ifndef lint
static char sccsid[] = "@(#)20  1.1  src/bldenv/sbtools/libode/sci.c, bldprocess, bos412, GOLDA411a 1/19/94 17:42:29";
#endif /* not lint */

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: sci.c,v $ $Revision: 1.5.7.21 $ (OSF) $Date: 1993/12/02 22:01:44 $";
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utime.h>
#include <unistd.h>
#include <ode/copyrights.h>
#include <ode/errno.h>
#include <ode/history.h>
#include <ode/interface.h>
#include <ode/misc.h>
#include <ode/odedefs.h>
#include <ode/odexm.h>
#include <ode/project.h>
#include <ode/public/error.h>
#include <ode/public/odexm_client.h>
#include <ode/parse_rc_file.h>
#include <ode/run.h>
#include <ode/sandboxes.h>
#include <ode/sci.h>
#include <ode/sets.h>
#include <ode/src_ctl_rcs.h>
#include <ode/util.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>

#define TMPMODE         0777    /* mode for temporary working file */
#define LOGMODE         0644    /* mode for temporary message file */
#define LOGPROMPT       "<<<replace with log message"
#define LOGMSGSIZE      4096    /* -m<msg> buffer size */
/* comment syntax for C files */
#ifndef DEF_EDITOR
#    define DEF_EDITOR  "vi"    /* default editor */
#endif


#define ATOI(n, p) \
    (n) = 0; \
    if ('0' > *(p) || *(p) > '9') \
        return(1); \
    while ('0' <= *(p) && *(p) <= '9') { \
        (n) = (n) * 10 + (*(p) - '0'); \
        (p)++; \
    }

/*
 * GLOBAL VARIABLES:
 */
int sci_local = FALSE;

#define MAX_MONITORS 3
#define RCS_MONITOR 0
#define SRC_MONITOR 1
#define LOGS_MONITOR 2
OXMINIT oxminit [MAX_MONITORS];

OXM_CNXTN rcs_monitor;
OXM_CNXTN src_monitor;

extern int valid_copyrights;
extern char * copyright_name[];
extern char * raw_copyright_list[];
char *copyright_years;

STATIC
int file_mode = 0;

STATIC
int submitting = FALSE;

STATIC
const char * org_path;

char * src_path;

STATIC
const char * EDIT_PROG;

char temp1[MAXPATHLEN], temp2[MAXPATHLEN], temp3[MAXPATHLEN];
STATIC
char temp4[MAXPATHLEN], temp5[MAXPATHLEN];
STATIC
int temp_merge_setup = FALSE;

char mesgfile[MAXPATHLEN];      /* temporary file for log message */

STATIC
char *rcfile_source_host;
STATIC
char *rcfile_rcs_host;

char *BCSTEMP;                  /* R */
char *BCSSET_NAME;              /* R/W */
STATIC
char *USER;                     /* R */

STATIC
char bcsconfig[MAXPATHLEN];     /* bcs config file */
STATIC
char bcsset[MAXPATHLEN];        /* bcs set file */
char bcslog[MAXPATHLEN];        /* bcs log file */
STATIC
char bcspath[MAXPATHLEN];       /* bcs path file */
STATIC
char bcstempbuf[MAXPATHLEN];    /* buffer for generated BCSTEMP */
STATIC
char trackfile[MAXPATHLEN];     /* path and name of tracking file */

char working_file[MAXPATHLEN];
STATIC
char working_file_dir[MAXPATHLEN];
char working_file_tail[MAXPATHLEN];
char canon_working_file[MAXPATHLEN];
char temp_working_file[MAXPATHLEN];

STATIC
int usetrunk;                   /* using trunk directly */
STATIC
char setrev[MAXPATHLEN];        /* set revision */

/*
 * PROTOTYPES
 */

STATIC
src_ctl_set_remove( SCI_ELEM );

STATIC
void set_cleanup( void );

BOOLEAN
confirm_alloc ( SCI_LIST );

int
real_check_in_file ( SCI_ELEM , char * , char * ,
                     BOOLEAN );

void
sci_read_file ( char * , char * , char ** , BOOLEAN * , ERR_LOG * );

int
temp_func ( SCI_ELEM sci_ptr );

/*
 * FUNCTIONS/PROCEDURES
 */

int
get_src_and_org_paths ( char * sb_base, char * sb )

        /* This procedure determines the source directory for the user
           and then does a cd to it.  First, however, it finds out where
           the user is and sets up the current directory.  This is used
           for searching for paths for files to submit later. */

{
    char      * src_dir,                 /* points to string with source dir */
              * ptr_org,                              /* misc string pointer */
                tmpdir [ PATH_LEN ],                        /* misc string */
                tmpdir2 [ PATH_LEN ];                        /* misc string */

  concat ( tmpdir, sizeof (tmpdir), sb_base, "/", sb, "/src", NULL );
  src_dir = strdup ( tmpdir );
  if ( *src_dir != SLASH )
    uquit ( ERROR, FALSE,
       "\tvalue of %s field does not begin with a %c.\n", SOURCE_BASE, SLASH );

  if (( isdir ( src_dir ) == ERROR ))
    uquit ( ERROR, FALSE, "\tno source directory, %s, in sandbox.\n", src_dir );

  if ( getcwd ( tmpdir, sizeof(tmpdir) ) == NULL )
    uquit ( ERROR, FALSE, "\tgetcwd: %s.\n", strerror(errno) );
  chdir ( src_dir );

  if ( getcwd ( tmpdir2, sizeof(tmpdir2) ) == NULL )
    uquit ( ERROR, FALSE, "\tgetcwd: %s.\n", strerror(errno) );

  if (src_dir != NULL)
	free (src_dir);
  src_dir = strdup ( tmpdir2 );

  if ( strncmp ( src_dir, tmpdir, strlen ( src_dir )) != 0 ) {
    ui_print ( VFATAL, "current directory not in source base: %s.\n",
                       src_dir);
    return ( ERROR );
  } /* if */
  ptr_org = tmpdir + strlen ( src_dir );
                                                /* going to skip front part */
  if ( *ptr_org != SLASH ) {
    org_path = "./";
  } else {
    while ( *ptr_org == SLASH )
      ptr_org++;

    concat ( tmpdir2, sizeof(tmpdir2), "./", ptr_org, NULL );
    org_path = strdup ( tmpdir2 );
  } /* if */
  src_path = strdup ( src_dir );
  return ( OK );
} /* end get_src_path */

/*
 * Generate the full path and name for a given set
 */
int full_set_name ( char ** fsn, char * set_name )
{
  char        tmp_name1 [ PATH_LEN ];                        /* misc string */

/*
 * Not sure what this next statement does
  if ((( *set_name < 'A' ) || ( *set_name > 'Z' )) &&
      ( strncmp ( tmp_name, set_name, ptr - tmp_name )))
*/
/*
 * Not sure what this next statement does
  else
    strcpy ( set_file_name, set_name );
 */
  /* end if */

  concat ( tmp_name1, NAME_LEN, BCS_SET, set_name, NULL );
  *fsn = strdup ( tmp_name1 );
  if ( *fsn == NULL)
    return ( ERROR );
  else
    return ( OK );
  /* end if */
}

int
bcreate_undo( SCI_ELEM sci_ptr, char * set_name )
/*
 * routine to undo a file just created
 */
{
    int status;
    ERR_LOG log;

    /*
     * if the source control info does not exist, then abort
     */
    status = src_ctl_file_exists( sci_ptr -> name, &log );

    if (status == ERROR )
        return( ERROR );
    if (status == 1) {
        ui_print ( VFATAL, "[ source control information does not exist ]\n");
        return( ERROR );
    }

    /*
     * undo create operation
     */
    if (sci_ptr->ver_user != NULL)
    	log  = (ERR_LOG) src_ctl_undo_create( sci_ptr -> name, 
			sci_ptr -> ver_user, FALSE );

    if (log  != OE_OK)
        return( ERROR );

    /*
     * remove the branch symbol
     */
    if (sci_ptr->ver_user != NULL) {
    	log = (ERR_LOG) src_ctl_remove_symbol( sci_ptr -> name, BCSSET_NAME );
    	if (log  != OE_OK)
        	return( ERROR );
    }

    /*
     *  If the rcs file is empty, then remove it.
     */
    log = (ERR_LOG) src_ctl_remove_file( sci_ptr -> name );


    /*
     * update set file,
     * then delete working file from sandbox
     */
    (void) src_ctl_set_remove ( sci_ptr );
    if (unlink(working_file) == 0 )
        ui_print ( VNORMAL, "rm: removing %s\n", working_file);
    /* if */
    return( OK );
}

/*
 * START OF SCI_* PROCEDURES
 */

BOOLEAN sci_has_log ( SCI_ELEM sci_ptr )
{
  return ( strcmp ( sci_ptr -> leader, "NONE" ) != 0 &&
           strcmp ( sci_ptr -> leader, "BIN" ) != 0 );
} /* end sci_has_log */

ERR_LOG
sci_ancestor_update_list( SCI_LIST sl )
{
  SCI_ELEM sci_ptr;
  char * ancestry;
  ERR_LOG log;
  char new_ancestry[MAXPATHLEN];
  int len;
  int len2;

  if ( ! confirm_alloc ( sl ) )
    return ( err_log ( OE_ALLOC) );
  /* end if */
  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK ) {
    leave ();
    return ( log );
  } /* if */
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    len = strlen(sci_ptr -> ancestry );
    concat ( new_ancestry, sizeof(new_ancestry), sci_ptr -> ver_user, ">",
                       sci_ptr -> ver_merge, NULL );
    len2 = len + strlen (new_ancestry);
    ancestry = (char *)malloc(len2);
    concat ( ancestry, len2, new_ancestry,";", sci_ptr -> ancestry, NULL );
    free ( sci_ptr -> ancestry );
    sci_ptr -> ancestry = ancestry;
    src_ctl_add_ancestry ( sci_ptr -> name, new_ancestry );
  }
  if ( ( log = oxm_close ( rcs_monitor ) ) != OE_OK ) {
    leave ();
    return ( log );
  } /* if */
  return( log );
} /* sci_ancestor_update_list */

ERR_LOG
sci_ancestor_update_list2 ( SCI_LIST sl )
{
  SCI_ELEM sci_ptr;
  char * ancestry;
  ERR_LOG log;
  char new_ancestry[MAXPATHLEN];
  int len;
  int len2;

  if ( ! confirm_alloc ( sl ) )
    return ( err_log ( OE_ALLOC) );
  /* end if */
  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK ) {
    leave ();
    return ( log );
  } /* if */
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    src_ctl_get_ancestry ( sci_ptr -> name, &(sci_ptr -> ancestry) );
    len = strlen(sci_ptr -> ancestry );
    concat ( new_ancestry, sizeof(new_ancestry), sci_ptr -> ver_user, ">",
                       sci_ptr -> ver_config, NULL );
    len2= len + strlen ( new_ancestry );
    ancestry = (char *)malloc(len2);
    concat ( ancestry, len2, new_ancestry, ";", sci_ptr -> ancestry, NULL );
    free ( sci_ptr -> ancestry );
    sci_ptr -> ancestry = ancestry;
    src_ctl_add_ancestry ( sci_ptr -> name, new_ancestry );
  }
  if ( ( log = oxm_close ( rcs_monitor ) ) != OE_OK ) {
    leave ();
    return ( log );
  } /* if */
  return( log );
} /* sci_ancestor_update_list2 */

int
makedir( char *dir )
{
  char dbuf[MAXPATHLEN];
  char *ptr;

  ptr = concat(dbuf, sizeof(dbuf), dir, NULL);
  if (*(ptr-1) != '/')
    *ptr++ = '/';
  *ptr++ = '.';
  *ptr++ = '\0';
  return(makepath(dbuf, NULL, TRUE, TRUE));
} /* end makedir */

int
simple_cmp_func( char *str1, char *str2, int arg, int *skipped )
{
    int cmp;
    cmp = strcmp(str1, str2);
    if (cmp == 0)
        *skipped = TRUE;
    return(cmp);
} /* end simple_cmp_func */


/*
 * This is rather archaic, and should be re-done, but it
 * is okay for now.
 */
STATIC
int check_path( char *w )
{
  char dbuf[MAXPATHLEN], fbuf[MAXPATHLEN];
  char *ptr;

  /*
   * first we check the working file
   */
  enter ( "check_path" );
  path(w, dbuf, fbuf);
  (void) strcpy(working_file_dir, dbuf);
  if (*fbuf == '.' && *(fbuf+1) == '\0') {
    ui_print ( VFATAL, ".: invalid directory\n");
    return( ERROR );
  }
  if (*dbuf == '.' && *(dbuf+1) == '\0')
    *dbuf = '\0';
  else {
    if (*dbuf == '.' && *(dbuf+1) == '/')
        (void) strcpy(dbuf, dbuf+2);
    ptr = dbuf + strlen(dbuf);
    *ptr++ = '/';
    *ptr = '\0';
  }
  (void) strcpy(working_file, w);
  (void) strcpy(working_file_tail, fbuf);
  (void) concat(canon_working_file, sizeof(canon_working_file),
                "./", dbuf, fbuf, NULL);
  (void) concat(temp_working_file, sizeof(temp_working_file),
                BCSTEMP, "/", fbuf, NULL);
  leave ( );

  return( OK );
} /* check_path */


STATIC
int set_insert(void )
{
    char cwbuf[MAXPATHLEN];

    (void) concat(cwbuf, sizeof(cwbuf), canon_working_file, "\n", NULL);
    return(insert_line_in_sorted_file(bcsset, cwbuf,
           (int (*) (char *, ... ) )simple_cmp_func, 0));
} /* end set_insert */


STATIC
int remove_working_file( BOOLEAN local )
{
    int i;
    int status;
    const char *av[16];
    ERR_LOG log;

    if (local ) {
        (void) unlink(working_file);
        return( OK );
    }
    i = 0;
    av[i++] = "rm";
    av[i++] = "-f";
    av[i++] = canon_working_file;
    log = oxm_runcmd ( src_monitor, i, av, NULL );
    log = oxm_endcmd ( src_monitor, &status );
    if ( status != 0 )
        return( ERROR );
    return( OK );
} /* end remove_working_file */


STATIC
int copy_file ( char *src, char *dst,
                BOOLEAN local )
                /* local file copy? If FALSE, dst is on another machine */
{
    char tmp[MAXPATHLEN];
    int sfd;
    int tfd;
    struct utimbuf tv;
    struct stat st, statb;
  ERR_LOG log;

  ui_print ( VDEBUG, "Entering copy_file\n" );
  ui_print ( VDEBUG, "src :%s:\n", src );
  ui_print ( VDEBUG, "dst :%s:\n", dst );
  if (stat(src, &st) < 0) {
    ui_print ( VFATAL, "stat '%s' \n", src);
    return( ERROR );
  }
  if ( ! local ) {
    int status;
    int i;
    const char *av[16];
    i = 0;
    av[i++] = "-t1";
    av[i++] = "1";
    av[i++] = "odexm_cp";
    av[i++] = src;
    av[i++] = canon_working_file;
    log = oxm_runcmd ( src_monitor, i, av, NULL );
    log = oxm_endcmd ( src_monitor, &status );
    if (status != 0)
      return( ERROR );
    return( OK );
  }
  sfd = open(src, O_RDONLY, 0);
  if (sfd < 0) {
    ui_print ( VFATAL, "open %s\n", src);
    return( ERROR );
  }
  (void) concat(tmp, sizeof(tmp), dst, ".tmp", NULL);
  (void) unlink(tmp);
  tfd = open(tmp, O_WRONLY|O_CREAT|O_EXCL, 0600);
  if (tfd < 0 && strcmp( dst, working_file) == 0 ) {
    if (stat(working_file_dir, &statb) < 0) {
      if (makedir(working_file_dir) != 0)
        return( ERROR );
    } else if ((statb.st_mode&S_IFMT) != S_IFDIR) {
      ui_print ( VFATAL, "%s: not a directory\n", working_file_dir);
      return( ERROR );
    }
    tfd = open(tmp, O_WRONLY|O_CREAT|O_EXCL, 0600);
  }
  if (tfd < 0) {
    ui_print ( VFATAL, "open %s\n", tmp);
    (void) close(sfd);
    return( ERROR );
  }
  if (filecopy(sfd, tfd) < 0) {
        ui_print ( VFATAL, "filecopy %s to %s failed\n", src, tmp);
        (void) close(sfd);
        (void) close(tfd);
        (void) unlink(tmp);
        return( ERROR );
    }
    if (close(sfd) < 0) {
        ui_print ( VFATAL, "close %s\n", src);
        (void) close(tfd);
        (void) unlink(tmp);
        return( ERROR );
    }
    if (close(tfd) < 0) {
        ui_print ( VFATAL, "close %s\n", tmp);
        (void) close(tfd);
        (void) unlink(tmp);
        return( ERROR );
    }
    if (chmod(tmp, (int)st.st_mode&0777) < 0) {
        ui_print ( VFATAL, "chmod %s\n", tmp);
        (void) unlink(tmp);
        return( ERROR );
    }
    tv.actime = st.st_atime;
    tv.modtime = st.st_mtime;
    if (utime(tmp, &tv) < 0) {
        ui_print ( VFATAL, "utime %s\n", tmp);
        (void) unlink(tmp);
        return( ERROR );
    }
    if (rename(tmp, dst) < 0) {
        ui_print ( VFATAL, "2 rename %s to %s failed\n", tmp, dst);
        return( ERROR );
    }
  ui_print ( VDEBUG, "Leaving copy_file\n" );
    return( OK );
} /* end copy_file */

/*
 * This procedure either creates or updates the tracking file.
 * It makes sure a new entry is put in the file in the appropriate
 * place to remain sorted.
 */
STATIC
int track_insert ( char * file_name )
{
  char cwbuf[MAXPATHLEN];

  (void) concat(cwbuf, sizeof(cwbuf), file_name, "\n", NULL);
  return(insert_line_in_sorted_file(trackfile, cwbuf,
          (int (*) (char *, ... ) ) simple_cmp_func, 0));
}

/*
 * log_error and is_in_error are temporary place holders
 * for more comprehensive error handling routines.
 * They are only necessary for a few routines.
 */

int global_status = OK;

void log_error ( void )
{
  global_status = ERROR;
} /* end log_error */


BOOLEAN is_in_error ( void )
{
  return ( global_status == ERROR );
} /* end is_in_error */

STATIC
int sci_get_comment_leader( char * rcs_file, char *leader )
{
    char buf[MAXPATHLEN];
    char *ptr;
    char *lptr;
    int found;
    int status;
    int i;
    const char *av[16];
    ERR_LOG log;

    i = 0;

    av[i++] = "rlog";
    av[i++] = "-h";
    av[i++] = rcs_file;
    av[i] = NULL;
    if ( ( log = oxm_runcmd ( rcs_monitor, i, av, NULL ) ) != OE_OK )
      return ( ERROR );
    /* if */

    found = FALSE;
    while ( oxm_gets( rcs_monitor, buf, sizeof(buf), &log ) != NULL) {
        if (strncmp(buf, "comment leader:", 15) == 0) {
            found = TRUE;
            break;
        }
    }

    if (!found) {
        ui_print ( VFATAL, "Missing \"comment leader\" header in %s", rcs_file);
        return( ERROR );
    }
    if ((ptr = strchr(buf, '\n')) == NULL) {
        ui_print ( VFATAL, "Bad comment leader for %s", rcs_file);

        return( ERROR );
    }
    *ptr-- = '\0';
    if (*ptr != '"') {
        ui_print ( VFATAL, "missing '\"' after comment for %s", rcs_file);
        return( ERROR );
    }
    lptr = strchr(buf, *ptr);
    if (lptr++ == ptr) {
        ui_print ( VFATAL, "bad comment leader for %s", rcs_file);
        return( ERROR );
    }
    memcpy(leader, lptr, ptr - lptr);
    leader[ptr-lptr] = '\0';
    if ( strcmp ( leader, "BIN" ) != 0 && strcmp ( leader, "NONE" ) != 0 &&
         leader[strlen(leader)-1] != ' ' ) {
      ui_print ( VWARN, "Comment leader must end with space.\n" );
      ui_print ( VCONT, "File: %s.\n", rcs_file );
      ui_print ( VCONT, "Comment leader: '%s'.\n", leader );
      return ( ERROR );
    } /* if */
    ui_print ( VDEBUG, "In sci_get_comment_leader, leader is '%s'\n", leader );
    log = oxm_endcmd( rcs_monitor, &status );

    return( OK );
} /* end sci_get_comment_leader */

STATIC
void
src_ctl_setup_merge( void )
{
    ui_print ( VDEBUG, "Entering src_ctl_setup_merge\n" );
    (void) concat(temp1, sizeof(temp1), BCSTEMP, "/_BMERGE_1", NULL);
    (void) concat(temp2, sizeof(temp2), BCSTEMP, "/_BMERGE_2", NULL);
    (void) concat(temp3, sizeof(temp3), BCSTEMP, "/_BMERGE_3", NULL);
    (void) concat(temp4, sizeof(temp4), BCSTEMP, "/_BMERGE_4", NULL);
    (void) concat(temp5, sizeof(temp5), BCSTEMP, "/_BMERGE_5", NULL);
    temp_merge_setup = TRUE;
    ui_print ( VDEBUG, "Leaving src_ctl_setup_merge\n" );
} /* end src_ctl_setup_merge */


int
src_ctl_prep_merge( char *rev1, char *rev2, char *rev3,
                        SCI_ELEM sci_ptr)
                        /* expecting only an element, not an entire list */
{
  int called_getancestor;
  ERR_LOG log;

  ui_print ( VDEBUG, "Entering src_ctl_prep_merge\n" );

  /*
   * determine revision number for "common" revision
   */
  called_getancestor = FALSE;
  log  = (ERR_LOG) src_ctl_ancestor( sci_ptr -> name, sci_ptr -> ver_config,
                                     rev1, rev2, &rev3, &called_getancestor,
                                     sci_ptr -> ancestry );
  sci_ptr -> called_getancestor = called_getancestor;
  if ( log != OE_OK )
    return( (int) log );

  sci_ptr -> same23 = (strcmp(rev2, rev3) == 0);
  sci_ptr -> same13 = (strcmp(rev1, rev3) == 0);
  if ( sci_ptr -> same23) {
    ui_print ( VALWAYS, "base revision %s\n", rev2);
    ui_print ( VALWAYS, "common ancestor %s; will just use revision %s\n",
       rev3, rev1);
    ui_print ( VALWAYS, "No merge required\n", rev3 );
    sci_ptr -> ver_ancestor = strdup ( rev3 );
    if ( sci_ptr -> ver_ancestor == NULL ) {
      ui_print ( VFATAL, "strdup failed\n" );
      return ( ERROR );
    }
    return( OK );
  }
  if ( sci_ptr -> same13) {
    sci_ptr -> ver_ancestor = strdup ( rev3 );
    (void) unlink(temp1);
    return( OK );
  }

/*
 * Test for binary file, if isn't allow merges.
 */
  if ( sci_ptr -> leader == NULL || strcmp(sci_ptr -> leader, "BIN") != 0)  {
    sci_ptr -> need_merge = TRUE;
  }
  sci_ptr -> ver_ancestor = strdup ( rev3 );
  if ( sci_ptr -> ver_ancestor == NULL ) {
    ui_print ( VFATAL, "strdup failed\n" );
    return ( ERROR );
  }
  ui_print ( VDEBUG, "Leaving src_ctl_prep_merge\n" );
  return ( OK );
} /* src_ctl_prep_merge */

STATIC
int
src_ctl_config_lookup( SCI_ELEM sci_ptr, char * rev )
{
  char buffer[MAXPATHLEN];
  char cwbuf[MAXPATHLEN];
  FILE *inf;
  char *ptr, *p;
  int len;

  ui_print ( VDEBUG, "Entering src_ctl_config_lookup\n" );
  ptr = concat(cwbuf, sizeof(cwbuf), sci_ptr -> name , ",v\t", NULL);
  len = ptr - cwbuf;
  if ((inf = fopen(bcsconfig, "r")) == NULL) {
    ui_print ( VFATAL, "Can't open ancestor version file.\n" );
    ui_print ( VCONT, "This file is created/updated as files are checked out.\n"
               );
    ui_print ( VCONT, "File: '%s'\n", bcsconfig );
    return( ERROR );
  } /* end if */
  *rev = '\0';
  while (fgets(buffer, sizeof(buffer), inf) != NULL) {
    rm_newline ( buffer) ;
    if (strncmp(buffer, cwbuf, len) != 0)
      continue;
    ptr = buffer + len;
    while (*ptr == ' ' || *ptr == '\t')
      ptr++;
    if (*ptr < '0' || *ptr > '9') {
      if (strcmp(ptr, "defunct") == 0) {
        strcpy(rev, "defunct");
        sci_ptr -> defunct = TRUE;
        break;
      }
      ui_print ( VFATAL, "Invalid configuration line\n");
      (void) fputs(buffer, stderr);
      (void) fclose(inf);
      return( ERROR );
    } else if ( sci_ptr -> defunct) {
      ui_print ( VFATAL, "Contradiction between .BCSconfig file and\n" );
      ui_print ( VCONT, "information in source control system\n" );
      ui_print ( VCONT, "regarding defunct status.\n" );
      ui_print ( VCONT, "For file: '%s'\n", sci_ptr -> name );
      return ( ERROR );
    } /* end if */
    for (p = rev; (*p = *ptr) != '\0'; p++, ptr++)
        if (*p != '.' && (*p < '0' || *p > '9'))
            break;
    *p = '\0';
    break;
  }
  if (ferror(inf) || fclose(inf) == EOF) {
    ui_print ( VFATAL, "Error reading %s\n", bcsconfig);
    return( ERROR );
  } /* end if */
  ui_print ( VDEBUG, "Leaving src_ctl_config_lookup\n" );
  if ( *rev == '\0' ) {
    ui_print ( VFATAL, "No information for file in .BCSconfig file.\n" );
    ui_print ( VCONT, "File: '%s'\n", sci_ptr -> name );
    return ( ERROR );
  } else
    return ( OK );
  /* end if */
}

STATIC
const char *bmerge_action[] = {
    "abort",
#define MA_ABORT        0
    "ok",
#define MA_OK           1
    "edit",
#define MA_EDIT         2
    "merge",
#define MA_MERGE        3
    "co",
#define MA_CO           4
    "rco",
#define MA_RCO          5
    "diff",
#define MA_DIFF         6
    "rdiff",
#define MA_RDIFF        7
    "leader",
#define MA_LEADER	8
    "help",
#define MA_HELP         9
    NULL
};

STATIC
const char *bci_action[] = {
    "abort",
#define BA_ABORT        0
    "diff",
#define BA_DIFF         1
    "edit",
#define BA_EDIT         2
    "check-in",
#define BA_CHECKIN      3
    "leader",
#define BA_LEADER       4
    "xtract",
#define BA_XTRACT       5
    "log",
#define BA_LOG          6
    "logdiff",
#define BA_LOGDIFF      7
    "list",
#define BA_LIST         8
    "auto",
#define BA_AUTO         9
    "help",
#define BA_HELP         10
    NULL
};

STATIC
int
src_ctl_merge ( SCI_ELEM sci_ptr )
{
  int fd;
  int status;
  int pid;
  ERR_LOG log;
  HIST_ELEM merge_list;
  HIST_ELEM user_list;
  HIST_ELEM ancestor_list;
  HIST_ELEM final_list;

  ui_print ( VDEBUG, "Entering src_ctl_merge\n" );
  if ( sci_ptr -> same23 || ! sci_ptr -> same13) {
    (void) unlink(temp1);
    if ((fd = open(temp1, O_WRONLY|O_TRUNC|O_CREAT, 0600)) < 0) {
      ui_print ( VFATAL, "Unable to open %s for write\n", temp1);
      (void) unlink(temp1);
      return( ERROR );
    }
    ui_print ( VDETAIL, "Retrieving revision %s\n", sci_ptr -> ver_merge );
    status = src_ctl_check_out_with_fd( sci_ptr -> name, sci_ptr -> ver_merge,
                                        fd, sci_ptr -> leader, &log);
    (void) close(fd);
    if (status != 0) {
      ui_print ( VFATAL, "Check out failed.\n" );
      (void) unlink(temp1);
      return( ERROR );
    }
    if ( sci_ptr -> same23 ) {
      if (rename(temp1, temp_working_file) < 0) {
        ui_print ( VFATAL, "Unable to rename %s to %s\n", temp1, temp_working_file);
        (void) unlink(temp1);
        return( ERROR );
      }
      return( OK );
    }
  }
  (void) unlink(temp2);
  if ((fd = open(temp2, O_WRONLY|O_TRUNC|O_CREAT, 0600)) < 0) {
    ui_print ( VFATAL, "Unable to open %s for write", temp2);
    (void) unlink(temp1);
    (void) unlink(temp2);
    return( ERROR );
  }
  ui_print ( VDETAIL, "Retrieving revision %s\n", sci_ptr -> ver_user );
  status = src_ctl_check_out_with_fd( sci_ptr -> name, sci_ptr -> ver_user, fd,
                                      sci_ptr -> leader, &log);
  (void) close(fd);

  if (status !=  OK ) {
    ui_print ( VFATAL, "Check out failed.\n", sci_ptr -> ver_user );
    (void) unlink(temp1);
    (void) unlink(temp2);
    return( ERROR );
  }
  if ( sci_ptr -> same13) {
    (void) unlink(temp1);
    if (rename(temp2, temp_working_file) < 0) {
      ui_print ( VFATAL, "Unable to rename %s to %s\n", temp2, temp_working_file);
      (void) unlink(temp2);
      return( ERROR );
    }
    return( OK );
  }
  if ( sci_ptr -> called_getancestor ) {
    ui_print ( VALWAYS, "\n");
    ui_print ( VALWAYS, "*** WARNING -- calculated common ancestor ***\n");
    ui_print ( VALWAYS, "*** Check the merge differences carefully ***\n");
    ui_print ( VALWAYS, "\n");
  }

  (void) unlink(temp3);
  if ((fd = open(temp3, O_WRONLY|O_TRUNC|O_CREAT, 0600)) < 0) {
    ui_print ( VFATAL, "Unable to open %s for write\n", temp3);
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    return( ERROR );
  }
  ui_print ( VDETAIL, "Retrieving common ancestor %s\n",
             sci_ptr -> ver_ancestor );
  status = src_ctl_check_out_with_fd( sci_ptr -> name, sci_ptr -> ver_ancestor,
                                      fd, sci_ptr -> leader, &log);
  (void) close(fd);

  if ( status != OK ) {
    ui_print ( VFATAL, "Failed to check out %s\n", sci_ptr -> ver_ancestor );
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    return( ERROR );
  }

  ui_print ( VDETAIL, "Merging differences between %s and %s to %s\n",
             sci_ptr -> ver_merge, sci_ptr -> ver_user, temp_working_file);
  (void) unlink(temp4);
  if ((fd = open(temp4, O_WRONLY|O_TRUNC|O_CREAT, 0600)) < 0) {
    ui_print ( VFATAL, "Unable to open %s for write\n", temp4);
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    (void) unlink(temp4);
    return( ERROR );
  }
  merge_list = hst_xtract_file (temp1, sci_ptr->leader);
  user_list = hst_xtract_file (temp2, sci_ptr->leader);
  ancestor_list = hst_xtract_file (temp3, sci_ptr->leader);
  pid = fd_runcmd("diff", BCSTEMP, TRUE, -1, fd,
                  "diff", temp1, temp3, NULL);
  (void) close(fd);
  if (pid == -1) {
    ui_print ( VFATAL, "exec of diff failed.\n" );
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    (void) unlink(temp4);
    return( ERROR );
  }
  status = endcmd(pid);

  if (status != 0 && status != 1) {
    ui_print ( VFATAL, "Diff command failed with status %d\n", status);
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    (void) unlink(temp4);
    return( ERROR );
  }
  (void) unlink(temp5);
  if ((fd = open(temp5, O_WRONLY|O_TRUNC|O_CREAT, 0600)) < 0) {
    ui_print ( VFATAL, "Unable to open %s for write\n", temp5);
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    (void) unlink(temp4);
    (void) unlink(temp5);
    return( ERROR );
  }
  pid = fd_runcmd("diff", BCSTEMP, TRUE, -1, fd,
                  "diff", temp2, temp3, NULL);
  (void) close(fd);
  if (pid == -1) {
    ui_print ( VFATAL, "exec of diff failed.\n" );
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    (void) unlink(temp4);
    (void) unlink(temp5);
    return( ERROR );
  }
  status = endcmd(pid);
  if (status != 0 && status != 1) {
    ui_print ( VFATAL, "exec of diff failed.\n" );
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    (void) unlink(temp4);
    (void) unlink(temp5);
    return( ERROR );
  }




  fd = open(temp_working_file, O_WRONLY|O_TRUNC|O_CREAT, 0600);
  if (fd < 0) {
    ui_print ( VFATAL, "Unable to open %s for write\n", temp_working_file);
    (void) unlink(temp_working_file);
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    (void) unlink(temp4);
    (void) unlink(temp5);
    return( ERROR );
  }
  pid = fd_runcmd("rcsdiff3", BCSTEMP, TRUE, -1, fd,
                  "rcsdiff3", "-r", temp4, temp5, temp1, temp2, temp3,
		sci_ptr -> ver_merge, sci_ptr -> ver_user, NULL);
  if (pid == -1) {
    ui_print ( VFATAL, "exec of rcsdiff3 failed.\n" );
    (void) close(fd);
    (void) unlink(temp_working_file);
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    (void) unlink(temp4);
    (void) unlink(temp5);
    return( ERROR );
  }
  (void) close(fd);
  status = endcmd(pid);
  if (status == -1) {
    ui_print ( VFATAL, "rcsdiff3 failed\n");
    (void) unlink(temp_working_file);
    (void) unlink(temp1);
    (void) unlink(temp2);
    (void) unlink(temp3);
    (void) unlink(temp4);
    (void) unlink(temp5);
    return( ERROR );
  }
  if (status == 255) {
    ui_print ( VWARN, "Merge failed\n");
    (void) unlink(temp_working_file);
  } else if (status != 0)
    ui_print ( VALWAYS, "Warning: %d overlaps during merge.\n", status);
  if (status == 0)
    ui_print ( VALWAYS, "Merge successful.\n");

  final_list = hst_merge_lists (merge_list, user_list, ancestor_list);

  hst_insert_file (temp_working_file, final_list, sci_ptr -> leader);

  (void) unlink(temp1);
  (void) unlink(temp2);
  (void) unlink(temp3);
  (void) unlink(temp4);
  (void) unlink(temp5);

  ui_print ( VDEBUG, "Leaving src_ctl_merge\n" );
  return(status ? ERROR : 1);
} /* end src_ctl_merge */

STATIC
void
get_str( char *prompt, char *deflt, char *buf)
{
  (void) getstr(prompt, deflt, buf);
} /* get_str */

/*
 * FIXME
 * Temporary. This makes sure that the rcs_monitor connection
 * is closed during user input. This won't be necessary if the odexm
 * supports time-outs.
 */
BOOLEAN cnxtn_open = FALSE;

STATIC
int get_bmerge_action( SCI_ELEM sci_ptr, int initial_key, BOOLEAN no_log )
{
  const char *def = NULL;
  int key;
  struct stat st;
  int status;
  BOOLEAN improper = FALSE;
  char leader [ MAXPATHLEN ];
  char prompt[MAXPATHLEN];
  char deflt[MAXPATHLEN];
  ERR_LOG log;

  key = initial_key;
  for (;;) {
    switch (key) {
    case MA_ABORT:
      (void) unlink(temp_working_file);
      return ( ABORT );
    case MA_OK:
      if (stat(temp_working_file, &st) < 0) {
        def = "merge";
        break;
      }
      if ( sci_has_log ( sci_ptr ) )
        if ( ( log = checklogstate ( temp_working_file, sci_ptr -> leader,
                                       &improper )) != OE_OK) {
   	  ui_print ( VWARN, 
		    "Improper markers or history section in file %s\n",
		    sci_ptr->name);
          return ( ERROR );
        } else if ( improper ) {
          def = "edit";
          break;
        } /* if */
      /* if */
      return ( OK );
    case MA_EDIT:
            if (stat(temp_working_file, &st) < 0) {
              break;
            }
            ui_print ( VDEBUG, "%s %s\n", EDIT_PROG, temp_working_file );
            if ( cnxtn_open ) {
              log = oxm_close ( rcs_monitor );
              cnxtn_open = FALSE;
            } /* if */
            (void) runp(EDIT_PROG, EDIT_PROG, temp_working_file, NULL);
            def = "rdiff";
            break;
        case MA_MERGE:
            (void) unlink(temp_working_file);
            if ( !cnxtn_open ) {
              log = oxm_open ( &rcs_monitor, RCS_MONITOR );
              cnxtn_open = TRUE;
            } /* if */
            status = src_ctl_merge( sci_ptr );
            if (status == 0) {
                def = "ok";
                break;
            }
            if (stat(temp_working_file, &st) < 0)
              def = "merge";
            else if (status == 1)
              def = "rdiff";
            else
              def = "edit";
            break;
        case MA_RCO:
            (void) unlink(temp_working_file);
            if ( !cnxtn_open ) {
              log = oxm_open ( &rcs_monitor, RCS_MONITOR );
              cnxtn_open = TRUE;
            } /* if */
            if ( src_ctl_check_out( sci_ptr -> name, sci_ptr -> ver_user,
                                     sci_ptr -> leader, &log )
                 == 0) {
                if (stat(temp_working_file, &st) < 0) {
                  ui_print ( VFATAL, "stat %s", temp_working_file);
                  return ( ERROR );
                } else if (chmod(temp_working_file,
                               (int)(st.st_mode|S_IWRITE)&0777) < 0) {
                  ui_print ( VFATAL, "chmod %s", temp_working_file);
                  return ( ERROR );
                }
                def = "diff";
            }
            break;
        case MA_CO:
            (void) unlink(temp_working_file);
            if ( !cnxtn_open ) {
              log = oxm_open ( &rcs_monitor, RCS_MONITOR );
              cnxtn_open = TRUE;
            } /* if */
            if ( src_ctl_check_out( sci_ptr -> name, sci_ptr -> ver_merge,
                                    sci_ptr -> leader, &log )
                 == 0) {

                if (stat(temp_working_file, &st) < 0) {
                  ui_print ( VFATAL, "stat %s", temp_working_file);
                  return ( ERROR );
                } else if (chmod(temp_working_file,
                               (int)(st.st_mode|S_IWRITE)&0777) < 0) {
                  ui_print ( VFATAL, "chmod %s", temp_working_file);
                }
                def = "ok";
            }
            break;
        case MA_RDIFF:
          if (stat(temp_working_file, &st) < 0) {
            def = "merge";
            break;
          }
          if ( !cnxtn_open ) {
            log = oxm_open ( &rcs_monitor, RCS_MONITOR );
            cnxtn_open = TRUE;
          } /* if */
          if (src_ctl_diff_rev_with_file( sci_ptr -> ver_user,
                                          temp_working_file, sci_ptr -> name,
                                          -1, FALSE, FALSE, &log) != 0) {
            break;
          }
          def = "diff";
          break;
        case MA_DIFF:
          if (stat(temp_working_file, &st) < 0) {
            def = "merge";
            break;
          }
          if ( !cnxtn_open ) {
            log = oxm_open ( &rcs_monitor, RCS_MONITOR );
            cnxtn_open = TRUE;
          } /* if */
          if (src_ctl_diff_rev_with_file( sci_ptr -> ver_merge,
                                          temp_working_file, sci_ptr -> name,
                                          -1, FALSE, FALSE, &log) != 0) {
              break;
          }
          def = "ok";
          break;
        case MA_LEADER:
            strcpy ( leader, sci_ptr -> leader );
            (void) concat(prompt, sizeof(prompt),
                          "Comment leader for ", sci_ptr -> name, NULL);
            (void) strcpy(deflt, ( *leader == NUL) ? "NONE" : leader);
            for (;;) {
                get_str(prompt, deflt, leader);
                if (strcmp(leader, deflt) == 0)
                    break;
                (void) strcpy(deflt, leader);
            }
            if ( !cnxtn_open ) {
              log = oxm_open ( &rcs_monitor, RCS_MONITOR );
              cnxtn_open = TRUE;
            } /* if */
            sci_set_comment_leader ( sci_ptr, leader );
            break;
        case MA_HELP:
          ui_print ( VALWAYS, "One of the following:\n\n");
          ui_print ( VALWAYS, " abort - abort merge for %s\n", canon_working_file);
          ui_print ( VALWAYS, " ok    - done with merged %s; do next file\n",
                 canon_working_file);
          ui_print ( VALWAYS, " edit  - edit merged %s\n", canon_working_file);
          ui_print ( VALWAYS, " merge - merge source version of set '%s' into %s\n",
                 BCSSET_NAME, canon_working_file);
          ui_print ( VALWAYS, " co    - check-out %s from submit build without merging\n",
                 canon_working_file);
          ui_print ( VALWAYS, " rco   - check-out %s from user's private branch without merging\n",
                 canon_working_file);
          ui_print ( VALWAYS, " diff  - compare merged %s with submit build\n",
                 canon_working_file);
          ui_print ( VALWAYS, " rdiff - compare merged %s with user's private branch\n",
                 canon_working_file);
          ui_print ( VALWAYS, " leader - set the comment leader\n" );
          ui_print ( VALWAYS, "\n");

        }
/*
 * This is rather messy, but it works.
 * Test if file is binary, if it isn't, allow the merge.
 * If no real merge is required, key is set to MA_OK
 * and the MA_OK action is taken.
 */
        if ( (sci_ptr -> need_merge || improper) && (strcmp(sci_ptr -> leader, "BIN") != 0) ) {
          if ( cnxtn_open ) {
            log = oxm_close ( rcs_monitor );
            cnxtn_open = FALSE;
          } /* if */
          key = getstab("Abort, ok, edit, merge, rco, co,\nrdiff, diff, leader",
                         bmerge_action, def);
        } else
          key = MA_OK;
    } /* for */
} /* end get_bmerge_action */


STATIC
int
merge_elem ( SCI_ELEM sci_ptr, BOOLEAN no_log )
{
  int status = OK;
  int bmerge_status;

  ui_print ( VDEBUG, "Entering merge_elem\n" );
  check_path ( sci_ptr -> name );
  bmerge_status = get_bmerge_action ( sci_ptr, MA_MERGE, no_log );
  if ( bmerge_status != OK )
    status = bmerge_status;
  /* end if */
  ui_print ( VDEBUG, "Leaving merge_elem\n" );
  return ( status );
} /* end merge_elem */

STATIC
int
setup_bcstemp( void )
{
  struct stat st;
  int changes = FALSE;

  if (stat(BCSTEMP, &st) < 0 || (st.st_mode&S_IFMT) != S_IFDIR) {
    ui_print ( VDETAIL, "Creating %s\n", BCSTEMP);
    (void) unlink(BCSTEMP);
    if (makedir(BCSTEMP) != 0) {
      ui_print ( VFATAL, "Unable to create %s directory\n", BCSTEMP);
      return ( ERROR );
    } /* end if */
    if (stat(BCSTEMP, &st) < 0) {
      ui_print ( VFATAL, "Unable to stat %s directory\n", BCSTEMP);
      return ( ERROR );
    } /* end if */
    changes = TRUE;
  }
  if (changes) {
    if ( ui_ver_level() >= VDETAIL ) {
      ui_print ( VDEBUG, "ls -lgd %s\n", BCSTEMP );
      (void) runp("ls", "ls", "-lgd", BCSTEMP, NULL);
    } /* end if */
  } /* end if */
  return ( OK );
} /* end setup_bcstemp */


STATIC
int
src_ctl_set_remove( SCI_ELEM sci_ptr )
{
  char buf[MAXPATHLEN];
  char buffer[MAXPATHLEN];
  char cwbuf[MAXPATHLEN];
  FILE *inf, *outf;
  int wcnt;

  ui_print ( VDEBUG, "[ updating ./.BCSset-%s ]\n", BCSSET_NAME);
  (void) concat(buf, sizeof(buf), bcsset, ".tmp", NULL);
  (void) unlink(buf);
  if ((outf = fopen(buf, "w")) == NULL) {
    ui_print ( VFATAL, "Unable to open %s for write\n", buf);
    return( ERROR );
  }
  (void) concat(cwbuf, sizeof(cwbuf), sci_ptr -> name, "\n", NULL);
  wcnt = 0;
  if ((inf = fopen(bcsset, "r")) != NULL) {
    while (fgets(buffer, sizeof(buffer), inf) != NULL) {
      if (strcmp(buffer, cwbuf) == 0) {
        if ( ui_ver_level () >= VQUIET )
          fprintf(stderr, "< %s", cwbuf);
        continue;
      }
      (void) fputs(buffer, outf);
      wcnt++;
    }
    if (ferror(inf) || fclose(inf) == EOF) {
      ui_print ( VFATAL, "Error reading %s\n", bcsset);
      (void) fclose(outf);
      (void) unlink(buf);
      return( ERROR );
    }
  }
  if (ferror(outf) || fclose(outf) == EOF) {
    ui_print ( VFATAL, "Error writing %s\n", buf);
    (void) unlink(buf);
    return( ERROR );
  }
  if (rename(buf, bcsset) < 0) {
    ui_print ( VFATAL, "4 Rename %s to %s failed\n", buf, bcsset);
    return ( ERROR );
  }
  if (wcnt == 0)
    set_cleanup();
  return( OK );
}

STATIC
void set_cleanup( void )
{
  struct stat st;

  if (stat(bcsset, &st) == 0 && st.st_size > 0)
    return;
  if (unlink(bcsset) == 0 )
    ui_print ( VDETAIL, "rm: removing ./.BCSset-%s\n", BCSSET_NAME);
  if (unlink(bcslog) == 0 )
    ui_print ( VDETAIL, "rm: removing ./.BCSlog-%s\n", BCSSET_NAME);
  if (unlink(bcspath) == 0 )
    ui_print ( VDETAIL, "rm: removing ./.BCSpath-%s\n", BCSSET_NAME);
}

/*
 * END OF SUPPORTING FUNCTIONS
 */

char * check_out_config;
/*
 * FUNCTION set_source_info
 */
int
set_source_info ( struct rcfile * contents, char * usr_rcfile,
                  char * sb_base, char * sb, char ** def_set )
{
  char * rcs_relay;
  char * src_relay;
  char * logs_relay;
  char * submit_host;
  char buf [MAXPATHLEN];
  char buf2 [MAXPATHLEN];
  char * project_name;
  char * def_build;
  char * project;
  char * sub_project;
  struct rcfile sb_contents;
  char * backing_project;
  char * backing_build;
  BOOLEAN ode_sc;
  BOOLEAN ode_build_env;
  char * tcp_service_number = NULL;
  char copyright_file [MAXPATHLEN];
  char * check_copyrights;

/*
  if (parse_rc_file(sb_rcfile, &rcfile) != 0) {
*/
  prj_read ( sb_full_path ( sb_base, sb ), org_path, &project, &sub_project );
  sb_conf_read ( &sb_contents, sb_full_path ( sb_base, sb ), project,
                 sub_project );
  sb_conf_std ( &sb_contents, &backing_project, &backing_build, &ode_sc,
                &ode_build_env );
  if ( backing_build == NULL ) {
    if ( submitting ) {
      ui_print ( VWARN, "This build is not backed.\n" );
      ui_print ( VCONT, "You cannot submit files from here.\n" );
      return ( ERROR );
    } /* if */
    backing_build = sb_full_path ( sb_base, sb );
  } /* if */
  if ( sub_project == NULL ) {
    concat ( buf2, sizeof (buf2), backing_build, "/rc_files/", project, NULL );
  } else {
    concat ( buf2, sizeof (buf2), backing_build, "/rc_files/", project, NULL );
  } /* if */
  concat ( buf, sizeof (buf), buf2, "/sc.conf", NULL );
  if ( init_rc_contents ( contents, buf ) == ERROR ) {
    ui_print ( VFATAL, "Unable to parse sandbox");
    return ( ERROR ); /* unable to continue */
  } /* end if */
  if ( get_rc_value ( "check_copyrights", &check_copyrights, contents, FALSE )
                     == OK ) {
    if ( strcmp ( check_copyrights, "true" ) == 0 ) {
      concat ( copyright_file, sizeof(copyright_file), buf2, "/copyrights",
                               NULL );
      if ( ( ERR_LOG ) read_legal_copyrights2 ( copyright_file ) != OE_OK ) {
        return ( ERROR );
      } /* if */
    } /* if */
  } /* if */
  get_rc_value ( "rcs_host", &rcfile_rcs_host, contents, TRUE );
  get_rc_value ( "source_host", &rcfile_source_host, contents, TRUE );

  if ( get_rc_value ( "rcs_relay", &rcs_relay, contents, TRUE ) != OK )
    return ( ERROR );
  /* if */
#ifdef notdef
  if ((stat( rcs_relay, &st_buf) < 0))
  {
      ui_print ( VFATAL, "Unable to access relay program (%s).\n", rcs_relay);
      return ( ERROR ); /* unable to continue */
  } /* if */ 
#endif
  if ( get_rc_value ( "src_relay", &src_relay, contents, TRUE ) != OK )
    return ( ERROR );
  /* if */
#ifdef notdef
  if ((stat( src_relay, &st_buf) < 0))
  {
      ui_print ( VFATAL, "Unable to access relay program (%s).\n", src_relay);
      return ( ERROR ); /* unable to continue */
  } /* if */ 
#endif
  if ( get_rc_value ( "logs_relay", &logs_relay, contents, TRUE ) != OK )
    return ( ERROR );
  /* if */
#ifdef notdef
  if ((stat( logs_relay, &st_buf) < 0))
  {
      ui_print ( VFATAL, "Unable to access relay program (%s).\n", logs_relay);
      return ( ERROR ); /* unable to continue */
  } /* if */ 
#endif
  get_rc_value ( "submit_host", &submit_host, contents, TRUE );
  get_rc_value ( "default_build", &def_build, contents, TRUE );
  get_rc_value ( "project_name", &project_name, contents, TRUE );
  get_rc_value ( "check_out_config", &check_out_config, contents, TRUE );
  get_rc_value ( "default_set", def_set, contents, TRUE );
  get_rc_value ( "tcp_service_number", &tcp_service_number, contents, FALSE );
  oxminit [RCS_MONITOR].monitor = RCS_MONITOR;
  oxminit [RCS_MONITOR].host = strdup ( rcfile_rcs_host ); 
  oxminit [RCS_MONITOR].relay = strdup ( rcs_relay );
  if ( sub_project == NULL ) {
    concat ( buf, sizeof ( buf ), "rcs/", project_name, "/", def_build, NULL );
  } else {
    concat ( buf, sizeof ( buf ), "rcs/", project_name, "/", sub_project, "/",
                  def_build, NULL );
  } /* if */
  oxminit [RCS_MONITOR].ident = strdup ( buf );
  oxminit [RCS_MONITOR].port = tcp_service_number;
  oxminit [SRC_MONITOR].monitor = SRC_MONITOR;
  oxminit [SRC_MONITOR].host = strdup ( rcfile_source_host ); 
  oxminit [SRC_MONITOR].relay = strdup ( src_relay );
  concat ( buf, sizeof ( buf ), "src/", project_name, "/", def_build, NULL );
  oxminit [SRC_MONITOR].ident = strdup ( buf );
  oxminit [SRC_MONITOR].port = tcp_service_number;
  oxminit [LOGS_MONITOR].monitor = LOGS_MONITOR;
  oxminit [LOGS_MONITOR].host = strdup ( submit_host ); 
  oxminit [LOGS_MONITOR].relay = strdup ( logs_relay );
  concat ( buf, sizeof ( buf ), "logs/", project_name, "/", def_build, NULL );
  oxminit [LOGS_MONITOR].ident = strdup ( buf );
  oxminit [LOGS_MONITOR].port = tcp_service_number;
  return ( OK );
} /* end set_source_info */

int getenv_user ( char ** user )

        /* This procedure gets the user's name. */

{
    char      * env_ptr;                              /* point to env string */

  if (( env_ptr = getenv ( "USER" )) == NULL ) {          /* insert user name */
    ui_print ( VFATAL, "USER not found in environment.\n" );
    return (ERROR );
  }

  *user = strdup ( env_ptr );
  return ( OK );
}

void
get_full_setname ( char * setn, char * user, char ** symbolic_name )

        /* This procedure prepends the user name to the setname
           if the setname does not start with a capital letter
           and it isn't already there.  It puts the final setname
           in setinfo. */

{
    char        tmp_name [ NAME_LEN ],                        /* misc string */
                tmp_name2 [ NAME_LEN ],                        /* misc string */
              * ptr;                                  /* point to env string */

  if (( ptr = concat ( tmp_name, NAME_LEN , user, "_", NULL ))
            == NULL)
    uquit ( ERROR, FALSE, "\tno room in buffer for '%s_'\n", user );

  if ((( *setn < 'A' ) || ( *setn > 'Z' )) &&
      ( strncmp ( tmp_name, setn, ptr - tmp_name ))) {
    concat ( tmp_name2, NAME_LEN, tmp_name, setn, NULL );
    *symbolic_name = strdup ( tmp_name2 );
  } else {
    *symbolic_name = strdup ( setn );
  } /* if */
}

int
sci_init3 ( const char * submit_set, struct rcfile * contents )
{
  char config_file[MAXPATHLEN];

  concat(config_file, sizeof(config_file), "ode2.3_server_base/sets/",
                      submit_set, "/sc.conf", NULL );
  if ( init_rc_contents ( contents, config_file ) == ERROR ) {
    ui_print ( VFATAL, "Unable to parse sandbox");
    return ( ERROR ); /* unable to continue */
  } /* end if */
  return ( OK );
} /* end sci_init3 */

int
sci_init2 ( char * submit_set )
{
  char * src_relay;
  char * logs_relay;
  char * submit_host;
  char buf [MAXPATHLEN];
  char * project_name;
  char * def_build;
  char * def_set;
  struct rcfile contents;
  ERR_LOG log;
  char * tcp_service_number = NULL;

  BCSTEMP = strdup ( "/tmp" );
  sci_init3 ( submit_set, &contents );
  log = (ERR_LOG) read_legal_copyrights ( &contents );
  get_rc_value ( "rcs_host", &rcfile_rcs_host, &contents, TRUE );
  get_rc_value ( "source_host", &rcfile_source_host, &contents, TRUE );

  if ( get_rc_value ( "src_relay", &src_relay, &contents, TRUE ) != OK )
    return ( ERROR );
  /* if */
#ifdef notdef
  if ((stat( src_relay, &st_buf) < 0))
  {
      ui_print ( VFATAL, "Unable to access relay program (%s).\n", src_relay);
      return ( ERROR ); /* unable to continue */
  } /* if */ 
#endif
  if ( get_rc_value ( "logs_relay", &logs_relay, &contents, TRUE ) != OK )
    return ( ERROR );
  /* if */
#ifdef notdef
  if ((stat( logs_relay, &st_buf) < 0))
  {
      ui_print ( VFATAL, "Unable to access relay program (%s).\n", logs_relay);
      return ( ERROR ); /* unable to continue */
  } /* if */ 
#endif
  get_rc_value ( "submit_host", &submit_host, &contents, TRUE );
  get_rc_value ( "default_build", &def_build, &contents, TRUE );
  get_rc_value ( "project_name", &project_name, &contents, TRUE );
  get_rc_value ( "check_out_config", &check_out_config, &contents, TRUE );
  get_rc_value ( "default_set", &def_set, &contents, TRUE );
  get_rc_value ( "tcp_service_number", &tcp_service_number, &contents, FALSE );
  oxminit [SRC_MONITOR].monitor = SRC_MONITOR;
  oxminit [SRC_MONITOR].host = strdup ( rcfile_source_host ); 
  oxminit [SRC_MONITOR].relay = strdup ( src_relay );
  concat ( buf, sizeof ( buf ), "src/", project_name, "/", def_build, NULL );
  oxminit [SRC_MONITOR].ident = strdup ( buf );
  oxminit [SRC_MONITOR].port = tcp_service_number;
  oxminit [LOGS_MONITOR].monitor = LOGS_MONITOR;
  oxminit [LOGS_MONITOR].host = strdup ( submit_host ); 
  oxminit [LOGS_MONITOR].relay = strdup ( logs_relay );
  concat ( buf, sizeof ( buf ), "logs/", project_name, "/", def_build, NULL );
  oxminit [LOGS_MONITOR].ident = strdup ( buf );
  oxminit [LOGS_MONITOR].port = tcp_service_number;
  oxm_init ( MAX_MONITORS, oxminit );
  return ( OK );
} /* end sci_init2 */

/*
 * FUNCTION sci_init
 *
 * This function needs to be called at the beginning of
 * a source control sesion, before any other sci_* calls are
 * made, to get setup information such as which server to use, etc.
 */
int
sci_init ( 
struct rcfile * contents,
char ** sb,
char ** sb_base,
char ** set,
char ** submit_set,
char ** sbrc_file,
int f,
BOOLEAN s )
{
  char buf[MAXPATHLEN];
  int status;
  char * symbolic_name = NULL;
  char * setdir = NULL;
  char def_build_rc_file [ MAXPATHLEN ];
  ERR_LOG log;
  char * rc_file = NULL;

  ui_print ( VDEBUG, "Entering sci_init\n" );
  file_mode = f;
  submitting = s;
  getenv_user ( &USER );
  if ( *set != NULL ) {
    get_full_setname ( *set, USER, &symbolic_name );
  } /* if */
  if ( current_sb ( sb, sb_base, sbrc_file, &rc_file ) == ERROR ) {
    ui_print ( VFATAL, "Could not establish sandbox environment.\n" );
    exit ( ERROR );
  } /* if */
  if ( current_set ( &symbolic_name, &setdir, sb, &rc_file ) == ERROR ) {
     if ( symbolic_name == NULL )
      uquit ( ERROR, FALSE, "\tsb, %s, has no default set.\n", *sb );
    else
      uquit ( ERROR, FALSE, "\tthe set, %s, is not part of the sandbox, %s.\n",
                               symbolic_name, *sb );
  } /* if */
#ifdef notdef
  if ( default_build ( &build_dir, &def_build, &def_set, *sbrc_file )
                      == ERROR ) {
    ui_print ( VFATAL, "Could not find project directory or default build.\n" );
    return ( ERROR );
  } /* if */
  concat ( def_build_rc_file, sizeof ( def_build_rc_file ), build_dir, "/",
                              def_build, "/", SANDBOXRC, NULL );
#endif
  if (get_src_and_org_paths ( *sb_base, *sb ) == ERROR)
	exit ( ERROR );

  if ( set_source_info ( contents, def_build_rc_file, *sb_base, *sb,
       submit_set ) != OK ) {
    return ( ERROR );
  } /* if */

  *set = symbolic_name;

  oxm_init ( MAX_MONITORS, oxminit );

  atomic_init ();

  status = OK;
  if (getuid() != geteuid() || getgid() != getegid())  {
    ui_print ( VFATAL, "Branch rcs commands should not run setuid\n" );
    status = ERROR;
  } /* if */
  if ((EDIT_PROG = getenv(EDITOR)) == NULL)
    EDIT_PROG = DEF_EDITOR;
  /* if */

  /*
   * BCSTEMP - temporary directory for bcs commands
   */
	    (void) concat(bcstempbuf, sizeof(bcstempbuf),
			  *sb_base, "/", *sb, "/tmp", NULL);
	    BCSTEMP = bcstempbuf;
    status = setup_bcstemp ( );
    if ( status != OK )
      return ( status );
    /* end if */

    (void) concat(mesgfile, sizeof(mesgfile), BCSTEMP, "/_LOG_", NULL);

    /*
     * BCSSET_NAME - set name to use for bcs commands
     */
    BCSSET_NAME = *set;
    if (strcmp(BCSSET_NAME, "TRUNK") == 0) {
	usetrunk = TRUE;
	(void) strcpy(setrev, "-r");
    } else {
	if (BCSSET_NAME[0] >= '0' && BCSSET_NAME[0] <= '9') {
	    usetrunk = TRUE;
	} else {
	    usetrunk = FALSE;
	    if (strncmp(BCSSET_NAME, USER, strlen(USER)) != 0 &&
		(BCSSET_NAME[0] < 'A' || BCSSET_NAME[0] > 'Z')) {
		(void) concat(setrev, sizeof(setrev),
			      USER, "_", BCSSET_NAME, NULL);
	  (void) strcpy(buf, setrev);
	  BCSSET_NAME = buf;
        }
      }
      (void) concat(setrev, sizeof(setrev), "-r", BCSSET_NAME, NULL);
    }
    if (setenv(BCSSET, BCSSET_NAME, TRUE) < 0) {
      ui_print ( VFATAL, "BCSSET setenv failed");
      return ( ERROR );
    }
    BCSSET_NAME = getenv(BCSSET);
    if (BCSSET_NAME == NULL) {
      ui_print ( VFATAL, "BCSSET not defined");
    return ( ERROR );
  }

  /*
   * generate set dependent information
   */
  (void) concat(bcsconfig, sizeof(bcsconfig),
		  src_path, "/.BCSconfig", NULL);
  (void) concat(bcsset, sizeof(bcsset),
		  src_path, "/.BCSset-", BCSSET_NAME, NULL);
  (void) concat(bcslog, sizeof(bcslog),
		  src_path, "/.BCSlog-", BCSSET_NAME, NULL);
  (void) concat(bcspath, sizeof(bcspath),
	  src_path, "/.BCSpath-", BCSSET_NAME, NULL);

  if ( chdir ( src_path ) < 0 ) {
    ui_print ( VFATAL, "Could not cd to source path: '%s'\n", src_path );
    return ( ERROR );
  } /* if */

  if ( (log = (ERR_LOG) read_legal_copyrights ( contents ) ) != OE_OK ) {
    ui_print ( VFATAL, "%s\n", err_str ( log ) );
    status = ERROR;
  } /* if */
  get_rc_value ("copyright_years", &copyright_years, contents, FALSE);
  ui_print ( VDEBUG, "Leaving sci_init\n" );
  return ( status );
} /* end sci_init */


/*
 * The following variables are utilized by sci_new_list and confirm_alloc.
 */

#define MAX_SCI_LIST 100

STATIC
int serial_num = 0;

STATIC
SCI_LIST serial_list [ MAX_SCI_LIST ];

/*
 * FUNCTION confirm_alloc
 *
 * Confirm that the given object was actually allocated.
 */
BOOLEAN
confirm_alloc ( SCI_LIST sl )
{
  if ( sl != NULL && sl -> serial_num >= 0 &&
       sl -> serial_num <= ( MAX_SCI_LIST - 1) &&
       serial_list [ sl -> serial_num ] == sl )
    return ( TRUE );
  else {
    ui_print ( VALWAYS, "\n*** INTERNAL ERROR 0001 ***\n\n" );
    ui_print ( VFATAL, "Unallocated object detected.\n" );
    ui_print ( VCONT, "Report failure immediately!!\n" );
    report_current_function ( );
    ui_print ( VALWAYS, "\n" );
    log_error ( );
    return ( FALSE );
  } /* end if */
} /* end confirm_alloc */


/*
 * FUNCTION sci_new_list
 *
 * Create a new list. Must be called before any calls to sci_add_to_list
 * are made.
 */
int
sci_new_list ( SCI_LIST * sl )
{
  BOOLEAN status;

  ui_print ( VDEBUG, "Entering sci_new_list\n" );
  *sl = ( SCI_LIST ) malloc ( (size_t) sizeof ( struct sci_list ) );
  if ( *sl == NULL) {
    ui_print ( VFATAL, "Unable to allocate %d bytes of memory!\n" );
    ui_print ( VFATAL, "Function: sci_new_list.\n" );
    status = ERROR;
  } else {
    if ( serial_num == MAX_SCI_LIST ) {
      ui_print ( VFATAL, "Exceeded limit set on number of\n" );
      ui_print ( VCONT, "SCI_LIST objects that may be allocated.\n" );
      ui_print ( VCONT, "Limit currently set at: %d.\n", MAX_SCI_LIST );
      status = ERROR;
    } else {
      serial_list [ serial_num ] = *sl;
      ( *sl ) -> serial_num = serial_num++;
      ( *sl ) -> head = NULL;
      ( *sl ) -> tail = NULL;
      ( *sl ) -> elem_cnt = 0;
      status = OK;
    } /* end if */
  } /* end if */
  ui_print ( VDEBUG, "Leaving sci_new_list\n" );
  return ( status );
} /* end sci_new_list */


/*
 * FUNCTION sci_add_to_list_as_is
 */
int sci_add_to_list_as_is ( SCI_LIST sl, char * file_name )
{
  SCI_ELEM sci_ptr;

#ifndef NO_DEBUG
  enter ( "sci_add_to_list_as_is" );
#endif
  if ( ! confirm_alloc ( sl ) ) {
#ifndef NO_DEBUG
    leave ( );
#endif
    return ( ERROR );
  } /* end if */
 /*
  * The F_OK access is to see if the file is present.
  * If it is not present then it should not be added to the list
  * if -changed or -saved is set.
  */

  if ( file_mode == 0 || ( access ( file_name, F_OK ) == 0 &&
       ( ( file_mode == 1 && access ( file_name, W_OK) == 0 ) ||
         ( file_mode == 2 && access ( file_name, W_OK ) < 0 ))))
    ui_print ( VDEBUG, "Adding %s to a list.\n", file_name );
  else {
    ui_print ( VDEBUG, "Skipping file %s.\n", file_name );
    leave ( );
    return ( OK );
  } /* if */
  sci_ptr = ( SCI_ELEM ) malloc ( (size_t) sizeof ( struct sci_elem ) );
  if ( sci_ptr == NULL ) {
    ui_print ( VFATAL, "alloc of sci_ptr failed.\n" );
    leave ( );
    return ( ERROR );
  } /* end if */
  sci_ptr -> name = strdup ( file_name );
  sci_ptr -> ver_user = NULL;
  sci_ptr -> ver_ancestor = NULL;
  sci_ptr -> ver_config = NULL;
  sci_ptr -> ver_merge = NULL;
  sci_ptr -> ver_latest = NULL;
  sci_ptr -> leader = NULL;
  sci_ptr -> skip = FALSE;
  sci_ptr -> locked = FALSE;
  sci_ptr -> need_merge = FALSE;
  sci_ptr -> merged_up = FALSE;
  sci_ptr -> called_getancestor = FALSE;
  sci_ptr -> defunct = FALSE;
  sci_ptr -> status = OK;
  sci_ptr -> same13 = FALSE;
  sci_ptr -> same23 = FALSE;
  sci_ptr -> has_user_branch = FALSE;
  sci_ptr -> has_merge_branch = FALSE;
  sci_ptr -> next = NULL;
  if ( sl -> tail == NULL )
    sl -> head = sci_ptr;
  else
    ( sl -> tail ) -> next = sci_ptr;
  /* end if */
  sl -> tail = sci_ptr;
  sl -> elem_cnt += 1;
#ifndef NO_DEBUG
  leave ( );
#endif
  return ( OK );
} /* end sci_add_to_list_as_is */

/*
 * FUNCTION sci_add_to_list
 */
int sci_add_to_list ( SCI_LIST sl, char * file_name )
{
  SCI_ELEM sci_ptr;
  char buf [MAXPATHLEN];
  char canonical_name [MAXPATHLEN];

#ifndef NO_DEBUG
  enter ( "sci_add_to_list" );
#endif
  if ( ! confirm_alloc ( sl ) ) {
#ifndef NO_DEBUG
    leave ( );
#endif
    return ( ERROR );
  } /* end if */
  ui_print ( VDEBUG, "Adding %s to a list.\n", file_name );
  sci_ptr = ( SCI_ELEM ) malloc ( (size_t) sizeof ( struct sci_elem ) );
  if ( sci_ptr == NULL ) {
    ui_print ( VFATAL, "alloc of sci_ptr failed.\n" );
    leave ( );
    return ( ERROR );
  } /* end if */
  if (( *file_name == SLASH ) ||               /* absolute path; ./ beginning */
      (( *file_name == PERIOD ) && (*( file_name + 1 ) == SLASH )) ||
      ( *org_path == NUL )) {                   /* not below src */
    strcpy ( buf, file_name );
  } else {                       /* relative path so use ./org_path/field */
    concat ( buf, sizeof ( buf ), org_path, "/", file_name,
             NULL );
  } /* else */
/*
 * This is messy, but it works for now.
 */
  if ( canonicalize ( ".", buf, canonical_name, sizeof (canonical_name) )
       != OK ) {
    leave ( );
    return ( ERROR );
  } /* end if */
  sci_add_to_list_as_is ( sl, canonical_name );
  leave ( );
  return ( OK );
} /* end sci_add_to_list */


/*
 * FUNCTION sci_first
 *
 * Give the first element in a given list
 */
SCI_ELEM
sci_first ( SCI_LIST sl )
{
  if ( ! confirm_alloc ( sl ) ) {
    log_error ( );
    return ( NULL );
  } /* end if */
  return ( sl -> head );
} /* end sci_first */


/*
 * FUNCTION sci_next
 *
 * Give the next element in a list from the element given.
 */
SCI_ELEM
sci_next ( SCI_ELEM se )
{
  return ( se -> next );
} /* end sci_next */

int
sci_elem_cnt ( SCI_LIST sl )
{
  return ( sl -> elem_cnt );
} /* end sci_elem_cnt */

/*
 * FUNCTION sci_lookup_leader_list
 */
int sci_lookup_leader_list ( SCI_LIST sl )
{
  SCI_ELEM sci_ptr;
  char leader [MAXPATHLEN];
  ERR_LOG log;

  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* end if */

  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
    return ( ERROR );
  /* if */

  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_get_comment_leader( sci_ptr -> name, leader ) == ERROR )
      return( ERROR );
    /* end if */
    sci_ptr -> leader = strdup ( leader );
  } /* end for */
  log = oxm_close ( rcs_monitor );

  return( OK );
} /* sci_lookup_leader_list */

/*
 * FUNCTION sci_set_cmt_leader_list
 */
int sci_set_cmt_leader_list ( SCI_LIST sl, char * leader )
{
  SCI_ELEM sci_ptr;
  ERR_LOG log;

  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* end if */

  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
    return ( ERROR );
  /* if */

  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_set_comment_leader ( sci_ptr , leader ) == ERROR )
      return( ERROR );
    /* end if */
    sci_ptr -> leader = strdup ( leader );
  } /* end for */
  log = oxm_close ( rcs_monitor );
  return( OK );
} /* sci_set_cmt_leader_list */


/*
 * In the old bsubmit, the code equivalent to sci_init was invoked multiple
 * times. Once for each invokation of bci, bco, etc. Thus, bcsset
 * was set multiple times. The following routine is a temporary
 * kludge to accomplish the same thing for the two variables that require
 * being set a second time.
 */

void
set_and_log_kludge ( char * set_name )
{
  (void) concat(bcsset, sizeof(bcsset),
		  src_path, "/.BCSset-", set_name, NULL);
}


/*
 * FUNCTION sci_all_list
 *
 * Create a list of all files in a given set. The new list will
 * be allocated. A new list does not need to be passed in, and
 * will in fact be overwritten.
 */
int sci_all_list ( SCI_LIST * sl, char *set_name )
{
  FILE      * fp;
  char        filebuf [ PATH_LEN ],                         /* misc string */
              tmp_buf [ PATH_LEN ],                         /* misc string */
            * ptr_buff,                                /* misc ptr to char */
            * set_file_name;                           /* name of set file */

  ui_print ( VDEBUG, "In sci_all_list\n" );
  sci_new_list ( sl );
  if ( full_set_name ( &set_file_name, set_name ) == ERROR ) {
    ui_print ( VDEBUG, "Leaving sci_all_list\n" );
    return ( ERROR );
  }
  if (( fp = fopen ( set_file_name, READ )) == NULL ) {
    ui_print ( VWARN, "No files in set.\n" );
    ui_print ( VCONT, "Set: %s\n", set_name );
    ui_print ( VCONT, "Set file: %s\n", set_file_name );
    return ( ERROR );
  }
  while ( fgets ( filebuf, PATH_LEN, fp ) != NULL ) {
    rm_newline ( filebuf );
    strcpy ( tmp_buf, filebuf );
    if (( ptr_buff = strdup ( tmp_buf )) == NULL ) {
      ui_print ( VFATAL, "No room for strdup of: %s\n", tmp_buf );
      ui_print ( VDEBUG, "Leaving sci_all_list\n" );
      return ( ERROR );
    }
    if ( sci_add_to_list_as_is ( *sl, ptr_buff ) != OK ) {
      return ( ERROR );
    } /* end if */
  } /* while */
  fclose ( fp );
  ui_print ( VDEBUG, "Leaving sci_all_list\n" );
  return ( OK );
}


/*
 * FUNCTION sci_lookup_user_rev_list
 *
 * Provide user revisions for each file in a given list and a given set
 */
int sci_lookup_user_rev_list ( SCI_LIST sl, char * set_name,
                               int * missing_revs )
{
  int status;
  char buf [PATH_LEN];
  char * buf_ptr;
  char * ver_user;
  char * def_str;
  SCI_ELEM sci_ptr;
  ERR_LOG log;

  ui_print ( VDEBUG, "Entering sci_lookup_user_rev_list.\n" );
  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* end if */
  confirm_alloc ( sl );
  status = OK;
  * missing_revs = FALSE;
  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
    return ( ERROR );
  /* if */

  for (sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
#ifdef notdef
    if ( sci_ptr -> skip )
      continue;
    /* end if */
#endif
    src_ctl_lookup_revision ( sci_ptr -> name, set_name, buf, &log );
    if ( log  != OE_OK ) {
      status = ERROR;
      continue;
    }
    if ( buf == NULL || *buf == '\0' ) {
      sci_ptr -> ver_user = NULL;
      * missing_revs = TRUE;
    } else {
      buf_ptr = buf;
      ver_user = nxtarg ( &buf_ptr, WHITESPACE );
      def_str = nxtarg ( &buf_ptr, WHITESPACE );
      if ( strcmp ( def_str, "(defunct)" ) == 0 ) {
        ui_print ( VDEBUG, "version is defunct\n" );
        sci_ptr -> defunct = TRUE;
      } else if ( sci_ptr -> defunct ) {
        ui_print ( VFATAL, "Contradiction between .BCSconfig file and\n" );
        ui_print ( VCONT, "information in source control system\n" );
        ui_print ( VCONT, "regarding defunct status.\n" );
        ui_print ( VCONT, "For file: '%s'\n", sci_ptr -> name );
        status = ERROR;
      }
      sci_ptr -> ver_user = strdup ( ver_user );
      if ( sci_ptr -> ver_user == NULL ) {
        ui_print ( VFATAL, "strdup of ver_user failed\n" );
        status = ERROR;
        break;
      } /* end if */
    } /* end if */
  } /* end for */
  log = oxm_close ( rcs_monitor );
  ui_print ( VDEBUG, "Leaving sci_lookup_user_rev_list.\n" );
  return ( status );
} /* sci_lookup_user_rev_list */


/*
 * FUNCTION sci_lookup_latest_rev_list
 *
 * Provide user revisions for each file in a given list and a given set
 */
int sci_lookup_latest_rev_list ( SCI_LIST sl, char * set_name,
                                 int * missing_revs)
{
  int status = OK;
  char buf [PATH_LEN];
  SCI_ELEM sci_ptr;
  ERR_LOG log;

  ui_print ( VDEBUG, "Entering sci_lookup_latest_rev_list.\n" );

  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
    return ( ERROR );
  /* if */

  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* if */

  * missing_revs = FALSE;
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next) {
    if ( sci_ptr -> defunct )
      continue;
    /* if */
    src_ctl_lookup_revision ( sci_ptr -> name, set_name, buf, &log );
    if ( buf == NULL || *buf == '\0' ) {
      sci_ptr -> ver_latest = NULL;
      * missing_revs = TRUE;
    } else {
      sci_ptr -> ver_latest = strdup ( buf );
      if ( sci_ptr -> ver_latest == NULL ) {
        ui_print ( VFATAL, "strdup of sci_ptr -> ver_latest failed.\n" );
        status = ERROR;
        break;
      } /* if */
    } /* if */
  } /* for */
  log = oxm_close ( rcs_monitor );

  ui_print ( VDEBUG, "Leaving sci_lookup_latest_rev_list.\n" );
  return ( status );
} /* sci_lookup_latest_rev_list */

/*
 * FUNCTION sci_lookup_ancestor_rev_list
 *
 * Provide ancestor revisions for each file in a given list and a rev string
 */
int sci_lookup_ancestor_rev_list ( SCI_LIST sl, char * rev_str,
                                   int * missing_revs )
{
  int status = OK;
  char buf [PATH_LEN];
  SCI_ELEM sci_ptr;
  ERR_LOG log;

  ui_print ( VDEBUG, "Entering sci_lookup_ancestor_rev_list.\n" );

  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* if */

  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
    return ( ERROR );
  /* if */

  * missing_revs = FALSE;
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next) {
    if ( sci_ptr -> defunct )
      continue;
    /* if */
    src_ctl_lookup_revision ( sci_ptr -> name, rev_str, buf, &log );
    if ( buf == NULL || *buf == '\0' ) {
      sci_ptr -> ver_ancestor = NULL;
      * missing_revs = TRUE;
    } else {
      sci_ptr -> ver_ancestor = strdup ( buf );
      if ( sci_ptr -> ver_ancestor == NULL ) {
        ui_print ( VFATAL, "strdup of sci_ptr -> ver_ancestor failed.\n" );
        status = ERROR;
        break;
      } /* if */
      sci_ptr -> need_merge = TRUE;
    } /* if */
  } /* for */
  log = oxm_close ( rcs_monitor );

  ui_print ( VDEBUG, "Leaving sci_lookup_latest_rev_list.\n" );
  return ( status );
} /* sci_lookup_latest_rev_list */

/*
 * FUNCTION sci_lookup_rev_list
 *
 * Provide revisions for each file in a given list
 */
int sci_lookup_rev_list ( SCI_LIST sl, char * rev_str, BOOLEAN which_rev,
                          int * missing_revs )
{
  int status = OK;
  char buf [PATH_LEN];
  SCI_ELEM sci_ptr;
  ERR_LOG log;

  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
    return ( ERROR );
  /* if */

  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* if */

  * missing_revs = FALSE;
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next) {
    if ( sci_ptr -> defunct )
      continue;
    /* if */
    src_ctl_lookup_revision ( sci_ptr -> name, rev_str, buf, &log );
    if ( buf == NULL || *buf == '\0' ) {
      if ( which_rev == REV_GENERIC1 )
        sci_ptr -> ver_generic1 = NULL;
      else if ( which_rev == REV_GENERIC1 )
        sci_ptr -> ver_generic2 = NULL;
      /* if */
      * missing_revs = TRUE;
    } else {
      if ( which_rev == REV_GENERIC1 ) {
        sci_ptr -> ver_generic1 = strdup ( buf );
        if ( sci_ptr -> ver_generic1 == NULL ) {
          ui_print ( VFATAL, "strdup of sci_ptr -> ver_generic1 failed.\n" );
          status = ERROR;
          break;
        } /* if */
      } else if ( which_rev == REV_GENERIC2 ) {
        sci_ptr -> ver_generic2 = strdup ( buf );
        if ( sci_ptr -> ver_generic2 == NULL ) {
          ui_print ( VFATAL, "strdup of sci_ptr -> ver_generic2 failed.\n" );
          status = ERROR;
          break;
        } /* if */
      } /* if */
    } /* if */
  } /* for */
  log = oxm_close ( rcs_monitor );

  ui_print ( VDEBUG, "Leaving sci_lookup_rev_list.\n" );
  return ( status );
} /* sci_lookup_rev_list */


/*
 * FUNCTION sci_lookup_merge_rev_list
 *
 * Find the correct version to merge against ( whether an actual merge
 * is necessary or not) .
 */
int sci_lookup_merge_rev_list ( SCI_LIST sl, char * rev_str,
                                char * config_str )
{
  SCI_ELEM sci_ptr;
  char rev1[32];
  int status;
  ERR_LOG log;

 /*
  * determine revision number for version to be merged with.
  */
  status = OK;
  enter ( "sci_lookup_merge_rev_list" );

  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
    return ( ERROR );
  /* if */

  for ( sci_ptr = sci_first ( sl ); sci_ptr != NULL;
        sci_ptr = sci_next ( sci_ptr ) ) {
    if ( src_ctl_lookup_revision( sci_ptr -> name, rev_str, rev1, &log) != 0 ) {
/*
      ui_print ( VFATAL, "Revision %s not found\n", rev_str );
      status = ERROR;
      sci_ptr -> status = status;
*/
      sci_ptr -> has_merge_branch = FALSE;
      if ( src_ctl_lookup_revision ( sci_ptr -> name, config_str, rev1, &log )
           != 0 ) {
        ui_print ( VFATAL, "Revision %s not found\n", rev_str );
        status = ERROR;
        sci_ptr -> status = status;
        continue;
      } /* end if */
    } else
      sci_ptr -> has_merge_branch = TRUE;
    /* end if */
    if ( strstr ( rev1, "(defunct)" ) != NULL ) {
      sci_ptr -> defunct = TRUE;
      status = ERROR;
      ui_print ( VALWAYS, "File %s is defunct.\n", sci_ptr -> name );
    } /* if */
    sci_ptr -> ver_merge = strdup ( rev1 );
  } /* end for */
 /*
  * Error from sci_first or sci_next ?
  */
  if ( is_in_error ( ) )
    status = ERROR;
  /* end */
  log = oxm_close ( rcs_monitor );
  leave ( );
  return ( status );
} /* end sci_lookup_merge_rev_list */


/*
 * FUNCTION sci_ancestor_list
 *
 * Find the ancestors for the files given and determine if
 * a merge is necessary
 */
int sci_ancestor_list ( SCI_LIST sl )
{
  char rev1[32], rev2[32], rev3[32];
  int status;
  SCI_ELEM sci_ptr;
  ERR_LOG log;

  enter ( "sci_ancestor_list" );
  if ( ! confirm_alloc ( sl ) ) {
    leave ();
    return ( ERROR );
  } /* end if */
  status = OK;
  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK ) {
    leave ();
    return ( ERROR );
  } /* if */
  for ( sci_ptr = sl -> head; sci_ptr != NULL;
        sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> ver_user == NULL ) {
      sci_ptr -> status = ERROR;
      continue;
    } /* end if */
    if ( sci_ptr -> defunct ) {
      continue;
    } /* end if */

    src_ctl_get_ancestry ( sci_ptr -> name, &(sci_ptr -> ancestry) );

    /*
     * determine revision number for "our" revision
     */

    strcpy ( rev1, sci_ptr -> ver_merge );
    strcpy ( rev2, sci_ptr -> ver_user );

    rev3[0] = '\0';

    if ( src_ctl_prep_merge ( rev1, rev2, rev3, sci_ptr ) != OK ) {
      status = ERROR;
      sci_ptr -> status = status;
      continue;
    }
  } /* end while */
  log = oxm_close ( rcs_monitor );
  leave ();
  return ( status );
} /* end sci_ancestor_list */

/*
 * FUNCTION sci_is_branch
 *
 * Determine if the given revisions represent branches
 */
int
sci_is_branch ( SCI_LIST sl, int * bad_branches )
{
  SCI_ELEM sci_ptr;
  char * p;
  int i;
  int revision;

  enter ( "sci_is_branch" );
  if ( ! confirm_alloc ( sl ) ) {
    leave ( );
    return ( ERROR );
  } /* if */
  *bad_branches = FALSE;
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> ver_user == NULL)
      continue;
    /* if */
    i = 0;
    for ( p = sci_ptr -> ver_user; *p != NUL ; p++ ) {
      if ( *p == '.' ) {
        i++;
        if ( i == 3 ) {
          p++;
          break;
        }
        /* if */
      } /* if */
    } /* for */
    if ( i == 3 ) {
      ATOI ( revision, p );
    } else
      return ( ERROR );
    /* if */
    if ( revision > 1 )
      sci_ptr -> has_user_branch = TRUE;
    else
      *bad_branches = TRUE;
    /* if */
  } /* for */
  leave ( );
  return ( OK );
} /* end sci_is_branch */

ERR_LOG
sci_check_in_elem ( SCI_ELEM sci_ptr, const char * build_set,
                    const char * user_set, const char * state )
{
  char logmsg [LOGMSGSIZE];
  ERR_LOG log = OE_OK;
  int status = OK;
  char branch_rev[32];
  char * b_ptr;
  BOOLEAN done;
  char comma_v_file[MAXPATHLEN];

  logmsg [0] = '\0';
  begin_atomic ( );
  if ( ! sci_local ) {
    if ( copy_file ( sci_ptr -> name, temp_working_file, TRUE ) != 0) {
      status = ERROR;
      end_atomic ( );
      return ( log );
    } /* if */
  } /* if */
  if ( ! sci_has_log ( sci_ptr ) ) {
    if ( create_leaderless_log ( sci_ptr -> name, user_set, logmsg,
                                 mesgfile ) != OK) {
      status = ERROR;
      end_atomic ( );
      return ( log );
    } /* if */
  } else if ( ! sci_ptr -> defunct ) {
    if (( log = hst_lookup_logmsg( sci_ptr -> ver_user,
                                       sci_ptr -> leader, TRUE,
                                       sci_ptr -> ver_user, 1 ) ) != OE_OK) {
      end_atomic ( );
      return ( log );
    } /* if */
    if ( (log = okmesg ( sci_ptr -> leader, logmsg )) != OE_OK ) {
      end_atomic ( );
      return ( log );
    } /* if */
  } /* if */
 /*
  * If a branch already exists for the set we are submitting to, then
  * we can just bump up the revision number. E.g., W.X.Y.Z goes to
  * W.X.Y.Z+1. Otherwise, we need to create W.X.new branch.1, where
  * NEWBRANCH is determined by src_ctl_create_branch. Then we can bump
  * up the revision number as in the first case.
  */
  if ( ! sci_ptr -> has_merge_branch ) {
    done = FALSE;
    strcpy ( branch_rev, sci_ptr -> ver_merge );
    for ( b_ptr = branch_rev; *b_ptr != '\0'; b_ptr++) {
      if (*b_ptr != '.')
        continue;
      /* if */
      if ( done ) {
        *b_ptr = '\0';
        break;
      } /* if */
      done = TRUE;
    } /* end for */
    concat ( comma_v_file, sizeof(comma_v_file), sci_ptr -> name, ",v", NULL );
    if ( sci_ptr -> defunct ) {
      strcpy ( logmsg, "File is defunct" );
    } /* if */
    if ( src_ctl_create_branch2 ( temp_working_file, comma_v_file, FALSE,
                                  branch_rev, build_set,
                                  sci_ptr -> ver_ancestor, NULL, logmsg,
                                  &log ) != OK) {
      end_atomic ( );
      if ( log == OE_OK ) {
        return ( err_log ( OE_INTERNAL ) );
      } /* if */
      return ( log );
    } /* if */
   /*
    * NOTE: branch_rev is an 'out' parameter. The old value from
    * above is overwritten.
    */
    if ( src_ctl_lookup_revision( sci_ptr -> name, build_set, branch_rev,
                                  &log )
         != 0 ) {
      ui_print ( VFATAL, "Could not find newly created branch\n" );
      ui_print ( VCONT, "for set: '%s'.\n", build_set );
      end_atomic ( );
      return ( log );
    } else
      sci_ptr -> ver_merge = strdup ( branch_rev );
    /* if */
  } /* if */

/*
 * The braces are necessary for this if statement. Do not remove them!
 */
  if ( sci_ptr -> defunct ) {
    if ( src_ctl_check_in ( sci_ptr -> name, sci_ptr -> ver_merge,
                            "File is defunct", "Defunct", &log )
         != OK || log != OE_OK ) {
      end_atomic ( );
      if ( log == OE_OK ) {
        return ( err_log ( OE_INTERNAL ) );
      } /* if */
      return ( log );
    } /* if */
  } else {
    if ( sci_ptr -> has_merge_branch ) {
      if ( src_ctl_check_in ( sci_ptr -> name, sci_ptr -> ver_merge, logmsg ,
                              state, &log ) != OK || log != OE_OK ) {
        end_atomic ( );
        if ( log == OE_OK ) {
          return ( err_log ( OE_INTERNAL ) );
        } /* if */
        return ( log );
      } /* if */
    } /* if */
    if ( save_log_message( mesgfile ) != OE_OK) {
      status = ERROR;
      end_atomic ( );
      return ( log );
    } /* if */
    (void) unlink(mesgfile);
    if ( ! sci_local )
      (void) unlink ( temp_working_file );
  }/* if */
  end_atomic ( );
  return ( log );
} /* end sci_check_in_elem */

/*
 * FUNCTION sci_check_in_list
 *
 * Check in given files.
 */
ERR_LOG
sci_check_in_list ( SCI_LIST sl, char * build_set, char * user_set,
                    char * state, BOOLEAN no_log )
{
  SCI_ELEM sci_ptr;
  ERR_LOG log;
  ERR_LOG e_log;

  enter ( "sci_check_in_list" );
  e_log = OE_OK;
  if ( ! confirm_alloc ( sl ) ) {
    leave ( );
    return ( err_log ( OE_ALLOC ) );
  } /* if */
  set_and_log_kludge ( build_set );
  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
    return ( log );
  /* if */
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> skip) {
      sci_ptr -> skip = FALSE;
      continue;
    }
    check_path ( sci_ptr -> name );
    if ( ( log = sci_check_in_elem ( sci_ptr, build_set, user_set, state ) )
         != OE_OK )
      e_log = log;
    /* if */
  } /* end for */
  if ( (log = oxm_close ( rcs_monitor ) ) != OE_OK )
    e_log = log;
  /* if */
  leave ( );
  return ( e_log );
} /* sci_check_in_list */

/*
 * FIXME:
 * sci_check_in_list2 is an alternate version of sci_check_in_list that
 * works for bci. sci_check_in_list and sci_check_in_list2 should
 * be merged at some point.
 */
int sci_check_in_list2 ( SCI_LIST sl, char * user_set, char * msg_str,
                         BOOLEAN defunct )
{
  SCI_ELEM sci_ptr;
  int status = OK;
  int tmp_status;
  ERR_LOG log;

  enter ( "sci_check_in_list2" );
  if ( ! confirm_alloc ( sl ) ) {
    leave ( );
    return ( ERROR );
  } /* if */
  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK ) {
    leave ();
    return ( ERROR );
  } /* if */
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    temp_func ( sci_ptr );
    if ( sci_has_log ( sci_ptr ) ) {
      if ( ( log = hst_lookup_logmsg ( 
                          sci_ptr -> ver_user, sci_ptr -> leader,
                          FALSE, sci_ptr -> ver_user, 1 ) ) != OE_OK )
        status = ERROR;
      /* if */
    /* if */
    }
    if ( ( tmp_status = real_check_in_file ( sci_ptr, user_set, msg_str,
                                               defunct ) ) != OK )
        status = tmp_status;
    /* if */
  } /* for */
  log = oxm_close ( rcs_monitor );
  leave ();
  return ( status );
} /* sci_check_in_list2 */

int
temp_func ( SCI_ELEM sci_ptr )
{
  check_path ( sci_ptr -> name );
  if (copy_file(working_file, temp_working_file, TRUE) != 0) {
    return ( ERROR );
  } /* if */
  return ( OK );
}

int
real_check_in_file ( SCI_ELEM sci_ptr, char * user_set, char * msg_str,
                     BOOLEAN defunct )
{
  int improper;
  int status;
  int process_logs;
  char logmsg [LOGMSGSIZE];
  char * next_ver_user;
  ERR_LOG log;
  HIST_ELEM new_log;
  HIST_ELEM history;

  process_logs = FALSE;
  logmsg [0] = '\0';
  if ( sci_has_log ( sci_ptr ) ) {
    process_logs = TRUE;
  } /* if */
  if ( process_logs || msg_str == NULL || *msg_str == '\0' ) {
    if ( ( log = okmesg ( sci_ptr -> leader, logmsg ) ) != OE_OK ) {
      status = ERROR;
    } /* if */
  } /* if */
  if ( *logmsg == '\0' )
    concat ( logmsg, LOGMSGSIZE, "\t", msg_str, NULL );
  /* if */
  if ( process_logs &&
	( checklogstate ( temp_working_file, sci_ptr -> leader,
                        &improper ) != OK || improper )) {
    ui_print ( VWARN, "Improper markers or history section in file %s\n",
		sci_ptr->name);
    return ( ERROR );
  }
  else {
    if ( defunct ) {
      src_ctl_check_in ( sci_ptr -> name, sci_ptr -> ver_user, logmsg,
                         "Defunct", &log );
      (void) unlink ( working_file );
    } else {
      status = src_ctl_check_in ( sci_ptr -> name, sci_ptr -> ver_user, 
		logmsg, "Exp", &log );
      if (status == 0) { 
	if (process_logs) {
	   next_ver_user = (char *)hst_next_revision(sci_ptr->ver_user);
      	   new_log = hst_alloc_entry (sci_ptr -> leader, logmsg, next_ver_user);
	   free (next_ver_user);
      	   history = hst_xtract_file (working_file, sci_ptr->leader);
      	   new_log->next = history;
      	   hst_insert_file (working_file, new_log, sci_ptr -> leader);
	   hst_freelist(new_log);
	}
        chmod ( sci_ptr -> name, S_IRUSR | S_IRGRP | S_IROTH );
      } else {
	ui_print (VALWAYS, 
	  "Source control reported an error during check-in on file %s, history is not updated\n",
		sci_ptr->name);
      }

    } /* if */
  } /* if */
  ui_print ( VNORMAL, "%s\n", sci_ptr -> name );
  return ( OK );
} /* real_check_in_file */

int
sci_check_in_file ( SCI_ELEM sci_ptr, char * user_set, char * msg_str,
                    BOOLEAN defunct )
{
  char buf [MAXPATHLEN];
  char prompt [MAXPATHLEN];
  char deflt [MAXPATHLEN];
  char leader [MAXPATHLEN];
  char *ptr;
  FILE *inf;
  int key;
  int fd;
  const char *def;
  struct stat st;
  ERR_LOG log;
  char logmsg [LOGMSGSIZE];
  BOOLEAN autoflag;

  (void) unlink (mesgfile);
  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
    return ( ERROR );
  /* if */
  temp_func ( sci_ptr );
  log = oxm_close ( rcs_monitor );

  key = BA_XTRACT;
  def = "xtract";

 /*
  * FIXME
  * This module should know nothing about interface.c routines!
  */
  autoflag = ui_is_auto ();

  for (;;) {
    switch (key) {
    case BA_ABORT:
      (void) unlink(temp_working_file);
      return ( ABORT );
    case BA_AUTO:
      autoflag = TRUE;
      break;
    case BA_CHECKIN:
      logmsg [0] = '\0';
      if ( ( log = okmesg ( sci_ptr -> leader, logmsg ) ) == OE_OK ) {
        if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
          return ( ERROR );
        /* if */
        real_check_in_file ( sci_ptr, user_set, msg_str, defunct );
        log = oxm_close ( rcs_monitor );
        return ( OK );
      } else {
        if ( err_type ( log ) == RECOVERABLE ) {
          def = "log";
          break;
        } else
          return ( ERROR );
        /* if */
      } /* if */
    case BA_LEADER:
      (void) concat(prompt, sizeof(prompt),
                    "Comment leader for ", canon_working_file, NULL);
      (void) strcpy(deflt, (*(sci_ptr -> leader ) == NUL)
                           ? "NONE" : sci_ptr -> leader);
      for (;;) {
          get_str(prompt, deflt, leader);
          if (strcmp(leader, deflt) == 0)
              break;
          (void) strcpy(deflt, leader);
      }
      if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
        return ( ERROR );
      /* if */
      if (sci_set_comment_leader ( sci_ptr , leader) != 0)
        return( ERROR );
      /* if */
      log = oxm_close ( rcs_monitor );
      def = "xtract";
      break;
    case BA_XTRACT:
      if ( sci_has_log ( sci_ptr ) )
        if ( ( log = hst_lookup_logmsg ( 
                            sci_ptr -> ver_user, sci_ptr -> leader,
                            FALSE, sci_ptr -> ver_user, 1 ) ) != OE_OK )
          if ( err_type ( log ) != RECOVERABLE )
            return ( ERROR );
          /* if */
        /* if */
      /* if */
      if ( stat ( mesgfile, &st) == 0 && st.st_size > 0 ) {
        def = "check-in";
      } else if ( msg_str != NULL ) {
        fd = open(mesgfile, O_WRONLY|O_TRUNC|O_CREAT, LOGMODE);
        if (fd < 0) {
          ui_print ( VFATAL, "open %s\n", mesgfile);
          break;
        }
        if (write(fd, msg_str, strlen ( msg_str ) ) != strlen ( msg_str ) ) {
          ui_print ( VFATAL, "write %s\n", mesgfile);
          (void) close(fd);
          (void) unlink(mesgfile);
          break;
        }
        (void) close(fd);
        def = "check-in";
      } else {
        ptr = concat(buf, sizeof(buf),
                     LOGPROMPT, " for ", canon_working_file, ">>>\n",
                     NULL);
        fd = open(mesgfile, O_WRONLY|O_TRUNC|O_CREAT, LOGMODE);
        if (fd < 0) {
          ui_print ( VFATAL, "open %s\n", mesgfile);
          break;
        }
        if (write(fd, buf, strlen ( buf ) ) != strlen ( buf ) ) {
          ui_print ( VFATAL, "write %s\n", mesgfile);
          (void) close(fd);
          (void) unlink(mesgfile);
          break;
        }
        (void) close(fd);
        ui_print ( VALWAYS, "[ Please create a log message describing your changes ]\n");
        def = "log";
        autoflag = FALSE;
      } /* if */
      break;
    case BA_DIFF:
      if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
        return ( ERROR );
      /* if */
      src_ctl_diff_rev_with_file ( sci_ptr -> ver_user, temp_working_file,
                                   sci_ptr -> name, -1, FALSE, FALSE, &log );
      log = oxm_close ( rcs_monitor );
      break;
    case BA_EDIT:
      (void) runp(EDIT_PROG, EDIT_PROG, temp_working_file, NULL);
      if (stat(mesgfile, &st) < 0 || st.st_size == 0)
        def = "xtract";
      else
        def = "check-in";
      /* if */
      break;
    case BA_LOG:
      (void) runp(EDIT_PROG, EDIT_PROG, mesgfile, NULL);
      def = "check-in";
      break;
    case BA_LOGDIFF:
      ptr = concat(buf, sizeof(buf),
                   LOGPROMPT, " for ", canon_working_file, ">>>\n",
                   NULL);
      fd = open(mesgfile, O_WRONLY|O_TRUNC|O_CREAT, LOGMODE);
      if (fd < 0) {
        ui_print ( VWARN, "open %s\n", mesgfile);
        break;
      }
      if (write(fd, buf, ptr-buf) != ptr-buf) {
        ui_print ( VWARN, "write %s\n", mesgfile);
        (void) close(fd);
        (void) unlink(mesgfile);
        break;
      }
      if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
        return ( ERROR );
      /* if */
      src_ctl_diff_rev_with_file( sci_ptr -> ver_user, temp_working_file,
                                  sci_ptr -> name, fd, FALSE, FALSE, &log);
      (void) close(fd);
      log = oxm_close ( rcs_monitor );
      def = "log";
      break;
    case BA_LIST:
      if ((inf = fopen(mesgfile, "r")) == NULL) {
        ui_print ( VWARN, "fopen %s\n", mesgfile);
        break;
      }
      while (fgets(buf, sizeof(buf), inf) != NULL) {
        (void) fputs( sci_ptr -> leader, stdout);
        if (*buf != NUL && *buf != '\n') {
          (void) putchar('\t');
        }
        (void) fputs(buf, stdout);
      }
      if (ferror(inf) || fclose(inf) == EOF) {
        ui_print ( VWARN, "error reading %s\n", mesgfile);
        (void) fclose(inf);
      }
      break;
    case BA_HELP:
      ui_print ( VALWAYS, "One of the following:\n\n");
      ui_print ( VALWAYS, " abort    - abort check-in\n");
      ui_print ( VALWAYS, " auto     - turn on automatic mode\n");
      ui_print ( VALWAYS, " diff     - diff working file with previous revision\n");
      ui_print ( VALWAYS, " edit     - edit working file\n");
      ui_print ( VALWAYS, " check-in - check-in working file\n");
      ui_print ( VALWAYS, " leader   - set comment leader\n");
      ui_print ( VALWAYS, " xtract   - extract log message\n");
      ui_print ( VALWAYS, " log      - edit log message\n");
      ui_print ( VALWAYS, " logdiff  - use diffs for initial log message\n");
      ui_print ( VALWAYS, " list     - list log message\n");
      break;
    } /* switch */
    if ( autoflag )
      key = stablk ( def, bci_action, FALSE );
    else
      key = getstab("Command? (type \"help\" for a list)", bci_action, def);
    /* if */
  } /* for */
} /* sci_check_in_file */

/*
 * FUNCTION sci_lock_list
 *
 * Lock given files
 * Will not do a lock if sl -> has_merge_branch is FALSE.
 */
int
sci_lock_list ( SCI_LIST sl, int which_rev )
{
  SCI_ELEM sci_ptr;
  char tmp_buf [MAXPATHLEN];
  int status = OK;
  ERR_LOG log;
  struct stat st;

  enter ( "sci_lock_list" );
  if ( ! confirm_alloc ( sl ) ) {
    leave ();
    return ( ERROR );
  } /* if */
  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
    return ( ERROR );
  /* if */
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> skip ) {
      sci_ptr -> skip = FALSE;
      continue;
    } /* if */
    concat ( tmp_buf, sizeof (tmp_buf), sci_ptr -> name, ",v" , NULL );
    begin_atomic ( );
    if ( which_rev == 1 ) {
      if ( sci_ptr -> has_merge_branch )
        if ( src_ctl_lock_revision ( sci_ptr -> ver_merge, tmp_buf, &log )
                  != OK )
          status = ERROR;
        else
          if ( submitting )
            track_insert ( sci_ptr -> name );
          /* if */
        /* if */
      /* if */
    } else if ( which_rev == 2 ) {
      if ( src_ctl_lock_revision ( sci_ptr -> ver_user, tmp_buf, &log )
                != OK )
        status = ERROR;
      /* if */
      if (access( sci_ptr -> name, W_OK) < 0)
        if (stat( sci_ptr -> name, &st) == 0)
          if (chmod( sci_ptr -> name, (int)(st.st_mode|S_IWRITE)&MODEMASK)
              < 0) {
            ui_print ( VFATAL, "chmod %s\n",  sci_ptr -> name);
            status = ERROR;
          } /* if */
        /* if */
      /* if */
    } else if ( which_rev == 3 ) {
      if ( src_ctl_lock_revision ( sci_ptr -> ver_latest, tmp_buf, &log )
                != OK )
        status = ERROR;
      /* if */
    } /* if */
    end_atomic ( );
  } /* end for */
  if ( ( log = oxm_close ( rcs_monitor ) ) != OE_OK )
    return ( ERROR );
  /* if */
  leave ();
  return ( status );
} /* end sci_lock_list */

/*
 * FUNCTION sci_merge_list
 *
 * This function and merge_elem are temporary kludges. 
 * They should be replaced with other functions such that
 * the merging can be controlled externally.
 */
int
sci_merge_list ( SCI_LIST sl, BOOLEAN no_log )
{
  SCI_ELEM sci_ptr;
  int status = OK;
  int status2;
  int m_e_status;
  int fd;
  ERR_LOG log;

  enter ( "sci_merge_list" );
  if ( ! confirm_alloc ( sl ) ) {
    leave ( );
    return ( ERROR );
  } /* if */

  if (!temp_merge_setup)
    src_ctl_setup_merge();
  /* end if */
  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
    return ( ERROR );
  /* if */
  cnxtn_open = TRUE;
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> skip ) {
      sci_ptr -> skip = FALSE;
      continue;
    }
    ui_print ( VALWAYS, "%s\n", sci_ptr -> name );
    check_path ( sci_ptr -> name );
    if ( sci_ptr -> defunct ) {
      (void) unlink(temp2);
      if ((fd = open(temp2, O_WRONLY|O_TRUNC|O_CREAT, 0600)) < 0) {
        ui_print ( VFATAL, "Unable to open %s for write", temp2);
        (void) unlink(temp1);
        (void) unlink(temp2);
        status = ERROR;
        continue;
      }
      ui_print ( VDETAIL, "Retrieving revision %s\n", sci_ptr -> ver_merge );
      status2 = OK;
      if ( src_ctl_check_out_with_fd( sci_ptr -> name, sci_ptr -> ver_merge,
                                      fd, sci_ptr -> leader, &log) == ERROR)
        status2 = ERROR;
      /* if */
      (void) close(fd);
      if ( status2 != OK ) {
        ui_print ( VFATAL, "Check out failed.\n", sci_ptr -> ver_user );
        (void) unlink(temp1);
        (void) unlink(temp2);
        status = ERROR;
        continue;
      }
      if (rename(temp2, temp_working_file) < 0) {
        ui_print ( VFATAL, "Unable to rename %s to %s\n", temp2,
                   temp_working_file);
        (void) unlink(temp2);
        status = ERROR;
        continue;
      }
    } else {
      if ( submitting || sci_ptr -> need_merge ) {
        m_e_status = merge_elem ( sci_ptr, no_log );
        if ( m_e_status != OK ) {
          status = m_e_status;
          if ( status == ABORT )
            break;
          else
            continue;
          /* if */
        } /* if */
      } /* if */
      sci_ptr -> merged_up = TRUE;
    }
    if ( submitting || sci_ptr -> need_merge ) {
      if ( rename ( temp_working_file, sci_ptr -> name ) != 0) {
        status = ERROR;
        continue;
      } /* if */
      unlink ( temp_working_file );
    } /* if */
#ifdef notdef
    if ( submitting )
      track_insert ( sci_ptr -> name );
    /* if */
#endif
  } /* end for */

  if ( cnxtn_open )
    if ( ( log = oxm_close ( rcs_monitor ) ) != OE_OK ) {
      ui_print ( VALWAYS, "Problem on close\n" );
      return ( ERROR );
    } /* if */
  /* if */

  leave ( );
  return ( status );
} /* end sci_merge_list */

ERR_LOG
sci_update_build_file ( SCI_ELEM sci_ptr )
{
  int status;
  ERR_LOG log = OE_OK;

    begin_atomic ( );
    if ( sci_ptr -> defunct ) {
      if ( remove_working_file ( FALSE ) != OK ) {
        status = ERROR;
        ui_print (VFATAL, "Unable to update %s in default build\n", sci_ptr->name);
        end_atomic ( );
        return ( log );
      } /* end if */
    } else {
      if ( copy_file ( temp_working_file, sci_ptr -> name, FALSE ) !=  OK ) {
        (void) unlink(temp_working_file);
        status = ERROR;
        ui_print (VFATAL, "Unable to update %s in default build\n", sci_ptr->name);
        end_atomic ( );
        return ( log );
      } /* end if */
    } /* end if */
    end_atomic ( );
    return ( log );
} /* end sci_update_build_file */

/*
 * FUNCTION sci_update_build_list
 *
 * Update the backing build copies of the files. Either check them out
 * or remove (defunct) them.
 */
int
sci_update_build_list ( SCI_LIST sl )
{
  SCI_ELEM sci_ptr;
  int status = OK;
  ERR_LOG log;

  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* if */
  if ( ( log = oxm_open ( &src_monitor, SRC_MONITOR ) ) != OE_OK )
    return ( ERROR );
  /* if */
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> skip) {
      sci_ptr -> skip = FALSE;
      continue;
    }
    check_path ( sci_ptr -> name );
    sci_update_build_file ( sci_ptr );
  } /* end for */
  if ( ( log = oxm_close ( src_monitor ) ) != OE_OK ) {
    return ( ERROR );
  } /* if */
  return ( status );
} /* end sci_update_build_list */


/*
 * FUNCTION sci_outdate_list
 */
int
sci_outdate_list ( SCI_LIST sl, char * set_name )
{
  SCI_ELEM sci_ptr;
  int status = OK;
  char o_string[MAXPATHLEN];
  int i;
  const char *av[16];
  char rcs_file_name [MAXPATHLEN];
  char symbolic_name_switch [MAXPATHLEN];
  ERR_LOG log;

  enter ( "sci_outdate_list" );
  if ( ! confirm_alloc ( sl ) ) {
    leave ();
    return ( ERROR );
  } /* end if */
  set_and_log_kludge ( set_name );
  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK ) {
    leave ();
    return ( ERROR );
  } /* if */

  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> skip) {
      sci_ptr -> skip = FALSE;
      continue;
    }
    concat ( o_string, sizeof ( o_string ), "-o:", sci_ptr -> ver_user, NULL );
    i = 0;
    concat ( rcs_file_name, sizeof ( rcs_file_name ), sci_ptr -> name,
             ",v", NULL );
    concat ( symbolic_name_switch, sizeof ( symbolic_name_switch ), "-n",
             set_name, NULL );
    av[i++] = "outdate";
    av[i++] = o_string;
    av[i++] = symbolic_name_switch;
    av[i++] = rcs_file_name;
    av[i++] = NULL;

    begin_atomic ( );

    if ( ( log = oxm_runcmd ( rcs_monitor, i, av, NULL ) ) != OE_OK ) {
      status = ERROR;
      end_atomic ();
      continue;
    } /* if */
    log = oxm_endcmd( rcs_monitor, &status);

    if ( src_ctl_set_remove ( sci_ptr ) != OK ) {
      status = ERROR;
      end_atomic ();
      continue;
    }
    if ( unlink ( sci_ptr -> name ) == 0 )
      ui_print ( VDETAIL, "rm: removing %s\n", sci_ptr -> name);
    /* end if */
    if ( submitting )
      track_insert ( sci_ptr -> name );
    /* if */
    end_atomic ( );
  } /* end for */
  log = oxm_close ( rcs_monitor );
  leave ();
  return ( status );
} /* end sci_outdate_list */

/*
 * FUNCTION sci_outdate_list
 */
int
sci_outdate_list_p1 ( SCI_LIST sl, char * set_name )
{
  SCI_ELEM sci_ptr;
  int status = OK;
  ERR_LOG log;

  enter ( "sci_outdate_list" );
  if ( ! confirm_alloc ( sl ) ) {
    leave ();
    return ( ERROR );
  } /* end if */
  set_and_log_kludge ( set_name );
  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK ) {
    leave ();
    return ( ERROR );
  } /* if */

  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> skip) {
      sci_ptr -> skip = FALSE;
      continue;
    }
    if ( sci_ptr -> ver_user == NULL )
      continue;
    /* if */
    src_ctl_outdate ( sci_ptr -> name, sci_ptr -> ver_user, set_name, FALSE );
    if ( submitting )
      track_insert ( sci_ptr -> name );
    /* if */
    ui_print ( VNORMAL, "%s\n", sci_ptr -> name );
    end_atomic ( );
  } /* end for */
  log = oxm_close ( rcs_monitor );
  leave ();
  return ( status );
} /* end sci_outdate_list_p1 */

/*
 * FUNCTION sci_outdate_list
 */
int
sci_outdate_list_p2 ( SCI_LIST sl, char * set_name )
{
  SCI_ELEM sci_ptr;
  int status = OK;

  enter ( "sci_outdate_list" );
  if ( ! confirm_alloc ( sl ) ) {
    leave ();
    return ( ERROR );
  } /* end if */
  set_and_log_kludge ( set_name );
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> skip) {
      sci_ptr -> skip = FALSE;
      continue;
    }

    if ( src_ctl_set_remove ( sci_ptr ) != OK ) {
      status = ERROR;
      end_atomic ();
      continue;
    }
    if ( unlink ( sci_ptr -> name ) == 0 )
      ui_print ( VDETAIL, "rm: removing %s\n", sci_ptr -> name);
    /* end if */
    ui_print ( VNORMAL, "rm: removing %s\n", sci_ptr -> name);
    end_atomic ( );
  } /* end for */
  leave ();
  return ( status );
} /* end sci_outdate_list_p2 */


/*
 * FUNCTION sci_trackfile
 *
 * The following function is a temporary kludge.
 * externally to set the track file.
 */
void
sci_trackfile ( char * file_name, char * log_file )
{
  ui_print ( VDEBUG, "Entering sci_trackfile\n" );
  strcpy ( trackfile, file_name );
  strcpy ( bcslog, log_file );
  ui_print ( VDEBUG, "Leaving sci_trackfile\n" );
}

/*
 * FUNCTION sci_config_lookup_list
 */
int
sci_config_lookup_list ( SCI_LIST sl )
{
  SCI_ELEM sci_ptr;
  char config_rev [32];
  int status;

  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* end if */
  status = OK;
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( src_ctl_config_lookup ( sci_ptr, config_rev ) != OK ) {
      status = ERROR;
      continue;
    }
    sci_ptr -> ver_config = strdup ( config_rev );
  } /* end if */
  return ( status );
} /* end sci_config_lookup_list */

/*
 * FUNCTION: sci_show_log_list
 */
ERR_LOG
sci_show_log_list ( SCI_LIST sl, BOOLEAN rev, BOOLEAN lock_users, 
		    BOOLEAN header,
                    BOOLEAN rcs_path, BOOLEAN long_format )
{
  SCI_ELEM sci_ptr;
  int status;
  ERR_LOG log;

  enter ( "sci_show_log_list" );

  if ( ! confirm_alloc ( sl ) )
    return ( err_log ( OE_INTERNAL ) );
  /* end if */
  status = OK;

  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK )
    return ( log );
  /* if */

  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( rev == REV_USER ) {
      if ( sci_ptr -> ver_user == NULL ) {
        ui_print ( VNORMAL, "%s has no branch for this sandbox.\n",
                            sci_ptr -> name );
      } else {
        if ( src_ctl_show_log ( sci_ptr -> name,
                                get_branch (sci_ptr -> ver_user),
                                lock_users, header, rcs_path,
                                long_format, &log ) != OK ) {
          status = ERROR;
          continue;
        } /* if */
      } /* if */
    } else if ( rev == REV_LATEST ) {
      if ( sci_ptr -> ver_latest == NULL ) {
        ui_print ( VNORMAL, "%s does not contain the specified revision.\n",
                            sci_ptr -> name );
      } else {
        if ( src_ctl_show_log ( sci_ptr -> name,
                                get_branch ( sci_ptr -> ver_latest ),
                                lock_users, header, rcs_path,
                                long_format, &log ) != OK ) {
          status = ERROR;
          continue;
        } /* if */
      } /* if */
    } else if ( rev == REV_ALL ) {
        if ( src_ctl_show_log ( sci_ptr -> name,
                                NULL,
                                lock_users, header, rcs_path,
                                long_format, &log ) != OK ) {
          status = ERROR;
          continue;
        } /* if */
    } /* if */
  } /* end if */
  if ( ( log = oxm_close ( rcs_monitor ) ) != OE_OK )
    return ( log );
  /* if */
  if ( status == ERROR )
    log = err_log ( OE_INTERNAL );
  /* if */
  leave ();
  return ( log );
} /* end sci_show_log_list */

/*
 * FUNCTION: sci_diff_rev_with_file
 */
ERR_LOG
sci_diff_rev_with_file ( SCI_LIST sl, BOOLEAN context, BOOLEAN whitespace )
{
  ERR_LOG log;
  SCI_ELEM sci_ptr;
  struct stat statbuf;
  char real_file[MAXPATHLEN];
  char temp_file[MAXPATHLEN];
  int status;

  enter ( "sci_diff_rev_with_file" );
  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK ) {
    leave ();
    return ( log );
  } /* if */

  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if ( sci_ptr -> ver_generic1 == NULL) {
	ui_print (VWARN, "There is no such revision associated \n");
	ui_print (VCONT, "with file %s\n", sci_ptr->name);
	continue;
    }

    status = lstat (sci_ptr->name, &statbuf);
    if ((statbuf.st_mode & S_IFMT) == S_IFLNK) {
	/* Follow all links to a regular file. */
	strcpy (temp_file, sci_ptr->name);
	for(;;) {
		readlink (temp_file, real_file, MAXPATHLEN);
    		status = lstat (real_file, &statbuf);
		if (status != 0) {
			break;
		} else if ((statbuf.st_mode & S_IFMT) == S_IFREG) {
			break;
		} else if ((statbuf.st_mode & S_IFMT) == S_IFLNK) 
			strcpy(temp_file, real_file);
	}
	if (status != 0) {
		ui_print (VWARN, 
			"Cannot check differences on file %s\n",
			sci_ptr->name);
	} else {
    		check_path ( real_file );
    		src_ctl_diff_rev_with_file ( sci_ptr -> ver_generic1, real_file,
                                             sci_ptr -> name, -1, context,
                                             whitespace, &log );
	}

    } else {
	/* Regular file */
    	check_path ( sci_ptr -> name );
    	src_ctl_diff_rev_with_file ( sci_ptr -> ver_generic1, working_file,
                                     sci_ptr -> name, -1, context,
                                     whitespace, &log );
    }
  } /* for */

  if ( ( log = oxm_close ( rcs_monitor ) ) != OE_OK ) {
    leave ();
    return ( log );
  } /* if */
  leave ();
  return ( log );
} /* end sci_diff_rev_with_file */

/*
 * FUNCTION: sci_diff_rev_with_rev
 */
ERR_LOG
sci_diff_rev_with_rev ( SCI_LIST sl, BOOLEAN context, BOOLEAN whitespace )
{
  ERR_LOG log;
  SCI_ELEM sci_ptr;

  enter ( "sci_diff_rev_with_rev" );
  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK ) {
    leave ();
    return ( log );
  } /* if */

  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    if (sci_ptr->ver_generic1 == NULL) {
	ui_print (VWARN, 
		"The revision specified with the '-r' option cannot\n");
	ui_print (VCONT, 
		"be found in file %s\n", sci_ptr->name);
	continue;
    }
    if (sci_ptr->ver_generic2 == NULL) {
	ui_print (VWARN, 
		"The revision specified with the '-R' option cannot\n");
	ui_print (VCONT, 
		"be found in file %s\n", sci_ptr->name);
	continue;
    }
    	check_path ( sci_ptr -> name );
    	src_ctl_show_diffs ( sci_ptr -> ver_generic1, sci_ptr -> ver_generic2,
                         sci_ptr -> name, context, whitespace, &log );
  } /* for */

  if ( ( log = oxm_close ( rcs_monitor ) ) != OE_OK ) {
    leave ();
    return ( log );
  } /* if */
  leave ();
  return ( log );
} /* end sci_diff_rev_with_rev */

/*
 * FUNCTION: sci_set_symbol_list
 */
ERR_LOG
sci_set_symbol_list ( SCI_LIST sl, char * sym_str, BOOLEAN override )
{
  ERR_LOG log;
  SCI_ELEM sci_ptr;

  enter ( "sci_set_symbol_list" );
  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK ) {
    leave ();
    return ( log );
  } /* if */

  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_ptr -> next ) {
    log = (ERR_LOG) src_ctl_add_symbol ( sci_ptr -> name, sym_str, override );
    if ( log != OE_OK )
      ui_print ( VFATAL, "%s\n", err_str ( log ) );
    /* if */
  } /* for */

  if ( ( log = oxm_close ( rcs_monitor ) ) != OE_OK ) {
    leave ();
    return ( log );
  } /* if */
  leave ();
  return ( OE_OK );
} /* end sci_set_symbol_list */

extern int oxm_local;

ERR_LOG
sci_submit ( SCI_LIST file_set, char * build_set, char * user_set,
             BOOLEAN info )
{
  SCI_ELEM sci_ptr;
  ERR_LOG log;
  char tempfile [MAXPATHLEN];
  char tempdir [MAXPATHLEN];
  int status;

  enter ( "sci_submit" );
  if ( ! info ) {
    oxm_local = TRUE;
    if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK ) {
      leave ();
      return ( log );
    } /* if */
    oxm_local = FALSE;
    if ( ( log = oxm_open ( &src_monitor, SRC_MONITOR ) ) != OE_OK ) {
      leave ();
      return ( log );
    } /* if */

    strcpy ( tempfile , "dummy" );
    temp_working_file[0] = '\0';
    strcpy ( tempdir, "#srvtmpXXXXXX" );
    opentemp ( tempdir , tempdir );
    (void) concat(mesgfile, sizeof(mesgfile), tempdir, "/_LOG_", NULL);
  } else {
    ui_print ( VNORMAL, "Would submit the following files:\n" );
    log = OE_OK;
  } /* if */
  for ( sci_ptr = file_set -> head; sci_ptr != NULL; sci_ptr = sci_ptr-> next) {
    if ( sci_ptr -> skip ) {
      sci_ptr -> skip = FALSE;
      continue;
    } /* if */
    check_path ( sci_ptr -> name );
    if ( !info ) { 
      oxm_local = TRUE;
      sci_local = TRUE;
      concat ( temp_working_file, sizeof(temp_working_file), tempdir,
               "/", working_file_tail, NULL );
      if ( ( status = src_ctl_check_out( sci_ptr -> name, sci_ptr -> ver_user,
                              sci_ptr -> leader, &log ) ) != 0 ) {
        leave ();
        return ( err_log ( OE_INTERNAL ) );
      } /* if */
      if ( log != OE_OK ) {
        leave ();
        return ( log );
      } /* if */
      if ( ( log = sci_check_in_elem ( sci_ptr, build_set, user_set, "Exp" ) )
            != OE_OK ) {
        leave ();
        return ( log );
      } /* if */
      sci_local = FALSE;
      oxm_local = FALSE;
      if ( ( log = sci_update_build_file ( sci_ptr ) ) != OE_OK ) {
        leave ();
        return ( log );
      } /* if */
      track_insert ( sci_ptr -> name );
      unlink ( temp_working_file );
    } /* if */
    ui_print ( VNORMAL, "%s\n", sci_ptr -> name );
  } /* for */
  if ( !info ) {
    rmdir ( tempdir );

    oxm_local = FALSE;
    if ( ( log = oxm_close ( src_monitor ) ) != OE_OK ) {
      leave ();
      return ( log );
    } /* if */
    oxm_local = TRUE;
    if ( ( log = oxm_close ( rcs_monitor ) ) != OE_OK ) {
      leave ();
      return ( log );
    } /* if */
    oxm_local = FALSE;
  } /* if */
  leave();
  return ( log );
} /* end sci_submit */

ERR_LOG
sci_rm_submit ( SCI_LIST file_set, char * set_name )
{
  ERR_LOG log;
  ERR_LOG e_log;
  SCI_ELEM sci_ptr;
  int status;
  char logmsg [MAXPATHLEN];
  char ver_latest [MAXPATHLEN];
  char tempfile [MAXPATHLEN];
  char tempdir [MAXPATHLEN];

  if ( ! confirm_alloc ( file_set ) )
    return ( (ERR_LOG) OE_INTERNAL );
  /* if */
  enter ( "sci_rm_submit" );
  oxm_local = TRUE;
  if ( ( log = oxm_open ( &rcs_monitor, RCS_MONITOR ) ) != OE_OK ) {
    leave ();
    return ( log );
  } /* if */
  oxm_local = FALSE;
  if ( ( log = oxm_open ( &src_monitor, SRC_MONITOR ) ) != OE_OK ) {
    leave ();
    return ( log );
  } /* if */

  strcpy ( tempfile , "dummy" );
  temp_working_file[0] = '\0';
  strcpy ( tempdir, "#srvtmpXXXXXX" );
  opentemp ( tempdir , tempdir );

  for ( sci_ptr = file_set -> head; sci_ptr != NULL; sci_ptr = sci_ptr-> next) {
    if ( sci_ptr -> skip )
      continue;
    /* if */
    check_path ( sci_ptr -> name );
    sci_local = TRUE;
    oxm_local = TRUE;
    concat ( temp_working_file, sizeof(temp_working_file), tempdir,
             "/", working_file_tail, NULL );
    src_ctl_check_out ( sci_ptr -> name, sci_ptr -> ver_merge,
                        sci_ptr -> leader, &log );
    strcpy ( logmsg, "\tCover submission to restore file to previous state" );
    if ( src_ctl_check_in ( sci_ptr -> name, sci_ptr -> ver_latest, logmsg ,
                            "Exp", &log ) != OK ) {
       e_log = log;
    } /* if */
    src_ctl_lookup_revision ( sci_ptr -> name, set_name, ver_latest, &log );
#ifdef notdef
    src_ctl_check_out ( sci_ptr -> name, ver_latest,
                        sci_ptr -> leader, &log );
#endif
    sci_local = FALSE;
    oxm_local = FALSE;
    if ( copy_file ( temp_working_file, sci_ptr -> name, FALSE ) !=  OK ) {
      (void) unlink(temp_working_file);
      status = ERROR;
      ui_print (VFATAL, "Unable to update %s in default build\n",
                         sci_ptr->name);
      end_atomic ( );
      continue;
    } /* end if */
    (void) unlink(temp_working_file);
    ui_print ( VNORMAL, "%s\n", sci_ptr -> name );
  } /* for */

  oxm_local = FALSE;
  if ( ( log = oxm_close ( src_monitor ) ) != OE_OK ) {
    leave ();
    return ( log );
  } /* if */
  oxm_local = TRUE;
  if ( ( log = oxm_close ( rcs_monitor ) ) != OE_OK ) {
    leave ();
    return ( log );
  } /* if */
  oxm_local = FALSE;
  leave ();
  return ( OE_OK );
} /* end sci_rm_submit */

/****************************** New sci functions *************************/
/* Note: some of these functions are not currently used. Some of the ones */
/* that aren't being used don't work.                                     */
/****************************** New sci functions *************************/

STATIC
char * tmpfile_name(char * );

STATIC
int tmpfile_create(char * );

STATIC
void
tmpfile_delete(char * );

STATIC
char * tmpfile_base(void);

STATIC
char * tail_pathname(char *);

/*
 *  PROCEDURE sci_create_files ()
 *
 *  ARGUMENTS: A list of files (in the structure SCI_LIST) to place under
 *             source control.
 *
 *  PURPOSE - Given a list of file names.  This procedure will place those
 * 	      files under source control.  If the files to place under
 *            source control do not exist in the sandbox, then a file
 *            is created in the sandbox with a copyright
 *            marker and history section.
 *
 */
int
sci_create_files ( SCI_LIST sl, const char * copy_name,
                   BOOLEAN check_copyrights )
{
   SCI_ELEM sl_elem;
   char * tail_name;

   FILE *fbuf;
   struct stat st;

   int i, ii = 0;
   int fd;
   const char *av[16];
   char rcs_file[MAXPATHLEN];
   ERR_LOG log;
   int status;
   struct stat statb;
   char * ptr;

   if (check_copyrights == TRUE) {
   	if (copy_name == NULL)
		copy_name = "DEFAULT";

   	for (ii = 0; ii < valid_copyrights; ii++) {
		if (strcmp(copy_name, copyright_name[ii]) == 0)
			break;
   	}
   }

   sl_elem = sci_first ( sl );
   log = oxm_open ( &rcs_monitor, RCS_MONITOR );
   while ( sl_elem != NULL ) {

      if (src_ctl_file_exists(sl_elem->name, &log) == 0) {
        	ui_print (VALWAYS,
                	"File %s is already under source control.\n",
                	sl_elem->name);
      } else {

        tail_name = (char *) tail_pathname (sl_elem->name);
      
        /*
         * Set up temporary file. 
         */
        fd = tmpfile_create( sl_elem->name );
	close  ( fd );

	   /*  Set up for remote execution. */
        concat (rcs_file, sizeof(rcs_file), sl_elem->name, ",v", NULL);

    i = 0;
    av[i++] = "rcs";
    av[i++] = "-U";
    av[i++] = "-q";
    av[i++] = "-i";
    av[i++] = "-t/dev/null";
    av[i++] = rcs_file;
    log = oxm_runcmd ( rcs_monitor, i, av, NULL );
    log = oxm_endcmd( rcs_monitor, &status );

	i = 0;
    	av[i++] = "-t2";
    	av[i++] = "6";
    	av[i++] = "ci";
        av[i++] = "-q";
    	av[i++] = "-f";
    	av[i++] = "-u1.1";
    	av[i++] = "-d01-Jan-1990 00:00:00";
    	av[i++] = "-m*** Initial Trunk Revision ***";
    	av[i++] = tail_name;
        av[i++] = rcs_file;
        log = oxm_runcmd ( rcs_monitor, i, av, tmpfile_base() );
    	log = oxm_endcmd( rcs_monitor, &status );
	/*  End of remote execution. */

	/* Set the comment leader. */
	if (sl_elem->leader == NULL) 
	   	sl_elem->leader= (char *)"NONE";


  	   /* Create a branch off the initial revision "1.1". */
        check_path ( sl_elem -> name );
        src_ctl_create_branch ( sl_elem -> name, "1.1", sl_elem -> set, NULL,
				sl_elem -> leader, &log );
        set_insert ( );
	/* Set up the working file in the sandbox. */
	/**********************************************************************/
	/***********  This section of code should be moved somewhere else *****/
     	   if (access(sl_elem->name, F_OK) == 0) {
         	   ui_print(VALWAYS, "[ %s already exists, not modified ]\n",
	              	sl_elem->name);
         	if (access(sl_elem->name, W_OK) < 0) {
             		if (stat(sl_elem->name, &st) < 0)
                 	   ui_print (VALWAYS, 
				"Unable to fix access rights on %s", 
				sl_elem->name);
             		else if (chmod(sl_elem->name, 
				(int)(st.st_mode|S_IWRITE)&0777) < 0)
                 	   ui_print (VALWAYS, 
				"Unable to fix access rights on %s", 
				sl_elem->name);
         	}
     	   } else {

             /* File does not exist.  Create a new file 
                with appropriate header. */
             if (stat(working_file_dir, &statb) < 0) {
                 if (makedir(working_file_dir) != 0)
                     return( ERROR );
             } else if ((statb.st_mode&S_IFMT) != S_IFDIR) {
                 ui_print ( VFATAL, "%s: not a directory\n", working_file_dir);
                 return( ERROR );
             }
            if ((fbuf = fopen(sl_elem->name, "w")) == NULL) {
                	ui_print (VALWAYS, "unable to open %s for writing", 
				sl_elem->name);
	    } else {
            	if  ((sl_elem->leader != NULL) &&
                 sci_has_log ( sl_elem ) ) {
              		if (strcmp(sl_elem->leader, " * ") == 0 )
                		fprintf(fbuf,"/*\n");

			if (check_copyrights == FALSE) {
              			fprintf (fbuf,"%s%s\n", sl_elem->leader, 
					default_copyright());
				fprintf (fbuf, "%s", sl_elem->leader);
			} else {
              		  fprintf (fbuf,"%sCOPYRIGHT NOTICE\n", sl_elem->leader);
			  fprintf (fbuf, "%s", sl_elem->leader);
			  for (ptr = raw_copyright_list[ii]; ((ptr != NULL) && (*ptr != '\0'));
				) {
				if (*ptr == '\n')  {
					fprintf (fbuf, "\n");
					fprintf (fbuf, "%s", sl_elem->leader);
					ptr++;
				} else if ( (strlen(ptr) >= 7) && 
					(strncmp(ptr, "@YEARS@", 7) == 0) ) {
					if (copyright_years != NULL)
						fprintf (fbuf, "%s", copyright_years);
					else
						ui_print (VWARN, "Variable copyright_years is not defined in the resource files.\n");
					ptr = ptr + 7;
				} else {
					fprintf (fbuf, "%c", *ptr);
					ptr++;
				}
			  }	
			}

              		if (strcmp(sl_elem->leader, " * ") == 0 ) {
                		fprintf(fbuf,"\n */\n");
                		fprintf(fbuf,"/*\n");
              		} else
                		fprintf(fbuf,"\n%s\n",sl_elem->leader);
              		fprintf(fbuf,"%sHISTORY\n",sl_elem->leader);
              /* the following line has to be written in two pieces to keep
                 it from being expanded by RCS */
              		fprintf(fbuf,"%s$Lo",sl_elem->leader);
              		fprintf(fbuf,"g: $\n");
              		fprintf(fbuf,"%s$EndLog$\n",sl_elem->leader);
              		if (strcmp(sl_elem->leader, " * ") == 0 )
                		fprintf(fbuf," */\n");
              		fclose(fbuf);
		}
            }
           }
	   /*********************************************************************/

	   tmpfile_delete ( sl_elem->name );
      }
      ui_print ( VNORMAL, "%s created.\n", sl_elem->name );
      sl_elem = sci_next ( sl_elem );
   }
   log = oxm_close ( rcs_monitor );
   return ( OK );
}

/*
 *  PROCEDURE sci_delete_files ()
 *
 *  ARGUMENTS: A list of files (in the structure SCI_LIST) to remove from 
 *             source control.
 *
 *  PURPOSE - Given a list of file names.  This procedure will "undo"
 *            a file(s) that has just been created. 
 *
 */
ERR_LOG
sci_delete_files ( SCI_LIST sl, char * set_name )
{
   SCI_ELEM sl_elem;
   ERR_LOG log;

   sl_elem = sci_first ( sl );
   log = oxm_open ( &rcs_monitor, RCS_MONITOR );
   while ( sl_elem != NULL ) {
     check_path ( sl_elem -> name );
     bcreate_undo ( sl_elem, set_name );
     sl_elem = sci_next ( sl_elem );
   }
   log = oxm_close ( rcs_monitor );
   return ( log );
}

STATIC
char * 
tail_pathname ( char * pathname )
{

   char *tail; 
   char *cptr;
   int  i,
	len; 
/*
   return ( working_file_tail );
*/
   /* If we're passed a null pointer or a pointer to an empty string
      then pass back an empty string. */
   if ((pathname == NULL) || (*pathname == '\0'))
	return ((char *)"");


   /*  Extract the last component of 'pathname'. */


   /* Find the end of the string. */
   for (i = 0, cptr = pathname; *cptr != '\0'; i++, cptr++);

   /* Go back to the last component in the pathname ('/') or beginning of
      the string. */
   for (len = 0; (i != 0) && (*cptr != '/') ; len++, cptr--, i--);

   /* Adjust pointer */
   if ( *cptr == '/' ) {
	cptr++;
	len--;
   }

   tail = (char *)malloc ( (size_t) (len + 1) );

   strcpy (tail, cptr);

   return (tail);
}

int
sci_read_files ( SCI_LIST sl, char * revision )
{
  SCI_ELEM sl_elem;
  char * selected_rev;
  ERR_LOG log;
  BOOLEAN defunct;
  struct stat st;

  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* end if */
  sl_elem = sci_first ( sl );
  log = oxm_open ( &rcs_monitor, RCS_MONITOR );
  while ( sl_elem != NULL ) {
    check_path ( sl_elem -> name );
    sci_read_file ( sl_elem->name, revision, &selected_rev, &defunct, &log );
    if ( log == OE_OK ) { 
      stat ( sl_elem->name, &st );
      chmod ( sl_elem->name, ( st.st_mode & ( S_IXUSR | S_IXGRP | S_IXOTH ) ) |                              S_IRUSR | S_IRGRP | S_IROTH );
      ui_print ( VNORMAL , "%s\n", sl_elem -> name );
      /* Select next element for processing.  */
    } /* if */
    sl_elem = sci_next ( sl_elem );
  }
  log = oxm_close ( rcs_monitor );
  return (OK);
}

void
sci_read_file ( char * pathname, char * rev_str, char ** selected_rev,
                BOOLEAN * defunct, ERR_LOG * log )
{
  int i;
  const char *av[16];
  char *rev_arg;
  char rcs_file[MAXPATHLEN];
  int status;
  char revision [32];
  char *tempfile;
/*
 * FIXME: redundant?
   if (src_ctl_file_exists(pathname, &log) != 0) {
	ui_print (VALWAYS,
		"File %s is not under source control.\n",
		pathname);
	return;
   }
 */
   if ( src_ctl_lookup_revision ( pathname, rev_str, revision, log ) != 0 ) {
     ui_print ( VWARN, "File does not contain the revision specified. \n" );
     ui_print ( VCONT, "File: %s\n", pathname );
     ui_print ( VCONT, "Revision: %s\n", rev_str );
     *log = err_log ( OE_REVISION );
     return;
   } /* if */
     
   *selected_rev = strdup ( revision );
   if ( strstr ( revision, "defunct" ) != NULL ) {
     *defunct = TRUE;
     return;
   } /* if */
   *defunct = FALSE;
   tmpfile_delete ( pathname );
/*
   tail = tail_pathname (pathname);
*/
   concat (rcs_file, sizeof(rcs_file), pathname, ",v", NULL);
   rev_arg = alloc_switch('r', revision );

   i = 0;
   av[i++] = "-t0";
   av[i++] = "3";
   av[i++] = "co";
   av[i++] = "-q";
   av[i++] = rev_arg;
   av[i++] = working_file_tail;
   av[i++] = rcs_file;
   *log = oxm_runcmd ( rcs_monitor, i, av, tmpfile_base () );
   *log = oxm_endcmd ( rcs_monitor, &status );
   /* Copy the file into the sandbox. */
   (void) unlink (pathname);
   tempfile = tmpfile_name(pathname);
   makedir ( working_file_dir );
   rename ( tempfile, pathname );
   free(tempfile);
   free(rev_arg);
}

void
trunk_revision ( char * full_rev, char ** trunk_rev )
{
  int first;
  char * rev;
  /*
   * Keep only the first two fields of revision
   */

  *trunk_rev = strdup ( full_rev );    
  first = FALSE;
  for (rev = *trunk_rev; *rev != '\0'; rev++) {
    if (*rev != '.')
        continue;
    if (first) {
      *rev = '\0';
      break;
    }
    first = TRUE;
  }
} /* trunk_revision */

ERR_LOG
sci_select_not_exists ( SCI_LIST file_set, SCI_LIST * files_not_exist )
{
  SCI_ELEM sci_ptr;
  SCI_ELEM sci_tmp;

  sci_new_list ( files_not_exist );
  for ( sci_ptr = sci_first ( file_set ); sci_ptr != NULL;
        sci_ptr = sci_next ( sci_ptr ) )
    if ( ! sci_ptr -> exists ) {
      sci_tmp = (SCI_ELEM) malloc ( (size_t) sizeof ( struct sci_elem ) );
      memcpy ( (void *)sci_tmp, (void *)sci_ptr,
                sizeof ( struct sci_elem ) );
      if ( (*files_not_exist) -> tail == NULL )
        (*files_not_exist) -> head = sci_tmp;
      else
        ( (*files_not_exist) -> tail ) -> next = sci_tmp;
      /* end if */
      (*files_not_exist) -> tail = sci_tmp;
      (*files_not_exist) -> elem_cnt += 1;
    } /* if */
  /* for */
  return ( OE_OK );
} /* end sci_select_not_exists */
 
int
sci_edit_files ( SCI_LIST sl, char * symbolic_name, char * config_str )
{
   SCI_ELEM sl_elem;
   struct stat st;
   int i;
   int fd;
   const char *av[16];
   char *rev_arg;
   char rcs_file[MAXPATHLEN];
   char *tempfile;
   ERR_LOG log;
   int status;
   char tempslot [2];
   char * selected_rev;
   char * trunk_rev;
   BOOLEAN defunct;
   BOOLEAN is_link;

  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* end if */
   sl_elem = sci_first ( sl );
   log = oxm_open ( &rcs_monitor, RCS_MONITOR );
   while ( sl_elem != NULL ) {
     check_path ( sl_elem -> name );
     lstat ( sl_elem->name, &st );
     is_link = ( st.st_mode & S_IFMT ) == S_IFLNK;
     stat ( sl_elem->name, &st );
     if ( (access (sl_elem->name, F_OK) == 0 ) && 	
		((st.st_mode & S_IWUSR) && ! is_link )) {
        ui_print ( VWARN, "File %s is writeable.\n", sl_elem -> name );
        ui_print ( VCONT, "No check out performed for this file.\n" );
     } else if (src_ctl_file_exists (sl_elem->name, &log) != 0) {
	ui_print (VALWAYS,
		"File %s is not under source control.\n",
		sl_elem->name);
        sl_elem->exists = FALSE;
     } else {
       sl_elem->exists = TRUE;
       /*
        * remove temp file
        */
        tmpfile_delete ( sl_elem->name );

/*
	tail = tail_pathname (sl_elem->name);
*/

        /*  Set up for remote execution. */
        concat (rcs_file, sizeof(rcs_file), sl_elem->name, ",v", NULL);
/*
        src_ctl_lookup_revision ( sl_elem -> name, symbolic_name, revision,
                                  &log );
*/
        if ( sl_elem -> ver_user != NULL ) {

	      rev_arg = alloc_switch('r', sl_elem -> ver_user );
              i = 0;
              av[i++] = "-t2";
              av[i++] = tempslot;
              av[i++] = "co";
/*
              av[i++] = "-l";
*/
              av[i++] = "-q";
	      av[i++] = rev_arg;
      
              tempslot [0]  = '3';
              tempslot [1]  = '\0';
	      /* test if file is binary */
/*
  	      if (strcmp(sci_get_comment_leader ( sl_elem->name ), "BIN")
                         == 0) {
    		      av[i++] = "-ko";
                tempslot [0]  = '4';
	      }
*/
              av[i++] = working_file_tail;
              av[i++] = rcs_file;
              log = oxm_runcmd ( rcs_monitor, i, av, tmpfile_base () );
              log = oxm_endcmd( rcs_monitor, &status );
   
              /*  End of remote execution. */
   
  	      /* Copy the file into the sandbox and make it writable. */
	      (void) unlink (sl_elem->name);
	      tempfile = tmpfile_name (sl_elem->name);
	      rename ( tempfile, sl_elem->name );
	      free (tempfile);

  	      tmpfile_delete ( sl_elem->name );
	} else {

	   sci_read_file ( sl_elem->name, config_str, &selected_rev,
                           &defunct, &log ) ;

           if ( defunct ) {
             /* FIXME: should not print here, should return in status */
             ui_print ( VALWAYS, "'%s' is defunct, aborting check-out.\n",
                        sl_elem -> name );
             sl_elem = sci_next ( sl_elem );
             continue;
           } /* if */
           fd = tmpfile_create ( sl_elem->name );
	   tempfile = tmpfile_name (sl_elem->name);
	   copy_file (sl_elem->name, tempfile, TRUE );
	   free(tempfile);

           trunk_revision ( selected_rev, &trunk_rev );
           ui_print ( VDETAIL, "Creating branch for user from ancestor %s\n",
                               selected_rev );
           src_ctl_create_branch ( sl_elem -> name, trunk_rev, symbolic_name,
                                   selected_rev, NULL, &log );
           set_insert ( );
	   free(trunk_rev);
	   free(selected_rev);

	   (void) close (fd);

           tmpfile_delete ( sl_elem->name );

	}
	/* Leave a writable file in the sandbox. */
	chmod (sl_elem->name, 0644);
        ui_print ( VNORMAL , "%s\n", sl_elem -> name );
     }
     /* Select next element for processing.  */
     sl_elem = sci_next ( sl_elem );
   }
   log = oxm_close ( rcs_monitor );
   return ( OK );
}

int
sci_set_comment_leader ( SCI_ELEM sci_ptr, char * leader )
{
    int i;
    const char *av[16];
    char *leader_arg;
    char rcs_file[MAXPATHLEN];
    int status;
    ERR_LOG log;

    sci_ptr -> leader = strdup ( leader );
    leader_arg = alloc_switch('c', leader );
    concat ( rcs_file, sizeof(rcs_file), sci_ptr -> name, ",v", NULL );
    i = 0;
    av[i++] = "rcs";
    av[i++] = "-q";
    av[i++] = leader_arg;
    if ( ! sci_has_log ( sci_ptr ) ) {
      av[i++] = "-ko";
    } else {
      av[i++] = "-kkv";
    } /* if */
    av[i++] = rcs_file;
    av[i] = NULL;
    if ( ( log = oxm_runcmd ( rcs_monitor, i, av, NULL ) ) != OE_OK )
      return ( ERROR );
    /* if */
    if ( ( log = oxm_endcmd ( rcs_monitor, &status ) ) != OE_OK )
      return ( ERROR );
    /* if */

    return(status);
}

char *
alloc_comment_leader ( char * file, struct rcfile rc_info)
{
   char dbuf[MAXPATHLEN], fbuf[MAXPATHLEN];
   char pattern[PATH_LEN];
   char leader[PATH_LEN];
   char cmt[PATH_LEN];
   char *str;
   int more, i, status, j;


   get_rc_value ("COMMENT_LEADERS", &str, &rc_info, FALSE);
   if (str == NULL)
	return( NULL );
   else
        strcpy (cmt, str);
   pattern[0] = '\0';
   leader[0]='\0';
   path(file, dbuf, fbuf);

   more = TRUE;
   i = 0;

   while (more == TRUE) {
     /* Find the next set of parenthesis. */
     for (; (cmt[i] != '(') && (cmt[i] != '\0'); i++);

     /* If we've run out of comment leaders then return NULL */
     if (cmt[i] == '\0')
	break;

     /* Extract the pattern, */
     i++;
     for (j = 0; (cmt[i] != ';') && (cmt[i] != '\0'); j++, i++) {
	pattern[j] = cmt[i];
     }
     pattern[j] = '\0';

     if (cmt[i] == '\0')
	break;

     /* Extract the comment leader. */
     i++;
     for (j = 0; (cmt[i] != ')') && (cmt[i] != '\0'); j++, i++) {
	leader[j] = cmt[i];
     }
     leader[j] = '\0';

     if (cmt[i] == '\0')
	break;
   
     status = gmatch (fbuf, pattern);
     if (status == TRUE)
	return (strdup(leader));

   }

   return ( NULL );

}

int
match_comment_leader (char * comment_leader, struct rcfile rc_info)
{
   char pattern[PATH_LEN];
   char leader[PATH_LEN];
   char cmt[PATH_LEN];
   char *str;
   int more, i, j;

   get_rc_value ("COMMENT_LEADERS", &str, &rc_info, FALSE);
   if (str == NULL)
	return(FALSE);
   else
        strcpy (cmt, str);

   pattern[0] = '\0';
   leader[0]='\0';

   more = TRUE;
   i = 0;

   while (more == TRUE) {
     /* Find the next set of parenthesis. */
     for (; (cmt[i] != '(') && (cmt[i] != '\0'); i++);

     /* If we've run out of comment leaders then return NONE */
     if (cmt[i] == '\0')
	break;

     /* Extract the pattern, */
     i++;
     for (j = 0; (cmt[i] != ';') && (cmt[i] != '\0'); j++, i++) {
	pattern[j] = cmt[i];
     }
     pattern[j] = '\0';

     if (cmt[i] == '\0')
	break;

     /* Extract the comment leader. */
     i++;
     for (j = 0; (cmt[i] != ')') && (cmt[i] != '\0'); j++, i++) {
	leader[j] = cmt[i];
     }
     leader[j] = '\0';

     if (cmt[i] == '\0')
	break;
   
     if (strcmp( comment_leader, leader) == 0)
	return (TRUE);

   }

   return (FALSE);
}

/*	   Routines for manipulating the ".BCSset-???" files in a sandbox */

char *
set_file_pathname (const char * setname)
{
    char pathname[MAXPATHLEN];
    (void)  concat(pathname, sizeof(pathname), "./.BCSset-",
		setname, NULL);

    return (strdup (pathname));
}

char *
set_log_pathname (char * setname)
{
    char pathname[MAXPATHLEN];
    (void)  concat(pathname, sizeof(pathname), "./.BCSlog-",
                setname, NULL);

    return (strdup (pathname));
}

char *
set_path_pathname (char * setname)
{
    char pathname[MAXPATHLEN];
    (void)  concat(pathname, sizeof(pathname), "./.BCSpath-",
                setname, NULL);

    return (strdup (pathname));
}

int 
set_exists (char * setname)
{

   if (access(set_file_pathname(setname), R_OK) < 0)
	return(FALSE); 
   else
	return(TRUE);

}

int
set_create (char * setname)
{
    int fd;

    fd = open(set_file_pathname (setname), O_WRONLY|O_CREAT|O_EXCL, 0666);

    return (fd);
}

void
set_delete ( char * setname )
{
  char * pathname;

  pathname = set_file_pathname (setname);

  (void) unlink(pathname);
}

void
new_set_cleanup(char * setname)
{
    struct stat st;

    if (stat(set_file_pathname(setname), &st) == 0 && st.st_size > 0)
        return;
    if (unlink(set_file_pathname(setname)) == 0)
        ui_print(VALWAYS,
		"rm: removing %s\n", set_file_pathname(setname));
    if (unlink(set_log_pathname(setname)) == 0)
        ui_print(VALWAYS,
		"rm: removing %s\n", set_log_pathname(setname));
    if (unlink(set_path_pathname(setname)) == 0)
        ui_print(VALWAYS,
		"rm: removing %s\n", set_path_pathname(setname));
}

FILE *
set_fopen ( const char * setname, const char * file_access )
{
  char * working_set_file;
  FILE * inf;

  working_set_file = set_file_pathname (setname);

  inf = fopen(working_set_file, file_access);

  if (inf == NULL)
	ui_print (VALWAYS, "Unable to open set file %s\n",
		working_set_file);
  return (inf);

}

/* Maybe this should be simplified to "file_fread()". */
int
set_fread ( char line[], int maxlinelength, FILE * inf )
{
  int cnt;
  int linelength;

  linelength = maxlinelength;

  cnt = 0;

  if (fgets(line, linelength, inf) == NULL) {
        strcpy (line, "");
        return (-1);
  } else {
        cnt = 0;
        while (cnt < maxlinelength) {
                if (line[cnt] == '\n') {
                        line[cnt] = '\0';
                }
                cnt++;
        }
  }
  return (strlen(line));
}

int
new_set_insert(char * setname, char * pathname)
{
    char tmp_setname[MAXPATHLEN];
    FILE *inf, 
	 *inf_tmp;
    char buffer[MAXPATHLEN];
    int found, 
	setname_inserted;

    concat(tmp_setname, sizeof(tmp_setname), setname, ".tmp", NULL);

    /* Initailize tmpfile */
    set_delete (tmp_setname);
    (void) close (set_create (tmp_setname));

    if (set_exists(setname) == FALSE)
    	(void) close (set_create (setname));

    if ((inf = set_fopen(setname, "r")) == NULL)
        return(FALSE);

    if ((inf_tmp = set_fopen(tmp_setname, "w")) == NULL) {
	(void) fclose (inf);
        return(FALSE);
    }

    found = FALSE;
    setname_inserted = FALSE;
    while (set_fread(buffer, sizeof(buffer), inf) != -1) {
	if (setname_inserted == FALSE) {
		if (strcmp (pathname, buffer) > 0) {
			fputs (buffer, inf_tmp);
			fputc ('\n', inf_tmp);
		} else if (strcmp (pathname, buffer) < 0) {
			fputs (pathname, inf_tmp);
			fputc ('\n', inf_tmp);
			fputs (buffer, inf_tmp);
			fputc ('\n', inf_tmp);
			setname_inserted = TRUE;
		} else {
			/* pathname is already in set. */
			fputs (buffer, inf_tmp);
			fputc ('\n', inf_tmp);
			setname_inserted = TRUE;
		}
	} else {
		fputs (buffer, inf_tmp);
	}
    }
    if (setname_inserted == FALSE) {
	/* Add the name to the end of the file. */
	fputs (pathname, inf_tmp);
	fputc ('\n', inf_tmp);
    }

    (void) fclose (inf);
    (void) fclose (inf_tmp);

    if (rename(set_file_pathname(tmp_setname), 
		set_file_pathname(setname)) < 0) {
        ui_print(VALWAYS, 
		"5 rename %s to %s failed", set_file_pathname(tmp_setname),
		set_file_pathname(setname));
        (void) unlink(set_file_pathname(tmp_setname));
        return(1);
    }

    return(0);
}


int
set_remove(char * setname, char * pathname)
{
    char tmp_setname[MAXPATHLEN];
    char buffer[MAXPATHLEN];
    FILE *inf,
         *inf_tmp;
    int lines_written,
        setname_removed;

    if (set_exists (setname) == FALSE) {
	fprintf (stderr, "Set %s does not exist\n", setname);
	return(1);
    }

    (void) concat(tmp_setname, sizeof(tmp_setname),
                setname, ".tmp", NULL);

    set_delete (tmp_setname);
    (void) close (set_create(tmp_setname));

    if ((inf = set_fopen(setname, "r")) == NULL)
        return(FALSE);

    if ((inf_tmp = set_fopen(tmp_setname, "w")) == NULL) {
        (void) fclose (inf);
        return(FALSE);
    }
	
    lines_written = 0;
    setname_removed = FALSE;
    while (set_fread(buffer, sizeof(buffer), inf) != -1) {
        if (setname_removed == FALSE) {
                if (strcmp (pathname, buffer) == 0) {
			setname_removed = TRUE;
                } else {
			lines_written++;
                        fputs (buffer, inf_tmp);
                        fputc ('\n', inf_tmp);
                }
        } else {
		lines_written++;
                fputs (buffer, inf_tmp);
                fputc ('\n', inf_tmp);
        }
    }

    (void) fclose (inf);
    (void) fclose (inf_tmp);

    if (rename(set_file_pathname(tmp_setname),
                set_file_pathname(setname)) < 0) {
        ui_print(VALWAYS, 
		"6 rename %s to %s failed", set_file_pathname(tmp_setname),
                set_file_pathname(setname));
        (void) unlink(set_file_pathname(tmp_setname));
        return(1);
    }

    if (lines_written == 0) {
	/* Set file is empty, delete it */
	new_set_cleanup (setname);
    }
    return(0);
}

int
set_lookup(char * setname, char * pathname)
{
    char buffer[MAXPATHLEN];
    FILE *inf;
    int found;


    if ((inf = fopen( BCSSET, "r" )) == NULL)
	return(FALSE);

    found = FALSE;
    while (set_fread(buffer, sizeof(buffer), inf) != -1) {
	if (strcmp( buffer, pathname ) == 0 ) 
		found = TRUE;
    }

    fclose(inf);
    return(found);
}

/*         Temporary file routines      */

STATIC
int
tmpfile_create ( char * pathname )
{
  char * tempfile;
  int fd;

  tempfile = tmpfile_name (pathname);

  (void) unlink(temp_working_file);
  fd = open(tempfile, O_WRONLY|O_CREAT|O_EXCL, 0666);

  free(tempfile);
  return (fd);
}

STATIC
char *
tmpfile_name ( char * pathname )
{
  char tempfile[MAXPATHLEN];
  char *tail, *base;

  tail = (char *) tail_pathname (pathname);
  base = tmpfile_base();
  concat ( tempfile, sizeof(tempfile), base, "/",
	tail, NULL);

  free(tail);
  return ( strdup( tempfile ) );
}

STATIC
void
tmpfile_delete ( char * pathname )
{
  char * tempfile;

  tempfile = tmpfile_name (pathname);

  (void) unlink(tempfile);
  free(tempfile);
}

STATIC
char *
tmpfile_base (void)
{

   return (BCSTEMP);

}


/*
 * FUNCTION sci_real_fast_lookup_user_rev_list
 *
 * Provide user revisions for each file in a given list and a given set
 */
int sci_real_fast_lookup_user_rev_list ( SCI_LIST sl , char * set_name ,
                                         int * missing_revs )
{
  int status, ret;
  BOOLEAN keep_looping;
  SCI_ELEM sci_ptr;
  int pid, fd;
  FILE *tempfile_in_fd;
  char template_file [MAXPATHLEN];
  char tempdir  [MAXPATHLEN];
  char tempfile_in  [MAXPATHLEN];
  char tempfile_out  [MAXPATHLEN];
  char file [MAXPATHLEN];
  char dir [MAXPATHLEN];
  char revision [MAXPATHLEN];
  char buffer [MAXPATHLEN];
  char *rev_sw, *file_sw;
  FILE *tempfile_out_fd;

  ui_print ( VDEBUG, "Entering sci_real_fast_lookup_user_rev_list.\n" );
  if ( ! confirm_alloc ( sl ) ) {
    return ( ERROR );
  }
  /* end if */
  status = OK;
  * missing_revs = FALSE;

  /* This routines dumps all filenames in the "sl" list to a temporary file.
     'rcsstat' is executed with the temporaru file as input ("-f").
     The output is directed to yet another temporary file.  We
     then step through the "sl" list and the output temporary file
     looking to match filenames and update the "sl" elements with the
     revision numbers returned from "rcsstat".
   */

  getcwd (dir, sizeof(dir));
  strcpy ( template_file, "#srvtmpXXXXXX" );

  /* Set up directory for temporary files. */
  opentemp ( template_file , tempdir);
  concat (tempfile_out, sizeof(tempfile_out), dir, "/", tempdir, "/rcsstat_out", NULL);
  concat (tempfile_in, sizeof(tempfile_in), dir, "/", tempdir, "/rcsstat_in", NULL);

  /* Open "rcsstat" temporary output file. */ 
  if ((fd = open(tempfile_out, O_WRONLY|O_TRUNC|O_CREAT, 0777)) < 0) { 
	return(ERROR);
  }


  /* Open and write to "rcsstat" temporary input file. */ 
  tempfile_in_fd = fopen (tempfile_in, "w");
  for (sci_ptr = sci_first(sl); sci_ptr != NULL; 	
		sci_ptr = sci_next(sci_ptr) ) {
#ifdef notdef
    if ( sci_ptr -> skip )
      continue;
    /* end if */
#endif
    fprintf (tempfile_in_fd, "%s\n", sci_ptr->name);
  }
  fclose (tempfile_in_fd);

  /* Allocate switches to pass to "rcsstat". */
  rev_sw  = alloc_switch ('r', set_name);
  file_sw = alloc_switch ('f', tempfile_in);

  /* Execute the rcsstat command. */
  pid = fd_runcmd("rcsstat", NULL, TRUE, -1, fd,
	 "rcsstat", "-q", "-V", "-R", "-D", rev_sw, file_sw, NULL); 
  close(fd);

  ret = endcmd(pid);

  /* Open the output file and start sifting through the results. */
  tempfile_out_fd = fopen (tempfile_out, "r");
  if (fgets (buffer, sizeof(buffer), tempfile_out_fd) == NULL) {
	return ( ERROR );
  }
  sscanf (buffer, "%s\t%s\n", file, revision);


  file[strlen(file)-2] = '\0';
  * missing_revs = FALSE;
  keep_looping = TRUE;
  for ( sci_ptr = sci_first(sl); (keep_looping == TRUE) && 
	(sci_ptr != NULL); ) {
#ifdef notdef
    if ( sci_ptr -> skip )
      continue;
    /* end if */
#endif
    /* Compare an entry in the output file with entry in the "sl" list. */
    if (strcmp (file, sci_ptr->name) == 0) {
        /* The're identical, fill in the version number for this element. */
        if ( strstr ( buffer, "(defunct)" ) != NULL ) {
        	ui_print ( VDEBUG, "version %s of file %s is defunct\n",
			revision, file);
        	sci_ptr -> defunct = TRUE;
        } else if ( sci_ptr -> defunct ) {
          	ui_print ( VFATAL, "Contradiction between sandbox state and\n" );
          	ui_print ( VCONT, "information in source control system\n" );
          	ui_print ( VCONT, "regarding defunct status\n" );
          	ui_print ( VCONT, "for file: '%s'\n", sci_ptr -> name );
          	status = ERROR;
        }
	sci_ptr->ver_user = strdup (revision);
	if ( sci_ptr -> ver_user == NULL ) {
	   ui_print ( VFATAL, "strdup of ver_user failed\n" );
	   status = ERROR;
	   break;
	}
	sci_ptr= sci_next(sci_ptr);
    	if (fgets (buffer, sizeof(buffer), tempfile_out_fd) != NULL) {
    		sscanf (buffer, "%s\t%s\n", file, revision);
    		file[strlen(file)-2] = '\0';
	} else {
		keep_looping = FALSE;
	}
    } else if (strcmp (file, sci_ptr->name) > 0) {
        /* The "sl" list and the rcsstat output file are out of sync */
	/* This is probably due to a file not having a revision to match
	   the revision string (i.e. the file doesn't have that revision).
	   Skip this element in the "sl" list. */
	sci_ptr->ver_user = NULL;
        * missing_revs = TRUE;
	sci_ptr = sci_next(sci_ptr); 
	ui_print (VWARN, "Revision information on file %s is not available.\n",
		sci_ptr->name);	
	status = ERROR;
    } else {
	/* Somehow, rcsstat returned more entries then it should. skipt them */
    	if (fgets (buffer, sizeof(buffer), tempfile_out_fd) != NULL) {
    		sscanf (buffer, "%s\t%s\n", file, revision);
    		file[strlen(file)-2] = '\0';
	} else {
		keep_looping = FALSE;
	}
	ui_print (VWARN, "Revision information '%s' was unexpected.\n",
		file);	
	status = ERROR;
    }

  } /* end for */
  fclose (tempfile_out_fd);
  unlink (tempfile_out);
  unlink (tempfile_in);
  rmdir  (tempdir);

  ui_print ( VDEBUG, "Leaving sci_real_fast_lookup_user_rev_list.\n" );
  return ( status );
} /* sci_real_fast_lookup_user_rev_list */

/*
 * FUNCTION sci_real_fast_lookup_latest_rev_list
 *
 * Provide user revisions for each file in a given list and a given set
 */
int
sci_real_fast_lookup_latest_rev_list ( SCI_LIST sl , char * set_name ,
                                       int * missing_revs )
{
  int status = OK, ret;
  BOOLEAN keep_looping;
  SCI_ELEM sci_ptr;
  int pid, fd;
  FILE *tempfile_in_fd;
  char template_file [MAXPATHLEN];
  char tempdir  [MAXPATHLEN];
  char tempfile_in  [MAXPATHLEN];
  char tempfile_out  [MAXPATHLEN];
  char file [MAXPATHLEN];
  char revision [MAXPATHLEN];
  char buffer [MAXPATHLEN];
  char *rev_sw, *file_sw;
  FILE *tempfile_out_fd;
  char dir [MAXPATHLEN];
  BOOLEAN all_defunct = TRUE;

  ui_print ( VDEBUG, "Entering sci_real_fast_lookup_latest_rev_list.\n" );
  if ( ! confirm_alloc ( sl ) )
    return ( ERROR );
  /* if */

  /* Open temporary file. */ 
  strcpy ( template_file, "#srvtmpXXXXXX" );
  getcwd (dir, sizeof(dir));
  opentemp ( template_file , tempdir);
  concat (tempfile_out, sizeof(tempfile_out), dir, "/", tempdir, "/rcsstat_out", NULL);
  concat (tempfile_in, sizeof(tempfile_in), dir, "/", tempdir, "/rcsstat_in", NULL);
  if ((fd = open(tempfile_out, O_WRONLY|O_TRUNC|O_CREAT, 0777)) < 0) { 
	return (ERROR);
  }

  tempfile_in_fd = fopen (tempfile_in, "w");
  for ( sci_ptr = sl -> head; sci_ptr != NULL; sci_ptr = sci_next(sci_ptr)) {
    if ( sci_ptr -> defunct )
      continue;
    all_defunct = FALSE;
    fprintf (tempfile_in_fd, "%s\n", sci_ptr->name);
  }
  fclose (tempfile_in_fd);

  if ( all_defunct ) {
    unlink (tempfile_in);
    rmdir  (tempdir);
    return ( OK );
  } /* if */
  rev_sw  = alloc_switch ('r', set_name);
  file_sw = alloc_switch ('f', tempfile_in);

  pid = fd_runcmd("rcsstat", NULL, TRUE, -1, fd,
	 "rcsstat", "-q", "-V", "-R", rev_sw, file_sw, NULL); 
  close(fd);

  ret = endcmd(pid);
  tempfile_out_fd = fopen (tempfile_out, "r");
  if (fgets (buffer, sizeof(buffer), tempfile_out_fd) == NULL)
	return (ERROR);
  sscanf (buffer, "%s\t%s\n", file, revision);
  file[strlen(file)-2] = '\0';
  * missing_revs = FALSE;
  keep_looping = TRUE;
  for ( sci_ptr = sci_first ( sl ); 
		(keep_looping == TRUE) && (sci_ptr != NULL); ) {
    if ( sci_ptr -> defunct ) {
      sci_ptr= sci_next(sci_ptr);
      continue;
    } /* if */
    if (strcmp (file, sci_ptr->name) == 0) {
	sci_ptr->ver_latest = strdup (revision);
	if ( sci_ptr -> ver_latest == NULL ) {
	   ui_print ( VFATAL, "strdup of sci_ptr -> ver_latest failed.\n" );
           status = ERROR;
           break;
	}
	sci_ptr= sci_next(sci_ptr);
	if (fgets (buffer, sizeof(buffer), tempfile_out_fd) != NULL) {
    		sscanf (buffer, "%s\t%s\n", file, revision);
    		file[strlen(file)-2] = '\0';
	} else {
		keep_looping = FALSE;
	}
    } else if (strcmp (file, sci_ptr->name) > 0) {
        /* The "sl" list and the rcsstat output file are out of sync */
	/* This is probably due to a file not having a revision to match
	   the revision string (i.e. the file doesn't have that revision).
	   Skip this element in the "sl" list. */
	sci_ptr->ver_latest = NULL;
        * missing_revs = TRUE;
	sci_ptr = sci_next(sci_ptr); 
	status = ERROR;
    } else {
	if (fgets (buffer, sizeof(buffer), tempfile_out_fd) != NULL) {
    		sscanf (buffer, "%s\t%s\n", file, revision);
    		file[strlen(file)-2] = '\0';
	} else {
		keep_looping = FALSE;
	}
	status = ERROR;
    }
  }
  fclose (tempfile_out_fd);
  unlink (tempfile_out);
  unlink (tempfile_in);
  rmdir  (tempdir);

  ui_print ( VDEBUG, "Leaving sci_real_fast_lookup_latest_rev_list.\n" );
  return ( status );
} /* sci_real_fast_lookup_latest_rev_list */
