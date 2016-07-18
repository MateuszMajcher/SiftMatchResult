#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include<algorithm>
#include "match.h"

vector<ImageMatch> match;

int bestMatch = 1000;  //liczba dopasowan
int classCount = 5; //liczba klas

int imagePerClassTrain = 80;
int imagePerClassQuery = 60;

int totalQueriesImages = 623;
int treshold = 0.04;

int groupCount = 20;  //liczba branych pod uwage podobnych obrazów

string matchesFileName = "matching_model5_2_500.txt";
string modelCountFileName = "modelCount.txt";
string modelCountFileNametemp = "modelCount1.txt";

bool stata = true;						//dla obiektów 
bool withStat = true;					//dla modeli 

int noObjCount = 0;						//liczba obiektów niesklasyfikowanych (dla budynktów, nie dotyczy modeli)


int   gcs[] = { 500, 1000 };
float treshs[] = { 0.00f, 0.5f, 0.6f };

vector<int> ranges;
Range range(classCount);

int main() {

	ranges = QueryRanges(classCount);
	//range = QueryRangesTemp(classCount);
	//pobranie obiektow
	getDataStat(matchesFileName);

	
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 3; j++) {
			checkModel(gcs[i], treshs[j]);
		}
	}

	return 0;
}



/*wczytanie pliku wynikowego match*/
void getDataStat(string filename) {
	match.resize(totalQueriesImages * bestMatch);

	ifstream ifs(filename);
	cout << "wczytywanie pilku wynikowego..." << endl;
	for (int i = 0; i < match.size(); i++) {
		ifs >> match[i].i;
		ifs >> match[i].i2;
		ifs >> match[i].v;
	}
	cout << "wczytano" << endl;
	ifs.close();
}

void checkModel(int gc, float tresh) {
	int groupCount = gc;			cout << "group Count " << groupCount << endl;
	float treshold = tresh;			cout << "treshold    " << treshold << endl;

	//GetDataStat(matchesFileName);
	//vector<int> ranges = QueryRanges(classCount);

	int bad = 0;
	int notDetected = 0;
	int good = 0;
	ofstream ofs2("badMatch.txt");
	for (int i = 0; i < totalQueriesImages; i++)
	{
		if (i == 4717)
		{
			int x = 2 + 2;
		}

		//dla testow
		vector<ImageMatch> tmp2; //tmp2.resize(bestMatch);
		for (int k = 0; k < bestMatch; k++)
			tmp2.push_back(match[i*bestMatch + k]);

		vector<Res> res;
		res.resize(groupCount);

		for (int k = 0; k < groupCount; k++)
		{
			res[k].res = (int)match[i*bestMatch + k].i2 / imagePerClassTrain;
			res[k].val = match[i*bestMatch + k].v;
		}

		vector<Res> tmp = GroupResult(res);

		int val1;														//klasa train
		int val2 = CheckClassQuery(ranges, match[i*bestMatch].i);		//klasa query 

		if (withStat)
			val1 = tmp[0].res;								//klasa query
		else
			val1 = res[0].res;

		if (tmp[0].val < treshold)
			notDetected++;
		else if (val1 != val2)
		{
			bad++;
			ofs2 << "{" << i << "} Powinno " << ToModel(val2) << "   ->  " << ToModel(val1) << endl;
			//ofs2 <<"{" << i << "} Powinno " << val2 << "   ->  " << val1 << endl;
			//cout << i * bestMatch << endl;
		}
		else
			good++;
	}

	ofs2.close();
	float ppsk = ((float)((totalQueriesImages - notDetected) - bad) / (totalQueriesImages - notDetected));
	float ns = ((float)(notDetected) / totalQueriesImages);
	cout << "Procent poprawnie sklasyfikowanych klasyfikacji: " << ppsk * 100 << endl;
	cout << "Nie sklasyfikowano: " << ns * 100 << endl;

	ofstream ofs("out.txt", ios::app);
	ofs << groupCount << " " << treshold << " " << ppsk << " " << ns << " " << ppsk *(1 - ns) << endl;
	ofs.close();
	

}

