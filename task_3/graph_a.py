#!/usr/bin/python

import matplotlib.pyplot as plot
import numpy

with open("data") as file:
    times_1 = [numpy.array([float(value) for value in line.split(" ")[:-1]]) for line in file][0]

with open("data") as file:
    times_p = [numpy.array([float(value) for value in line.split(" ")[:-1]]) for line in file][1]

a_1 = [times_1[0] / time for time in times_1]
a_p = [times_p[0] / time for time in times_p]
p = range(1, len(a_1) + 1)

plot.figure()
plot.subplot()
plot.plot([0, 16], [0, 16], label="y = x")
plot.plot(p, a_1, label="a(p) for O(1)")
plot.plot(p, a_p, label="a(p) for O(p)")
axis = plot.gca()
axis.set_xlabel("p")
axis.set_ylabel("a")
plot.legend()
plot.show()
