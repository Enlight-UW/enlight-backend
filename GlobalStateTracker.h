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

    char const* getSerializedState() const;
    int getSerializedStateSize() const;

    void setValveState(int state);
    void setRestrictState(int state);

private:

};

#endif	/* GLOBALSTATETRACKER_H */

