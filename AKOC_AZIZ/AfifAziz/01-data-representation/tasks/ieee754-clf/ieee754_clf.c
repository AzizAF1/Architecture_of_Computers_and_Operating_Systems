#include <stdint.h>
#include <string.h>

#include "ieee754_clf.h"

float_class_t classify(double x) {
    uint64_t bits;
    memcpy(&bits, &x, sizeof(bits));

    uint64_t sign = bits >> 63;
    uint64_t exp = (bits >> 52) & 0x7FF;
    uint64_t frac = bits & 0xFFFFFFFFFFFFF;

    if(exp == 0 && frac == 0) {
        return sign ? MinusZero : Zero;
    } else if(exp == 0 && frac != 0) {
        if (sign) return MinusDenormal;
        return Denormal;
    } else if(exp == 0x7FF && frac == 0) {
        if (sign) return MinusInf;
        return Inf;
    } else if(exp == 0x7FF && frac != 0) {
        return NaN;
    } else {
        if (sign) return MinusRegular;
        return Regular;
    }
}
