#include <utility>
#include <fstream>
#include "headers/LALR1.h"

using namespace std;

int currentScope[MaxDepth]; //记录当前作用域范围
int scopeSequence[MaxDepth];    //记录各深度的作用域标号
stack<int> M;   //用于模拟控制语句中的空产生式，保存 next quad 的值
stack<int> N;

int counts = 0;

//循环递归求一个状态中的所有产生式
void CompleteStatus(const string& symbol, const string& lookahead, set<string> preSet, Status *&S)
{
    //首先得出它应该添加的向前看符号是什么，如果是终结符，直接return
    auto iter1 = Reflect.find(symbol);
    if (iter1 == Reflect.end())return;
    if (!lookahead.empty())
    {
        auto iterAhead = Reflect.find(lookahead);
        if (iterAhead != Reflect.end())
        {
            preSet.clear();
            preSet = iterAhead->second->getFirst();
        }
        else
        {
            preSet.clear();
            preSet.insert({lookahead});
        }
    }
    //此处进行遍历，然后递归求出该状态下所有的产生式及其向前看符号
    for (int i = 0; i < iter1->second->getProduction().size(); i++)
    {
        bool m = true;
        bool fl = false;
        vector<string> iter2 = iter1->second->getProduction()[i];
        auto *tempP = new ProductionInSta();
        tempP->productions.emplace_back(symbol);
        tempP->productions.insert(tempP->productions.end(), iter2.begin(), iter2.end());
        //for循环中寻找是否有相同产生式，若有则直接加入向前看符号
        for (auto & iter3 : S->allProduction)
        {
            if (tempP->productions.size() == iter3.productions.size()&& tempP->productions == iter3.productions && iter3.dot == 1)
            {
                //处理向前看符号和左递归，如果已经包含则直接跳过，防止左递归
                fl = true;
                set<string>::iterator it;
                set<string>::iterator it1;
                if (iter3.followSet.size() >= preSet.size())
                {
                    for (it1 = preSet.begin(); it1 != preSet.end(); it1++)
                    {
                        if (iter3.followSet.find(*it1) == iter3.followSet.end())
                        {
                            fl = false;
                        }
                    }
                }
                else fl = false;
                iter3.followSet.insert(preSet.begin(), preSet.end());
                tempP->dot = 1;
                tempP->followSet = iter3.followSet;
                m = false;
                break;
            }
        }
        if (fl) continue;
        if (m)//没有相同产生式则加入该产生式
        {
            tempP->dot = 1;
            tempP->followSet = preSet;
        }
        //处理完该产生式后，查找后续产生式
        string tempSymbol;
        string tempLookahead;
        set<string> tempStr = tempP->followSet;
        //判断dot后面的符号是不是非终结符
        if (Reflect.find(tempP->productions[tempP->dot]) != Reflect.end())
        {
            tempSymbol = tempP->productions[tempP->dot];
            //找出向前看符号
            if (tempP->productions.size() >= 3)
            {
                tempLookahead = tempP->productions[2];
            }
            if (m)S->allProduction.emplace_back(*tempP);
            CompleteStatus(tempSymbol, tempLookahead, tempP->followSet, S);
        }
            //如果是终结符则直接添加
        else
        {
            if (m)S->allProduction.emplace_back(*tempP);
        }
        delete tempP;
        //tempP->productions.resize();
    }
}

