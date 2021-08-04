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
// A "Node" consists of a Routing plus an App module which represents the
// aggregate traffic of the subnets connected to that router.
//
module Server	//module¸´ºÏÄ£¿é
{
    parameters:
        int address;
        int rack_address;
        @display("i=device/server");
    gates:
        inout port;
    submodules:
        app: App {
            parameters:
            	@display("p=52,116");
            	address = address;
        		rack_address = rack_address;
        }
        l2Queue: L2Queue {
            @display("p=94,196");
        }
    connections:
        app.out --> l2Queue.in;
        l2Queue.out --> app.in;
        l2Queue.line <--> port;
}


