from __future__ import print_function

import argparse
import numpy as np
import glob
import os
import tempfile
import shutil
import json
from scipy.sparse import csr_matrix, vstack
from sklearn.externals import joblib
import matplotlib.pyplot as plt

NUM_RUNS = 5


def get_times(output_path, wtype, num_threads):
    if os.path.exists(output_path):
        os.remove(output_path)

    for _ in range(NUM_RUNS):
        os.system('mvn exec:java -Dexec.mainClass="Experiments" -Dexec.args="%d %d" > %s 2>NUL' % (wtype, num_threads, output_path))
    lines = open(output_path, 'r').read().splitlines()

    num_ops = []
    num_aborts = []

    for line in lines:
        if line.startswith('Num ops:'):
            num_ops.append(int(line[len('Num ops:') + 1:]))
        elif line.startswith("Num aborts:"):
            num_aborts.append(int(line[len('Num aborts:') + 1:]))

    return np.array(num_ops).mean(), np.array(num_aborts).mean()


def preprocess_ops(nops):
    nops = np.array(nops)
    nops /= 1e6
    return nops


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('input_path_1', type=str)
    parser.add_argument('input_path_2', type=str)
    parser.add_argument('title', type=str)
    args = parser.parse_args()

    assert os.path.isfile(args.input_path_1)
    assert os.path.isfile(args.input_path_2)

    nthread_1, nops_1, naborts_1 = joblib.load(args.input_path_1)
    nthread_2, nops_2, naborts_2 = joblib.load(args.input_path_2)

    nops_1 = preprocess_ops(nops_1)
    nops_2 = preprocess_ops(nops_2)

    plt.scatter(nthread_1, nops_1, label='Ours')
    plt.plot(nthread_1, nops_1)
    plt.scatter(nthread_2, nops_2, label='Theirs')
    plt.plot(nthread_2, nops_2)
    plt.legend()
    plt.xlabel("# Of Threads")
    plt.ylabel("Millions Of Ops")
    plt.title(args.title)
    plt.show()

if __name__ == "__main__":
    main()
