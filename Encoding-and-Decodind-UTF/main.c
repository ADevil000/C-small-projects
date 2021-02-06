#include <stdio.h>
#include <stdlib.h>

enum Encoding {
    UTF8,
    UTF8_BOM,
    UTF16_LE,
    UTF16_BE,
    UTF32_LE,
    UTF32_BE
};

void fill0(unsigned char *a, size_t *j, size_t n) {
    for (size_t i = 0; i < n; i++) {
        a[*j] = 0;
        *j += 1;
    }
}

void swap(unsigned char* a, unsigned char* b) {
    unsigned char tmp = *a;
    *a = *b;
    *b = tmp;
}

void change_wrong_UTF8_byte(unsigned char *a, unsigned char b) {
    a[0] = '\xDC';
    a[1] = b;
}

void change_ending16(unsigned char *c, size_t size) {
    for (size_t i = 0; i < size - 1; i += 2) {
        swap(c + i, c + i + 1);
    }
}

void change_ending32(unsigned char *c, size_t size) {
    for (size_t i = 0; i < size - 3; i += 4) {
        swap(c + i, c + i + 3);
        swap(c + i + 1, c + i + 2);
    }
}

size_t UTF8_to_32BE(unsigned char **c, size_t size) {
    unsigned char *U32 = malloc((4 * size + 4) * sizeof(unsigned char));
    if (U32 == NULL) {
        printf("Problem with memory allocation");
        free(*c);
        *c = NULL;
        return 0;
    }
    size_t i = (**c != (*U32 = '\xEF')) ? 0 : 3;
    size_t j = 0;
    fill0(U32, &j, 2);
    U32[j++] = '\xFE';
    U32[j++] = '\xFF';
    for (; i < size;) {
        if ((*c)[i] < 128) {
            // 0xxxxxxx
            fill0(U32, &j, 3);
            U32[j++] = (*c)[i++];
        } else if ((*c)[i] < (7 << 5)) {
            // 110xxxxx 10xxxxxx
            fill0(U32, &j, 2);
            //если неверные байты
            if (i + 1 >= size || ((*c)[i] >> 6) != 3 || ((*c)[i + 1] >> 6) < 2) {
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                continue;
            } // неверный первый байт
            if (((*c)[i + 1] >> 6) > 2) {
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                fill0(U32, &j, 2);
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                continue;
            } // неверный второй байт
            // если все хорошо
            U32[j++] = ((*c)[i++] ^ (3 << 6)) >> 2;
            U32[j++] = (*c)[i] ^ (1 << 7) ^ ((*c)[i - 1] << 6);
            i++;
        } else if ((*c)[i] < (15 << 4)) {
            // 1110xxxx 10xxxxxx 10xxxxxx
            fill0(U32, &j, 2);
            if (i + 1 >= size || ((*c)[i] >> 4) != 14 || ((*c)[i + 1] >> 6) < 2) {
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                continue;
            } // неверный первый байт
            if (i + 2 >= size || ((*c)[i + 1] >> 6) > 2 || ((*c)[i + 2] >> 6) < 2) {
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                fill0(U32, &j, 2);
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                continue;
            } // неверный второй байт
            if (((*c)[i + 2] >> 6) > 2) {
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                fill0(U32, &j, 2);
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                fill0(U32, &j, 2);
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                continue;
            } // неверный третий байт
            // все байты верные
            U32[j++] = ((*c)[i++] << 4) ^ (((*c)[i] ^ (1 << 7)) >> 2);
            U32[j++] = ((*c)[i++] << 6) ^ ((*c)[i++] ^ (1 << 7));
        } else if ((*c)[i] < (31 << 3)) {
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            fill0(U32, &j, 1);
            if (i + 1 >= size || ((*c)[i] >> 3) != 30 || ((*c)[i + 1] >> 6) < 2) {
                fill0(U32, &j, 1);
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                continue;
            } // неверный первый байт
            if (i + 2 >= size || ((*c)[i + 1] >> 6) > 2 || ((*c)[i + 2] >> 6) < 2) {
                fill0(U32, &j, 1);
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                fill0(U32, &j, 2);
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                continue;
            } // неверный второй байт
            if (i + 3 >= size || ((*c)[i + 2] >> 6) > 2 || ((*c)[i + 3] >> 6) < 2) {
                fill0(U32, &j, 1);
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                fill0(U32, &j, 2);
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                fill0(U32, &j, 2);
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                continue;
            } // неверный третий байт
            if (((*c)[i + 3] >> 6) > 2) {
                fill0(U32, &j, 1);
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                fill0(U32, &j, 2);
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                fill0(U32, &j, 2);
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                fill0(U32, &j, 2);
                change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
                j += 2;
                continue;
            } // неверный четвертый байт
            // если все хорошо
            U32[j++] = (((*c)[i++] ^ (15 << 4)) << 2) ^ (((*c)[i] ^ (1 << 7)) >> 4);
            U32[j++] = ((*c)[i++] << 4) ^ (((*c)[i] ^ (1 << 7)) >> 2);
            U32[j++] = ((*c)[i++] << 6) ^ ((*c)[i++] ^ (1 << 7));
        } else {
            // 11111xx - wrong
            fill0(U32, &j, 2);
            change_wrong_UTF8_byte(U32 + j, (*c)[i++]);
            j += 2;
        }
        // добавить обработку неправильных UTF8 символов
    }
    free(*c);
    *c = U32;
    return j;
}

