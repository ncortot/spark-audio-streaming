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


# Target channels
AUDIO_CHANNELS = 1
# Target frequency
AUDIO_FREQUENCY = 44100
# DAC resolution
AUDIO_BITS = 12
# Bits added to every sample for DAC control
DAC_CONTROL = 0x7000

# Store data here when a conversion is required
TEMP_FILE = '_temp_data.wav'


def read_sound(path):
    """Read a sound and convert it if required."""

    source = pysoundfile.SoundFile(path)
    sys.stderr.write('Found source with {0} channels at {1} Hz\n'.format(
        source.channels, source.sample_rate))

    if (
        source.channels == AUDIO_CHANNELS
        and source.sample_rate == AUDIO_FREQUENCY
    ):

        # Return  1-d array
        return numpy.concatenate(source.read())

    sys.stderr.write('Converting to {0} channels {1} Hz\n'.format(
        AUDIO_CHANNELS, AUDIO_FREQUENCY))

    target = pysoundfile.SoundFile(
        TEMP_FILE,
        sample_rate=AUDIO_FREQUENCY,
        channels=AUDIO_CHANNELS,
        format=pysoundfile.wave_file,
        mode=pysoundfile.write_mode,
    )

    target.write(source.read())
    return read_sound(TEMP_FILE)


def convert_buffer(data):
    """Convert sound data for the DAC."""

    # Convert float32 in [-1, 1] to int in [0, max_value]
    max_value = 1 << AUDIO_BITS
    buf = (data * max_value / 2).astype(numpy.uint16) + int(max_value / 2)
    # Add DAC control bits
    buf += DAC_CONTROL
    return buf


def write_samples(data):
    """Write samples as a C array."""

    def format_line(buffer):
        line = numpy.vectorize(hex)(buffer)
        return '    {0}'.format(', '.join(line))

    # Write bytes MSB first
    data = data.astype(numpy.dtype('>u2'))
    lines = (format_line(data[i:i + 8]) for i in range(0, data.size, 8))

    print('// {0}'.format(sys.argv[1]))
    print('')
    print('#define SAMPLE_SIZE {0}'.format(data.size))
    print('')
    print('static const uint16_t audio_sample[SAMPLE_SIZE] = {')
    print(',\n'.join(lines))
    print('};')


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('Usage: {0} audio_sample.ogg')
    else:
        sound = read_sound(sys.argv[1])
        data = convert_buffer(sound)
        write_samples(data)
