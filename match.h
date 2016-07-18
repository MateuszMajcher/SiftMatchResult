#pragma once
#ifndef MATCH_H
#define MATCH_H
#include<vector>
using namespace std;

class ImageMatch {
public:
	int i; //index obrazu query
	int i2; //index obrazu train
	float v; //podobienstwo
};

class Res {
public:
	int res;
	float val;
};

class Range{
public:
	Range(int size) {
		name.resize(size);
		count_query.resize(size);
		count_train.resize(size);
	}

	vector<string> name;
	vector<int> count_train;
	vector<int> count_query;
};

std::ostream& operator<<(std::ostream &strm, const ImageMatch &a)
{
	return strm <<"id query: "<< a.i << ", id train: " << a.i2 <<", v: "<<a.v;
}

std::ostream& operator<<(std::ostream &strm, const Res &a) {
	return strm << "res : " << a.res << ", val: : " << a.val;
}

vector<int> QueryRanges(int);
void getDataStat(string);
void checkModel(int, float);
vector<Res> GroupResult(vector<Res>);
int CheckClassQuery(vector<int>, int );
string ToModel(int);
Range QueryRangesTemp(int modelCount);


#endif // !MATCH_H

