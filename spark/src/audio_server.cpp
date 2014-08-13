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

#include "audio_server.h"

#define BUFFER_START (_buffer)
#define BUFFER_HALF (_buffer + AUDIO_SERVER_PLAY_BUFFER_SIZE / 2)
#define BUFFER_END (_buffer + AUDIO_SERVER_PLAY_BUFFER_SIZE)


bool volatile _player_active = false;
bool volatile _transfer_complete = false;
bool player_callback(bool transfer_complete);


static bool inline isOpen(long sd)
{
   return sd != MAX_SOCK_NUM;
}


AudioServer::AudioServer() : _sock(MAX_SOCK_NUM)
{
    flush();
}

int AudioServer::listen(uint16_t port, AudioPlayer &player)
{
    int bound = 0;
    sockaddr tUDPAddr;

    if (WiFi.ready()) {
        _sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (_sock >= 0) {
            flush();
            _port = port;

            memset(&tUDPAddr, 0, sizeof(tUDPAddr));
            tUDPAddr.sa_family = AF_INET;
            tUDPAddr.sa_data[0] = (_port & 0xFF00) >> 8;
            tUDPAddr.sa_data[1] = (_port & 0x00FF);

            bound = bind(_sock, (sockaddr*)&tUDPAddr, sizeof(tUDPAddr)) >= 0;

            if (bound) {
                _player = &player;
            } else {
                stop();
            }
        }
    }

    return bound;
}

void AudioServer::stop()
{
    if (isOpen(_sock)) {
        closesocket(_sock);
    }
    _sock = MAX_SOCK_NUM;
    _player = NULL;
    _player_active = false;
}

void AudioServer::flush()
{
    _read_start = BUFFER_START;
    _read_end = BUFFER_HALF;
    _read_underflow = false;

    _player_active = false;
    _transfer_complete = false;
}

/**
 * Try an fill the buffer, start the player when ready.
 */
int AudioServer::loop()
{
    int read_count = 0;

    if (WiFi.ready() && isOpen(_sock)) {
        int len = -1;
        while (len != 0) {
            len = _read_once();
            read_count += len;
        }
    }

    if (!_player_active && _player != NULL && read_count) {
        _player_active = true;
        _player->play(_buffer, AUDIO_SERVER_PLAY_BUFFER_SIZE, &player_callback);
    }

    return read_count;
}

/**
 * Read data from the socket once.
 */
int AudioServer::_read_once()
{
    int len = _read_len();

    if (len > 0) {
        _types_fd_set_cc3000 readSet;
        timeval timeout;

        FD_ZERO(&readSet);
        FD_SET(_sock, &readSet);

        timeout.tv_sec = 0;
        timeout.tv_usec = 5000;

        if (select(_sock + 1, &readSet, NULL, NULL, &timeout) > 0) {
            if (FD_ISSET(_sock, &readSet)) {
                int ret = recvfrom(_sock, _read_start, len, 0, NULL, NULL);
                if (ret > 0) {
                    _read_inc(ret);
                    return ret;
                }
            }
        }
    }

    return 0;
}

/**
 * Return the number of bytes to read from the network.
 *
 * We can read at most AUDIO_SERVER_READ_BUFFER_SIZE.
 */
uint16_t AudioServer::_read_len()
{
    if (_transfer_complete) {
        if (_read_end == BUFFER_HALF) {
            _read_end = BUFFER_END;
            _read_underflow = (_read_start < BUFFER_HALF);
        }
    } else {
        if (_read_end == BUFFER_END) {
            _read_end = BUFFER_HALF;
            _read_underflow = (_read_start >= BUFFER_HALF);
        }
    }

    if (_read_underflow) {
        return AUDIO_SERVER_READ_BUFFER_SIZE;
    }
    uint16_t len = _read_end - _read_start;
    if (len > AUDIO_SERVER_READ_BUFFER_SIZE) {
        return AUDIO_SERVER_READ_BUFFER_SIZE;
    }
    return len;
}

/**
 * Increment the read buffer.
 *
 */
uint16_t AudioServer::_read_inc(uint16_t len)
{
    _read_start += len;

    if (_read_underflow) {
        if (_transfer_complete) {
            _read_underflow = (_read_start < BUFFER_HALF);
        } else {
            _read_underflow = (_read_start >= BUFFER_HALF);
        }
        digitalWrite(D7, _read_underflow ? LOW: HIGH);
    }

    if (_read_start >= BUFFER_END) {
        _read_start = BUFFER_START;
    }
}

bool player_callback(bool transfer_complete)
{
    _transfer_complete = transfer_complete;
    return _player_active;
}