size_t UTF16BE_to_32BE(unsigned char **c, size_t size) {
    unsigned char *U32 = malloc((2 * size + 4) * sizeof(unsigned char));
    unsigned char check;
    if (U32 == NULL) {
        printf("Problem in memory allocation");
        free(*c);
        *c = NULL;
        return 0;
    }
    size_t j = 0, i = 0;
    fill0(U32, &j, 2);
    U32[j++] = (*c)[i++];
    U32[j++] = (*c)[i++];
    for (; i < size; ) {
        if ((*c)[i] < (check = '\xD8') || (*c)[i] > (check = '\xDF')) {
            fill0(U32, &j, 2);
            if (i + 1 < size) {
                U32[j++] = (*c)[i++];
                U32[j++] = (*c)[i++];
            } else {
                U32[j++] = '\xFF';
                U32[j++] = '\xFD';
                i += 2;
            }
        } else {
            if (i + 1 >= size || (*c)[i] >= (check = '\xDC')) {
                fill0(U32, &j, 2);
                if (i + 1 < size) {
                    U32[j++] = (*c)[i++];
                    U32[j++] = (*c)[i++];
                } else {
                    U32[j++] = '\xFF';
                    U32[j++] = '\xFD';
                    i += 2;
                }
                continue;
            } // если неверный первый в суррогатной паре
            if (i + 3 >= size) {
                fill0(U32, &j, 2);
                U32[j++] = '\xFF';
                U32[j++] = '\xFD';
                i += 2;
                continue;
            } // второй суррогат отсутствует или неполный
            if ((*c)[i + 2] < check) {
                fill0(U32, &j, 2);
                U32[j++] = (*c)[i++];
                U32[j++] = (*c)[i++];
                continue;
            } // второй суррогат не соответствует формату второго суррогата
            // если оба верные
            fill0(U32, &j, 1);
            check = '\xD8';
            U32[j++] = ((((*c)[i++] - check) << 2) ^ ((*c)[i++] >> 6)) + 1;
            check = '\xDC';
            U32[j++] = ((*c)[i - 1] << 2) ^ ((*c)[i++] - check);
            U32[j++] = (*c)[i++];
        }
    }
    free(*c);
    *c = U32;
    return j;
}

size_t add_del_BOM(unsigned char **c, size_t n, int code) {
    unsigned char *new;
    new = malloc((n + (code == 0) ? -3 : 3) * sizeof(unsigned char));
    if (new == NULL) {
        printf("Problem with memory allocation");
        free(*c);
        *c = NULL;
        return 0;
    }
    if (code == 0) {
        for (size_t i = 3; i < n; i++) {
            new[i - 3] = (*c)[i];
        }
        n -= 3;
    } else {
        new[0] = '\xEF';
        new[1] = '\xBB';
        new[2] = '\xBF';
        for (size_t i = 0; i < n; i++) {
            new[i + 3] = (*c)[i];
        }
        n += 3;
    }
    free(*c);
    *c = new;
    return n;
}

