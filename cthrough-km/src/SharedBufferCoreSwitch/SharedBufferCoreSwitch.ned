//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


package cthrough.SharedBufferCoreSwitch;
//
// A "Node" consists of a Routing plus an App module which represents the
// aggregate traffic of the subnets connected to that router.
//
simple SharedBufferCoreSwitch
{
    parameters:
        int numOfRack;
        @display("i=device/switch;bgl=3");
    gates:
        inout port[numOfRack];
}


