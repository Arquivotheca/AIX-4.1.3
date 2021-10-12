static char sccsid[] = "@(#)10  1.3  src/bos/usr/lpp/bos/migrate/merge_smit_db.c, cmdsmit, bos411, 9436D411a 9/8/94 16:01:34";
/*
 *   COMPONENT_NAME: CMDSMIT
 *
 *   FUNCTIONS: main
 *		merge_menu_opts
 *		merge_name_hdrs
 *		merge_cmd_hdrs
 *		merge_cmd_opts
 *		add_to_deletions_table
 *		delete_from_table
 *		hashstring
 *		remove_old_database
 *		fatal
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <csmitlocal.h>
#include <limits.h>

/* Obsolesced keys for each of the tables */
#include "menu_opt.obs.h"
#include "name_hdr.obs.h"
#include "cmd_hdr.obs.h"
#include "cmd_opt.obs.h"

/* Definitions to tell difference between database entries in hash table */
#define MENU_OPT_TABLE 1
#define NAME_HDR_TABLE 2
#define CMD_HDR_TABLE  3
#define CMD_OPT_TABLE  4

/* Routines to merge the tables between the two databases */
extern int merge_menu_opts(char *, char *);
extern int merge_name_hdrs(char *, char *);
extern int merge_cmd_hdrs(char *, char *);
extern int merge_cmd_opts(char *, char *);

/* Routines to manage the deletions hash table */
extern int delete_from_table ( char *, char *, char *, int );
extern int add_to_deletions_table ( char *, char *, char *, int );
extern int create_deletions_table ();
extern unsigned int hashstring ( char *, int );

/* Miscellaneous routines */
extern void fatal();
extern void remove_old_database();

/* hash table structure */
#define HASHTABLESIZE 2048
struct hashtab {
   char *key1;
   char *key2;
   char *key3;
   int  tabletype;
   struct hashtab *next;
} deletions[HASHTABLESIZE];

/* Directories for source and target databases. */
char *source_dir = NULL;
char *target_dir = NULL;

/* On a 3.2->4.1 migration, remove obsolesced entries */
int process_obsolesced = 0;

/*
 * main - Merge smit databases from two directories by adding those entries
 *	  from the source database which do not exist in the target database,
 *	  omitting obsolesced entries as defined in the <table>.obs.h files
 *	  if this is a 3.2->4.1 migration.
 *
 *        This function is expected to be called from a BOS migration install
 *	  environment, so BOSLEVEL is expected to indicate the level of the
 *	  system being migrated from.
 *
 * Arguments:
 *       argv[1] -- directory where source database is located
 *       argv[2] -- directory where target database is located
 *
 * Returns: 0 on success, 1 on failure
 *
 */

main (int argc, char **argv)
{
    char *migration_level = NULL;
    int errors = 0;

    /* Set source and target directories */
    if (argc != 3) {
	fprintf(stderr,"Usage: merge_smit_dbs sourcedir targetdir\n");
	exit(1);
    } else {
	source_dir = argv[1];
	target_dir = argv[2];
    }

    /* Check to see if this is a 3.2->4.1 migration */
    if (((migration_level = getenv ("BOSLEVEL")) != NULL) &&
	 (!strcmp(migration_level,"3.2")))
    	    process_obsolesced = 1;

    /* Add non-obsolesced entries not already in the target database */
    errors += merge_menu_opts(source_dir, target_dir);
    errors += merge_name_hdrs(source_dir, target_dir);
    errors += merge_cmd_hdrs(source_dir, target_dir);
    errors += merge_cmd_opts(source_dir, target_dir);

    /* Report any failures */
    if (errors > 0) {
    	fatal();
    } else {
	/* Clean out old database upon success */
	remove_old_database();
    	exit(0);
    }

}

/*
 * Function: merge_menu_opts
 *	  Merge the source menu_opts into the target menu_opts
 *
 * Arguments:
 *       odmold  -- directory where source database is located
 *       odmnew  -- directory where target database is located
 *
 * Returns: 0 on success, 1 on failure
 *
 */

