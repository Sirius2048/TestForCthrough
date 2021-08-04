
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

        vector< vector<int> > line;//��¼����send��receive�ıߣ�����б�Ϊ1��û����Ϊ0
        vector< vector<int> > weight;//��¼���г���

        vector<int> mark;//��ǽڵ㣨���Yi�еĽڵ��Ƿ񱻱�ƥ�䣬��1��0��
        vector<int> cx;//���ս������Xiƥ���Y��
        vector<int> cy;//���ս������Yiƥ���X��

        int nx,ny;//X��Y�����ж������
        int N; // node
        int threshold;//��ֵ

    protected:
        virtual void init();
        virtual int find( int u);
};

Hungarian::Hungarian(vector< vector<int> > buf_len) //���г�����һ����ά���鱣��Ŀ�Ľڵ��źͶ�Ӧ��Ȩֵ
{
    weight = buf_len;//Ȩ�ص��ڶ��г���
    N = buf_len.size();//
    nx = N;
    ny = N;
}

void Hungarian::init()//��ʼ��
{
    cx.assign(N, -1);
    cy.assign(N, -1);//Yiδ���κ�Xiƥ��
    line.resize(N);

    //�Ƚ�line��ά�����ڵ�ֵȫ����0
    for(int i = 0; i < nx; i++)
        line[i].assign(N,0);

    int threshold = 8;//������ֵ

    //����>8kb�Ļ�����У��ڶ�Ӧ��Դ-Ŀ�Ľڵ�佨����ͨ·���ҳ�weight��>=threshold�����line�ж�Ӧ����0��
    //ע��weight��line�ߴ��С��ͬ��weight��¼�ѻ���Ķ��г��ȣ�line��¼send��receive�Ƿ��б�����(��1��0)
    for(int i = 0; i < nx; i++)
        for(int j = 0; j < ny; j++)
            if(weight[i][j] >= threshold)
                line[i][j] = 1;
}

int Hungarian::find(int u)                  //Ѱ��Դ�ڵ�ΪXu������·(��1��0)
//����·����������ôһ��·������·��Դ�㿪ʼһֱһ��һ�ε������˻�㣬���ң�����·�ϵ�ÿһ�ζ���������<������
//ע�⣬���ϸ��<,������<=����ô������һ�����ҵ�����·�ϵ�ÿһ�ε�(����-����)��ֵ���е���Сֵdelta�����ǰ���
//��·��ÿһ�ε��������������delta��һ�����Ա�֤�������Ȼ�ǿ��������������Ǿ͵õ���һ���������������������
//֮ǰ������+delta��������·�ͽ�������·
{
    for(int v = 0; v < ny; v++)    //���Xi��ĳ������u����������Yi����v
    {
        if(line[u][v] && !mark[v] ) //��u��v�������� �� vû�б����
        {
            mark[v] = 1;            //���v
            if(cy[v] == -1 || find(cy[v])) //���vû�б�ƥ�䣬��v�Ѿ�ƥ���˵���v���������ҵ�����·
            {
                cx[u] = v;//��vƥ���u
                cy[v] = u;//��uƥ���v
                return 1; //�ҵ�������·
             // break;
            }
        }
    }
    return 0;
}

int Hungarian::maxMatch()          //�õ���Yiƥ���X��
{
    int res = 0;
    cx.assign(N, -1);
    cy.assign(N, -1);
    for(int i=0; i<nx; i++)
       {
           if(cx[i]==-1) //��ÿ��δ�ǵ��������Ѱ������·
           {
               mark.assign(N, 0);
               res+=find(i); //ÿ�ҵ�һ������·����ʹ��ƥ������1
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





