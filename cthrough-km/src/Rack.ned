//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 
package cthrough;

import cthrough.Server.Server;
//import cthrough.ToR.Router;
import cthrough.SharedBufferToR.SharedBufferToR;
//
// Sample network, consisting of a source and a sink.
//
module Rack
{
    parameters:
        int RackNo;			//机架编号
        int numOfNode;		//其他parameters参数含义详见omnetpp.ini文件
        int numOfCoreSwitch;
        int numOfMEMS;
        volatile int Rack_Rate @unit(Gbps);

        @display("i=device/router;bgl=2");
    gates:
        inout MEMS_Controller;
        inout E_Switch[];
        inout O_Switch[];
    types:
        channel C extends ned.DatarateChannel
        {
            parameters:
                //                delay = default(0.1ms);
                datarate = default(Rack_Rate);
        }
    submodules:
        Server[numOfNode]: Server {
            parameters:
                @display("p=116,134,m");
                address = index;	//服务器编号。  index关键字：当前Server的编号(0~numOfRack-1)
                rack_address = RackNo;
            gates:
                port;
        }
//        router: Router {
//            parameters:
//                RackNo = RackNo;
//                @display("p=53,70");
//            gates:
//                port[numOfNode];
//                e_port[numOfCoreSwitch];
//                o_port[numOfMEMS];
//                MEMS_Controller;
//        }
		router: SharedBufferToR{
		    parameters:
		        RackNo = RackNo;
		    gates:
		        port[numOfNode+numOfMEMS];
		        MEMS_Controller;
		}
    connections:
        //        Server[0].port++ <--> C <--> Server[1].port++;
        for i=0..numOfNode-1 {
            Server[i].port <--> C <--> router.port++;
        }
        for i=0..numOfMEMS-1 {
            O_Switch++ <--> router.port++;
        }
        for i=0..numOfCoreSwitch-1 {
            E_Switch++ <--> router.port++;
        }
      
        router.MEMS_Controller <--> MEMS_Controller;	//router.MEMS_Controller：ToR与机架服务器的端口
        												//MEMS_Controller:机架服务器与ToR的端口
}

//问题1：ToR是电交换机还是光交换机？
