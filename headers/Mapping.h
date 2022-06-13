#ifndef GRAMMAR_MAPPING_H
#define GRAMMAR_MAPPING_H

#include <iostream>
using namespace std;

/**
 * 用于符号与数字间映射，与词法分析产生的token序列相同
 * 1-15 保留字
 * 16-57 界符、运算符
 * 58-97 非终结符
 * 98 字符串常量
 * 99 常量
 * 100 标识符
 * (101 终结指示符 #)
 * (102 block_item_list 新增非终结符)
 */
const string allSymbol[102]={ // NOLINT(cert-err58-cpp)
        "BREAK",
        "CASE",
        "CHAR",
        "CONTINUE",
        "DEFAULT",
        "DO",
        "ELSE",
        "FOR",
        "IF",
        "INT",
        "REAL",
        "RETURN",
        "SWITCH",
        "VOID",
        "WHILE",
//-------------------------------------------
        "+",
        "-",
        "*",
        "/",
        "<",
        "<=",
        ">",
        ">=",
        "=",
        "==",
        "<>",
        ";",
        "(",
        ")",
        ",",
        "\"",
        "\'",
        "\\n",
        "\\r",
        "\\t",
        "++",
        "--",
        "&",
        "&&",
        "|",
        "||",
        "<<",
        ">>",
        "[",
        "]",
        "{",
        "}",
        "!",
        ":",
        "?",
        ".",
        "%",
        "+=",
        "-=",
        "*=",
        "/=",
        "%=",
//-------------------------------------------
        "primary_expression",
        "postfix_expression",
        "argument_expression_list",
        "unary_expression",
        "unary_operator",
        "multiplicative_expression",
        "additive_expression",
        "relational_expression",
        "equality_expression",
        "logical_and_expression",
        "logical_or_expression",
        "conditional_expression",
        "assignment_expression",
        "assignment_operator",
        "expression",
        "declaration",
        "declaration_specifiers",
        "init_declarator_list",
        "init_declarator",
        "type_specifier",
        "declarator",
        "direct_declarator",
        "pointer",
        "parameter_list",
        "parameter_declaration",
        "identifier_list",
        "initializer",
        "initializer_lists",
        "statement",
        "compound_statement",
        "declaration_list",
        "block_item",
        "expression_statement",
        "selection_statement",
        "iteration_statement",
        "jump_statement",
        "translation_unit",
        "translation_units",
        "external_declaration",
        "function_definition",
//-------------------------------------------
        "STRING_LITERAL",
        "CONSTANT",
        "IDENTIFIER",
        "#",
        "block_item_list"
};

/**
 * 将符号转换为映射值
 * @param sym 符号
 * @return 映射值，未能映射成功会在标准错误流中打印信息
 */
int Symbol2Int(const string& sym);

/**
 * 将映射值转换为对应的符号
 * @param number 映射值
 * @return 符号，未能映射成功会在标准错误流中打印信息
 */
string Int2Symbol(int number);


/**
 * 用于产生式与数字间映射
 * number = （Reflect中第几号非终结符）*1000 + （该非终结符中第几号产生式）
 */

 /**
  * 将产生式转为其对应的整形值
  * @param Symbol 非终结符
  * @param production 该非终结符的一条产生式右部
  * @return 映射值，若未查找到返回-1
  */
int Production2Int(const string& Symbol, vector<string> production);

 /**
  * 将映射值转为相应的产生式
  * @param number 映射值
  * @param Symbol 用于接收返回的非终结符
  * @return 产生式，未找到则返回空VECTOR
  */
vector<string> Int2Production(int number, string& Symbol);
#endif //GRAMMAR_MAPPING_H
