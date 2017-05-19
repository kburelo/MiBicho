/*
 * helloworld.c
 *
 *  Created on: Feb 2017 for tutorial on dynap-se
 *      Author: federico
 */

#include "helloworld.h"
#include "base/mainloop.h"
#include "base/module.h"
#include <time.h>



struct HWFilter_state {
	// user settings
	int eventSourceID; 
	//bool doConnection;
	//bool displayNeuNumber;
	//bool setBiases;
	clock_t start;
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
	//sshsNodePutBoolIfAbsent(moduleData->moduleNode, "stimulate1", false);
	//sshsNodePutBoolIfAbsent(moduleData->moduleNode, "stimulate2", false);
	//sshsNodePutBoolIfAbsent(moduleData->moduleNode, "stimulate3", false);
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
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_RFR_N", 8,108, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_TAU1_N", 6,24, "LowBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_TAU2_N", 5, 15, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "IF_THR_N", 4, 20, "HighBias", "NBias");
			caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, coreid, "NPDPIE_TAU_F_P", 5, 41, "HighBias","PBias");
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
	  	caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 1, "NPDPIE_TAU_F_P", 5, 53, "HighBias", "PBias");
		caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 1, "IF_TAU1_N", 6,25, "LowBias", "NBias");
		caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 1, "IF_RFR_N", 5,108, "HighBias", "NBias");

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
				caerDynapseWriteCam(stateSource->deviceState, 6, neuronid, camid+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 15, neuronid, camid+6, DYNAPSE_CONFIG_CAMTYPE_F_EXC);

				caerDynapseWriteCam(stateSource->deviceState, 2, neuronid+16, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 8, neuronid+16, camid+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 9, neuronid+16, camid+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 5, neuronid+16, camid+6, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 6, neuronid+16, camid+8, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 15, neuronid+16, camid+10, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 16, neuronid+16, camid+12, DYNAPSE_CONFIG_CAMTYPE_F_EXC);

				caerDynapseWriteCam(stateSource->deviceState, 3, neuronid+16*2, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 9, neuronid+16*2, camid+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 10, neuronid+16*2, camid+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 5, neuronid+16*2, camid+6, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 6, neuronid+16*2, camid+8, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 15, neuronid+16*2, camid+10, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 16, neuronid+16*2, camid+12, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				
				caerDynapseWriteCam(stateSource->deviceState, 4, neuronid+16*3, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 10, neuronid+16*3, camid+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 5, neuronid+16*3, camid+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);	
				caerDynapseWriteCam(stateSource->deviceState, 6, neuronid+16*3, camid+6, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 16, neuronid+16*3, camid+8, DYNAPSE_CONFIG_CAMTYPE_F_EXC);

							
			}
			caerDynapseWriteSram(stateSource->deviceState, 0, neuronid, 0, 0, 0, 0, 0, 1, 6);
			caerDynapseWriteSram(stateSource->deviceState, 0, neuronid+16, 0, 0, 0, 0, 0, 1, 6);
			caerDynapseWriteSram(stateSource->deviceState, 0, neuronid+16*2, 0, 0, 0, 0, 0, 1, 6);
			caerDynapseWriteSram(stateSource->deviceState, 0, neuronid+16*3, 0, 0, 0, 0, 0, 1, 6);

		}

		//Setting cams for the neurons in core 2 (V1 neurons)

		size_t camid = 0;
		for(size_t preneuron = 0; preneuron<6; preneuron++){
			caerDynapseWriteCam(stateSource->deviceState, preneuron, 529, camid, DYNAPSE_CONFIG_CAMTYPE_S_INH);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16, 529, camid+1, DYNAPSE_CONFIG_CAMTYPE_S_INH);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*2, 529, camid+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*3, 529, camid+3, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron, 577, camid, DYNAPSE_CONFIG_CAMTYPE_S_INH);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16, 577, camid+1, DYNAPSE_CONFIG_CAMTYPE_S_INH);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*2, 577, camid+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*3, 577, camid+3, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			camid+=4;
		}
		caerDynapseWriteCam(stateSource->deviceState, 0, 410, 0, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState, 1, 410, 4, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState, 2, 410, 8, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState, 3, 410, 12, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState, 4, 410, 16, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState, 5, 410, 20, DYNAPSE_CONFIG_CAMTYPE_S_INH);

		caerDynapseWriteCam(stateSource->deviceState, 0+16, 410, 0+1, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState, 1+16, 410, 4+1, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState, 2+16, 410, 8+1, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState, 3+16, 410, 12+1, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState, 4+16, 410, 16+1, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState, 5+16, 410, 20+1, DYNAPSE_CONFIG_CAMTYPE_S_INH);

		caerDynapseWriteCam(stateSource->deviceState, 0+16*2, 410, 0+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 1+16*2, 410, 4+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 2+16*2, 410, 8+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 3+16*2, 410, 12+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 4+16*2, 410, 16+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 5+16*2, 410, 20+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);

		caerDynapseWriteCam(stateSource->deviceState, 0+16*3, 410, 0+3, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 1+16*3, 410, 4+3, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 2+16*3, 410, 8+3, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 3+16*3, 410, 12+3, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 4+16*3, 410, 16+3, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 5+16*3, 410, 20+3, DYNAPSE_CONFIG_CAMTYPE_F_EXC);

		caerDynapseWriteCam(stateSource->deviceState, 0+16*2, 410, 0+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 1+16*2, 410, 4+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 2+16*2, 410, 8+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 3+16*2, 410, 12+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 4+16*2, 410, 16+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 5+16*2, 410, 20+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);

		caerDynapseWriteCam(stateSource->deviceState, 0+16*3, 410, 0+5, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 1+16*3, 410, 4+5, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 2+16*3, 410, 8+5, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 3+16*3, 410, 12+5, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 4+16*3, 410, 16+5, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 5+16*3, 410, 20+5, DYNAPSE_CONFIG_CAMTYPE_F_EXC);

		caerDynapseWriteCam(stateSource->deviceState, 410, 529, 24, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState, 410, 529, 25, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState, 410, 529, 26, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState, 410, 529, 27, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState, 410, 529, 28, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState, 410, 581, 1, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 410, 581, 2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState, 410, 581, 3, DYNAPSE_CONFIG_CAMTYPE_F_EXC);

		for (camid=0;camid<5;camid++){
			caerDynapseWriteCam(stateSource->deviceState, 529, 533, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			caerDynapseWriteCam(stateSource->deviceState, 577, 353, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		}
		
		caerDynapseWriteSram(stateSource->deviceState, 1, 154, 1, 0, 0, 0, 0, 1, 4);
		caerDynapseWriteSram(stateSource->deviceState, 2, 17, 2, 0, 0, 0, 0, 1, 4);
		caerDynapseWriteSram(stateSource->deviceState, 2, 65, 2, 0, 0, 0, 0, 1, 4);
		

		//Self excitation and inhibition from neighbour 
		/*caerDynapseWriteCam(stateSource->deviceState,529, 529, 25, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		caerDynapseWriteCam(stateSource->deviceState,529, 529, 26, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		//caerDynapseWriteCam(stateSource->deviceState,529, 529, 27, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
		//caerDynapseWriteCam(stateSource->deviceState,561, 529, camid, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState,273, 529, 27, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState,273, 529, 28, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState,273, 529, 29, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState,273, 529, 30, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState,273, 529, 31, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState,273, 529, 32, DYNAPSE_CONFIG_CAMTYPE_F_INH);
		caerDynapseWriteCam(stateSource->deviceState,273, 529, 33, DYNAPSE_CONFIG_CAMTYPE_F_INH);
		caerDynapseWriteCam(stateSource->deviceState,273, 529, 34, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState,273, 529, 35, DYNAPSE_CONFIG_CAMTYPE_S_INH);
		caerDynapseWriteCam(stateSource->deviceState,273, 529, 36, DYNAPSE_CONFIG_CAMTYPE_S_INH);*/

		
		/*for (camid=25; camid<27; camid++){
			//caerDynapseWriteCam(stateSource->deviceState,529, 529, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			caerDynapseWriteCam(stateSource->deviceState,545, 529, camid, DYNAPSE_CONFIG_CAMTYPE_S_INH);
			caerDynapseWriteCam(stateSource->deviceState,545, 529, camid, DYNAPSE_CONFIG_CAMTYPE_F_INH);
			caerDynapseWriteCam(stateSource->deviceState,545, 529, camid, DYNAPSE_CONFIG_CAMTYPE_F_INH);
		}*/
		
		caerLog(CAER_LOG_NOTICE, __func__, "Cams and Srams for chip U0 have been set");

		state->init = true;
	}
	if (state->stimulate1 == true ) { 
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
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 110.8 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 116){
		
		}
		//Bar moving every 5 seconds 1st time width = 1
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 116 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 121){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 121 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 126){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 126 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 131){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 3);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 131 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 136){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 4);
		}
		//Bar moving every 5 seconds 2nd time width = 1
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 136 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 141){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 141 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 146){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 146 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 151){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 3);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 151 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 156){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 4);
		}
		//Bar moving every 5 seconds 3rd time width = 1
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 156 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 161){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 161 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 166){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 166 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 171){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 3);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 171 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 176){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 4);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 176 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 182){
		
		}
		//Bar moving every 3 seconds 1st time width = 1
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 182 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 185){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 185 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 188){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 188 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 191){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 3);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 191 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 194){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 4);
		}
		//Bar moving every 3 seconds 2nd time width = 1
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 194 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 197){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 197 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 200){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 200 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 203){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 3);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 203 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 206){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 4);
		}
		//Bar moving every 3 seconds 3rd time width = 1
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 206 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 209){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 209 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 212){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 212 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 215){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 3);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 215 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 218){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 4);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 218 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 224){
		
		}
		//Bar moving every 1 seconds 1st time width = 1
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 224 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 225){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 225 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 226){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 226 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 227){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 3);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 227 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 228){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 4);
		}
		//Bar moving every 1 seconds 2nd time width = 1
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 228 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 229){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 229 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 230){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 230 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 231){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 3);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 231 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 232){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 4);
		}
		//Bar moving every 1 seconds 3rd time width = 1
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 232 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 233){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 233 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 234){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 234 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 235){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 3);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 235 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 236){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 4);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 236 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 242){
		
		}
		//Bar moving every 0.5 seconds 1st time width = 1
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 242 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 242.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 242.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 243){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 243 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 243.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 3);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 243.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 244){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 4);
		}
		//Bar moving every 0.5 seconds 2nd time width = 1
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 244 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 244.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 244.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 245){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 245 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 245.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 3);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 245.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 246){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 4);
		}
		//Bar moving every 0.5 seconds 3rd time width = 1
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 246 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 246.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 246.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 247){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 247 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 247.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 3);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 247.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 248){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 4);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 248 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 254){
		
		}
		//Bar moving every 0.2 seconds 1st time width = 1
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 254 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 254.2){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 254.2 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 254.4){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 254.4 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 254.6){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 3);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 254.6 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 254.8){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 4);
		}
		//Bar moving every 0.2 seconds 2nd time width = 1
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 254.8 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 255){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 255 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 255.2){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 255.2 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 255.4){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 3);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 255.4 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 255.6){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 4);
		}
		//Bar moving every 0.2 seconds 3rd time width = 1
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 255.6 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 255.8){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 255.8 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 256){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 256 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 256.2){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 3);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 256.2 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 256.4){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 4);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 256.4 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 265){
		
		}
		//Bar moving every 5 seconds 1st time width = 3
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 265 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 270){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 270 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 275){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar moving every 5 seconds 2nd time width = 3
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 275 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 280){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 280 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 285){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar moving every 5 seconds 3rd time width = 3
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 285 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 290){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 290 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 295){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 295 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 300){
		
		}
		//Bar moving every 3 seconds 1st time width = 3
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 300 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 303){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 303 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 306){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar moving every 3 seconds 2nd time width = 3
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 306 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 309){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 309 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 312){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar moving every 3 seconds 3rd time width = 3
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 312 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 315){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 315 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 318){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 318 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 323){
		
		}
		//Bar moving every 1 seconds 1st time width = 3
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 323 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 324){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 324 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 325){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar moving every 1 seconds 2nd time width = 3
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 325 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 326){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 326 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 327){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar moving every 1 seconds 3rd time width = 3
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 327 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 328){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 328 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 329){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 329 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 335){
		
		}
		//Bar moving every 0.5 seconds 1st time width = 3
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 335 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 335.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 335.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 336){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar moving every 0.5 seconds 2nd time width = 3
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 336 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 336.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 336.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 337){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar moving every 0.5 seconds 3rd time width = 3
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 337 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 337.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 337.5 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 338){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 338 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 344){
		
		}
		//Bar moving every 0.2 seconds 1st time width = 3
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 344 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 344.2){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 344.2 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 344.4){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar moving every 0.2 seconds 2nd time width = 3
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 344.4 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 344.6){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 344.6 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 344.8){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar moving every 0.2 seconds 3rd time width = 3
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 344.8 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 345){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 345 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 345.2){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		DisableStimuliGenL(moduleData);
	}
	if (state->stimulate2 == true ) { 
		//Bar appearing and desappearing (1 second) in postition 3 widht = 1
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC < 1 ){
			EnableStimuliGenL(moduleData, 3);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 1 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 2){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 3){
			EnableStimuliGenL(moduleData, 3);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 3 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 4){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 5){
			EnableStimuliGenL(moduleData, 3);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 6){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 6 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 12){
			
		}
		//Bar appearing and desappearing (1 second) in postition 4 widht = 1
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 12 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 13){
			EnableStimuliGenL(moduleData, 4);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 13 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 14){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 14 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 15){
			EnableStimuliGenL(moduleData, 4);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 15 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 16){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 16 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 17){
			EnableStimuliGenL(moduleData, 4);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 17 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 18){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 18 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 24){
		
		}
		//Bar appearing and desappearing (1 second) in postition 3 and 4 widht = 2
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 24 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 25){
			EnableStimuliGenL(moduleData, 10);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 25 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 26){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 26 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 27){
			EnableStimuliGenL(moduleData, 10);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 27 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 28){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 28 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 29){
			EnableStimuliGenL(moduleData, 10);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 29 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 30){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 30 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 36){
		
		}
		//Bar appearing and desappearing (1 second) in postition 2,3 and 4 widht = 3
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 36 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 37){
			EnableStimuliGenL(moduleData, 5);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 37 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 38){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 38 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 39){
			EnableStimuliGenL(moduleData, 5);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 39 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 40){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 40 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 41){
			EnableStimuliGenL(moduleData, 5);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 41 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 42){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 42 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 48){
		
		}
		//Bar appearing and desappearing (1 second) in postition 1,2,3 and 4 widht = 4
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 48 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 49){
			EnableStimuliGenL(moduleData, 6);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 49 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 50){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 50 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 51){
			EnableStimuliGenL(moduleData, 6);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 51 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 52){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 52 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 53){
			EnableStimuliGenL(moduleData, 6);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 53 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 54){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 54 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 60){
		
		}
		//Bar appearing and desappearing (3 seconds) in postition 3 widht = 1
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 60 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 63){
			EnableStimuliGenL(moduleData, 3);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 63 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 66){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 66 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 69){
			EnableStimuliGenL(moduleData, 3);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 69 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 72){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 72 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 75){
			EnableStimuliGenL(moduleData, 3);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 75 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 78){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 78 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 84){
			
		}
		//Bar appearing and desappearing (3 seconds) in postition 4 widht = 1
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 84 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 87){
			EnableStimuliGenL(moduleData, 4);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 87 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 90){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 90 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 93){
			EnableStimuliGenL(moduleData, 4);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 93 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 96){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 96 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 99){
			EnableStimuliGenL(moduleData, 4);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 99 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 102){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 102 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 108){
		
		}
		//Bar appearing and desappearing (3 seconds) in postition 3 and 4 widht = 2
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 108 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 111){
			EnableStimuliGenL(moduleData, 10);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 111 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 114){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 114 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 117){
			EnableStimuliGenL(moduleData, 10);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 117 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 120){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 120 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 123){
			EnableStimuliGenL(moduleData, 10);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 123 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 126){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 126 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 132){
		
		}
		//Bar appearing and desappearing (3 seconds) in postition 2,3 and 4 widht = 3
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 132 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 135){
			EnableStimuliGenL(moduleData, 5);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 135 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 138){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 138 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 141){
			EnableStimuliGenL(moduleData, 5);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 141 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 144){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 144 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 147){
			EnableStimuliGenL(moduleData, 5);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 147 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 150){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 150 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 156){
		
		}
		//Bar appearing and desappearing (3 seconds) in postition 1,2,3 and 4 widht = 4
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 156 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 159){
			EnableStimuliGenL(moduleData, 6);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 159 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 162){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 162 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 165){
			EnableStimuliGenL(moduleData, 6);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 165 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 168){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 168 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 171){
			EnableStimuliGenL(moduleData, 6);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 171 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 174){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 174 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 180){
		
		}
		//Bar appearing and desappearing (5 seconds) in postition 3 widht = 1
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 180 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 185){
			EnableStimuliGenL(moduleData, 3);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 185 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 190){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 190 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 195){
			EnableStimuliGenL(moduleData, 3);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 195 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 200){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 200 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 205){
			EnableStimuliGenL(moduleData, 3);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 205 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 210){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 210 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 215){
			
		}
		//Bar appearing and desappearing (5 seconds) in postition 4 widht = 1
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 215 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 220){
			EnableStimuliGenL(moduleData, 4);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 220 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 225){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 225 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 230){
			EnableStimuliGenL(moduleData, 4);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 230 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 235){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 235 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 240){
			EnableStimuliGenL(moduleData, 4);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 240 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 245){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 245 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 250){
		
		}
		//Bar appearing and desappearing (5 seconds) in postition 3 and 4 widht = 2
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 250 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 255){
			EnableStimuliGenL(moduleData, 10);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 255 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 260){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 260 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 265){
			EnableStimuliGenL(moduleData, 10);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 265 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 270){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 270 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 275){
			EnableStimuliGenL(moduleData, 10);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 275 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 280){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 280 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 285){
		
		}
		//Bar appearing and desappearing (5 seconds) in postition 2,3 and 4 widht = 3
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 285 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 290){
			EnableStimuliGenL(moduleData, 5);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 290 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 295){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 295 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 300){
			EnableStimuliGenL(moduleData, 5);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 300 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 305){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 305 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 310){
			EnableStimuliGenL(moduleData, 5);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 310 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 315){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 315 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 320){
		
		}
		//Bar appearing and desappearing (5 seconds) in postition 1,2,3 and 4 widht = 4
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 320 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 325){
			EnableStimuliGenL(moduleData, 6);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 325 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 330){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 330 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 335){
			EnableStimuliGenL(moduleData, 6);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 335 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 340){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 340 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 345){
			EnableStimuliGenL(moduleData, 6);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 345 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 350){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 350 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 355){
		
		}
		//Bar appearing and desappearing (0.5 seconds) in postition 3 widht = 1
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 355 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 355.5){
			EnableStimuliGenL(moduleData, 3);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 355.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 356){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 356 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 356.5){
			EnableStimuliGenL(moduleData, 3);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 356.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 357){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 357 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 357.5){
			EnableStimuliGenL(moduleData, 3);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 357.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 358){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 358 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 364){
			
		}
		//Bar appearing and desappearing (0.5 seconds) in postition 4 widht = 1
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 364 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 364.5){
			EnableStimuliGenL(moduleData, 4);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 364.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 365){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 365 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 365.5){
			EnableStimuliGenL(moduleData, 4);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 365.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 366){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 366 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 366.5){
			EnableStimuliGenL(moduleData, 4);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 366.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 367){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 367 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 373){
		
		}
		//Bar appearing and desappearing (0.5 seconds) in postition 3 and 4 widht = 2
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 373 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 373.5){
			EnableStimuliGenL(moduleData, 10);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 373.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 374){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 374 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 374.5){
			EnableStimuliGenL(moduleData, 10);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 374.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 375){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 375 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 375.5){
			EnableStimuliGenL(moduleData, 10);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 375.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 376){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 376 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 376.5){
		
		}
		//Bar appearing and desappearing (0.5 seconds) in postition 2,3 and 4 widht = 3
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 376.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 377){
			EnableStimuliGenL(moduleData, 5);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 377 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 377.5){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 377.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 378){
			EnableStimuliGenL(moduleData, 5);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 378 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 378.5){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 378.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 379){
			EnableStimuliGenL(moduleData, 5);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 379 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 379.5){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 379.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 380){
		
		}
		//Bar appearing and desappearing (0.5 seconds) in postition 1,2,3 and 4 widht = 4
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 380 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 380.5){
			EnableStimuliGenL(moduleData, 6);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 380.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 381){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 381 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 381.5){
			EnableStimuliGenL(moduleData, 6);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 381.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 382){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 382 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 382.5){
			EnableStimuliGenL(moduleData, 6);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 382.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 383){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 383 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 389){
		
		}
		//Bar appearing and desappearing (0.2 seconds) in postition 3 widht = 1
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 389 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 389.2){
			EnableStimuliGenL(moduleData, 3);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 389.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 389.4){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 389.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 389.6){
			EnableStimuliGenL(moduleData, 3);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 389.6 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 389.8){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 389.8 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 390){
			EnableStimuliGenL(moduleData, 3);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 390 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 390.2){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 390.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 396.2){
			
		}
		//Bar appearing and desappearing (0.2 seconds) in postition 4 widht = 1
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 396.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 396.4){
			EnableStimuliGenL(moduleData, 4);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 396.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 396.6){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 396.6 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 396.8){
			EnableStimuliGenL(moduleData, 4);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 396.8 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 397){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 397 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 397.2){
			EnableStimuliGenL(moduleData, 4);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 397.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 397.4){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 397.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 403.4){
		
		}
		//Bar appearing and desappearing (0.2 seconds) in postition 3 and 4 widht = 2
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 403.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 403.6){
			EnableStimuliGenL(moduleData, 10);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 403.6 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 403.8){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 403.8 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 404){
			EnableStimuliGenL(moduleData, 10);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 404 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 404.2){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 404.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 404.4){
			EnableStimuliGenL(moduleData, 10);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 404.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 404.6){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 404.6 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 410.6){
		
		}
		//Bar appearing and desappearing (0.2 seconds) in postition 2,3 and 4 widht = 3
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 410.6 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 410.8){
			EnableStimuliGenL(moduleData, 5);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 410.8 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 411){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 411 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 411.2){
			EnableStimuliGenL(moduleData, 5);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 411.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 411.4){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 411.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 411.6){
			EnableStimuliGenL(moduleData, 5);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 411.6 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 411.8){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 411.8 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 417.8){
		
		}
		//Bar appearing and desappearing (0.2 seconds) in postition 1,2,3 and 4 widht = 4
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 417.8 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 418){
			EnableStimuliGenL(moduleData, 6);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 418 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 418.2){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 418.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 418.4){
			EnableStimuliGenL(moduleData, 6);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 418.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 418.6){
			DisableStimuliGenL(moduleData);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 418.6 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 418.8){
			EnableStimuliGenL(moduleData, 6);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 418.8 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 419){
			DisableStimuliGenL(moduleData);
		}
		DisableStimuliGenL(moduleData);
	}
		
}
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
	sshsNodePutInt(spikeNode, "stim_duration", 5); //seconds 
	sshsNodePutInt(spikeNode, "stim_avr", 50); //Hz
	sshsNodePutInt(spikeNode, "address", address);
	sshsNodePutInt(spikeNode, "core_d", 1);
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






