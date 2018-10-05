import glob
import os
import random

num = 1000000
dict_files = glob.glob(os.path.dirname(os.path.abspath(__file__)) + '/*/*.dict')

for dict_file in dict_files:
  print('Input:', dict_file)

  path, ext = os.path.splitext(dict_file)
  lines = open(dict_file, 'rb').readlines()

  out_file = path + '.' + str(num) + '.rnd_dict'
  with open(out_file, 'wb') as fout:
    for i in range(num):
      fout.write(random.choice(lines))

  print('Output:', out_file)
