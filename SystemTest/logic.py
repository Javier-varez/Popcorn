#!/usr/bin/env python3
""" Abstraction for the internal logic analyzer implementation
    based on a Saleae logic analyzer """

import struct
from enum import Enum
import saleae
import numpy


class Logic:
    """ logic analyzer base class """
    def __init__(self):
        """ Creates the analyzer and sets default settings """
        self.analyzer = saleae.Saleae(quiet=True)
        self.sample_rate = 24000000
        self.sample_time = 10
        self.analyzer.set_sample_rate_by_minimum(self.sample_rate)

    def set_capture_length(self, seconds):
        """ Sets the amount of time for the capture settings """
        self.sample_time = seconds
        self.analyzer.set_capture_seconds(self.sample_time)

    def trigger_capture(self):
        """ Triggers a capture and waits for completion """
        self.analyzer.capture_start_and_wait_until_finished()

    def export_data(self, path, name):
        """" Exports captured data to file with binary format """
        self.analyzer.export_data2(path + '/' + name, format='binary',
                                   each_sample=False, word_size=8)

    class Channel(Enum):
        """ Enum containing valid channels """
        CHANNEL_0 = 1
        CHANNEL_1 = 2
        CHANNEL_2 = 4
        CHANNEL_3 = 8
        CHANNEL_4 = 16
        CHANNEL_5 = 32
        CHANNEL_6 = 64
        CHANNEL_7 = 128

    def get_active_usage(self: 'Logic', data: 'AnalyzerData', channel: Channel) -> float:
        """ Gets the total time in active level (1) with respect to the sample time """
        active_data = []
        for record in data.get_records():
            current_state = record[1] & channel.value
            if record[0] != 0:
                if current_state != prev_state:
                    if current_state:
                        start = record[0]
                    else:
                        end = record[0]
                        active_data.append(float(end - start) / self.sample_rate)
            prev_state = current_state
        return numpy.sum(active_data) / self.sample_time

    def get_toggle_frequency(self: 'Logic',
                             data: 'AnalyzerData',
                             channel: Channel) -> (float, float):
        """ Gets the toggling frequency of a channel """
        freq_array = []
        prev_timestamp = 0
        prev_state = 0
        for record in data.get_records():
            current_state = record[1] & channel.value
            current_timestamp = record[0]
            if current_timestamp != 0:
                if current_state != prev_state:
                    # We need to discard timestamp 0, as it is not a transition
                    if prev_timestamp != 0:
                        current_period = current_timestamp - prev_timestamp
                        freq_array.append(self.sample_rate / float(current_period))
                    prev_timestamp = current_timestamp
            prev_state = current_state
        freq_mean = numpy.mean(freq_array)
        freq_stddev = numpy.std(freq_array)
        return freq_mean, freq_stddev


class AnalyzerData:
    """ Encapsulates data for a given logic analyzer capture for all channels
        Records are stored in a list containing a tuple with (timestamp, state)
    """
    # pylint: disable=too-few-public-methods
    def __init__(self, file_path: str):
        self.records = []
        with open(file_path, 'rb') as file:
            record_length = 9
            while True:
                data = file.read(record_length)
                if len(data) != record_length:
                    break
                record = struct.unpack('<QB', data)
                self.records.append(record)

    def get_records(self):
        """ Returns the records in the data """
        return self.records
