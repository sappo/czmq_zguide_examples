/*  =========================================================================
    lpclient - Lazy Pirate client

    Copyright (c) the Authors
    =========================================================================
*/

/*
@header
    lpclient - Lazy Pirate client
    Use zmq_poll to do a safe request-reply
    To run, start lpserver and then randomly kill/restart it
@discuss
@end
*/

#include "czguide_classes.h"

#define REQUEST_TIMEOUT     2500    //  msecs, (> 1000!)
#define REQUEST_RETRIES     3       //  Before we abandon
#define SERVER_ENDPOINT     "tcp://localhost:5555"

int main (int argc, char *argv [])
{
    bool verbose = false;
    int argn;
    for (argn = 1; argn < argc; argn++) {
        if (streq (argv [argn], "--help")
        ||  streq (argv [argn], "-h")) {
            puts ("lpclient [options] ...");
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
        zsys_info ("lpclient - Lazy Pirate client");

    printf ("I: connecting to server…\n");
    zsock_t *client = zsock_new (ZMQ_REQ);
    assert (client);
    int rc = zsock_connect (client, "%s", SERVER_ENDPOINT);
    assert (rc == 0);

    uint8_t sequence = 0;
    uint8_t retries_left = REQUEST_RETRIES;
    while (retries_left && !zctx_interrupted) {
        //  We send a request, then we work to get a reply
        char request [10];
        sprintf (request, "%"PRIu8, ++sequence);
        zstr_send (client, request);

        int expect_reply = 1;
        while (expect_reply) {
            //  Poll socket for a reply, with timeout
            zpoller_t *poller = zpoller_new (client, NULL);
            zsock_t *sock = (zsock_t *) zpoller_wait (poller, REQUEST_TIMEOUT);
            if (zsys_interrupted) {
                zpoller_destroy (&poller);
                break;          //  Interrupted
            }

            //  Here we process a server reply and exit our loop if the
            //  reply is valid. If we didn't a reply we close the client
            //  socket and resend the request. We try a number of times
            //  before finally abandoning:

            if (sock) {
                //  We got a reply from the server, must match sequence
                char *reply = zstr_recv (client);
                if (!reply) {
                    zpoller_destroy (&poller);
                    break;      //  Interrupted
                }

                if (atoi (reply) == sequence) {
                    printf ("I: server replied OK (%s)\n", reply);
                    retries_left = REQUEST_RETRIES;
                    expect_reply = 0;
                }
                else
                    printf ("E: malformed reply from server: %s\n", reply);

                free (reply);
            }
            else
            if (--retries_left == 0) {
                printf ("E: server seems to be offline, abandoning\n");
                zpoller_destroy (&poller);
                break;
            }
            else {
                printf ("W: no response from server, retrying…\n");
                //  Old socket is confused; close it and open a new one
                zsock_destroy (&client);
                printf ("I: reconnecting to server…\n");
                client = zsock_new (ZMQ_REQ);
                zsock_connect (client, SERVER_ENDPOINT);
                //  Send request again, on new socket
                zstr_send (client, request);
            }
            zpoller_destroy (&poller);
        }
    }
    zsock_destroy (&client);
    return 0;
}
