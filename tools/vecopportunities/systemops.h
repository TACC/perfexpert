#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdlib>

using namespace std;

#define MAXPATHLEN 1024

bool isFileFound(string fileName,ifstream& infile){
        infile.open(fileName);
        return (infile.is_open()?true:false);
}

int executeCommand(string cmd,string& result,bool isGetConsoleOutput){
        
	int ret;
	FILE *pip = popen(cmd.c_str(),"r");
        if(pip == NULL)
                return -1;
	if(isGetConsoleOutput){
        	while(!feof(pip)){
                	char buffer[1024];
                	while(fgets(buffer,sizeof(buffer),pip) != NULL){
                        	//buffer[sizeof(buffer)-1] = '\0';
                        	result += buffer;
                        }
               }
	}
        ret = pclose(pip);
        pip = NULL;
        return ret;
}

string get_working_path()
{
        char temp[MAXPATHLEN];
        if(getcwd(temp,MAXPATHLEN) != NULL)
                return string(temp);
        else
                return string("");

}

void appendEnvironmentVariables(string varName,vector<string> arguments){

	char* path = NULL;
	if(path = getenv(varName.c_str())){
		for(int i=0;i<arguments.size();i++){
			strcat(path,(char*)arguments[i].c_str());
			setenv(varName.c_str(),path,1);
		}
	}
	path = NULL;
}

void addEnvironmentVariables(string varName,string value){
	setenv(varName.c_str(),(char*)value.c_str(),1);
}

void clearEnvironmentVariable(string varName){
	unsetenv(varName.c_str());
}
