import matplotlib.pyplot as plt
import numpy as np
import soundfile as sf

# Leemos el fichero del fonema con la libreria "soundfile"
data, samplerate = sf.read("./fonema_E.wav") 
t = np.arange(0, len(data))/samplerate

# Calculamos la autocorrelación de los datos del fichero de audio
corr = np.correlate(data, data, 'full') / len(data)
corr = corr[int(corr.size/2):] # Consideramos solo la mitad positiva de la autocorrelación

min_index = np.argmin(corr)
max_index = np.argmax(corr[min_index:])
max_value = np.max(corr[min_index:])
fig, axs = plt.subplots(2)
      
# Hacemos la representación gráfica
axs[0].plot(t, data)
axs[1].plot(t, corr)
# Mostramos el valor del primer máximo secundario de la autoccorrelación para determinar el mejor candidato para el periodo de pitch
axs[1].plot((min_index+max_index)/samplerate, max_value, 'ro', label='Pitch Estimation = {}ms'.format((min_index+max_index)*1000/samplerate))

axs[0].set_title('Fonema Sonoro')
axs[1].set_title('Autocorrelación de la señal')
axs[1].set_xlabel('Tiempo (s)')
axs[1].legend()

fig.tight_layout()
plt.show()