int merge_menu_opts(char * odmold, char * odmnew )
{
    struct Class *sm_menu_opt_class_old;
    struct Class *sm_menu_opt_class_new;
    struct sm_menu_opt *menu_opt_old = NULL;
    struct sm_menu_opt *menu_opt_new = NULL;
    struct listinfo menu_opt_info_old;
    struct listinfo menu_opt_info_new;
    int index = 0;
    int numentries = 0;
    int errors = 0;


    /* Read in target table and add to hash table */
    odm_set_path(odmnew);

    sm_menu_opt_class_new = odm_mount_class("sm_menu_opt");

    menu_opt_new = get_sm_menu_opt_list (sm_menu_opt_class_new,NULL,
   					&menu_opt_info_new, 512, 1);

    /* Add entries to the deletions table */
    for (index = 0; index < menu_opt_info_new.num; index++) {
	if (add_to_deletions_table(menu_opt_new[index].id,
			           menu_opt_new[index].next_id, 
			           "",
			           MENU_OPT_TABLE) != 0) {
					errors++;
					break;
	}
    }

    /* If 3.2->4.1 migration, add obsolete entries to deletions table */

    if ((!errors) && (process_obsolesced)) {
       /* Add entries to the hash table */
          for (index = 0; index < NUM_OBS_MENUS; index++) {
	      if (add_to_deletions_table(obs_menus[index].id,
			                 obs_menus[index].next_id, 
			                 "",
			                 MENU_OPT_TABLE) != 0) {
					    errors++;
					    break;
	      }
          }
    }

    /* Read in source table and add to target table if not in hash table */
    if (!odm_set_path(odmold))
	  errors++;

    sm_menu_opt_class_old = odm_mount_class("sm_menu_opt");

    /* Bail out if we've had any errors processing to this point */

    if (errors) {
        odm_free_list(menu_opt_new, &menu_opt_info_new);
        odm_close_class(sm_menu_opt_class_new);
	fatal();
    }

    menu_opt_old = get_sm_menu_opt_list (sm_menu_opt_class_old,NULL,
   					&menu_opt_info_old, 512, 1);
					
    /* Add the entry to the target database if it is not found
     * in the deletions table.
     */
    odm_set_path(odmnew);
    sm_menu_opt_class_new = odm_mount_class("sm_menu_opt");

    for (index = 0; index < menu_opt_info_old.num; index++) {
	if (!delete_from_table(menu_opt_old[index].id,
			       menu_opt_old[index].next_id, 
			       "",
			       MENU_OPT_TABLE)) {
	     if (process_obsolesced) {
		menu_opt_old[index].text_msg_file = NULL;
		menu_opt_old[index].help_msg_id[0] = '\0';
		menu_opt_old[index].help_msg_loc = NULL;
	     }
	     if (odm_add_obj(sm_menu_opt_class_new,&menu_opt_old[index]) == -1)
	     {
		  /* Failed to add entry successfully */
		   errors++;
		   break;
	     }
        }
    }

    /* Done with these lists, so free up the memory */
    odm_free_list(menu_opt_new, &menu_opt_info_new);
    odm_free_list(menu_opt_old, &menu_opt_info_old);

    /* Done with these tables, so free up the memory */
    odm_close_class(sm_menu_opt_class_new);
    odm_close_class(sm_menu_opt_class_old);

    return( errors );

}

/*
 * Function: merge_name_hdrs
 *	  Merge the source name_hdrs into the target name_hdrs
 *
 * Arguments:
 *       odmold  -- directory where source database is located
 *       odmnew  -- directory where target database is located
 *
 * Returns: 0 on success, 1 on failure
 *
 */

