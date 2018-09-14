MTObjects

Python implementation by Caroline Haigh (University of Groningen).

Based on the original C implementation by Paul Teeninga et al [1]

--------------------------

Build instructions:

Python dependencies - pip install:

scikit-image
astropy
matplotlib
Pillow
SciPy
numpy

The program is written for python 3.

To recompile the C libraries, run ./recompile.sh

--------------------------

To get help: python mto.py -h

To run with default parameters: python mto.py [path/to/image.fits]

--------------------------

Arguments:

  -h, --help            Show the help message and exit
  -out  	        Location to save filtered image. Supports .fits and .png filenames
  -par_out		Location to save calculated parameters. Saves in .csv format
  -soft_bias		Constant bias to subtract from the image - not usually necessary
  -gain		        Gain (estimated by default)
  -bg_mean		Mean background (estimated by default)
  -bg_variance		Background variance (estimated by default)
  -alpha	        Significance level - for the original test, this must be 1e-6
  -move_factor          Reduces the spread of large objects.
				Default = 0.5; Lower values can be interesting
  -min_distance         Minimum brightness difference between objects.
				Default = 0.0
  -sub			Set the baseline from which objects' parameters are calculated
				Default = 'minIfNested' - see below for details
  -verbosity {0,1,2}    Verbosity level (0-2)

-------------------------

The -sub argument:

This aims to prevent objects on a bright background from having an artificially inflated flux value in output parameters.
To do this, a value representing the local background is subtracted from all pixels in the object when calculating parameters.
The -sub argument controls how this value is chosen.

Options:
	'min' 			Subtract the minimum valued pixel within the object's bounds
	'minIfNested'		As above, but only applied to nested objects
	'parent'		Subtract the parent pixel from the tree (value may be higher than minimum due to smoothing)
	'parentIfNested'	As above, but only applied to nested objects
	'none'			No subtraction

NB - this parameter is a temporary solution, which we hope to replace with a more robust method in future.


-------------------------

[1] Citing:

@inproceedings{teeninga2015improved,
  title={Improved detection of faint extended astronomical objects through statistical attribute filtering},
  author={Teeninga, Paul and Moschini, Ugo and Trager, Scott C and Wilkinson, Michael HF},
  booktitle={International Symposium on Mathematical Morphology and Its Applications to Signal and Image Processing},
  pages={157--168},
  year={2015},
  organization={Springer}
}

