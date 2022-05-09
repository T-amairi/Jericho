#include "NFs.h"

void NFs::setLCA(std::map<std::string,std::string> mapLCA)
{
    myLCA = mapLCA;
}

void NFs::sendToFather(PackageREQUEST* pack)
{
    msgRequest->setContextPointer(pack);
    send(msgRequest->dup(),"out",1);
}

void NFs::caseISSUE(cMessage* msg)
{
    if(ID == -1)
    {
        ID = 0;
        EV << "My ID in the cluster is " << ID << "\n";
        auto msgID = new cMessage("Setting IDs in the cluster",ID);
        
        for(int i = 2; i < gateSize("out"); i++)
        {
            auto id = new int(i - 1);
            msgID->setContextPointer(id);
            send(msgID->dup(),"out",i);
        }

        delete msgID;
    }

    auto pack = (PackageISSUE*) msg->getContextPointer();
    auto newPack = new PackageREQUEST(pack->Tx);

    newPack->NFrChecker = myLCA[pack->otherNFs];
    newPack->state = (TxState) 2; 
    newPack->path.push_back(getName());
    myLedger[newPack->Tx] = (TxState) 2;
    queueTx.push(newPack);

    if(ID == currentProposer && !inConsensus)
    {
        inConsensus = true;
        getDisplayString().setTagArg("t", 0, "Proposer");
        scheduleAt(simTime(), msgNext);
    }

    delete msg;
    delete pack; 
}

void NFs::caseRESPONSE(cMessage* msg)
{
    auto pack = (PackageRESPONSE_NFs*) msg->getContextPointer();
    auto newPack = new PackageRESPONSE_NM(pack->Tx,pack->finalState);

    myLedger[pack->Tx] = pack->finalState;

    EV << "Verification process completed for " << newPack->Tx << ", sending it back to NM module\n";
    msgResponse->setContextPointer(newPack);
    send(msgResponse->dup(),"out",0);

    delete msg;
    delete pack;
}

void NFs::caseUPDATE(cMessage* msg)
{
    auto pack =  (PackageUPDATE*) msg->getContextPointer();
    myLedger[pack->Tx] = pack->finalState;

    EV << "Updating my local ledger with " << pack->Tx << "\n";

    delete pack;
    delete msg;
}

void NFs::caseID(cMessage* msg)
{
    auto id = (int*) msg->getContextPointer();
    ID = *id;
    inConsensus = true;
    
    EV << "My ID in the cluster is " << ID << "\n";

    delete id;
    delete msg;
}

void NFs::casePRE_PROPOSE(cMessage* msg)   
{
    getDisplayString().setTagArg("i",1,"orange");

    //Get preproposal
    auto preProp = (std::string*) msg->getContextPointer();

    //Delivery phase
    v = *preProp;
    EV << "Received the preproposal, setting v to " << v << "\n";

    //Compute phase
    computePropose();

    //Broadcast the result (Send phase of Round Propose)
    if(proposal != "null")
    {
        EV << "Broadcasting my proposal result...\n";
        broadcastMsg(msgPropose,proposal);
    }
    
    delete preProp;
    delete msg;
}

void NFs::casePROPOSE(cMessage* msg)
{
    EV << "Collecting the proposals from other NFs...\n";

    //Collect proposal during round Propose
    auto prop = (std::string*) msg->getContextPointer();

    if(propCount.find(*prop) != propCount.end())
    {
        EV << "Incrementing the proposal of " << *prop << "\n";
        propCount[*prop]++;
    }

    else
    {
        EV << "Adding " << *prop << " to the map\n";
        propCount[*prop] = 1;
    }
    
    delete prop;
    delete msg;
}

void NFs::caseVOTE(cMessage* msg)
{
    //get vote
    auto recvVote = (std::string*) msg->getContextPointer();

    //vote
    if(!haveVoted)
    {
        haveVoted = true;
        getDisplayString().setTagArg("i",1,"green");
        computeVote();

        if(vote != "null")
        {
            EV << "Sending my vote !\n";
            broadcastMsg(msgVote,vote);
        }        
    }
    
    EV << "Collecting the votes from other NFs...\n";

    //Delivery phase
    if(voteCount.find(*recvVote) != voteCount.end())
    {
        EV << "Incrementing the vote of " << *recvVote << "\n";
        voteCount[*recvVote]++;
    }

    else
    {
        EV << "Adding " << *recvVote << " to the map\n";
        voteCount[*recvVote] = 1;
    }
    
    delete recvVote;
    delete msg;
}

void NFs::caseSYNC(cMessage* msg)
{
    getDisplayString().setTagArg("i",1,"black");
    auto tag = (std::string*) msg->getContextPointer();

    haveVoted = false;
    inConsensus = false;
    decision = "null";
    v = "null";
    vote = "null";
    propCount.clear();
    voteCount.clear();
    currentProposer = currentProposer >= NodeNumber ? 0 : currentProposer + 1;

    EV << "Changing to the proposer " << currentProposer << "\n";

    if(currentProposer == ID)
    {
        if(!queueTx.empty())
        {
            getDisplayString().setTagArg("t", 0, "Proposer");
            scheduleAt(simTime(), msgNext);
        }

        else
        {
            EV << "My queue is empty...\n";
            broadcastMsg(msgSync,std::string("Empty"));
        }
    }

    inConsensus = true;
    getDisplayString().setTagArg("i",1,"red");

    delete tag;
    delete msg;
}

