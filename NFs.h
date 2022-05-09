#pragma once
#include "NF.h"
#include <algorithm>
#include <queue>

//NFs : "spreader" node (i.e a leaf node in the network)
class NFs : public NF
{
    public:
        void setLCA(std::map<std::string,std::string> mapLCA); //to set myLCA 
        void sendToFather(PackageREQUEST* pack); //send to the father of this node a request package
        
        void caseISSUE(cMessage* msg); //to get a new tx from NM 
        void caseRESPONSE(cMessage* msg) override; //to response to a NM module 
        void caseUPDATE(cMessage* msg) override; //to update the ledger
        void caseID(cMessage* msg); //to set ID 

        /* Tendermint consensus */
        void casePRE_PROPOSE(cMessage* msg); //prepropose phase
        void casePROPOSE(cMessage* msg); //propose phase
        void caseVOTE(cMessage* msg); //vote phase
        void caseWAIT(cMessage* msg); //delevery & computing phases
        void caseSYNC(cMessage* msg); //sync phase
        void caseNEXT(cMessage* msg); //to initiate a new round or epoch

        void getValue(); //get a tx to propose
        bool isValidTx(std::string Tx); //check if Tx is valid w.r.t the ledger
        void computePropose(); //set proposal value 
        void computeVote(); //set vote value
        void broadcastMsg(cMessage* msg, std::string value); //broadcast msg in the cluster during consensus 

        //overridden functions from cSimpleModule class
        void initialize() override;
        void handleMessage(cMessage * msg) override;
        void finish() override;

    private:
        cMessage* msgResponse; //to respond to a NM module
        cMessage* msgRequest; //to request a verification
        
        std::map<std::string,std::string> myLCA; //keep a record of all lowest common ancestor for each NFs
        
        /* Tendermint consensus */
        std::map<std::string,int> propCount; //to count the number of proposals
        std::map<std::string,int> voteCount; //to count the number of votes
        std::queue<PackageREQUEST*> queueTx; //to cache transaction
 
        int ID = -1; //the ID of the node in the consensus round
        int NodeNumber; //number of node in the cluster involved in the consensus
        int currentProposer = 0; //the ID of the current proposer 
        bool haveVoted = false; //check if the node has voted yet
        bool inConsensus = false; //check if the node is currently in consensus
        int waitType = 0; //the wait type for the proposer during the delevery & computing phases
        int f; //number of Byzantine node

        std::string decision = "null"; //this variable stocks the decision of the validator
        std::string proposal; //this variable stocks the value the validator will (pre-)propose
        std::string v = "null"; //local variable stocking the pre-preposal if delivered
        std::string vote = "null"; //the vote of the node
        
        //Messsages passed during the consensus
        cMessage* msgPrePropose; //to send the preproposal value
        cMessage* msgPropose; //to broadcast the proposal value
        cMessage* msgVote; //to broadcast the vote
        cMessage* msgWait; //to wait during delevery & computing phases
        cMessage* msgSync; //to sync at each epoch & round end
        cMessage* msgNext; //to start the next round or epoch
};

Define_Module(NFs);
