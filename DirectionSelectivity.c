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
	  	//caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 2, "NPDPII_TAU_S_P", 5, 28, "HighBias", "PBias");
		//caerDynapseSetBias(stateSource, DYNAPSE_CONFIG_DYNAPSE_U0, 1, "PS_WEIGHT_EXC_S_N", 0, 150, "HighBias", "NBias");

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
				caerDynapseWriteCam(stateSource->deviceState, 2, neuronid+16, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 3, neuronid+16*2, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 4, neuronid+16*3, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 5, neuronid+16*4, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
				caerDynapseWriteCam(stateSource->deviceState, 6, neuronid+16*5, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);			
			}
			caerDynapseWriteSram(stateSource->deviceState, 0, neuronid, 0, 0, 0, 0, 0, 1, 6);
			caerDynapseWriteSram(stateSource->deviceState, 0, neuronid+16, 0, 0, 0, 0, 0, 1, 6);
			caerDynapseWriteSram(stateSource->deviceState, 0, neuronid+16*2, 0, 0, 0, 0, 0, 1, 6);
			caerDynapseWriteSram(stateSource->deviceState, 0, neuronid+16*3, 0, 0, 0, 0, 0, 1, 6);
			caerDynapseWriteSram(stateSource->deviceState, 0, neuronid+16*4, 0, 0, 0, 0, 0, 1, 6);
			caerDynapseWriteSram(stateSource->deviceState, 0, neuronid+16*5, 0, 0, 0, 0, 0, 1, 6);

		}

		//Setting cams for the neurons in core 2 (V1 neurons)

		size_t camid = 0;
		for(size_t preneuron = 0; preneuron<6; preneuron++){
			caerDynapseWriteCam(stateSource->deviceState, preneuron, 529, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16, 529, camid+1, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*2, 529, camid+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*3, 529, camid+3, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*4, 529, camid+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*5, 273, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*4, 273, camid+1, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*3, 273, camid+2, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16*2, 273, camid+3, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			caerDynapseWriteCam(stateSource->deviceState, preneuron+16, 273, camid+4, DYNAPSE_CONFIG_CAMTYPE_F_EXC);
			camid+=5;
		}
		for (camid=0; camid<6; camid++){
			caerDynapseWriteCam(stateSource->deviceState, 50, 282, camid, DYNAPSE_CONFIG_CAMTYPE_F_EXC);//26 core 1
		}

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
	if (state->stimulate == true ) { 
		//Bar going right (5 seconds)
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC < 5 ){
			EnableStimuliGenL(moduleData, 1);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >=5 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 10){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 10 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 15){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 3);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >=15 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 20){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 4);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >=20 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 25){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 5);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >=25 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 30){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 6);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 30 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 35){
	
		}
		//Bar going left (5 seconds)
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 35 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 40){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 6);	
		}	
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 40 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 45){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 5);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 45 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 50){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 4);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 50 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 55){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 3);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 55 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 60){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 2);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 60 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 65){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 1);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 65 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 70){	
						
		}
		//Bar going right (3 seconds)
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 70 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 73){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 1);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 73 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 76){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 2);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 76 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 79){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 3);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 79 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 82){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 4);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 82 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 85){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 5);			
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 85 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 88){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 6);			
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 88 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 93){	
					
		}
		//Bar going left (3 seconds)
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 93 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 96){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 6);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 96 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 99){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 5);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 99 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 102){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 4);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 102 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 105){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 3);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 105 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 108){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 2);			
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 108 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 111){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 1);			
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 111 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 116){	
						
		}
		//Bar going right (1 second)
		if ( (float)(clock() - state->start) / CLOCKS_PER_SEC > 116 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 117 ){
			EnableStimuliGenL(moduleData, 1);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >=117 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 118){	
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 2);
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 118 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 119){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 3);	
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >=119 &&(float)(clock() - state->start) / CLOCKS_PER_SEC < 120){
			DisableStimuliGenL(moduleData);
			EnableStimuliGenL(moduleData, 4);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 120 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 121){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 5);			
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 121 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 122){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 6);			
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 122 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 127){
	
		}
		//Bar going left (1 second)
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 127 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 128){
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 6);	
		}	
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 128 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 129){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 5);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 129 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 130){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 4);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 130 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 131){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 3);				
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 132 && (float)(clock() - state->start) / CLOCKS_PER_SEC < 133){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 2);			
		}
		if( (float)(clock() - state->start) / CLOCKS_PER_SEC >= 133 && (float)(clock() - state->start) / CLOCKS_PER_SEC <134){	
			DisableStimuliGenL(moduleData);	
			EnableStimuliGenL(moduleData, 1);			
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

	// this will update parameters, from user input
	//state->displayNeuNumber = sshsNodeGetBool(moduleData->moduleNode, "displayNeuNumber");
	//state->setBiases = sshsNodeGetBool(moduleData->moduleNode, "setBiases");
	//state->doConnection = sshsNodeGetBool(moduleData->moduleNode, "doConnection");
	bool newStimulate = sshsNodeGetBool(moduleData->moduleNode, "stimulate");
	if (newStimulate && !state->stimulate) {
		state->stimulate = newStimulate;
		state->start = clock();
	}
	else if (!newStimulate && state->stimulate) {
		state->stimulate = newStimulate;
	}

	//state->stimulate = sshsNodeGetBool(moduleData->moduleNode, "stimulate");


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






