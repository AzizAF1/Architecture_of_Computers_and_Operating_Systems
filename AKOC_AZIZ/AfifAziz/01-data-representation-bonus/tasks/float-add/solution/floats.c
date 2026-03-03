#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Структура Float
struct Float {
    bool sign;               // Знак числа
    uint32_t exponent;       // Экспонента
    uint64_t mantissa;       // Мантисса
    int exponent_bits;       // Количество битов для экспоненты
    int mantissa_bits;       // Количество битов для мантиссы
};

typedef struct Float Float;

// Функция для инициализации числа с плавающей точкой
int float_init(Float* self, int exponent_bits, int mantissa_bits) {
    if (!self || exponent_bits <= 0 || mantissa_bits <= 0) {
        return -1; // Ошибка, если параметры некорректны
    }

    self->exponent_bits = exponent_bits;
    self->mantissa_bits = mantissa_bits;
    self->sign = false; // По умолчанию положительное число
    self->exponent = 0; // По умолчанию экспонента равна 0
    self->mantissa = 0; // По умолчанию мантисса равна 0

    return 0; // Успешная инициализация
}

// Деструктор (ничего не освобождается, так как память не выделяется динамически)
void float_destroy(Float* self) {
    // Нет динамически выделенной памяти, ничего не нужно освобождать
}

// Получение количества бит для экспоненты
int float_get_exponent_bits(Float* self) {
    return self->exponent_bits;
}

// Получение количества бит для мантиссы
int float_get_mantissa_bits(Float* self) {
    return self->mantissa_bits;
}

// Установка знака
void float_set_sign(Float* self, bool sign) {
    self->sign = sign;
}

// Получение знака
bool float_get_sign(const Float* self) {
    return self->sign;
}

// Установка экспоненты
void float_set_exponent(Float* self, const void* exponent) {
    uint8_t* exp_bytes = (uint8_t*) exponent;
    self->exponent = 0;

    for (int i = 0; i < (self->exponent_bits + 7) / 8; ++i) {
        self->exponent |= (uint32_t)exp_bytes[i] << (8 * i);
    }
}

// Получение экспоненты
void float_get_exponent(const Float* self, void* target) {
    uint8_t* exp_bytes = (uint8_t*) target;
    for (int i = 0; i < (self->exponent_bits + 7) / 8; ++i) {
        exp_bytes[i] = (self->exponent >> (8 * i)) & 0xFF;
    }
}

// Установка мантиссы
void float_set_mantissa(Float* self, const void* mantissa) {
    uint8_t* mant_bytes = (uint8_t*) mantissa;
    self->mantissa = 0;

    for (int i = 0; i < (self->mantissa_bits + 7) / 8; ++i) {
        self->mantissa |= (uint64_t)mant_bytes[i] << (8 * i);
    }
}

// Получение мантиссы
void float_get_mantissa(const Float* self, void* target) {
    uint8_t* mant_bytes = (uint8_t*) target;
    for (int i = 0; i < (self->mantissa_bits + 7) / 8; ++i) {
        mant_bytes[i] = (self->mantissa >> (8 * i)) & 0xFF;
    }
}

// Операция умножения двух чисел с плавающей точкой
void float_mul(Float* result, const Float* a, const Float* b) {
    // Умножение экспонент
    result->exponent = a->exponent + b->exponent - (1 << (a->exponent_bits - 1));

    // Умножение мантисс
    uint64_t mantissa_a = a->mantissa + (1 << a->mantissa_bits); // мантисса а в нормализованной форме
    uint64_t mantissa_b = b->mantissa + (1 << b->mantissa_bits); // мантисса b в нормализованной форме

    uint64_t result_mantissa = (mantissa_a * mantissa_b) >> a->mantissa_bits;
    
    result->mantissa = result_mantissa;
    result->sign = a->sign != b->sign;  // Знак будет результатом умножения знаков
}

// Операция сложения двух чисел с плавающей точкой
void float_add(Float* result, const Float* a, const Float* b) {
    // Приведение экспонент к одинаковому значению (выравнивание мантисс)
    uint64_t mantissa_a = a->mantissa + (1 << a->mantissa_bits);
    uint64_t mantissa_b = b->mantissa + (1 << b->mantissa_bits);

    if (a->exponent > b->exponent) {
        mantissa_b >>= (a->exponent - b->exponent);
    } else if (b->exponent > a->exponent) {
        mantissa_a >>= (b->exponent - a->exponent);
    }

    uint64_t result_mantissa = mantissa_a + mantissa_b;
    result->mantissa = result_mantissa;

    // Установка экспоненты
    result->exponent = a->exponent;

    // Определение знака
    if (a->sign == b->sign) {
        result->sign = a->sign;
    } else {
        result->sign = (mantissa_a > mantissa_b) ? a->sign : b->sign;
    }
}

// Операция вычитания двух чисел с плавающей точкой
void float_sub(Float* result, const Float* a, const Float* b) {
    // Приведение экспонент к одинаковому значению (выравнивание мантисс)
    uint64_t mantissa_a = a->mantissa + (1 << a->mantissa_bits);
    uint64_t mantissa_b = b->mantissa + (1 << b->mantissa_bits);

    if (a->exponent > b->exponent) {
        mantissa_b >>= (a->exponent - b->exponent);
    } else if (b->exponent > a->exponent) {
        mantissa_a >>= (b->exponent - a->exponent);
    }

    uint64_t result_mantissa = mantissa_a - mantissa_b;
    result->mantissa = result_mantissa;

    // Установка экспоненты
    result->exponent = a->exponent;

    // Определение знака
    result->sign = a->sign;
}

// Операция деления двух чисел с плавающей точкой
void float_div(Float* result, const Float* a, const Float* b) {
    // Деление экспонент
    result->exponent = a->exponent - b->exponent;

    // Деление мантисс
    uint64_t mantissa_a = a->mantissa + (1 << a->mantissa_bits);
    uint64_t mantissa_b = b->mantissa + (1 << b->mantissa_bits);

    uint64_t result_mantissa = (mantissa_a << (a->mantissa_bits + 1)) / mantissa_b;
    
    result->mantissa = result_mantissa;
    result->sign = a->sign != b->sign;
}

// Переход к ближайшему большему числу с плавающей точкой
void float_next(Float* self) {
    if (self->sign) {
        self->mantissa += 1;
    } else {
        self->mantissa -= 1;
    }
}

// Переход к ближайшему меньшему числу с плавающей точкой
void float_prev(Float* self) {
    if (self->sign) {
        self->mantissa -= 1;
    } else {
        self->mantissa += 1;
    }
}

// Преобразование числа с плавающей точкой в строку
int float_string(const Float* self, char* string, int n) {
    int length = snprintf(string, n, "%f", /* преобразование числа в строку */);
    return length;
}

// Парсинг строки в число с плавающей точкой
void float_parse(Float* self, const char* string) {
    sscanf(string, "%f", /* присваиваем значение */);
}

#ifdef __cplusplus
}
#endif
