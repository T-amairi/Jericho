#include "NFr.h"

void NFr::sendToFather(cMessage* msg)
{
    send(msg,"out",0);
}

void NFr::sendResponse(PackageRESPONSE_NFs* pack1, PackageRESPONSE_NFs* pack2)
{
    auto msg1 = msgResponse->dup();
    auto msg2 = msgResponse->dup();

    msg1->setContextPointer(pack1);
    msg2->setContextPointer(pack2);

    EV << "Sending results for " << pack1->Tx << " to " << pack1->path[0] << " and " << pack2->path[0] << "\n";
    send(msg1,"out",myNeighbors[pack1->path.back()]);
    send(msg2,"out",myNeighbors[pack2->path.back()]);
}

void NFr::spreadTx(cMessage* msg, PackageUPDATE* pack) 
{
    EV << "Spreading " << pack->Tx << " to my neighbour\n";
    auto senderModule = msg->getSenderModule();

    for(int i = 0; i < gateSize("out"); i++)
    {
        cGate *g = gate("out",i);

        if(!g->pathContains(senderModule))
        {
            auto copyPack = new PackageUPDATE(pack->Tx,pack->finalState);
            msgUpdate->setContextPointer(copyPack);
            send(msgUpdate->dup(),"out",i);
        }
    }
}

void NFr::broadcastTx(std::string Tx, TxState finalState)
{
    EV << "Broadcasting " << Tx << "\n";
    
    for(int i = 0; i < gateSize("out"); i++)
    {
        auto copyPack = new PackageUPDATE(Tx,finalState);
        msgUpdate->setContextPointer(copyPack);
        send(msgUpdate->dup(),"out",i);
    }
}

TxState NFr::isValid(TxState st1, TxState st2) const
{
    if(st1 == REJECTED || st2 == REJECTED)
    {
        return (TxState) 1;
    }

    return (TxState) 0;
}

PackageRESPONSE_NFs* NFr::getResponsePackage(const PackageREQUEST* pack, TxState finalState) const
{
    auto newPack = new PackageRESPONSE_NFs(pack->Tx,finalState);
    newPack->path = pack->path;
    return newPack;
}

void NFr::handleVerification(cMessage* msg)
{
    auto pack = (PackageREQUEST*) msg->getContextPointer();
    auto it = myBuffer.find(pack->Tx);

    if(it != myBuffer.end())
    {
        auto finalState = isValid(pack->state,(*it).second->state);
        auto packResp1 = getResponsePackage(pack,finalState);
        auto packResp2 = getResponsePackage((*it).second,finalState);

        sendResponse(packResp1,packResp2);
        broadcastTx(pack->Tx,finalState);

        myLedger[pack->Tx] = finalState;

        delete pack;
        delete (*it).second;
        myBuffer.erase(it);
    }

    else
    {
        EV << "Caching the request...\n"; 
        myBuffer[pack->Tx] = pack;
    }

    delete msg;
}

void NFr::caseREQUEST(cMessage* msg)
{
    auto pack = (PackageREQUEST*) msg->getContextPointer();

    if(pack->NFrChecker == getName())
    {
        EV << "Received " << pack->Tx << ", beginning verification process\n";
        handleVerification(msg);
    }

    else
    {
        EV << "Sending " << pack->Tx << " to " << pack->NFrChecker << "\n"; 
        pack->path.push_back(getName());
        sendToFather(msg);
    }
}

void NFr::caseUPDATE(cMessage* msg)
{
    auto pack =  (PackageUPDATE*) msg->getContextPointer();
    myLedger[pack->Tx] = pack->finalState;

    EV << "Updating my local ledger with " << pack->Tx << "\n";
    spreadTx(msg,pack);

    delete pack;
    delete msg;
}

void NFr::caseRESPONSE(cMessage* msg)
{
    auto pack = (PackageRESPONSE_NFs*) msg->getContextPointer();
    pack->path.pop_back();
    send(msg,"out",myNeighbors[pack->path.back()]);
}

void NFr::initialize()
{
    msgUpdate = new cMessage("Broadcasting a transaction",UPDATE);
    msgResponse = new cMessage("Responding to a NFs module",RESPONSE_NFs);
}

void NFr::handleMessage(cMessage * msg)
{
    switch(msg->getKind())
    {
        case MessageType::REQUEST:
            caseREQUEST(msg);
            break;

        case MessageType::RESPONSE_NFs:
            caseRESPONSE(msg);
            break;

        case MessageType::UPDATE:
            caseUPDATE(msg);
            break;
    
        default:
            break;
    }
}

void NFr::finish()
{
    delete msgUpdate;
    delete msgResponse;
}
