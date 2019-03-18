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
from struct import unpack, calcsize

NUM_RUNS = 5


def get_times(exe_path, output_path, wtype, num_threads):
    if os.path.exists(output_path):
        os.remove(output_path)

    for _ in range(NUM_RUNS):
        os.system('%s %d %d >> %s' % (exe_path, wtype, num_threads, output_path))
    lines = open(output_path, 'r').read().splitlines()

    num_ops = []
    num_aborts = []

    for line in lines:
        if line.startswith('Num ops:'):
            num_ops.append(int(line[len('Num ops:') + 1:]))
        elif line.startswith("Num aborts:"):
            num_aborts.append(int(line[len('Num aborts:') + 1:]))

    return np.array(num_ops).mean(), np.array(num_aborts).mean()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('exe_path', type=str, help='Path to experiments exe')
    parser.add_argument('wtype', type=int, help='Workload type')
    parser.add_argument('output_path', type=str, help='Results output path')
    args = parser.parse_args()

    assert os.path.isfile(args.exe_path)

    dirpath = tempfile.mkdtemp()

    num_threads = [0, 2, 4, 8, 16, 24, 32]
    ops = []
    aborts = []

    try:
        for i in num_threads:
            num_ops, num_aborts = get_times(args.exe_path, os.path.join(dirpath, str(i) + '.txt'), args.wtype, i)
            ops.append(num_ops)
            aborts.append(num_aborts)
            print("Num Threads: %d Num ops: %d Num aborts: %d" % (i, num_ops, num_aborts))
    except:
        shutil.rmtree(dirpath)
        raise

    shutil.rmtree(dirpath)

    joblib.dump((num_threads, ops, aborts), args.output_path)


if __name__ == "__main__":
    main()
