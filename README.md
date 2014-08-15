spark-audio-streaming
=====================

Audio streaming and playback project for the Spark Core, a tiny Wi-Fi development kit.  https://www.spark.io/

Plays sound on pin A0 of the spark core using PWM.

To send sound to the Spark Core, use:

  server/stream.py <core ip> 2222 <server/audio_sample.ogg>

You need the [PySoundFile](https://pypi.python.org/pypi/PySoundFile/0.2.1) library to run the streamer.
