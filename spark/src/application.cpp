#include "application.h"
#include "audio_player.h"
#include "audio_server.h"
#include "audio_sample.h"

#define AUDIO_PORT 2222


AudioPlayer player;
AudioServer server;


void setup()
{
    player.begin();

    pinMode(D7, OUTPUT);
    digitalWrite(D7, LOW);

    // Play a sine wave for 1s in the background
    player.repeat((uint16_t *) audio_sample, SAMPLE_SIZE, 450);
    // Avoid DMA transfers when playing
    delay(500);

    server.listen(AUDIO_PORT, player);
}


void loop()
{
    server.loop();
}
