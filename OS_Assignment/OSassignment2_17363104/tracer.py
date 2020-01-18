import numpy as np
import matplotlib.pyplot as plt
import random

data = []
b1 = 3
b2 = 100
a= 500
num = 0

def combine(num, offset):
	out  = (num << 8) +offset
	return out


with open('addresses-locality.txt','w') as f:
	for i in range(10000):
		if (i % a) == 0 or num < 10:
			offset = random.randint(0, 255)
			num = random.randint(0, 255)
			a= abs(int(np.random.normal(500, 1)))
		else:
			num = abs(random.randint(max(num - b1, 0), min(num + b1, 255)))
			offset = abs(random.randint(max(offset - b2, 0), min(offset + b2, 255)))
		data.append(num + offset / 256)
		p = str(combine(num, offset))
		f.write(p+'\n')

plt.scatter(range(1,10001), data, s = 0.03)
plt.show()