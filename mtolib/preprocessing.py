"""Image pre-processing functions."""

import numpy as np
from scipy import stats
import scipy.ndimage.filters as filters

from mtolib import background, utils


def preprocess_image(img, p, gaussian_blur=True, n=2, nan_value=np.inf):
    """Estimate an image's background, subtract it, smooth and truncate."""

    # Estimate and subtract the background
    estimate_background(img, p)
    new_img = subtract_background(img, p.bg_mean)

    # Smooth the image
    if gaussian_blur:
        new_img = smooth_image(new_img, n)

    # Remove negative values and NANs
    new_img = replace_nans(truncate(new_img), nan_value)

    return new_img


def estimate_background(img, p):
    """Estimate background mean & variance"""

    if p.bg_mean is None or p.bg_variance < 0:

        if np.isnan(img).any():
            if p.verbosity > 0:
                print("WARNING: image contains NAN values which may affect output parameters")

        bg_mean_tmp, bg_variance_tmp = utils.time_function(background.estimate_bg,
                                                           (img, p.verbosity), p.verbosity,
                                                           "estimate background")

        if p.bg_mean is None:
            p.bg_mean = bg_mean_tmp

        if p.bg_variance < 0:
            p.bg_variance = bg_variance_tmp

    estimate_gain(img, p)

    if p.verbosity:
        print("\n---Background Estimates---")
        print("Background mean: ", p.bg_mean)
        print("Background variance: ", p.bg_variance)
        print("Gain: ", p.gain, " electrons/ADU")


def estimate_gain(img, p):
    """Estimate gain."""

    # Negative gains break sig test 4 - estimated gain should be positive
    if p.gain < 0:
        image_minimum = np.nanmin(img)
        if image_minimum < 0:
            p.soft_bias = image_minimum

        p.gain = (p.bg_mean - p.soft_bias) / p.bg_variance


def subtract_background(img, value):
    """Subtract the background from an image and truncate."""
    return img - value


def truncate(img):
    """Set all negative values in an array to zero."""
    return img.clip(min=0)


def smooth_image(img, n=2):
    """Apply a gaussian smoothing function to an image."""
    return filters.gaussian_filter(img, utils.fwhm_to_sigma(n))


def replace_nans(img, value=np.inf):
    if value == 0:
        return np.nan_to_num(img)
    else:
        img[np.isnan(img)] = value
        return img
