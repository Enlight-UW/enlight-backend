/* 
 * File:   GlobalStateTracker.h
 * Author: Alex Kersten
 *
 * Created on December 8, 2012, 1:20 AM
 * 
 * Header file for GlobalStateTracker.
 */

#ifndef GLOBALSTATETRACKER_H
#define	GLOBALSTATETRACKER_H

class GlobalStateTracker {
public:
    GlobalStateTracker();
    GlobalStateTracker(const GlobalStateTracker& orig);
    virtual ~GlobalStateTracker();

    void generateSerializedState();
    char const* getSerializedStateValue() const;
    int getSerializedStateSize() const;
    
    int getValveState() const;
    int getNextValveState() const;

    void setNextValveState(int state);
    void setRestrictState(int state);
    
    bool ease();

private:

};

#endif	/* GLOBALSTATETRACKER_H */

