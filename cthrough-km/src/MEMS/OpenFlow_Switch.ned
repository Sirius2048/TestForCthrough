//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

package cthrough.MEMS;

//
// Models a router.
//
simple MEMS
{
    parameters:
        @display("i=block/switch");
        @signal[drop](type="long");	//声明一个信号(signal)，取名为drop，类型为long
        @signal[outputIf](type="long");
        @statistic[drop](title="dropped packet byte length";unit=bytes;record=vector?,count,sum;interpolationmode=none);	//声明一个统计信息(statistic)，取名为drop
        @statistic[outputIf](title="output interface for each routed packet";record=histogram;interpolationmode=none);
    gates:
        inout ToR[];
        inout MEMS_Controller;
}

//@statistic属性：	//详见manual 4.15.2.2
//	source:定义record的输入。缺失时，以statistic的名称作为信号名称
//	record:包含记录了模式(mode)的列表，以逗号分隔。定义了如何记录源(source)
//		vector?:将相关信息记录到vector文件中(.vec)中，若省略则将相关信息记录到scalar文件(.sca)中
//		count:将输入值的计数记录到输出标量(scalar)中;功能上等同于last(count)
//		sum:将输入值的和记录到输出标量中,如果没有则记为0);功能等同于last(sum)		//详见manual 4.15.2.3
//	title:（用于作图时的可视化）统计信号的描述性名称
//	unit:（用于作图时的可视化）单位
//	interpolationmode:（用于作图时的可视化）在需要的地方插入信号值，none,sample-hold,back-sample-hold,linear
//	enum:
