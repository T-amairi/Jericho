#include "ConfiguratorModule.h"

std::vector<int> ConfiguratorModule::split(const std::string& line) const
{
    std::vector<int> tokens;
    std::string token;
    std::istringstream tokenStream(line);
    bool isNMCoord = false;

    while(std::getline(tokenStream, token, ','))
    {
        if(token != "NM")
        {
            tokens.push_back(stoi(token));
        }

        else
        {
            isNMCoord = true;
        }
    }

    if(isNMCoord)
    {
        auto moduleNM = getParentModule()->getSubmodule("NM");
        auto nm = check_and_cast<NM*>(moduleNM);
        cDisplayString& dispStr = nm->getDisplayString();
        dispStr.setTagArg("p", 0, tokens[0]);
        dispStr.setTagArg("p", 1, tokens[1]);
        tokens.clear();
    }

    return tokens;
}

void ConfiguratorModule::setModulesAndEgdes() 
{
    std::string path = "./topology/CSV/Tree.csv";
    std::fstream file;
    file.open(path,std::ios::in);

    if(!file.is_open()) throw std::runtime_error("Could not open CSV file");

    std::string line;
    
    while(getline(file,line))
    {
        auto edgeList = split(line);
        auto id = edgeList[0];
        
        id < 0 ? myModules[id] = createModule("NFs","NFs[" + std::to_string(abs(id)) + "]") : myModules[id] = createModule("NFr","NFr[" + std::to_string(id) + "]");
        auto nf = check_and_cast<NF*>(myModules[id]);

        if(id != 0)
        {
            if(id < 0)
            {
                auto nfs = static_cast<NFs*>(nf);
                nfs->setLCA(std::map<std::string,std::string>(myLCA[id].begin(),myLCA[id].end()));
            }
        }
        
        myEdges[id] = std::vector<int>(edgeList.begin() + 1, edgeList.begin() + edgeList.size());
    }

    file.close();
}

void ConfiguratorModule::setLCA() 
{
    std::string path = "./topology/CSV/LCA.csv";
    std::fstream file;
    file.open(path,std::ios::in);

    if(!file.is_open()) throw std::runtime_error("Could not open CSV file");

    std::string line;
    
    while(getline(file,line))
    {
        auto LCA = split(line);
        if(LCA.empty()) continue;
        auto id = LCA[0];
        auto it = myLCA.find(id);

        if(it == myLCA.end())
        {
            myLCA[id] = std::vector<std::pair<std::string,std::string>>({std::make_pair("NFs[" + std::to_string(abs(LCA[1])) + "]","NFr[" + std::to_string(LCA[2]) + "]")});
        }

        else
        {
            myLCA[id].push_back(std::make_pair("NFs[" + std::to_string(abs(LCA[1])) + "]","NFr[" + std::to_string(LCA[2]) + "]"));
        }        
    }

    file.close();
}

void ConfiguratorModule::setCoords() 
{
    std::string path = "./topology/CSV/Coords.csv";
    std::fstream file;
    file.open(path,std::ios::in);

    if(!file.is_open()) throw std::runtime_error("Could not open CSV file");

    std::string line;
    
    while(getline(file,line))
    {
        auto coords = split(line);

        if(!coords.empty())
        {
            auto id = coords[0];
            myCoords[id] = std::make_pair(coords[1],coords[2]);
        }
    }

    file.close();
}

void ConfiguratorModule::connectModules(cModule* moduleOut, cModule* moduleIn)
{
    double delay = 0.0;

    if(getParentModule()->par("ifRandDelay"))
    {
       double minDelay = getParentModule()->par("minDelay");
       double maxDelay = getParentModule()->par("maxDelay");
       delay = uniform(minDelay,maxDelay);
    }

    else
    {
       delay = getParentModule()->par("delay");
    }

    auto channel = cDelayChannel::create("Channel");
    channel->setDelay(delay);

    moduleOut->setGateSize("out",moduleOut->gateSize("out") + 1);
    moduleIn->setGateSize("in",moduleIn->gateSize("in") + 1);

    auto gOut = moduleOut->gate("out",moduleOut->gateSize("out") - 1);
    auto gIn = moduleIn->gate("in",moduleIn->gateSize("in") - 1);

    gOut->connectTo(gIn,channel);
    gOut->getChannel()->callInitialize();
}

cModule* ConfiguratorModule::createModule(std::string type, std::string name) const
{
    auto moduleType = cModuleType::get(type.c_str());
    auto newModule = moduleType->create(name.c_str(), getParentModule());
    newModule->finalizeParameters();
    newModule->buildInside();

    return newModule;
}

void ConfiguratorModule::initialize()
{
    EV << "Setting up connections:\n";

    auto moduleNM = getParentModule()->getSubmodule("NM");
    auto nm = check_and_cast<NM*>(moduleNM);
    setLCA();
    setCoords();
    setModulesAndEgdes();
    
    for(auto const& edge : myEdges)
    {
        auto moduleOut = myModules[edge.first];
        auto nf = check_and_cast<NF*>(moduleOut);

        cDisplayString& dispStr = moduleOut->getDisplayString();
        dispStr.setTagArg("p", 0, myCoords[edge.first].first);
        dispStr.setTagArg("p", 1, myCoords[edge.first].second);
       
        if(edge.first < 0)
        {
            connectModules(moduleNM,moduleOut);
            connectModules(moduleOut,moduleNM);

            nm->setNeighbors(moduleNM->gateSize("out") - 1,moduleOut->getName());
            nf->setNeighbors(moduleNM->getName(),moduleOut->gateSize("out") - 1);
            
            EV << "Module " << moduleNM->getName() << " ---> Module " << moduleOut->getName() << "\n";
            EV << "Module " << moduleOut->getName() << " ---> Module " << moduleNM->getName() << "\n";
        }
    
        for(int idx : edge.second)
        {
            auto moduleIn = myModules[idx];
            connectModules(moduleOut,moduleIn);
            nf->setNeighbors(moduleIn->getName(),moduleOut->gateSize("out") - 1);
            
            EV << "Module " << moduleOut->getName() << " ---> Module " << moduleIn->getName() << "\n";
        }
    }

    myModules.clear();
    myEdges.clear();
    myLCA.clear();
    myCoords.clear();
}
