//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


package cthrough.SharedBufferToR;
//
// A "Node" consists of a Routing plus an App module which represents the
// aggregate traffic of the subnets connected to that router.
//
simple SharedBufferToR
{
    parameters:
        int RackNo;
        int numOfNode;
        int numOfRack;
        int numOfMEMS;
        int numOfCoreSwitch;
        double T_report;
        @display("i=device/switch;bgl=3");
    gates:
        inout port[numOfNode+numOfMEMS];	//ToR对机架中的服务器（数量numOfNode）+ToR对光交换机（数量numOfMEMS）的端口
        inout MEMS_Controller;				//ToR对光控制器的端口
}

