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
package cthrough.MEMS;
//
// Sample network, consisting of a source and a sink.
//
simple MEMS_Controller
{
    parameters:
        double T_day;
        double T_night;
        double T_report;
        int numOfMEMS;
        int algorithm_ID;
        double deadlineWeight;
        
        //************************* add by xxr ************************//
        double oDataRate;	//add by xxr!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //*************************************************************//
    gates:
        inout ToR[];
        inout MEMS[];
}