int merge_name_hdrs(char * odmold, char * odmnew )
{
    struct Class *sm_name_hdr_class_old;
    struct Class *sm_name_hdr_class_new;
    struct sm_name_hdr *name_hdr_old = NULL;
    struct sm_name_hdr *name_hdr_new = NULL;
    struct listinfo name_hdr_info_old;
    struct listinfo name_hdr_info_new;
    int index = 0;
    int numentries = 0;
    int errors = 0;


    /* Read in target table and add to hash table */
    odm_set_path(odmnew);

    sm_name_hdr_class_new = odm_mount_class("sm_name_hdr");

    name_hdr_new = get_sm_name_hdr_list (sm_name_hdr_class_new,NULL,
   					&name_hdr_info_new, 256, 1);

    /* Add entries to the deletions table */
    for (index = 0; index < name_hdr_info_new.num; index++) {
	if (add_to_deletions_table(name_hdr_new[index].id,
			           name_hdr_new[index].next_id, 
			           name_hdr_new[index].option_id, 
			           NAME_HDR_TABLE) != 0) {
					errors++;
					break;
	}
    }

    /* If 3.2->4.1 migration, add obsolete entries to deletions table */

    if ((!errors) && (process_obsolesced)) {
       /* Add entries to the hash table */
          for (index = 0; index < NUM_OBS_NAME_HDRS; index++) {
	      if (add_to_deletions_table(obs_name_hdrs[index].id,
			                 obs_name_hdrs[index].next_id, 
			                 obs_name_hdrs[index].option_id, 
			                 NAME_HDR_TABLE) != 0) {
					    errors++;
					    break;
	      }
          }
    }

    /* Read in source table and add to target table if not in hash table */
    if (!odm_set_path(odmold))
	  errors++;

    sm_name_hdr_class_old = odm_mount_class("sm_name_hdr");

    /* Bail out if we've had any errors processing to this point */

    if (errors) {
        odm_free_list(name_hdr_new, &name_hdr_info_new);
        odm_close_class(sm_name_hdr_class_new);
	fatal();
    }

    name_hdr_old = get_sm_name_hdr_list (sm_name_hdr_class_old,NULL,
   					&name_hdr_info_old, 256, 1);
					
    /* Add the entry to the target database if it is not found
     * in the deletions table.
     */
    odm_set_path(odmnew);
    sm_name_hdr_class_new = odm_mount_class("sm_name_hdr");

    for (index = 0; index < name_hdr_info_old.num; index++) {
	if (!delete_from_table(name_hdr_old[index].id,
			       name_hdr_old[index].next_id, 
			       name_hdr_old[index].option_id, 
			       NAME_HDR_TABLE)) {
	     if (process_obsolesced) {
		name_hdr_old[index].name_msg_file = NULL;
		name_hdr_old[index].help_msg_id[0] = '\0';
		name_hdr_old[index].help_msg_loc = NULL;
	     }
	     if (odm_add_obj(sm_name_hdr_class_new,&name_hdr_old[index]) == -1)
	     {
		  /* Failed to add entry successfully */
		   errors++;
		   break;
	     }
        }
    }

    /* Done with these lists, so free up the memory */
    odm_free_list(name_hdr_new, &name_hdr_info_new);
    odm_free_list(name_hdr_old, &name_hdr_info_old);


    /* Done with these tables, so free up the memory */
    odm_close_class(sm_name_hdr_class_new);
    odm_close_class(sm_name_hdr_class_old);

    return( errors );

}

/*
 * Function: merge_cmd_hdrs
 *	  Merge the source cmd_hdrs into the target cmd_hdrs
 *
 * Arguments:
 *       odmold  -- directory where source database is located
 *       odmnew  -- directory where target database is located
 *
 * Returns: 0 on success, 1 on failure
 *
 */

