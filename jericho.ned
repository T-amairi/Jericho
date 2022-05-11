//ConfiguratorModule : non-networked module setting up the network
simple ConfiguratorModule
{
    @display("i=block/cogwheel");
    @display("is=vs");
}

//NM : "mobile" node (i.e user issuing transaction)  
simple NM
{
    parameters:
        @display("i=,yellow");
        @display("is=vs");

        //******* PARAMETERS *******//
        double rateMean @unit(s) = 0.588s; //exponential distribution with the given mean
        int transactionLimit = 3; //how much transactions the node will issue
        
    gates:
        input in[];
        output out[];
}

//******************* NF : "fixed" node composing the network *******************//
//NFr : "responsible" node (i.e not a leaf node in the network)
simple NFr
{
    parameters:
        @display("i=,white");
        @display("is=vs");

    gates:
        input in[];
        output out[];
}

//NFs : "spreader" node (i.e a leaf node in the network)
simple NFs 
{
    parameters:
        @display("i=,white");
        @display("is=vs");

    gates:
        input in[];
        output out[];
}

//A generated network based on the python script "GenTopo.py" in the topology folder
network Network
{
    parameters:
        //******* CONSENSUS *******// 
        int f = 0; //number of Byzantine node in the cluster
        double waitTime @unit(s) = 1s; //how much time the proposer waits for each phases
        
		//******* DELAY *******// 
		bool ifRandDelay = false; //to set up a random delay
        double delay = 0.000588; 	//the delay in SECONDS !
        double minDelay = 50.0; //parameter used by the uniform distribution during initialization for setting the delay in SECONDS !
        double maxDelay = 100.0; //parameter used by the uniform distribution during initialization for setting the delay in SECONDS !

    submodules:
        Configurator: ConfiguratorModule; 
        NM : NM;
}