/* Koro's macros */
#ifndef koro_macros_h
#define koro_macros_h

#define SWAP(x, y)                                                                \
    do                                                                            \
    {                                                                             \
        unsigned char swap_temp[sizeof(x) == sizeof(y) ? (signed)sizeof(x) : -1]; \
        memcpy(swap_temp, &y, sizeof(x));                                         \
        memcpy(&y, &x, sizeof(x));                                                \
        memcpy(&x, swap_temp, sizeof(x));                                         \
    } while (0)

#define PI 3.1415926558

#define SWITCH(x) ((x = (x) ? (0) : (1)))
#undef min
#undef max
#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))

#define nfree(x)    \
    do              \
    {               \
        free((x));  \
        (x) = NULL; \
    } while (0)
#define ndestroy_bitmap(x)      \
    do                          \
    {                           \
        al_destroy_bitmap((x)); \
        (x) = NULL;             \
    } while (0)

#endif
