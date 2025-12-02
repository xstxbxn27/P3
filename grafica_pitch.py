import matplotlib.pyplot as plt
import numpy as np
import soundfile as sf

# Leemos el fichero del fonema con la libreria "soundfile"
data, samplerate = sf.read("./prueba.wav") 

# Cargamos los datos obtenidos con nuestro sistema
f0 = np.loadtxt("prueba.f0", dtype=float)
f0ref = np.loadtxt("prueba.f0ref", dtype=float)

time = np.arange(0,len(f0)).astype(float)/samplerate

# Hacemos la representación gráfica
fig, axs = plt.subplots(1, 1)
axs.plot(time, f0ref, 'r', label='Pitch referencia')
axs.plot(time, f0, 'g', label='Pitch estimado')
axs.set_xlim((time[0], time[-1]))
axs.set_xlabel('Tiempo [s]')
axs.set_ylabel('Pitch [Hz]')
axs.set_title('Comparación de pitch')
axs.grid(which='both', color='#777777', linestyle=':', linewidth=0.5)

fig.tight_layout()
plt.legend()
plt.show()