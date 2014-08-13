/**

  Simple audio streaming library for the Spark Core
  Copyright (C) 2014 Nicolas Cortot
  https://github.com/ncortot/spark-audio-streaming

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef AUDIO_SERVER_H_
#define AUDIO_SERVER_H_

#include "audio_player.h"

#define AUDIO_SERVER_READ_BUFFER_SIZE 128
#define AUDIO_SERVER_PLAY_BUFFER_SIZE 1024

class AudioServer {

public:

    AudioServer();
    virtual ~AudioServer() {};

    virtual int listen(uint16_t port, AudioPlayer &player);
    virtual void flush();
    virtual void stop();
    virtual int loop();

private:

    long _sock;
    uint16_t _port;
    AudioPlayer *_player;

    uint8_t _buffer[AUDIO_SERVER_PLAY_BUFFER_SIZE];

    uint8_t *_read_start;
    uint8_t *_read_end;
    bool _read_underflow;

    inline int _read_once();
    inline uint16_t _read_len();
    inline uint16_t _read_inc(uint16_t len);
};

#endif // AUDIO_SERVER_H_