void FindAllStatus()
{
    //首先遍历所有非终结符，查看dot位置是否有非终结符
    for (auto & iter : Reflect)
    {
        auto *Snext = new Status();
        string dotNow;
        bool m = false;
        for (auto & iter1 : allStatus[counts].allProduction)
        {
            auto *P = new ProductionInSta();
            if (iter1.dot < iter1.productions.size())
            {
                if (iter1.productions[iter1.dot] == iter.first)
                {
                    P->dot = iter1.dot + 1;
                    P->productions = iter1.productions;
                    P->followSet = iter1.followSet;
                    Snext->allProduction.emplace_back(*P);
                    dotNow = iter.first;
                    m = true;
                }
            }
        }
        //调用Complete函数求出Snext中的所有产生式
        auto *Stemp = new Status;
        for (auto & iter2 : Snext->allProduction)
        {
            auto *Ptemp = new ProductionInSta();
            Ptemp->productions = iter2.productions;
            Ptemp->dot = iter2.dot;
            Ptemp->followSet = iter2.followSet;
            Stemp->allProduction.emplace_back(*Ptemp);
        }
        for (auto & iter2 : Stemp->allProduction)
        {
            if (iter2.dot == iter2.productions.size() - 1)
            {
                CompleteStatus(iter2.productions[iter2.dot], "", iter2.followSet, Snext);
            }
            else if (iter2.dot < iter2.productions.size() - 1)
            {
                CompleteStatus(iter2.productions[iter2.dot], iter2.productions[iter2.dot + 1], iter2.followSet, Snext);
            }
        }
        //将Snext加入到所有的产生式，以及S的后续节点
        if (m)
        {
            if (JudgeEqual(Snext))
            {
                allStatus[counts].nextStatus.insert(pair<string, Status*>(dotNow, Snext));
            }
            else
            {
                allStatus[counts].nextStatus.insert(pair<string, Status*>(dotNow, Snext));
                //递归调用该函数求出Snext的所有next
                Snext->number = (int)allStatus.size() + 1;
                allStatus.emplace_back(*Snext);
            }
        }
    }
    //再遍历所有的终结符，查看dot后是否跟的是非终结符
    for (vector<string>::const_iterator iter = t_e.terminals.begin(); iter != t_e.terminals.end(); iter++)
    {
        auto *Snext = new Status();
        string dotNow;
        bool m = false;
        for (auto & iter1 : allStatus[counts].allProduction)
        {
            auto *P = new ProductionInSta();
            if (iter1.dot < iter1.productions.size())
            {
                if (iter1.productions[iter1.dot] == *iter)
                {
                    P->dot = iter1.dot + 1;
                    P->productions = iter1.productions;
                    P->followSet = iter1.followSet;
                    Snext->allProduction.emplace_back(*P);
                    dotNow = *iter;
                    m = true;
                }
            }
        }
        //调用Complete函数求出Snext中的所有产生式
        auto *Stemp = new Status();
        for (auto & iter2 : Snext->allProduction)
        {
            auto *Ptemp = new ProductionInSta();
            Ptemp->productions = iter2.productions;
            Ptemp->dot = iter2.dot;
            Ptemp->followSet = iter2.followSet;
            Stemp->allProduction.emplace_back(*Ptemp);
        }
        for (auto & iter2 : Stemp->allProduction)
        {
            if (iter2.dot == iter2.productions.size() - 1)
            {
                CompleteStatus(iter2.productions[iter2.dot], "", iter2.followSet, Snext);
            }
            else if (iter2.dot < iter2.productions.size() - 1)
            {
                CompleteStatus(iter2.productions[iter2.dot], iter2.productions[iter2.dot + 1], iter2.followSet, Snext);
            }
        }
        //将Snext加入到所有的产生式，以及S的后续节点
        if (m)
        {
            if (JudgeEqual(Snext))
            {
                allStatus[counts].nextStatus.insert(pair<string, Status*>(dotNow, Snext));
            }
            else
            {
                allStatus[counts].nextStatus.insert(pair<string, Status*>(dotNow, Snext));
                //递归调用该函数求出Snext的所有next
                Snext->number = (int)allStatus.size() + 1;
                allStatus.emplace_back(*Snext);
            }
        }
    }
}

//判断新生成的项目集簇是否在原来的里面有相同状态
bool JudgeEqual(Status *&S)
{
    for (auto & allStatu : allStatus)
    {
        bool p = true;
        if (S->allProduction.size() == allStatu.allProduction.size())
        {
            for (auto & iter1 : S->allProduction)
            {
                bool m = false;
                for (auto & iter2 : allStatu.allProduction)
                {
                    if (iter1 == iter2)
                    {
                        m = true;
                    }
                }
                if (!m)
                {
                    p = false;
                    break;
                }
            }
            if (p)
            {
                *S = allStatu;
                return true;
            }
        }
    }
    return false;
}

