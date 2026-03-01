#include "seaclaw/websocket/websocket.h"
#include "seaclaw/core/allocator.h"
#include "seaclaw/core/error.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define SC_WS_MAGIC "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define SC_WS_MAX_FRAME_PAYLOAD (4 * 1024 * 1024)

/* RFC 6455 opcodes */
#define SC_WS_OP_CONTINUATION 0
#define SC_WS_OP_TEXT         1
#define SC_WS_OP_BINARY       2
#define SC_WS_OP_CLOSE        8
#define SC_WS_OP_PING        9
#define SC_WS_OP_PONG        10

struct sc_ws_client {
    void *tls_ctx;       /* placeholder for TLS connection; NULL when stubbed */
    sc_allocator_t *alloc;
};

/* Build a masked WebSocket frame into buf. Returns bytes written. */
size_t sc_ws_build_frame(char *buf, size_t buf_size,
    unsigned opcode,
    const char *payload, size_t payload_len,
    const unsigned char mask_key[4])
{
    if (buf_size < 6 || payload_len > SC_WS_MAX_FRAME_PAYLOAD)
        return 0;

    size_t pos = 0;
    buf[pos++] = (char)(0x80 | (opcode & 0x0F));  /* FIN=1, opcode */

    if (payload_len <= 125) {
        buf[pos++] = (char)(0x80 | (payload_len & 0x7F));  /* MASK=1, len */
    } else if (payload_len <= 65535) {
        buf[pos++] = (char)(0x80 | 126);
        buf[pos++] = (char)((payload_len >> 8) & 0xFF);
        buf[pos++] = (char)(payload_len & 0xFF);
    } else {
        buf[pos++] = (char)(0x80 | 127);
        buf[pos++] = (char)((payload_len >> 56) & 0xFF);
        buf[pos++] = (char)((payload_len >> 48) & 0xFF);
        buf[pos++] = (char)((payload_len >> 40) & 0xFF);
        buf[pos++] = (char)((payload_len >> 32) & 0xFF);
        buf[pos++] = (char)((payload_len >> 24) & 0xFF);
        buf[pos++] = (char)((payload_len >> 16) & 0xFF);
        buf[pos++] = (char)((payload_len >> 8) & 0xFF);
        buf[pos++] = (char)(payload_len & 0xFF);
    }

    buf[pos++] = mask_key[0];
    buf[pos++] = mask_key[1];
    buf[pos++] = mask_key[2];
    buf[pos++] = mask_key[3];

    if (pos + payload_len > buf_size)
        return 0;

    for (size_t i = 0; i < payload_len; i++)
        buf[pos + i] = (char)((unsigned char)payload[i] ^ mask_key[i % 4]);
    pos += payload_len;

    return pos;
}

/* Apply XOR mask in-place (for server→client frames). */
void sc_ws_apply_mask(char *payload, size_t len, const unsigned char mask_key[4])
{
    for (size_t i = 0; i < len; i++)
        payload[i] = (char)((unsigned char)payload[i] ^ mask_key[i % 4]);
}

/* Parse WebSocket frame header. Returns 0 on success, non-zero on error. */
int sc_ws_parse_header(const char *bytes, size_t bytes_len, sc_ws_parsed_header_t *out)
{
    if (bytes_len < 2 || !out) return -1;

    out->fin = (bytes[0] & 0x80) != 0;
    out->opcode = bytes[0] & 0x0F;
    out->masked = (bytes[1] & 0x80) != 0;
    uint64_t payload_len = bytes[1] & 0x7F;
    size_t hlen = 2;

    if (payload_len == 126) {
        if (bytes_len < 4) return -1;
        payload_len = ((uint64_t)(unsigned char)bytes[2] << 8) | (unsigned char)bytes[3];
        hlen = 4;
    } else if (payload_len == 127) {
        if (bytes_len < 10) return -1;
        payload_len = 0;
        for (int i = 2; i < 10; i++)
            payload_len = (payload_len << 8) | (unsigned char)bytes[i];
        hlen = 10;
    }

    if (out->masked) hlen += 4;
    if (payload_len > SC_WS_MAX_FRAME_PAYLOAD) return -1;

    out->payload_len = (unsigned long long)payload_len;
    out->header_bytes = hlen;
    return 0;
}

sc_error_t sc_ws_connect(sc_allocator_t *alloc,
    const char *url,
    sc_ws_client_t **out)
{
    if (!alloc || !url || !out) return SC_ERR_INVALID_ARGUMENT;
    (void)alloc;
    (void)url;
    (void)out;
    /* Stub: real implementation would use raw TCP + TLS (OpenSSL/mbedTLS).
     * WebSocket requires wss:// (TLS) for production; ws:// only for localhost. */
    return SC_ERR_NOT_SUPPORTED;
}

sc_error_t sc_ws_send(sc_ws_client_t *ws,
    const char *data,
    size_t data_len)
{
    if (!ws || !data) return SC_ERR_INVALID_ARGUMENT;
    (void)data_len;
    /* Stub: would encode frame and write to TLS connection */
    return SC_ERR_NOT_SUPPORTED;
}

sc_error_t sc_ws_recv(sc_ws_client_t *ws,
    sc_allocator_t *alloc,
    char **data_out,
    size_t *data_len_out)
{
    if (!ws || !alloc || !data_out || !data_len_out) return SC_ERR_INVALID_ARGUMENT;
    (void)alloc;
    /* Stub: would read frame, handle ping/pong, return text/binary payload */
    return SC_ERR_NOT_SUPPORTED;
}

void sc_ws_close(sc_ws_client_t *ws, sc_allocator_t *alloc)
{
    if (!ws) return;
    (void)alloc;
    /* Would send close frame and shutdown TLS */
    memset(ws, 0, sizeof(*ws));
}
