#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t simpleUnicodeUpper(uint16_t c);
uint16_t simpleUnicodeLower(uint16_t c);
uint16_t simpleUnicodeFold(uint16_t c);
uint16_t simpleUnicodeTitle(uint16_t c);
uint16_t simpleUnicodeUnacc(uint16_t c, uint16_t** p, int* l);

#ifdef __cplusplus
}
#endif
