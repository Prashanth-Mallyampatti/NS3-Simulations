import os

allFiles = [f for f in os.listdir(".") if ".txt" in f]

for file in allFiles:
	tmp = file[0:-4].split("-")
	mode = tmp[1]
	qSize = tmp[4]
	path = tmp[5]
	with open(file) as f:
		for line in f.readlines():
			tmp1 = line.split(" ")
			print(path+","+mode+","+tmp1[0]+","+tmp1[1])
