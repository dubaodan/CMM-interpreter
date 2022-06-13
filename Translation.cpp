#include "headers/Translation.h"

#include <utility>

int nextQuad = 1;   //从1开始标号,0处插入nullptr
vector<int> recursiveOrNot;

void _emit(string operator_, symbol *op1, symbol *op2, symbol *dest) {
    auto q = new Quad(std::move(operator_),op1,op2,dest);
    Quads.emplace_back(q);
    nextQuad++;
}

string getIName(Node *node) {
    if (node->token->id != 100)
        return "";
    return vIdentifier[node->token->index].name;
}

string getCValue(Node *node){
    if (node->token->id != 99)
        return "";
    return vConst[node->token->index].value;
}

string getSContent(Node *node){
    if (node->token->id != 98)
        return "";
    return vString[node->token->index].content;
}

symbol * FindSymbol(const string& name, int *level) {
    int distance = 0;
    symbol *returnSymbol = nullptr;
    for (auto & iter : SymbolTable){
        //判断当前符号是否匹配，且是否在作用域内
        if (iter->name == name && isIncluded(iter->level,level)) {
            int currentDistance = scopeDistance(iter->level, level);
            //取最近的符合条件的符号
            if (distance < currentDistance) {
                distance  = currentDistance;
                returnSymbol = iter;
            }
        }
    }
    terrno = returnSymbol == nullptr ? 1 : terrno;
    return returnSymbol;
}

bool isIncluded(const int *first, const int *second) {
    for (int i = 0; i < MaxDepth && first[i] != 0; ++i)
        if (first[i] != second [i])
            return false;
    return true;
}

bool isSame(const int *first, const int *second) {
    for (int i = 0; i < MaxDepth ; ++i)
        if(first[i] != second[i])
            return false;
    return true;
}

int scopeDistance(const int *first, const int *second) {
    int i = 0;
    for (; i < MaxDepth && (first[i] == second[i] != 0); ++i);
    return i;
}

int depth(const int *first) {
    int i = 0;
    for(; i < MaxDepth && first[i] != 0; ++i);
    return i;
}

int tempCount = 0;
symbol * AddTemp(string type) {
    string name = to_string(tempCount) + "_temp";
    tempCount++;
    auto temp = new symbol(name, std::move(type));
    temp->index = SymbolTable.back()->index + 1;
    SymbolTable.emplace_back(temp);
    return temp;
}

bool AddSymbol(const string& name, int *level) {
    for (auto & iter : SymbolTable) {
        if (iter->name == name) {
            if (isSame(iter->level, level)) {
                setErrno(2); //重复定义
                return false;
            } else if(isIncluded(iter->level, level)) {
                setErrno(3); //覆盖定义
            }
        }
    }
    auto newSymbol = new symbol(name,"unknown");
    newSymbol->index = SymbolTable.back()->index + 1;
    memcpy(newSymbol->level,level, sizeof(int) * MaxDepth);
    SymbolTable.emplace_back(newSymbol);
    return true;
}

bool AddISymbol(const string& name, int *level, int value) {
    //查找是否有重定义的符号
    for (auto & iter : SymbolTable) {
        if (iter->name == name) {
            if (isSame(iter->level, level)) {
                setErrno(2); //重复定义
                return false;
            } else if(isIncluded(iter->level, level)) {
                setErrno(3); //覆盖定义
            }
        }
    }
    auto newSymbol = new symbol(name,"INT");
    newSymbol->iValue = value;
    newSymbol->index = SymbolTable.back()->index + 1;
    memcpy(newSymbol->level,level, sizeof(int) * MaxDepth);
    SymbolTable.emplace_back(newSymbol);
    return true;
}

bool AddRSymbol(const string& name, int *level, double value) {
    //查找是否有重定义的符号
    for (auto & iter : SymbolTable) {
        if (iter->name == name) {
            if (isSame(iter->level, level)) {
                setErrno(2); //重复定义
                return false;
            } else if(isIncluded(iter->level, level)) {
                setErrno(3); //覆盖定义
            }
        }
    }
    auto newSymbol = new symbol(name,"REAL");
    newSymbol->rValue = value;
    newSymbol->index = SymbolTable.back()->index + 1;
    memcpy(newSymbol->level,level, sizeof(int) * MaxDepth);
    SymbolTable.emplace_back(newSymbol);
    return true;
}

