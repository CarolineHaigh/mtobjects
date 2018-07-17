"""Classes to facilitate interaction with MTObjects C libraries."""
import ctypes as ct


def init_classes(d_type):
    """Initialise the classes required for interaction with C libraries."""

    float_type = d_type
    pixel_type = d_type

    MtImageLocation._fields_ = [("x", ct.c_int16),
                    ("y", ct.c_int16)]

    MtPixel._fields_ = [("location", MtImageLocation),
                    ("value", pixel_type)]

    MtHeap._fields_ = [("entries", ct.POINTER(MtPixel)),
                    ("num_entries", ct.c_int32),
                    ("max_entries", ct.c_int32)]

    MtStack._fields_ = [("entries", ct.POINTER(MtPixel)),
                    ("num_entries", ct.c_int32),
                    ("max_entries", ct.c_int32)]

    MtConnectivity._fields_ = [("neighbours", ct.POINTER(ct.c_int)),
                    ("height", ct.c_int),
                    ("width", ct.c_int)]

    MtNodeAttributes._fields_ = [("volume", float_type),
                    ("power", float_type)]

    MtNode._fields_ = [("parent", ct.c_int32),
                    ("area", ct.c_int32)]

    Image._fields_ = [("data", ct.POINTER(pixel_type)),
                    ("height", ct.c_int16),
                    ("width", ct.c_int16),
                    ("size", ct.c_int32)]

    MtData._fields_ = [("root", ct.POINTER(MtNode)),
                    ("nodes", ct.POINTER(MtNode)),
                    ("node_attributes", ct.POINTER(MtNodeAttributes)),
                    ("heap", MtHeap),
                    ("stack", MtStack),
                    ("img", Image),
                    ("connectivity", MtConnectivity),
                    ("verbosity_level", ct.c_int)]

    MtParameters._fields_ = [("verbosity", ct.c_int),
                           ("bg_variance", ct.c_double),
                           ("gain", ct.c_double),
                           ("move_factor", ct.c_double),
                           ("alpha", ct.c_double),
                           ("min_distance", ct.c_double)]

    MtObjectData._fields_ = [("mt", ct.POINTER(MtData)),
                             ("paras", ct.POINTER(MtParameters)),
                             ("flags", ct.POINTER(ct.c_uint8)),
                             ("relevant_indices", ct.POINTER(ct.c_int32)),
                             ("relevant_indices_len", ct.c_int32),
                             ("closest_significant_ancestors", ct.POINTER(ct.c_int32)),
                             ("main_branches", ct.POINTER(ct.c_int32)),
                             ("main_power_branches", ct.POINTER(ct.c_int32)),
                             ("object_ids", ct.POINTER(ct.c_int32)),
                             ("num_significant_nodes", ct.c_int32),
                             ("num_objects", ct.c_int32),
                             # Pointer to significance test function
                             ("node_significance_test", SIGTEST_TYPE),
                             # Pointer to significant nodes function
                             ("significant_nodes", SIGNODES_TYPE),
                             ("node_significance_test_data", ct.c_void_p),
                             ("node_significance_test_data_free",
                              ct.CFUNCTYPE(ct.c_void_p, ct.POINTER(MtObjectData)))]


class MtImageLocation(ct.Structure):
    pass


class MtPixel(ct.Structure):
    pass


class MtHeap(ct.Structure):
    pass


class MtStack(ct.Structure):
    pass


class MtConnectivity(ct.Structure):
    pass


class MtNodeAttributes(ct.Structure):
    pass


class MtNode(ct.Structure):
    pass


class Image(ct.Structure):
    pass


class MtData(ct.Structure):
    pass


class MtObjectData(ct.Structure):
    pass


class MtParameters(ct.Structure):
    pass

SIGTEST_TYPE = ct.CFUNCTYPE(ct.c_int, ct.POINTER(MtObjectData), ct.c_int32)
SIGNODES_TYPE = ct.CFUNCTYPE(None, ct.POINTER(MtObjectData))
INIT_TYPE = ct.CFUNCTYPE(None, ct.POINTER(MtObjectData))