#!/usr/bin/env python3

import glob
import os
import random

num = 1000000
dict_files = glob.glob('*/*.dict')

for dict_file in dict_files:
  print('Input:', dict_file)

  path, ext = os.path.splitext(dict_file)
  lines = open(dict_file, 'rb').readlines()

  out_file = path + '.' + str(num) + '.rnd_dict'
  with open(out_file, 'wb') as fout:
    for i in range(num):
      fout.write(random.choice(lines))

  print('Output:', out_file)