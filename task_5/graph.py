#!/usr/bin/python

import numpy as numpyp
import matplotlib.pyplot as plot
import fileinput

times = [float(line[:-1]) for line in fileinput.input()]

print(times)

plot.plot(range(1, len(times) + 1), [times[0] / time for time in times], label="S(p)")
axis = plot.gca()
axis.set_title("S(p)")
axis.set_xlabel("p")
axis.set_ylabel("S(p)")

plot.figure()
plot.plot(range(1, len(times) + 1), [(times[0] / times[i]) / (i + 1) for i in range(len(times))], label="E(p)")
axis = plot.gca()
axis.set_title("E(p)")
axis.set_xlabel("p")
axis.set_ylabel("E(p)")

plot.show();
