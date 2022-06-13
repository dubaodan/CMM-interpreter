#include <vector>
#include "headers/Mapping.h"
#include "headers/Symbol.h"

int Symbol2Int(const string& sym){
    if (sym == "#")
        return 101;
    //通过计算字符串长度来加速查找
    int len = sym.length();
    int begin = 0, end = 102;

    if (len < 1)
        return 0;
    else if (len >= 1 && len <= 5)
        end = 57;
    else if (len >= 6 && len <= 8){
        if(sym == "CONSTANT")
            return 99;
        else if(sym == "pointer")
            return 80;
        else{
            end = 13;
        }
    } else // len >= 9
        begin = 57;

    for (; begin < end; ++begin) {
        if (allSymbol[begin] == sym)
            return begin + 1;
    }
    cerr<<"error Symbol2Int "<<sym<<endl;
    return 0;
}

string Int2Symbol(int number){
    if (number == 101)
        return "#";
    if (number < 1 || number > 103){
        cerr<<"error Int2Symbol "<<number<<endl;
        return "";
    }
    return allSymbol[number-1];
}


int Production2Int(const string& Symbol, vector<string> production){
    int sCount = 0, pCount = 0;
    production.erase(production.begin());
    for (auto & iter1 : Reflect){
        if (iter1.first == Symbol){
            for (auto & iter2 : iter1.second->getProduction()){
                if (iter2 == production){
                    return sCount*1000 + pCount + 1;
                }
                pCount++;
            }
            cerr<<"error Production2Int "<<Symbol<<endl;
            for (auto & iter : production)
                cerr<<iter<<endl;
            return -1;
        }
        sCount++;
    }
    cerr<<"error Production2Int "<<Symbol<<endl;
    for (auto & iter : production)
        cerr<<iter<<endl;
    return -1;
}

/*
 * number：映射值
 * Symbol：用于接收非终结符
 *
 * return：产生式，未找到则返回空VECTOR
 */
vector<string> Int2Production(int number, string& Symbol){
    number--;
    int sCount = number/1000, pCount = number%1000;
    try {
        int i = 0, j = 0;
        for (auto & iter1 : Reflect) {
            if (i == sCount){
                for (auto & iter2 : iter1.second->getProduction()) {
                    if (j == pCount){
                        Symbol = iter1.first;
                        return iter2;
                    }
                    j++;
                }
            }
            i++;
        }
    } catch (exception& e) {
        cout<<e.what()<<endl;
    }
    cerr<<"error Int2Production "<<number<<endl;
    return vector<string>();
}
