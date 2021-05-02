import matplotlib.pyplot as mpl
import numpy as np


mix_quad_x = [
  [ -1,  1, -1, 1],
  [ -1, -1,  1, 1],
  [  1,  1,  1, 1],
  [  1, -1, -1, 1]
]

mixes = mix_quad_x

def mixer(inputs):
  output = np.array([0.0, 0.0, 0.0, 0.0])
  
  for mi, m in enumerate(mixes):
    for i,v in enumerate(inputs):
      output[mi] = output[mi] + v * m[i]
  
  mmax = max(output)
  mmin = min(output)
  
  mrange = mmax - mmin
  
  if(mrange < 1.0):
    mrange = 1.0
  
  # print(inputs)
  # print(output)
  return output #/mrange - mmin/mrange #+ mmax/mrange
  


inputs = np.linspace(
  [-1.0, -1.0, 1.0, 1.0], 
  [1.0, 1.0, -1.0, 1.0], 
  2048)

out = list(map(mixer, inputs))
# print(out)



motors = []
for i in range(4):
  motors.append([m[i] for m in out])
  # mpl.stackplot([ i[1] for i in inputs], )
  
for m in range(4):
  mpl.plot([ i[0] for i in inputs], motors[m], label='M'+str(m+1))

mpl.legend(loc='upper left')
mpl.show()