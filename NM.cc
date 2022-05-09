#include "NM.h"

std::string NM::createTx()
{
    auto newTx = getName() + std::to_string(txCount);
    myLedger[newTx] = (TxState) 2;
    txCount++;
    return newTx;
}

std::pair<int,int> NM::getRandomIdx() const
{
    auto idx1 = uniform(0,NFsCount - 1);
    int idx2 = uniform(0,NFsCount - 1);

    while(idx1 == idx2)
    {
        idx2 = uniform(0,NFsCount - 1);
    }

    return std::make_pair(idx1,idx2);
}

void NM::sendToNFs(std::string Tx)
{
    auto idx = getRandomIdx();

    EV << "Sending " << Tx << " to: " << myNeighbors[idx.second] << " & " << myNeighbors[idx.first] << "\n";

    auto pack1 = new PackageISSUE(Tx,myNeighbors[idx.second]);
    auto pack2 = new PackageISSUE(Tx,myNeighbors[idx.first]);
 
    msgIssue->setContextPointer(pack1);
    send(msgIssue->dup(),"out",idx.first);

    msgIssue->setContextPointer(pack2);
    send(msgIssue->dup(),"out",idx.second);
} 

void NM::setNeighbors(int key, std::string value)
{
    myNeighbors[key] = value;
}

void NM::caseSELF()
{
    if(txCount < txLimit)
    {
        auto newTx = createTx();
        sendToNFs(newTx);

        scheduleAt(simTime() + exponential(rateMean), msgSelf);
    }

    else
    {
        EV << "Number of transactions reached: stopping issuing\n";
    }
}

void NM::caseRESPONSE(cMessage* msg)
{
    auto pack =  (PackageRESPONSE_NM*) msg->getContextPointer();
    auto it = myLedger.find(pack->Tx);

    if((*it).second == PENDING)
    {
        (*it).second = pack->finalState;
        EV << "Received the final state for " << pack->Tx << ": " << pack->finalState << "\n";
    }

    else
    {
        EV << "Already received the final state for " << pack->Tx << "\n";
    }

    delete pack;
    delete msg;
}

void NM::initialize()
{
    txLimit = par("transactionLimit");
    rateMean = par("rateMean");
    NFsCount = gateSize("out");

    msgSelf = new cMessage("Creating a new transaction",SELF);
    msgIssue = new cMessage("Issuing a new transaction",ISSUE);

    scheduleAt(simTime() + exponential(rateMean), msgSelf);
}

void NM::handleMessage(cMessage* msg)
{
    switch(msg->getKind())
    {
        case MessageType::SELF:
            caseSELF();
            break;

        case MessageType::RESPONSE_NM:
            caseRESPONSE(msg);
            break;
    
        default:
            break;
    }  
}

void NM::finish()
{
    delete msgSelf;
    delete msgIssue;

    for(auto tx : myLedger)
    {
        std::string state;

        if(tx.second == 0)
        {
            state = "validated";
        }

        else if(tx.second == 1)
        {
            state = "rejected";
        }

        else
        {
            state = "pending";
        }

        EV << "Tx : " << tx.first << " state : " << state << "\n";
    }
}
