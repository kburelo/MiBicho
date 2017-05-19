/*
 * helloworld.c
 *
 *  Created on: Feb 2017 for tutorial on dynap-se
 *      Author: federico
 */

#include "helloworld.h"
#include "base/mainloop.h"
#include "base/module.h"



struct HWFilter_state {
	// user settings
	int eventSourceID; //what is this?
	//bool doConnection;
	//bool displayNeuNumber;
	//bool setBiases;
	bool init;
	bool stimulate;
	// usb utils
	caerInputDynapseState eventSourceModuleState;
	sshsNode eventSourceConfigNode;
};

typedef struct HWFilter_state *HWFilterState;

static bool caerHelloWorldModuleInit(caerModuleData moduleData);
static void caerHelloWorldModuleRun(caerModuleData moduleData, size_t argsNumber, va_list args);
static void caerHelloWorldModuleConfig(caerModuleData moduleData);
static void caerHelloWorldModuleExit(caerModuleData moduleData);
static void caerHelloWorldModuleReset(caerModuleData moduleData, uint16_t resetCallSourceID);
static bool EnableStimuliGenL(caerModuleData moduleData);
static bool DisableStimuliGenL(caerModuleData moduleData);
static struct caer_module_functions caerHelloWorldModuleFunctions = { .moduleInit =
	&caerHelloWorldModuleInit, .moduleRun = &caerHelloWorldModuleRun, .moduleConfig =
	&caerHelloWorldModuleConfig, .moduleExit = &caerHelloWorldModuleExit, .moduleReset =
	&caerHelloWorldModuleReset };
void caerHelloWorldModule(uint16_t moduleID, caerSpikeEventPacket spike) {
	caerModuleData moduleData = caerMainloopFindModule(moduleID, "Hello-world", CAER_MODULE_PROCESSOR);
	if (moduleData == NULL) {
		return;
	}

	caerModuleSM(&caerHelloWorldModuleFunctions, moduleData, sizeof(struct HWFilter_state), 1, spike);
}


static int time_count = 0;
static int time_count_last = 0;
static int stimdisabled = 0;

static bool caerHelloWorldModuleInit(caerModuleData moduleData) {
	// create parameters
	//sshsNodePutBoolIfAbsent(moduleData->moduleNode, "displayNeuNumber", false);
	//sshsNodePutBoolIfAbsent(moduleData->moduleNode, "setBiases", false);
	//sshsNodePutBoolIfAbsent(moduleData->moduleNode, "doConnection", false);
	HWFilterState state = moduleData->moduleState;
	// update node state
	//state->displayNeuNumber = sshsNodeGetBool(moduleData->moduleNode, "displayNeuNumber");
	//state->setBiases = sshsNodeGetBool(moduleData->moduleNode, "setBiases");
	//state->doConnection = sshsNodeGetBool(moduleData->moduleNode, "doConnection");
	state->stimulate = sshsNodeGetBool(moduleData->moduleNode, "stimulate");
	state->init = false;

	// Add config listeners last - let's the user interact with the parameter -
	sshsNodeAddAttributeListener(moduleData->moduleNode, moduleData, &caerModuleConfigDefaultListener);

	// Nothing that can fail here.
	return (true);
}