size_t UTF32BE_to_16BE(unsigned char **c, size_t size) {
    unsigned char *UTF16 = malloc(size * sizeof(unsigned char));
    if (UTF16 == NULL) {
        printf("Problem with memory allocation");
        free(*c);
        *c = NULL;
        return 0;
    }
    size_t j = 0, i = 4;
    unsigned char operand;
    UTF16[j++] = '\xFE';
    UTF16[j++] = '\xFF';
    for (; i < size;) {
        i++;
        if ((*c)[i] == 0) {
            i++;
            UTF16[j++] = (*c)[i++];
            UTF16[j++] = (*c)[i++];
        } else {
            operand = '\xD8';
            UTF16[j++] = (((*c)[i] - 1) >> 2) + operand;
            UTF16[j++] = (((*c)[i++] - 1) << 6) + ((*c)[i] >> 2);
            operand = '\xDC';
            UTF16[j++] = ((*c)[i++] & 3) + operand;
            UTF16[j++] = (*c)[i++];
        }
    }
    free(*c);
    *c = UTF16;
    return j;
}

size_t UTF32BE_to_8(unsigned char **c, size_t size, _Bool bom) {
    unsigned char broken = '\xDC';
    unsigned char *UTF8 = malloc(size * sizeof(unsigned char) + ((bom) ? 3 : 0));
    if (UTF8 == NULL) {
        printf("Problem with memory allocation");
        free(*c);
        *c = NULL;
        return 0;
    }
    size_t i = 4, j = 0;
    if (bom) {
        UTF8[j++] = '\xEF';
        UTF8[j++] = '\xBB';
        UTF8[j++] = '\xBF';
    }
    for (; i < size;) {
        i++;
        if ((*c)[i] != 0) {
            UTF8[j++] = ((*c)[i] >> 2) ^ (30 << 3);
            UTF8[j++] = (((*c)[i++] & 3) << 4) ^ ((*c)[i] >> 4) ^ (1 << 7);
            UTF8[j++] = (((*c)[i++] & 15) << 2) ^ ((*c)[i] >> 6) ^ (1 << 7);
            UTF8[j++] = ((*c)[i++] & 63) ^ (1 << 7);
        } else {
            i++;
            if ((*c)[i] == broken) {
                i++;
                UTF8[j++] = (*c)[i++];
                continue;
            }
            if ((*c)[i] >= 8) {
                UTF8[j++] = ((*c)[i] >> 4) ^ (14 << 4);
                UTF8[j++] = (((*c)[i++] & 15) << 2) ^ ((*c)[i] >> 6) ^ (1 << 7);
                UTF8[j++] = ((*c)[i++] & 63) ^ (1 << 7);
            } else {
                if ((*c)[i] == 0 && (*c)[i + 1] < 128) {
                    i++;
                    UTF8[j++] = (*c)[i++];
                } else {
                    UTF8[j++] = ((*c)[i++] << 2) ^ ((*c)[i] >> 6) ^ (6 << 5);
                    UTF8[j++] = ((*c)[i++] & 63) ^ (1 << 7);
                }
            }
        }
    }
    free(*c);
    *c = UTF8;
    return j;
}

_Bool is_UTF32_BE(unsigned char const * const c) {
    unsigned char bom;
    return c[0] == (bom = '\x00') && c[1] == (bom = '\x00') && c[2] == (bom = '\xFE') && c[3] == (bom = '\xFF');
}

_Bool is_UTF32_LE(unsigned char const * const c) {
    unsigned char bom;
    return c[0] == (bom = '\xFF') && c[1] == (bom = '\xFE') && c[2] == (bom = '\x00') && c[3] == (bom = '\x00');
}

_Bool is_UTF16_LE(unsigned char const * const c) {
    unsigned char bom;
    return c[0] == (bom = '\xFF') && c[1] == (bom = '\xFE');
}

_Bool is_UTF16_BE(unsigned char const * const c) {
    unsigned char bom;
    return c[0] == (bom = '\xFE') && c[1] == (bom = '\xFF');
}

_Bool is_UTF8_BOM(unsigned char const * const c) {
    unsigned char bom;
    return c[0] == (bom = '\xEF') && c[1] == (bom = '\xBB') && c[2] == (bom = '\xBF');
}

