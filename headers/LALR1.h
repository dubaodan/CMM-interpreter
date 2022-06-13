#ifndef GRAMMAR_LALR1_H
#define GRAMMAR_LALR1_H

#include <iostream>
#include "DataStruct.h"
#include "Status.h"
#include "Symbol.h"

#define TableLine 1000
#define TableColumn 103
#define TablePath "./analyse_table.txt"

extern TerminalSymbol t_e;
extern vector<Status> allStatus;
extern int statusTable[TableLine][TableColumn];
extern vector<Token> vToken;


void InitStatus(Status *&S);

void CompleteStatus(const string& symbol, const string& lookahead, set<string> preSet, Status *&S);

bool JudgeEqual(Status *&S);

void createTable();

///保存分析表
void saveTable();

///加载分析表
void loadTable();

//分析栈
extern stack<Node*> AnalyseStack;
//状态栈
extern stack<int> StatusStack;

///获取一个节点的内容对应的映射值
int getNodeNumber(Node *node);

///将一个Token打包成节点，用于从token序列中读入分析栈
Node* Token2Node(Token *token);

/**
 *  将规约得到的序列加入到语法树中
 *  同时执行相应的翻译动作
 *
 *  leftPart：产生式左部对应的序号
 *  rightPart：产生式右部节点序列（从分析栈中得出）
 *
 *  return：父节点（规约得到的非终结符）
 */
Node* generateTree(int leftPart, const vector<Node*>& rightPart);

///打印语法分析树
void printTree(Node *node);

void analyzeProgram();
bool analyzeTK(Node *nd);


/**
 *  在语法分析过程中动态地模拟空产生式规约，帮助语义分析进行
 *  if
 *  while
 *  for
 */
bool findLastIf();
bool findLastWhile();
void findLastFor();

#endif //GRAMMAR_LALR1_H
