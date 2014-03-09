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
import sys

from make_sample import AUDIO_FREQUENCY, convert_buffer, write_samples


def make_sine(frequency):
    period_samples = int(AUDIO_FREQUENCY / frequency)
    x = numpy.linspace(-numpy.pi, numpy.pi, period_samples)
    return numpy.sin(x[:-1])


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('Usage: {0} frequency')
    else:
        data = make_sine(int(sys.argv[1]))
        data = convert_buffer(data)
        write_samples(data)
