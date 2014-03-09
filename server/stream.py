#!/usr/bin/env python
#
# Simple audio streaming library for the Spark Core
# Copyright (C) 2014 Nicolas Cortot
# https://github.com/ncortot/spark-audio-streaming
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import asynchat
import asyncore
import itertools
import numpy
import socket
import sys
import time

from make_sample import AUDIO_FREQUENCY, convert_buffer, read_sound

PACKET_SIZE = 512


def stream_buffer(host, port, data):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.connect((host, int(port)))

    packet_samples = int(PACKET_SIZE / data.dtype.itemsize)
    packet_data = itertools.cycle(
        data[i:i + packet_samples].tostring()
        for i in range(0, data.size, packet_samples)
    )

    sys.stderr.write('Streaming...\n')

    packet_period = packet_samples / AUDIO_FREQUENCY;
    packet_count = 10000

    group_start = time.time();
    packet_start = group_start;

    while (True):
        for i in range(packet_count):
            try:
                sock.send(next(packet_data))
                packet_start += packet_period
                packet_sleep = packet_start - time.time()
                if packet_sleep > 0:
                    time.sleep(packet_sleep)
            except Exception as e:
                sys.stderr.write('E')
                print(e)
                time.sleep(1)

        group_stop = time.time()
        sample_count = packet_count * packet_samples
        sample_time = group_stop - group_start
        group_start = group_stop
        sys.stderr.write('{0} Hz\n'.format(int(sample_count / sample_time)))


if __name__ == '__main__':
    if len(sys.argv) != 4:
        sys.stderr.write('Usage: {0} <spark ip> <spark port> <audio file>\n')
    else:
        data = convert_buffer(read_sound(sys.argv[3]))
        stream_buffer(sys.argv[1], int(sys.argv[2]), data)
