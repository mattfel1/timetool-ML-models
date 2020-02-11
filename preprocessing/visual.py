import csv
import matplotlib.pyplot as plt
import matplotlib
import pandas as pd
matplotlib.use('tkagg')
import sys

print("Args are: fileId, plotType (heat or line), comma-separated rows (for line only)")

fileId = int(sys.argv[1])
plotType = sys.argv[2]

if (plotType == 'line'):
        rows = [int(x) for x in sys.argv[3].split(',')]

file = '/u1/coffee/2dtimetool_simulation_data/raw/./chirp-2000_interferedelay1650_photonen6.0_carriertagdiamond_nfibers109_netalon1_1.00_1.00_interference.out.' + str(fileId)

data = pd.read_csv(file, skiprows=7, sep='\t', header=None)
data = data.iloc[:, :-1]

# Plot one row

if plotType == 'line':
        plt.plot(data.iloc[rows].T)
else:
        plt.pcolor(data.T)

plt.show(block=False)
input("Press Enter to continue...")
plt.close('all')
