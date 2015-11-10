/*
* AUTHOR: SREEDEVI SURENDRAN
* The University of Texas at Austin
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
#include "vectorops.h"
#include <cmath>

#define runMacpoAlignment "--macpo:check-alignment="
#define runMacpoPrefix "macpo.sh "
#define perfExpertExp "perfexpert_run_exp ./"
#define perfExpertAnalyzer "perfexpert_analyzer -t 0.1 -i "
#define flags " -g -O3 -c "
#define macpoPrefix "[macpo]"
#define make "make -C "
#define macpoDefaultBatchSize "5"

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
	cout<<endl;
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

set<string> invokeMacpoStaticHelper(string command,vector<string>& hotloops_notorpartvec,unordered_map<string,string>& macpoStaticAnalysisOutput){
	set<string> macpoStaticAnalysis;
        string result = "";
	command += " 2>&1";
        if(executeCommand(command,result,true) == 0){
		if(result != ""){
        		for(auto it=hotloops_notorpartvec.begin();it!=hotloops_notorpartvec.end();it++){
                		macpoStaticAnalysisOutput.insert(make_pair<string,string>(*it,""));
        		}
			vector<string> resultLines = splitStringByDelimiter(result,'\n');		
			for(auto it=resultLines.begin();it!=resultLines.end();it++){
				if((*it).find(string(macpoPrefix)) != string::npos){
					for(auto iu=macpoStaticAnalysisOutput.begin();iu!=macpoStaticAnalysisOutput.end();iu++){
						if(iu->second == ""){
							if((*it).find(iu->first) != string::npos){
								macpoStaticAnalysisOutput[iu->first] = *it;
							}
						}
					}
				}
			}
		}
        }

        for(auto it=macpoStaticAnalysisOutput.begin();it!=macpoStaticAnalysisOutput.end();it++){
        	string out;
                transform((it->second).begin(),(it->second).end(),back_inserter(out),::toupper);
                if(out.find("[MACPO] ANALYZED CODE AT") != string::npos)
			macpoStaticAnalysis.insert(it->first);
        }

	return macpoStaticAnalysis;
}

bool getHotLoops(string executableName,string executableArguments,set<string>& hotLoops){

        string result = "";
        string xmlFileName = "";
	string perfexpertCmd = string(perfExpertExp) + executableName + " " + executableArguments;
        int ret = executeCommand(perfexpertCmd,result,true);
        //if(executeCommand(perfexpertCmd,result,true) == 0){
        //perfexpert currently doesn't return 0 on successful execution
        //this will not matter when vecopps becomes a perfexpert module
        if(!result.empty()){
                vector<string> result_by_line = splitStringByDelimiter(result,'\n');
                for(int i=0;i<result_by_line.size();i++){
                        string out;
                        transform(result_by_line[i].begin(),result_by_line[i].end(),back_inserter(out),::toupper);
                        if(out.find("PERFEXPERT_ANALYZER") != string::npos){
                                vector<string> xmlFileLine = splitStringByDelimiter(result_by_line[i],' ');
                                if(xmlFileLine.size() >= 4)
                                        xmlFileName = xmlFileLine[3];
                                break;
                        }
                }
                if(xmlFileName != ""){
                        result = "";
                        if(executeCommand(string(perfExpertAnalyzer) + xmlFileName,result,true) == 0){
                                vector<string> perfExpertOutput = splitStringByDelimiter(result,'\n');
                                for(auto ii=perfExpertOutput.begin();ii!=perfExpertOutput.end();ii++){
                                        string out;
                                        transform((*ii).begin(),(*ii).end(),back_inserter(out),::toupper);
                                        if(out.find("LOOP IN FUNCTION") != string::npos){
                                                vector<string> hotLoopParts = splitStringByDelimiter(*ii,' ');
                                                for(int i=0;i<hotLoopParts.size();i++){
                                                        if((hotLoopParts[i].find(":") != string::npos) && (hotLoopParts[i].find("::") == string::npos))
                                                                hotLoops.insert(hotLoopParts[i]);
                                                }
                                        }
                                }
                        }
                        else
                                return false;
                }
                else
                        return false;
        }
        else
                return false;
        return true;
}

unordered_map<string,string> getNoVecPartVec(unordered_map<string,set<string>>& htabLines){
        unordered_map<string,string> noVecPartVec;
        for(unordered_map<string,set<string>>::iterator it=htabLines.begin();it!=htabLines.end();it++){
                set<string> reasons = htabLines.at(it->first);
                int intIndexOfDelimiter = (it->first).find_first_of(";");
                string strFileName = (it->first).substr(0,intIndexOfDelimiter);
                string strLineNb = (it->first).substr(intIndexOfDelimiter+1,(it->first).size()-intIndexOfDelimiter-1);
                string strReason = addCategory(reasons);
                if(strReason.substr(0,14) == "NOT VECTORIZED"){
                        if(strFileName.find("/") != string::npos){
                                size_t indexOfLastSlash = strFileName.find_last_of("/");
                                noVecPartVec.insert(make_pair<string,string>(strFileName.substr(indexOfLastSlash + 1) + ":" + strLineNb,strFileName));
                        }
                        else
                                noVecPartVec.insert(make_pair<string,string>(strFileName + ":" + strLineNb,strFileName));
                }
                else if(strReason.substr(0,8) == "CONFLICT"){
                        string out;
                        transform(strReason.begin(),strReason.end(),back_inserter(out),::toupper);
                        if(out.find("PARTIAL LOOP WAS VECTORIZED") != string::npos){
                                if(strFileName.find("/") != string::npos){
                                        size_t indexOfLastSlash = strFileName.find_last_of("/");
                                        noVecPartVec.insert(make_pair<string,string>(strFileName.substr(indexOfLastSlash + 1) + ":" + strLineNb,strFileName));
                                }
                                else
                                        noVecPartVec.insert(make_pair<string,string>(strFileName + ":" + strLineNb,strFileName));
                        }
                }
        }
        return noVecPartVec;
}

set<string> invokeMacpoStatic(vector<string>& hotloops_notorpartvec,unordered_map<string,string>& macpoStaticAnalysisOutput,string makeFileDir,string makeFileTarg,string compiler){
	set<string> macpoStaticAnalysis;
	//(1) construct make command
	string makeCmd = string(make) + makeFileDir + " " + makeFileTarg;
	//(2) set environment variables
	string flagVariable;
	if(compiler == "CXX")
		flagVariable = "CXXFLAGS";
	else
		flagVariable = "CFLAGS";
 
	clearEnvironmentVariable(compiler);
	addEnvironmentVariables(compiler,string(runMacpoPrefix));
	clearEnvironmentVariable(flagVariable);
	string hotspotValues = "";
	for(auto it=hotloops_notorpartvec.begin();it!=hotloops_notorpartvec.end();it++){
		hotspotValues += string(runMacpoAlignment) + (*it) + " ";
	}
	hotspotValues += string(flags);
	addEnvironmentVariables(flagVariable,hotspotValues);

	//(3) parse makeoutput
	string out = "";
	executeCommand("make clean 2>&1",out,true);
	macpoStaticAnalysis = invokeMacpoStaticHelper(makeCmd,hotloops_notorpartvec,macpoStaticAnalysisOutput);
	return macpoStaticAnalysis;
}

set<string> invokeMacpoStatic(vector<string>& hotloops_notorpartvec,unordered_map<string,string>& noVecPartVec,string outDir,bool isPwd,unordered_map<string,string>& macpoStaticAnalysisOutput,string& objFile){
        set<string> macpoStaticAnalysis;

        string objFilePath = "";
        if(isPwd || (!isPwd && outDir.find("vecreportparser-temp") != string::npos))
                objFilePath = outDir;
	
	string hotspotValues = "";	
	set<string> srcFiles;
	for(auto ii=hotloops_notorpartvec.begin();ii!=hotloops_notorpartvec.end();ii++){
		srcFiles.insert(noVecPartVec.at(*ii));
		hotspotValues += string(runMacpoAlignment) + (*ii) + " ";
	}
	string fileNames = "";
	for(auto ii=srcFiles.begin();ii!=srcFiles.end();ii++){
		fileNames += *ii + " "; 
	}

	objFile = objFilePath + "mpo0.o";

	string macpoCmd = string(runMacpoPrefix) + hotspotValues + string(flags) + fileNames + " -o " + objFile;
	//the map "macpoStaticAnalysisOutput" will be populated with a hotspot:macpoOutput mapping after the following function call
	macpoStaticAnalysis = invokeMacpoStaticHelper(macpoCmd,hotloops_notorpartvec,macpoStaticAnalysisOutput);
        return macpoStaticAnalysis;
}

int invokeMacpoRuntimeHelperLink(string command){
	string result = "";
	return executeCommand(command,result,true);
}

set<string> invokeMacpoRuntimeHelperExecute(string command,unordered_map<string,string>& macpoRuntimeAnalysisOutput){
	set<string> macpoRuntimeAnalysis;
	string result = "";
	if(executeCommand(command,result,true) == 0){
                vector<string> resultLines = splitStringByDelimiter(result,'\n');
                for(auto it=resultLines.begin();it!=resultLines.end();it++){
			if((*it).find(string(macpoPrefix)) != string::npos){
                        	size_t idxBracket = (*it).find_first_of("]");
                                string line = (*it).substr(idxBracket+1);
                                line = trimWhiteSpaces(line);
                                size_t idxPeriod = line.find_first_of("|");
                                if(idxPeriod != string::npos){
                                	string strKey = line.substr(0,idxPeriod);
                                        string recmdn = line.substr(idxPeriod+1);
                                        std::replace(recmdn.begin(),recmdn.end(),'|','.');
                                        if(macpoRuntimeAnalysisOutput.find(strKey) != macpoRuntimeAnalysisOutput.end()){
                                        	string recmdnExisting = macpoRuntimeAnalysisOutput.at(strKey);
                                                recmdnExisting += recmdn + ";";
                                                macpoRuntimeAnalysisOutput.at(strKey) = recmdnExisting;
						if(macpoRuntimeAnalysis.find(strKey) == macpoRuntimeAnalysis.end()){
							macpoRuntimeAnalysis.insert(strKey);
						}
                                        }
                                }
                        }
			
                }
        }
       	else{
		cout<<endl<<"Error executing binary"<<endl;
        }
	return macpoRuntimeAnalysis;
}

set<string> invokeMacpoRuntime(set<string>& macpoStaticAnalysis,string binaryPath,string executableArguments,unordered_map<string,string>& macpoRuntimeAnalysisOutput){
	set<string> macpoRuntimeAnalysis;
        for(auto it=macpoStaticAnalysis.begin();it!=macpoStaticAnalysis.end();it++){
                if((*it).find(":") != string::npos){
                        macpoRuntimeAnalysisOutput.insert(make_pair((*it),""));
                }
        }	
	string macpoCmd = binaryPath + " " + executableArguments + " 2>&1";
	macpoRuntimeAnalysis = invokeMacpoRuntimeHelperExecute(macpoCmd,macpoRuntimeAnalysisOutput);
	return macpoRuntimeAnalysis;
}

set<string> invokeMacpoRuntime(set<string>& macpoStaticAnalysis,string outDir,bool isPwd,string& objFile,string objFilesForLinking,string executableArguments,unordered_map<string,string>& macpoRuntimeAnalysisOutput){

        set<string> macpoRuntimeAnalysis;
        string result = "";

        string binaryFilePath = "";
        if(isPwd || (!isPwd && outDir.find("vecreportparser-temp") != string::npos))
                binaryFilePath = outDir;
	
	for(auto it=macpoStaticAnalysis.begin();it!=macpoStaticAnalysis.end();it++){
		if((*it).find(":") != string::npos){
			macpoRuntimeAnalysisOutput.insert(make_pair((*it),""));
		}		
	}

	string macpoCmd = string(runMacpoPrefix) + "-o " + binaryFilePath + "mpoB " + objFile + " " + objFilesForLinking;
	if(invokeMacpoRuntimeHelperLink(macpoCmd) == 0){
		macpoCmd = binaryFilePath + "mpoB " + executableArguments + " 2>&1";
		macpoRuntimeAnalysis = invokeMacpoRuntimeHelperExecute(macpoCmd,macpoRuntimeAnalysisOutput);
	}
	else	
		cout<<endl<<"Linking failed"<<endl;
		
	return macpoRuntimeAnalysis;
}

struct cmd_line_args{
private:	
	vector<string> params;
	bool isOutputTempDir;
        string vecreport6FileName;
        string vecreport7FileName;
	string vec7messagesFileName;
	string objectFiles;
	string arguments;
	string execName;
	bool isMakeFile;
	string makeFileDir;
	string makeFileTarg;
	string binaryPath;
	string compiler; //can be CC or CXX
	string macpoBatchSize;
public:        
	cmd_line_args():isOutputTempDir(false),vecreport6FileName(""),vecreport7FileName(""),vec7messagesFileName(""),objectFiles(""),arguments(""),execName(""),makeFileDir(""),makeFileTarg(""),binaryPath(""),isMakeFile(false),compiler("CC"),macpoBatchSize(string(macpoDefaultBatchSize)){}
        cmd_line_args(vector<string> parameters){
		params = parameters;
        	isOutputTempDir = (std::find(params.begin(),params.end(),"-t") != params.end()) ? true : false;
                vecreport6FileName = getValue("-v6");
                vecreport7FileName = getValue("-v7");
		vec7messagesFileName = getValue("-vm");
		objectFiles = getValue("-o");
		arguments = getValue("-a");
		if(arguments.find("~i") != string::npos)
			std::replace(arguments.begin(),arguments.end(),'~','-');
		execName = getValue("-e");
		makeFileDir = getValue("-makedir");
		makeFileTarg = getValue("-maketarg");
		if(makeFileTarg == "")
			makeFileTarg = "make ";
		binaryPath = getValue("-binpath");
		isMakeFile = (std::find(params.begin(),params.end(),"-p") != params.end()) ? true : false;
		compiler = getValue("-src"); 
		macpoBatchSize = getValue("-bs");
		if(macpoBatchSize == "")
			macpoBatchSize = string(macpoDefaultBatchSize);
        }
        string getValue(string arg){
        	int pos = std::find(params.begin(),params.end(),arg) - params.begin();
		string result = "";
                if(pos != (params.end() - params.begin())){
			for(int j=pos+1;j<params.size();j++){
				if(params[j][0] == '-')
					break;
				result += " " + params[j];
			}
		}
		result = trimWhiteSpaces(result);	
              	return result;
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
	string getObjectFiles(){
		return objectFiles;
	}
	string getArguments(){
		return arguments;
	}
	string getExecName(){
		return execName;
	}
	bool getIsMakeFile(){
		return isMakeFile;
	}
	string getMakeFileDir(){
		return makeFileDir;
	}
	string getMakeFileTarg(){
		return makeFileTarg;
	}
	string getBinaryPath(){
		return binaryPath;
	}
	string getCompiler(){
		return compiler;
	}
	int getMacpoBatchSize(){
		if(stoi(macpoBatchSize) <= 0)
			macpoBatchSize = string(macpoDefaultBatchSize);
		return stoi(macpoBatchSize);
	}
};

int main(int argc,char* argv[]){

	vector<string> params(argv,argc+argv);
	cmd_line_args carg(params);

	if(carg.getVecreport6FileName().empty() && carg.getVecreport7FileName().empty()){
		cout<<endl<<"vec reports 6 and 7 not specified"<<endl;
		return 0;
	}
        if(carg.getExecName().empty()){
                cout<<endl<<"Executable not specified (to execute Perfexpert)"<<endl;
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
	string outDir = getOutputDirectory(finalOutputFileName);

	//get hotspot loops of the program using perfexpert
	//fetch hotloops (at present parsing the console output)
	set<string> hotLoops;
        bool isPerfExpertEnabled = getHotLoops(carg.getExecName(),carg.getArguments(),hotLoops);
	if(!isPerfExpertEnabled){
                cout<<endl<<"Check Perfexpert"<<endl;
                return 0;
        }

	//get the loops which are not vectorized / partially vectorized (from vectorization reports)
        unordered_map<string,string> noVecPartVec = getNoVecPartVec(htabLines);
        vector<string> v1;
        v1.reserve(noVecPartVec.size());
        for(auto kv=noVecPartVec.begin();kv!=noVecPartVec.end();kv++)
                v1.push_back(kv->first);
        vector<string> v2(hotLoops.begin(),hotLoops.end());
        vector<string> hotloops_notorpartvec = getIntersection(v1,v2);
	//display for diagnostic purposes only
        cout<<endl<<"Hot Loops that are Not Vectorized or Partially Vectorized:"<<endl;
        for(auto ii=hotloops_notorpartvec.begin();ii!=hotloops_notorpartvec.end();ii++)
                cout<<*ii<<endl;

	uint64_t set1_size = 0,set2_size = 0,set3_size = 0,ct=0;
	int nbBatch = (int)ceil((double)hotloops_notorpartvec.size()/(double)carg.getMacpoBatchSize());
	int batchSize = (hotloops_notorpartvec.size() >= carg.getMacpoBatchSize()) ? carg.getMacpoBatchSize() : hotloops_notorpartvec.size(); 
	for(uint64_t i=0;i<nbBatch;i++){
		vector<string> hotloops_notorpartvec_batch;
		for(uint64_t j=0;j<batchSize;j++,ct++)
			hotloops_notorpartvec_batch.push_back(hotloops_notorpartvec[ct]);

		//declare variables	
        	unordered_map<string,string> macpoStaticAnalysisOutput;
        	unordered_map<string,string> macpoRuntimeAnalysisOutput;
		set<string> macpoStaticAnalysis;
		set<string> macpoRuntimeAnalysis;
		string objFile;

		//static analysis of code using macpo
		if(carg.getIsMakeFile()){
			if(carg.getMakeFileDir().empty()){
				cout<<endl<<"Makefile path not specified"<<endl;
				return 0;
			}
			if(carg.getCompiler().empty()){
				cout<<endl<<"Source file type not specified"<<endl;
				return 0;
			}
			macpoStaticAnalysis = invokeMacpoStatic(hotloops_notorpartvec_batch,macpoStaticAnalysisOutput,carg.getMakeFileDir(),carg.getMakeFileTarg(),carg.getCompiler());
		}
		else
        		macpoStaticAnalysis = invokeMacpoStatic(hotloops_notorpartvec_batch,noVecPartVec,outDir,isPwd,macpoStaticAnalysisOutput,objFile);
		//display for diagnostic purposes only
		cout << endl << "Macpo Static Analysis:" << endl;
		for(auto ii=macpoStaticAnalysisOutput.begin();ii!=macpoStaticAnalysisOutput.end();ii++)
			cout<<ii->first<< " => " <<ii->second<<endl;

		if(macpoStaticAnalysis.size() == 0){
			cout<<endl<<"Could not perform static analysis using Macpo"<<endl;
			return 0;
		}

		clearEnvironmentVariable("MACPO_DISPLAY_BOT");
		addEnvironmentVariables("MACPO_DISPLAY_BOT","true");
	
		if(carg.getIsMakeFile()){
                	if(carg.getBinaryPath().empty()){
                        	cout<<endl<<"Binary path not specified"<<endl;
                        	return 0;
                	}
			macpoRuntimeAnalysis = invokeMacpoRuntime(macpoStaticAnalysis,carg.getBinaryPath(),carg.getArguments(),macpoRuntimeAnalysisOutput);
		}
		else
			macpoRuntimeAnalysis = invokeMacpoRuntime(macpoStaticAnalysis,outDir,isPwd,objFile,carg.getObjectFiles(),carg.getArguments(),macpoRuntimeAnalysisOutput);
		//display for diagnostic purposes only
                cout << endl << "Macpo Runtime Analysis:" << endl;
                for(auto ii=macpoRuntimeAnalysisOutput.begin();ii!=macpoRuntimeAnalysisOutput.end();ii++)
                        cout<<ii->first<< " => " <<ii->second<<endl;

		set1_size += hotloops_notorpartvec_batch.size();
		set2_size += macpoStaticAnalysis.size();
		set3_size += macpoRuntimeAnalysis.size();
	}

	//Final output
	cout<<endl<<"Of the "<<set1_size<<" loops that are not vectorized/partially vectorized and are hotspots, "<<set2_size<<" can be analyzed by Macpo, of which "<<set3_size<<" can be instrumented by Macpo"<<endl;

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
