
#include<iostream>
#include<cstring>
#include<cstdlib>
#include<cstdio>
#include<climits>
#include<algorithm>
#include<iomanip>
#include<vector>

using namespace std;

class Hungarian
{
    public:

        Hungarian(vector< vector<int> > buf_len);
        virtual int maxMatch();
        vector<int> calculate();

    private:

        vector< vector<int> > line;//记录连接send和receive的边，如果有边为1，没有则为0
        vector< vector<int> > weight;//记录队列长度

        vector<int> mark;//标记节点（标记Yi中的节点是否被被匹配，是1否0）
        vector<int> cx;//最终结果中与Xi匹配的Y点
        vector<int> cy;//最终结果中与Yi匹配的X点

        int nx,ny;//X和Y集合中定点个数
        int N; // node
        int threshold;//阈值

    protected:
        virtual void init();
        virtual int find( int u);
};

Hungarian::Hungarian(vector< vector<int> > buf_len) //队列长队是一个二维数组保存目的节点编号和对应的权值
{
    weight = buf_len;//权重等于队列长度
    N = buf_len.size();//
    nx = N;
    ny = N;
}

void Hungarian::init()//初始化
{
    cx.assign(N, -1);
    cy.assign(N, -1);//Yi未与任何Xi匹配
    line.resize(N);

    //先将line二维数组内的值全部置0
    for(int i = 0; i < nx; i++)
        line[i].assign(N,0);

    int threshold = 8;//设置阈值

    //对于>8kb的缓存队列，在对应的源-目的节点间建立光通路（找出weight中>=threshold的项，将line中对应项置0）
    //注：weight和line尺寸大小相同，weight记录已缓存的队列长度，line记录send和receive是否有边相连(有1无0)
    for(int i = 0; i < nx; i++)
        for(int j = 0; j < ny; j++)
            if(weight[i][j] >= threshold)
                line[i][j] = 1;
}

int Hungarian::find(int u)                  //寻找源节点为Xu的增广路(有1无0)
//增广路：假如有这么一条路，这条路从源点开始一直一段一段的连到了汇点，并且，这条路上的每一段都满足流量<容量，
//注意，是严格的<,而不是<=。那么，我们一定能找到这条路上的每一段的(容量-流量)的值当中的最小值delta。我们把这
//条路上每一段的流量都加上这个delta，一定可以保证这个流依然是可行流。这样我们就得到了一个更大的流，他的流量是
//之前的流量+delta，而这条路就叫做增广路
{
    for(int v = 0; v < ny; v++)    //针对Xi的某个顶点u，考虑所有Yi顶点v
    {
        if(line[u][v] && !mark[v] ) //若u与v存在连接 且 v没有被标记
        {
            mark[v] = 1;            //标记v
            if(cy[v] == -1 || find(cy[v])) //如果v没有被匹配，或v已经匹配了但从v出发可以找到增广路
            {
                cx[u] = v;//将v匹配给u
                cy[v] = u;//将u匹配给v
                return 1; //找到可增广路
             // break;
            }
        }
    }
    return 0;
}

int Hungarian::maxMatch()          //得到与Yi匹配的X点
{
    int res = 0;
    cx.assign(N, -1);
    cy.assign(N, -1);
    for(int i=0; i<nx; i++)
       {
           if(cx[i]==-1) //从每个未盖点出发进行寻找增广路
           {
               mark.assign(N, 0);
               res+=find(i); //每找到一条增广路，可使得匹配数加1
           }
           else
           {
               mark.assign(N, 0);
               res+=find(i);
           }
       }
       return res;
}

vector<int> Hungarian::calculate()
{

    init();
//    int max;

    maxMatch();
//    cout<<max<<endl;

    return cy;
}





