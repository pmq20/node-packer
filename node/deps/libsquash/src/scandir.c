/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "squash.h"
#include <stdlib.h>


typedef int(*qsort_compar)(const void *, const void *);

int squash_scandir(sqfs *fs, const char *dirname, struct SQUASH_DIRENT ***namelist,
	int (*select)(const struct SQUASH_DIRENT *),
	int (*compar)(const struct SQUASH_DIRENT **, const struct SQUASH_DIRENT **))
{
	SQUASH_DIR * openeddir = 0;
	size_t n = 0;
	struct SQUASH_DIRENT **list = NULL;
	struct SQUASH_DIRENT *ent = 0 ,*p = 0;

	if((dirname == NULL) || (namelist == NULL))
		return -1;

	openeddir = squash_opendir(fs, dirname);
	if(openeddir == NULL)
		return -1;


	list = (struct SQUASH_DIRENT **)malloc(MAX_DIR_ENT*sizeof(struct SQUASH_DIRENT *));


	while(( ent = squash_readdir(openeddir)) != NULL)
	{
		if( select && !select(ent))
			continue;

		p = (struct SQUASH_DIRENT *)malloc(sizeof(struct SQUASH_DIRENT));

		memcpy((void *)p,(void *)ent,sizeof(struct SQUASH_DIRENT));
		list[n] = p;

		n++;
		if(n >= MAX_DIR_ENT)
			break;

	}

	//close the squash_dir
	squash_closedir(openeddir);

	//realloc the array
	*namelist = realloc((void *)list,n*sizeof(struct SQUASH_DIRENT *));
	if(*namelist == NULL)
		*namelist = list;


	//sort the array
	if(compar)
		qsort((void *)*namelist,n,sizeof(struct SQUASH_DIRENT *),(qsort_compar)compar);

	return n;

}