#!/usr/bin/env python

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
