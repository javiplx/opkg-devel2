/* release.c - the opkg package management system

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
#include <unistd.h>

#include "release.h"
#include "libopkg/opkg_download.h"
#include "sprintf_alloc.h"
#include "file_util.h"
#include "dist_src_list.h"

#include "opkg_utils.h"

#include "libbb/libbb.h"


static void
release_init(release_t *release)
{
     release->name = NULL;
     release->datestring = NULL;
     release->architectures = NULL;
     release->components = NULL;
     release->md5sums = NULL;
#if defined HAVE_SHA256
     release->sha256sums = NULL;
#endif
}

release_t *
release_new(void)
{
     release_t *release;

     release = xcalloc(1, sizeof(release_t));
     release_init(release);

     return release;
}

void
release_deinit(release_t *release)
{
    free(release->name);
    free(release->datestring);
    simple_list_deinit(release->architectures);
    simple_list_deinit(release->components);
    release_cksum_list_deinit(release->md5sums);
#if defined HAVE_SHA256
    release_cksum_list_deinit(release->sha256sums);
#endif
}

static int
is_release_field(const char *type, const char *line)
{
        if (!strncmp(line, type, strlen(type)))
                return 1;
        return 0;
}

static char *
release_parse_simple(const char *type, const char *line)
{
	return trim_xstrdup(line + strlen(type) + 2);
}

static char **
release_parse_list(const char *type, const char *line)
{
	return parse_simple_list(line + strlen(type) + 2);
}

static int
release_parse_line(release_t *release, const char *line)
{
	static int reading_md5sums = 0;
#if defined HAVE_SHA256
	static int reading_sha256sums = 0;
#endif
	int ret = 0;

	switch (*line) {
	case 'A':
		if (is_release_field("Architectures", line)) {
			release->architectures = release_parse_list("Architectures", line);
		}
		break;

	case 'C':
		if (is_release_field("Codename", line)) {
			release->name = release_parse_simple("Codename", line);
	    	}
		else if (is_release_field("Components", line)) {
			release->components = release_parse_list("Components", line);
	    	}
		break;

	case 'D':
		if (is_release_field("Date", line)) {
			release->datestring = release_parse_simple("Date", line);
		}
		break;

	case 'M':
		if (is_release_field("MD5sum", line)) {
			reading_md5sums = 1;
			if (release->md5sums == NULL) {
			     release->md5sums = xcalloc(1, sizeof(release_cksum_list_t));
			     release_cksum_list_init(release->md5sums);
			}
			goto dont_reset_flags;
	    	}
		break;

#if defined HAVE_SHA256
	case 'S':
		if (is_release_field("SHA256", line)) {
			reading_sha256sums = 1;
			if (release->sha256sums == NULL) {
			     release->sha256sums = xcalloc(1, sizeof(release_cksum_list_t));
			     release_cksum_list_init(release->sha256sums);
			}
			goto dont_reset_flags;
	    	}
		break;
#endif

	case ' ':
		if (reading_md5sums) {

			char **list = parse_simple_list(line+1);
			release_cksum_list_append(release->md5sums, list);
			//release_cksum_list_append(release->md5sums, *(list+2), atoi(*(list+1)), *list);
			simple_list_deinit(list);

			goto dont_reset_flags;
		}
#if defined HAVE_SHA256
		else if (reading_sha256sums) {

			char **list = parse_simple_list(line+1);
			release_cksum_list_append(release->sha256sums, list);
			//release_cksum_list_append(release->sha256sums, *(list+2), atoi(*(list+1)), *list);
			simple_list_deinit(list);

			goto dont_reset_flags;
		}
#endif
		break;

	default:
		ret = 1;
	}

	reading_md5sums = 0;
	reading_sha256sums = 0;

dont_reset_flags:

	return ret;
}

int
release_parse_from_stream(release_t *release, FILE *fp)
{
	int ret = 0;
	char *buf = NULL;
	size_t buflen, nread;

	nread = getline(&buf, &buflen, fp);
	while ( nread != -1 ) {
		if (buf[nread-1] == '\n') buf[nread-1] = '\0';
		if (release_parse_line(release, buf))
                        opkg_msg(ERROR, "Failed to parse release line for %s:\n\t%s\n",
					release->name, buf);
		nread = getline(&buf, &buflen, fp);
	}

	if (!feof(fp)) {
		opkg_perror(ERROR, "Problems reading Release file for %sd\n", release->name);
		ret = -1;
	}

	return ret;
}

int
release_init_from_file(release_t *release, const char *filename)
{
	int err = 0;
	FILE *release_file;

	release_file = fopen(filename, "r");
	if (release_file == NULL) {
		opkg_perror(ERROR, "Failed to open %s", filename);
		return -1;
	}

	if ((err = release_parse_from_stream(release, release_file))) {
		if (err == 1) {
			opkg_msg(ERROR, "Malformed release file %s.\n",
				filename);
		}
		err = -1;
	}

	if (!release_has_architecture(HOST_CPU_STR, release)) {
		if (err == 1) {
			opkg_msg(ERROR, "Architecture '%s' not found on Release file.\n",
				HOST_CPU_STR);
		}
		err = -1;
	}

	if ( err != 0 )
		unlink (filename);

	return err;
}

int
release_has_architecture(const char *archname, release_t *release)
{
     int ret = 0;
     char **arch = release->architectures;

     while (*arch != NULL ) {
	  if (strcmp(archname, *arch++) == 0) {
	       ret = 1;
	       break;
	       }
     }

     return ret;
}

int
release_has_component(const char *compname, release_t *release)
{
     int ret = 0;
     char **comp = release->components;

     while (*comp != NULL ) {
	  if (strcmp(compname, *comp++) == 0) {
	       ret = 1;
	       break;
	       }
     }

     return ret;
}

int
release_get_packages(release_t *release, dist_src_t *dist, char *lists_dir, char *tmpdir)
{
     int ret = 0;

     int err;
     char **comp = dist->extra_data;

     while (*comp != NULL ) {
	  char *url;
	  char *tmp_file_name, *list_file_name;

	  if (!release_has_component(*comp, release)) {
	       opkg_msg(ERROR, "Component '%s' not defined on %s.\n", *comp, dist->name);
	  } else {

	       char *location = dist_src_location(dist);
	       char *package = dist_src_package(dist, *comp);
	       sprintf_alloc(&url, "%s/%s.gz", location, package);

	       sprintf_alloc(&list_file_name, "%s/%s-%s", lists_dir, dist->name, *comp);

	       sprintf_alloc(&tmp_file_name, "%s/%s-%s.gz", tmpdir, dist->name, *comp);
	       err = opkg_download(url, tmp_file_name, NULL, NULL);
	       if (err == 0) {
		    FILE *in, *out;
		    opkg_msg(NOTICE, "Inflating %s.\n", url);
		    in = fopen (tmp_file_name, "r");
		    out = fopen (list_file_name, "w");
		    if (in && out) {
			 if (unzip (in, out))
			      opkg_msg(INFO, "Corrumpt file at %s.\n", url);
		    } else
			 err = 1;
		    if (in)
			 fclose (in);
		    if (out)
			 fclose (out);
		    unlink (tmp_file_name);
		    free(url);

		    if (!err) {
			 char *stored_md5 = release_get_md5(package,release,"gz");

			 char *md5fname;
			 sprintf_alloc(&md5fname, "%s/%s-%s", lists_dir, dist->name, stored_md5);
			 free(stored_md5);

			 char *md5 = file_md5sum_alloc(list_file_name);

			 FILE *md5fd = fopen(md5fname, "w");
			 fprintf(md5fd, "%s", md5);
			 fclose(md5fd);

			 free(md5fname);
			 free(md5);
		    }

	       }

	       if (err!=0) {
		    sprintf_alloc(&url, "%s/%s", location, package);
		    err = opkg_download(url, list_file_name, NULL, NULL);
		    if (err!=0)
			 ret = -1;
		    free(url);
	       }

	       free(package);
	       free(location);

	       free(tmp_file_name);
	       free(list_file_name);
	  }
	  comp++;
     }

     return ret;
}


int
release_get_size(const char *filename, release_t *release, char *extension)
{
     int size = -1;

     char *name;
     if (extension)
	  sprintf_alloc(&name, "%s.%s", filename, extension);
     else
	  name = (char *) filename;

     release_cksum_list_elt_t *iter;
     release_cksum_t *cksum;

     for (iter = void_list_first(release->md5sums); iter; iter = void_list_next(release->md5sums, iter)) {
	  cksum = (release_cksum_t *)iter->data;
	  if (strcmp(name,cksum->filename)==0) {
	       size = cksum->filesize;
	       break;
	  }
     }

#if defined HAVE_SHA256
     if (size == -1)
	  for (iter = void_list_first(release->sha256sums); iter; iter = void_list_next(release->sha256sums, iter)) {
	       cksum = (release_cksum_t *)iter->data;
	       if (strcmp(name,cksum->filename)==0) {
		    size = cksum->filesize;
		    break;
	       }
	  }
#endif

     if (extension)
	  free(name);

     return size;
}

char *
release_get_md5(const char *filename, release_t *release, char *extension)
{
     char *value = NULL;

     char *name;
     if (extension)
	  sprintf_alloc(&name, "%s.%s", filename, extension);
     else
	  name = (char *) filename;

     release_cksum_list_elt_t *iter;
     release_cksum_t *cksum;

     for (iter = void_list_first(release->md5sums); iter; iter = void_list_next(release->md5sums, iter)) {
	  cksum = (release_cksum_t *)iter->data;
	  if (strcmp(name,cksum->filename)==0) {
	       value = xstrdup(cksum->value);
	       break;
	  }
     }

     if (extension)
	  free(name);

     return value;
}

#if defined HAVE_SHA256
char *
release_get_sha256(const char *filename, release_t *release, char *extension)
{
     char *value = NULL;

     char *name;
     if (extension)
	  sprintf_alloc(&name, "%s.%s", filename, extension);
     else
	  name = (char *) filename;

     release_cksum_list_elt_t *iter;
     release_cksum_t *cksum;

     for (iter = void_list_first(release->sha256sums); iter; iter = void_list_next(release->sha256sums, iter)) {
	  cksum = (release_cksum_t *)iter->data;
	  if (strcmp(name,cksum->filename)==0) {
	       xstrdup(value = cksum->value);
	       break;
	  }
     }

     if (extension)
	  free(name);

     return value;
}
#endif

void
release_formatted_field(FILE *fp, release_t *release, const char *field)
{

     switch (field[0])
     {
     case 'A':
	  if (strcasecmp(field, "Architectures") == 0) {
	       if (release->architectures) {
		    char **arch = release->architectures;
		    fprintf(fp, "Architectures:");
		    while(*arch != NULL)
			 fprintf(fp, " %s", *arch++);
                    fprintf(fp, "\n");
	       }
	  } else {
	       goto UNKNOWN_FMT_FIELD;
	  }
	  break;

     case 'C':
	  if (strcasecmp(field, "Codename") == 0) {
	       fprintf(fp, "Codename: %s\n", release->name);
	  }
	  else if (strcasecmp(field, "Components") == 0) {
	       if (release->components) {
		    char **comp = release->components;
		    fprintf(fp, "Components:");
		    while(*comp != NULL)
			 fprintf(fp, " %s", *comp++);
                    fprintf(fp, "\n");
	       }
	  } else {
	       goto UNKNOWN_FMT_FIELD;
	  }
	  break;

     case 'D':
	  if (strcasecmp(field, "Date") == 0) {
	       fprintf(fp, "Date: %s\n", release->datestring);
	  } else {
	       goto UNKNOWN_FMT_FIELD;
	  }
	  break;

     case 'M':
	  if (strcasecmp(field, "MD5sum") == 0) {
	       if (release->md5sums) {
		    fprintf(fp, "MD5sum:\n");
		    release_cksum_list_elt_t *iter;
		    release_cksum_t *cksum;
		    for (iter = void_list_first(release->md5sums); iter; iter = void_list_next(release->md5sums, iter)) {
			 cksum = (release_cksum_t *)iter->data;
			 fprintf(fp, " %s %10d %s\n", cksum->value, cksum->filesize, cksum->filename);
		    }
	       }
	  } else {
	       goto UNKNOWN_FMT_FIELD;
	  }
	  break;

     case 'S':
	  if (strcasecmp(field, "SHA256") == 0) {
	       if (release->sha256sums!=NULL) {
		    fprintf(fp, "SHA256:");
		    release_cksum_list_elt_t *iter;
		    release_cksum_t *cksum;
		    for (iter = void_list_first(release->sha256sums); iter; iter = void_list_next(release->sha256sums, iter)) {
			 cksum = (release_cksum_t *)iter->data;
			 fprintf(fp, " %s %10d %s\n", cksum->value, cksum->filesize, cksum->filename);
		    }
	       }
	  } else {
	       goto UNKNOWN_FMT_FIELD;
	  }
	  break;

     default:
	  goto UNKNOWN_FMT_FIELD;
     }

     return;

UNKNOWN_FMT_FIELD:
     opkg_msg(ERROR, "Internal error: field=%s\n", field);
}

void
release_formatted_info(FILE *fp, release_t *release)
{
	release_formatted_field(fp, release, "Codename");
	release_formatted_field(fp, release, "Date");
	release_formatted_field(fp, release, "Architectures");
	release_formatted_field(fp, release, "Components");
	release_formatted_field(fp, release, "MD5sum");
#if defined HAVE_SHA256
	release_formatted_field(fp, release, "SHA256");
#endif
	fputs("\n", fp);
}

