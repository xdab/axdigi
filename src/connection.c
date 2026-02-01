#include "connection.h"
#include "common.h"

int connection_init(connection_t *conn, const char *host, int port, const char *socket)
{
    nonnull(conn, "conn");

    if (socket != NULL && socket[0] != '\0')
    {
        conn->type = CONNECTION_UDS;
        if (uds_client_init(&conn->client.uds, socket, UDS_DEF_TIMEOUT_MS) < 0)
            return -1;
        return 0;
    }

    conn->type = CONNECTION_TCP;
    if (tcp_client_init(&conn->client.tcp, host, port, TCP_DEF_TIMEOUT_MS) < 0)
        return -1;

    return 0;
}

void connection_free(connection_t *conn)
{
    nonnull(conn, "conn");

    if (conn->type == CONNECTION_UDS)
        uds_client_free(&conn->client.uds);
    else
        tcp_client_free(&conn->client.tcp);
}

int connection_listen(connection_t *conn, buffer_t *buf)
{
    nonnull(conn, "conn");
    assert_buffer_valid(buf);

    if (conn->type == CONNECTION_UDS)
        return uds_client_listen(&conn->client.uds, buf);

    return tcp_client_listen(&conn->client.tcp, buf);
}

int connection_send(connection_t *conn, const buffer_t *buf)
{
    nonnull(conn, "conn");
    assert_buffer_valid(buf);

    if (conn->type == CONNECTION_UDS)
        return uds_client_send(&conn->client.uds, buf);

    return tcp_client_send(&conn->client.tcp, buf);
}
