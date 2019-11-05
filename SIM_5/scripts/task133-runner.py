import os

kVals = [1,2,3,4,5,6,7,8,9,10,15,20,25,30,40,50,60,70,80,90,100]

for k in kVals:
	cmd = '''./waf --run "scratch/hw4 --K={} --dctcp=true"'''.format(k)
	print("Running " +cmd)
	os.system(cmd + " > Hw4/task133-{}.log 2>&1".format(k))
