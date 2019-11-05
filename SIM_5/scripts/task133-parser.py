import os

allFiles = [f for f in os.listdir(".") if ".log" in f]

for file in allFiles:
	tmp = file[0:-4].split("-")
	k = tmp[1]
	with open(file) as f:
		for line in f.readlines():
			if "calculated flow throughput" in line:
				tp = line.split(" ")[-2]
			if "Avg Queue len" in line:
				ql = line.split(" ")[-1]
	print(k+","+tp+","+ql)
