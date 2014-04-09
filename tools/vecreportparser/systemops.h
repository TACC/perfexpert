#include <iostream>
#include <fstream>

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
