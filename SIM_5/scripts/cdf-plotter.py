import pandas
import numpy
import pylab as plot
import scipy.stats as stats

dataDCTCP = pandas.read_csv("dctcp.csv")
dataTCP = pandas.read_csv("tcp.csv")
allData = [dataDCTCP, dataTCP]

i=0
for x in allData:
    for col in x.columns:
        print(i)
        if i%2 != 0:
            print(i)
            h = x[col]
            h = sorted(h)
            fit = stats.norm.cdf(h, numpy.mean(h), numpy.std(h))

            if i==1:
                plot.plot(h, fit, 'b', label='DCTCP')
            else:
                plot.plot(h, fit, 'y', label='TCP')
            plot.xlabel("Queue length")
            plot.ylabel("Cumulative Fraction")
        i=i+1

plot.legend()
plot.grid()
plot.savefig("task132.png")
