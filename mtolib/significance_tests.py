"""Statistical tests for max tree filtering."""

from scipy import stats
from mtolib._ctype_classes import SIGTEST_TYPE, INIT_TYPE
import numpy as np


NO_PARENT = -3


class SignificanceTest:
    """A container class for statistical tests on the maxtree."""
    def __init__(self, test_function, init_function):
        self.test = SIGTEST_TYPE(test_function)
        self.init_test = INIT_TYPE(init_function)

    def setup_test(self, mto):
        mto.node_significance_test = self.test

        self.init_test(mto)


class MtoAccess:
    """Wrapper for mto objects, to simplify python data access."""
    def __init__(self, mto):
        self.mto = mto.contents
        self.mt = self.mto.mt.contents

        self.img = self.mt.img.data
        self.attributes = self.mt.node_attributes
        self.nodes = self.mt.nodes
        self.flags = self.mto.flags

        self.closest_sig_ancs = self.mto.closest_significant_ancestors
        self.main_branches = self.mto.main_branches
        self.main_power_branches = self.mto.main_power_branches

        self.paras = self.mto.paras.contents
        self.sqr_gain = self.paras.gain ** 2
        self.std = np.sqrt(self.paras.bg_variance)

        self.sig_level = self.paras.alpha

    def get_area(self, node):
        return self.nodes[node].area

    def get_parent(self, node):
        return self.nodes[node].parent

    def get_vol(self, node):
        return self.attributes[node].volume

    def get_pow(self, node):
        return self.attributes[node].power

    def get_value(self, node):
        return self.img[node]

    def get_sig_anc_value(self, node):
        return self.get_value(self.get_sig_anc(node))

    def get_sig_anc(self, node):
        return self.closest_sig_ancs[node]

    def has_sig_anc(self, node):
        return self.get_sig_anc(node) != NO_PARENT

    def has_sig_dec(self, node_id):
        return self.main_branches[node_id] != NO_PARENT

    def is_significant(self, node_id):
        return self.flags[node_id] & 1


def default_sig_test(mto_lib):
    """Return the default (original) significance test"""
    return SignificanceTest(mto_lib.mt_node_test_4,
                            mto_lib.mt_use_node_test_4)