static void caerHelloWorldModuleRun(caerModuleData moduleData, size_t argsNumber, va_list args){
	UNUSED_ARGUMENT(argsNumber);

	// Interpret variable arguments (same as above in main function).
	caerSpikeEventPacket spike = va_arg(args, caerSpikeEventPacket);
	HWFilterState state = moduleData->moduleState;
	// Only process packets with content.
	if (spike == NULL) {
		return;
	}
		if(state->eventSourceID == NULL){
		state->eventSourceID  = 1; //caerEventPacketHeaderGetEventSource(&spike->packetHeader);// into state so that all functions can use it after init.
	}

	

  	// now we can do crazy processing etc..
	// first find out which one is the module producing the spikes. and get usb handle
	// --- start  usb handle / from spike event source id
	int sourceID = caerEventPacketHeaderGetEventSource(&spike->packetHeader);
	state->eventSourceModuleState = caerMainloopGetSourceState(U16T(sourceID));
	state->eventSourceConfigNode = caerMainloopGetSourceNode(U16T(sourceID));
	if(state->eventSourceModuleState == NULL || state->eventSourceConfigNode == NULL){
		return;
	}
	caerInputDynapseState stateSource = state->eventSourceModuleState;
	//sshsNode deviceConfigNodeMain = sshsGetRelativeNode(stateSource->eventSourceConfigNode, chipIDToName(DYNAPSE_CHIP_DYNAPSE, true));
	//sshsNode spikeNode = sshsGetRelativeNode(deviceConfigNodeMain, "spikeGen/");

	if(stateSource->deviceState == NULL){
		return;
	}
	// --- end usb handle

	if(state->init == false){
		// do the initialization

		caerLog(CAER_LOG_NOTICE, __func__, "start init of hello world");
	
		caerLog(CAER_LOG_NOTICE, __func__, "setting default biases");
		for(size_t coreid=0; coreid<4; coreid++){
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_AHTAU_N", 7, 35, "LowBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_AHTHR_N", 7, 1, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_AHW_P", 7, 1, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_BUF_P", 3, 80, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_CASC_N", 7, 1, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_DC_P", 7, 1, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_NMDA_N", 7, 1, "HighBias", "PBias"); //7,0-->7,1,P
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_RFR_N", 8,108, "HighBias", "NBias");// 6, 255
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_TAU1_N", 6,24, "LowBias", "NBias");//4, 200
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_TAU2_N", 5, 15, "HighBias", "NBias");//6,15
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_THR_N", 4, 20, "HighBias", "NBias");//3, 100
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "NPDPIE_TAU_F_P", 5, 41, "HighBias","PBias");// 6,105
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "NPDPIE_TAU_S_P", 7, 40, "HighBias","NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "NPDPIE_THR_F_P", 2, 200, "HighBias", "PBias");//0,220
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "NPDPIE_THR_S_P", 7, 0, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "NPDPII_TAU_F_P", 7, 40, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "NPDPII_TAU_S_P", 7, 40, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "NPDPII_THR_F_P", 7, 40, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "NPDPII_THR_S_P", 7, 40, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "PS_WEIGHT_EXC_F_N", 0, 126, "HighBias", "NBias");//0,76
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "PS_WEIGHT_EXC_S_N", 7, 1, "HighBias", "NBias");//7,0
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "PS_WEIGHT_INH_F_N", 7, 0, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "PS_WEIGHT_INH_S_N", 7, 0, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "PULSE_PWLK_P", 0, 43, "HighBias", "PBias");//3,50
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "R2R_P", 4, 85, "HighBias", "PBias");		
	  	}
	  	for(size_t coreid=0; coreid<4; coreid++){
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "IF_AHTAU_N", 7, 35, "LowBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "IF_AHTHR_N", 7, 1, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "IF_AHW_P", 7, 1, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "IF_BUF_P", 3, 80, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "IF_CASC_N", 7, 1, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "IF_DC_P", 7, 1, "HighBias", "PBias"); 
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "IF_NMDA_N", 7, 1, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "IF_RFR_N", 8, 108, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "IF_TAU1_N", 6, 24, "LowBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "IF_TAU2_N", 5, 15, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "IF_THR_N", 4, 20, "HighBias", "NBias"); 
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "NPDPIE_TAU_F_P", 5, 41, "HighBias","PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "NPDPIE_TAU_S_P", 7, 40, "HighBias","NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "NPDPIE_THR_F_P", 2, 200, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "NPDPIE_THR_S_P", 7, 0, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "NPDPII_TAU_F_P", 7, 40, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "NPDPII_TAU_S_P", 7, 40, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "NPDPII_THR_F_P", 7, 40, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "NPDPII_THR_S_P", 7, 40, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "PS_WEIGHT_EXC_F_N", 3, 126, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "PS_WEIGHT_EXC_S_N", 7, 1, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "PS_WEIGHT_INH_F_N", 7, 0, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "PS_WEIGHT_INH_S_N", 7, 0, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "PULSE_PWLK_P", 0, 43, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "R2R_P", 4, 85, "HighBias", "PBias");	
		}	
	  	
	  	for(size_t coreid=0; coreid<4; coreid++){
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "IF_AHTAU_N", 7, 35, "LowBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "IF_AHTHR_N", 7, 0, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "IF_AHW_P", 7, 0, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "IF_BUF_P", 7, 0, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "IF_CASC_N", 7, 0, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "IF_DC_P", 2, 30, "HighBias", "PBias"); 
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "IF_NMDA_N", 7, 0, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "IF_RFR_N", 3, 3, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "IF_TAU1_N", 7, 10, "LowBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "IF_TAU2_N", 6, 100, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "IF_THR_N", 4, 120, "HighBias", "NBias"); 
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "NPDPIE_TAU_F_P", 7, 40, "HighBias","PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "NPDPIE_TAU_S_P", 7, 0, "HighBias","NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "NPDPIE_THR_F_P", 7, 0, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "NPDPIE_THR_S_P", 7, 0, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "NPDPII_TAU_F_P", 7, 0, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "NPDPII_TAU_S_P", 7, 40, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "NPDPII_THR_F_P", 7, 0, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "NPDPII_THR_S_P", 7, 40, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "PS_WEIGHT_EXC_F_N", 15, 0, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "PS_WEIGHT_EXC_S_N", 7, 0, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "PS_WEIGHT_INH_F_N", 7, 0, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "PS_WEIGHT_INH_S_N", 7, 0, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "PULSE_PWLK_P", 3, 106, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "R2R_P", 4, 85, "HighBias", "PBias");		
	  	}
	  	for(size_t coreid=0; coreid<4; coreid++){
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "IF_AHTAU_N", 7, 35, "LowBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "IF_AHTHR_N", 7, 0, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "IF_AHW_P", 7, 0, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "IF_BUF_P", 7, 0, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "IF_CASC_N", 7, 0, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "IF_DC_P", 2, 30, "HighBias", "PBias"); 
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "IF_NMDA_N", 7, 0, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "IF_RFR_N", 3, 3, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "IF_TAU1_N", 7, 10, "LowBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "IF_TAU2_N", 6, 100, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "IF_THR_N", 4, 120, "HighBias", "NBias"); 
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "NPDPIE_TAU_F_P", 7, 40, "HighBias","PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "NPDPIE_TAU_S_P", 7, 0, "HighBias","NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "NPDPIE_THR_F_P", 7, 0, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "NPDPIE_THR_S_P", 7, 0, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "NPDPII_TAU_F_P", 7, 0, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "NPDPII_TAU_S_P", 7, 40, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "NPDPII_THR_F_P", 7, 0, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "NPDPII_THR_S_P", 7, 40, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "PS_WEIGHT_EXC_F_N", 15, 0, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "PS_WEIGHT_EXC_S_N", 7, 0, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "PS_WEIGHT_INH_F_N", 7, 0, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "PS_WEIGHT_INH_S_N", 7, 0, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "PULSE_PWLK_P", 3, 106, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "R2R_P", 4, 85, "HighBias", "PBias");		
	  	}
	  	caerLog(CAER_LOG_NOTICE, __func__, "initial baises have been set");

	  	//TO SET SRAMS
		//coreid = 0
		//neuron id = 45
		//virtual core = 0
	 	//sx = 1 west:1 east:0
	 	//dx = 1
	 	//sy = 0 north:0 south:1
	 	//dy = 0
	 	//sram = 2 (we can choose from 1-3, this was a random selection)
		//destination core = 15 (to all cores in the chip we target)

		//TO SET CAMS
		//pre neuron = address
		//post neuron = neuron you are trying to excite
		//camid
		//type of synapse
	  	
	  	
	    //Clearing cams on chip U0
		caerLog(CAER_LOG_NOTICE, __func__, "Clearing cams for all neurons in chip U0");
		caerDeviceConfigSet(stateSource->deviceState, DYNAPSE_CONFIG_CHIP, DYNAPSE_CONFIG_CHIP_ID, DYNAPSE_CONFIG_DYNAPSE_U0);
		for(size_t neuronid = 0; neuronid < 1024; neuronid++){
			for(size_t camid=0; camid<64; camid++){
				caerDynapseWriteCam(stateSource->deviceState, 0, neuronid, camid, DYNAPSE_CONFIG_CAMTYPE_F_INH);
			}
		}
		caerLog(CAER_LOG_NOTICE, __func__, "All cams of chip U1 have been reset to address 0");
		caerLog(CAER_LOG_NOTICE, __func__, "Setting all cams for chip U0");
		
		int horizontal_count;
		for(horizontal_count = -16; horizontal_count < 240; horizontal_count += 80){
			for(size_t pixelid = horizontal_count + 16; pixelid <  horizontal_count + 32; pixelid++){
				for (size_t camid = 0; camid < 2; camid++){					
					caerDynapseWriteCam(stateSource->deviceState, 7, pixelid, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				}
			}
			for(size_t pixelid = horizontal_count + 32; pixelid <  horizontal_count + 48; pixelid++){				
				for (size_t camid = 0; camid < 2; camid++){
					caerDynapseWriteCam(stateSource->deviceState, 7, pixelid, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				}
			}
			for(size_t pixelid = horizontal_count + 16 + 256; pixelid <  horizontal_count + 32 + 256; pixelid++){
				for (size_t camid = 0; camid < 2; camid++){					
					caerDynapseWriteCam(stateSource->deviceState, 7, pixelid, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				}
			}
			for(size_t pixelid = horizontal_count + 32 + 256; pixelid <  horizontal_count + 48 + 256; pixelid++){				
				for (size_t camid = 0; camid < 2; camid++){
					caerDynapseWriteCam(stateSource->deviceState, 7, pixelid, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				}
			}
			for(size_t pixelid = horizontal_count + 16 + 512; pixelid <  horizontal_count + 32 + 512; pixelid++){
				for (size_t camid = 0; camid < 2; camid++){					
					caerDynapseWriteCam(stateSource->deviceState, 7, pixelid, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				}
			}
			for(size_t pixelid = horizontal_count + 32 + 512; pixelid <  horizontal_count + 48 + 512; pixelid++){				
				for (size_t camid = 0; camid < 2; camid++){
					caerDynapseWriteCam(stateSource->deviceState, 7, pixelid, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				}
			}
			for(size_t pixelid = horizontal_count + 16 + 768; pixelid <  horizontal_count + 32 + 768; pixelid++){
				for (size_t camid = 0; camid < 2; camid++){					
					caerDynapseWriteCam(stateSource->deviceState, 7, pixelid, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				}
			}
			for(size_t pixelid = horizontal_count + 32 + 768; pixelid <  horizontal_count + 48 + 768; pixelid++){				
				for (size_t camid = 0; camid < 2; camid++){
					caerDynapseWriteCam(stateSource->deviceState, 7, pixelid, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				}
			}	
		}
		for(size_t v1neuronid = 0; v1neuronid < 256; v1neuronid++){	
			for (size_t coreid = 0; coreid < 4; coreid ++){
				caerDynapseWriteSram(stateSource->deviceState, coreid, v1neuronid, coreid, 0, 0, 1, 1, 1, 15);
			}
		}
		
		caerLog(CAER_LOG_NOTICE, __func__, "Cams and Srams for chip U0 have been set");
		caerLog(CAER_LOG_NOTICE, __func__, "Clearing cams for all neurons in core 0 of chip U2");
		 //Configuring ship U2, clearing the cams
		caerDeviceConfigSet(stateSource->deviceState, DYNAPSE_CONFIG_CHIP, DYNAPSE_CONFIG_CHIP_ID, DYNAPSE_CONFIG_DYNAPSE_U2);
		for(size_t neuronid = 0; neuronid < 1024; neuronid++){
			for(size_t camid=0; camid<64; camid++){
				caerDynapseWriteCam(stateSource->deviceState, 0, neuronid, camid, DYNAPSE_CONFIG_CAMTYPE_F_INH);
			}
		}
		
		caerLog(CAER_LOG_NOTICE, __func__, "Setting Cams for core 0 in chip U2");	
		
		horizontal_count = -16;
		int vertical_count = 0;
		int midlinea = 0;
		int midlineb = 0;
		int midlinec = 0;
		int midlined = 0;
		int midlinee = 0;
		int midlinef = 0;
		int column;

		for (size_t v1neuronid = 0; v1neuronid < 813; v1neuronid++){
			int rfsize = rand() % 2;
			int onoffzones = rand() % 2;
			size_t camid = 0;
			for (size_t preneuron = horizontal_count + 16 + vertical_count; preneuron <  horizontal_count + 22 + vertical_count; preneuron++){
				if ((preneuron>175 && preneuron<192)|| (preneuron>431 && preneuron<448)){
					midlinea = 0;
					midlineb = 0;
					midlinec = 0;
					midlined = 0;
					midlinee = 0;
					midlinef = 256;
				}
				if ((preneuron>191 && preneuron<208)|| (preneuron>447 && preneuron<464)){
					midlinea = 0;
					midlineb = 0;
					midlinec = 0;
					midlined = 0;
					midlinee = 256;
					midlinef = 256;
				}
				if ((preneuron>207 && preneuron<224)|| (preneuron>463 && preneuron<480)){
					midlinea = 0;
					midlineb = 0;
					midlinec = 0;
					midlined = 256;
					midlinee = 256;
					midlinef = 256;
				}
				if ((preneuron>223 && preneuron<240) || (preneuron>479 && preneuron<496)){
					midlinea = 0;
					midlineb = 0;
					midlinec = 256;
					midlined = 256;
					midlinee = 256;
					midlinef = 256;
				}
				if ((preneuron>239 && preneuron<256)|| (preneuron>495 && preneuron<512)){
					midlinea = 0;
					midlineb = 256;
					midlinec = 256;
					midlined = 256;
					midlinee = 256;
					midlinef = 256;
				}
			
				if ((preneuron>687 && preneuron<704) || (preneuron>704 && preneuron<720)|| (preneuron>943 && preneuron<960)
					|| (preneuron>959 && preneuron<976)) {
					rfsize = 0;
				}
				if (rfsize == 0){
					if (onoffzones == 0){
						column = 0;
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 1);
						camid++;
						column = 1;
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 3);
						camid++;
						column = 2;
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 3);
						camid++;
						column = 3;				
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 1);
						camid++;
					}
					else if (onoffzones == 1){
						column = 0;
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 3);
						camid++;
						column = 1;
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 3);
						camid++;
						column = 2;
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 1);
						camid++;
						column = 3;				
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 1);
						camid++;	
					}
				}
				else if (rfsize == 1){
					if (onoffzones == 0){
						column = 0;
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 1);
						camid++;
						column = 1;
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 1);
						camid++;
						column = 2;
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 3);
						camid++;
						column = 3;				
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 3);
						camid++;
						column = 4;				
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 1);
						camid++;
						column = 5;				
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 1);
						camid++;
					}
					else if (onoffzones == 1){
						column = 0;
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 3);
						camid++;
						column = 1;
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 3);
						camid++;
						column = 2;
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 3);
						camid++;
						column = 3;				
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 1);
						camid++;
						column = 4;				
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 1);
						camid++;
						column = 5;				
						caerDynapseWriteCam(stateSource->deviceState, (preneuron + 16)*column, v1neuronid, camid, 1);
						camid++;	
					}
				}
			}
			horizontal_count = horizontal_count + 16;
			if (horizontal_count > 688){
				horizontal_count = -16;
				vertical_count++;
			}
		}
		caerLog(CAER_LOG_NOTICE, __func__, "Cams for core 0 in chip U2 are set");


		state->init = true;
	}
		
	if (state->stimulate == true) { //it runs when there is a spike
		EnableStimuliGenL(moduleData);
	}

}	


	// Iterate over spikes in the packet
	/*CAER_SPIKE_ITERATOR_VALID_START(spike)
		int32_t timestamp =  caerSpikeEventGetTimestamp(caerSpikeIteratorElement);
	  	uint8_t chipid 	  =  caerSpikeEventGetChipID(caerSpikeIteratorElement);
	  	uint8_t neuronid  =  caerSpikeEventGetNeuronID(caerSpikeIteratorElement);
	  	uint8_t coreid    =  caerSpikeEventGetSourceCoreID(caerSpikeIteratorElement);

	  	// only if user wants to display
	  	if(state->displayNeuNumber){
			// identify spikes from chip0 (which comes out as 1) core 0
			if(chipid == 1 && coreid == 0 ){
				// tell which neuron
				caerLog(CAER_LOG_NOTICE, __func__, "Neuron id %d \n", neuronid);
			}
	  	}

	CAER_SPIKE_ITERATOR_VALID_END*/

	//if(state->setBiases){}
	//if(state->doConnection){}


