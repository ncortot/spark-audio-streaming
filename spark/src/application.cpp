#include "application.h"
#include "audio_player.h"
#include "audio_server.h"

#define AUDIO_PORT 2222


AudioPlayer player;
AudioServer server;


void setup()
{
    // Show when the buffer is properly filled
    pinMode(D7, OUTPUT);
    digitalWrite(D7, LOW);

    // Listen for music!
    player.beep(250);
    server.listen(AUDIO_PORT, player);
}


void loop()
{
    server.loop();
}
