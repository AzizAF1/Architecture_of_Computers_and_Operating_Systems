
#include "floats.h"
#include <cstdint>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int float_init(Float* self, int exponent_bits, int mantissa_bits) {
    // TODO
    if (!self || exponent_bits <= 0 || mantissa_bits <= 0) {
        return -1;  // Invalid input
    }

    self->exponent_bits = exponent_bits;
    self->mantissa_bits = mantissa_bits;
    self->sign = 0;
    self->exponent = 0;
    self->mantissa = 0;

    return 0;  // Success
}

void float_destroy(Float* self) {
    // For now, no dynamic memory allocation was used, so no special cleanup is needed.
    memset(self, 0, sizeof(Float));  // Clear memory (good practice)
}

void float_set_sign(Float* self, bool sign) {
        self->sign = sign;
    }

void float_set_mantissa(Float* self, const void* mantissa) {
        memcpy(&self->mantissa, mantissa, (self->mantissa_bits + 7) / 8);
    }

void float_set_exponent(Float* self, const void* exponent) {
        memcpy(&self->exponent, exponent, (self->exponent_bits + 7) / 8);
    }

bool float_get_sign(const Float* self) {
        return self->sign;
    }

void float_get_mantissa(const Float* self, void* target) {
        memcpy(target, &self->mantissa, (self->mantissa_bits + 7) / 8);
    }

void float_get_exponent(const Float* self, void* target) {
        memcpy(target, &self->exponent, (self->exponent_bits + 7) / 8);
    }


void float_mul(Float* result, const Float* a, const Float* b) {
        if (!result || !a || !b) {
            return;
        }

        // Step 1: Add the exponents
        uint32_t exponent_a = a->exponent;
        uint32_t exponent_b = b->exponent;
        uint32_t new_exponent = exponent_a + exponent_b - (1 << (a->exponent_bits - 1));

        // Step 2: Multiply the mantissas
        uint64_t mantissa_a = a->mantissa;
        uint64_t mantissa_b = b->mantissa;
        uint64_t new_mantissa = (mantissa_a + (1 << a->mantissa_bits)) * (mantissa_b + (1 << b->mantissa_bits));

        // Step 3: Normalize mantissa and adjust exponent
        uint64_t max_mantissa = (1 << (a->mantissa_bits + b->mantissa_bits));  // Resulting mantissa size
        if (new_mantissa >= max_mantissa) {
            new_mantissa >>= 1;  // Shift mantissa if it's too large
            new_exponent++;
        }

        // Step 4: Round the result (optional - rounding logic to be added based on IEEE standards)

        // Step 5: Set the result
        result->sign = a->sign ^ b->sign;  // XOR the signs for multiplication
        result->exponent = new_exponent;
        result->mantissa = new_mantissa;
    }


#ifdef __cplusplus
}
#endif
