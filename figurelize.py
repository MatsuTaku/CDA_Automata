#!/usr/bin/env python

import matplotlib.pyplot as plt
import numpy
import sys
import os.path

if __name__ == '__main__':
	argv = sys.argv
	if len(argv) <= 1:
		print("Input error!: ", argv)
		exit()

	figure = plt.figure()
	ax = figure.add_subplot(1,1,1)

	for datas in argv[1:]:
		with open(datas) as f:
			lines = f.readlines()
			sizes = []
			times = []
			for line in lines:
				vals = line.strip().split(',')
				if len(vals) < 3:
					continue
				label, size, time = vals[0], float(vals[1]), float(vals[2])
				sizes.append(size)
				times.append(time)
				ax.annotate(label, (size, time))
			root, ext = os.path.splitext(datas)
			tag1 = ext.split('.')[-1]
			tag2 = tag1.split('_results')[0]
			ax.scatter(sizes, times, label=tag2)

	title, titleext = os.path.splitext(os.path.basename(argv[1]))
	ax.set_title(title)
	ax.set_xlabel('size (Byte)')
	ax.set_ylabel('search time (µs/key)')
	axis = ax.axis()
	ax.axis((0, axis[1], 0, axis[3]))
	ax.legend()
	figure.show()
	root, ext = os.path.splitext(argv[1])
	plot_name = root + '.plot.png'
	figure.savefig(plot_name)
