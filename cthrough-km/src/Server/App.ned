//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

package cthrough.Server;

//
// Application model to generate traffic for the network.
//
simple App	//simple简单模块
{
    parameters:
        int address;  // local node address
        int rack_address;
        int numOfNode;
        int numOfRack;
        double Server_Rate;
        double offerload;
        volatile double sendIaTime @unit(s) = default(exponential(1s*Server_Rate*(1/offerload))); // time between generating packets
        volatile int packetLength @unit(byte);  // length of one message (fixed! no "volatile" modifier)
        
        @display("i=block/browser");
        @signal[endToEndDelay](type="simtime_t");
        @signal[hopCount](type="long");
        @signal[sourceAddress](type="long");
        //@statistic：用于存储统计信息
        @statistic[endToEndDelay](title="end-to-end delay of arrived packets";unit=s;record=vector,mean,max;interpolationmode=none);
        @statistic[hopCount](title="hop count of arrived packets";interpolationmode=none;record=vector?,mean,max);
        @statistic[sourceAddress](title="source address of arrived packets";interpolationmode=none;record=vector?);
    gates:
        input in;
        output out;
}


