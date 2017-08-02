/*  =========================================================================
    lpserver - Lazy Pirate server

    Copyright (c) the Authors
    =========================================================================
*/

/*
@header
    lpserver - Lazy Pirate server
    Binds REQ socket to tcp://\*:5555
    Like hwserver except:
     - echoes request as-is
     - randomly runs slowly, or exits to simulate a crash.
@discuss
@end
*/

#include "czguide_classes.h"

int main (int argc, char *argv [])
{
    bool verbose = false;
    int argn;
    for (argn = 1; argn < argc; argn++) {
        if (streq (argv [argn], "--help")
        ||  streq (argv [argn], "-h")) {
            puts ("lpserver [options] ...");
            puts ("  --verbose / -v         verbose test output");
            puts ("  --help / -h            this information");
            return 0;
        }
        else
        if (streq (argv [argn], "--verbose")
        ||  streq (argv [argn], "-v"))
            verbose = true;
        else {
            printf ("Unknown option: %s\n", argv [argn]);
            return 1;
        }
    }
    //  Insert main code here
    if (verbose)
        zsys_info ("lpserver - Lazy Pirate server");

    srandom ((unsigned) time (NULL));

    zsock_t *server = zsock_new (ZMQ_REP);
    zsock_bind (server, "%s", "tcp://*:5555");

    int cycles = 0;
    while (1) {
        char *request = zstr_recv (server);
        cycles++;

        //  Simulate various problems, after a few cycles
        if (cycles > 3 && randof (3) == 0) {
            printf ("I: simulating a crash\n");
            zstr_free (&request);
            break;
        }
        else
        if (cycles > 3 && randof (3) == 0) {
            printf ("I: simulating CPU overload\n");
            sleep (2);
        }
        printf ("I: normal request (%s)\n", request);
        sleep (1);              //  Do some heavy work
        zstr_send (server, request);
        zstr_free (&request);
    }
    zsock_destroy (&server);
    return 0;
}
