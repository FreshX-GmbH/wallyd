#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct{
    size_t len;        // current length of buffer (used bytes)
    size_t limit;      // maximum length of buffer (allocated)
    uint8_t *data;     // insert bytes here
}mybuf_t;

mybuf_t * mybuf_size(mybuf_t *buf, size_t len);
void mybuf_push(mybuf_t *buf, uint8_t c);
void mybuf_destroy(mybuf_t *buf);
void mybuf_concat(mybuf_t *buf, uint8_t *data, size_t len);
char * mybuf_tostr(mybuf_t *buf);
