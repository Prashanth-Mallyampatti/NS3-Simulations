import os

ecmpModes = [0, 1, 2, 3]
queueVals = ["10p", "30p", "60p", "100p", "200p", "300p"]

for mode in ecmpModes:
	for val in queueVals:
		cmd = '''./waf --run "scratch/hw3-3 --EcmpMode={} --QueueSize={}"'''.format(mode, val)
		print("Running " + cmd)
		os.system(cmd + " > Hw3-3/task332-{}-{}.log 2>&1".format(mode, val))
