/* ============================================================================
 * src/utils/utf8.h - UTF-8 String Handling
 * ========================================================================== */
#ifndef FONTKIT_UTF8_H
#define FONTKIT_UTF8_H

#include <stdint.h>
#include <stddef.h>

uint32_t fk_utf8_decode(const char **str);
int fk_utf8_encode(uint32_t codepoint, char *buffer);
size_t fk_utf8_strlen(const char *str);

#endif /* FONTKIT_UTF8_H */