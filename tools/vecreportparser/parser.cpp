/*
* AUTHOR: SREEDEVI SURENDRAN
*
*/

/* 

Perfexpert module for parsing intel vectorization reports (6 and 7) and embedding collated messages as comments to a copy of the original source code

Approach
a. Parse the vectorization reports 6 and 7 and collate messages for every source line mentioned in both the reports. Note that for vec report 7, the messages will be translated to their 'English' forms.
b. For every source line mentioned in both the reports, embed the collated message from (c) as a comment into a copy of the original source file.
c. Print the consolidated report on stdout

Command line arguments
a. -t : if set, indicates whether a temporary directory is to be created (within the current directory) for the annotated source file
b. -v6 : to be followed by the fully-qualified path for vectorization report 6
c. -v7 : to be followed by the fully-qualified path for vectorization report 7
d. -vm : to be followed by the fully-qualified path for vectorization report 7 translation dictionary 

Output (Remarks)
a. printed on stdout in the following format:
    File <filename>:Line <line_no>:<Category>:<Reasons>
	where 
		<Category> can be 'CONFLICT', 'VECTORIZED' or 'NOT VECTORIZED' 
		<Reasons> will be a "; delimited" string of all messages for line <line_no> from vec reports 6 and 7 (combined)
b. a new annotated source file, for eg. main_out.c created in the current directory or a temporary directory within the current directory (depends on whether -t flag is set)

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
#include <cerrno>
#include "stringops.h"
#include "systemops.h"

using namespace std;

void parseVecReport(ifstream& infile,unordered_map<string,string>& htabVecMessages,unordered_map<string,set<string>>& htabLines,bool isVec6){
	string line;
	bool isMsg = false;
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

void embed_in_source_code(set<string>& sourceFiles,unordered_map<string,set<string>>& htabLines,int returnValue,string& finalOutputFileName,bool& isPwdFound,bool isOutputTempDir){
	string pwd = get_working_path();
        isPwdFound = (!pwd.empty()?true:false);
	if(isOutputTempDir){
		char templ[] = ".vecreportparser-temp.XXXXXX";
		char* dirPtr = mkdtemp(templ);
		if(dirPtr)
			pwd = isPwdFound ? pwd + "/" + string(dirPtr) + "/" : string(dirPtr) + "/";
		else
			pwd = isPwdFound ? pwd + "/" : "";
	}
	else{
		pwd = isPwdFound ? pwd + "/" : "";
	}
        for(set<string>::iterator it=sourceFiles.begin();it!=sourceFiles.end();it++){
                ifstream infile(*it);
                int intIndexOfDelimiter = (*it).find_first_of(".");
                string rawFileName = (*it).substr(0,intIndexOfDelimiter);
                string fileExtension = (*it).substr(intIndexOfDelimiter+1,(*it).size()-intIndexOfDelimiter-1);
                string tempOutputFileName = pwd + rawFileName + "_temp." + fileExtension;
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

                //string finalOutputFileName;
                finalOutputFileName = pwd + rawFileName + "_out." + fileExtension;
		string cmdRemoveLineFeedCharacters = "sed -e 's/\r//g' " + tempOutputFileName + " > " + finalOutputFileName;
                string consoleOutput = "";
		returnValue = executeCommand(cmdRemoveLineFeedCharacters,consoleOutput,false);
                returnValue = executeCommand("rm " + tempOutputFileName,consoleOutput,false);
        }
}

void populateVec7Messages(ifstream& infile,unordered_map<string,string>& htabVecMessages){

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
}

string getOutputDirectory(string finalOutputFileName){
        
	size_t lastSlashIndex = finalOutputFileName.find_last_of("/");
        if(lastSlashIndex != string::npos){
        	return finalOutputFileName.substr(0,lastSlashIndex + 1);
        }
	return finalOutputFileName;

}

struct cmd_line_args{
private:	
	bool isOutputTempDir;
        string vecreport6FileName;
        string vecreport7FileName;
	string vec7messagesFileName;
public:        
	cmd_line_args():isOutputTempDir(false),vecreport6FileName(""),vecreport7FileName(""),vec7messagesFileName(""){}
        cmd_line_args(vector<string>& params){
        	isOutputTempDir = (std::find(params.begin(),params.end(),"-t") != params.end()) ? true : false;
                vecreport6FileName = getValue(params,"-v6");
                vecreport7FileName = getValue(params,"-v7");
		vec7messagesFileName = getValue(params,"-vm");
        }
        string getValue(vector<string>& params,string arg){
        	int pos = std::find(params.begin(),params.end(),arg) - params.begin();
                if(pos != (params.end() - params.begin())){
                	if(pos+1 < params.size())
                        	return params[pos+1];
                }
              	return "";
        }
	bool getIsOutputTempDir(){
		return isOutputTempDir;
	}
	string getVecreport6FileName(){
		return vecreport6FileName;
	}
	string getVecreport7FileName(){
		return vecreport7FileName;
	}
	string getVec7messagesFileName(){
		return vec7messagesFileName;
	}
};

int main(int argc,char* argv[]){

	vector<string> params(argv,argc+argv);
	cmd_line_args carg(params);

	if(carg.getVecreport6FileName().empty() && carg.getVecreport7FileName().empty()){
		cout<<endl<<"vec reports 6 and 7 not specified"<<endl;
		return 0;
	}

	if(carg.getVecreport6FileName().empty())
		cout<<endl<<"vec report 6 file not specified"<<endl;
        if(carg.getVecreport7FileName().empty())
                cout<<endl<<"vec report 7 file not specified"<<endl;
	else{
        	if(carg.getVec7messagesFileName().empty())
                	cout<<endl<<"vec 7 message dictionary file not specified"<<endl;
	}

	//initialize variables
	ifstream infile;
	unordered_map<string,string> htabVecMessages;
	string cmd = "";
	int returnValue;	
       	string out;
	unordered_map<string,set<string>> htabLines;

	//parse vec report 6
	if(!carg.getVecreport6FileName().empty()){
		if(isFileFound(carg.getVecreport6FileName(),infile)){
			//parse vec report 6 (if file "vecreport6.txt" can be found in the current directory)
			parseVecReport(infile,htabVecMessages,htabLines,true);
		}
		else{
			//if this file is not found, an error message is printed to the console, but processing continues
			cout<<endl<<"Could not open vec report 6"<<endl;
		}
	}

	//build htabVecMessages dictionary only if vec report 7 and vec 7 message dictionary have been specified
        if(!carg.getVecreport7FileName().empty() && !carg.getVec7messagesFileName().empty()){
                if(isFileFound(carg.getVec7messagesFileName(),infile))
                        populateVec7Messages(infile,htabVecMessages);
                else
                        cout<<endl<<"Could not open vec 7 message dictionary"<<endl;

                if(htabVecMessages.size() == 0){
                        cout<<endl<<"Could not populate data from vec 7 message dictionary"<<endl;
                }
        }

	//parse vec report 7
	if(!carg.getVecreport7FileName().empty()){
		if(isFileFound(carg.getVecreport7FileName(),infile)){
			//parse vec report 7 (if file "vecreport7.txt" can be found in the same directory)
			parseVecReport(infile,htabVecMessages,htabLines,false);
		}
		else{
			//if this file is not found, an error message is printed to the console, but processing continues
			cout<<endl<<"Could not open vec report 7"<<endl;
		}			
	}

	//if both vec reports 6 and 7 could not be parsed, return
	if(htabLines.size() == 0){
		cout<<endl<<"Unable to process vector reports"<<endl;
		cout<<endl<<"Please check vec reports 6 and 7"<<endl<<endl;
		return 0;
	}

	//print to stdout
	set<string> sourceFiles;
	display_on_console(sourceFiles,htabLines);

	//embed in source code
	string finalOutputFileName;
	bool isPwd = false;
	embed_in_source_code(sourceFiles,htabLines,returnValue,finalOutputFileName,isPwd,carg.getIsOutputTempDir());

	//display the directory name to which the annotated source file(s) are written
	if(isPwd){
		//if the current directory name was succesfully fetched
		cout<<endl<<"Output at: "<<getOutputDirectory(finalOutputFileName)<<endl;
	}
	else{
		//Unable to get the current directory name - possible buffer overflow - try increasing MAXPATHLEN
		cout<<endl<<"Output in the current directory at: "<<getOutputDirectory(finalOutputFileName)<<endl;
	}
		
	cout<<endl;
	return(0);
 }
