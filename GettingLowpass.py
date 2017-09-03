import socket
import struct
import numpy as np
import time
import matplotlib
import sys
from matplotlib import pyplot as plt
import glob, os
from read_events import read_events
from read_events import skip_header

#Setting paths
computer_path = '/Users/HHG'
data_path = computer_path + '/FilesRepositories'
'''
Important info for the paths:

1. Make sure that the plot_path has the same folder names (+ _py)
as the tdrDir to have a control between the read files of the dynapse
recordings and their respective extracted matrices.
2. If you are reading the first set of recordings you should save the 
matrices directly in the "MeanMatrix" folder. This will be the initial matrix
that later will concatenate the new matrices from later recordings 
and which should be saved in "NotAveraged" folder.
'''

#Path were matrices will be saved
plot_path = data_path + '/Python/NeuronsMiddle_py/SameBiases_15_py/NotAveraged/'
#Path where dynapse-recordings will be moved once they are read in this script
read_path = '/Users/HHG/FilesRepositories/Dynapse/NeuronsMiddle/SameBiases_15/Read/'
#Path where dynapse-recordings are located
tdrDir = '/Users/HHG/FilesRepositories/Dynapse/NeuronsMiddle/SameBiases_15/Read/'
dataDir = tdrDir
os.chdir(dataDir)

'''
Note that each of this sets will contain different data sets for each
temporal frequency used. The order is from faster stimulus to slower
The name of the data set specifies the parameters set in the python 
script that generates the grating stimulus:
R : right
sf : spatial frequency in cycles per 300 pixel
ph : sec per cycle 

The following table contains the Temporal Frequency and Spatial frequency 
values that should be use for the plots for each of the sf and ph values 

'''


#Spatial frequency to be read in cycles/300 pixels (see table and comment above)
spatial_freq = 0.7
recset = 1
#Reading data when a Right stimulus was played
RdataList = glob.glob("%d-R*sf%s*.aedat" % (recset, str(spatial_freq)))
#Reading data when a Left stimulus was played
LdataList = glob.glob("%d-L*sf%s*.aedat" % (recset, str(spatial_freq)))

#Checking if there are files in our path
if(len(RdataList) == 0 and len(LdataList) == 0):
	print "No files to read"
	sys.exit()
#Opening each data file in each data set
LdataListRead = []
RdataListRead = []

for i in RdataList:
	RdataListRead.append(open(i,"rb"))
	debug = False
for i in LdataList:
	LdataListRead.append(open(i,"rb"))
	debug = False

print "The length of the first data set is "
print len(RdataListRead)
print "The length of the second data set is "
print len(LdataListRead)

#Boolean vector that will indicate if the file in each set is opened or closed
Rdone_reading = [False]*len(RdataListRead)
Ldone_reading = [False]*len(LdataListRead)
#Skipping header of data files
for data in RdataListRead:
	skip_header(data)
for data in LdataListRead:
	skip_header(data)

#Id of the neurons in the populations:
neuron_ID = range(35,44) + range(51,60) + range(66,74)
neuron_ID_1 = range(35,44) + range(51,60) + range(66,74)
#neuron_ID = [35,36,37,38,39,41,43,52,56,58,66,68,69,70,72,73]
#neuron_ID_1 = [35,36,37,38,39,41,43,52,56,58,66,68,69,70,72,73]

num_neurons = len(neuron_ID) #Number of neurons with same receptive fields
num_datasets = 2 #Right and Left
num_tempfreq = 11 #Number of different temporal frequencies of the stimulus
num_recordings = 1 #How many times we record all the temporal frequencies set




'''
The following matrices will contain the responses of 9 non-direction-selectivity
neurons (in each core cores c0:core 0, c2: core 2 or c3:core 3), for each temporal
frequency and each time we record the activity of dynapse with same stimuli
Theoretically we want the core 0 neurons to have a frequency drop later that 
core 3 neurons. Core 2 neurons are receiving excitatory connections from all
9 non-DS neurons of core 0 and inhibitory connections from all 9 no
1  c1  3
4  c1  1
8  c1  0n-Ds neurons
of core 3, hopefully these neurons will show a bandpass filter response to temporal
frequency stimuli 
'''

Rneuron_Rds = np.zeros((num_neurons, num_tempfreq, num_recordings))
Rneuron_InhR = np.zeros((num_neurons, num_tempfreq, num_recordings))
Rneuron_InhL = np.zeros((num_neurons, num_tempfreq, num_recordings))

Lneuron_InhR = np.zeros((num_neurons, num_tempfreq, num_recordings))
Lneuron_Rds = np.zeros((num_neurons, num_tempfreq, num_recordings))
Lneuron_InhL = np.zeros((num_neurons, num_tempfreq, num_recordings))