bool EnableStimuliGenL(caerModuleData moduleData) {
	HWFilterState state = moduleData->moduleState;

	// --- start USB handle / from spike event source id
	state->eventSourceModuleState = caerMainloopGetSourceState(U16T(state->eventSourceID));
	state->eventSourceConfigNode = caerMainloopGetSourceNode(U16T(state->eventSourceID));
	// --- end USB handle

	sshsNode deviceConfigNodeMain = sshsGetRelativeNode(state->eventSourceConfigNode, chipIDToName(DYNAPSE_CHIP_DYNAPSE, true)); //DYNAPSE_CONFIG_DYNAPSE_U0
	sshsNode spikeNode = sshsGetRelativeNode(deviceConfigNodeMain, "spikeGen/");
	sshsNodePutBool(spikeNode, "running", true);
	sshsNodePutInt(spikeNode, "stim_type", 2); //regular
	sshsNodePutInt(spikeNode, "stim_duration", 10); //seconds 
	sshsNodePutInt(spikeNode, "stim_avr", 80); //Hz
	sshsNodePutInt(spikeNode, "address", 7);
	sshsNodePutInt(spikeNode, "core_d", 15);
	sshsNodePutInt(spikeNode, "chip_id", 0);
	//sshsNodePutBool(spikeNode, "repeat", true);
	sshsNodePutBool(spikeNode, "doStim", true);
	return (true);
}

