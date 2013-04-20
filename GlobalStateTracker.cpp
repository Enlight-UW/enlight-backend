/* 
 * File:   GlobalStateTracker.cpp
 * Author: Alex Kersten
 * 
 * Created on December 8, 2012, 1:20 AM
 * 
 * The server will have a GlobalStateTracker object which keeps track of the
 * status of everything that anyone might want to know about the fountain.
 */
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "GlobalStateTracker.h"

using namespace std;

//Here is the master list of all the state variables the native server keeps
//track of. Some of these may be loaded from disk upon creation of this object,
//but these should all be accounted for in your serialize method!
string controlState = "Unassigned";

int nextValveState = 0; //Whatever we *want* the next state to be. This will be
//updated asynchronously via API requests and whatnot,
//but we'll ease into it before actually setting the
//state.

int valveState = 0; // Aligned to the right (MSB bits 0): V1, V2 ... HC, HR
int restrictState = 0; //Alignment the same as valveState

GlobalStateTracker::GlobalStateTracker() {
}

GlobalStateTracker::GlobalStateTracker(const GlobalStateTracker& orig) {
}

GlobalStateTracker::~GlobalStateTracker() {
}

char const* stateString;
int stateStringSize = 0; //Internal tracking of state string size

/**
 * Generates the serialized current state. Call first before getting the value
 * or size. Internally sets the cstring and size variable to keep track of it.
 */
void GlobalStateTracker::generateSerializedState() {
    std::stringstream streambuilder;
    string builder;

    streambuilder << "<valveState>" << valveState << "</><restrictState>" << restrictState << "</>";

    builder = streambuilder.str();
    stateStringSize = builder.size();


    stateString = builder.c_str();
}

/**
 * Gets the previously generated state of the fountain.
 * 
 * @return A cstring to the serialized state of the fountain.
 */
char const* GlobalStateTracker::getSerializedStateValue() const {
    return stateString;
}

/**
 * Gets the size of the last serialized state string.
 * 
 * @return The character length of the previous state serialization. 
 */
int GlobalStateTracker::getSerializedStateSize() const {
    //TODO: Make sure stateStringSize doesn't get too big, otherwise we'll run
    //over our network buffer and not transmit some of the data.
    return stateStringSize;
}

void GlobalStateTracker::setNextValveState(int state) {
    nextValveState = state;
}

void GlobalStateTracker::setRestrictState(int state) {
    restrictState = state;
}

/**
 * Used for things like bitmasking the existing state to toggle the valves.
 * @return The current state represented as an integer.
 */
int GlobalStateTracker::getValveState() const {
    return valveState;
}

int GlobalStateTracker::getNextValveState() const {
    return nextValveState;
}

/**
 * Called by the main process loop to ease us in to the next state. Will update
 * a single valve from nextValveState into valveState and then stop updating to
 * prevent too many from changing state at once. The timing of this method is
 * controlled by a constant in main, determining the length of time needed
 * between single valves changing state. Therefore, this method will move
 * exactly one valve to its new value, but be called multiple times in the
 * course of a second (or even in the course of cRIO update clocks), so the
 * actual display won't look like it's just doing one at once.
 *  
 * @return true if we actually updated the current state, false otherwise. */
bool GlobalStateTracker::ease() {
    //Set chkIdx to however many total valves there are, minus one.
    int chkIdx = 23;


    while (chkIdx >= 0) {
        int chkMask = 1 << chkIdx;
        
        //Compare next state and current state from the MSB and moving right
        if (valveState & chkMask != nextValveState & chkMask) {
            //These are different - update this valve and return.
            //Blank the current one and add whatever the movement would be.
            valveState &= ~chkMask;
            valveState |= (nextValveState & chkMask);
            
            return true;
        }
        
        chkIdx--;
    }
    
    
    return false;
}