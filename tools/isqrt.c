#include <stdio.h>

typedef unsigned char u8;
typedef unsigned short u16;

u8 isqrt(u16 v)
{
    u8 x = 0;
    u16 r = 0;

    for (int i = 0; i < 8; i++)
    {
        u16 rn = (r << 2) + (v >> 14);
        if (rn >= 4 * x + 1)
        {
            r = rn - (4 * x + 1);
            x = (x << 1) + 1;
        }
        else
        {
            r = rn;
            x <<= 1;
        }
        v <<= 2;
    }

    return x;
}

int main()
{
    int x;
    scanf("%d", &x);
    printf("%d", (int)isqrt((u16)x));
}