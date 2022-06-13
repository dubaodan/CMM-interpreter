#include <iostream>
#include "lexical.cpp"
#include "headers/LALR1.h"
#include "headers/PreDefined.h"

using namespace std;

/**
 * 语法分析使用的为自下而上LALR（1）
 * lexical.cpp  词法分析程序
 * Symbol.h     所有非终结符、终结符、产生式、语义分析动作
 * Mapping.h    将所有符号、所有产生式映射成整数
 * DataStruct.h 数据类型结构体：词法单元序列、标识符表、常数表、字符串表、四元式、符号表、函数表、语法树节点
 * PreDefined.h CMM预定义函数
 * Status.h     LR（1）项目结构体
 * LALR1.h      自下而上LALR（1）语法分析，包括生成预测分析表、总控程序、生成语法树
 * Translation.h    进行语义分析，包括生成四元式及对四元式进行翻译
 */

map<string,Symbol*> Reflect;    //根据类名获取类的对象

vector<Token> vToken;   //Token序列
vector<String> vString; //字符串表
vector<Const> vConst;   //常量表
vector<Identifier> vIdentifier; //标识符表
stack<Node*> AnalyseStack; //分析栈
stack<int> StatusStack; //状态栈

vector<Quad*> Quads;    //四元组
vector<symbol*> SymbolTable;    //符号表
vector<Func*> FunctionTable;    //函数表
symbol * register_rax;  //返回值寄存

vector<symbol> functionSymbolTable; //函数的标识符表，用来在递归时压入调用
vector<int> paraIntStack;   //参数传递时的整数参数栈
vector<double> paraRealStack;   //参数传递时的浮点数参数栈
string paraSequence;    //记录参数类型的序列
vector<int> returnJmps; //记录函数返回后的地址
vector<Func*> FunctionUseTable; //记录调用过程中的函数

vector<Status> allStatus;   //LR（1）项目集族
TerminalSymbol t_e; // NOLINT(cert-err58-cpp) 终结符类
int statusTable[TableLine][TableColumn];    //LALR（1）分析表

void addFunc(){
    int i = 0;
    for (const string& s : preDefinedFunc)
        vIdentifier.emplace_back(Identifier(s,++i));
}

int main() {
    primary_expression pr_e;
    postfix_expression po_e;
    argument_expression_list a_e_l;
    unary_expression u_e;
    unary_operator u_o;
    multiplicative_expression m_e;
    additive_expression add_e;
    relational_expression rel_e;
    equality_expression equ_e;
    logical_and_expression l_a_e;
    logical_or_expression l_o_e;
    conditional_expression c_e;
    assignment_expression ass_e;
    assignment_operator ass_o;
    expression exp;
    declaration dec;
    declaration_specifiers dec_s;
    init_declarator_list i_d_l;
    init_declarator i_d;
    type_specifier t_s;
    declarator declarator;
    direct_declarator d_d;
    pointer poi;
    parameter_list p_l;
    parameter_declaration p_d;
    identifier_list i_l;
    initializer init;
    initializer_lists init_l;
    statement stat;
    compound_statement c_stat;
    block_item b_i;
    block_item_list b_i_l;
    declaration_list d_l;
    expression_statement e_s;
    selection_statement s_s;
    iteration_statement i_s;
    jump_statement j_s;
    translation_unit t_u;
    external_declaration e_d;
    function_definition f_d;

    Reflect = {
            {"primary_expression",&pr_e},
            {"postfix_expression",&po_e},
            {"argument_expression_list", &a_e_l},
            {"unary_expression",&u_e},
            {"unary_operator",&u_o},
            {"multiplicative_expression",&m_e},
            {"additive_expression",&add_e},
            {"relational_expression",&rel_e},
            {"equality_expression",&equ_e},
            {"logical_and_expression",&l_a_e},
            {"logical_or_expression",&l_o_e},
            {"conditional_expression",&c_e},
            {"assignment_expression",&ass_e},
            {"assignment_operator",&ass_o},
            {"expression",&exp},
            {"declaration",&dec},
            {"declaration_specifiers",&dec_s},
            {"init_declarator_list",&i_d_l},
            {"init_declarator",&i_d},
            {"type_specifier",&t_s},
            {"declarator",&declarator},
            {"direct_declarator",&d_d},
            {"pointer",&poi},
            {"parameter_list",&p_l},
            {"parameter_declaration",&p_d},
            {"identifier_list",&i_l},
            {"initializer",&init},
            {"initializer_lists",&init_l},
            {"statement",&stat},
            {"compound_statement",&c_stat},
            {"block_item",&b_i},
            {"block_item_list",&b_i_l},
            {"declaration_list",&d_l},
            {"expression_statement",&e_s},
            {"selection_statement",&s_s},
            {"iteration_statement",&i_s},
            {"jump_statement",&j_s},
            {"translation_unit",&t_u},
            {"external_declaration",&e_d},
            {"function_definition",&f_d}

    };

    runFirst(); // 运行一次得到FIRST集和FOLLOW集
    Status *S;
    InitStatus(S);

    vConst.emplace_back("",0);
    vIdentifier.emplace_back("",0);
    addFunc();
    vString.emplace_back("",0);

    int anyError = Lexical();
    if (anyError > 0){
        cerr<<anyError<<"error(s) occurred in lexical analysis."<<endl;
        return 0;
    } else
        cout<<"lexcial analyse success."<<endl;

    analyzeProgram();

    printTree(AnalyseStack.top());
    printSymbolTableNoTemp();
    printQuads();
    printFunctionTable();
    analyzeQuads();
    printSymbolTableNoTemp();

//    for (auto & itt : vToken)
//        cout<<itt.line<<"  "<<itt.id<<"  "<<itt.index<<endl;
//    for (auto & itc : vConst)
//        cout<<itc.index<<"  "<<itc.value<<endl;
//    for (auto & iti : vIdentifier)
//        cout<<iti.index<<"  "<<iti.name<<endl;
//    for (auto & its : vString)
//        cout<<its.index<<"  "<<its.content<<endl;

    return 0;
}
