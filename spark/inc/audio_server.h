#ifndef AUDIO_SERVER_H_
#define AUDIO_SERVER_H_

#include "audio_player.h"

#define AUDIO_SERVER_READ_BUFFER_SIZE 512
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

    sockaddr _remoteSockAddr;
    socklen_t _remoteSockAddrLen;

    uint8_t _buffer[AUDIO_SERVER_PLAY_BUFFER_SIZE];

    uint8_t *_read_start;
    uint8_t *_read_end;
    bool _read_underflow;

    inline uint16_t _read_len();
    inline uint16_t _read_inc(uint16_t len);

    inline int isWanReady();
};

#endif // AUDIO_SERVER_H_
