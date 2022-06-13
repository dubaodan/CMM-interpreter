#ifndef GRAMMAR_SIGN_H
#define GRAMMAR_SIGN_H

#include <set>
#include <vector>
#include <stack>
#include <map>
#include <string>
#include "Translation.h"
#include "Mapping.h"

using namespace std;

extern int currentScope[MaxDepth];
extern int scopeSequence[MaxDepth];
extern stack<int> M;
extern stack<int> N;
extern int nextQuad;
/**
 * 运行一次以求得所有的First集和Follow集
 */
void runFirst();

///符号类
class Symbol {
protected:
    set<string> first;
    set<string> follow;
    vector<vector<string>> production;

public:
    bool firstGiven{};
    bool followGiven{};
    set<string> First(const string& self);
    set<string> getFirst();
    set<string> Follow(const string& self);
    set<string> getFollow();
    vector<vector<string>> getProduction();
    virtual void translationAction(int number, Node * leftPart, vector<Node*> rightPart){ // NOLINT(performance-unnecessary-value-param)
        cerr<<"called NonterminalSymbol translationAction()"<<endl;
    };
};

//反射模拟
extern map<string, Symbol*> Reflect;

///终结符类
class TerminalSymbol: public Symbol{
public:
    vector<string> terminals;
    TerminalSymbol() {
        terminals = {
                "!","+","-","*","/","%","=","+=","-=","*=","/=","%=","++","--",
                "==","<>",">","<",">=","<=","&&","||",
                "{","}",",",".","[","]","(",")",";",":","?","!",
                "IF","ELSE","WHILE","DO","FOR","CONTINUE","BREAK","RETURN",
                "CHAR","INT","REAL","VOID",
                "IDENTIFIER","CONSTANT","STRING_LITERAL",
        };
    }
};

///非终结符类
class NonterminalSymbol: public Symbol{
public:
    NonterminalSymbol(){
        firstGiven = false;
        followGiven = false;
    }
};

