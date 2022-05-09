//includes
#pragma once
#include <omnetpp.h>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include "NFs.h"
#include "NM.h"

using namespace omnetpp;

class ConfiguratorModule : public cSimpleModule
{
    public:
        //Split a line (from the CSV file) in a list of int according to the delimeter ','
        std::vector<int> split(const std::string& line) const;
        //Fill myModules & myEdges by reading the CSV file from the topology folder
        void setModulesAndEgdes();
        //Fill myLCA by reading the CSV file from the topology folder
        void setLCA();
        //Fill myCoords
        void setCoords();
        
        //Connect two modules
        void connectModules(cModule* moduleOut, cModule* moduleIn);
        //Create a module (NFs or NFr)
        cModule* createModule(std::string type, std::string name) const;
        
        //Overriding the initialize() function from the cSimpleModule class
        void initialize() override;

    private:
        //a map containing all the modules (NFr and NFs) ([Node ID (key)] -> cModule object (value))
        std::map<int,cModule*> myModules;
        //a map containing the edge list for each node  ([Node ID (key)] -> Adjacent nodes (value))
        std::map<int,std::vector<int>> myEdges;
        //a map containing the lowest common ancestor for each leaf node
        std::map<int,std::vector<std::pair<std::string,std::string>>> myLCA; 
        //a map containing the coordinates of each module for the Qtenv simulation
        std::map<int,std::pair<double,double>> myCoords;

};

Define_Module(ConfiguratorModule);