#Reading data:
for rec in range(num_recordings):
	for k in range(2):
		if(k == 0):
			dataListRead = RdataListRead
			done_reading = Rdone_reading
			datafiles = len(RdataListRead)
		elif(k == 1):
			dataListRead = LdataListRead
			done_reading = Ldone_reading
			datafiles = len(LdataListRead)
		for i in range(datafiles):
			chip_id_tot = []
			core_id_tot = []
			neuron_id_tot = []
			ts_tot = []
			spec_type_tot = []
			spec_ts_tot = []
			while(done_reading[i] == False):
				try:
					core_id, chip_id, neuron_id, ts, spec_type, spec_ts = read_events(dataListRead[i])
					core_id_tot.extend(np.array(core_id))
					chip_id_tot.extend(np.array(chip_id))
					neuron_id_tot.extend(np.array(neuron_id))
					ts_tot.extend(np.array(ts))
					spec_type_tot.extend(np.array(spec_type))
					spec_ts_tot.extend(np.array(spec_ts))
				except NameError:# as e:
					#print str(e)
					dataListRead[i].close()
					done_reading[i] = True
	
			neuron_id_c0 = []
			ts_id_c0 = []
			neuron_id_c1 = []
			ts_id_c1 = []
			neuron_id_c2 = []
			ts_id_c2 = []
			neuron_id_c3 = []
			ts_id_c3 = []
			neuron_nondsc0 = []
			neuron_nondsc2 = []

	#Getting all neurons spiking in chip 0 core 0:
			for chip, core, neur, ts in zip(chip_id_tot, core_id_tot, neuron_id_tot, ts_tot):
				if chip == 1 and core == 0:
					neuron_id_c0.append(neur)
					ts_id_c0.append(ts)
	#Getting all neurons spiking in chip 0 core 1:
			for chip, core, neur, ts in zip(chip_id_tot, core_id_tot, neuron_id_tot, ts_tot):
				if chip == 1 and core == 1:
					neuron_id_c1.append(neur)
					ts_id_c1.append(ts)
	#Getting all neurons spiking in chip 0 core 2:
			for chip, core, neur, ts in zip(chip_id_tot, core_id_tot, neuron_id_tot, ts_tot):
				if chip == 1 and core == 2:
					neuron_id_c2.append(neur)
					ts_id_c2.append(ts)
			if(k == 0):
				for n,ID in enumerate(neuron_ID):
					# Rneuron_InhL[n,i,rec] = neuron_id_c2.count(ID) / ((ts_tot[-1]-ts_tot[0])*(10**-6))
					Rneuron_Rds[n,i,rec] = neuron_id_c0.count(ID) / ((ts_tot[-1]-ts_tot[0])*(10**-6))
				for n,ID in enumerate(neuron_ID_1):
					Rneuron_InhR[n,i,rec] = neuron_id_c1.count(ID) / ((ts_tot[-1]-ts_tot[0])*(10**-6))

			elif(k == 1):
				for n,ID in enumerate(neuron_ID):
					Lneuron_InhL[n,i,rec] = neuron_id_c2.count(ID) / ((ts_tot[-1]-ts_tot[0])*(10**-6))
					Lneuron_Rds[n,i,rec] = neuron_id_c0.count(ID) / ((ts_tot[-1]-ts_tot[0])*(10**-6))
				for n,ID in enumerate(neuron_ID_1):
					Lneuron_InhR[n,i,rec] = neuron_id_c1.count(ID) / ((ts_tot[-1]-ts_tot[0])*(10**-6))


print "All the data has been read"

print "Averaging over the number of recordings..."
#This code will save a 2D Matrix(neuron, tempfreq) since it will already 
#averaged over the recording sets


Rneuron_Rds = np.mean(Rneuron_Rds,2)
Rneuron_InhR = np.mean(Rneuron_InhR,2)
Rneuron_InhL = np.mean(Rneuron_InhL,2)
Lneuron_Rds = np.mean(Lneuron_Rds,2)
Lneuron_InhR = np.mean(Lneuron_InhR,2)
Lneuron_InhL = np.mean(Lneuron_InhL,2)


print "Saving matrices..."
#Saving matrices

if(len(RdataList) != 0):
	#np.save(plot_path + '%dn-R%sactivity_InhL.npy' % (recset, str(spatial_freq)), Rneuron_InhL)
	np.save(plot_path + '%d-R%sactivity_Rds.npy' % (recset, str(spatial_freq)), Rneuron_Rds)
	np.save(plot_path + '%d-R%sactivity_InhR.npy' % (recset, str(spatial_freq)), Rneuron_InhR)

if(len(LdataList) != 0):
	np.save(plot_path + '%d-L%sactivity_Rds.npy' % (recset, str(spatial_freq)), Lneuron_Rds)
	np.save(plot_path + '%d-L%sactivity_InhR.npy' % (recset, str(spatial_freq)), Lneuron_InhR)
	#np.save(plot_path + '%d-L%sactivity_InhL.npy' % (recset, str(spatial_freq)), Lneuron_InhL)

print "Moving read files to the Read directory..."
for j in range (len(RdataList)):
	os.rename(tdrDir + RdataList[j] , read_path + RdataList[j] )
for j in range (len(LdataList)):
    os.rename(tdrDir + LdataList[j] , read_path + LdataList[j] )











