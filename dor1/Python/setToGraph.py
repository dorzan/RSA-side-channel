import sys
import numpy as np
import matplotlib.pyplot as plt
import array

mod = int(sys.argv[1])
num_of_samples =int(sys.argv[2])
num_of_sets = int(sys.argv[3])
num_of_slices = int(sys.argv[4])

if mod == 0:
    #SIngal_Times = "/home/ubu/eclipse-workspace/dor1/attacked_routine/Routine_Times.txt"
    #f_res = open(SIngal_Times, "r")
    #times = np.fromfile(f_res, dtype=np.uint64)
   # f_res.close()
    #plt.plot(times,np.ones(len(times)), "*")

    for i in range(num_of_slices):
	
        FILE_ALLSETS = "/home/ubu/eclipse-workspace/dor1/datashit{}.txt".format(i)
        f_res = open(FILE_ALLSETS, "r")
        res = np.fromfile(f_res, dtype=np.int16)
        f_res.close()
        print i, float(sum(res == 0)) / len(res)
        FILE_ALLSETS_TIMES = "/home/ubu/eclipse-workspace/dor1/datashitTimes{}.txt".format(i)
        f_res = open(FILE_ALLSETS_TIMES, "r")
        res_times = np.fromfile(f_res, dtype=np.uint64)
        f_res.close()
    #res_signal_times = times[(times>res_times[0]) & (times<res_times[-1])]
        plt.plot(res_times,- 13 + res + 13 * i)
    #plt.plot(res_times,res + 13 * i,"^")
	#plt.plot(res_signal_times,np.ones(len(res_signal_times))+13*i,"*")
    plt.show()
	
	
if mod == 1:
    print("plotting graphs")
    FILE_ALLSETS = "/home/ubu/eclipse-workspace/dor1/datashit.txt"
    f_res=open(FILE_ALLSETS,"r")
    for i in range (num_of_sets):
        res = np.fromfile(f_res, dtype=np.int16, count=num_of_samples)
        plt.plot(res + 13 * i)
    f_res.close()
    plt.show()

	
if mod == 2:
    FILE_ALLSETS = "/home/ubu/eclipse-workspace/dor1/speSet.txt"
    f_res = open(FILE_ALLSETS, "r")
    res = np.fromfile(f_res, dtype=np.int16)
    f_res.close()
    FILE_ALLSETS_TIMES = "/home/ubu/eclipse-workspace/dor1/speSetTimes.txt"
    f_res = open(FILE_ALLSETS_TIMES, "r")
    res_times = np.fromfile(f_res, dtype=np.uint64)
    f_res.close()
    specRes = res
    print "Active = ", float(sum(specRes > 0)) / len(specRes),
    intervalArr = []
    flag = 0
    for j in range(len(specRes)-1):
        #if (specRes[j]==0) and (specRes[j+1]>0):
        if (specRes[j+1]>specRes[j] and flag == 0):
            intervalArr.append(res_times[j])
            flag =1
        if (specRes[j+1]<specRes[j]):
            flag =0


    intervalArr = np.asarray(intervalArr)
    intervalArr = intervalArr[1:] - intervalArr[:-1]
    
    if num_of_sets > 3:
        intervalHis = [0]*1000
        if len(intervalArr) > 0:
            for i in range(len(intervalArr)):
                if intervalArr[i]<1000000:
                    intervalHis[int(intervalArr[i]/1000)]+=1

        plt.plot(intervalHis)
        plt.show()
        #plt.plot(res_times,(specRes>0) )
    #plt.plot(res_times,np.ones(len(res_times)),"*")
        #plt.show()
        plt.plot(intervalArr)
        plt.show()
		
		
'''if mod == 2:
    print("rape")
    FILE_ALLSETS = "/home/ubu/eclipse-workspace/dor1/mean.txt"
    f_mean = open(FILE_ALLSETS, "r")
    res_mean = int(f_mean.readline().strip())
				  
    f_mean.close()
    print res_mean
    FILE_ALLSETS = "/home/ubu/eclipse-workspace/dor1/speSet.txt"
    f_res = open(FILE_ALLSETS, "r")
    res = np.fromfile(f_res, dtype=np.int16)
    f_res.close()
    FILE_ALLSETS_TIMES = "/home/ubu/eclipse-workspace/dor1/speSetTimes.txt"
    f_res = open(FILE_ALLSETS_TIMES, "r")
    res_times = np.fromfile(f_res, dtype=np.uint64)
    f_res.close()
    specRes = res
    print "Active = ", float(sum(specRes > 0)) / len(specRes),
    intervalArr = []

    for j in range(len(specRes)-1):
        #if (specRes[j]==0) and (specRes[j+1]>0):
        if (specRes[j+1]>specRes[j]):
            intervalArr.append(res_times[j])

    intervalArr = np.asarray(intervalArr)
    intervalArr = intervalArr[1:] - intervalArr[:-1]
    score = sum((intervalArr > 8000) & (intervalArr < 60000))
    print "Score = ", score



    res_mean += score
    FILE_ALLSETS = "/home/ubu/eclipse-workspace/dor1/mean.txt"
    f_mean = open(FILE_ALLSETS, "w")
    f_mean.write("{}".format(res_mean))
    f_mean.close()
    
    
    if num_of_sets > 3:
        intervalHis = [0]*1000
        if len(intervalArr) > 0:
            for i in range(len(intervalArr)):
                if intervalArr[i]<1000000:
                    intervalHis[int(intervalArr[i]/1000)]+=1

        plt.plot(intervalHis)
        plt.show()
        #plt.plot(res_times,(specRes>0) )
    #plt.plot(res_times,np.ones(len(res_times)),"*")
        #plt.show()
        plt.plot(intervalArr)
        plt.show()
'''
        
        
        
if mod == 3:
    FILE_ALLSETS = "/home/ubu/eclipse-workspace/dor1/mean.txt"
    f_mean = open(FILE_ALLSETS, "r")
    res_mean = int(f_mean.readline().strip())
				  
    f_mean.close()
    print res_mean
    FILE_ALLSETS = "/home/ubu/eclipse-workspace/dor1/speSet.txt"
    f_res = open(FILE_ALLSETS, "r")
    res = np.fromfile(f_res, dtype=np.int16)
    f_res.close()
    FILE_ALLSETS_TIMES = "/home/ubu/eclipse-workspace/dor1/speSetTimes.txt"
    f_res = open(FILE_ALLSETS_TIMES, "r")
    res_times = np.fromfile(f_res, dtype=np.uint64)
    f_res.close()
    specRes = res
    print "Active = ", float(sum(specRes > 0)) / len(specRes),
    intervalArr = []

    for j in range(len(specRes)-1):
        if (specRes[j]==0) and (specRes[j+1]!=0):
            intervalArr.append(res_times[j])

    intervalArr = np.asarray(intervalArr)
    intervalArr = intervalArr[1:] - intervalArr[:-1]

    #score = sum((intervalArr > 100000) & (intervalArr < 150000))  
    #print "Score = ", score
    res_mean += score
    FILE_ALLSETS = "/home/ubu/eclipse-workspace/dor1/mean.txt"
    f_mean = open(FILE_ALLSETS, "w")
    f_mean.write("{}".format(res_mean))
    f_mean.close()
    if num_of_sets > 3:
        plt.plot(res_times,(specRes>0) )
        #plt.plot(res_times,np.ones(len(res_times)),"*")
        plt.show()
        plt.plot(intervalArr)
        plt.show()

# plt.plot(res)
# plt.show()

