"""Functions to generate statistics and labels from object maps"""

import numpy as np
import warnings
from skimage.color import label2rgb


def colour_labels(label_map):
    """Apply a colour to each object in the image non-sequentially and convert to 8-bit int RGB format"""
    return np.uint8(label2rgb(label_map)*255)


def relabel_segments(label_map, shuffle_labels=False):
    """Relabel segments with sequential numbers"""

    original_shape = label_map.shape

    label_map = label_map.ravel()
    output = np.zeros(label_map.shape, dtype=label_map.dtype) -1

    # Sort the object ID map for faster pixel retrieval
    sorted_ids = label_map.argsort()
    id_set = list(set(label_map))
    id_set.sort()
    
    id_set.remove(-1)
    
    # Get the locations in sorted_ids of the matching pixels
    right_indices = np.searchsorted(label_map, id_set, side='right', sorter=sorted_ids)
    left_indices = np.searchsorted(label_map, id_set, side='left', sorter=sorted_ids)
    
    # Generate a list of labels, starting from 1
    label_list = list(range(1, 1 + len(id_set)))
    
    # Shuffle order in which labels are allocated
    if shuffle_labels:
        np.random.shuffle(label_list)
        
    # Relabel pixels
    for n in range(len(id_set)):
        pixel_indices = np.unravel_index(sorted_ids[left_indices[n]:right_indices[n]], label_map.shape)

        output[pixel_indices] = label_list[n]

    return output.reshape(original_shape)


def levelled_segments(img, label_map):
    """Replace object ids with the value at which the object was detected"""
    output = np.zeros(img.shape)

    label_map = label_map.ravel()

    # Sort the object ID map for faster pixel retrieval
    sorted_ids = label_map.argsort()
    id_set = list(set(label_map))

    id_set.remove(-1)

    # Get the locations in sorted_ids of the matching pixels
    right_indices = np.searchsorted(label_map, id_set, side='right', sorter=sorted_ids)
    left_indices = np.searchsorted(label_map, id_set, side='left', sorter=sorted_ids)

    for n in range(len(id_set)):
        pixel_indices = np.unravel_index(sorted_ids[left_indices[n]:right_indices[n]], img.shape)

        pixel_values = img[pixel_indices]

        min_value = np.min(pixel_values)

        output[pixel_indices] = min_value

    return output


def get_image_parameters(img, object_ids, sig_ancs, params,):
    """Calculate the parameters for all objects in an image"""

    # Treat warnings as exceptions
    warnings.filterwarnings('error', category=RuntimeWarning, append=True)

    parameters = []
    headings = ['ID', 'X', 'Y', 'A', 'B', 'theta',  # 'kurtosis',
                           'total_flux', 'mu_max', 'mu_median', 'mu_mean', 'R_fwhm', 'R_e', 'R10', 'R90']

    parameters.append(headings)

    # Sort the object ID map for faster pixel retrieval
    sorted_ids = object_ids.argsort()
    id_set = list(set(object_ids))

    if -1 in id_set:
        id_set.remove(-1)

    # Get the locations in sorted_ids of the matching pixels
    right_indices = np.searchsorted(object_ids, id_set, side='right', sorter=sorted_ids)
    left_indices = np.searchsorted(object_ids, id_set, side='left', sorter=sorted_ids)

    # For each object in the list, get the pixels, calculate parameters, and write to file
    for n in range(len(id_set)):

        pixel_indices = np.unravel_index(sorted_ids[left_indices[n]:right_indices[n]], img.shape)
        parameters.append(get_object_parameters(img, id_set[n], pixel_indices))

    # Return to printing warnings
    warnings.resetwarnings()

    return parameters


