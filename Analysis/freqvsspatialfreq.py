import socket
import struct
import numpy as np
import time
import matplotlib
from matplotlib import pyplot as plt
import glob, os

tdrDir = '/Users/HHG/FilesRepositories/AEDAT/SixLines/'
dataDir = tdrDir
os.chdir(dataDir)
dataFilenameList = glob.glob("*.aedat")
datareadlist = []
for ind in dataFilenameList:
        datareadlist.append(open(ind, "rb"))
debug = False

def read_events(file):
    """ A simple function that read dynap-se events from cAER aedat 3.0 file format"""
    
    # raise Exception at end of file
    data = file.read(28)
    if(len(data) <= 0):
        print("read all data\n")
        raise NameError('END OF DATA')


    # read header
    eventtype = struct.unpack('H', data[0:2])[0]
    eventsource = struct.unpack('H', data[2:4])[0]
    eventsize = struct.unpack('I', data[4:8])[0]
    eventoffset = struct.unpack('I', data[8:12])[0]
    eventtsoverflow = struct.unpack('I', data[12:16])[0]
    eventcapacity = struct.unpack('I', data[16:20])[0]
    eventnumber = struct.unpack('I', data[20:24])[0]
    eventvalid = struct.unpack('I', data[24:28])[0]
    next_read = eventcapacity * eventsize  # we now read the full packet
    data = file.read(next_read)    
    counter = 0  # eventnumber[0]
    #spike events
    core_id_tot = []
    chip_id_tot = []
    neuron_id_tot = []
    ts_tot =[]
    #special events
    spec_type_tot =[]
    spec_ts_tot = []

    if(eventtype == 0):
        spec_type_tot =[]
        spec_ts_tot = []
        while(data[counter:counter + eventsize]):  # loop over all event packets
            special_data = struct.unpack('I', data[counter:counter + 4])[0]
            timestamp = struct.unpack('I', data[counter + 4:counter + 8])[0]
            spec_type = (special_data >> 1) & 0x0000007F
            spec_type_tot.append(spec_type)
            spec_ts_tot.append(timestamp)
            if(spec_type == 6 or spec_type == 7 or spec_type == 9 or spec_type == 10):
                print (timestamp, spec_type)
            counter = counter + eventsize        
    elif(eventtype == 12):
        while(data[counter:counter + eventsize]):  # loop over all event packets
            aer_data = struct.unpack('I', data[counter:counter + 4])[0]
            timestamp = struct.unpack('I', data[counter + 4:counter + 8])[0]
            core_id = (aer_data >> 1) & 0x0000001F
            chip_id = (aer_data >> 6) & 0x0000003F
            neuron_id = (aer_data >> 12) & 0x000FFFFF
            core_id_tot.append(core_id)
            chip_id_tot.append(chip_id)
            neuron_id_tot.append(neuron_id)
            ts_tot.append(timestamp)
            counter = counter + eventsize
            if(debug):          
                print("chip id "+str(chip_id)+'\n')
                print("core_id "+str(core_id)+'\n')
                print("neuron_id "+str(neuron_id)+'\n')
                print("timestamp "+str(timestamp)+'\n')
                print("####\n")


    return core_id_tot, chip_id_tot, neuron_id_tot, ts_tot, spec_type_tot, spec_ts_tot

def skip_header(file):
    ''' This function skip the standard header of the recording file '''
    line = file.readline()
    while line.startswith("#"):
        if ( line == '#!END-HEADER\r\n'):
            break
        else:
            line = file.readline()


done_reading = [False,False,False,False,False,False]
for datastim in datareadlist:
    skip_header(datastim)

chip_id_tot = []
core_id_tot = []
neuron_id_tot = []
ts_tot  = []
spec_type_tot = []
spec_ts_tot = []
neuron_17 = []
neuron_21 = []
neuron_26 = []
neuron_65 = []
neuron_69 = []
times = []
for i in range(len(datareadlist)):
    while(done_reading[i] == False):
        try:
            core_id, chip_id, neuron_id, ts, spec_type, spec_ts = read_events(datareadlist[i])
            core_id_tot.extend(np.array(core_id))
            chip_id_tot.extend(np.array(chip_id))
            neuron_id_tot.extend(np.array(neuron_id))
            ts_tot.extend(np.array(ts))
            spec_type_tot.extend(np.array(spec_type))
            spec_ts_tot.extend(np.array(spec_ts))
            
        except NameError:
                 datareadlist[i].close()
                 done_reading[i] = True
    neuron_id_d = []
    for chip, core, neur, ts in zip(chip_id_tot, core_id_tot, neuron_id_tot, ts_tot):
        if chip == 1 and core == 2:
                neuron_id_d.append(neur)
    times.append(ts_tot[-1]-ts_tot[0])
    neuron_17.append(neuron_id_d.count(17))
    neuron_21.append(neuron_id_d.count(21))
    neuron_26.append(neuron_id_d.count(26))
    neuron_65.append(neuron_id_d.count(65))
    neuron_69.append(neuron_id_d.count(69))

activity_17 = []
activity_21 = []
activity_26 = []
activity_65 = []
activity_69 = []
times_sec = [i * (10**-6) for i in times]
#print neuron_17
for i,count in enumerate(neuron_17):
    activity_17.append(count/times_sec[i])
for i,count in enumerate(neuron_21):
    activity_21.append(count/times_sec[i])
for i,count in enumerate(neuron_26):
    activity_26.append(count/times_sec[i])
for i,count in enumerate(neuron_65):
    activity_65.append(count/times_sec[i])
for i,count in enumerate(neuron_69):
    activity_69.append(count/times_sec[i])

# Figures

stimulus = [1,2,3,4,5,6]
plt.figure(1)
plt.plot(stimulus, activity_17, 'g-')
plt.xlabel('Spatial frequency (number of lines)')    
plt.ylabel('Average response frequency neuron 17 (Hz)')
plt.figure(2)
plt.plot(stimulus, activity_21, 'g-')
plt.xlabel('Spatial frequency (number of lines)')    
plt.ylabel('Average response frequency neuron 21(Hz)')
plt.figure(3)
plt.plot(stimulus, activity_26, 'g-')
plt.xlabel('Spatial frequency (number of lines)')    
plt.ylabel('Average response frequency neuron 26 (Hz)')
plt.figure(4)
plt.plot(stimulus, activity_65, 'g-')
plt.xlabel('Spatial frequency (number of lines)')    
plt.ylabel('Average response frequency neuron 65(Hz)')
plt.figure(5)
plt.plot(stimulus, activity_69, 'g-')
plt.xlabel('Spatial frequency (number of lines)')    
plt.ylabel('Average response frequency neuron 69 (Hz)')


plt.show()