bool AddSSymbol(const string& name, int *level, string value) {
    //查找是否有重定义的符号
    for (auto & iter : SymbolTable) {
        if (iter->name == name) {
            if (isSame(iter->level, level)) {
                setErrno(2); //重复定义
                return false;
            } else if(isIncluded(iter->level, level)) {
                setErrno(3); //覆盖定义
            }
        }
    }
    auto newSymbol = new symbol(name,"STRING");
    newSymbol->sValue = std::move(value);
    newSymbol->index = SymbolTable.back()->index + 1;
    memcpy(newSymbol->level,level, sizeof(int) * MaxDepth);
    SymbolTable.emplace_back(newSymbol);
    return true;
}

const string errors[] = {// NOLINT(cert-err58-cpp)
        "",
        "错误：使用了未定义的变量",
        "错误：重复定义变量",
        "错误：覆盖了之前的变量",
        "错误：必须是可赋值的变量",
        "错误：使用了未初始化的变量",//5
        "错误：布尔表达式错误，关系运算符必须作用于可计算的数值型",
        "错误：错误的控制语句位置，必须在WHILE，DOWHILE，FOR循环中使用",
        "错误：函数返回语句中缺少返回值",
        "错误：函数应没有返回值",
        "错误：函数缺少返回语句",//10
        "错误：函数的返回值类型错误",
        "错误：调用函数的参数数目不匹配",
        "错误：不存在函数",
        "错误：调用函数格式错误",
        "错误：调用函数没有返回值，但是期望得到一个返回值",//15
        "错误：下标运算只能作用于数组类型和指针类型",
        "错误：数组下标访问越界",
        "错误：数组初始化参数过多",
        "错误：不兼容的数据类型",
        "错误：数组下标必须为整形值",//20
        "错误：数组初始化大小必须为常数",
        "错误：数组初始化失败，大小必须为正整数",
        "错误：没有与操作数匹配的运算符",
        "错误：赋值时左右值类型不同",
        "错误：赋值时此种转换不支持",
        "错误：不能除0",
        "错误：数组下标必须为整数",
        "错误：数组访问越界"

};

void setErrno(int newErrno){
    terrno = newErrno;
}

void pterror(int line, const string& str){
    if (terrno > 0) {
        cout << errors[terrno] + ": ";
        if (line !=0)
            cout<<"行" << line;
        cout<<"\t符号: "+str <<endl;
        terrno = 0;
    }
}

void setType(const string& type) {
    for (auto iter = SymbolTable.rbegin(); iter != SymbolTable.rend() ; iter++)
        if ((*iter)->type == "unknown")
            (*iter)->type = type;
        else if ((*iter)->type != "temporary")
            return;
}

void printSymbolTable() {
    for(auto & iter : SymbolTable)
        cout<<(*iter)<<endl;
}

void backPatch(const set<int>& list, int quad) {
    if(Quads.size() < 2)
        return;
    for (auto & iter : list) {
        //for test
//        if ((Quads[iter]->jDest != quad && Quads[iter]->jDest != -1)
//                || Quads[iter]->dest != nullptr)
//            cout<<"error backpatch";
        Quads[iter]->jDest = quad;
    }
}

set<int> merge(set<int> list1, set<int> list2) {
    set<int> merged = std::move(list1);
    merged.insert(list2.begin(),list2.end());
    return merged;
}

void printSymbolTableValue() {
    for(auto & iter : SymbolTable) {
        cout<<(*iter)<<"\t";
        switch (iter->type[0]) {
            case 'I':
                if (iter->type.back() == ']'){
                    int count = 0;
                    cout<<endl;
                    for (auto & i : iter->iValues) {
                        cout<<"\t["<<count<<"]\t"<<i<<endl;
                        count++;
                    }
                } else {
                    cout<<iter->iValue<<endl;
                }
                break;
            case 'R':
                if (iter->type.back() == ']'){
                    int count = 0;
                    cout<<endl;
                    for (auto & i : iter->rValues) {
                        cout<<"\t["<<count<<"]\t"<<i<<endl;
                         count++;
                    }
                } else {
                    cout<<iter->rValue<<endl;
                }
                break;
            case 'A':
                cout<<iter->sValue<<endl;
            default:
                cout<<"NaN"<<endl;
        }
    }
}

