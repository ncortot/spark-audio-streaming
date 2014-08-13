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

import numpy
import pysoundfile
import sys


from make_sample import convert_buffer, read_sound


def write_bin(path, data):
    fp = open(path, 'wb')

    # Write bytes MSB first
    data = data.astype(numpy.dtype('>u2'))

    for i in range(0, data.size, 1024):
        fp.write(data[i:i + 1024].tostring())

    fp.close()


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print('Usage: {0} audio_sample.ogg audio_sample.bin')
    else:
        sound = read_sound(sys.argv[1])
        data = convert_buffer(sound)
        write_bin(sys.argv[2], data)
