"""Input/output functions."""

from astropy.io import fits
import sys
import numpy as np
import argparse
from . import utils, postprocessing
from PIL import Image
import csv
import warnings
from astropy.utils.exceptions import AstropyDeprecationWarning


def get_file_extension(filename):
    """Get the extension part of a given filename."""

    if "." in filename:
        return filename.split(".")[-1]
    else:
        return None


def read_fits_file(filename):
    """Open a .fits file.
       Return the first data frame as a numpy array.
    """

    # Open the file
    try:
        hdulist = fits.open(filename)
        img_data = None
        hdu_index = 0

        while img_data is None:
            # Extract image data from file and close
            try:
                img_data = hdulist[hdu_index].data
                hdu_index += 1

            except IndexError:
                print("Could not find image data in file.")
                hdulist.close()
                sys.exit(1)

        hdulist.close()

        if img_data.dtype != np.double:
            img_data = img_data.astype(np.float)

        return img_data

    except IOError:
        print("Could not read file:", filename)
        sys.exit(1)


def get_fits_header(filename):
    """Get the headers of the first frame of a .fits file"""
    try:
        # Open the file and read the header
        hdulist = fits.open(filename)
        header = hdulist[0].header
        hdulist.close()
        return header

    except IOError:
        print("Could not read file:", filename)
    except:
        print("Could not read header")
    finally:
        # Create an empty header if one could not be found
        header = fits.Header()

    return header


def write_fits_file(data, header=None, filename='out.fits'):
    """Create a new fits object from data and headers, and write to file."""
    # Create hdu objects
    primary_hdu = fits.PrimaryHDU(data[0])

    if header is not None:
        primary_hdu.header = header

    image_hdus = [fits.ImageHDU(d) for d in data[1:]]

    # Ignore clobber warning
    warnings.filterwarnings('ignore', category=AstropyDeprecationWarning, append=True)

    # Write to file
    hdulist = fits.HDUList([primary_hdu, *image_hdus])
    hdulist.writeto(filename, clobber=True)  # Clobber deprecated for astropy 1.3
    hdulist.close()


def generate_image(img, object_ids, p,
                   levelled=False):
    """Save an image in .png or .fits format.

    The 'levelled' parameter generates an extra fits dataframe where objects are shown at their
    detection level.
    """

    if p.verbosity:
        print("\n---Generating segmentation map---")

    extension = get_file_extension(p.out)

    data = [object_ids.reshape(img.shape)]

    if levelled:
        data.append(postprocessing.levelled_segments(img, object_ids))

    if extension == "fits":
        if p.filename is not None:
            # Get original header
            header = get_fits_header(p.filename)
            # Write to file
            write_fits_file(data, header=header, filename=p.out)
        else:
            write_fits_file(data, filename=p.out)

    else:
        output = postprocessing.colour_labels(object_ids)

        Image.fromarray(np.flipud(output)).save(p.out)

    if p.verbosity:
        print("Saved output to", p.out)


def generate_parameters(img, object_ids, sig_ancs, p):
    """Write detected object parameters into a csv file"""

    if p.verbosity:
        print("\n---Calculating parameters---")

    object_ids = object_ids.ravel()

    # Mask NANs for parameter calculations
    img = np.ma.array(img, mask=np.isnan(img))

    # Open file and create a writer
    with open(p.par_out, 'w') as csvfile:
        param_writer = csv.writer(csvfile)

        param_writer.writerows(postprocessing.get_image_parameters(img, object_ids, sig_ancs, p))

    if p.verbosity:
        print("Saved parameters to", p.par_out)


# TODO move?
def make_parser():
    """Create an argument parser for MTObjects."""
    parser = argparse.ArgumentParser(description='Find objects in a fits file')
    parser.add_argument('filename', type=str, help='Location of input .fits file')
    parser.add_argument('-out', type=str, help='Location to save filtered image. '
                                               'Supports .fits and .png filenames', default='out.png')
    parser.add_argument('-par_out', type=str, help='Location to save output parameters (csv format).', default='parameters.csv')
    parser.add_argument('-soft_bias', type=float, help='Constant to subtract', default=None)
    parser.add_argument('-gain', type=float, help='Gain in electrons per ADU', default=-1)
    parser.add_argument('-bg_mean', type=float, help='Mean background', default=None)
    parser.add_argument('-bg_variance', type=float, help='Background variance', default=-1)
    parser.add_argument('-alpha', type=utils.validate_decimal, help='significance level', default=1e-6)
    parser.add_argument('-move_factor', type=utils.validate_positive, help='Moves up the object marker', default=0.5)
    parser.add_argument('-min_distance', type=utils.validate_positive,
                         help='Minimum brightness distance between objects', default=0.0)
    parser.add_argument('-verbosity', type=int, help='Verbosity level (0-2)', choices=range(0, 3), default=0)

    return parser