bool DisableStimuliGenL(caerModuleData moduleData) {
	HWFilterState state = moduleData->moduleState;

	// --- start USB handle / from spike event source id
	state->eventSourceModuleState = caerMainloopGetSourceState(U16T(state->eventSourceID));
	state->eventSourceConfigNode = caerMainloopGetSourceNode(U16T(state->eventSourceID));
	// --- end USB handle

	sshsNode deviceConfigNodeMain = sshsGetRelativeNode(state->eventSourceConfigNode, chipIDToName(DYNAPSE_CHIP_DYNAPSE, true));
	sshsNode spikeNode = sshsGetRelativeNode(deviceConfigNodeMain, "spikeGen/");
	sshsNodePutBool(spikeNode, "running", true);
	sshsNodePutBool(spikeNode, "doStim", false);
	return (true);
}

static void caerHelloWorldModuleConfig(caerModuleData moduleData) {
	caerModuleConfigUpdateReset(moduleData);

	HWFilterState state = moduleData->moduleState;

	// this will update parameters, from user input
	//state->displayNeuNumber = sshsNodeGetBool(moduleData->moduleNode, "displayNeuNumber");
	//state->setBiases = sshsNodeGetBool(moduleData->moduleNode, "setBiases");
	//state->doConnection = sshsNodeGetBool(moduleData->moduleNode, "doConnection");
	state->stimulate = sshsNodeGetBool(moduleData->moduleNode, "stimulate");

}

static void caerHelloWorldModuleExit(caerModuleData moduleData) {
	// Remove listener, which can reference invalid memory in userData.
	sshsNodeRemoveAttributeListener(moduleData->moduleNode, moduleData, &caerModuleConfigDefaultListener);

	HWFilterState state = moduleData->moduleState;

	// here we should free memory and other shutdown procedures if needed

}

static void caerHelloWorldModuleReset(caerModuleData moduleData, uint16_t resetCallSourceID) {
	UNUSED_ARGUMENT(resetCallSourceID);

	HWFilterState state = moduleData->moduleState;

}