void createTable()
{
    //首先存入所有终结符和非终结符;
    statusTable[0][0] = 0;
    for (int i = 1; i < TableColumn; i++)statusTable[0][i] = i;
    counts = 0;
    while (counts < allStatus.size())
    {
        statusTable[counts + 1][0] = counts + 1;
        //先处理所有的移进项
        for (auto iter1 = allStatus[counts].nextStatus.begin(); iter1 != allStatus[counts].nextStatus.end(); iter1++)
        {
            for (int i = 1; i < TableColumn; i++)
            {
                string tempSymbol = Int2Symbol(i);
                if (!tempSymbol.empty()) {
                    if (tempSymbol == iter1->first)
                    {
                        if (statusTable[counts + 1][i] != 0)
                            cerr<<"conflict "<<(counts +1) <<"\t"<< i<<"\t"<<statusTable[counts + 1][i]<<"\t"<<iter1->second->number<<endl;
                        statusTable[counts + 1][i] = iter1->second->number;
                    }
                }
            }

        }
        //再处理所有的规约项
        counts++;
    }
    counts = 0;
    while (counts < allStatus.size())
    {
        for (auto iter1 = allStatus[counts].allProduction.begin(); iter1 != allStatus[counts].allProduction.end(); iter1++)
        {
            if (iter1->dot == iter1->productions.size())
            {
                for (auto iter2 = iter1->followSet.begin(); iter2 != iter1->followSet.end(); iter2++)
                {
                    int numberNow = Symbol2Int(*iter2);

                    if (numberNow != 0)
                    {
                        int numberNow1 = Production2Int(iter1->productions[0], iter1->productions);
                        if (iter1->productions[0] == "translation_units") statusTable[counts + 1][numberNow] = -999;
                        else {
                            if (statusTable[counts + 1][numberNow] != 0)
                                cerr<<"conflict "<<(counts +1) <<"\t"<< numberNow<<"\t"<<statusTable[counts + 1][numberNow]<<"\t"<<-numberNow1<<endl;
                            statusTable[counts + 1][numberNow] = -numberNow1;
                        }
                    }
                }
            }
        }
        counts++;
    }
}

//初始化所有状态集簇
void InitStatus(Status *&S)
{
    char answer;
    cout<<"读入已保存的状态表？（y/n）" ;
    cin>>answer;
    if(answer == 'y') {
        loadTable();
        return;
    }

    S = new Status();
    auto *P = new ProductionInSta();
    vector<string> tempPro = { "translation_units","translation_unit" };

//    vector<string> tempPro = { "H","S" };
    P->productions = tempPro;
    P->dot = 1;
    P->followSet.insert("#");
    S->allProduction.emplace_back(*P);
    CompleteStatus("translation_unit", "", P->followSet, S);

//    CompleteStatus("S", "", P->followSet, S);
    allStatus.emplace_back(*S);
    while (counts < allStatus.size())
    {
        FindAllStatus();
        counts++;
    }
    createTable();

    cout<<"保存（覆盖）状态表？（y/n）" ;
    cin>>answer;
    if(answer == 'y')
        saveTable();
}

int getNodeNumber(Node *node) {
    return node->token->id;
}

Node* Token2Node(Token *token) {
    return new Node(token);
}

Node* generateTree(int leftPart, const vector<Node*>& rightPart) {
    //执行语义分析的类
    auto E = Reflect.find(Int2Symbol(leftPart))->second;
    int action = 0;

    vector<string> production;
    production.reserve(rightPart.size());
    for (auto & iter : rightPart)
        production.emplace_back(Int2Symbol(iter->token->id));

    auto left = new Node(leftPart,rightPart);
    for (auto & iter : E->getProduction()){
        if (iter == production) {
            //调用相应非终结符的产生式对应的语义规则
            E->translationAction(action, left, rightPart);
            return left;
        }
        action++;
    }

    return new Node(leftPart,rightPart);
}

//打印语法树
void printTree(Node *node, int height){
    string shift;
    for (int i = 0; i < height; ++i) {
        shift += '\t';
    }
    int number = getNodeNumber(node);
    switch (number){
        case 100:
            cout<<shift<<Int2Symbol(number)<<": "<<vIdentifier[node->token->index].name<<endl;
            break;
        case 99:
            cout<<shift<<Int2Symbol(number)<<": "<<vConst[node->token->index].value<<endl;
            break;
        case 98:
            cout<<shift<<Int2Symbol(number)<<": "<<vString[node->token->index].content<<endl;
            break;
        default:
            cout<<shift<<Int2Symbol(number)<<endl;
    }
    for (auto & iter : node->children){
        printTree(iter,height + 1);
    }
}

void printTree(Node *node) {
    printTree(node,0);
}

