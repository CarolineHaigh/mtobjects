"""Miscellaneous utilities."""

import time
from astropy.stats import gaussian_fwhm_to_sigma
import argparse


def time_function(function, parameters, verbosity, task_string):
    """Time a function, return the function's output, and print the time taken."""
    start_time = time.time()

    output = function(*parameters)

    if verbosity > 1:
        elapsed_time = (time.time() - start_time)
        print('{:.4} seconds to {}.'.format(elapsed_time, task_string))

    return output


def fwhm_to_sigma(x):
    """Convert the full width at half maximum (FWHM) of a function to a sigma."""
    return x * gaussian_fwhm_to_sigma


def validate_decimal(x):
    """Validate that a value is between zero and one."""
    value = float(x)
    if validate_in_range(value, 0, 1):
        return value
    raise argparse.ArgumentTypeError('Alpha must be between 0 and 1')


def validate_in_range(x, a, b):
    """Validate that a value is in a given range."""
    return a < x < b


def validate_positive(x):
    """Validate that a value is positive."""
    value = float(x)
    if value >= 0:
        return value
    raise argparse.ArgumentTypeError('Move factor and min distance must be positive')
