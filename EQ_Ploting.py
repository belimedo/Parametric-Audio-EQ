%matplotlib notebook
import numpy as np
import scipy.signal as s
import matplotlib.pyplot as plt

""" Ploting start filter coefficinets """
t = open('Lab05\\Debug\\test.txt', 'r')
contents = t.read().splitlines()
f0=125
prvi = True;
linije = np.array(contents)

for i in range(int(linije.size/2)):
    a=np.array(linije[i*2].split('#'), dtype=float)
    b=np.array(linije[i*2+1].split('#'), dtype=float)
    w, H =s.freqz(b,a,worN=4096, whole=False, plot=None, fs=48000)
    plt.figure()     
    plt.plot(w[:2048], 20*np.log10(H[:2048]))
    plt.axvline(x=250, color = 'green', linestyle='--')
    plt.axvline(x=500, color = 'green', linestyle='--')
    plt.axvline(x=2000, color = 'green', linestyle='--')
    plt.axvline(x=4000, color = 'green', linestyle='--')
    plt.axvline(x=100, color = 'red', linestyle='-.')
    plt.axvline(x=350, color = 'red', linestyle='-.')
    plt.axvline(x=1000, color = 'red', linestyle='-.')
    plt.axvline(x=3000, color = 'red', linestyle='-.')
    plt.axvline(x=5000, color = 'red', linestyle='-.')
    if(prvi):
        Hs = np.array(H,ndmin = 2)
        prvi = False;
    else:
        Hs = np.vstack((Hs, H))
T = Hs[0,:]
for i in range(1,int(linije.size/2)):
     T*=Hs[i,:]
plt.figure()     
plt.plot(w[:2048], 20*np.log10(T[:2048]))
plt.axvline(x=250, color = 'green', linestyle='--')
plt.axvline(x=500, color = 'green', linestyle='--')
plt.axvline(x=2000, color = 'green', linestyle='--')
plt.axvline(x=4000, color = 'green', linestyle='--')

""" Ploting changed filter coefficinets """
t = open('Lab05\\Debug\\changed.txt', 'r')
contents = t.read().splitlines()
f0=125
prvi = True;
linije = np.array(contents)

for i in range(int(linije.size/2)):
    a=np.array(linije[i*2].split('#'), dtype=float)
    b=np.array(linije[i*2+1].split('#'), dtype=float)
    w, H =s.freqz(b,a,worN=4096, whole=False, plot=None, fs=48000)
    plt.figure()     
    plt.plot(w[:2048], 20*np.log10(H[:2048]))
    plt.axvline(x=250, color = 'green', linestyle='--')
    plt.axvline(x=500, color = 'green', linestyle='--')
    plt.axvline(x=2000, color = 'green', linestyle='--')
    plt.axvline(x=4000, color = 'green', linestyle='--')
    plt.axvline(x=100, color = 'red', linestyle='-.')
    plt.axvline(x=350, color = 'red', linestyle='-.')
    plt.axvline(x=1000, color = 'red', linestyle='-.')
    plt.axvline(x=3000, color = 'red', linestyle='-.')
    plt.axvline(x=5000, color = 'red', linestyle='-.')
    if(prvi):
        Hs = np.array(H,ndmin = 2)
        prvi = False;
    else:
        Hs = np.vstack((Hs, H))
T = Hs[0,:]
for i in range(1,int(linije.size/2)):
     T*=Hs[i,:]
plt.figure()     
plt.plot(w[:2048], 20*np.log10(T[:2048]))
plt.axvline(x=250, color = 'green', linestyle='--')
plt.axvline(x=500, color = 'green', linestyle='--')
plt.axvline(x=2000, color = 'green', linestyle='--')
plt.axvline(x=4000, color = 'green', linestyle='--')