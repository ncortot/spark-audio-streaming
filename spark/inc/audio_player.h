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

#ifndef AUDIO_PLAYER_H_
#define AUDIO_PLAYER_H_

#include "spark_wiring.h"

class AudioPlayer
{
  private:

    void _setup_dma();
    void _setup_spi();
    void _setup_timer();

  public:

    AudioPlayer();
    ~AudioPlayer();
    void begin();
    bool available();
    void play(uint16_t *buffer, size_t size);
    void play(uint16_t *buffer, size_t size, bool (*callback)(bool));
    void repeat(uint16_t *buffer, size_t size, uint16_t count);
    void beep(uint16_t millis);

    inline void play(uint8_t *buffer, size_t size) {
        play((uint16_t *) buffer, size / 2);
    };

    inline void play(uint8_t *buffer, size_t size, bool (*callback)(bool)) {
        play((uint16_t *) buffer, size / 2, callback);
    };

    inline void repeat(uint8_t *buffer, size_t size, uint16_t count) {
        repeat((uint16_t *) buffer, size / 2, count);
    };
};

#endif  // AUDIO_PLAYER_H_