int merge_cmd_hdrs(char * odmold, char * odmnew )
{
    struct Class *sm_cmd_hdr_class_old;
    struct Class *sm_cmd_hdr_class_new;
    struct sm_cmd_hdr *cmd_hdr_old = NULL;
    struct sm_cmd_hdr *cmd_hdr_new = NULL;
    struct listinfo cmd_hdr_info_old;
    struct listinfo cmd_hdr_info_new;
    int index = 0;
    int numentries = 0;
    int errors = 0;


    /* Read in target table and add to hash table */
    odm_set_path(odmnew);

    sm_cmd_hdr_class_new = odm_mount_class("sm_cmd_hdr");

    cmd_hdr_new = get_sm_cmd_hdr_list (sm_cmd_hdr_class_new,NULL,
   					&cmd_hdr_info_new, 512, 1);

    /* Add entries to the deletions table */
    for (index = 0; index < cmd_hdr_info_new.num; index++) {
	if (add_to_deletions_table(cmd_hdr_new[index].id,
			           cmd_hdr_new[index].option_id,
			           "",
			           CMD_HDR_TABLE) != 0) {
					errors++;
					break;
	}
    }

    /* If 3.2->4.1 migration, add obsolete entries to deletions table */

    if ((!errors) && (process_obsolesced)) {
       /* Add entries to the hash table */
          for (index = 0; index < NUM_OBS_CMD_HDRS; index++) {
	      if (add_to_deletions_table(obs_cmd_hdrs[index].id,
			                 obs_cmd_hdrs[index].option_id, 
			                 "",
			                 CMD_HDR_TABLE) != 0) {
					    errors++;
					    break;
	      }
          }
    }

    /* Read in source table and add to target table if not in hash table */
    if (!odm_set_path(odmold))
	  errors++;

    sm_cmd_hdr_class_old = odm_mount_class("sm_cmd_hdr");

    /* Bail out if we've had any errors processing to this point */

    if (errors) {
        odm_free_list(cmd_hdr_new, &cmd_hdr_info_new);
        odm_close_class(sm_cmd_hdr_class_new);
	fatal();
    }

    cmd_hdr_old = get_sm_cmd_hdr_list (sm_cmd_hdr_class_old,NULL,
   					&cmd_hdr_info_old, 512, 1);
					
    /* Add the entry to the target database if it is not found
     * in the deletions table.
     */
    odm_set_path(odmnew);
    sm_cmd_hdr_class_new = odm_mount_class("sm_cmd_hdr");

    for (index = 0; index < cmd_hdr_info_old.num; index++) {
	if (!delete_from_table(cmd_hdr_old[index].id,
			       cmd_hdr_old[index].option_id, 
			       "",
			       CMD_HDR_TABLE)) {
	     if (process_obsolesced) {
		cmd_hdr_old[index].name_msg_file = NULL;
		cmd_hdr_old[index].help_msg_id[0] = '\0';
		cmd_hdr_old[index].help_msg_loc = NULL;
	     }
	     if (odm_add_obj(sm_cmd_hdr_class_new,&cmd_hdr_old[index]) == -1)
	     {
		  /* Failed to add entry successfully */
		   errors++;
		   break;
	     }
        }
    }

    /* Done with these lists, so free up the memory */
    odm_free_list(cmd_hdr_new, &cmd_hdr_info_new);
    odm_free_list(cmd_hdr_old, &cmd_hdr_info_old);


    /* Done with these tables, so free up the memory */
    odm_close_class(sm_cmd_hdr_class_new);
    odm_close_class(sm_cmd_hdr_class_old);

    return( errors );

}

/*
 * Function: merge_cmd_opts
 *	  Merge the source cmd_opts into the target cmd_opts
 *
 * Arguments:
 *       odmold  -- directory where source database is located
 *       odmnew  -- directory where target database is located
 *
 * Returns: 0 on success, 1 on failure
 *
 */

