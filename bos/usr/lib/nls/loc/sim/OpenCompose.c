/* @(#)16	1.1  src/bos/usr/lib/nls/loc/sim/OpenCompose.c, cmdims, bos411, 9428A410j 7/8/93 14:40:40 */
/*
 * COMPONENT_NAME : (CMDIMS) SBCS Input Method
 *
 * FUNCTIONS : Dynamic Composing Input Method (DIM)
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*----------------------------------------------------------------------*
 *	Include Files
 *----------------------------------------------------------------------*/
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <im.h>
#include "dim.h"

#define	DEFAULT_COMPOSE		"/usr/lib/nls/loc/sbcs.imcompose"
#define	COMPOSE_POSTFIX		".imcompose"

/*----------------------------------------------------------------------*
 *	read compiled compose definition
 *----------------------------------------------------------------------*/
static int	ReadCompose(		/* sccess -> 0  error -> -1 */
	char		*file,
	DIMFep          dimfep)
{
	struct stat	buff;
	unsigned int	tmp[6], table_size, switch_size;
	unsigned char	*compose, *result_string, *malloc();
	ComposeTable	*compose_head;
	int		fd, i;

	/* open compose definition file */
	if((fd = open(file, O_RDONLY)) < 0){
		return(-1);
	}

	/* get file size */
	if(fstat(fd, &buff) < 0){
		return(-1);
	}

	/* if file size is 0, do nothing */
	if(buff.st_size == 0){
		return 0;
	}

	/* check file size */
	if(buff.st_size < sizeof(tmp)){
		return(-1);
	}

	/* allocate memory area and read the compose definition */
	if((compose = malloc(buff.st_size)) == NULL){
		return(-1);
	}
	if(read(fd, compose, buff.st_size) != buff.st_size){
		free(compose);
		return(-1);
	}
	close(fd);

	/* check DIM version */
	memcpy(tmp, compose, sizeof(tmp));
	if(dimfep->version != tmp[0]){
		free(compose);
		return(-1);
	}

	/* check data */
	table_size = tmp[1] * sizeof(ComposeTable);
	switch_size = tmp[3] * sizeof(LayerSwitch);
	if(buff.st_size != sizeof(tmp) + table_size
				+ tmp[2] + switch_size + tmp[5]){
		free(compose);
		return(-1);
	}

	/* set the data to IMFep */
	dimfep->compose = compose;
	compose += sizeof(tmp);
        if(tmp[1] != 0){
		compose_head = (ComposeTable *)compose;
		dimfep->compose_table = compose_head;
        }else{
		dimfep->compose_table = NULL;
	}
	compose += table_size;
	result_string = compose;
	compose += tmp[2];
	dimfep->layer_switch_num = tmp[3];
        if(tmp[3] != 0){
		dimfep->layer_switch = (LayerSwitch *)compose;
	}else{
		dimfep->layer_switch = NULL;
	}
	compose += switch_size;
	dimfep->compose_error = tmp[4];
        if(tmp[5] != 0){
		dimfep->compose_error_str = compose;
	}else{
		dimfep->compose_error_str = NULL;
	}

	/* convert from array index into address */
	for(i = 0; i < tmp[1]; i++){
		if(compose_head[i].result_string == (unsigned char *)(-1)){
			compose_head[i].result_string = NULL;
		}else{
			compose_head[i].result_string =
			&result_string[(int)compose_head[i].result_string];
		}
		if(compose_head[i].brother == (ComposeTable *)0){
			compose_head[i].brother = NULL;
		}else{
			compose_head[i].brother =
				&compose_head[(int)compose_head[i].brother];
		}
		if(compose_head[i].child == (ComposeTable *)0){
			compose_head[i].child = NULL;
		}else{
			compose_head[i].child =
				&compose_head[(int)compose_head[i].child];
		}
	}

	return 0;
}


/*----------------------------------------------------------------------*
 *	search and read compiled compose definition
 *----------------------------------------------------------------------*/
int	OpenCompose(			/* sccess -> 0  error -> -1 */
	IMLanguage	lang,
	DIMFep          dimfep)
{
	char	*home, *fullpath, *getenv();
	char	file[2048];

	/* $HOEM/.imcompose */
	if((home = getenv("HOME")) != NULL){
		sprintf(file, "%s/%s/%s", home, lang, COMPOSE_POSTFIX);
		if(ReadCompose(file, dimfep) == 0){
			return 0;
		}
	}

	/* $LOCPATH/lang.imcompose */
	fullpath = _IMAreYouThere(lang, COMPOSE_POSTFIX);
	if(fullpath != NULL){
		if(ReadCompose(fullpath, dimfep) == 0){
			free(fullpath);
			return 0;
		}
		free(fullpath);
	}

	/* default : /usr/lib/nls/loc/sbcs.imcompose */
	if(ReadCompose(DEFAULT_COMPOSE, dimfep) == 0){
		return 0;
	}

	return(-1);
}
