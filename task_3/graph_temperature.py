#!/usr/bin/python

import numpy as numpyp
import matplotlib.pyplot as plot
import fileinput

input = []
counter = 0
for line in fileinput.input():
    if counter == 0:
        h, n, m = line.split(" ")
        h = float(h)
        n = int(n)
        m = int(m)
    else:
        input.append(line)

    counter += 1

matrix = [[float(value) for value in line.split(" ")[:-1]] for line in input][::-1]

x = [h * i + h / 2 for i in range(n)]
y = [h * i + h / 2 for i in range(m)]

plot.pcolormesh(x, y, matrix)
plot.colorbar()

plot.figure()
x = [h * i + h / 2 for i in range(m)]
for level in [0.1, 0.2, 0.3, 0.4]:
    y = matrix[int((level / (n * h) ) * n)]

    plot.subplot()
    plot.plot(x, y, label="horizontal level {}".format(level))
    plot.legend()

plot.figure()
for level in [0.1, 0.2, 0.3, 0.4]:
    y = [matrix[i][int((level / (m * h)) * m)] for i in range(m)]

    plot.subplot()
    plot.plot(x, y, label="vertical level {}".format(level))
    plot.legend()


plot.show();
