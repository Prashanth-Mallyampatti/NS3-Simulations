import os

ecmpModes = [0, 1, 2, 3]
flowVals = [3, 6, 9, 12, 15, 18, 21, 24, 27, 30]
for mode in ecmpModes:
	for flow in flowVals:
		cmd = '''./waf --run "scratch/hw3-2-udp --EcmpMode={} --nFlow={}"'''.format(mode, flow)
		print("Running " + cmd)
		os.system(cmd + "> Hw3-2/task232-{}-{}.log 2>&1".format(mode, flow))
