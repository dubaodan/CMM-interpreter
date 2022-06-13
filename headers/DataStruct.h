#ifndef GRAMMAR_DATASTRUCT_H
#define GRAMMAR_DATASTRUCT_H
#define MaxDepth 10

#include <string>
#include <utility>
#include <vector>
#include <set>

using namespace std;
struct Token{
    Token(int line,int id,int index){
        this->line = line;
        this->id = id;
        this->index =index;
    }
    int line;   //行号
    int id;     //类别码
    int index;  //在字符串、常量、标识符表中的下标值
};

struct Identifier{
    Identifier(string name, int index){
        this->name = move(name);
        this->index = index;
    }
    string name;
    int index;
};

struct Const{
    Const(string value, int index){
        this->value = move(value);
        this->index = index;
    }
    string value;
    int index;
};

struct String{
    String(string content, int index){
        this->content = move(content);
        this->index = index;
    }
    string content;
    int index;
};


struct symbol{
    int index{};
    string name;
    string type;
    int level[MaxDepth]{};
    bool initialized = false;   //是否初始化
    //所有属性值
    int iValue{};
    double rValue{};
    string sValue;

    vector<int> iValues;
    vector<double> rValues;
    int size;

    symbol * arrayRef;
    symbol * arrayIndexRef;

    symbol(string name, string type){
        this->name = std::move(name);
        this->type = std::move(type);
    }
    friend ostream &operator<<(ostream &output, const symbol &s){
        output<<"("<<s.index<<")\t"+s.name+"\t"+s.type;
        return output;
    }
};

struct Func{
    string name;
    symbol *begin;
    symbol *end;
    vector<string> parameters;
    string returnType;
    int entryPoint;
    int exitPoint{};
    Func(int entryPoint, string name, symbol * begin,
            symbol * end, string returnType, vector<string> parameters = {}) {
        this->entryPoint = entryPoint;
        this->name = std::move(name);
        this->begin = begin;
        this->end = end;
        this->returnType = std::move(returnType);
        this->parameters = std::move(parameters);
    }
    friend ostream &operator<<(ostream &output, const Func &F){
        output<<F.returnType + "\t" + F.name + "\t(";
        for (auto & iter : F.parameters)
            output<<iter + " ";
        output<<")\t entry: "<<F.entryPoint<<" exit: "<<F.exitPoint<<"\tSymbol:"<<F.begin->index<<" to "<<F.end->index;
        return output;
    }
};

struct Quad{
    string _operator;
    symbol *op1;
    symbol *op2;
    symbol *dest;
    int jDest;  //跳转指令的跳转地址
    Quad(string _operator, symbol *op1, symbol *op2, symbol *dest){
        this->_operator = std::move(_operator);
        this->op1 = op1;
        this->op2 = op2;
        this->dest = dest;
        this->jDest = -1;   //非跳转指令为-1
    }
    friend ostream &operator<<(ostream &output, const Quad &D){
        output<<"(" + D._operator + ",\t";
        if (D.op1 == nullptr){
            output<<"__ ,\t";
        } else {
            output<<"(" <<D.op1->index<< ")" + D.op1->name + ",\t";
        }
        if (D.op2 == nullptr){
                output << "__ ,\t";
        } else {
            if(D._operator == "[]=" && D.op2->name.empty()){
                output<<D.op2->iValue<<",\t";
            } else {
                output << "(" << D.op2->index << ")" + D.op2->name + ",\t";
            }
        }

        if (D._operator[0] == 'j'){
            output<<D.jDest<<")";
        } else if (D._operator == "ret"){
            if (D.dest == nullptr)
                output<<"__)";
            else
                output<<D.dest->name + ")";
        } else if(D.dest == nullptr && D.op1 != nullptr){
            output<<"(" <<D.op1->index<< ")" + D.op1->name + ")";
        }
        else {
            output<<"(" <<D.dest->index<< ")" + D.dest->name + ")";
        }
        return output;
    }
};

struct Node{
    explicit Node(int numNTSymbol, vector<Node*> ch){ //构造非终结符节点
        this->token = new Token(0, numNTSymbol, 0);
        this->children = std::move(ch);
        this->place = nullptr;
        this->which = '\0';
    }
    explicit Node(Token *t){    //用Token构造Node
        this->token = t;
        this->children = vector<Node*>();
        this->place = nullptr;
        this->which = '\0';
    }
    Token * token;  //该节点对应的token或非终结符，其中token的ID在表示非终结符时为其映射的整形值
    vector<Node*> children; //非终结符节点此处为空
    symbol * place;  //在符号表中的位置
    char which; //何种符号 'i','c','s','b'
    //int level[MaxDepth] = {0};    //当前节点（符号）作用域深度，同一深度下从1开始排序，全局变量处于最高级作用范围域内，且唯一({1,0,0,0...})

    vector<string> parameters;  //参数类型表
    int parametersCount;    //参数数量，调用函数时记录

    vector<symbol*> initalizers;
    int size = 0;

    set<int> trueList;  //真出口链
    set<int> falseList; //假出口链
    set<int> nextList;  //出口链
    set<int> breakList; //break链
    set<int> continueList;  //continue链
    set<int> returnList;    //返回链
};


#endif //GRAMMAR_DATASTRUCT_H
