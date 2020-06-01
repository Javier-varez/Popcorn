#!/usr/bin/env python3
""" System test fixtures used in other modules """

from os import getcwd, mkdir
from shutil import rmtree
import pytest
from logic import Logic


@pytest.fixture(scope='session')
def logic_analyzer():
    """Creates a saleae device for usage during the tests"""
    return Logic()


@pytest.fixture(scope='session')
def data_path():
    """Creates the output data path"""
    path = getcwd() + '/output_data'
    # clean previous test executions
    rmtree(path, ignore_errors=True)
    mkdir(path)
    yield path
    rmtree(path, ignore_errors=True)
