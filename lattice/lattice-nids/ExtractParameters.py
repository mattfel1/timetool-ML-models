#!/usr/bin/env python
# coding: utf-8



import tensorflow as tf
import tensorflow_lattice as tfl
import re
import csv
import os
from shutil import copyfile
import argparse
import sys


# Get checkpoint file
cp_file = "checkpoint" 
output_dir = "."
cp_path = output_dir + "/" + cp_file
fhandle = open ( cp_path,"r" )
lines = fhandle.readlines()
fhandle.close()

# Assume last line is final model checkpoint
last_line = lines[-1]
prefix = "all_model_checkpoint_paths: \""
suffix = "\"\n"
model = last_line[len(prefix):-len(suffix)]
model_path =  "./" + output_dir + "/" + model

# Read checkpoint file and get dictionary keys to model
reader = tf.train.NewCheckpointReader(model_path)
tensor_dict =reader.get_variable_to_shape_map() 
keys = tensor_dict.keys()


# Set up lists to store params
lattice_parameter_keys = []
calibrator_in_keys = []
calibrator_out_keys = []

# Regex patterns to match on
lp_pattern = 'tfl_calibrated_rtl/lattice_[0-9]+/hypercube_lattice_parameters'
cal_in_pattern = 'tfl_calibrated_rtl/pwl_calibration/[a-zA-Z_]+_keypoints_inputs'
cal_out_pattern = 'tfl_calibrated_rtl/pwl_calibration/[a-zA-Z_]+_keypoints_outputs'

# Extract parameters from dictionary
for key in keys:
    if re.match(lp_pattern, key):
        lattice_parameter_keys.append(key)
        
    if re.match(cal_in_pattern, key):
        calibrator_in_keys.append(key)
        
    if re.match(cal_out_pattern, key):
        calibrator_out_keys.append(key)

# Write params to a file
dirname = "LATTICE_NETWORK_"+str(len(lattice_parameter_keys))
if(not os.path.isdir(dirname)):
    os.mkdir(dirname)
lattice_csv_filename = dirname + "/" + "LATTICE_PARAMS.csv"


with open(lattice_csv_filename, 'w+') as csvfile:
    writer = csv.writer(csvfile, delimiter=",", quotechar='|', quoting=csv.QUOTE_MINIMAL)

    for key in lattice_parameter_keys:

        params = reader.get_tensor(key)[0]
        row = []
        for p in params:
            row.append(p)

        writer.writerow(row)
        
        
cal_in_dirname = "CALIBRATOR_INPUT_PARAMS"
if(not os.path.isdir(dirname + "/" + cal_in_dirname)):
    os.mkdir(dirname + "/" + cal_in_dirname)
    
cal_prefix = 'tfl_calibrated_rtl/pwl_calibration/'
for key in calibrator_in_keys:
    key_csv = key[len(cal_prefix):] + ".csv"
    
    with open(dirname+'/'+cal_in_dirname+'/'+key_csv, 'w+') as csvfile:
        writer = csv.writer(csvfile, delimiter=",", quotechar='|', quoting=csv.QUOTE_MINIMAL)
        row = reader.get_tensor(key)
        writer.writerow(row)
    

cal_out_dirname = "CALIBRATOR_OUTPUT_PARAMS"
if(not os.path.isdir(dirname + "/" + cal_out_dirname)):
    os.mkdir(dirname + "/" + cal_out_dirname)
    
cal_prefix = 'tfl_calibrated_rtl/pwl_calibration/'
for key in calibrator_out_keys:
    key_csv = key[len(cal_prefix):] + ".csv"
    
    with open(dirname+'/'+cal_out_dirname+'/'+key_csv, 'w+') as csvfile:
        writer = csv.writer(csvfile, delimiter=",", quotechar='|', quoting=csv.QUOTE_MINIMAL)
        row = reader.get_tensor(key)
        writer.writerow(row)
    


rtl_src = output_dir + "/" + "rtl_structure.csv"
rtl_dst = dirname + "/" + "rtl_structure.csv"

copyfile(rtl_src, rtl_dst)


