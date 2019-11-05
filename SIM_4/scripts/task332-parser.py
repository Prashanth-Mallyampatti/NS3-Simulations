import os

allFiles = [f for f in os.listdir(".") if ".log" in f]

for file in allFiles:
	tmp = file[0:-4].split("-")
	mode = tmp[1]
	queueSize = tmp[2]
	with open(file) as f:
		for line in f.readlines():
			if "calculated flow throughput" in line:
				tp = line.split(" ")[-2]
			if "calculated flow completion time" in line:
				t = line.split(" ")[-2]
	print(mode+","+queueSize[0:-1]+","+tp+","+t)	
			
