#include <iostream>
#include "headers/Symbol.h"

set<string> Symbol::First(const string& self) {
    auto iter = Reflect.find(self);
    if (iter != Reflect.end()){
        if (iter->second->firstGiven)
            return iter->second->getFirst();
        else
            iter->second->firstGiven = true;
    }
    auto it = production.begin();
    while (it != production.end()){
        string target = *(it->begin());
        if (target == self){
            it++;
            continue;
        }
        iter = Reflect.find(target);
        if(iter == Reflect.end()){
            first.insert({target});
        } else {
            auto subFirst = iter->second->First(target);
            first.insert(subFirst.begin(),subFirst.end());
        }
        it++;
    }
    return first;
}

set<string> Symbol::getFirst() {
    return first;
}

set<string> Symbol::Follow(const string &self) {
    /*
     * 查找所有产生式中非终结符出现的位置，
     * 1、后面无符号，加入该产生式所属的非终结符的follow集
     * 2、后面为终结符，加入终结符
     * 3、后面为非终结符，加入该终结符的first集
     */

    auto iter = Reflect.find(self);
    if (iter != Reflect.end()){
        if (iter->second->followGiven)
            return iter->second->getFollow();
        else
            iter->second->followGiven = true;
    }

    auto iter0 = Reflect.begin();
    while (iter0 != Reflect.end()){
        auto iter1 = iter0->second->production.begin();
        while (iter1 != iter0->second->production.end()){
            auto iter2 = iter1->begin();
            while (iter2 != iter1->end()){
                string symbol = *(iter2);
                if (symbol == self){
                    if ((iter2+1) == iter1->end()){
                        auto subFollow = iter0->second->Follow(iter0->first);
                        follow.insert(subFollow.begin(),subFollow.end());
                    } else if (Reflect.find(*(iter2+1)) == Reflect.end()){
                        follow.insert(*(iter2+1));
                    } else {
                        auto subFollow = Reflect.find(*(iter2+1))->second->First(*(iter2+1));
                        follow.insert(subFollow.begin(),subFollow.end());
                    }
                }
                iter2++;
            }
            iter1++;
        }
        iter0++;
    }

    return follow;
}

set<string> Symbol::getFollow() {
    return follow;
}

vector<vector<string>> Symbol::getProduction() {
    return production;
}

void runFirst(){
    for (auto & iter : Reflect){
//        cout<<iter.first<<"\n\t\t";
        auto first = iter.second->First(iter.first);
//        auto it1 = first.begin();
//        while (it1 != first.end()){
//            cout<<*it1<<" ";
//            it1++;
//        }
//        cout<<endl<<"\t\t";
        auto follow = iter.second->Follow(iter.first);
//        auto it2 = follow.begin();
//        while (it2 != follow.end()){
//            cout<<*it2<<" ";
//            it2++;
//        }
//        cout<<endl<<endl;
    }
}
