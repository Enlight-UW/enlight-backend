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
int valveState = 0; // Aligned to the right (MSB bits 0): V1, V2 ... HC, HR
int restrictState = 0; //Alignment the same as valveState
int stateStringSize = 0; //Internal tracking of state string size

GlobalStateTracker::GlobalStateTracker() {
}

GlobalStateTracker::GlobalStateTracker(const GlobalStateTracker& orig) {
}

GlobalStateTracker::~GlobalStateTracker() {
}

char const* GlobalStateTracker::getSerializedState() const {
    std::stringstream streambuilder;
    string builder;

    streambuilder << "<valveState>" << valveState << "</><restrictState>" << restrictState << "</>";

    builder = streambuilder.str();
    stateStringSize = builder.size();

    return builder.c_str();
}

int GlobalStateTracker::getSerializedStateSize() const {
    //TODO: Make sure stateStringSize doesn't get too big, otherwise we'll run
    //over our network buffer and not transmit some of the data.
    return stateStringSize;
}

void GlobalStateTracker::setValveState(int state) {
    valveState = state;
}

void GlobalStateTracker::setRestrictState(int state) {
    restrictState = state;
}

/** we need some kind of function like "moveToState" that takes care of issues
 like turning too many values on/off at the same time and all valve update requests
 need to go through this function so we don't break anything. */
