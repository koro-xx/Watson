#include "sound.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include "main.h"

ALLEGRO_SAMPLE *sound_sample[NUMBER_OF_SOUNDS];

char *sound_sample_filename[] = {
    [SOUND_HIDE_TILE] = "sounds/click-hide.wav",
    [SOUND_GUESS_TILE] = "sounds/click-guess.wav",
    [SOUND_WIN] = "sounds/win.wav",
    [SOUND_WRONG] = "sounds/wrong.wav",
    [SOUND_UNHIDE_TILE] = "sounds/click-unhide.wav",
    [SOUND_CLICK] = "sounds/click-sound.wav",
    [SOUND_STONE] = "sounds/stone.wav"};

int init_sound(void)
{
    int i, err = 0;

    al_init_acodec_addon();

    if (!al_install_audio())
    {
        errlog("failed to initialize the audio!\n");
        return -1;
    }

    deblog("initialized audio addon");
    for (i = 0; i < NUMBER_OF_SOUNDS; i++)
    {
        if (!(sound_sample[i] = al_load_sample(sound_sample_filename[i])))
        {
            errlog("Error loading sample %s\n", sound_sample_filename[i]);
            err = 1;
        }
    }

    deblog("loaded samples");

    al_reserve_samples(RESERVED_SAMPLES);
    if (err)
        return -1;
    else
        return 0;
};

void play_sound(SOUND s)
{
    if (sound_sample[s])
    {
        al_play_sample(sound_sample[s], 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    }
};

void destroy_sound(void)
{
    int i;
    for (i = 0; i < NUMBER_OF_SOUNDS; i++)
    {
        if (sound_sample[i])
            al_destroy_sample(sound_sample[i]);
    }
}
