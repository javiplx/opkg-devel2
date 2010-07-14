/* dist_src_list.c - the opkg package management system

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

#include <ctype.h>

#include "dist_src_list.h"
#include "void_list.h"
#include "libbb/libbb.h"

int dist_src_init(dist_src_t *dist, const char *name, const char *base_url, const char *extra_data, int plain)
{
    dist->name = xstrdup(name);
    dist->value = xstrdup(base_url);
    dist->extra_data = parse_simple_list( extra_data );
    dist->plain = plain;
    if ( dist->extra_data == NULL ) {
        opkg_msg(ERROR, "Problems while processing enty for dist %s. ", name);
        return 1;
        }
    return 0;
}

void dist_src_deinit(dist_src_t *dist)
{
    free(dist->name);
    free(dist->value);
    simple_list_deinit(dist->extra_data);
}

void dist_src_list_init(dist_src_list_t *list)
{
    void_list_init((void_list_t *) list);
}

void dist_src_list_deinit(dist_src_list_t *list)
{
    dist_src_list_elt_t *iter, *n;
    dist_src_t *dist_src;

    list_for_each_entry_safe(iter, n, &list->head, node) {
      dist_src = (dist_src_t *)iter->data;
      dist_src_deinit(dist_src);

      /* malloced in dist_src_list_append */
      free(dist_src);
      iter->data = NULL;
    }
    void_list_deinit((void_list_t *) list);
}

dist_src_t *dist_src_list_append(dist_src_list_t *list,
			       const char *name, const char *base_url, const char *extra_data,
			       int plain)
{
    /* freed in dist_src_list_deinit */
    dist_src_t *dist_src = xcalloc(1, sizeof(dist_src_t));
    dist_src_init(dist_src, name, base_url, extra_data, plain);

    void_list_append((void_list_t *) list, dist_src);

    return dist_src;
}

char **parse_simple_list(const char *raw) {
    int n = 0;

    char **list = (char **) xcalloc(n+1, sizeof(char *));
    if (list == NULL) {
        return NULL;
    }
    *list = NULL;

    char *c0 = (char *) raw;
    while ( *raw != '\0' ) {
        if (isspace(*raw)) {
            *(list+n) = xstrndup(c0, raw-c0);
            list = (char **) xrealloc(list, (++n+1) * sizeof (char *));
            *(list+n) = NULL;
            while(isspace(*raw))
                raw++;
            c0 = (char *) raw;
            raw--;
        }
        raw++;
    }
    *(list+n) = xstrndup( c0 , raw-c0 );
    list = (char **) xrealloc (list , (++n+1) * sizeof (char *));
    *(list+n) = NULL;

    return list;
}

void simple_list_deinit(char **list) {
    char **l = list;
    if (list != NULL )
        while (*list != NULL )
            free(*list++);
    free(l);
}
