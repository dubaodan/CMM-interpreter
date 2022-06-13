#ifndef GRAMMAR_TRANSLATION_H
#define GRAMMAR_TRANSLATION_H

#include <iostream>
#include "DataStruct.h"
using namespace std;

extern vector<Quad*> Quads; //四元式表
extern vector<symbol*> SymbolTable; //符号表
extern vector<String> vString; //字符串表
extern vector<Const> vConst;   //常量表
extern vector<Identifier> vIdentifier; //标识符表
extern vector<Func*> FunctionTable; //函数表

extern vector<symbol> functionSymbolTable; //函数的标识符表，用来在递归时压入调用
extern vector<int> paraIntStack;
extern vector<double> paraRealStack;
extern string paraSequence;
extern vector<int> returnJmps;
extern vector<Func*> FunctionUseTable;
/**
 * 产生四元式，将四元式放入四元式表中
 * @param operator_ 四元式的操作符
 * @param op1 第一个操作数，可以为空
 * @param op2 第二个操作数，可以为空
 * @param dest 第三个操作数（目标地址），可以为空（ret)
 */
void _emit(string operator_, symbol *op1 = nullptr, symbol *op2 = nullptr, symbol *dest = nullptr);

/**
 * 回填出口值，将list中指向的四元式jump地址修改为quad
 * @param list 需要回填的链
 * @param quad 回填值
 */
void backPatch(const set<int>& list, int quad);

/**
 * 合并两个set（链）
 * @param list1
 * @param list2
 * @return 合并之后的set
 */
set<int> merge(set<int> list1, set<int> list2);

/**
 * @details Mapping.h
 * @param number
 * @param Symbol
 * @return
 */
vector<string> Int2Production(int number, string& Symbol);

/**
 * 获得对应标识符的名称
 * @param node 语法分析过程中语法树节点
 * @return 名称
 */
string getIName(Node *node);

/**
 * 获得对应常量的值
 * @param node 语法分析过程中语法树节点
 * @return 值（字符串形式）
 */
string getCValue(Node *node);

/**
 * 获得对应字符串的内容
 * @param node 语法分析过程中语法树节点
 * @return 字符串内容
 */
string getSContent(Node *node);


///判断作用范围first是否包含second (first>=second 返回true)
bool isIncluded(const int *first, const int *second);

///判断两个作用域是否相同
bool isSame(const int *first, const int *second);

///获取两个作用范围深度的距离，越大说明越近，同名变量始终使用最近的一个
int scopeDistance(const int *first, const int *second);

///返回某一范围的深度
int depth(const int *first);

/**
 * 查找符号，在指定作用域下
 * @param name
 * @param level
 * @return
 */
symbol * FindSymbol(const string& name, int *level);

Func * FindFunc(const string& name);

bool AddSymbol(const string& name, int *level);

//弃用方法
bool AddISymbol(const string& name, int *level, int value);
bool AddRSymbol(const string& name, int *level, double value);
bool AddSSymbol(const string& name, int *level, string value);

symbol * AddTemp(string type = "temporary");

void setType(const string& type);

///错误码
static int terrno = 0;
void setErrno(int newErrno);
void pterror(int line = 0, const string& str = "");

///打印符号表
void printSymbolTable();
///打印指定范围的符号表
void printSymbolTableValue();
///打印除临时变量外的符号表
void printSymbolTableNoTemp();
///打印函数表
void printFunctionTable();

///打印生成的四元式
void printQuads();

///准备解释执行环境
void prepareEnv();

///返回值寄存器
extern symbol * register_rax ; // NOLINT(cert-err58-cpp)

void analyzeQuads();

#endif //GRAMMAR_TRANSLATION_H
