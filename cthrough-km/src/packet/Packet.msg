//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


//
// Represents a packet in the network.
//
packet Packet
{
    int srcAddr;	//源服务器在机架中的编号
    int srcRack;	//源服务器所在机架的编号（机架号）
    int destAddr;	//目的服务器在机架中的编号
    int destRack;	//目的服务器所在机架的编号（机架号）
    int hopCount;	//已转发的跳数
    int vlan;
    int flowId;		//流的id
    int pkId;		//数据包在流中的id(编号)
}

