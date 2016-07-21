/* buf: a sized buffer type. */

#include "util.h"
#include "buf.h"

void mybuf_destroy(mybuf_t *buf){
        free(buf->data);
        free(buf);
}

mybuf_t * mybuf_size(mybuf_t *buf, size_t len)
{
    if (buf == NULL)
    {
        buf = malloc(sizeof(mybuf_t));
        buf->data = NULL;
        buf->len = 0;
    }

    buf->data = realloc(buf->data, len);
    
    if (buf->len > len)
        buf->len = len;
    buf->limit = len;

    return buf;
}

void mybuf_push(mybuf_t *buf, uint8_t c)
{
    assert(buf->len < buf->limit);
    buf->data[buf->len++] = c;
}

void mybuf_concat(mybuf_t *dst, uint8_t *src, size_t len)
{
    assert(dst->len + len <= dst->limit);

    for (size_t i = 0; i < len; i++)
        dst->data[dst->len++] = src[i];
}

char * mybuf_tostr(mybuf_t *buf)
{
    char *str = malloc(buf->len + 1);

    memcpy(str, buf->data, buf->len);
    str[buf->len] = '\0';

    return str;
}
