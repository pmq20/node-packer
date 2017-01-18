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

int squash_scandir(sqfs *fs, const char *dirname, struct dirent ***namelist,
	int (*select)(const struct dirent *),
	int (*compar)(const struct dirent **, const struct dirent **))
{
	SQUASH_DIR * openeddir = 0;
	size_t n = 0;
	struct dirent **list = NULL;
	struct dirent *ent = 0 ,*p = 0;

	if((dirname == NULL) || (namelist == NULL))
		return -1;

	openeddir = squash_opendir(fs, dirname);
	if(openeddir == NULL)
		return -1;


	list = (struct dirent **)malloc(MAX_DIR_ENT*sizeof(struct dirent *));


	while(( ent = squash_readdir(openeddir)) != NULL)
	{
		if( select && !select(ent))
			continue;

		p = (struct dirent *)malloc(sizeof(struct dirent));

		memcpy((void *)p,(void *)ent,sizeof(struct dirent));
		list[n] = p;

		n++;
		if(n >= MAX_DIR_ENT)
			break;

	}

	//close the squash_dir
	squash_closedir(openeddir);

	//realloc the array
	*namelist = realloc((void *)list,n*sizeof(struct dirent *));
	if(*namelist == NULL)
		*namelist = list;


	//sort the array
	if(compar)
		qsort((void *)*namelist,n,sizeof(struct dirent *),(qsort_compar)compar);

	return n;

}