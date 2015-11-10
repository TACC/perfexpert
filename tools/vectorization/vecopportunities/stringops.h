#include <iostream>
#include <vector>
#include <sstream>
#include <cstring>

using namespace std;

string trimWhiteSpaces(string str){
        string whitespaces(" \t\f\v\n\r");
        std::size_t found = str.find_first_not_of(whitespaces);
        if(found != string::npos)
                str = str.erase(0,found);
        found = str.find_last_not_of(whitespaces);
        if(found != string::npos)
                str = str.erase(found+1);
        return str;
}

vector<string> splitStringByDelimiter(string str,char delim){
        vector<string> vecTemp;
        istringstream partialString(str);
        string tempPartialLine;
        while(getline(partialString,tempPartialLine,delim)){
                vecTemp.push_back(tempPartialLine);
        }
        return vecTemp;
}

