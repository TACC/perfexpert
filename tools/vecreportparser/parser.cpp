/*
* AUTHOR: SREEDEVI SURENDRAN
*
*/

/* 

Perfexpert module for parsing intel vectorization reports (6 and 7) and embedding collated messages as comments to a copy of the original source code

Approach
a. Generate vec report 6 (with optimization level O3) for the given source file.
b. Generate vec report 7 (with optimization level O3) for the given source file.
c. Parse the reports from (a) and (b) and collate messages for every source line mentioned in both the reports. Note that for vec report 7, the messages will be translated to their 'English' forms.
d. For every source line mentioned in both the reports, embed the collated message from (c) as a comment into a copy of the original source file.
e. Print the consolidated report on stdout

Invocation
a. the current user should have read-write access to the current directory
b. remove <source_file_name>_out.<source_file_extension> (if any) from the current directory
c. remove fip1.txt (if any) from the current directory
d. remove fip2.txt (if any) from the current directory
c. run ./a.out <fully_qualified_source_file>

Input (Remarks)
name of the source file, for eg. main.c
The default optimization level is O3. Possible input source file formats are .c and .cpp.

Output (Remarks)
a. printed on stdout in the following format:
    File <filename>:Line <line_no>:<Category>:<Reasons>
	where 
		<Category> can be 'CONFLICT', 'VECTORIZED' or 'NOT VECTORIZED' 
		<Reasons> will be a "; delimited" string of all messages for line <line_no> from vec reports 6 and 7 (combined)
b. a new annotated source file, for eg. main_out.c created in the current directory

Temporary files created (in the current directory)
a. fip1.txt - vec report 6
b. fip2.txt - vec report 7

*/

#include <iostream>
#include <string>
#include <set>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <cstring>
#include <cstddef>
#include "stdlib.h"
#include <unistd.h>

#define vecReportLevel "-vec-report="
#define optimizationLevel "-O3"
#define flags " -c -O3 -g "
#define ccompiler "icc "
#define cpluspluscompiler "icpc "
#define vec7messagesFileName "vec7messages.txt"

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

void parseVecReport(string fileName,unordered_map<string,string>& htabVecMessages,unordered_map<string,set<string>>& htabLines,bool isVec6){
	string line;
	bool isMsg = false;
	ifstream infile(fileName);
	if(infile.is_open()){
		while(getline(infile,line)){
			if(line.find("error") != string::npos){
				cout<<endl<<line;
				infile.close();
				exit(0);
			}		
			else if(line.find("warning") != string::npos)
				continue;
			else{
              			vector<string> vecTemp = splitStringByDelimiter(line,':');
				if(vecTemp.size() < 3){
                  			if(!isMsg){
                                        	cerr<<endl<<endl<<"The following line(s) from the vec report have not been processed:"<<endl;
                                        	isMsg = true;
                                	}
                                	cerr<<endl<<line;				
				}
				else{
					string strFileName = vecTemp[0].substr(0,vecTemp[0].find_first_of("("));
					string strLineNb = vecTemp[0].substr(vecTemp[0].find("(")+1,vecTemp[0].find(")")-vecTemp[0].find("(")-1);
					string strKey = strFileName + ";" + strLineNb;
                                	string strReason;
                                	for(int j=2;j<vecTemp.size();j++){
						if(isVec6){
							strReason += trimWhiteSpaces(vecTemp[j]);
						}
						else{
							string trimmedString = trimWhiteSpaces(vecTemp[j]);
							vector<string> vecInner = splitStringByDelimiter(trimmedString,' ');
							if(vecInner.size() > 0){
								if(htabVecMessages.find(vecInner[0]) != htabVecMessages.end()){
									strReason += htabVecMessages.at(vecInner[0]);
									if(vecInner.size() == 2)
										strReason += vecInner[1];	
								}
								else{
									strReason += trimmedString;
								}
							}
						}
                                        	if(j==vecTemp.size()-1)
                                                	strReason+=";";
                                        	else
                                                	strReason+=" - ";
                                	}
					if(htabLines.find(strKey) != htabLines.end()){
						set<string> reasons = htabLines.at(strKey);
						if(reasons.find(strReason) == reasons.end()){
							reasons.insert(strReason);
						}
						htabLines.at(strKey) = reasons;
					}				
					else{
						set<string> reasons;
						reasons.insert(strReason);
						htabLines.insert(make_pair<string,set<string>>(strKey,reasons));
					}							
				}
			}	
		}
		infile.close();
	}		
}

string addCategory(set<string> reasons){
	string strReason="";
        for(set<string>::iterator itReasons=reasons.begin();itReasons!=reasons.end();itReasons++){
        	strReason+=*itReasons;
        }
        string out;
        transform(strReason.begin(),strReason.end(),std::back_inserter(out),::toupper);
        bool blNV = (out.find("LOOP WAS NOT VECTORIZED") != string::npos) ? true : false;
        bool blV = (out.find("LOOP WAS VECTORIZED") != string::npos) ? true : false;
        bool blPV = (out.find("PARTIAL LOOP WAS VECTORIZED") != string::npos) ? true : false;
        if((blNV && blV) || (blNV && blPV) || (blV && blPV))
        	strReason = "CONFLICT:" + strReason;
        else{
        	if(blPV)
                	strReason = "PARTIALLY VECTORIZED:" + strReason;
                else if(blV)
                	strReason = "VECTORIZED:" + strReason;
                else
                	strReason = "NOT VECTORIZED:" + strReason;
        }
        return strReason;	
}

int executeCommand(string cmd){
	//const char *c = cmd.c_str();
	//return system(c);
        
	FILE *pip = popen(cmd.c_str(),"r");
        if(pip == NULL)
                return -1;
        pclose(pip);
        pip = NULL;	
	return 0;
}