//保存分析表
void saveTable() {
    ofstream analyseTable(TablePath,ios::out|ios::trunc);
    if (!analyseTable.is_open()) {
        cerr << "can't open this file" << endl;
        return;
    }
    for (auto &i : statusTable){
        for (int j : i)
            analyseTable<<j<<"\t";
        analyseTable<<endl;
    }
    cout<<"save success."<<endl;
    analyseTable.close();
}
//加载分析表
void loadTable() {
    ifstream analyseTable(TablePath);
    if (!analyseTable.is_open()) {
        cerr << "can't open this file" << endl;
        return;
    }
    string temp;
    int i = 0 ,j = 0;
    for (; i < TableLine && getline(analyseTable,temp); ++i) {
        char * sEnd = (char*)temp.c_str();
        for (j = 0; j < TableColumn && sEnd != nullptr; ++j) {
            statusTable[i][j] = (int)strtol(sEnd, &sEnd,0);
        }
        if (j != TableColumn)
            cerr<<"error loading table"<<i<<","<<j<<endl;
    }
    if (i != TableLine)
        cerr<<"error loading table"<<i<<","<<j<<endl;
    else
        cout<<"load complete."<<endl;
    analyseTable.close();
}

//语法分析总控程序
void analyzeProgram()
{
    vector<Token> tempStack;
    StatusStack.push(1);

    prepareEnv();

    currentScope[0] = 1;    //初始化全局作用域
    scopeSequence[0] = 1;
    int depth = 0;
    for (auto & iter1 : vToken) {
        string nextSymbol = Int2Symbol(iter1.id);
        if (nextSymbol == "{") {    //深度加深一层，该层序号值为上一个序号+1
            depth++;
            currentScope[depth] = ++scopeSequence[depth];
        } else if (nextSymbol == "}") { //深度变浅一层，回到上一层中
            currentScope[depth] = 0;
            depth--;
        }
        if (!AnalyseStack.empty()
            && ((AnalyseStack.top()->token->id == 39)  // &&
                || (AnalyseStack.top()->token->id == 41)    // ||
                || (AnalyseStack.top()->token->id == 6) // do
                || (AnalyseStack.top()->token->id == 15))){ //while
            M.push(nextQuad);
        } else if (nextSymbol == "ELSE") {
            N.push(nextQuad);
            _emit("j");
            M.push(nextQuad);
            Quads.back()->jDest = 0;
        } else if (findLastIf()) {
            M.push(nextQuad);
        } else if (findLastWhile() && nextSymbol != ";") {
            M.push(nextQuad);
        } else {
            findLastFor();
        }

        Node *nodeNow = Token2Node(&iter1);
        //此处默认token中的id就是mapping中的id
        if (!analyzeTK(nodeNow))
        {
            switch (getNodeNumber(nodeNow)){
                case 100:
                    break;
                case 99:
                    cout<<"index"<<nodeNow->token->index<<endl;
                    break;
                case 98:
                    break;
            }
            cout << "语法分析出现错误，错误在" << iter1.line << "行，出现错误的符号是" << Int2Symbol(iter1.id)<< endl;
            return;
        }
    }
}

bool analyzeTK(Node *nd)
{
    bool pushed = false;
    int statNow = StatusStack.top();
    int statJmp = statusTable[statNow][getNodeNumber(nd)];
    if (statJmp == 0)
        return false;
    if (statJmp > 0)
    {
        pushed = true;
        StatusStack.push(statJmp);
//        cout<<"Push:"<<statJmp<<"\t";
        AnalyseStack.push(nd);
//        cout<< Int2Symbol(getNodeNumber(nd))<<endl;
    }
    else
    {
        if (statusTable[statNow][getNodeNumber(nd)] == -999)
        {
            cout << "acc" << endl;
            return true;
        }
        vector<string> tempStr;
        Node *ntNode;
        vector<Node*> tempNodes;
        string tempSymbol;
        int generateSymbol;
        tempStr = Int2Production(-statusTable[statNow][getNodeNumber(nd)], tempSymbol);
        generateSymbol = Symbol2Int(tempSymbol);
        for (auto iter2 = tempStr.rbegin(); iter2 != tempStr.rend(); iter2++)
        {
            int tempSymbolNum = Symbol2Int(*iter2);
            if (getNodeNumber(AnalyseStack.top()) == tempSymbolNum)
            {
                tempNodes.insert(tempNodes.begin(), AnalyseStack.top());
                StatusStack.pop();
                AnalyseStack.pop();
            }
            else
                return false;
        }
        ntNode = generateTree(generateSymbol, tempNodes);

        if (!analyzeTK(ntNode))return false;
    }
    if (!pushed)
        analyzeTK(nd);
    return true;
}

