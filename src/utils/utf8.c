/**
 * utf8.c - UTF-8 string handling
 */

#include "utf8.h"

uint32_t fk_utf8_decode(const char **str) {
    const uint8_t *s = (const uint8_t *)*str;
    uint32_t codepoint;
    
    if (!s || !*s) return 0;
    
    if (s[0] < 0x80) {
        /* 1-byte sequence (ASCII) */
        codepoint = s[0];
        *str += 1;
    } else if ((s[0] & 0xE0) == 0xC0) {
        /* 2-byte sequence */
        if ((s[1] & 0xC0) != 0x80) return 0;
        codepoint = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
        *str += 2;
    } else if ((s[0] & 0xF0) == 0xE0) {
        /* 3-byte sequence */
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return 0;
        codepoint = ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        *str += 3;
    } else if ((s[0] & 0xF8) == 0xF0) {
        /* 4-byte sequence */
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 || (s[3] & 0xC0) != 0x80) return 0;
        codepoint = ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12) | 
                    ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        *str += 4;
    } else {
        return 0;  /* Invalid sequence */
    }
    
    return codepoint;
}

int fk_utf8_encode(uint32_t codepoint, char *buffer) {
    if (codepoint < 0x80) {
        /* 1 byte */
        buffer[0] = (char)codepoint;
        return 1;
    } else if (codepoint < 0x800) {
        /* 2 bytes */
        buffer[0] = (char)(0xC0 | (codepoint >> 6));
        buffer[1] = (char)(0x80 | (codepoint & 0x3F));
        return 2;
    } else if (codepoint < 0x10000) {
        /* 3 bytes */
        buffer[0] = (char)(0xE0 | (codepoint >> 12));
        buffer[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        buffer[2] = (char)(0x80 | (codepoint & 0x3F));
        return 3;
    } else if (codepoint < 0x110000) {
        /* 4 bytes */
        buffer[0] = (char)(0xF0 | (codepoint >> 18));
        buffer[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
        buffer[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        buffer[3] = (char)(0x80 | (codepoint & 0x3F));
        return 4;
    }
    
    return 0;  /* Invalid codepoint */
}

size_t fk_utf8_strlen(const char *str) {
    size_t len = 0;
    while (*str) {
        uint32_t cp = fk_utf8_decode(&str);
        if (cp == 0) break;
        len++;
    }
    return len;
}