void display_on_console(set<string>& sourceFiles,unordered_map<string,set<string>>& htabLines){
	for(unordered_map<string,set<string>>::iterator it=htabLines.begin();it!=htabLines.end();it++){
                set<string> reasons = htabLines.at(it->first);
                int intIndexOfDelimiter = (it->first).find_first_of(";");
                string strFileName = (it->first).substr(0,intIndexOfDelimiter);
                string strLineNb = (it->first).substr(intIndexOfDelimiter+1,(it->first).size()-intIndexOfDelimiter-1);
                if(sourceFiles.find(strFileName) == sourceFiles.end()){
                        sourceFiles.insert(strFileName);
                }
                string strReason = addCategory(reasons);
                cout<<"File "<<strFileName<<":Line "<<strLineNb<<":"<<strReason;
                cout<<endl;
        }
}

void embed_in_soource_code(set<string>& sourceFiles,unordered_map<string,set<string>>& htabLines,int returnValue){
        for(set<string>::iterator it=sourceFiles.begin();it!=sourceFiles.end();it++){
                ifstream infile(*it);
                int intIndexOfDelimiter = (*it).find_first_of(".");
                string rawFileName = (*it).substr(0,intIndexOfDelimiter);
                string fileExtension = (*it).substr(intIndexOfDelimiter+1,(*it).size()-intIndexOfDelimiter-1);
                string tempOutputFileName = rawFileName + "_temp." + fileExtension;
                ofstream outfile(tempOutputFileName);
                int ct = 0;
                string line;
                if(infile.is_open()){
                        while(getline(infile,line)){
                                ct++;
                                string strCount = std::to_string(static_cast<long long>(ct));
                                string strKey = (*it) + ";" + strCount;
                                if(htabLines.find(strKey) != htabLines.end()){
                                        set<string> reasons = htabLines.at(strKey);
                                        string sourceLine = line + "/* " + addCategory(reasons) + " */";
                                        outfile<<sourceLine<<endl;
                                }
                                else{
                                        outfile<<line<<endl;
                                }

                        }
                        infile.close();
                        outfile.close();
                }
                string finalOutputFileName = rawFileName + "_out." + fileExtension;
                string cmdRemoveLineFeedCharacters = "sed -e 's/\r//g' " + tempOutputFileName + " > " + finalOutputFileName;
                returnValue = executeCommand(cmdRemoveLineFeedCharacters);
                returnValue = executeCommand("rm " + tempOutputFileName);
        }
}

bool populateVec7Messages(unordered_map<string,string>& htabVecMessages){

	ifstream infile(vec7messagesFileName);
	if(infile.is_open()){
		string line;
		while(getline(infile,line)){
			if(line.find(",") != string::npos){
				vector<string> messageLine = splitStringByDelimiter(line,',');
				if(messageLine.size() == 2){
					string key = trimWhiteSpaces(messageLine[0]);
					string value = messageLine[1];
					if(htabVecMessages.find(key) != htabVecMessages.end()){
						string temp = htabVecMessages.at(key);
						temp = temp + ";" + value;
						htabVecMessages.at(key) = temp;
					}
					else
						htabVecMessages.insert(make_pair<string,string>(key,value));
				}
			}
		}
		infile.close();
	}
	else
		return false;
	return true;
}

int main(int argc,char* argv[]){

	//if the command line is not of the form ./a.out <fully_qualified_source_file>, return
       if(argc < 2){
                cout<<endl<<"Enter filename:"<<endl;
                return 0;
        }

	//initialize message dictionary
	unordered_map<string,string> htabVecMessages;
	bool isVecMessagesPopulated = populateVec7Messages(htabVecMessages);
	if(!isVecMessagesPopulated){
		cout<<endl<<"/vec7messages.txt file not found"<<endl<<endl;
	}

	if(htabVecMessages.size() == 0){
		cout<<endl<<"Could not populate data from /vec7messages.txt file"<<endl<<endl;
	}

        //initialize variables
	vector<string> params(argv,argc+argv);
	string cmd = "";
	int returnValue;	
       	string out;
	unordered_map<string,set<string>> htabLines;

	//check if the source file is a .c or .cpp file to invoke the corresponding compiler
        transform(params[1].begin(),params[1].end(),back_inserter(out),::toupper);
        bool blC = (out.find(".C") != string::npos) ? true : false;
        bool blCPP = (out.find(".CPP") != string::npos) ? true : false;
 	out = "";	
	if(blC){
		out = ccompiler;
	}
	else if(blCPP)
		out = cpluspluscompiler;		

	//generate vec report 6 and write into fip1.txt
	cmd = out + params[1] + flags + vecReportLevel + "6 2> fip1.txt";
	returnValue = executeCommand(cmd);

	//generate vec report 7 and write into fip2.txt
	cmd = out + params[1] + flags + vecReportLevel + "7 2> fip2.txt";
	returnValue = executeCommand(cmd);

	//parse vec report 6
	parseVecReport("fip1.txt",htabVecMessages,htabLines,true);
	//parse vec report 7
	parseVecReport("fip2.txt",htabVecMessages,htabLines,false);	
	
	//if the vec reports 6 and 7 could not be parsed, return
	if(htabLines.size() == 0){
		cout<<endl<<"Unable to process vector reports"<<endl;
		cout<<endl<<"Please check /fip1.txt and /fip2.txt"<<endl<<endl;
		return 0;
	}

	//print to stdout
	set<string> sourceFiles;
	display_on_console(sourceFiles,htabLines);

	//embed in source code
	embed_in_soource_code(sourceFiles,htabLines,returnValue);

	cout<<endl;
	return(0);
 }