void printQuads() {
    if (Quads.size() == 1)
        return;
    int size = Quads.size();
    for (int i = 1; i < size; ++i) {
        cout<<i<<"\t";
        cout<<*Quads[i]<<endl;
    }
}

Func *FindFunc(const string &name) {
    for (auto & funcs : FunctionTable){
        if (funcs->name == name)
            return funcs;
    }
    return nullptr;
}

void printSymbolTableNoTemp() {
    for(auto & iter : SymbolTable) {
        if (iter->name[0] >= '0' && iter->name[0] <= '9')
            continue;
        cout << (*iter) << "\t";
        switch (iter->type[0]) {
            case 'I':
                if (iter->type.back() == ']') {
                    int count = 0;
                    cout << endl;
                    for (auto &i : iter->iValues) {
                        cout << "\t[" << count << "]\t" << i << endl;
                        count++;
                    }
                } else {
                    cout << iter->iValue << endl;
                }
                break;
            case 'R':
                if (iter->type.back() == ']') {
                    int count = 0;
                    cout << endl;
                    for (auto &i : iter->rValues) {
                        cout << "\t[" << count << "]\t" << i << endl;
                        count++;
                    }
                } else {
                    cout << iter->rValue << endl;
                }
                break;
            case 'A':
                cout << iter->sValue << endl;
            default:
                cout << "NaN" << endl;
        }
    }

}

void printFunctionTable() {
    for(auto & iter : FunctionTable)
        cout<<(*iter)<<endl;
}

void prepareEnv() {
    //初始化返回值
    register_rax = new symbol("return_val","");
    register_rax->initialized = true;
    register_rax->index = 0;
    SymbolTable.emplace_back(new symbol("",""));    //初始化符号表
    Quads.emplace_back(nullptr);    //四元组从1开始标号，可以直接通过标号进行下标访问

    //增加print函数

    auto * printFuncS = new symbol("print","function");
    printFuncS->level[0] = 1;
    printFuncS->initialized = true;
    printFuncS->index = 1;
    auto * printFuncP = new symbol("out","AUTO");
    printFuncP->index = 2;
    SymbolTable.emplace_back(printFuncS);
    SymbolTable.emplace_back(printFuncP);
    Quad * pq1 = new Quad("cout", nullptr, nullptr,printFuncP);
    Quad * pq2 = new Quad("ret", nullptr, nullptr, nullptr);
    Quads.emplace_back(pq1);
    Quads.emplace_back(pq2);
    Func * printFunc = new Func(1,"print",printFuncS,printFuncP,"VOID",{"AUTO"});
    printFunc->exitPoint = 2;
    FunctionTable.emplace_back(printFunc);

    //增加scan函数

    auto * scanFuncS = new symbol("scan","function");
    scanFuncS->level[0] = 1;
    scanFuncS->initialized = true;
    scanFuncS->index = 3;
    auto * scanFuncP = new symbol("in","INT");
    scanFuncP->index = 4;
    SymbolTable.emplace_back(scanFuncS);
    SymbolTable.emplace_back(scanFuncP);
    Quad * sq1 = new Quad("cin", nullptr, nullptr, scanFuncP);
    Quad * sq2 = new Quad("ret", nullptr, nullptr, scanFuncP);
    Quads.emplace_back(sq1);
    Quads.emplace_back(sq2);
    Func * scanFunc = new Func(3,"scan",scanFuncS,scanFuncP,"INT");
    scanFunc->exitPoint =4;
    FunctionTable.emplace_back(scanFunc);

    nextQuad = 5;
}

