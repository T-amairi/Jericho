#pragma once
#include "NF.h"

//NFr : "responsible" node (i.e not a leaf node in the network)
class NFr : public NF
{
    public:
        void sendToFather(cMessage* msg); //send to the father of this node a request package
        void sendResponse(PackageRESPONSE_NFs* pack1, PackageRESPONSE_NFs* pack2); //send a response to NFs modules
        void spreadTx(cMessage* msg, PackageUPDATE* pack); //broadcast to the neighbors modules the received transaction
        void broadcastTx(std::string Tx, TxState finalState); //broadcast to the neighbors modules the issued transaction
        TxState isValid(TxState st1, TxState st2) const; //return the final state of the transaction
        PackageRESPONSE_NFs* getResponsePackage(const PackageREQUEST* pack, TxState finalState) const; //get a response package based on the request
        void handleVerification(cMessage* msg); //handle the verification process
        
        //called in handleMessage()
        void caseREQUEST(cMessage* msg);
        void caseUPDATE(cMessage* msg) override;
        void caseRESPONSE(cMessage* msg) override;

        //overridden functions from cSimpleModule class
        void initialize() override;
        void handleMessage(cMessage * msg) override;
        void finish() override;

    private:
        cMessage* msgUpdate; //to send to others modules to update their local ledger
        cMessage* msgResponse; //to respond to a request verification
        std::map<std::string,PackageREQUEST*> myBuffer; //a buffer for keeping verification request
};

Define_Module(NFr);
