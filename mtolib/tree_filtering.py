"""Filter a maxtree."""

import ctypes as ct
import numpy as np

import mtolib.significance_tests as mt_sig
from mtolib import _ctype_classes as mt_class
from mtolib.utils import time_function

# Get access to the compiled C maxtree library
# Defaults to float version
mto_lib = ct.CDLL('mtolib/lib/mt_objects.so')


def init_double_filtering(params):
    """Set up the double version of the maxtree library."""
    global mto_lib

    # If the image is 64 bit, use the double version of the library
    if params.d_type == ct.c_double:
        mto_lib = ct.CDLL('mtolib/lib/mt_objects_double.so')


def up_tree():
    """Process a tree from root to leaves."""
    return mt_class.SIGNODES_TYPE(mto_lib.mt_significant_nodes_up)


def down_tree():
    """Process a tree from leaves to root."""
    return mt_class.SIGNODES_TYPE(mto_lib.mt_significant_nodes_down)


def default_sig_test():
    return mt_sig.default_sig_test(mto_lib)


def get_c_significant_nodes(lib_name):
    """Get a significant nodes function from a compiled library."""

    # Get access to a compiled C mt_object library
    c_lib = ct.CDLL(lib_name)

    return mt_class.SIGNODES_TYPE(c_lib.significant_nodes)


def filter_tree(mt_in, image, params, sig_test=default_sig_test,
                sig_nodes_function=up_tree):
    if params.verbosity:
        print("\n---Finding Objects---")
    return time_function(filter_tree_timed, (mt_in, image, params, sig_test, sig_nodes_function),
                         params.verbosity, 'find objects')


def filter_tree_timed(mt_in, image, params, sig_test=default_sig_test,
                sig_nodes_function=up_tree):
    """Filter a maxtree using a given significance test and processing method,
     and return an object id map"""

    # Convert the maxtree object for ctypes compatibility
    mt = mt_in.ctypes_maxtree()

    # Declare an int type pointer type for the object id array
    object_id_type = ct.POINTER(ct.c_int32)

    # Create an object id array and get a pointer to it
    object_ids = np.zeros(image.shape, dtype=ct.c_int32)
    id_pointer = object_ids.ctypes.data_as(object_id_type)

    # Ditto for significant ancestors
    sig_ancs = np.zeros(image.shape, dtype=ct.c_int32) -3
    sig_anc_pointer = sig_ancs.ctypes.data_as(object_id_type)

    # Get up/down tree functions if necessary
    if sig_nodes_function == up_tree:
        sig_nodes_function = up_tree()
    elif sig_nodes_function == down_tree:
        sig_nodes_function = down_tree()

    # Get sig test if necessary
    if sig_test == default_sig_test:
        sig_test = default_sig_test()

    # Create a parameters object
    mto_params = mt_class.MtParameters(bg_variance=params.bg_variance, gain=params.gain,
                                       move_factor=params.move_factor, alpha=params.alpha,
                                       verbosity=params.verbosity, min_distance=params.min_distance)


    # Create the MTO struct and a pointer
    # Avoids bizarre memory management issues - creating it in C seems to go very wrong
    mto_struct = mt_class.MtObjectData(object_ids=id_pointer, mt=ct.pointer(mt),
                                       paras=ct.pointer(mto_params),
                                       significant_nodes=sig_nodes_function,
                                       node_significance_test=sig_test.test,
                                       closest_significant_ancestors=sig_anc_pointer)

    mto_pointer = ct.pointer(mto_struct)

    sig_test.init_test(mto_pointer)

    # Set up the mt_objects c function interface
    mto_lib.mt_objects.argtypes = [ct.POINTER(mt_class.MtObjectData)]
    mto_lib.mt_objects(mto_pointer)

    return object_ids, sig_ancs
