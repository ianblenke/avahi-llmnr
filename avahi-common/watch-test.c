/* $Id: watch-test.c 879 2005-10-26 21:27:40Z lennart $ */

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include "watch.h"
#include "simple-watch.h"
#include "timeval.h"
#include "gccmacro.h"

static const AvahiPoll *api = NULL;
static AvahiSimplePoll *simple_poll = NULL;

static void callback(AvahiWatch *w, int fd, AvahiWatchEvent event, AVAHI_GCC_UNUSED void *userdata) {

    if (event & AVAHI_WATCH_IN) {
        ssize_t r;
        char c;
        
        if ((r = read(fd, &c, 1)) <= 0) {
            fprintf(stderr, "read() failed: %s\n", r < 0 ? strerror(errno) : "EOF");
            api->watch_free(w);
            return;
        }

        printf("Read: %c\n", c >= 32 && c < 127 ? c : '.');
    }
}

static void wakeup(AvahiTimeout *t, AVAHI_GCC_UNUSED void *userdata) {
    static int i = 0;
    struct timeval tv;

    printf("Wakeup #%i\n", i++);

    if (i > 10)
        avahi_simple_poll_quit(simple_poll);
    else {
        avahi_elapse_time(&tv, 1000, 0);
        api->timeout_update(t, &tv);
    }
}

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {
    struct timeval tv;
    
    simple_poll = avahi_simple_poll_new();

    api = avahi_simple_poll_get(simple_poll);
    api->watch_new(api, 0, AVAHI_WATCH_IN, callback, NULL);

    avahi_elapse_time(&tv, 1000, 0);
    api->timeout_new(api, &tv, wakeup, NULL);

    /* Our main loop */
    avahi_simple_poll_loop(simple_poll);


    avahi_simple_poll_free(simple_poll);
    
    return 0;
}
