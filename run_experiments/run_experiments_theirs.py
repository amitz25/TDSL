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
    cpu_times = []
    all_times = []
    num_commits = []
    num_aborts = []

    for line in lines:
        if line.startswith('CPU Time: '):
            cpu_times.append(float(line[len('CPU Time: '):line.index('s', len('CPU Time: '))]))
            all_times.append(float(line[line.index('s', len('CPU Time: ')):-1]))
        if line.startswith('Total commits: '):
            num_commits.append(int(line[len('Total commits: '):]))
        elif line.startswith("Total aborts: "):
            num_aborts.append(int(line[len('Total aborts: '):]))

    return np.array(cpu_times).mean(), np.array(all_times).mean(), np.array(num_commits).mean(), np.array(num_aborts).mean()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('exe_path', type=str, help='Path to experiments exe')
    parser.add_argument('wtype', type=int, help='Workload type')
    parser.add_argument('output_path', type=str, help='Results output path')
    args = parser.parse_args()

    assert os.path.isfile(args.exe_path)

    dirpath = tempfile.mkdtemp()

    num_threads = [0, 2, 4, 8, 16, 24, 32]
    cpu_times = []
    all_times = []
    num_commits = []
    num_aborts = []

    try:
        for i in num_threads:
            cpu_time, all_time, num_commit, num_abort = \
                get_times(args.exe_path, os.path.join(dirpath, str(i) + '.txt'), args.wtype, i)
            cpu_times.append(cpu_time)
            all_times.append(all_time)
            num_commits.append(num_commit)
            num_aborts.append(num_abort)
            print("stats: {}\t{}\t{}\t{}".format(i, cpu_time, all_time, num_commit, num_abort))
    except:
        shutil.rmtree(dirpath)
        raise

    shutil.rmtree(dirpath)

    joblib.dump((num_threads, cpu_times, all_times, num_commits, num_aborts), args.output_path)


if __name__ == "__main__":
    main()
