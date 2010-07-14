/* release_cksum_lis.c - the opkg package management system

   Javier Palacios

   Copyright (C) 2010 Javier Palacios

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
*/

#include "config.h"

#include <stdio.h>

#include "release_cksum_list.h"
#include "libbb/libbb.h"


int release_cksum_init(release_cksum_t *cksum, char **itemlist)
//int release_cksum_init(release_cksum_t *cksum, const char *name, int size, const char *value)
{
/*
    cksum->filename = xstrdup(name);
    cksum->filesize = size;
    cksum->value = xstrdup(value);
*/
    cksum->value = xstrdup(*itemlist++);
    cksum->filesize = atoi(*itemlist++);
    cksum->filename = xstrdup(*itemlist++);
    return 0;
}

void release_cksum_deinit(release_cksum_t *cksum)
{
    free (cksum->filename);
    free (cksum->value);
}

void release_cksum_list_init(release_cksum_list_t *list)
{
    void_list_init((void_list_t *) list);
}

void release_cksum_list_deinit(release_cksum_list_t *list)
{
    release_cksum_list_elt_t *iter, *n;
    release_cksum_t *cksum;

    list_for_each_entry_safe(iter, n, &list->head, node) {
      cksum = (release_cksum_t *)iter->data;
      release_cksum_deinit(cksum);

      /* malloced in release_cksum_list_append */
      free(cksum);
      iter->data = NULL;
    }
    void_list_deinit((void_list_t *) list);
}

release_cksum_t *release_cksum_list_append(release_cksum_list_t *list, char **itemlist)
/*
release_cksum_t *release_cksum_list_append(release_cksum_list_t *list,
			       const char *name, int size, const char *value)
*/
{
    /* freed in release_cksum_list_deinit */
    release_cksum_t *cksum = xcalloc(1, sizeof(release_cksum_t));
    release_cksum_init(cksum, itemlist);
    //release_cksum_init(cksum, name, size, value);

    void_list_append((void_list_t *) list, cksum);

    return cksum;
}

