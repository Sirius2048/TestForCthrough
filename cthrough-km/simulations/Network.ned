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

package cthrough.simulations;	//packet:同文件夹下

import cthrough.Rack;	//import:不同文件夹下
import cthrough.SharedBufferCoreSwitch.SharedBufferCoreSwitch;
import cthrough.MEMS.MEMS_Controller;
import cthrough.MEMS.MEMS;
//
// Sample network, consisting of a source and a sink.
//
network Network
{
    parameters:

        int numOfRack;	//parameters参数含义详见omnetpp.ini文件
        int numOfCoreSwitch;
        int numOfMEMS;
        double oDataRate;	//add by xxr!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        volatile int EDataRate @unit(Gbps);	//volatile：一种关键字，旨在告诉编译器不要优化此变量，而是按代码所写的一步一步编译
        									//@unit：单位属性，用于声明该变量的单位
        volatile int ODataRate @unit(Gbps);	//首字母大写：模块类的名称  首字母小写：端口的名称、参数的名称
        @display("bgl=2");

//		@signal[endToEndDelay](type="simtime_t");	//@+参数名：属性。 可用于文件(files),模块(modules),参数(parameters),端口(gates),链路(connections)...
//		@signal[hopCount](type="long");
//		@signal[sourceAddress](type="long");
//		@statistic[endToEndDelay](title="end-to-end delay of arrived packets";unit=s;record=vector,mean,max;interpolationmode=none);
//		@statistic[hopCount](title="hop count of arrived packets";interpolationmode=none;record=vector?,mean,max);
//		@statistic[sourceAddress](title="source address of arrived packets";interpolationmode=none;record=vector?);

    types:
        channel EC extends ned.DatarateChannel
        {
            parameters:
                //                delay = default(0.1ms);
                datarate = default(EDataRate);
        }
        channel OC extends ned.DatarateChannel
        {
            parameters:
                //                delay = default(0.1ms);
                datarate = default(ODataRate);
        }
    submodules:
        coreswitch[numOfCoreSwitch]: SharedBufferCoreSwitch {
            @display("p=50,50,r,50");
            gates:	//即端口机架间的端口
                port[numOfRack];	//电交换机与机架间的端口
        }
        rack[numOfRack]: Rack {
            parameters:
                RackNo = index;	//机架编号。  index关键字：当前rack的编号(0~numOfRack-1)
                @display("p=50,150,r,50");
            gates:
                E_Switch[numOfCoreSwitch];	//机架与电交换机间的端口
                O_Switch[numOfMEMS];		//机架与光交换机间的端口
                MEMS_Controller;			//机架(ToR)与光控制器间的端口
        }
        openflow_controller: MEMS_Controller {
            parameters:		//add by xxr!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
                @display("p=50,250");
                oDataRate = default(ODataRate);
            gates:
                ToR[numOfRack];	//光控制器与ToR间的端口
                MEMS[numOfMEMS];//光控制器与MEMS光交换机间的端口
        }
        openflow_switch[numOfMEMS]: MEMS {
            @display("p=204,250,r,100");
            gates:
                MEMS_Controller;//光交换机与光控制器间的端口
                ToR[numOfRack];//光交换机与ToR间的端口
        }
    connections:
        for i=0..numOfRack-1 {
            rack[i].MEMS_Controller <--> openflow_controller.ToR++;	//有“++”：表示与该名称的所有节点相连，相当于一个for循环
        }
//		相当于：
//        for i=0..numOfRack-1, for j=0..numOfMEMS-1 {
//            rack[i].MEMS_Controller <--> openflow_controller.ToR[j];	
//        }
        for i=0..numOfMEMS-1 {
            openflow_controller.MEMS[i] <--> openflow_switch[i].MEMS_Controller;
        }
        for i=0..numOfRack-1, for j=0..numOfMEMS-1 {
            rack[i].O_Switch$o[j] --> OC --> openflow_switch[j].ToR$i[i];
            openflow_switch[j].ToR$o[i] --> rack[i].O_Switch$i[j];//////////////////////////////////
        }
        for i=0..numOfRack-1, for j=0..numOfCoreSwitch-1 {
            rack[i].E_Switch[j] <--> EC <--> coreswitch[j].port[i];
        }

}
