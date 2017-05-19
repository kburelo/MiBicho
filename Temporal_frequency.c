/*
 * helloworld.c
 *
 *  Created on: Feb 2017 for tutorial on dynap-se
 *      Author: federico
 */
//This code is to see the activity of our original neurons 17,21,26,65,69 when a stiumuli is moving at different velocities
//The optimal spatial frequency was chosen for each neuron. Stimulate 1 is for 17 (only one with 3 lines of positive receptive field)
// Stimulate2 is for neuron 21 and 65 that has a positive receptive field of 2 lines but the whole receptive field is 6 lines lenght
// Finally stimulate 3 is for neuron 26 and 69 with a positive receptive field of 2 lines but different from neurons 21 and 65 the 
// total lenght of the receptive field is 4.

#include "helloworld.h"
#include "base/mainloop.h"
#include "base/module.h"

struct HWFilter_state {
	// user settings
	clock_t start;
	int eventSourceID; //what is this?
	bool init;
	bool stimulate1;
	bool stimulate2;
	bool stimulate3;
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
static bool EnableStimuliGenL(caerModuleData moduleData, uint16_t address);
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
	sshsNodePutBoolIfAbsent(moduleData->moduleNode, "stimulate1", false);
	sshsNodePutBoolIfAbsent(moduleData->moduleNode, "stimulate2", false);
	sshsNodePutBoolIfAbsent(moduleData->moduleNode, "stimulate3", false);
	HWFilterState state = moduleData->moduleState;
	// update node state
	state->stimulate1 = sshsNodeGetBool(moduleData->moduleNode, "stimulate1");
	state->stimulate2 = sshsNodeGetBool(moduleData->moduleNode, "stimulate2");
	state->stimulate3 = sshsNodeGetBool(moduleData->moduleNode, "stimulate3");

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
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "IF_AHTAU_N", 7, 35, "LowBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "IF_AHTHR_N", 7, 1, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "IF_AHW_P", 7, 1, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "IF_BUF_P",3, 80, "HighBias", "PBias"); 
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "IF_CASC_N", 7, 1, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "IF_DC_P", 7, 1, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "IF_NMDA_N", 2,126, "HighBias", "PBias"); 
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "IF_RFR_N", 8, 108, "HighBias", "NBias"); 
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "IF_TAU1_N", 6, 24, "LowBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "IF_TAU2_N",5, 15, "HighBias", "NBias"); 
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "IF_THR_N", 4, 20, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "NPDPIE_TAU_F_P", 5, 41, "HighBias","PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "NPDPIE_TAU_S_P",  5, 40, "HighBias","NBias");//7,40
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "NPDPIE_THR_F_P", 2, 200, "HighBias", "PBias"); //0,220
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "NPDPIE_THR_S_P", 0, 10, "HighBias", "PBias"); 
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "NPDPII_TAU_F_P", 7, 40, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "NPDPII_TAU_S_P", 7, 40, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "NPDPII_THR_F_P", 7, 40, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "NPDPII_THR_S_P", 7, 40, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "PS_WEIGHT_EXC_F_N", 0,126, "HighBias", "NBias"); //0,76
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "PS_WEIGHT_EXC_S_N", 0, 126, "HighBias", "NBias"); //7,0
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "PS_WEIGHT_INH_F_N", 7, 0, "HighBias", "NBias"); //7,0
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "PS_WEIGHT_INH_S_N", 7, 0, "HighBias", "NBias"); //7,0
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "PULSE_PWLK_P",0, 43, "HighBias", "PBias"); //3,50
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 0, "R2R_P", 4, 85, "HighBias", "PBias");
		for(size_t coreid=1; coreid<4; coreid++){
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_AHTAU_N", 7, 35, "LowBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_AHTHR_N", 7, 1, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_AHW_P", 7, 1, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_BUF_P", 3, 80, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_CASC_N", 7, 1, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_DC_P", 7, 1, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_NMDA_N", 2, 126, "HighBias", "NBias"); 
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_RFR_N", 0,108, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_TAU1_N", 6,24, "LowBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_TAU2_N", 5, 15, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_THR_N", 4, 20, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "NPDPIE_TAU_F_P", 4, 61, "HighBias","PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "NPDPIE_TAU_S_P", 5, 41, "HighBias","PBias"); //7,40
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "NPDPIE_THR_F_P", 2, 200, "HighBias", "PBias");//0,220
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "NPDPIE_THR_S_P", 2, 200, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "NPDPII_TAU_F_P", 5, 41, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "NPDPII_TAU_S_P", 3, 41, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "NPDPII_THR_F_P", 2, 200, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "NPDPII_THR_S_P", 3, 200, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "PS_WEIGHT_EXC_F_N", 0, 114, "HighBias", "NBias");//0,76
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "PS_WEIGHT_EXC_S_N", 0, 126, "HighBias", "NBias");//7,0
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "PS_WEIGHT_INH_F_N", 0, 126, "HighBias", "NBias");//7,0
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "PS_WEIGHT_INH_S_N", 0, 250, "HighBias", "NBias");//7,0
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
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "IF_NMDA_N", 2, 126, "HighBias", "PBias"); // 7,1
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "IF_RFR_N", 8, 108, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "IF_TAU1_N", 6, 24, "LowBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "IF_TAU2_N", 5, 15, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "IF_THR_N", 4, 20, "HighBias", "NBias"); 
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "NPDPIE_TAU_F_P", 5, 41, "HighBias","PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "NPDPIE_TAU_S_P", 5, 40, "HighBias","NBias"); // 7,40
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "NPDPIE_THR_F_P", 2, 200, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "NPDPIE_THR_S_P", 0, 10, "HighBias", "PBias"); // 7,0
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "NPDPII_TAU_F_P", 7, 40, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "NPDPII_TAU_S_P", 7, 40, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "NPDPII_THR_F_P", 7, 40, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "NPDPII_THR_S_P", 7, 40, "HighBias", "PBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "PS_WEIGHT_EXC_F_N", 2, 126, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U2, coreid, "PS_WEIGHT_EXC_S_N", 2, 126, "HighBias", "NBias");
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
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U1, coreid, "PS_WEIGHT_EXC_F_N", 7, 0, "HighBias", "NBias");
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
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U3, coreid, "PS_WEIGHT_EXC_F_N", 7, 0, "HighBias", "NBias");
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
		caerLog(CAER_LOG_NOTICE, __func__, "Setting all cams and srams for chip U0");
		
		// Cams for the neurons that receive the spike generator
		for(size_t neuronid = 0; neuronid<6; neuronid++){
			for (size_t camid = 0; camid<2; camid++){
				caerDynapseWriteCam(stateSource->deviceState, 1, neuronid, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 8, neuronid, camid+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 15, neuronid, camid+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 22, neuronid, camid+6, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 29, neuronid, camid+8, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 32, neuronid, camid+10, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 11, neuronid, camid+12, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 12, neuronid, camid+14, DYNAPSE_CONFIG_CAMTYPE_F_EXC);


			}
			for (size_t camid = 0; camid<2; camid++){
				caerDynapseWriteCam(stateSource->deviceState, 2, neuronid+16, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 8, neuronid+16, camid+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 9, neuronid+16, camid+20, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 15, neuronid+16, camid+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 16, neuronid+16, camid+6, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 22, neuronid+16, camid+8, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 23, neuronid+16, camid+10, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 29, neuronid+16, camid+12, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 30, neuronid+16, camid+14, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 32, neuronid+16, camid+16, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 33, neuronid+16, camid+18, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 12, neuronid+16, camid+22, DYNAPSE_CONFIG_CAMTYPE_F_EXC);

				
			}
			for (size_t camid = 0; camid<2; camid++){
				caerDynapseWriteCam(stateSource->deviceState, 3, neuronid+16*2, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 9, neuronid+16*2, camid+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 10, neuronid+16*2, camid+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 15, neuronid+16*2, camid+6, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 16, neuronid+16*2, camid+8, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 17, neuronid+16*2, camid+10, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 22, neuronid+16*2, camid+12, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 23, neuronid+16*2, camid+14, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 24, neuronid+16*2, camid+16, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 29, neuronid+16*2, camid+18, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 30, neuronid+16*2, camid+20, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 31, neuronid+16*2, camid+22, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 32, neuronid+16*2, camid+24, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 33, neuronid+16*2, camid+26, DYNAPSE_CONFIG_CAMTYPE_F_EXC);

			}
			for (size_t camid = 0; camid<2; camid++){
				caerDynapseWriteCam(stateSource->deviceState, 4, neuronid+16*3, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 10, neuronid+16*3, camid+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 11, neuronid+16*3, camid+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 16, neuronid+16*3, camid+6, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 17, neuronid+16*3, camid+8, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 18, neuronid+16*3, camid+10, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 22, neuronid+16*3, camid+12, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 23, neuronid+16*3, camid+14, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 24, neuronid+16*3, camid+16, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 25, neuronid+16*3, camid+18, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 29, neuronid+16*3, camid+20, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 30, neuronid+16*3, camid+22, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 31, neuronid+16*3, camid+24, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 32, neuronid+16*3, camid+26, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 33, neuronid+16*3, camid+28, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				

			}
			for (size_t camid = 0; camid<2; camid++){
				caerDynapseWriteCam(stateSource->deviceState, 5, neuronid+16*4, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 11, neuronid+16*4, camid+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 12, neuronid+16*4, camid+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 17, neuronid+16*4, camid+6, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 18, neuronid+16*4, camid+8, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 19, neuronid+16*4, camid+10, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 23, neuronid+16*4, camid+12, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 24, neuronid+16*4, camid+14, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 25, neuronid+16*4, camid+16, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 29, neuronid+16*4, camid+18, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 30, neuronid+16*4, camid+20, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 31, neuronid+16*4, camid+22, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 32, neuronid+16*4, camid+24, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 33, neuronid+16*4, camid+26, DYNAPSE_CONFIG_CAMTYPE_F_EXC);

			}
			for (size_t camid = 0; camid<2; camid++){
				caerDynapseWriteCam(stateSource->deviceState, 6, neuronid+16*5, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 12, neuronid+16*5, camid+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 13, neuronid+16*5, camid+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 18, neuronid+16*5, camid+6, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 19, neuronid+16*5, camid+8, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 24, neuronid+16*5, camid+10, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 25, neuronid+16*5, camid+12, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 30, neuronid+16*5, camid+14, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 31, neuronid+16*5, camid+16, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 32, neuronid+16*5, camid+18, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 33, neuronid+16*5, camid+20, DYNAPSE_CONFIG_CAMTYPE_F_EXC);

			}
			for (size_t camid = 0; camid<2; camid++){
				caerDynapseWriteCam(stateSource->deviceState, 7, neuronid+16*6, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 13, neuronid+16*6, camid+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 19, neuronid+16*6, camid+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 25, neuronid+16*6, camid+6, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 31, neuronid+16*6, camid+8, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 33, neuronid+16*6, camid+10, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			}
			for (size_t camid = 10; camid<12; camid++){
				caerDynapseWriteCam(stateSource->deviceState, 34, neuronid, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 34, neuronid+16, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 34, neuronid+16*4, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 34, neuronid+16*5, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			}

		//Sending the axons of the neurons that receive the spike generator to the same chip, core 2 id 4 
			caerDynapseWriteSram(stateSource->deviceState, 0, neuronid, 0, 0, 0, 0, 0, 1, 4);
			caerDynapseWriteSram(stateSource->deviceState, 0, neuronid+16, 0, 0, 0, 0, 0, 1, 4);
			caerDynapseWriteSram(stateSource->deviceState, 0, neuronid+16*2, 0, 0, 0, 0, 0, 1, 4);
			caerDynapseWriteSram(stateSource->deviceState, 0, neuronid+16*3, 0, 0, 0, 0, 0, 1, 4);
			caerDynapseWriteSram(stateSource->deviceState, 0, neuronid+16*4, 0, 0, 0, 0, 0, 1, 4);
			caerDynapseWriteSram(stateSource->deviceState, 0, neuronid+16*5, 0, 0, 0, 0, 0, 1, 4);
		}
		
		//Setting cams for the neurons in core 2 (V1 neurons)
		size_t camid = 0;
		for(size_t preneuron = 0; preneuron<6; preneuron++){
			caerDynapseWriteCam(stateSource->deviceState, preneuron, 529, camid, DYNAPSE_CONFIG_CAMTYPE_S_EXC);//17
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16, 529, camid+1, DYNAPSE_CONFIG_CAMTYPE_S_EXC);
			//caerDynapseWriteCam(stateSource->deviceState, preneuron+16, 529, 63, DYNAPSE_CONFIG_CAMTYPE_S_EXC);
			//cerDynapseWriteCam(stateSource->deviceState, preneuron+16, 529, 62, DYNAPSE_CONFIG_CAMTYPE_S_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*2, 529, camid+2, DYNAPSE_CONFIG_CAMTYPE_S_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*3, 529, camid+3, DYNAPSE_CONFIG_CAMTYPE_S_INH);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*4, 529, camid+4, DYNAPSE_CONFIG_CAMTYPE_S_INH);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*5, 529, camid+5, DYNAPSE_CONFIG_CAMTYPE_S_INH);

			caerDynapseWriteCam(stateSource->deviceState, preneuron, 577, camid, DYNAPSE_CONFIG_CAMTYPE_S_EXC);//65
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16, 577, camid+1, DYNAPSE_CONFIG_CAMTYPE_S_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*2, 577, camid+2, DYNAPSE_CONFIG_CAMTYPE_S_INH);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*3, 577, camid+3, DYNAPSE_CONFIG_CAMTYPE_S_INH);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*4, 577, camid+4, DYNAPSE_CONFIG_CAMTYPE_S_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*5, 577, camid+5, DYNAPSE_CONFIG_CAMTYPE_S_EXC);

			caerDynapseWriteCam(stateSource->deviceState, preneuron, 533, camid, DYNAPSE_CONFIG_CAMTYPE_S_INH);//21
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16, 533, camid+1, DYNAPSE_CONFIG_CAMTYPE_S_INH);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*2, 533, camid+2, DYNAPSE_CONFIG_CAMTYPE_S_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*3, 533, camid+3, DYNAPSE_CONFIG_CAMTYPE_S_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*4, 533, camid+4, DYNAPSE_CONFIG_CAMTYPE_S_INH);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*5, 533, camid+5, DYNAPSE_CONFIG_CAMTYPE_S_INH);

			caerDynapseWriteCam(stateSource->deviceState, preneuron, 581, camid, DYNAPSE_CONFIG_CAMTYPE_S_INH);//26
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16, 581, camid+1, DYNAPSE_CONFIG_CAMTYPE_S_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*2, 581, camid+2, DYNAPSE_CONFIG_CAMTYPE_S_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*3, 581, camid+3, DYNAPSE_CONFIG_CAMTYPE_S_INH);

			caerDynapseWriteCam(stateSource->deviceState, preneuron, 538, camid, DYNAPSE_CONFIG_CAMTYPE_S_INH);//69
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16, 538, camid+1, DYNAPSE_CONFIG_CAMTYPE_S_INH);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*2, 538, camid+2, DYNAPSE_CONFIG_CAMTYPE_S_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*3, 538, camid+3, DYNAPSE_CONFIG_CAMTYPE_S_EXC);
			
			camid+=6;
		}

		caerLog(CAER_LOG_NOTICE, __func__, "Cams and Srams for chip U0 have been set");
		


		state->init = true;
	}
		
	if (state->stimulate1 == true) { //it runs when there is a spike
		//Bar moving every 5 seconds 1st time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC < 5 ){
			EnableStimuliGenL(moduleData, 15);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 10){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 10 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 15){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 17);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 15 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 20){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 18);				
		}
		//Bar moving every 5 seconds 2nd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 20 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 25){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 25 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 30){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 30 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 35){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 17);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 35 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 40){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 18);				
		}
		//Bar moving every 5 seconds 3rd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 40 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 45){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 45 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 50){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 50 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 55){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 17);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 55 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 60){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 18);				
		}
		//Rest
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 60 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 66){
			
		}
		//Bar moving every 3 seconds 1st time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 66 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 69){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 69 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 72){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 72 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 75){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 17);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 75 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 78){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 18);				
		}
		//Bar moving every 3 seconds 2nd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 78 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 81){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 81 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 84){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 84 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 87){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 17);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 87 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 90){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 18);				
		}
		//Bar moving every 3 seconds 3rd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 90 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 93){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 93 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 96){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 96 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 99){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 17);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 99 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 102){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 18);				
		}
		//Rest
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 102 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 108){
			
		}
		//Bar moving every 1 second 1st time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 108 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 109){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 109 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 110){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 110 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 111){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 17);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 111 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 112){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 18);				
		}
		//Bar moving every 1 second 2nd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 112 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 113){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 113 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 114){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 114 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 115){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 17);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 115 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 116){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 18);				
		}
		//Bar moving every 1 second 3rd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 116 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 117){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 117 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 118){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 118 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 119){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 17);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 119 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 120){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 18);				
		}
		//Rest
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 120 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 126){
			
		}
		//Bar moving every 0.5 seconds 1st time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 126 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 126.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 126.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 127){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 127 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 127.5){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 17);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 127.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 128){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 18);				
		}
		//Bar moving every 0.5 seconds 2nd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 128 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 128.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 128.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 129){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 129 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 129.5){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 17);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 129.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 130){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 18);				
		}
		//Bar moving every 0.5 seconds 3rd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 130 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 130.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 130.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 131){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 131 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 131.5){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 17);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 131.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 132){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 18);				
		}
		//Rest
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 132 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 147){
			
		}
		//Bar moving every 0.2 seconds 1st time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 147 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 147.2){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 147.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 147.4){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 147.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 147.6){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 17);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 147.6 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 147.8){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 18);				
		}
		//Bar moving every 0.2 seconds 2nd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 147.8 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 148){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 148 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 148.2){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 148.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 148.4){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 17);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 148.4 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 148.6){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 18);				
		}
		//Bar moving every 0.2 seconds 3rd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 148.6 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 148.8){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 148.8 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 149){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 149 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 149.2){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 17);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 149.2 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 149.4){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 18);				
		}
		DisableStimuliGenL(moduleData);
	}

	if (state->stimulate2 == true) { //it runs when there is a spike
		//Bar moving every 5 seconds 1st time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC < 5 ){
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 10){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 10 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 15){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 15 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 20){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 11);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 20 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 25){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 12);				
		}
		//Bar moving every 5 seconds 2nd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 25 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 30){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 30 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 35){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 35 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 40){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 40 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 45){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 11);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 45 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 50){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 12);				
		}
		//Bar moving every 5 seconds 3rd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 50 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 55){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 55 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 60){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 60 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 65){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 65 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 70){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 11);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 70 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 75){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 12);				
		}
		//Rest
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 75 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 81){
			
		}
		//Bar moving every 3 seconds 1st time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 81 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 84){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 84 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 87){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 87 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 90){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 90 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 93){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 11);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 93 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 96){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 12);				
		}
		//Bar moving every 3 seconds 2nd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 96 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 99){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 99 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 102){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 102 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 105){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 105 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 108){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 11);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 108 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 111){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 12);				
		}
		//Bar moving every 3 seconds 3rd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 111 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 114){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 114 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 117){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 117 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 120){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 120 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 123){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 11);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 123 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 126){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 12);				
		}
		//Rest
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 126 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 132){
			
		}
		//Bar moving every 1 second 1st time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 132 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 133){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 133 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 134){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 134 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 135){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 135 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 136){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 11);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 136 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 137){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 12);				
		}
		//Bar moving every 1 second 2nd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 137 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 138){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 138 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 139){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 139 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 140){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 140 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 141){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 11);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 141 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 142){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 12);				
		}
		//Bar moving every 1 second 3rd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 142 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 143){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 143 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 144){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 144 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 145){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 145 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 146){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 11);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 146 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 147){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 12);				
		}
		//Rest
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 147 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 153){
			
		}
		//Bar moving every 0.5 seconds 1st time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 153 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 153.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 153.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 154){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 154 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 154.5){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 154.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 155){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 11);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 155 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 155.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 12);				
		}
		//Bar moving every 0.5 seconds 2nd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 155.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 156){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 156 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 156.5){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 156.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 157){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 157 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 157.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 11);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 157.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 158){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 12);				
		}
		//Bar moving every 0.5 seconds 3rd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 158 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 158.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 158.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 159){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 159 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 159.5){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 159.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 160){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 11);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 160 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 160.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 12);				
		}
		//Rest
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 160.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 166){
			
		}
		//Bar moving every 0.2 seconds 1st time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 166 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 166.2){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 166.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 166.4){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 166.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 166.6){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 166.6 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 166.8){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 11);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 166.8 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 167){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 12);				
		}
		//Bar moving every 0.2 seconds 2nd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 167 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 167.2){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 167.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 167.4){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 167.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 167.6){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 167.6 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 167.8){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 11);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 167.8 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 168){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 12);				
		}
		//Bar moving every 0.2 seconds 3rd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 168 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 168.2){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 168.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 168.4){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 168.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 168.6){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 168.6 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 168.8){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 11);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 168.8 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 169){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 12);				
		}
		DisableStimuliGenL(moduleData);
	}

	if (state->stimulate3 == true) { //it runs when there is a spike
		//Bar moving every 5 seconds 1st time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC < 5 ){
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 10){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 10 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 15){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		//Bar moving every 5 seconds 2nd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 15 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 20){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 20 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 25){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 25 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 30){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		//Bar moving every 5 seconds 3rd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 30 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 35){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 35 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 40){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 40 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 45){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		//Rest
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 45 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 51){
			
		}
		//Bar moving every 3 seconds 1st time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 51 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 54){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 54 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 57){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 57 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 60){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		//Bar moving every 3 seconds 2nd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 60 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 63){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 63 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 66){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 66 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 69){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		//Bar moving every 3 seconds 3rd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 69 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 72){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 72 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 75){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 75 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 78){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		//Rest
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 78 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 84){
			
		}
		//Bar moving every 1 second 1st time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 84 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 85){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 85 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 86){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 86 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 87){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		//Bar moving every 1 second 2nd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 87 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 88){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 88 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 89){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 89 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 90){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		//Bar moving every 1 second 3rd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 90 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 91){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 91 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 92){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 92 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 93){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		//Rest
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 93 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 99){
			
		}
		//Bar moving every 0.5 seconds 1st time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 99 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 99.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 99.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 100){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 100 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 100.5){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		//Bar moving every 0.5 seconds 2nd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 100.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 101){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 101 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 101.5){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 101.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 102){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		//Bar moving every 0.5 seconds 3rd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 102 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 102.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 102.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 103){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 103 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 103.5){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		//Rest
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 103.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 109){
			
		}
		//Bar moving every 0.2 seconds 1st time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 109 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 109.2){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 109.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 109.4){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 109.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 109.6){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		//Bar moving every 0.2 seconds 2nd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 109.6 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 109.8){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 109.8 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 110){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 110 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 110.2){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		//Bar moving every 0.2 seconds 3rd time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 110.2 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 110.4){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 110.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 110.6){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 110.6 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 110.8){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		DisableStimuliGenL(moduleData);
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


bool EnableStimuliGenL(caerModuleData moduleData, uint16_t address) {
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
	sshsNodePutInt(spikeNode, "stim_avr", 70); //Hz
	sshsNodePutInt(spikeNode, "address", address);
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

	bool newStimulate1 = sshsNodeGetBool(moduleData->moduleNode, "stimulate1");
	if (newStimulate1 && !state->stimulate1) {
		state->stimulate1 = newStimulate1;
		state->start = clock();
	}
	else if (!newStimulate1 && state->stimulate1) {
		state->stimulate1 = newStimulate1;
	}
	bool newStimulate2 = sshsNodeGetBool(moduleData->moduleNode, "stimulate2");
	if (newStimulate2 && !state->stimulate2) {
		state->stimulate2 = newStimulate2;
		state->start = clock();
	}
	else if (!newStimulate2 && state->stimulate2) {
		state->stimulate2 = newStimulate2;
	}
	bool newStimulate3 = sshsNodeGetBool(moduleData->moduleNode, "stimulate3");
	if (newStimulate3 && !state->stimulate3) {
		state->stimulate3 = newStimulate3;
		state->start = clock();
	}
	else if (!newStimulate3 && state->stimulate3) {
		state->stimulate3 = newStimulate3;
	}
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
