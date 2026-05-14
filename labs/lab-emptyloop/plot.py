import argparse
import matplotlib.pyplot as plt
import numpy as np
import sys

def load(input_file):
    return np.loadtxt(input_file, dtype={'names': ('start', 'duration'), 'formats': (int, int)}, skiprows=1, delimiter=',')

def time_plot(data, output_file):
    plt.plot(data['start'], data['duration'], linestyle='none', marker='.')
    plt.gcf().set_size_inches(12, 8)
    plt.gca().set_yscale('log')
    plt.savefig(output_file)

def histogram_plot(data, output_file):
    plt.hist(data['duration'], log=True, bins=1000)
    plt.gcf().set_size_inches(12, 8)
    plt.gca().set_yscale('log')
    plt.gca().set_xscale('log')
    plt.savefig(output_file)

def get_argparser():
    parser = argparse.ArgumentParser()
    parser.add_argument('mode', choices=('time', 'histogram'))
    parser.add_argument('input')
    parser.add_argument('output')
    return parser

if __name__ == '__main__':
    parser = get_argparser()
    args = parser.parse_args()
    data = load(args.input)
    if args.mode == 'time':
        time_plot(data, args.output)
    elif args.mode =='histogram':
        histogram_plot(data, args.output)

