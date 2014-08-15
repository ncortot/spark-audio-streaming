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

    void _play(uint8_t *buffer, size_t size);
    void _start_timer();

  public:

    AudioPlayer();
    ~AudioPlayer();
    bool available();
    void play(uint8_t *buffer, size_t size);
    void play(uint8_t *buffer, size_t size, bool (*callback)(bool));
    void repeat(uint8_t *buffer, size_t size, uint16_t count);
    void beep(uint16_t millis);
};

#endif  // AUDIO_PLAYER_H_
