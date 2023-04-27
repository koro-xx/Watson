#ifndef __freesherlock__sound__
#define __freesherlock__sound__

#include <stdlib.h>
#include "macros.h"

#define RESERVED_SAMPLES 64

typedef enum SOUND
{
    SOUND_HIDE_TILE,
    SOUND_GUESS_TILE,
    SOUND_WIN,
    SOUND_WRONG,
    SOUND_UNHIDE_TILE,
    SOUND_CLICK,
    SOUND_STONE,
    NUMBER_OF_SOUNDS
} SOUND;

/* Prototypes */
int init_sound(void);
void play_sound(SOUND s);
void destroy_sound(void);

#endif /* defined(__freesherlock__sound__) */
