import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns

def plotVarT(ax, path, filename, nVar):
    data = np.fromfile(path+filename)
    T = data[0::6]; var = data[nVar::6];
    #print(var)
    ax.plot(T, var)

path = "../data/"
figdir = "../figurer/"
sns.set()
sns.set_style("darkgrid")
sns.set_palette("Set2")
plt.rc('text', usetex=True)
plt.rc('font', family='serif')


files = ["metropolis40.bin", "metropolis60.bin", "metropolis80.bin", "metropolis100.bin"]
variables = [r"$\left\langle E\right\rangle$",r"$C_V$",r"$\left\langle M\right\rangle$",r"$\chi$"]
legends = ["L=40","L=60"]
names=["energy.pdf","heat_capacity.pdf","magnetization.pdf","susceptibility.pdf"]

for i in [1,2,3,4]:
    fig, ax = plt.subplots()
    for file in files:
        plotVarT(ax, path, file, i)
    ax.legend(legends,loc='upper center', bbox_to_anchor=(0.5, 1.18),fontsize=14, frameon=False, ncol=2)
    ax.set_xlabel("T",fontsize=14)
    ax.set_ylabel(variables[i-1],fontsize=14)
    plt.savefig(figdir+names[i-1])

plt.show()
