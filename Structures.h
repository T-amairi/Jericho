#pragma once
#include <string>
#include <vector>

enum MessageType{ID,SELF,ISSUE,RESPONSE_NFs,RESPONSE_NM,REQUEST,UPDATE,PRE_PROPOSE,PROPOSE,VOTE,WAIT,SYNC,NEXT};
enum TxState{VALIDATED,REJECTED,PENDING};

//data sent by NM modules to NFs modules 
struct PackageISSUE
{   
    PackageISSUE(std::string _Tx, std::string nfs):Tx(_Tx),otherNFs(nfs){}; //constructor
    const std::string Tx; //the transaction
    const std::string otherNFs; //the other NFs module receiving the request 
};

//data sent by NF modules to update the ledger
struct PackageUPDATE
{   
    PackageUPDATE(std::string _Tx,TxState fState):Tx(_Tx),finalState(fState){}; //constructor
    const std::string Tx; //the transaction
    const TxState finalState; //its final state 
};

//data sent by NFs modules to NFr modules 
struct PackageREQUEST
{   
    PackageREQUEST(std::string _Tx):Tx(_Tx){}; //constructor
    const std::string Tx; //the transaction
    TxState state; //its current state seen by the NFs sender
    std::string NFrChecker; //the LCA node checking both request 
    std::vector<std::string> path; //the path taken by the request 
};

//data sent by NFr modules to NFs modules 
struct PackageRESPONSE_NFs
{   
    PackageRESPONSE_NFs(std::string _Tx,TxState fState):Tx(_Tx),finalState(fState){}; //constructor
    const std::string Tx; //the transaction
    const TxState finalState; //its final state
    std::vector<std::string> path; //the path taken by the request 
};

//data sent by NFs modules to NM modules 
struct PackageRESPONSE_NM
{   
    PackageRESPONSE_NM(std::string _Tx,TxState fState):Tx(_Tx),finalState(fState){}; //constructor
    const std::string Tx; //the transaction
    const TxState finalState; //its final state
};