Range QueryRangesTemp(int modelCount) {
	int q = 0;
	int t = 0;
	Range count(classCount);
	ifstream ifs(modelCountFileNametemp);
	//pobranie liczby poszczegolnych marek
	if (ifs.is_open())
	{
		for (int i = 0; i < modelCount; i++) {
			ifs >> count.name[i];
			ifs >> count.count_query[i];
			ifs >> count.count_train[i];

			q += count.count_query[i];
			t += count.count_train[i];
			//cout << "model " << count[i].name << "query " << count[i].count_query << "train " << count[i].count_train << endl;
		}
		ifs.close();
	} else cout << "Unable to open file: " << modelCountFileNametemp << endl;;

	cout << "sum: " << q << " , " << t << endl;

	Range ranges(classCount);
	for (int i = 1; i < count.count_query.size(); i++) {
		int tmp_query = 0;
		int tmp_train = 0;
		for (int j = 0; j < i; j++) {
			tmp_query += count.count_query[j];
			tmp_train += count.count_train[j];
		}
		ranges.count_query[i - 1] = tmp_query;
		ranges.count_train[i - 1] = tmp_train;
		std::cout << "ranges: " << ranges.count_query[i - 1] << endl;
		std::cout << "train: " << ranges.count_train[i - 1] << endl;
	}


	return ranges;

}

vector<int> QueryRanges(int modelCount) {
	vector<int> count;
	count.resize(modelCount);
	ifstream ifs(modelCountFileName);
	//pobranie liczby poszczegolnych marek
	for (int i = 0; i < modelCount; i++) {
		ifs >> count[i];
	}
	ifs.close();


	vector<int> ranges;
	ranges.resize(count.size() - 1);
	for (int i = 1; i < count.size(); i++) {
		int tmp = 0;
		for (int j = 0; j < i; j++)
			tmp += count[j];
		ranges[i - 1] = tmp;
		std::cout << "ranges: " << ranges[i - 1] << endl;
	}


	return ranges;
}

vector<Res> GroupResult(vector<Res> res) {
	vector<Res> tmp;								//pomacniczy wektro przechowujacy pogrupowane wyniki 
	tmp.push_back(res[0]);							//dodnie pierwszego 

	for (int i = 1; i < res.size(); i++)			//przetwarzanie wszystkich pozycji (najlepszych)
	{
		bool isVal = false;							//pamietanie czy 
		int localIdx = -1;
		for (int j = 0; j < tmp.size(); j++)
			if (res[i].res == tmp[j].res)
			{
				isVal = true;
				localIdx = j;
			}

		if (!isVal)
			tmp.push_back(res[i]);
		else
		{
			tmp[localIdx].val += res[i].val;
		}

	}

	sort(tmp.begin(), tmp.end(), [](Res d1, Res d2) { return d1.val > d2.val; });
	return tmp;
}

int CheckClassQuery(vector<int> ranges, int idx) { //klasyfikacja na podstawie przedia³ów
	for (int i = 0; i < ranges.size(); i++) 
		if (idx < ranges[i])
			return i;
	
	if (idx > ranges[ranges.size() - 1]);
	return classCount;
}

string ToModel(int idx)
{
	if (idx == 0) return "a3_1";
	if (idx == 1) return "a3_2";
	if (idx == 2) return "a4_b5";
	if (idx == 3) return "astra_1";
	if (idx == 4) return "astra_2";
	if (idx == 5) return "astra_3";
	if (idx == 6) return "brava";
	if (idx == 7) return "corsa_b";
	if (idx == 8) return "corsa_c";
	if (idx == 9) return "corsa_c_fl";
	if (idx == 10) return "escort_mk7";
	if (idx == 11) return "fiesta_mk6";
	if (idx == 12) return "focus_c_max";
	if (idx == 13) return "focus_mk1";
	if (idx == 14) return "focus_mk1_fl";
	if (idx == 15) return "focus_mk2";
	if (idx == 16) return "fusion";
	if (idx == 17) return "golf_4";
	if (idx == 18) return "golf_5";
	if (idx == 19) return "golf_6";
	if (idx == 20) return "ibiza_3";
	if (idx == 21) return "insignia";
	if (idx == 22) return "leon_1";
	if (idx == 23) return "leon_2";
	if (idx == 24) return "mondeo_mk2";
	if (idx == 25) return "mondeo_mk3";
	if (idx == 26) return "mondeo_mk4";
	if (idx == 27) return "panda";
	if (idx == 28) return "passat_b5";
	if (idx == 29) return "passat_b5-fl";
	if (idx == 30) return "passat_b6";
	if (idx == 31) return "polo_3";
	if (idx == 32) return "punto_2";
	if (idx == 33) return "punto_2_fl";
	if (idx == 34) return "seicento";
	if (idx == 35) return "s-max";
	if (idx == 36) return "stilo";
	if (idx == 37) return "toledo_2";
	if (idx == 38) return "touran";
	if (idx == 39) return "vectra_b";
	if (idx == 40) return "vectra_c";
	if (idx == 41) return "vectra_GTS";
	if (idx == 42) return "vectra_GTS-fl";
	if (idx == 43) return "zafira_a";
	if (idx == 44) return "zafira_b";

	return 0;
}