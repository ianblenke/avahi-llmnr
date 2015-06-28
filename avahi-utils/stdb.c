/* $Id: stdb.c 902 2005-10-27 22:49:37Z lennart $ */

/***
  This file is part of avahi.
 
  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.
 
  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.
 
  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#include <gdbm.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <stdio.h>

#include <avahi-common/malloc.h>

#include "stdb.h"

static GDBM_FILE gdbm_file = NULL;
static char *buffer = NULL;

static int init(void) {

    if (gdbm_file)
        return 0;

    if (!(gdbm_file = gdbm_open((char*) DATABASE_FILE, 0, GDBM_READER, 0, NULL)))
        return -1;

    return 0;
}

const char* stdb_lookup(const char *name) {
    datum key, data;
    const char *loc;

    if (init() < 0)
        goto fail;

    data.dptr = NULL;
    data.dsize = 0;
    
    if ((loc = setlocale(LC_MESSAGES, NULL))) {
        char k[256];
        
        snprintf(k, sizeof(k), "%s[%s]", name, loc);
        key.dptr = k;
        key.dsize = strlen(k);
        data = gdbm_fetch(gdbm_file, key);

        if (!data.dptr) {
            char l[32], *e;
            snprintf(l, sizeof(l), "%s", loc);
            
            if ((e = strchr(l, '@'))) {
                *e = 0;
                snprintf(k, sizeof(k), "%s[%s]", name, l);
                key.dptr = k;
                key.dsize = strlen(k);
                data = gdbm_fetch(gdbm_file, key);
            }

            if (!data.dptr) {
                if ((e = strchr(l, '_'))) {
                    *e = 0;
                    snprintf(k, sizeof(k), "%s[%s]", name, l);
                    key.dptr = k;
                    key.dsize = strlen(k);
                    data = gdbm_fetch(gdbm_file, key);
                }
            }
        }
    }

    if (!data.dptr) {
        key.dptr = (char*) name;
        key.dsize = strlen(name);
        data = gdbm_fetch(gdbm_file, key);
    }

    if (!data.dptr)
        goto fail;

    avahi_free(buffer);
    buffer = avahi_strndup(data.dptr, data.dsize);
    free(data.dptr);
    
    return buffer;
    
fail:

    return name;
}

void stdb_shutdown(void) {
    if (gdbm_file)
        gdbm_close(gdbm_file);

    avahi_free(buffer);
}
