import os
from sklearn.externals import joblib
import matplotlib.pyplot as plt


def plot(x, y1, y2, l1, l2, xl, yl, t, path):
    plt.scatter(x, y1, label=l1)
    plt.plot(x, y1)
    plt.scatter(x, y2, label=l2)
    plt.plot(x, y2)
    plt.legend()
    plt.xlabel(xl)
    plt.ylabel(yl)
    plt.title(t)
    plt.savefig(path)
    plt.close()


theirs = []
for f in os.listdir('./tdsl/theirs/'):
    tx_size = int(f[9:-4])
    stats = joblib.load(os.path.join('tdsl', 'theirs', f))
    for stat in zip(*stats):
        theirs += [[tx_size] + list(stat) + [100 * stat[-1] / (stat[-1] + stat[-2])]]
theirs = sorted(theirs)

ours = []
for f in os.listdir('./tdsl/ours/'):
    tx_size = int(f[9:-4])
    stats = joblib.load(os.path.join('tdsl', 'ours', f))
    for stat in zip(*stats):
        ours += [[tx_size] + list(stat) + [100 * stat[-1] / (stat[-1] + stat[-2])]]
ours = sorted(ours)

plot(zip(*ours)[1][:9], zip(*ours)[3][:9], zip(*theirs)[3][:9], 'Ours', 'Theirs', '# of Threads', 'Wall Time', 'Wall Time v.s. # of Threads', 'wall_threads_tx2.png')
plot(zip(*ours)[1][:9], zip(*ours)[6][:9], zip(*theirs)[6][:9], 'Ours', 'Theirs', '# of Threads', '% Aborts', '% Aborts v.s. # of Threads', 'abrt_threads_tx2.png')
plot(zip(*ours)[0][5:-9:9], zip(*ours)[3][5:-9:9], zip(*theirs)[3][5:-9:9], 'Ours', 'Theirs', 'Transaction Size', 'Wall Time', 'Wall Time v.s. Trans. Size', 'wall_tx_th16.png')
plot(zip(*ours)[0][6:-9:9], zip(*ours)[3][6:-9:9], zip(*theirs)[3][6:-9:9], 'Ours', 'Theirs', 'Transaction Size', 'Wall Time', 'Wall Time v.s. Trans. Size', 'wall_tx_th32.png')
plot(zip(*ours)[0][6:-9:9], zip(*ours)[3][6:-9:9], zip(*theirs)[6][6:-9:9], 'Ours', 'Theirs', 'Transaction Size', '% Aborts', '% Aborts v.s. Trans, Size', 'abrt_tx_th32.png')
plot(zip(*ours)[0][5:-9:9], zip(*ours)[3][5:-9:9], zip(*theirs)[6][5:-9:9], 'Ours', 'Theirs', 'Transaction Size', '% Aborts', '% Aborts v.s. Trans, Size', 'abrt_tx_th16.png')

ours2 = []
for f in ['it10000tx2wlm.out']:
    tx_size = 2
    stats = joblib.load(os.path.join('tdsl', 'ours', f))
    for stat in zip(*stats):
        ours2 += [[tx_size] + list(stat) + [100 * stat[-1] / (stat[-1] + stat[-2])]]
ours2 = sorted(ours2)

theirs2 = []
for f in ['it10000tx2wlm.out']:
    tx_size = 2
    stats = joblib.load(os.path.join('tdsl', 'theirs', f))
    for stat in zip(*stats):
        theirs2 += [[tx_size] + list(stat) + [100 * stat[-1] / (stat[-1] + stat[-2])]]
theirs2 = sorted(theirs2)

plot(zip(*ours2)[1][:9], zip(*ours2)[6][:9], zip(*theirs2)[6][:9], 'Ours', 'Theirs', '# of Threads', '% Aborts', '% Aborts v.s. # of Threads', 'abrt_threads_tx2_wlm.png')
plot(zip(*ours2)[1][:9], zip(*ours2)[3][:9], zip(*theirs2)[3][:9], 'Ours', 'Theirs', '# of Threads', 'Wall Time', 'Wall Time v.s. # of Threads', 'wall_threads_tx2_wlm.png')