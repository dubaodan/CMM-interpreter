#pragma once

#include <set>
#include <vector>
#include <map>
#include <string>

using namespace std;

struct ProductionInSta
{
	vector<string> productions;
	int dot;
	set<string> followSet;

	bool operator==(ProductionInSta &P)const
	{
		if (dot != P.dot || productions != P.productions||followSet.size()!=P.followSet.size()) return false;
		
		for (const auto & iter : followSet)
			if (P.followSet.find(iter) == P.followSet.end())
				return false;
		return true;
	}
};

struct Status
{
    int number;
	vector<ProductionInSta> allProduction;
	map<string, Status*> nextStatus;
};
