/*  =========================================================================
    dafka_console_consumer - description

    Copyright (c) the Contributors as noted in the AUTHORS file.
    This file is part of CZMQ, the high-level C binding for 0MQ:
    http://czmq.zeromq.org.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/

/*
@header
    dafka_console_consumer -
@discuss
@end
*/

#include "dafka_classes.h"

int main (int argc, char *argv [])
{
    zsys_set_linger ((size_t ) -1);

    zargs_t *args = zargs_new (argc, argv);

    if (zargs_hasx (args, "--help", "-h", NULL) || zargs_arguments (args) != 1) {
        puts ("Usage: dafka_console_consumer [--verbose] [-c config] [--pub tower-pub-address] [--sub tower-sub-address] topic");
        return 0;
    }

    zconfig_t *config;

    if (zargs_has (args, "-c"))
        config = zconfig_load (zargs_get (args, "-c"));
    else
        config = zconfig_new ("root", NULL);

    if (zargs_has (args, "--verbose")) {
        zconfig_put (config, "beacon/verbose", "1");
        zconfig_put (config, "consumer/verbose", "1");
    }

    if (zargs_has (args, "--pub"))
        zconfig_put (config, "beacon/pub_address", zargs_get (args, "--pub"));

    if (zargs_has (args, "--sub"))
        zconfig_put (config, "beacon/sub_address", zargs_get (args, "--sub"));

    const char *topic = zargs_first (args);

    zactor_t *consumer = zactor_new (dafka_subscriber_actor, config);
    assert (consumer);

    int rc = zsock_send (consumer, "ss", "SUBSCRIBE", topic);
    assert (rc == 0);

    char *address;
    zframe_t *content;
    while (true) {
        zsock_brecv (consumer, "ssf", &topic, &address, &content);
        if (!content)
            break;      // Interrupted

        char *content_str = zframe_strdup (content);
        printf ("%s %s %s\n", topic, address, content_str);
        zstr_free (&content_str);
        zframe_destroy (&content);
    }

    zactor_destroy (&consumer);
    zconfig_destroy (&config);
    zargs_destroy (&args);

    return 0;
}
