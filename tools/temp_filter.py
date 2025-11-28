import numpy as np
import matplotlib.pyplot as plt
from scipy import signal

def get_alpha( fc, fs ):
    alpha = np.exp( -2 * np.pi * ( fc / fs ) )
    return alpha

def sp_lpf( alpha, length ):
    #dirac = signal.unit_impulse( length )
    dirac = np.zeros(length)
    dirac[30:] = 1
    y = np.zeros( length )
    for i in range( length ):
        y[i] = dirac[i] - alpha*(dirac[i] - y[i-1])
    return y

fs = 1 # 1Hz Temperature measurement
cutoff = 1 / (fs * 20) # every 20s
alpha = get_alpha(cutoff,1)

# Data over 5 minute period
y = sp_lpf(alpha, 1 * 60 * fs)

print(f'alpha = {alpha}')

plt.plot(y)
plt.show()

