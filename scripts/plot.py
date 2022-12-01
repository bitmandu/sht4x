#!/usr/bin/env python
"""Plot SHT4x data from serial output."""

import argparse
import matplotlib.pyplot as plt
import numpy as np
import serial
import serial.tools.list_ports as list_ports
import sys


def plot(monitor, prefix):
    """Plot data from serial monitor."""

    fig = plt.figure()
    ax1 = fig.add_subplot(2, 1, 1)
    ax2 = fig.add_subplot(2, 1, 2)

    x = []
    temp = []
    humidity = []

    n = len(prefix)

    while 1:
        line = monitor.read_until().decode()
        print(line, end="")

        if line[:n] != prefix:
            continue

        row = [float(x) for x in line[n:].split()]
        x.append(row[0])
        temp.append(row[1])
        humidity.append(row[2])

        x = x[-40:]
        temp = temp[-40:]
        humidity = humidity[-40:]

        ax1.clear()
        ax1.plot(x, temp, "-o")
        ax1.set_xlabel("Iteration")
        ax1.set_ylabel("Temperature (°C)")
        ax1.legend(["mean = {:.1f} °C".format(np.mean(temp))])

        ax2.clear()
        ax2.plot(x, humidity, "-o")
        ax2.set_xlabel("Iteration")
        ax2.set_ylabel("Relative Humidity")
        ax2.legend(["mean = {:.1f}%".format(np.mean(humidity))])
        plt.pause(0.5)

    plt.show()


if __name__ == "__main__":
    p = argparse.ArgumentParser(prog="plot.py", description=__doc__)
    p.add_argument("-b", dest="baud", type=int, default=115200, help="baud rate")
    p.add_argument("-f", dest="prefix", type=str, default="**", help="filter prefix")
    p.add_argument("-p", dest="port", type=str, help="serial port")
    args = p.parse_args()

    port = args.port if args.port else list_ports.comports()[0].device

    with serial.Serial(port, args.baud, timeout=5) as monitor:
        plot(monitor, args.prefix)