int merge_cmd_opts(char * odmold, char * odmnew )
{
    struct Class *sm_cmd_opt_class_old;
    struct Class *sm_cmd_opt_class_new;
    struct sm_cmd_opt *cmd_opt_old = NULL;
    struct sm_cmd_opt *cmd_opt_new = NULL;
    struct listinfo cmd_opt_info_old;
    struct listinfo cmd_opt_info_new;
    int index = 0;
    int numentries = 0;
    int errors = 0;


    /* Read in target table and add to hash table */
    odm_set_path(odmnew);

    sm_cmd_opt_class_new = odm_mount_class("sm_cmd_opt");

    cmd_opt_new = get_sm_cmd_opt_list (sm_cmd_opt_class_new,NULL,
   					&cmd_opt_info_new, 512, 1);

    /* Add entries to the deletions table */
    for (index = 0; index < cmd_opt_info_new.num; index++) {
	if (add_to_deletions_table(cmd_opt_new[index].id,
			           "", 
			           "",
			           CMD_OPT_TABLE) != 0) {
					errors++;
					break;
	}
    }

    /* If 3.2->4.1 migration, add obsolete entries to deletions table */

    if ((!errors) && (process_obsolesced)) {
       /* Add entries to the hash table */
          for (index = 0; index < NUM_OBS_CMDS; index++) {
	      if (add_to_deletions_table(obs_cmds[index].id,
			                 "", 
			                 "",
			                 CMD_OPT_TABLE) != 0) {
					    errors++;
					    break;
	      }
          }
    }

    /* Read in source table and add to target table if not in hash table */
    if (!odm_set_path(odmold))
	  errors++;

    sm_cmd_opt_class_old = odm_mount_class("sm_cmd_opt");

    /* Bail out if we've had any errors processing to this point */

    if (errors) {
        odm_free_list(cmd_opt_new, &cmd_opt_info_new);
        odm_close_class(sm_cmd_opt_class_new);
	fatal();
    }

    cmd_opt_old = get_sm_cmd_opt_list (sm_cmd_opt_class_old,NULL,
   					&cmd_opt_info_old, 512, 1);
					
    /* Add the entry to the target database if it is not found
     * in the deletions table.
     */
    odm_set_path(odmnew);
    sm_cmd_opt_class_new = odm_mount_class("sm_cmd_opt");

    for (index = 0; index < cmd_opt_info_old.num; index++) {
	if (!delete_from_table(cmd_opt_old[index].id,
			       "", 
			       "",
			       CMD_OPT_TABLE)) {
	     if (process_obsolesced) {
		cmd_opt_old[index].name_msg_file = NULL;
		cmd_opt_old[index].help_msg_id[0] = '\0';
		cmd_opt_old[index].help_msg_loc = NULL;
	     }
	     if (odm_add_obj(sm_cmd_opt_class_new,&cmd_opt_old[index]) == -1)
	     {
		  /* Failed to add entry successfully */
		   errors++;
		   break;
	     }
        }
    }

    /* Done with these lists, so free up the memory */
    odm_free_list(cmd_opt_new, &cmd_opt_info_new);
    odm_free_list(cmd_opt_old, &cmd_opt_info_old);


    /* Done with these tables, so free up the memory */
    odm_close_class(sm_cmd_opt_class_new);
    odm_close_class(sm_cmd_opt_class_old);

    return( errors );

}


/*
 * Function: add_to_deletions_table
 *	  Add an entry from the target database or the obsolecence table
 *	  to the deletions table.  If the entries from the old table match
 *	  the entries from the deletions table, then they will not be added
 *	  to the target database.
 *
 *	  The hash is computed simply on the characters in the first key and
 *	  the index of the table that it belongs to.
 *
 *	  sm_menu_opts has two keys:  id and next_id
 *	  sm_name_hdr has three keys: id, next_id and option_id
 *	  sm_cmd_hdr has two keys:    id and option_id
 *	  sm_cmd_opt has one key:     id
 *
 * Arguments:
 *       key1 - id for all tables
 *       key2 - next_id for sm_menu_opts & sm_name_hdr, option_id for sm_cmd_hdr
 *       key3 - option_id for sm_name_hdr
 *       table - table index (menu_opt=1, name_hdr=2, cmd_hdr=3, cmd_opt=4)
 *
 * Returns: 0 if entry successfully added, 1 if it fails
 *
 */

