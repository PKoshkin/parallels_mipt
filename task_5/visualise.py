#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys, random
from PyQt5.QtWidgets import QWidget, QApplication
from PyQt5.QtGui import QPainter, QColor, QPen
from PyQt5.QtCore import Qt, QBasicTimer

import PyQt5.QtCore

class Visualizator(QWidget):
    def __init__(self, interval, file_name):
        super().__init__()

        self.matrixes = []
        with open(file_name) as file:
            for line in file:
                self.matrix_height = int(line[:-1].split(' ')[0])
                self.matrix_width = int(line[:-1].split(' ')[1])
                self.steps = int(line[:-1].split(' ')[2])
                break
            for i in range(self.steps):
                counter = 0
                new_matrix = []
                for line in file:
                    new_matrix.append([int(value) for value in line[:-2].split(' ')])
                    counter += 1
                    if counter >= self.matrix_height:
                        break
                self.matrixes.append(new_matrix)

        self.rect_side = 1000 / max(self.matrix_height, self.matrix_width)

        self.iteration = 0
        self.timer = QBasicTimer()
        self.init_ui(self.rect_side * self.matrix_height, self.rect_side * self.matrix_width)
        self.timer.start(interval * 1000, self)

    def init_ui(self, height, width):
        self.setGeometry(0, 0, width, height)
        self.setWindowTitle("Visualization")
        self.show()

    def paintEvent(self, event):
        painter = QPainter()
        painter.begin(self)
        
        # need to paint self.iteration matrix
        for i in range(self.matrix_height):
            for j in range(self.matrix_width):
                if self.matrixes[self.iteration][i][j] == 1:
                    painter.fillRect(i * self.rect_side, j * self.rect_side, self.rect_side, self.rect_side, PyQt5.QtCore.Qt.black)

        painter.end()

    def timerEvent(self, event):
        if event.timerId() == self.timer.timerId():
            self.iteration += 1
            if self.iteration == self.steps:
                self.iteration = 0
            self.update()

if __name__ == '__main__':
    application = QApplication(sys.argv)
    visualizator = Visualizator(0.1, "data")
    sys.exit(application.exec_())