class primary_expression: public NonterminalSymbol{
public:
    primary_expression(){
        production = {
                {"IDENTIFIER"},
                {"CONSTANT"},
                {"STRING_LITERAL"},
                {"(","expression",")"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override {
        string name;
        switch (number){
            case 0:
                name = getIName(rightPart.front());
                leftPart->place = FindSymbol(name,currentScope);
                if (leftPart->place == nullptr){
                    pterror(rightPart.front()->token->line, name);
                    exit(0);
                }
                if (!leftPart->place->initialized){
                    setErrno(5);
                    pterror(rightPart.front()->token->line, name);
                    exit(0);
                }
                leftPart->which = 'i';
                break;
            case 1:
                name = getCValue(rightPart.front());
                if (name.find('.') == name.npos){
                    leftPart->place = new symbol(name, "INT");
                    leftPart->place->iValue = atoi(name.c_str());
                } else {
                    leftPart->place = new symbol(name, "REAL");
                    leftPart->place->rValue = atof(name.c_str());
                }
                leftPart->which = 'c';
                break;
            case 2:
                leftPart->place = new symbol(getSContent(rightPart.front()), "string");
                leftPart->which = 's';
                break;
            case 3:
                //运算过程中使用了无返回值的函数
                if (rightPart.back()->place == nullptr || rightPart.front()->place == nullptr) {
                    setErrno(15);
                    pterror(rightPart[0]->token->line,"()");
                    exit(0);
                }
                //向上传递
                leftPart->place = (rightPart.front()++)->place;
                //leftPart->which = (rightPart.front()++)->which;
                leftPart->falseList = rightPart.front()->falseList;
                leftPart->trueList = rightPart.front()->trueList;
                break;
            default:
                cerr<<"primary_expression::translationAction() error"<<endl;
        }
    }
};

class postfix_expression: public NonterminalSymbol{
public:
    postfix_expression(){
        production = {
                {"primary_expression"},
                {"postfix_expression","[","expression","]"},
                {"postfix_expression","(",")"},
                {"postfix_expression","(","argument_expression_list",")"},
                {"postfix_expression",".","IDENTIFIER"},
                {"postfix_expression","*","IDENTIFIER"},
                {"postfix_expression","++"},
                {"postfix_expression","--"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        Func * func;
        switch (number){
            case 0:
                leftPart->place = rightPart.front()->place;
                leftPart->which = rightPart.front()->which;
                leftPart->falseList = rightPart.front()->falseList;
                leftPart->trueList = rightPart.front()->trueList;
                break;
            case 1:
                //TODO: need to be implemented
                //下标运算符只能作用于数组和指针类型
                if (rightPart.front()->place->size <= 0) {
                    setErrno(16);
                    pterror(rightPart[1]->token->line,"[");
                    exit(0);
                }
                leftPart->place = AddTemp();
                //指示该临时变量为一数组元素
                leftPart->place->arrayRef = rightPart.front()->place;
                leftPart->place->arrayIndexRef = rightPart[2]->place;
                _emit("=[]",rightPart.front()->place,rightPart[2]->place,leftPart->place);
                leftPart->which = 'i';
                break;
            case 2:
                //TODO: need to be implemented
                //函数调用必须为标识符
                if (rightPart.front()->children.front()->children.front()->token->id != 100 ) {
                    setErrno(14);
                    pterror(rightPart[1]->token->line,"(");
                    exit(0);
                }
                func = FindFunc(rightPart.front()->children.front()->place->name);
                //找不到函数名
                if (func == nullptr){
                    setErrno(13);
                    pterror(rightPart[1]->token->line,rightPart.front()->children.front()->place->name);
                    exit(0);
                }
                //参数数目不匹配
                if (!func->parameters.empty() ) {
                    setErrno(12);
                    pterror(rightPart[1]->token->line,func->name);
                    exit(0);
                }
                leftPart->which = 'i';

                //无返回值的函数调用
                if (func->returnType == "VOID"){
                    leftPart->place = nullptr;
                } else {
                    leftPart->place = register_rax;
                }
                //func在函数表中结构体的begin值对应了它在符号表中的位置
                _emit("call", nullptr, nullptr, func->begin);
                break;
            case 3:
                //TODO: need to be implemented
                if (rightPart.front()->children.front()->children.front()->token->id != 100 ) {
                    setErrno(14);
                    pterror(rightPart[1]->token->line,"(");
                    exit(0);
                }
                func = FindFunc(rightPart.front()->children.front()->place->name);
                if (func == nullptr){
                    setErrno(13);
                    pterror(rightPart[1]->token->line,rightPart.front()->children.front()->place->name);
                    exit(0);
                }
                //参数数目不匹配
                if (func->parameters.size() != rightPart[2]->parametersCount) {
                    setErrno(12);
                    pterror(rightPart[1]->token->line,func->name);
                    exit(0);
                }
                //（如果有）参数列表中的错误
                pterror(rightPart[1]->token->line,func->name);

                //无返回值的函数调用
                if (func->returnType == "VOID"){
                    leftPart->place = nullptr;
                } else {
                    leftPart->place = register_rax;
                }
                leftPart->which = 'i';
                //func在函数表中结构体的begin值对应了它在符号表中的位置
                _emit("call", nullptr, nullptr, func->begin);
                break;
            case 4:
                //TODO: need to be implemented
                break;
            case 5:
                //TODO: need to be implemented
                break;
            case 6:
                //对无返回值的函数调用结果进行运算
                if (rightPart.front()->place == nullptr) {
                    setErrno(15);
                    pterror(rightPart[1]->token->line,"++");
                    exit(0);
                }
                leftPart->place = rightPart.front()->place;
                leftPart->which = rightPart.front()->which;
                if (leftPart->which != 'i') {
                    setErrno(4);
                    pterror(rightPart.back()->token->line,"++");
                    exit(0);
                }
                _emit("++",leftPart->place);
                break;
            case 7:
                if (rightPart.front()->place == nullptr) {
                    setErrno(15);
                    pterror(rightPart[1]->token->line,"--");
                    exit(0);
                }
                leftPart->place = rightPart.front()->place;
                leftPart->which = rightPart.front()->which;
                if (leftPart->which !='i') {
                    setErrno(4);
                    pterror(rightPart.back()->token->line,"--");
                    exit(0);
                }
                _emit("--",leftPart->place);
                break;
            default:
                cerr<<"postfix_expression::translationAction() error"<<endl;
        }
    }
};

class argument_expression_list: public NonterminalSymbol{
public:
    argument_expression_list(){
        production = {

                /**
                 * argument_expression_list
                 *              |
                 *              a_e_l , a_e(参数2)
                 *                 |
                 *                 a_e(参数1)
                 * 参数1先规约，参数2后规约，按照规约顺序进队列，函数调用时出队列赋值
                 */

                {"assignment_expression"},
                {"argument_expression_list",",","assignment_expression"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        //TODO
        switch (number) {
            case 0:
                //必须是 assignment_expression -> conditional_expression
                if (rightPart.front()->children.size() != 1){
                    setErrno(14);
                    //错误向上抛出
                }
                leftPart->parametersCount = 1;
                _emit("push", nullptr, nullptr, rightPart.front()->place);
                break;
            case 1:
                if (rightPart.back()->children.size() != 1){
                    setErrno(14);
                    //错误向上抛出
                }
                leftPart->parametersCount = rightPart.front()->parametersCount + 1;
                _emit("push", nullptr, nullptr, rightPart.back()->place);
                break;
            default:
                cerr<<"argument_expression_list::translationAction() error"<<endl;
        }
    }
};

class unary_expression: public NonterminalSymbol{
public:
    unary_expression(){
        production = {
                {"postfix_expression"},
                {"unary_operator","unary_expression"},
                {"++","unary_expression"},
                {"--","unary_expression"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        switch (number){
            case 0:
                leftPart->place = rightPart.front()->place;
                leftPart->which = rightPart.front()->which;
                leftPart->falseList = rightPart.front()->falseList;
                leftPart->trueList = rightPart.front()->trueList;
                break;
            case 1:
                //调用了无返回值的函数，试图使用返回值
                if (rightPart.back()->place == nullptr) {
                    setErrno(15);
                    pterror(rightPart.front()->children.front()->token->line);
                    exit(0);
                }

                //不改变原变量值的一元运算符，用临时变量保存
                leftPart->which = rightPart.back()->which;
                leftPart->place = AddTemp();
                _emit({rightPart.front()->which}, rightPart.back()->place, nullptr, leftPart->place);
                break;
            case 2:
                if (rightPart.back()->place == nullptr) {
                    setErrno(15);
                    pterror(rightPart.front()->token->line);
                    exit(0);
                }
                leftPart->place = rightPart.back()->place;
                leftPart->which = rightPart.back()->which;
                if (leftPart->which != 'i') {
                    terrno = 4;
                    pterror(rightPart.back()->token->line,"++");
                    exit(0);
                }
                _emit("++",leftPart->place);
                break;
            case 3:
                if (rightPart.back()->place == nullptr) {
                    setErrno(15);
                    pterror(rightPart.front()->token->line);
                    exit(0);
                }
                leftPart->place = rightPart.back()->place;
                leftPart->which = rightPart.back()->which;
                if (leftPart->which != 'i') {
                    terrno = 4;
                    pterror(rightPart.back()->token->line,"--");
                    exit(0);
                }
                _emit("--",leftPart->place);
                break;
            default:
                cerr<<"unary_expression::translationAction() error"<<endl;
        }
    }
};

class unary_operator:public NonterminalSymbol{
public:
    unary_operator(){
        production = {
                {"+"},
                {"-"},
                {"*"},
                {"!"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        leftPart->which = Int2Symbol(rightPart.front()->token->id)[0];
    }
};

class multiplicative_expression:public NonterminalSymbol {
public:
    multiplicative_expression() {
        production = {
                {"unary_expression"},
                {"multiplicative_expression","*","unary_expression"},
                {"multiplicative_expression","/","unary_expression"},
                {"multiplicative_expression","%","unary_expression"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        //运算过程中使用了无返回值的函数
        if (number>0){
            if (rightPart.back()->place == nullptr || rightPart.front()->place == nullptr) {
                setErrno(15);
                pterror(rightPart[1]->token->line,Int2Symbol(rightPart[1]->token->id));
                exit(0);
            }
        }
        switch (number) {
            case 0:
                leftPart->place = rightPart.front()->place;
                leftPart->which = rightPart.front()->which;
                leftPart->falseList = rightPart.front()->falseList;
                leftPart->trueList = rightPart.front()->trueList;
                break;
            case 1:
                leftPart->place = AddTemp();
                leftPart->which = 'i';
                _emit("*",rightPart.front()->place,rightPart.back()->place,leftPart->place);
                break;
            case 2:
                leftPart->place = AddTemp();
                leftPart->which = 'i';
                _emit("/",rightPart.front()->place,rightPart.back()->place,leftPart->place);
                break;
            case 3:
                leftPart->place = AddTemp();
                leftPart->which = 'i';
                _emit("%",rightPart.front()->place,rightPart.back()->place,leftPart->place);
                break;
            default:
                cerr<<"multiplicative_expression::translationAction() error"<<endl;
        }
    }
};

class additive_expression:public NonterminalSymbol {
public:
    additive_expression() {
        production = {
                {"multiplicative_expression"},
                {"additive_expression","+","multiplicative_expression"},
                {"additive_expression","-","multiplicative_expression"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        //运算过程中使用了无返回值的函数
        if (number>0){
            if (rightPart.back()->place == nullptr || rightPart.front()->place == nullptr) {
                setErrno(15);
                pterror(rightPart[1]->token->line,Int2Symbol(rightPart[1]->token->id));
                exit(0);
            }
        }
        switch (number) {
            case 0:
                leftPart->place = rightPart.front()->place;
                leftPart->which = rightPart.front()->which;
                leftPart->falseList = rightPart.front()->falseList;
                leftPart->trueList = rightPart.front()->trueList;
                break;
            case 1:
                leftPart->place = AddTemp();
                leftPart->which = 'i';
                _emit("+",rightPart.front()->place,rightPart.back()->place,leftPart->place);
                break;
            case 2:
                leftPart->place = AddTemp();
                leftPart->which = 'i';
                _emit("-",rightPart.front()->place,rightPart.back()->place,leftPart->place);
                break;
            default:
                cerr<<"additive_expression::translationAction() error"<<endl;
        }
    }
};

class relational_expression:public NonterminalSymbol {
public:
    relational_expression() {
        production = {
                {"additive_expression"},
                {"relational_expression","<","additive_expression"},
                {"relational_expression",">","additive_expression"},
                {"relational_expression","<=","additive_expression"},
                {"relational_expression",">=","additive_expression"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        //运算过程中使用了无返回值的函数
        if (number>0){
            if (rightPart.back()->place == nullptr || rightPart.front()->place == nullptr) {
                setErrno(15);
                pterror(rightPart[1]->token->line,Int2Symbol(rightPart[1]->token->id));
                exit(0);
            }
        }
        switch (number) {
            case 0:
                leftPart->place = rightPart.front()->place;
                leftPart->which = rightPart.front()->which;
                leftPart->falseList = rightPart.front()->falseList;
                leftPart->trueList = rightPart.front()->trueList;
                break;
            case 1:
                //TODO: need which = 'b'(bool)?
                // (1<2)<3, 1<2<3 ... are invalid, both sides must be 'i' or 'c'(aka identifier or number value)
                if((rightPart.front()->which != 'i' && rightPart.front()->which != 'c')
                    || (rightPart.back()->which != 'i' && rightPart.back()->which != 'c')) {
                    setErrno(6);
                    pterror(rightPart[1]->token->line,"<");
                    exit(0);
                }
                leftPart->place = AddTemp();
                leftPart->which = 'b';
                //TODO: clear before insert?
                // leftPart->trueList.clear();
                leftPart->trueList.insert(nextQuad);
                leftPart->falseList.insert(nextQuad+1);
                _emit("j<",rightPart.front()->place,rightPart.back()->place);
                _emit("j");
                break;
            case 2:
                if((rightPart.front()->which != 'i' && rightPart.front()->which != 'c')
                   || (rightPart.back()->which != 'i' && rightPart.back()->which != 'c')) {
                    setErrno(6);
                    pterror(rightPart[1]->token->line,">");
                    exit(0);
                }
                leftPart->place = AddTemp();
                leftPart->which = 'b';
                // leftPart->trueList.clear();
                leftPart->trueList.insert(nextQuad);
                leftPart->falseList.insert(nextQuad+1);
                _emit("j>",rightPart.front()->place,rightPart.back()->place);
                _emit("j");

                break;
            case 3:
                if((rightPart.front()->which != 'i' && rightPart.front()->which != 'c')
                   || (rightPart.back()->which != 'i' && rightPart.back()->which != 'c')) {
                    setErrno(6);
                    pterror(rightPart[1]->token->line,"<=");
                    exit(0);
                }
                leftPart->place = AddTemp();
                leftPart->which = 'b';
                // leftPart->trueList.clear();
                leftPart->trueList.insert(nextQuad);
                leftPart->falseList.insert(nextQuad+1);
                _emit("j<=",rightPart.front()->place,rightPart.back()->place);
                _emit("j");
            case 4:
                if((rightPart.front()->which != 'i' && rightPart.front()->which != 'c')
                   || (rightPart.back()->which != 'i' && rightPart.back()->which != 'c')) {
                    setErrno(6);
                    pterror(rightPart[1]->token->line,">=");
                    exit(0);
                }
                leftPart->place = AddTemp();
                leftPart->which = 'b';
                // leftPart->trueList.clear();
                leftPart->trueList.insert(nextQuad);
                leftPart->falseList.insert(nextQuad+1);
                _emit("j>=",rightPart.front()->place,rightPart.back()->place);
                _emit("j");
                break;
            default:
                cerr<<"relational_expression::translationAction() error"<<endl;
        }
    }
};

class equality_expression:public NonterminalSymbol{
public:
    equality_expression(){
        production = {
                {"relational_expression"},
                {"equality_expression","==","relational_expression"},
                {"equality_expression","<>","relational_expression"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        //运算过程中使用了无返回值的函数
        if (number>0){
            if (rightPart.back()->place == nullptr || rightPart.front()->place == nullptr) {
                setErrno(15);
                pterror(rightPart[1]->token->line,Int2Symbol(rightPart[1]->token->id));
                exit(0);
            }
        }
        switch (number) {
            case 0:
                leftPart->place = rightPart.front()->place;
                leftPart->which = rightPart.front()->which;
                leftPart->falseList = rightPart.front()->falseList;
                leftPart->trueList = rightPart.front()->trueList;
                break;
            case 1:
                //TODO: need which = 'b'(bool)?
                // (1<2)<3, 1<2<3 ... are invalid, both sides must be 'i'(aka identifier or number value)
                if((rightPart.front()->which != 'i' && rightPart.front()->which != 'c')
                   || (rightPart.back()->which != 'i' && rightPart.back()->which != 'c')) {
                    setErrno(6);
                    pterror(rightPart[1]->token->line,"==");
                    exit(0);
                }
                leftPart->place = AddTemp();
                leftPart->which = 'b';
                //TODO: clear before insert?
                // leftPart->trueList.clear();
                leftPart->trueList.insert(nextQuad);
                leftPart->falseList.insert(nextQuad+1);
                _emit("j==",rightPart.front()->place,rightPart.back()->place);
                _emit("j");
                break;
            case 2:
                if((rightPart.front()->which != 'i' && rightPart.front()->which != 'c')
                   || (rightPart.back()->which != 'i' && rightPart.back()->which != 'c')) {
                    setErrno(6);
                    pterror(rightPart[1]->token->line,"<>");
                    exit(0);
                }
                leftPart->place = AddTemp();
                leftPart->which = 'b';
                // leftPart->trueList.clear();
                leftPart->trueList.insert(nextQuad);
                leftPart->falseList.insert(nextQuad+1);
                _emit("j<>",rightPart.front()->place,rightPart.back()->place);
                _emit("j");
                break;
            default:
                cerr<<"equality_expression::translationAction() error"<<endl;
        }
    }
};

class logical_and_expression:public NonterminalSymbol{
public:
    logical_and_expression(){
        production = {
                {"equality_expression"},
                // l_a_e && M e_e
                {"logical_and_expression","&&","equality_expression"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        //运算过程中使用了无返回值的函数
        if (number>0){
            if (rightPart.back()->place == nullptr || rightPart.front()->place == nullptr) {
                setErrno(15);
                pterror(rightPart[1]->token->line,Int2Symbol(rightPart[1]->token->id));
                exit(0);
            }
        }
        switch (number) {
            case 0:
                leftPart->place = rightPart.front()->place;
                leftPart->which = rightPart.front()->which;
                leftPart->falseList = rightPart.front()->falseList;
                leftPart->trueList = rightPart.front()->trueList;
                break;
            case 1:
                //TODO: need which = 'b'(bool)?
                leftPart->which = 'b';
                backPatch(rightPart.front()->trueList,M.top());
                M.pop();
                leftPart->place = rightPart.front()->place;
                leftPart->trueList = rightPart.back()->trueList;
                leftPart->falseList = merge(rightPart.front()->falseList,rightPart.back()->falseList);
                break;
            default:
                cerr<<"logical_and_expression::translationAction() error"<<endl;
        }
    }
};

class logical_or_expression:public NonterminalSymbol{
public:
    logical_or_expression(){
        production = {
                {"logical_and_expression"},
                // l_o_e || M l_a_e
                {"logical_or_expression","||","logical_and_expression"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        //运算过程中使用了无返回值的函数
        if (number>0){
            if (rightPart.back()->place == nullptr || rightPart.front()->place == nullptr) {
                setErrno(15);
                pterror(rightPart[1]->token->line,Int2Symbol(rightPart[1]->token->id));
                exit(0);
            }
        }
        switch (number) {
            case 0:
                leftPart->place = rightPart.front()->place;
                leftPart->which = rightPart.front()->which;
                leftPart->falseList = rightPart.front()->falseList;
                leftPart->trueList = rightPart.front()->trueList;
                break;
            case 1:
                //TODO: need which = 'b'(bool)?
                leftPart->which = 'b';
                backPatch(rightPart.front()->falseList,M.top());
                M.pop();
                leftPart->place = rightPart.front()->place;
                leftPart->trueList = merge(rightPart.front()->trueList,rightPart.back()->trueList);
                leftPart->falseList = rightPart.back()->falseList;
                break;
            default:
                cerr<<"logical_or_expression::translationAction() error"<<endl;
        }
    }
};

class conditional_expression:public NonterminalSymbol{
public:
    conditional_expression(){
        production = {
                {"logical_or_expression"},
                {"logical_or_expression","?","expression",":","conditional_expression"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        switch (number) {
            case 0:
                leftPart->place = rightPart.front()->place;
                leftPart->which = rightPart.front()->which;
                leftPart->falseList = rightPart.front()->falseList;
                leftPart->trueList = rightPart.front()->trueList;
                break;
            case 1:
                //TODO: not necessary (give up)
                break;
            default:
                cerr<<"conditional_expression::translationAction() error"<<endl;
        }
    }
};

class assignment_expression:public NonterminalSymbol{
public:
    assignment_expression(){
        production = {
                {"conditional_expression"},
                {"unary_expression","assignment_operator","assignment_expression"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        string ass_op = "=";
        switch (number) {
            case 0:
                //TODO: needed?(yes)
                leftPart->place = rightPart.front()->place;
                leftPart->which = rightPart.front()->which;
                leftPart->falseList = rightPart.front()->falseList;
                leftPart->trueList = rightPart.front()->trueList;
                break;
            case 1:
                //TODO: need passing?(maybe not)
                //运算过程中使用了无返回值的函数
                if (rightPart.back()->place == nullptr) {
                    setErrno(15);
                    pterror(rightPart[1]->children.front()->token->line,
                            {rightPart[1]->which});
                    exit(0);
                }
                if (rightPart.front()->children.size() != 1
                    && rightPart.front()->children[0]->children.size() != 1
                    && rightPart.front()->children[0]->children[0]->children[0]->which != 'i') {
                    setErrno(4);
                    pterror(rightPart[1]->token->line,Int2Symbol(rightPart[1]->token->id));
                    exit(0);
                }

                //数组元素赋值过程
                if (rightPart.front()->place->arrayRef != nullptr) {
                    ass_op[0]= rightPart[1]->which;
                    _emit(ass_op == "=" ? "[]=" :"[]" + ass_op + "=",rightPart.back()->place,
                          rightPart.front()->place->arrayIndexRef,rightPart.front()->place->arrayRef);
                    break;
                }
                ass_op[0]= rightPart[1]->which;
                _emit(ass_op == "=" ? ":=" :ass_op + "=",rightPart.back()->place,
                        nullptr,rightPart.front()->place);
                break;
            default:
                cerr<<"assignment_expression::translationAction() error"<<endl;
        }
    }
};

class assignment_operator:public NonterminalSymbol{
public:
    assignment_operator(){
        production = {
                {"="},
                {"*="},
                {"/="},
                {"%="},
                {"+="},
                {"-="}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        //TODO: need to be implemented
        leftPart->which = production[number][0][0];
    }
};

class expression:public NonterminalSymbol{
public:
    expression(){
        production = {
                {"assignment_expression"},
                {"expression",",", "assignment_expression"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        switch (number){
            case 0:
                leftPart->place = rightPart.front()->place;
                leftPart->which = rightPart.front()->which;
                leftPart->falseList = rightPart.front()->falseList;
                leftPart->trueList = rightPart.front()->trueList;
                break;
                //TODO: need to be implemented
            case 1:
                break;
            default:
                cerr<<"expression::translationAction() error"<<endl;
        }
    }
};

class declaration:public NonterminalSymbol{
public:
    declaration(){
        production = {
                {"declaration_specifiers", ";"},
                {"declaration_specifiers","init_declarator_list", ";"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        switch (number) {
            case 0:
                //TODO: need to be implemented. not necessary?
                break;
            case 1:
                //数组初始化
                if (rightPart[1]->size > 0) {
                    leftPart->place = rightPart.front()->place;
                    setType(leftPart->place->type+"[]");

                    if(rightPart[1]->size < rightPart[1]->initalizers.size()){
                        setErrno(18);
                        pterror(rightPart.back()->token->line,"{");
                        exit(0);
                    }


                    int count = 0;
                    for(auto & iter : rightPart[1]->initalizers) {
                        _emit("[]=",iter, nullptr,rightPart[1]->place);
                        Quads.back()->op2 = new symbol("","INT");
                        Quads.back()->op2->iValue = count;
                        count++;
                    }
                }
                leftPart->place = rightPart.front()->place;
                setType(leftPart->place->type);
                break;
            default:
                cerr<<"declaration::translationAction() error"<<endl;
        }
    }
};

class declaration_specifiers:public NonterminalSymbol{
public:
    declaration_specifiers(){
        production = {
                {"type_specifier"},
                {"type_specifier", "declaration_specifiers"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        switch (number) {
            case 0:
                leftPart->place = rightPart.front()->place;
                break;
            case 1:
                //TODO: need to be implemented. not necessary?
                break;
            default:
                cerr<<"declaration_specifiers::translationAction() error"<<endl;
        }
    }
};

class init_declarator_list:public NonterminalSymbol{
public:
    init_declarator_list(){
        production = {
                {"init_declarator"},
                {"init_declarator_list",",","init_declarator"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        //fixme:
        //仅实现了第一个产生式
        leftPart->size = rightPart.front()->size;
        leftPart->place = rightPart.front()->place;
        leftPart->initalizers = rightPart.front()->initalizers;
    }
};

class init_declarator:public NonterminalSymbol{
public:
    init_declarator(){
        production = {
                {"declarator"},
                {"declarator","=","initializer"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        switch (number) {
            case 0:
                leftPart->place = rightPart.front()->place;
                leftPart->which = rightPart.front()->which;
                leftPart->size = rightPart.front()->size;
                break;
            case 1:
                //初始化数组
                if (rightPart.front()->size > 0){
                    leftPart->size = rightPart.front()->size;
                    leftPart->place = rightPart.front()->place;
                    leftPart->place->initialized = true;
                    leftPart->initalizers = rightPart.back()->initalizers;
                    break;
                }
                //用没有返回值的函数进行初始化（出错）
                if (rightPart.back()->place == nullptr) {
                    setErrno(15);
                    pterror(rightPart[1]->token->line,"=");
                    exit(0);
                }
                leftPart->place = rightPart.front()->place;
                leftPart->place->initialized = true;
                _emit(":=",rightPart.back()->place, nullptr,leftPart->place);
                break;
            default:
                cerr<<"init_declarator::translationAction() error"<<endl;
        }
    }
};

class type_specifier:public NonterminalSymbol{
public:
    type_specifier(){
        production = {
                {"VOID"},
                {"CHAR"},
                {"INT"},
                {"REAL"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        leftPart->place = new symbol("",Int2Symbol(rightPart.front()->token->id));
    }
};

class declarator:public NonterminalSymbol{
public:
    declarator(){
        production = {
                {"pointer", "direct_declarator"},
                {"direct_declarator"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        switch (number) {
            case 0:
                //TODO: need to be implemented
                break;
            case 1:
                leftPart->place = rightPart.front()->place;
                leftPart->size = rightPart.front()->size;
                break;
            default:
                cerr<<"declarator::translationAction() error"<<endl;
        }
    }
};

///数组只支持一维
class direct_declarator:public NonterminalSymbol{
public:
    direct_declarator(){
        production = {
                {"IDENTIFIER"},
                {"(" ,"declarator", ")"},
                {"direct_declarator", "[","conditional_expression", "]"},
                {"direct_declarator", "[" ,"]"},
                {"direct_declarator", "(" ,"parameter_list", ")"},
                {"direct_declarator", "(" ,"identifier_list", ")"},
                {"direct_declarator", "(", ")"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        switch (number) {
            case 0:
                if (AddSymbol(getIName(rightPart.front()),currentScope)) {
                    leftPart->place = SymbolTable.back();
                } else {
                    pterror(rightPart.front()->token->line,getIName(rightPart.front()));
                    exit(0);
                }
                break;
            case 1:
                //TODO: need to be implemented
                break;
            case 2:
                //数组初始化大小必须为常量
                if (rightPart[2]->which != 'c'){
                    setErrno(21);
                    pterror(rightPart[1]->token->line,"[");
                    exit(0);
                }
                leftPart->place = rightPart.front()->place;

                //数组下标必须为整数
                if (rightPart[2]->place->name.find('.') != rightPart[2]->place->name.npos) {
                    setErrno(20);
                    pterror(rightPart[1]->token->line,"[");
                    exit(0);
                }
                leftPart->size = atoi(rightPart[2]->place->name.c_str());
                leftPart->place->size = leftPart->size;
                for (int i = 0;i < leftPart->place->size;++i) {
                    leftPart->place->iValues.emplace_back(0);
                    leftPart->place->rValues.emplace_back(0);
                }
                if (leftPart->size <=0){
                    setErrno(22);
                    pterror(rightPart[1]->token->line,"[");
                    exit(0);
                }
                break;
            case 3:
                //TODO: need to be implemented
                break;
            case 4: {
                auto func = FindSymbol(getIName(rightPart.front()->children.front()), currentScope);
                leftPart->place = func;
                func->initialized = true;
                func->type = "function";
                int newScope;
                int newDepth = 0;
                while (currentScope[newDepth] != 0) {
                    newDepth++;
                }
                //此处newDepth比实际要大1，因为做这一步规约时，左大括号已经准备读入，深度已经加1
                newDepth--;
                newScope = scopeSequence[newDepth];
                auto parameter = SymbolTable.rbegin();
                while ((*parameter)->type!= "function"){
                    (*parameter)->level[newDepth] = newScope;
                    (*parameter)->initialized = true;   //参数设置为已初始化的，可以直接在函数中使用
                    parameter++;
                }
                auto newFunc = new Func(nextQuad, func->name, func, nullptr, "",
                                        rightPart[2]->parameters);
                FunctionTable.emplace_back(newFunc);
                break;
            }
            case 5:
                //TODO: need to be implemented
                break;
            case 6: {
                auto func = FindSymbol(getIName(rightPart.front()->children.front()), currentScope);
                leftPart->place = func;
                func->initialized = true;
                func->type = "function";
                auto newFunc = new Func(nextQuad, func->name, func, nullptr, "");
                FunctionTable.emplace_back(newFunc);
                break;
            }
            default:
                cerr<<"direct_declarator::translationAction() error"<<endl;
        }
    }
};

class pointer:public NonterminalSymbol{
public:
    pointer(){
        production = {
                {"*"},
                {"*", "pointer"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{

    }
};


class parameter_list:public NonterminalSymbol{
public:
    parameter_list(){
        production = {
                {"parameter_declaration"},
                {"parameter_list", ",", "parameter_declaration"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        switch (number) {
            case 0:
                //参数
                leftPart->parameters.push_back(*rightPart.front()->parameters.begin());
                break;
            case 1:
                //TODO: need to be implemented?
                leftPart->parameters = rightPart.front()->parameters ;//参数
                leftPart->parameters.push_back(*rightPart.back()->parameters.begin());
                break;
            default:
                cerr<<"parameter_list::translationAction() error"<<endl;
        }
    }
};

class parameter_declaration:public NonterminalSymbol{
public:
    parameter_declaration(){
        production = {
                {"declaration_specifiers", "declarator"},
                {"declaration_specifiers"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        switch (number) {
            case 0:
                leftPart->parameters.push_back(rightPart.front()->place->type);
                setType(rightPart.front()->place->type);
                break;
            case 1:
                //TODO: need to be implemented?
                break;
            default:
                cerr<<"parameter_declaration::translationAction() error"<<endl;
        }
    }
};

class identifier_list:public NonterminalSymbol{
public:
    identifier_list(){
        production = {
                {"IDENTIFIER"},
                {"identifier_list",",","IDENTIFIER"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{

    }
};

///数组只支持一维
class initializer:public NonterminalSymbol{
public:
    initializer(){
        production = {
                {"assignment_expression"},
                {"{" ,"initializer_lists", "}"},
                {"{" ,"initializer_lists", ",","}"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        switch (number) {
            case 0:
                leftPart->place = rightPart.front()->place;
                break;
            case 1:
                leftPart->initalizers = rightPart[1]->initalizers;
                break;
            case 2:
                leftPart->initalizers = rightPart[1]->initalizers;
            default:
                cerr<<"initializer::translationAction() error"<<endl;
        }
    }
};

///数组只支持一维
class initializer_lists:public NonterminalSymbol{
public:
    initializer_lists(){
        production = {
                {"initializer"},
                {"initializer_lists", "," ,"initializer"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        switch (number) {
            case 0:
                //调用了无返回值的函数用于初始化
                if (rightPart.front()->place == nullptr){
                    setErrno(15);
                    pterror();
                    exit(0);
                }
                leftPart->initalizers.emplace_back(rightPart.front()->place);
                break;
            case 1:
                if (rightPart.back()->place == nullptr){
                    setErrno(15);
                    pterror(rightPart[1]->token->line);
                    exit(0);
                }
                leftPart->initalizers = rightPart.front()->initalizers;
                leftPart->initalizers.emplace_back(rightPart.back()->place);
                break;
            default:
                cerr<<"initializer_lists::translationAction() error"<<endl;
        }
    }
};

class statement:public NonterminalSymbol{
public:
    statement(){
        production = {
                {"compound_statement"},
                {"expression_statement"},
                {"selection_statement"},
                {"iteration_statement"},
                {"jump_statement"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        //TODO: check it (only necessary for expression_statement?)
        leftPart->nextList = rightPart.front()->nextList;
        leftPart->trueList = rightPart.front()->trueList;
        leftPart->falseList = rightPart.front()->falseList;
        switch (number){
            case 0:
                leftPart->breakList = rightPart.front()->breakList;
                leftPart->continueList = rightPart.front()->continueList;
                leftPart->returnList = rightPart.front()->returnList;
                break;
            case 1:
                //TODO: nothing to do?
                //leftPart->place = rightPart.front()->place;
                break;
            case 2:
                backPatch(leftPart->nextList,nextQuad);
                //选择语句不能处理流程控制语句（BREAK，CONTINUE）
                //将它们向上传递，寻找上层嵌套的循环语句（如果有，否则报错）
                leftPart->breakList = rightPart.front()->breakList;
                leftPart->continueList = rightPart.front()->continueList;
                leftPart->returnList = rightPart.front()->returnList;
                break;
            case 3:
                backPatch(leftPart->nextList,nextQuad);
                leftPart->returnList = rightPart.front()->returnList;
                break;
            case 4:
                leftPart->breakList = rightPart.front()->breakList;
                leftPart->continueList = rightPart.front()->continueList;
                leftPart->returnList = rightPart.front()->returnList;
                break;
            default:
                cerr<<"statement::translationAction() error"<<endl;
        }
    }
};

class compound_statement :public NonterminalSymbol {
public:
    compound_statement() {
        production = {
                { "{","}" },
                { "{","block_item_list" ,"}" }
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        switch (number){
            case 0:
                //TODO: have nothing to do with empty{}?
                //leftPart->nextList.insert(nextQuad);
                //leftPart->falseList = rightPart.front()->falseList;
                //leftPart->trueList = rightPart.front()->trueList;
                break;
            case 1:
                //TODO: check insert(merge) or set(clear then insert) (or same)
                //leftPart->nextList.insert(nextQuad);
                leftPart->falseList = rightPart[1]->falseList;
                leftPart->trueList = rightPart[1]->trueList;
                leftPart->breakList = rightPart[1]->breakList;
                leftPart->continueList = rightPart[1]->continueList;
                leftPart->returnList = rightPart[1]->returnList;
                leftPart->nextList = rightPart[1]->nextList;
                break;
            default:
                cerr<<"compound_statement::translationAction() error"<<endl;
        }
    }
};

class block_item :public NonterminalSymbol {
public:
    block_item() {
        production = {
                { "declaration" },
                { "statement" }
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        switch (number){
            case 0:
                //TODO: have nothing to do with declaration?
                //leftPart->nextList.insert(nextQuad);
                //leftPart->falseList = rightPart.front()->falseList;
                //leftPart->trueList = rightPart.front()->trueList;
                break;
            case 1:
                //TODO: check insert(merge) or set(clear then insert) (or same)
                //leftPart->nextList.insert(nextQuad);
                leftPart->falseList = rightPart.front()->falseList;
                leftPart->trueList = rightPart.front()->trueList;
                leftPart->breakList = rightPart.front()->breakList;
                leftPart->continueList = rightPart.front()->continueList;
                leftPart->returnList = rightPart.front()->returnList;

                break;
            default:
                cerr<<"block_item::translationAction() error"<<endl;
        }
    }
};

class block_item_list :public NonterminalSymbol {
public:
    block_item_list() {
        production = {
                { "block_item_list", "block_item"},
                { "block_item" }
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        switch (number){
            case 0:
                //TODO: how to handle consecutive block_item?
                // renew nextList?
                //leftPart->nextList.clear();
                //leftPart->nextList.insert(nextQuad);
                leftPart->breakList = merge(rightPart.front()->breakList,rightPart.back()->breakList);
                leftPart->continueList = merge(rightPart.front()->continueList,rightPart.back()->continueList);
                leftPart->returnList = merge(rightPart.front()->returnList,rightPart.back()->returnList);
//                leftPart->trueList = merge(rightPart.front()->trueList,rightPart.back()->trueList);
//                leftPart->falseList = merge(rightPart.front()->falseList,rightPart.back()->falseList);
//                leftPart->nextList = merge(rightPart.front()->nextList,rightPart.back()->nextList);
                break;
            case 1:
                //TODO: check insert(merge) or set(clear then insert) (or same)
                //leftPart->nextList.insert(nextQuad);
                leftPart->falseList = rightPart.front()->falseList;
                leftPart->trueList = rightPart.front()->trueList;
                leftPart->breakList = rightPart.front()->breakList;
                leftPart->continueList = rightPart.front()->continueList;
                leftPart->returnList = rightPart.front()->returnList;
                break;
            default:
                cerr<<"block_item_list::translationAction() error"<<endl;
        }
    }
};

class declaration_list:public NonterminalSymbol{
public:
    declaration_list(){
        production = {
                {"declaration"},
                {"declaration_list" ,"declaration"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{

    }
};


class expression_statement:public NonterminalSymbol{
public:
    expression_statement(){
        production = {
                {";"},
                {"expression",";"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        //TODO:check it
        leftPart->nextList.insert(nextQuad);
        switch (number){
            case 0:
                //TODO: need to be implemented
                //leftPart->nextList.insert(nextQuad);
                break;
            case 1:
                //leftPart->nextList.insert(nextQuad);
                leftPart->falseList = rightPart.front()->falseList;
                leftPart->trueList = rightPart.front()->trueList;
                break;
            default:
                cerr<<"expression_statement::translationAction() error"<<endl;
        }
    }
};

class selection_statement:public NonterminalSymbol{
public:
    selection_statement(){
        production = {
                // IF ( exp ) M stat
                {"IF", "(", "expression", ")" ,"statement"},
                // IF ( exp ) M stat N else M
                {"IF", "(", "expression", ")", "statement", "ELSE" ,"statement"}
                //{"SWITCH", "(", "expression", ")", "statement"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        switch (number){
            case 0:
                //if语句中不处理Break和Continue
                leftPart->breakList = rightPart.back()->breakList;
                leftPart->continueList = rightPart.back()->continueList;
                backPatch(rightPart[2]->trueList,M.top());
                M.pop();
                leftPart->nextList = merge(rightPart[2]->falseList,rightPart[4]->nextList);
                //向上传递返回语句链
                leftPart->returnList = rightPart.back()->returnList;
                break;
            case 1:
                leftPart->breakList = merge(rightPart.back()->breakList,rightPart[4]->breakList);
                leftPart->continueList = merge(rightPart.back()->continueList,rightPart[4]->breakList);
                backPatch(rightPart[2]->falseList,M.top());
                M.pop();
                backPatch(rightPart[2]->trueList,M.top());
                M.pop();
                leftPart->nextList = merge(rightPart[4]->nextList,rightPart.back()->nextList);
                leftPart->nextList.insert(N.top());
                N.pop();
                //向上传递返回语句链
                leftPart->returnList = merge(rightPart.back()->returnList,rightPart[4]->returnList);
                break;
            default:
                cerr<<"selection_statement::translationAction() error"<<endl;
        }
    }
};

class iteration_statement:public NonterminalSymbol{
public:
    iteration_statement(){
        production = {
                // while M (exp) M statement
                {"WHILE", "(" ,"expression", ")", "statement"},
                // do M statement while M (exp);
                {"DO" ,"statement", "WHILE", "(" ,"expression", ")", ";"},
                //fixme: deprecated?
                {"FOR", "(" ,"expression_statement" ,"expression_statement", ")", "statement"},
                // for ( exp M exp M exp ) N statement
                {"FOR", "(" ,"expression_statement", "expression_statement", "expression", ")" ,"statement"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        int M1;
        switch (number){
            case 0:
                backPatch(rightPart[2]->trueList,M.top());
                M.pop();
                M1 = M.top();
                backPatch(rightPart[4]->nextList,M1);
                backPatch(rightPart.back()->continueList,M1);
                M.pop();
                leftPart->nextList = rightPart[2]->falseList;
                leftPart->nextList = merge(leftPart->nextList,rightPart.back()->breakList);
                _emit("j");
                Quads.back()->jDest = M1;
                //向上传递返回语句链
                leftPart->returnList = rightPart.back()->returnList;
                break;
            case 1:
                backPatch(rightPart[1]->nextList, M.top());
                backPatch(rightPart[1]->continueList,M.top());
                M.pop();
                backPatch(rightPart[4]->trueList,M.top());
                M.pop();
                leftPart->nextList = rightPart[4]->falseList;
                leftPart->nextList = merge(leftPart->nextList,rightPart[1]->breakList);
                //向上传递返回语句链
                leftPart->returnList = rightPart[1]->returnList;
                break;
            case 2:
                //TODO: necessary?
                break;
            case 3:
                backPatch(rightPart[6]->nextList,M.top());
                backPatch(rightPart.back()->continueList,M.top());
                _emit("j");
                Quads.back()->jDest = M.top();
                M.pop();
                backPatch({N.top()},M.top());
                M.pop();
                backPatch(rightPart[3]->trueList,N.top()+1);
                N.pop();
                leftPart->nextList = rightPart[3]->falseList;
                leftPart->nextList = merge(leftPart->nextList,rightPart.back()->breakList);
                //向上传递返回语句链
                leftPart->returnList = rightPart.back()->returnList;
                break;
            default:
                cerr<<"iteration_statement::translationAction() error"<<endl;
        }
    }
};

class jump_statement:public NonterminalSymbol{
public:
    jump_statement(){
        production = {
                {"CONTINUE" ,";"},
                {"BREAK", ";"},
                {"RETURN", ";"},
                {"RETURN", "expression", ";"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        switch (number){
            case 0:
                leftPart->continueList.insert(nextQuad);
                _emit("j");
                break;
            case 1:
                leftPart->breakList.insert(nextQuad);
                _emit("j");
                break;
            case 2:
                leftPart->returnList.insert(nextQuad);
                _emit("ret");
                break;
            case 3:
                leftPart->returnList.insert(nextQuad);
                _emit("ret", nullptr, nullptr,rightPart[1]->place);
                break;
            default:
                cerr<<"jump_statement::translationAction() error"<<endl;
        }
    }
};

class translation_unit:public NonterminalSymbol{
public:
    translation_unit(){
        production = {
                {"external_declaration"},
                {"translation_unit", "external_declaration"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{

    }
};

class external_declaration:public NonterminalSymbol{
public:
    external_declaration(){
        production = {
                {"function_definition"},
                {"declaration"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{

    }
};

class function_definition:public NonterminalSymbol{
public:
    function_definition(){
        production = {
                {"declaration_specifiers","declarator","declaration_list" ,"compound_statement"},
                {"declaration_specifiers", "declarator", "compound_statement"}
        };
    }
    void translationAction(int number, Node * leftPart, vector<Node*> rightPart) override{
        string funcName;
        string type;
        Func * func;
        switch (number){
            case 0:
                //TODO:?
                break;
            case 1:
                if(!rightPart.back()->breakList.empty() || !rightPart.back()->continueList.empty()){
                    //函数中不允许存在孤立的Break和Continue
                    setErrno(7);
                    pterror(rightPart.front()->children.front()->children.front()->token->line,
                            rightPart.back()->breakList.empty()?"CONTINUE":"BREAK");
                    exit(0);
                }
                funcName = rightPart[1]->place->name;
                func = FindFunc(funcName);
                type = func->returnType = rightPart[0]->place->type;

                if(type == "VOID"){
                    //检查所有返回语句是否没有返回值，也可以没有返回语句
                    //返回值存在四元式的DEST中
                    for (auto & iter : rightPart.back()->returnList) {
                        if (Quads[iter]->dest != nullptr) {
                            setErrno(9);
                            pterror(rightPart.front()->children.front()->children.front()->token->line,
                                    "return");
                            exit(0);
                        }
                    }
                } else {
                    //检查所有返回语句是否有返回值，必须有返回语句
                    if (rightPart.back()->returnList.empty()) {
                        setErrno(10);
                        pterror(rightPart.front()->children.front()->children.front()->token->line,
                                "return");
                        exit(0);
                    }
                    for (auto & iter : rightPart.back()->returnList) {
                        if (Quads[iter]->dest == nullptr){
                            setErrno(8);
                            pterror(rightPart.front()->children.front()->children.front()->token->line,
                                    "return");
                            exit(0);
                        }
                    }
                }

                func->end = SymbolTable.back();
                func->exitPoint = nextQuad;
                _emit("ret");

                break;
            default:
                cerr<<"function_definition::translationAction() error"<<endl;
        }
    }
};

#endif //GRAMMAR_SIGN_H