//fixme: need optimization
bool findLastIf() {
    // IF ( exp ) ^then^ {...}，在即将读入 { 前，找到何时出现when （确定空产生式的位置，用于地址回填）
    // IF ( exp ) M stat N else M
    // IF ( exp ) M stat
    if (AnalyseStack.size()<4)
        return false;
    auto right = AnalyseStack.top(); AnalyseStack.pop();
    auto exp = AnalyseStack.top(); AnalyseStack.pop();
    auto left = AnalyseStack.top(); AnalyseStack.pop();
    if (Int2Symbol(right->token->id) == ")" &&
        Int2Symbol(exp->token->id) == "expression" &&
        Int2Symbol(left->token->id) == "(" &&
            Int2Symbol(AnalyseStack.top()->token->id) == "IF") {
        AnalyseStack.push(left);
        AnalyseStack.push(exp);
        AnalyseStack.push(right);
        return true;
    }
    AnalyseStack.push(left);
    AnalyseStack.push(exp);
    AnalyseStack.push(right);
    return false;
}

//fixme: need optimization
bool findLastWhile() {
    // While ( exp ) ^do^ {...}，在即将读入 { 前，找到何时出现do  （确定空产生式的位置，用于地址回填）
    // while M (exp) M statement
    if (AnalyseStack.size()<4)
        return false;
    auto right = AnalyseStack.top();AnalyseStack.pop();
    auto exp = AnalyseStack.top(); AnalyseStack.pop();
    auto left = AnalyseStack.top(); AnalyseStack.pop();
    if (Int2Symbol(right->token->id) == ")" &&
        Int2Symbol(exp->token->id) == "expression" &&
        Int2Symbol(left->token->id) == "(" &&
        Int2Symbol(AnalyseStack.top()->token->id) == "WHILE") {
        AnalyseStack.push(left);
        AnalyseStack.push(exp);
        AnalyseStack.push(right);
        return true;
    }
    AnalyseStack.push(left);
    AnalyseStack.push(exp);
    AnalyseStack.push(right);
    return false;
}

//fixme: need optimization
void findLastFor() {
    // for ( exp_stat M exp_stat M exp ) N statement ,确定M和N（确定空产生式的位置，用于地址回填）
    int size = AnalyseStack.size();
    if (size < 4)
        return;
    // 当将要插入M1时，此时栈中应为 "for ( exp ;" (exp;在下一步中才会被规约成exp_stat)
    // 当将要插入M2时，此时栈中应为 "for ( exp_stat exp ;" (exp;在下一步中才会被规约成exp_stat)
    auto semicolon = AnalyseStack.top();AnalyseStack.pop();
    auto exp = AnalyseStack.top();AnalyseStack.pop();
    if (semicolon->token->id == 27 && exp->token->id == 72) {   //exp
        auto exp_stat1 = AnalyseStack.top();AnalyseStack.pop();
        if (exp_stat1->token->id == 90) {   //exp_stat
            if (size < 5) {
                AnalyseStack.push(exp_stat1);
                AnalyseStack.push(exp);
                AnalyseStack.push(semicolon);
                return;
            }
            auto left = AnalyseStack.top();AnalyseStack.pop();
            if (left->token->id == 28) {
                auto for_ = AnalyseStack.top();
                if (for_->token->id == 8) {
                    M.push(nextQuad);
                }
            }
            AnalyseStack.push(left);
            AnalyseStack.push(exp_stat1);
            AnalyseStack.push(exp);
            AnalyseStack.push(semicolon);
            return;
        } else if (exp_stat1->token->id == 28) {    // (
            auto for_ = AnalyseStack.top();
            if (for_->token->id == 8) {
                M.push(nextQuad);
            }
        }
        AnalyseStack.push(exp_stat1);
        AnalyseStack.push(exp);
        AnalyseStack.push(semicolon);
        return;
    } else if (semicolon->token->id == 29 && size > 5) {
        auto exp_stat2 = AnalyseStack.top();AnalyseStack.pop();
        auto exp_stat3 = AnalyseStack.top();AnalyseStack.pop();
        auto left = AnalyseStack.top();AnalyseStack.pop();
        auto for_ = AnalyseStack.top();
        if (exp->token->id == 72
            && exp_stat2->token->id == 90
            && exp_stat3->token->id == 90
            && left->token->id ==28
            && for_->token->id == 8) {
            N.push(nextQuad);
            _emit("j");
        }
        AnalyseStack.push(left);
        AnalyseStack.push(exp_stat3);
        AnalyseStack.push(exp_stat2);
        AnalyseStack.push(exp);
        AnalyseStack.push(semicolon);
        return;
    }
    AnalyseStack.push(exp);
    AnalyseStack.push(semicolon);
}