_Bool is_change_endian(enum Encoding code, enum Encoding out_code) {
    return ((code == UTF32_BE || code == UTF32_LE) && (out_code == UTF32_BE || out_code == UTF32_LE))
    || ((code == UTF16_BE || code == UTF16_LE) && (out_code == UTF16_BE || out_code == UTF16_LE))
    || ((code == UTF8_BOM || code == UTF8) && (out_code == UTF8_BOM || out_code == UTF8));
}

int main(int argc, char**argv) {
    if (argc != 4) {
        printf("Write input, output files, and number of encoder");
        return -1;
    }
    enum Encoding code, out_code = atoi(argv[3]);
    /*
        0 – UTF-8 without BOM;
        1 – UTF-8 with BOM;
        2 – UTF-16 Little Endian;
        3 – UTF-16 Big Endian;
        4 – UTF-32 Little Endian;
        5 – UTF-32 Big Endian.
    */
    FILE *f = fopen(argv[1], "rb");
    if (f == NULL) {
        printf("FIle %s not found", argv[1]);
        return -1;
    }
    unsigned char *c;
    size_t  n = 0, size = 1;
    c = malloc(size * sizeof(unsigned char));
    while (fread((c + n), sizeof(unsigned char), 1, f) == 1) {
        n++;
        if (n >= size) {
            size *= 2;
            unsigned char* tmp = realloc(c, size * sizeof(unsigned char));
            if (tmp != NULL) {
                c = tmp;
            } else {
                printf("Problem with memory allocation");
                free(c);
                fclose(f);
                return -1;
            }
        }
    }
    if (!feof(f)) {
        printf("Problem with reading of Input file");
        fclose(f);
        return -1;
    }
    fclose(f);
    if (n >= 4 && is_UTF32_BE(c)) {
        code = UTF32_BE;
    } else if (n >= 4 && is_UTF32_LE(c)) {
        code = UTF32_LE;
    } else if (n >= 3 && is_UTF8_BOM(c)) {
        code = UTF8_BOM;
    } else if (n >= 2 && is_UTF16_BE(c)) {
        code = UTF16_BE;
    } else if (n >= 2 && is_UTF16_LE(c)) {
        code = UTF16_LE;
    } else {
        code = UTF8;
    }

    if (code != out_code) {
        if (!is_change_endian(code, out_code)) {
            switch (code) {
                case UTF8:
                case UTF8_BOM:
                    n = UTF8_to_32BE(&c, n);
                    if (c == NULL) {
                        return -1;
                    }
                    break;
                case UTF16_LE:
                    change_ending16(c, n);
                case UTF16_BE:
                    n = UTF16BE_to_32BE(&c, n);
                    if (c == NULL) {
                        return -1;
                    }
                    break;
                case UTF32_LE:
                    change_ending32(c, n);
                    break;
                case UTF32_BE: break;
            }
            // from 32BE to needed;
            switch (out_code) {
                case UTF8:
                    n = UTF32BE_to_8(&c, n, 0);
                    if (c == NULL) {
                        return -1;
                    }
                    break;
                case UTF8_BOM:
                    n = UTF32BE_to_8(&c, n, 1);
                    if (c == NULL) {
                        return -1;
                    }
                    break;
                case UTF16_LE:
                    n = UTF32BE_to_16BE(&c, n);
                    if (c == NULL) {
                        return -1;
                    }
                    change_ending16(c, n);
                    break;
                case UTF16_BE:
                    n = UTF32BE_to_16BE(&c, n);
                    if (c == NULL) {
                        return -1;
                    }
                    break;
                case UTF32_LE:
                    change_ending32(c, n);
                    break;
                case UTF32_BE:
                    break;
            }
        } else {
            if ((code == UTF8_BOM || code == UTF8) && (out_code == UTF8_BOM || out_code == UTF8)) {
                n = add_del_BOM(&c, n, out_code);
                if (c == NULL) {
                    return -1;
                }
            } else if ((code == UTF16_BE || code == UTF16_LE) && (out_code == UTF16_BE || out_code == UTF16_LE)) {
                change_ending16(c, n);
            } else {
                change_ending32(c, n);
            }
        }
    }

    f = fopen(argv[2], "wb");
    if (f == NULL) {
        printf("File %s didn't find or create", argv[2]);
        free(c);
        return -1;
    }
    if (fwrite(c, sizeof(unsigned char), n, f) != n) {
        printf("Error in writing");
        free(c);
        fclose(f);
        return -1;
    }
    free(c);
    fclose(f);
    return 0;
}