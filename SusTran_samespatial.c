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
				caerDynapseWriteCam(stateSource->deviceState, 15, neuronid, camid+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 22, neuronid, camid+6, DYNAPSE_CONFIG_CAMTYPE_F_EXC);

				caerDynapseWriteCam(stateSource->deviceState, 2, neuronid+16, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 8, neuronid+16, camid+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 9, neuronid+16, camid+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 15, neuronid+16, camid+8, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 16, neuronid+16, camid+10, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 22, neuronid+16, camid+12, DYNAPSE_CONFIG_CAMTYPE_F_EXC);

				caerDynapseWriteCam(stateSource->deviceState, 1, neuronid+16*2, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 9, neuronid+16*2, camid+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 10, neuronid+16*2, camid+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 15, neuronid+16*2, camid+6, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 16, neuronid+16*2, camid+8, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 22, neuronid+16*2, camid+10, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				
				caerDynapseWriteCam(stateSource->deviceState, 2, neuronid+16*3, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 10, neuronid+16*3, camid+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 16, neuronid+16*3, camid+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);	
				caerDynapseWriteCam(stateSource->deviceState, 22, neuronid+16*3, camid+6, DYNAPSE_CONFIG_CAMTYPE_F_EXC);

							
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
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*2, 577, camid+1, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			
			camid+=4;
		}
		camid = 2;
		for(size_t preneuron = 0; preneuron<3; preneuron++){		
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16, 577, camid, DYNAPSE_CONFIG_CAMTYPE_S_INH);			
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*3, 577, camid+1, DYNAPSE_CONFIG_CAMTYPE_S_EXC);
			camid+=4;
		}
		camid = 14;
		for(size_t preneuron = 3; preneuron<6; preneuron++){		
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16, 577, camid, DYNAPSE_CONFIG_CAMTYPE_S_EXC);	
			//caerDynapseWriteCam(stateSource->deviceState, preneuron+16*3, 577, camid+1, DYNAPSE_CONFIG_CAMTYPE_S_INH);
			camid+=2;
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
		//Rest
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 110.8 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 116){
				
		}
		//Bar moving every 0.05 seconds 1st time
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 116 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 116.05){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 8);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 116.05 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 116.1){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 9);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 116.1 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 116.15){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		//Bar moving every 0.05 seconds 2nd time
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 116.15 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 116.2){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 8);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 116.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 116.25){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 9);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 116.25 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 116.30){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		//Bar moving every 0.05 seconds 3rd time
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 116.30 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 116.35){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData,8);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 116.35 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 116.4){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 9);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 116.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 116.45){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);	
		}
		DisableStimuliGenL(moduleData);
	}
	if (state->stimulate2 == true ) { 
		//Bar width = 1 moving every 5 seconds 1st time
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC < 5 ){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 10){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		//Bar width = 1 moving every 5 seconds 2nd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 10 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 15){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 15 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 20){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		//Bar width = 1 moving every 5 seconds 3rd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 20 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 25){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 25 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 30){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		//Rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 30 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 36){
			
		}
		//Bar width = 1 moving every 3 seconds 1st time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 36 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 39){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 39 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 42){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		//Bar width = 1 moving every 3 seconds 2nd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 42 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 45){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 45 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 48){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		//Bar width = 1 moving every 3 seconds 3rd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 48 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 51){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 51 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 54){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		//Rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 54 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 60){
			
		}
		//Bar width = 1 moving every 1 second 1st time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 60 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 61){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 61 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 62){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		//Bar width = 1 moving every 1 second 2nd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 62 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 63){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 63 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 64){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		//Bar width = 1 moving every 1 second 3rd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 64 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 65){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 65 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 66){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		//Rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 66 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 72){
			
		}
		//Bar width = 1 moving every 0.5 seconds 1st time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 72 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 72.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 72.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 73){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		//Bar width = 1 moving every 0.5 seconds 2nd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 73 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 73.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 73.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 74){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		//Bar width = 1 moving every 0.5 seconds 3rd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 74 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 74.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 74.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 75){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		//Rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 75 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 81){
			
		}
		//Bar width = 1 moving every 0.2 seconds 1st time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 81 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 81.2){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 81.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 81.4){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		//Bar width = 1 moving every 0.2 seconds 2nd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 81.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 81.6){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 81.6 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 81.8){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		//Bar width = 1 moving every 0.2 seconds 3rd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 81.8 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 82){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 1);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 82 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 82.2){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		//Big rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 82.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 90){
		
		}
		//Bar width = 2 moving every 5 seconds 1st time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 90 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 95){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 95 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 100){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 100 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 105){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 10);
		}
		//Bar width = 2 moving every 5 seconds 2nd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 105 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 110){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 110 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 115){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 115 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 120){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 10);
		}
		//Bar width = 2 moving every 5 seconds 3rd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 120 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 125){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 125 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 130){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 130 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 135){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 10);
		}
		//Rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 135 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 141){
			
		}
		//Bar width = 2 moving every 3 seconds 1st time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 141 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 144){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 144 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 147){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 147 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 150){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 10);
		}
		//Bar width = 2 moving every 3 seconds 2nd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 150 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 153){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 153 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 156){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 156 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 159){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 10);
		}
		//Bar width = 2 moving every 3 seconds 3rd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 159 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 162){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 162 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 165){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 165 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 168){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 10);
		}
		//Rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 168&& (float)(clock() - state->start) / CLOCKS_PER_SEC < 174){
			
		}
		//Bar width = 2 moving every 1 second 1st time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 174 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 175){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 175 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 176){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 176 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 177){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 10);
		}
		//Bar width = 2 moving every 1 second 2nd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 177 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 178){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 178 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 179){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 179 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 180){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 10);
		}
		//Bar width = 2 moving every 1 second 3rd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 180 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 181){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 181 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 182){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 182 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 183){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 10);
		}
		//Rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 183 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 189){
			
		}
		//Bar width = 2 moving every 0.5 seconds 1st time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 189 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 189.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 189.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 190){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 190 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 190.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 10);
		}
		//Bar width = 2 moving every 0.5 seconds 2nd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 190.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 191){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 191 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 191.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 191.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 192){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 10);
		}
		//Bar width = 2 moving every 0.5 seconds 3rd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 192 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 192.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 192.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 193){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 193 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 193.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 10);
		}
		//Rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 193.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 201.0){
			
		}
		//Bar width = 2 moving every 0.2 seconds 1st time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 201 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 201.2){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 201.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 201.4){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 201.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 201.6){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 10);
		}
		//Bar width = 2 moving every 0.2 seconds 2nd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 201.6 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 201.8){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 201.8 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 202){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 202 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 202.2){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 10);
		}
		//Bar width = 2 moving every 0.2 seconds 3rd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 202.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 202.4){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 8);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 202.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 202.6){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 9);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 202.6 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 202.8){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 10);
		}
		//Big Rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 202.8 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 210){
			
		}
		//Bar width = 3 moving every 5 seconds 1st time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 210 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 215){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 215 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 220){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar width = 3 moving every 5 seconds 2nd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 220 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 225){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 225 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 230){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar width = 3 moving every 5 seconds 3rd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 230 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 235){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 235 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 240){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 240 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 246){
			
		}
		//Bar width = 3 moving every 3 seconds 1st time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 246 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 249){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 249 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 252){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar width = 3 moving every 3 seconds 2nd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 252 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 255){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 255 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 258){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar width = 3 moving every 3 seconds 3rd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 258 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 261){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 261 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 264){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 264 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 270){
			
		}
		//Bar width = 3 moving every 1 second 1st time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 270 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 271){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 271 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 272){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar width = 3 moving every 1 second 2nd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 272 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 273){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 273 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 274){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar width = 3 moving every 1 second 3rd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 274 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 275){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 275 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 276){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 276 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 284){
			
		}
		//Bar width = 3 moving every 0.5 seconds 1st time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 284 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 284.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 284.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 285){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar width = 3 moving every 0.5 seconds 2nd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 285 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 285.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 285.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 286){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar width = 3 moving every 0.5 seconds 3rd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 286 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 286.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 286.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 287){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 287 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 295){
			
		}
		//Bar width = 3 moving every 0.2 seconds 1st time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 295 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 295.2){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 295.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 295.4){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar width = 3 moving every 0.2 seconds 2nd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 295.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 295.6){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 295.6 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 295.8){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Bar width = 3 moving every 0.2 seconds 3rd time
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 295.8 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 296){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 15);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 296 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 296.2){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 16);
		}
		//Big Rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 296.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 305){
			
		}
		//Bar width = 4 moving every 5 seconds 
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 305 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 310){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 22);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 310 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 315){
			
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 315 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 320){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 22);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 320 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 325){
			
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 325 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 330){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 22);
		}
		//Rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 330 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 336){
			
		}
		//Bar width = 4 moving every 3 seconds 
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 336 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 339){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 22);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 339 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 342){
			
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 342 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 345){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 22);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 345 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 348){
			
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 348 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 351){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 22);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 351 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 354){
			
		}
		//Rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 354 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 361){
			
		}
		//Bar width = 4 moving every 1 second 
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 361 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 362){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 22);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 362 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 363){
			
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 363 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 364){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 22);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 364 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 365){
			
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 365 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 366){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 22);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 366 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 367){
			
		}
		//Rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 367 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 373){
			
		}
		//Bar width = 4 moving every 0.5 seconds 
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 373 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 373.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 22);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 373.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 374){
			
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 374 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 374.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 22);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 374.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 375){
			
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 375 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 375.5){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 22);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 375.5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 376){
			
		}
		//Rest
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 376 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 385){
			
		}
		//Bar width = 4 moving every 0.2 seconds 
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 385 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 385.2){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 22);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 385.2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 385.4){
			
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 385.4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 385.6){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 22);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 385.6 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 385.8){
			
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 385.8 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 386){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 22);
		}
		if ((float)(clock() - state->start) / CLOCKS_PER_SEC >= 386 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 386.2){
			
		}
		DisableStimuliGenL(moduleData);
	}
	if (state->stimulate3 == true ) { 
		// w = 1 at 1 second
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC < 1 ){
			EnableStimuliGenL(moduleData, 1);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 1 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 2){	
			
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 2 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 3){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 1);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 3 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 4){	
			
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 4 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 5){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 1);
		}
		//Big Rest
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 15){	
			
		}
		// w = 2 at 1 second
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 15 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 16){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 16 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 17){	
			
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 17 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 18){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 18 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 19){	
			
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 19 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 20){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 10);
		}
		//Big Rest
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 20 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 30){	
			
		}
		// w = 3 at 1 second
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 30 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 31){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 16);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 31 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 32){	
			
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 32 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 33){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 16);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 33 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 34){	
			
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 34 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 35){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 16);
		}
		//Big Rest
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 35 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 45){	
			
		}
		// w = 4 at 1 second
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 45 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 46){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 22);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 46 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 47){	
			
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 47 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 48){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 22);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 48 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 49){	
			
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 49 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 50){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 22);
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






