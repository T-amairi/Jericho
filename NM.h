#pragma once
#include <omnetpp.h>
#include "Structures.h"

using namespace omnetpp;

class NM : public cSimpleModule
{
    public:
        std::string createTx(); //generate a new transaction
        std::pair<int,int> getRandomIdx() const; //get two non-equal random numbers
        void sendToNFs(std::string Tx); //send a tx to two random NFs module 
        void setNeighbors(int key, std::string value); //to fill myNeighbors

        //called in handleMessage()
        void caseSELF();
        void caseRESPONSE(cMessage* msg);

        //overridden functions from cSimpleModule class
        void initialize() override;
        void handleMessage(cMessage * msg) override;
        void finish() override;

    private:
        simtime_t rateMean; //exponential distribution with the given mean (set in the NED file)
        int txCount = 0; //counts the number of transactions issued by the module
        int txLimit; //how many transactions the module can issue (set in NED file)
        int NFsCount; //how many NM modules the node is connected to
        
        cMessage* msgSelf; //a self msg to create a new transaction
        cMessage* msgIssue; //to issue a transaction

        std::map<std::string,TxState> myLedger; //keep a record of all the issued transactions
        std::map<int,std::string> myNeighbors; //keep a record of all the adjacent ID module
};

Define_Module(NM);