def get_object_parameters(img, node_id, pixel_indices):
    """Calculate an object's parameters given the indices of its pixels"""
    p = [node_id]

    # Get pixel values for an object
    pixel_values = np.nan_to_num(img.data[pixel_indices])

    # Subtract min values if required
    pixel_values -= max(np.min(pixel_values), 0)

    flux_sum = np.sum(pixel_values)

    # Handle situations where flux_sum is 0 because of minimum subtraction
    if flux_sum == 0:
        almost_zero = np.nextafter(flux_sum,1)
        flux_sum = almost_zero
        pixel_values[pixel_values == 0] = almost_zero

    # Get first-order moments
    f_o_m = get_first_order_moments(pixel_indices, pixel_values, flux_sum)
    p.extend(f_o_m)

    # Get second-order moments
    second_order_moments = [*get_second_order_moments(pixel_indices, pixel_values, flux_sum, *f_o_m)]
    p.extend(second_order_moments)

    p.append(flux_sum)
    p.extend(get_basic_stats(pixel_values))

    radii, half_max = get_light_distribution(pixel_values, flux_sum)
    p.append(half_max)
    p.extend(radii)

    return p


def get_basic_stats(pixel_values):
    """Return basic statistics about a pixel distribution"""
    return np.max(pixel_values), np.median(pixel_values), np.mean(pixel_values)


def get_first_order_moments(indices, values, flux_sum):
    """Find the weighted centre of the object"""

    x = np.dot(indices[1], values)/flux_sum
    y = np.dot(indices[0], values)/flux_sum

    return x, y


def get_second_order_moments(indices, values, flux_sum, x, y):
    """Find the second order moments of the object"""
    x_indices = indices[1]
    y_indices = indices[0]

    x_pows = np.power(x_indices,2)
    y_pows = np.power(y_indices,2)

    # Find the second order moments
    x2 = (np.sum(np.dot(x_pows, values))/flux_sum) - np.power(x, 2)
    y2 = (np.sum(np.dot(y_pows, values))/flux_sum) - np.power(y, 2)
    xy = (np.sum(x_indices * values * y_indices)/flux_sum) - (x * y)

    lhs = (x2 + y2)/2
    rhs = np.sqrt(np.power((x2 - y2)/2, 2) + np.power(xy,2))

    # Find the major/minor axes
    with np.errstate(invalid='raise'):
        try:
            major_axis = np.sqrt(lhs + rhs)
        except:
            major_axis = 0

        try:
            minor_axis = np.sqrt(lhs - rhs)
        except:
            minor_axis = 0

    # Solve for theta - major axis angle
    try:
        t = np.arctan((2 * xy) / (x2 - y2))
    except RuntimeWarning:
        t = 0

    # Shift theta to the correct value
    if xy < 0 < t:
        theta = (t - np.pi)/2
    elif t < 0 < xy:
        theta = (t + np.pi)/2
    else:
        theta = t/2

    # # Calculate kurtosis
    # if major_axis < 10:
    #     x4 = (x_indices - x) ** 4
    #     y4 = (y_indices - y) ** 4
    #
    #     X4, Y4 = np.meshgrid(x4, y4)
    #
    #     m4x = np.sum(values * X4) / flux_sum
    #     m4y = np.sum(values * Y4) / flux_sum
    #
    #     sx = np.sqrt(x2 + np.power(x, 2))
    #     sy = np.sqrt(y2 + np.power(y, 2))
    #
    #     kx = m4x / sx ** 4
    #     ky = m4y / sy ** 4
    #
    #     kurtosis = (kx + ky) / 2
    # else:
    #     kurtosis = -99

    return major_axis, minor_axis, theta


def get_light_distribution(pixel_values, flux_sum):

    # Sort pixels into order
    sorted_pixels = np.sort(pixel_values)

    # Half maximum radius

    # Find half the maximum pixel value
    half_max = np.max(pixel_values) * 0.5

    # Find the number of pixels of at least that value
    area = sorted_pixels.size - np.searchsorted(sorted_pixels, half_max)
    half_max_rad = find_radius(area)

    # Nth percentile area

    # Get the cumulative sums of the reverse sorted pixels
    summed_pixels = np.cumsum(sorted_pixels[::-1])

    # Find the flux contained by n% of the object's pixels
    thresholds = np.array([0.5, 0.1, 0.9]) * flux_sum

    # Find the first instance where the cumulative sum is over each threshold
    areas = np.searchsorted(summed_pixels, thresholds)

    # Convert the areas to approximate radii
    radii = find_radius(areas)

    return radii, half_max_rad


def find_radius(area):
    """Calculate the radius of a circle of a given radius"""
    return np.sqrt(area/np.pi)
