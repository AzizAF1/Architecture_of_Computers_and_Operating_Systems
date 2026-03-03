#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Float {
    bool sign;                  // Sign bit (0 for positive, 1 for negative)
    uint32_t exponent;          // Exponent (unsigned integer)
    uint64_t mantissa;          // Mantissa (unsigned integer)
    int exponent_bits;          // Number of bits for the exponent
    int mantissa_bits;          // Number of bits for the mantissa
};

typedef struct Float Float;

// Function declarations
int float_init(Float* self, int exponent_bits, int mantissa_bits);
void float_destroy(Float* self);

void float_set_sign(Float* self, bool sign);
void float_set_mantissa(Float* self, const void* mantissa);
void float_set_exponent(Float* self, const void* exponent);

bool float_get_sign(const Float* self);
void float_get_mantissa(const Float* self, void* target);
void float_get_exponent(const Float* self, void* target);

void float_mul(Float* result, const Float* a, const Float* b);

#ifdef __cplusplus
}
#endif
