import numpy as np
import time
import matplotlib
import sys
from matplotlib import pyplot as plt
import glob, os

'''
This script will take the last averaged matrix saved and if there exists a new matrix from
a later recording set, it will concatenate these two and only save the new averaged matrix 
'''
computer_path = '/Users/HHG'
data_path = computer_path + '/FilesRepositories'
mean_matrix_path = data_path + '/Python/NeuronsMiddle_py/SameBiases_14_py/MeanMatrix/'
notaveraged_matrix_path = data_path + '/Python/NeuronsMiddle_py/SameBiases_14_py/NotAveraged/'
averaged_matrix_path = data_path + '/Python/NeuronsMiddle_py/SameBiases_14_py/Averaged/'

recordings = [3,4,5,6]
sf = [0.5, 0.7, 1.0, 1.4, 2.1]
tf = 11
#LOAD MEAN MATRIX IF EXIST:
Exist = False
'''
if((os.path.exists(mean_matrix_path + 'Ractivity_RRds.npy')) == True):
	RRds_Mean_matrix = np.load(mean_matrix_path + 'Ractivity_RRds.npy')
	RRds_Mean_matrix = np.expand_dims((RRds_Mean_matrix), axis = 2)
	RInhR_Mean_matrix = RInhR = np.load(mean_matrix_path+ 'Ractivity_RInhR.npy')
	RInhR_Mean_matrix = np.expand_dims((RInhR_Mean_matrix), axis = 2)
	print "Hello there"
	Exist = True
'''
	
print Exist
for j,recset in enumerate(recordings):
	for i,spatial_freq  in enumerate(sf):
		#Loading mean matrices
		RRds = np.load(notaveraged_matrix_path + '%d-R%sactivity_Rds.npy' % (recset ,str(spatial_freq)))
		RInhR = np.load(notaveraged_matrix_path + '%d-R%sactivity_InhR.npy' % (recset ,str(spatial_freq)))
		if (i == 0):
			RRds_neurons_mean = np.zeros(np.shape(RRds))
			RRds_neurons_mean = np.expand_dims((RRds_neurons_mean), axis = 2)
			RInhR_neurons_mean = np.zeros(np.shape(RInhR))
			RInhR_neurons_mean = np.expand_dims((RInhR_neurons_mean), axis = 2)
			if(j == 0 and Exist == False):
				RRds_Mean_matrix = np.zeros((tf,len(sf)))
				RRds_Mean_matrix = np.expand_dims((RRds_Mean_matrix), axis = 2)
				RInhR_Mean_matrix = np.zeros((tf,len(sf)))
				RInhR_Mean_matrix = np.expand_dims((RInhR_Mean_matrix), axis = 2)
		if (spatial_freq == 1.4):
			A = np.where(~RInhR.any(axis=1))[0]
			mask = np.ones(len(RInhR), dtype=bool)
			mask[A] = False
		print "Concatenating and averaging over neurons"
		if (len(RRds) == 0):
			print "oops"	
		if (len(RRds) > 0):
			if (RRds.ndim == 2):
				RRds = np.expand_dims(RRds, axis = 2)
		if (len(RInhR) > 0):
			if (RInhR.ndim == 2):
				RInhR = np.expand_dims(RInhR, axis = 2)
		RRds_neurons_mean = np.concatenate((RRds_neurons_mean,RRds), axis = 2)
		RInhR_neurons_mean = np.concatenate((RInhR_neurons_mean,RInhR), axis = 2)
		#os.rename(notaveraged_matrix_path +'%d-R%sactivity_Rds.npy' % (recset ,str(spatial_freq)), averaged_matrix_path + '%d-R%sactivity_Rds.npy' % (recset ,str(spatial_freq)))
		#os.rename(notaveraged_matrix_path + '%d-R%sactivity_InhR.npy' % (recset ,str(spatial_freq)) , averaged_matrix_path + '%d-R%sactivity_InhR.npy' % (recset ,str(spatial_freq)))
	RRds_neurons_mean = RRds_neurons_mean[mask,...]
	RInhR_neurons_mean = RInhR_neurons_mean[mask,...]
	RRds_neurons_mean = np.mean(RRds_neurons_mean, 0)
	RInhR_neurons_mean = np.mean(RInhR_neurons_mean, 0)
	RRds_neurons_mean = RRds_neurons_mean[:,1:]
	RInhR_neurons_mean = RInhR_neurons_mean[:,1:]
	RRds_neurons_mean = np.expand_dims(RRds_neurons_mean, axis = 2)
	RInhR_neurons_mean = np.expand_dims(RInhR_neurons_mean, axis = 2)
	RRds_Mean_matrix = np.concatenate((RRds_Mean_matrix,RRds_neurons_mean), axis = 2)
	RInhR_Mean_matrix = np.concatenate((RInhR_Mean_matrix,RInhR_neurons_mean), axis = 2)

if(Exist == False):
	RRds_Mean_matrix = RRds_Mean_matrix[:,:,1:]
	RInhR_Mean_matrix = RInhR_Mean_matrix[:,:,1:]
	print "Hey"

RRds_Mean_matrix = np.mean(RRds_Mean_matrix, 2)
RInhR_Mean_matrix = np.mean(RInhR_Mean_matrix, 2)
#First column is full of zeros


print "New mean matrices have been calculated and saved"

np.save(mean_matrix_path + 'Ractivity_RRds.npy' , RRds_Mean_matrix)
np.save(mean_matrix_path + 'Ractivity_RInhR.npy', RInhR_Mean_matrix)