int
add_to_deletions_table ( char *key1, char *key2, char *key3, int table )
{
   struct hashtab *newentry;
   unsigned int hashval;
   char *newline;

   /* Input lists are supposed to be unique, so don't search for 
    * existence before adding.  Just add at the front of the list.
    * Actually, add right into the header for the first entry, then
    * add right after the header thereafter.
    */
   hashval = hashstring(key1, table);

   if (deletions[hashval].tabletype == 0) { /* No entries can have table == 0 */
       deletions[hashval].key1 = key1;
       deletions[hashval].key2 = key2;
       deletions[hashval].key3 = key3;
       deletions[hashval].tabletype = table;
   } else { /* Add right after header */
       newentry = (struct hashtab *) malloc (sizeof (struct hashtab));
       if (newentry != (struct hashtab *) NULL) {
       	   newentry->key1 = key1;
       	   newentry->key2 = key2;
       	   newentry->key3 = key3;
       	   newentry->tabletype = table;
       	   newentry->next = deletions[hashval].next;
       	   deletions[hashval].next = newentry;
       } else {
           /* Out of memory */
           fatal();
       }
   }

   return 0;
}

/*
 * Function: delete_from_table
 *	  Determine if an entry from the source table already exists in
 *	  the target table or in the obsolesed table (if migrating from
 *	  3.2.) 
 *
 *	  sm_menu_opts has two keys:  id and next_id
 *	  sm_name_hdr has three keys: id, next_id and option_id
 *	  sm_cmd_hdr has two keys:    id and option_id
 *	  sm_cmd_opt has one key:     id
 *
 * Arguments:
 *       key1 - id for all tables
 *       key2 - next_id for sm_menu_opts & sm_name_hdr, option_id for sm_cmd_hdr
 *       key3 - option_id for sm_name_hdr
 *
 * Returns: 1 if entry should be deleted, 0 is entry should be added
 *
 */

int
delete_from_table ( char *key1, char *key2, char *key3, int db )
{
   struct hashtab *search;
   
   for (search = &deletions[hashstring(key1, db)];
        search != (struct hashtab *) NULL;
	search = search->next) {
	   /* Make sure we match the right kind of entry */
	   if (db != search->tabletype)
		continue;
           if (!strcmp(key1,search->key1)) {
	      if (db == CMD_OPT_TABLE) {  /* 1 key needed to match */
		  return(1);
	      } else {
		  if (!strcmp(key2,search->key2)) {
		      if (db != NAME_HDR_TABLE) {	/* 2 keys to match */
		  	  return(1);
		      } else { 
		  	    if (!strcmp(key3,search->key3)) { /*3 keys*/
		  	  	return(1);
			    }
		      }
		  }
	      }
	   }
   }
   return 0; /* No match */
}

/*
 * Function: hashstring
 *	  Computes a hash index based upon the id, the table id,
 *	  and the size of the hash table.
 *
 * Arguments:
 *       string - id for all tables
 *       table - table index (menu_opt=1, name_hdr=2, cmd_hdr=3, cmd_opt=4)
 *
 * Returns: hashvalue
 *
 */

unsigned int
hashstring ( char *string, int table )
{
   char *c = string;
   unsigned int hash = 0;

   if (string == NULL) {
       return table;
   }
   while (*c != '\0') {
       hash += (unsigned int) *c++;
   }
   hash = (hash + table) % HASHTABLESIZE;
   return hash;
}

/*
 * Function: remove_old_database
 *	  Upon successful return from merging the databases, remove the
 *	  old SMIT database.
 *
 * Arguments: none
 *
 * Returns: nothing
 *
 */
void
remove_old_database ( )
{
    char filebuffer[PATH_MAX+1];
    char *databases[] = {"/sm_menu_opt",
			 "/sm_name_hdr",
			 "/sm_cmd_hdr",
			 "/sm_cmd_opt"};
    int  i;

    /* Remove the old database files ($source_dir/database[.vc]) */
    for (i = 0; i < 4; i++) {
	strcpy(filebuffer, source_dir);
	strcat(filebuffer, databases[i]);
	unlink(filebuffer);

	strcat(filebuffer, ".vc");
	unlink(filebuffer);
    }
}

/*
 * Function: fatal
 *	  Exit upon a fatal error.  Emit message indicating that smit 
 *	  databases were unsuccesfully merged.
 *
 * Arguments: none
 *
 * Returns: nothing
 *
 */
void
fatal ( )
{
	fprintf(stderr,"Unable to continue merging SMIT databases.\n");
	fprintf(stderr,"Saved SMIT databases retained in %s.\n", source_dir);
	exit(1);
}
