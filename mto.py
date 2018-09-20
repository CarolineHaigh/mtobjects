import mtolib.main as mto

"""Example program - using original settings"""

# Get the input image and parameters
image, params = mto.setup()

# Pre-process the image
processed_image = mto.preprocess_image(image, params, n=2)

# Build a max tree
mt = mto.build_max_tree(processed_image, params)

# Filter the tree and find objects
id_map, sig_ancs = mto.filter_tree(mt, processed_image, params)

# Relabel objects for clearer visualisation
id_map = mto.relabel_segments(id_map, shuffle_labels=False)

# Generate output files
mto.generate_image(image, id_map, params)
mto.generate_parameters(image, id_map, sig_ancs, params)
