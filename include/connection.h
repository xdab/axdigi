#pragma once

#include <buffer.h>
#include <tcp.h>
#include <uds.h>

typedef enum
{
    CONNECTION_TCP,
    CONNECTION_UDS
} connection_type_e;

typedef struct
{
    connection_type_e type;
    union
    {
        tcp_client_t tcp;
        uds_client_t uds;
    } client;
} connection_t;

int connection_init(connection_t *conn, const char *host, int port, const char *socket);

void connection_free(connection_t *conn);

int connection_listen(connection_t *conn, buffer_t *buf);

int connection_send(connection_t *conn, const buffer_t *buf);
