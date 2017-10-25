#!/bin/python

import matplotlib.pyplot as plot

with open("times") as file:
    times = [float(line) for line in file]

plot.plot(range(1, len(times) + 1), [times[0] / time for time in times], label='S(p)');
plot.plot([1, len(times)], [1, len(times)], label='y=x')
axis = plot.gca()
axis.set_xlabel("p")
axis.set_ylabel("S")
plot.legend()
plot.show()