void analyzeQuads() {
    int count = 1, total = Quads.size();
    string judge;
    vector<int> allpoints;
    for (int m = 1; m < total; m++) {
        bool inside = true;
        for (auto & i : FunctionTable) {
            if (m >= i->entryPoint && m <= i->exitPoint) {
                inside = false;
                break;
            }
        }
        if (inside)
            allpoints.push_back(m);
    }
    bool inmain = true;
    for (; count<total; count++) {
        if (!allpoints.empty()) {
            count = allpoints.front();
            auto iter1 = allpoints.begin();
            allpoints.erase(iter1);
        } else {
            if (inmain) {
                Func *mainfun = FindFunc("main");
                count = mainfun->entryPoint;
                inmain = false;
            }
        }
        judge = Quads[count]->_operator;
        symbol *op1T = Quads[count]->op1;
        symbol *op2T = Quads[count]->op2;
        symbol *destT = Quads[count]->dest;
        switch (judge[0]) {
            //所有赋值处理
            case ':':
                if (op1T->type != destT->type) {
                    if (destT->type == "REAL") { setErrno(25); pterror(); destT->rValue = (double)op1T->iValue; }
                    setErrno(24);
                    pterror();
                    exit(1);
                }
                else {
                    if (destT->type == "INT") destT->iValue = op1T->iValue;
                    else if (destT->type == "REAL") destT->rValue = op1T->rValue;
                }
                break;

                //所有数组处理
            case '=':
                if (op2T->type != "INT") {
                    setErrno(27);
                    pterror();
                    exit(1);
                }
                if (destT->type == "temporary") {
                    if (op2T->iValue > op1T->size - 1) {
                        setErrno(28);
                        pterror();
                        exit(1);
                    }
                    else {
                        if (op1T->type == "INT[]") {
                            destT->type = op1T->type;
                            destT->type.pop_back();
                            destT->type.pop_back();
                            destT->iValue = op1T->iValues[op2T->iValue];
                        }
                        else {
                            destT->type = op1T->type;
                            destT->type.pop_back();
                            destT->type.pop_back();
                            destT->rValue = op1T->rValues[op2T->rValue];
                        }
                    }
                }
                else {
                    if (destT->type + "[]" != op1T->type) {
                        if (destT->type == "REAL") {
                            setErrno(25);
                            pterror();
                            destT->rValue = (double)op1T->iValues[op2T->iValue];
                        }
                        setErrno(24);
                        pterror();
                        exit(1);
                    }
                    else {
                        if (op2T->iValue > op1T->size - 1) {
                            setErrno(27);
                            pterror();
                            exit(1);
                        }
                        else {
                            if (destT->type == "INT")
                                destT->iValue = op1T->iValues[op2T->iValue];
                            else destT->rValue = op1T->rValues[op2T->iValue];
                        }
                    }
                }
                break;
            case '[':
                if (op2T->type != "INT") {
                    setErrno(27);
                    pterror();
                    exit(1);
                }
                else {
                    if (op1T->type + "[]" != destT->type) {
                        if (op1T->type == "REAL") {
                            setErrno(25);
                            pterror();
                            op1T->rValue = (double)destT->iValues[op2T->iValue];
                        }
                        setErrno(24);
                        pterror();
                        exit(1);
                    }
                    else if (destT->type == "INT[]") {
                        if (op2T->iValue > destT->size - 1) {
                            setErrno(28);
                            pterror();
                            exit(1);
                        }
                        else {
                            destT->iValues[op2T->iValue] = op1T->iValue;
                        }
                    }
                    else {
                        if (op2T->iValue > destT->size - 1) {
                            setErrno(28);
                            pterror();
                            exit(1);
                        }
                        else {
                            destT->rValues[op2T->iValue] = op1T->rValue;
                        }
                    }
                }
                break;
                //所有一元运算符，包括+=等赋值
            case '+':
                if (judge == "+=") {
                    if (op1T->type != destT->type) {
                        if (destT->type == "REAL") {
                            setErrno(25);
                            pterror();
                            destT->rValue += (double)op1T->iValue;
                        }
                        setErrno(24);
                        pterror();
                        exit(1);
                    }
                    else {
                        if (destT->type == "INT") destT->iValue += op1T->iValue;
                        else if (destT->type == "REAL") destT->rValue += op1T->rValue;
                    }
                }
                else if (judge == "++") {
                    if (op1T->type == "INT") op1T->iValue++;
                    else if (op1T->type == "REAL") op1T->rValue++;
                    else {
                        setErrno(23);
                        pterror();
                        exit(1);
                    }
                }
                else {
                    if (op1T->type != op2T->type) {
                        if (op1T->type == "REAL" && op2T->type == "INT") {
                            destT->type = "REAL";
                            destT->rValue = op1T->rValue + (double)op2T->iValue;
                        }
                        else {
                            destT->type = "REAL";
                            destT->rValue = (double)op1T->iValue + op2T->iValue;
                        }
                    }
                    else {
                        if (op1T->type == "INT") {
                            destT->type = "INT";
                            destT->iValue = op1T->iValue + op2T->iValue;
                        }
                        else {
                            destT->type = "REAL";
                            destT->rValue = op1T->rValue + op2T->rValue;
                        }
                    }
                }
                break;

            case '-':
                if (judge == "-=") {
                    if (op1T->type != destT->type) {
                        if (destT->type == "REAL") {
                            setErrno(25);
                            pterror(25);
                            destT->rValue -= (double)op1T->iValue;
                        }
                        setErrno(24);
                        pterror();
                        exit(1);
                    }
                    else {
                        if (destT->type == "INT") destT->iValue -= op1T->iValue;
                        else if (destT->type == "REAL") destT->rValue -= op1T->rValue;
                    }
                }
                else if (judge == "--") {
                    if (op1T->type == "INT") op1T->iValue--;
                    else if (op1T->type == "REAL") op1T->rValue--;
                    else {
                        setErrno(23);
                        pterror();
                        exit(1);
                    }
                }
                else {
                    if (op1T->type != op2T->type) {
                        if (op1T->type == "REAL" && op2T->type == "INT") {
                            destT->type = "REAL";
                            destT->rValue = op1T->rValue - (double)op2T->iValue;
                        }
                        else {
                            destT->type = "REAL";
                            destT->rValue = (double)op1T->iValue - op2T->iValue;
                        }
                    }
                    else {
                        if (op1T->type == "INT") {
                            destT->type = "INT";
                            destT->iValue = op1T->iValue - op2T->iValue;
                        }
                        else {
                            destT->type = "REAL";
                            destT->rValue = op1T->rValue - op2T->rValue;
                        }
                    }
                }
                break;

            case '*':
                if (judge == "*=") {
                    if (op1T->type != destT->type) {
                        if (destT->type == "REAL") {
                            setErrno(25);
                            pterror();
                            destT->rValue *= (double)op1T->iValue;
                        }
                        setErrno(24);
                        pterror();
                        exit(1);
                    }
                    else {
                        if (destT->type == "INT") destT->iValue *= op1T->iValue;
                        else if (destT->type == "REAL") destT->rValue *= op1T->rValue;
                    }
                }
                else {
                    if (op1T->type != op2T->type) {
                        if (op1T->type == "REAL" && op2T->type == "INT") {
                            destT->type = "REAL";
                            destT->rValue = op1T->rValue * (double)op2T->iValue;
                        }
                        else {
                            destT->type = "REAL";
                            destT->rValue = (double)op1T->iValue * op2T->iValue;
                        }
                    }
                    else {
                        if (op1T->type == "INT") {
                            destT->type = "INT";
                            destT->iValue = op1T->iValue * op2T->iValue;
                        }
                        else {
                            destT->type = "REAL";
                            destT->rValue = op1T->rValue * op2T->rValue;
                        }
                    }
                }
                break;

            case '/':
                if (judge == "/=") {
                    if (op1T->type != destT->type) {
                        if (destT->type == "REAL") {
                            setErrno(25);
                            pterror();
                            if (op1T->iValue == 0) { setErrno(26); pterror(26); exit(1); }
                            destT->rValue /= (double)op1T->iValue;
                        }
                        setErrno(24);
                        pterror();
                        exit(1);
                    }
                    else {
                        if (op1T->iValue == 0) {
                            if (op1T->iValue == 0) {
                                setErrno(26);
                                pterror();
                                exit(1);
                            }
                            if (destT->iValue % op1T->iValue == 0)destT->iValue /= op1T->iValue;
                            else {
                                setErrno(24);
                                pterror();
                                exit(1);
                            }
                        }
                        else if (destT->type == "REAL") {
                            if (op1T->rValue == 0) {
                                setErrno(26);
                                pterror();
                                exit(1);
                            }
                            destT->rValue /= op1T->rValue;
                        }
                    }
                }
                else {
                    if (op1T->type != op2T->type) {
                        if (op1T->type == "REAL" && op2T->type == "INT") {
                            if (op2T->iValue == 0) {
                                setErrno(26);
                                pterror();
                                exit(1);
                            }
                            destT->type = "REAL";
                            destT->rValue = (double)op1T->rValue / (double)op2T->iValue;
                        }
                        else {
                            if (op2T->rValue == 0) {
                                setErrno(26);
                                pterror();
                                exit(1);
                            }
                            destT->type = "REAL";
                            destT->rValue = (double)op1T->iValue / (double)op2T->iValue;
                        }
                    }
                    else {
                        if (op1T->type == "INT") {
                            if (op2T->iValue == 0) {
                                setErrno(26);
                                pterror();
                                exit(1);
                            }
                            if (op1T->iValue % op2T->iValue == 0) {
                                destT->type = "INT";
                                destT->iValue = op1T->iValue / op2T->iValue;
                            }
                            else {
                                destT->type = "REAL";
                                destT->rValue = (double)op1T->iValue / op2T->iValue;
                            }
                        }
                        else {
                            if (op2T->iValue == 0) {
                                setErrno(26);
                                pterror();
                                exit(1);
                            }
                            destT->type = "REAL";
                            destT->rValue = op1T->rValue / op2T->rValue;
                        }
                    }
                }
                break;

            case '%':
                if (judge == "%=") {
                    if (op1T->type != destT->type) {
                        setErrno(24);
                        pterror();
                        exit(1);
                    }
                    else {
                        if (destT->type == "INT") {
                            if (op1T->iValue == 0) {
                                setErrno(26);
                                pterror();
                                exit(1);
                            }
                            destT->iValue %= op1T->iValue;
                        }
                        else if (destT->type == "REAL") {
                            setErrno(23);
                            pterror();
                            exit(1);
                        }
                    }
                }
                else {
                    if (op1T->type != op2T->type) {
                        setErrno(24);
                        pterror();
                        exit(1);
                    }
                    else {
                        if (op1T->type == "INT") {
                            if (op2T->iValue == 0) {
                                setErrno(26);
                                pterror();
                                exit(1);
                            }
                            destT->iValue = op1T->iValue % op2T->iValue;
                        }
                        else if (op1T->type == "REAL") {
                            setErrno(23);
                            pterror();
                            exit(1);
                        }
                    }
                }
                break;

                //所有jmp相关的代码
            case 'j':
                if (judge == "j>") {
                    if (op1T->type != op2T->type) {
                        setErrno(9);
                        pterror();
                    }
                    else {
                        if (op1T->type == "INT") {
                            if (op1T->iValue > op2T->iValue) count = Quads[count]->jDest - 1;
                        }
                        else if (destT->type == "REAL") {
                            if (op1T->rValue > op2T->rValue) count = Quads[count]->jDest - 1;
                        } continue;
                    }
                }
                else if (judge == "j>=") {
                    if (op1T->type != op2T->type) {
                        setErrno(9);
                        pterror();
                    }
                    else {
                        if (op1T->type == "INT") {
                            if (op1T->iValue >= op2T->iValue) count = Quads[count]->jDest - 1;
                        }
                        else if (destT->type == "REAL") {
                            if (op1T->rValue >= op2T->rValue) count = Quads[count]->jDest - 1;
                        } continue;
                    }
                }
                else if (judge == "j<") {
                    if (op1T->type != op2T->type) {
                        setErrno(9);
                        pterror();
                    }
                    else {
                        if (op1T->type == "INT") {
                            if (op1T->iValue < op2T->iValue) count = Quads[count]->jDest - 1;
                        }
                        else if (destT->type == "REAL") {
                            if (op1T->rValue < op2T->rValue) count = Quads[count]->jDest - 1;
                        } continue;
                    }
                }
                else if (judge == "j<=") {
                    if (op1T->type != op2T->type) {
                        setErrno(9);
                        pterror();
                    }
                    else {
                        if (op1T->type == "INT") {
                            if (op1T->iValue <= op2T->iValue) count = Quads[count]->jDest - 1;
                        }
                        else if (destT->type == "REAL") {
                            if (op1T->rValue <= op2T->rValue) count = Quads[count]->jDest - 1;
                        } continue;
                    }
                }
                else if (judge == "j==") {
                    if (op1T->type != op2T->type) {
                        setErrno(9);
                        pterror();
                    }
                    else {
                        if (op1T->type == "INT") {
                            if (op1T->iValue == op2T->iValue) count = Quads[count]->jDest - 1;
                        }
                        else if (destT->type == "REAL") {
                            if (op1T->rValue == op2T->rValue) count = Quads[count]->jDest - 1;
                        } continue;
                    }
                }
                else if (judge == "j<>") {
                    if (op1T->type != op2T->type) {
                        setErrno(9);
                        pterror();
                    }
                    else {
                        if (op1T->type == "INT") {
                            if (op1T->iValue != op2T->iValue) count = Quads[count]->jDest - 1;
                        }
                        else if (destT->type == "REAL") {
                            if (op1T->rValue != op2T->rValue) count = Quads[count]->jDest - 1;
                        } continue;
                    }
                }
                else {
                    count = Quads[count]->jDest - 1;
                    continue;
                }
                break;

                //所有函数处理
            case 'c':
                if (judge == "call") {
                    //首先要存一个返回地址，以及该函数的符号表中的位置
                    returnJmps.push_back(count);
                    Func *funcnow = FindFunc(destT->name);
                    FunctionUseTable.push_back(funcnow);
                    //判断同名还是异名，也就是是不是递归调用
                    if (count >= funcnow->entryPoint && count <= funcnow->exitPoint) {
                        recursiveOrNot.push_back(1);
                        //如果是递归调用，首先将其符号表压栈，保存当时现场,并记录其为递归
                        int bg = funcnow->begin->index;
                        int ed = funcnow->end->index;
                        for (int i = ed; i >= bg; i--) {
                            auto *newSymbol = new symbol(SymbolTable[i]->name, SymbolTable[i]->type);
                            if (newSymbol->type == "REAL") {
                                newSymbol->rValue = SymbolTable[i]->rValue;
                                SymbolTable[i]->rValue = 0;
                            }
                            else if (newSymbol->type == "INT") {
                                newSymbol->iValue = SymbolTable[i]->iValue;
                                SymbolTable[i]->iValue = 0;
                            }
                            else if (newSymbol->type == "INT[]") {
                                newSymbol->iValues.insert(newSymbol->iValues.end(), SymbolTable[i]->iValues.begin(),
                                                          SymbolTable[i]->iValues.end());
                                newSymbol->size = SymbolTable[i]->size;
                                SymbolTable[i]->iValues.clear();
                                SymbolTable[i]->size = 0;
                            }
                            else if (newSymbol->type == "REAL[]") {
                                newSymbol->iValues.insert(newSymbol->iValues.end(), SymbolTable[i]->iValues.begin(),
                                                          SymbolTable[i]->iValues.end());
                                newSymbol->size = SymbolTable[i]->size;
                                SymbolTable[i]->rValues.clear();
                                SymbolTable[i]->size = 0;
                            }
                            functionSymbolTable.push_back(*newSymbol);
                        }
                        //处理参数传递问题，并且在使用完后清理栈
                        int pass1 = destT->index;
                        pass1++;
                        for (char now : paraSequence) {
                            symbol *nowSymbol = SymbolTable[pass1];
                            if (now == 'i') {
                                if (nowSymbol->type != "INT") {
                                    setErrno(24);
                                    pterror();
                                    exit(1);
                                }
                                auto iter1 = paraIntStack.begin();
                                nowSymbol->iValue = *iter1;
                                paraIntStack.erase(iter1);
                            }
                            else {
                                if (nowSymbol->type != "REAL") {
                                    setErrno(24);
                                    pterror();
                                    exit(1);
                                }
                                auto iter1 = paraRealStack.begin();
                                nowSymbol->rValue = *iter1;
                                paraRealStack.erase(iter1);
                            }
                        }
                        paraSequence.clear();
                        //处理函数语句跳转问题
                        count = funcnow->entryPoint - 1;
                    }
                        //如果不是递归调用，则直接清空该函数的所有函数内变量的数值，然后传参,并记录非递归
                    else {
                        recursiveOrNot.push_back(0);
                        int bg = funcnow->begin->index;
                        int ed = funcnow->end->index;
                        for (int i = ed; i >= bg; i--) {
                            if (SymbolTable[i]->type == "REAL") {
                                SymbolTable[i]->rValue = 0;
                            }
                            else if (SymbolTable[i]->type == "INT") {
                                SymbolTable[i]->iValue = 0;
                            }
                            else if (SymbolTable[i]->type == "INT[]") {
                                SymbolTable[i]->iValues.clear();
                                SymbolTable[i]->size = 0;
                            }
                            else if (SymbolTable[i]->type == "REAL[]") {
                                SymbolTable[i]->rValues.clear();
                                SymbolTable[i]->size = 0;
                            }
                        }
                    }
                    //处理参数传递
                    int pass1 = destT->index;
                    for (char now : paraSequence) {
                        pass1++;
                        symbol *nowSymbol = SymbolTable[pass1];

                        if (now == 'i') {
                            auto iter1 = paraIntStack.begin();
                            if (nowSymbol->type == "AUTO") {
                                nowSymbol->sValue = to_string(*iter1);
                            }
                            else if (nowSymbol->type != "INT") {
                                setErrno(24);
                                pterror();
                                exit(1);
                            }
                            else
                                nowSymbol->iValue = *iter1;
                            paraIntStack.erase(iter1);
                        }
                        else {
                            auto iter1 = paraRealStack.begin();
                            if (nowSymbol->type == "AUTO") {
                                nowSymbol->sValue = to_string(*iter1);
                            }
                            else if (nowSymbol->type != "REAL") {
                                setErrno(24);
                                pterror();
                                exit(1);
                            }
                            else
                                nowSymbol->rValue = *iter1;
                            paraRealStack.erase(iter1);
                        }
                    }
                    paraSequence.clear();
                    //处理函数语句跳转问题
                    count = funcnow->entryPoint - 1;
                }
                else if (judge == "cout") {
                    cout << destT->sValue << endl;
                }
                else if (judge == "cin") {
                    cin >> destT->sValue;
                }
                break;

                //处理函数的参数传递
            case 'p':
                if (judge == "push") {
                    if (destT->type == "INT") {
                        paraIntStack.push_back(destT->iValue);
                        paraSequence += "i";
                    }
                    else {
                        paraRealStack.push_back(destT->rValue);
                        paraSequence += "r";
                    }
                }
                break;

            case 'r':
                if (judge == "ret") {
                    //首先处理程序是否退出
                    Func *funcMain = FindFunc("main");
                    int st = funcMain->entryPoint;
                    int nd = funcMain->exitPoint;
                    if (st <= count && nd >= count)
                        return;

                    //首先处理返回值的情况
                    Func *funcUse = FunctionUseTable.back();
                    register_rax->type = funcUse->returnType;
                    if (register_rax->type == "INT")
                        register_rax->iValue = destT->iValue;
                    else if (register_rax->type == "REAL")
                        register_rax->rValue = destT->rValue;

                    if (funcUse->name == "scan") {
                        //fixme:
                        register_rax->type = "INT";
                        register_rax->iValue = atoi(destT->sValue.c_str());
                    }
                    //再把函数表中各个值归位
                    if (recursiveOrNot.back() == 1) {
                        int tbStart = funcUse->begin->index;
                        int tbEnd = funcUse->end->index;
                        for (int i = tbStart; i <= tbEnd; i++) {
                            symbol symbolTemp = functionSymbolTable.back();
                            if (SymbolTable[i]->type == "REAL") {
                                SymbolTable[i]->rValue = symbolTemp.rValue;
                            }
                            else if (SymbolTable[i]->type == "INT") {
                                SymbolTable[i]->iValue = symbolTemp.iValue;
                            }
                            else if (SymbolTable[i]->type == "INT[]") {
                                SymbolTable[i]->iValues.clear();
                                SymbolTable[i]->iValues.insert(SymbolTable[i]->iValues.end(),
                                                               symbolTemp.iValues.begin(),
                                                               symbolTemp.iValues.end());
                                SymbolTable[i]->size = symbolTemp.size;
                            }
                            else if (SymbolTable[i]->type == "REAL[]") {
                                SymbolTable[i]->iValues.clear();
                                SymbolTable[i]->rValues.insert(SymbolTable[i]->rValues.end(),
                                                               symbolTemp.rValues.begin(),
                                                               symbolTemp.rValues.end());
                                SymbolTable[i]->size = symbolTemp.size;
                            }
                            functionSymbolTable.pop_back();
                        }
                    }
                    count = returnJmps.back();
                    returnJmps.pop_back();
                    FunctionUseTable.pop_back();
                    recursiveOrNot.pop_back();
                }
                break;
            default:
                break;
        }
    }
}
