#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "bmp.h"

int cdf[256];
int freq[256];
int width, height;

float gray_val(unsigned long c) {
    float r, g, b;
    bmp_decode(c, &r, &g, &b);
    return r * 0.3 + g * 0.59 + b * 0.11;
}

void getr(void *pic, long y, long x, int grey, float *r1, float *g1, float *b1) {
    float r, g, b;
    float t1, t2;
    bmp_decode(bmp_get(pic, y, x), &r, &g, &b);
    t1 = g / r;
    t2 = b / r;
    *r1 = grey / 256.0 / (0.3 + 0.59 * t1 + 0.11 * t2);
    *g1 = t1 * *r1;
    *b1 = t2 * *r1;
}

int main(int argc, char const *argv[])
{
    FILE *f;
    long x, y;
    int i, j;
    unsigned char *pic;
    unsigned char *bmp;
    float grey_f;
    int grey_i;
    float r, g, b;
    unsigned char header[32];
    int **grey_list;
    int cdf_min;

    f = fopen(argv[1], "rb");
    fread(header, sizeof(header), 1, f);

    width = header[0x12] + 256 * header[0x13] + 256 * 256 * header[0x14] + 256 * 256 * 256 * header[0x15];
    height = header[0x16] + 256 * header[0x17] + 256 * 256 * header[0x18] + 256 * 256 * 256 * header[0x19];

    pic = (unsigned char *)malloc(BMP_SIZE(width, height));
    bmp = (unsigned char *)malloc(BMP_SIZE(width, height));

    fseek(f, 0, 0);
    fread(pic, BMP_SIZE(width, height), 1, f);

    printf("%dx%d\n", width, height);

    bmp_init(bmp, width, height);

    grey_list = (int **)malloc(sizeof(int *) * width);
    for (i = 0; i < width; ++i) {
        grey_list[i] = (int *)malloc(height * sizeof(int));
    }

    memset(freq, 0, sizeof(freq));
    memset(cdf, 0, sizeof(cdf));

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            long x1, y1;
            if (x < 28) {
                x1 = width + x - 28;
                y1 = y;
            } else {
                x1 = x - 28;
                y1 = y > 1 ? y - 1 : y - 1 + height;
            }
            grey_f = gray_val(bmp_get(pic, x, height - y));
            grey_i = (int)round(256 * grey_f);
            grey_list[x1][y1] = grey_i;
            freq[grey_i]++;
        }
    }

    for (i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            if (i < 1) {
                cdf[i] = freq[i];
            } else {
                for (j = i - 1; j >= 0; --j) {
                    if (freq[j] > 0) {
                        cdf[i] = freq[i] + cdf[j];
                        break;
                    }
                }
            }
        }
    }

    cdf_min = cdf[0];
    for (i = 1; i < 256; ++i)
        if (cdf[i] < cdf_min)
            cdf_min = cdf[i];

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int new_grey = round(255.0 * (cdf[grey_list[x][y]] - cdf_min) / (width * height - cdf_min));
            grey_list[x][y] = new_grey;
            getr(pic, x, y, new_grey, &r, &g, &b);
            bmp_set(bmp, x, y, bmp_encode(r, g, b));
        }
    }

    f = fopen("output.bmp", "wb");
    fwrite(bmp, BMP_SIZE(width, height), 1, f);
    fclose(f);

    free(bmp);
    free(pic);

    return 0;
}