void NFs::caseWAIT(cMessage* msg)
{
    if(waitType == 0)
    {
        EV << "Proposal phase finished... Next phase : voting\n";

        getDisplayString().setTagArg("i",1,"green");

        computeVote();
        
        broadcastMsg(msgVote,vote); //Broadcast the vote 
        haveVoted = true;
        waitType = 1;
        scheduleAt(simTime() + getParentModule()->par("waitTime"), msgWait);
        return;
    }

    if(waitType == 1)
    {
        EV << "Voting phase finished... Next phase : syncing\n";

        getDisplayString().setTagArg("i",1,"black");
        
        using pair_type = decltype(voteCount)::value_type;
    
        auto maxVote = std::max_element
        (
            std::begin(voteCount), std::end(voteCount),[](const pair_type & p1, const pair_type & p2){return p1.second < p2.second;}
        );

        if(!voteCount.empty() && maxVote->second >= 2*f + 1 && isValidTx(maxVote->first))
        {
            EV << "Selected " << maxVote->first << " with " << maxVote->second << " votes\n";
            decision = maxVote->first;
        }

        //Send result
        auto newPack = queueTx.front();
        queueTx.pop();
        newPack->state = (decision != newPack->Tx) ? (TxState) 1 : (TxState) 0;

        EV << "Sending " << newPack->Tx << " to " << newPack->NFrChecker << " for verification\n";
        sendToFather(newPack);
      
        //Sync phase for the proposer
        haveVoted = false;
        inConsensus = false;
        decision = "null";
        v = "null";
        vote = "null";
        propCount.clear();
        voteCount.clear();
        currentProposer = currentProposer >= NodeNumber ? 0 : currentProposer + 1;

        EV << "Changing to the proposer " << currentProposer << "\n";

        getDisplayString().setTagArg("t", 0, "Validator");
        getDisplayString().setTagArg("i",1,"red");

        broadcastMsg(msgSync,std::string("Not empty"));//Broadcast the sync

        return;
    }
}

void NFs::caseNEXT(cMessage* msg)
{
    getDisplayString().setTagArg("i",1,"orange");

    waitType = 0;
    getValue();

    EV << "Launching consensus with " << getName() << "\n";
    EV << "Setting my proposal : " << proposal << "\n"; 
    EV << "Sending my proposal...\n";

    broadcastMsg(msgPrePropose,proposal);
    scheduleAt(simTime() + getParentModule()->par("waitTime"), msgWait); 
}

void NFs::getValue()
{
    const auto pack = queueTx.front();
    proposal = pack->Tx;
    propCount[proposal] = 1;
}

bool NFs::isValidTx(std::string Tx)
{
    if(Tx == "null") return false;

    //check Tx wrt to the ledger here
    return true;
}

void NFs::computePropose()
{
    EV << "Checking the preproposal block...\n";

    if(!isValidTx(v))
    {
        proposal = "null";
    }

    else
    {
        proposal = v;
        propCount[proposal] = 2;
    }

    EV << "Proposal equals : " << proposal << "\n"; 
}

void NFs::computeVote()
{
    using pair_type = decltype(propCount)::value_type;
    
    auto maxProp = std::max_element
    (
        std::begin(propCount), std::end(propCount),[](const pair_type & p1, const pair_type & p2){return p1.second < p2.second;}
    );

    EV << "Computing my vote...\n";

    if(maxProp->second >= 2*f + 1 && isValidTx(maxProp->first))
    {
        EV << "Selected " << maxProp->first << " with " << maxProp->second << " proposals\n";
        vote = maxProp->first;
        voteCount[vote] = 1;
    }

    else
    {
        EV << "My vote has been set to null\n";
        vote = "null";
    }
}

void NFs::broadcastMsg(cMessage* msg, std::string value)
{
    for(int i = 2; i < gateSize("out"); i++)
    {
        if(!value.empty())
        {
            auto tmp = new std::string(value);
            msg->setContextPointer(tmp);    
        }
        
        send(msg->dup(),"out",i);
    }
}

void NFs::initialize()
{
    f = getParentModule()->par("f");
    NodeNumber = gateSize("out") - 2;

    msgPrePropose = new cMessage("Preproposing a new block",PRE_PROPOSE);
    msgPropose = new cMessage("Sending my proposal",PROPOSE);
    msgVote = new cMessage("Sending my vote",VOTE);
    msgWait = new cMessage("Waiting the current phase to finish...",WAIT);
    msgSync = new cMessage("Synchronizing for the next round...",SYNC);
    msgNext = new cMessage("Beginning a new round...",NEXT);

    msgRequest = new cMessage("Requesting a transaction verification",REQUEST);
    msgResponse = new cMessage("Responding to a NM module",RESPONSE_NM);

    getDisplayString().setTagArg("i",1,"red");
    getDisplayString().setTagArg("t", 0, "Validator");
}

void NFs::handleMessage(cMessage * msg)
{
    switch(msg->getKind())
    {
        case MessageType::ISSUE:
            caseISSUE(msg);
            break;

        case MessageType::RESPONSE_NFs:
            caseRESPONSE(msg);
            break;

        case MessageType::UPDATE:
            caseUPDATE(msg);
            break;

        case MessageType::ID:
            caseID(msg);
            break;

        case MessageType::PRE_PROPOSE:
            casePRE_PROPOSE(msg);
            break;

        case MessageType::PROPOSE:
            casePROPOSE(msg);
            break;

        case MessageType::VOTE:
            caseVOTE(msg);
            break;

        case MessageType::WAIT:
            caseWAIT(msg);
            break;

        case MessageType::SYNC:
            caseSYNC(msg);
            break;

        case MessageType::NEXT:
            caseNEXT(msg);
            break;
    
        default:
            break;
    }
}

void NFs::finish()
{
    delete msgRequest;
    delete msgResponse;

    delete msgPrePropose;
    delete msgPropose;
    delete msgVote;
    delete msgWait;
    delete msgSync;
    delete msgNext;
}
