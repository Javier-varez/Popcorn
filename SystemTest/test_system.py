#!/usr/bin/env python3
""" Test cases for the System Test """

import logging
import pytest
from logic import Logic, AnalyzerData


LOGGER = logging.getLogger(__name__)


@pytest.fixture(scope="module", name='target_data')
def fixture_target_data(logic_analyzer: Logic, data_path: str):
    """" Fixture to capture default target output data """
    logic_analyzer.set_capture_length(10)
    logic_analyzer.trigger_capture()
    file = 'test_cpu_usage.bin'
    logic_analyzer.export_data(data_path, file)
    data = AnalyzerData(data_path + '/' + file)
    return data


def test_context_change_overhead(logic_analyzer: Logic, target_data: AnalyzerData):
    """ Checks CPU context change overhead doesn't exceed the limit of 0.5% """
    percentage = logic_analyzer.get_active_usage(target_data, Logic.Channel.CHANNEL_2) * 100
    LOGGER.info('Context change overhead = %f %%', percentage)
    assert percentage < 0.5


def validate_frequency(measurement, target, tolerance):
    """ Validates a target frequency measurement """
    assert measurement[0] < target + tolerance
    assert measurement[0] > target - tolerance
    assert measurement[1] < tolerance


def test_tasks_work_in_parallel(logic_analyzer: Logic, target_data: AnalyzerData):
    """ Checks there are two toggling GPIOs driven by different tasks
        and toggling at 1Hz and 0.66 Hz
    """
    channel_0 = logic_analyzer.get_toggle_frequency(target_data, Logic.Channel.CHANNEL_0)
    channel_1 = logic_analyzer.get_toggle_frequency(target_data, Logic.Channel.CHANNEL_1)
    LOGGER.info('freq mean = %f, %f', channel_0[0], channel_1[0])
    LOGGER.info('freq stddev = %f, %f', channel_0[1], channel_1[1])
    validate_frequency(channel_0, 0.666666666, 0.001)
    validate_frequency(channel_1, 1.0, 0.001)
