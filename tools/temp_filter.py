import numpy as np
import matplotlib.pyplot as plt
from matplotlib.figure import Figure
from scipy import signal

def get_alpha( fc, fs ):
    alpha = np.exp( -2 * np.pi * ( fc / fs ) )
    return alpha

    step = np.zeros(length)
    step[11:] = 1
    y = np.zeros( length )
    for i in range( length ):
        y[i] = dirac[i] + alpha*(y[i-1] - step[i])

fs = 1 # 1Hz Temperature measurement
cutoff = 1 / (fs * 20) # every 20s
alpha = get_alpha(cutoff,1)
length = 60

# Data over 1 minute period
step = np.zeros(length)
step[10:] = 1
y = np.zeros( length )
for i in range( length ):
    y[i] = step[i] + alpha*(y[i-1] - step[i])

print(f'alpha = {alpha}')
fig, ax = plt.subplots(figsize=(8,6))
plt.xlabel('Samples')
plt.ylabel('Amplitude')
plt.title('Step response')
ax.plot(step)
ax.plot(y)
ax.legend(['Input','Step Response'])
plt.show()

