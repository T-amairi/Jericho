#pragma once
#include <omnetpp.h>
#include "Structures.h"

using namespace omnetpp;

//Abstract class for NFr and NFs 
class NF : public cSimpleModule
{
    public:
        //to fill myNeighbors
        void setNeighbors(std::string key, int value); 

        //called in handleMessage()
        virtual void caseUPDATE(cMessage* msg) = 0;
        virtual void caseRESPONSE(cMessage* msg) = 0;
        
    protected:
        std::map<std::string,int> myNeighbors; //keep a record of all the adjacent ID module
        std::map<std::string,TxState> myLedger; //keep a record of all the issued transactions
};
