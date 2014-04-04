#include <iostream>
#include <fstream>

using namespace std;

#define MAXPATHLEN 1024

bool isFileFound(string fileName,ifstream& infile){
        infile.open(fileName);
        return (infile.is_open()?true:false);
}

int executeCommand(string cmd){

        FILE *pip = popen(cmd.c_str(),"r");
        if(pip == NULL)
                return -1;
        pclose(pip);
        pip = NULL;
        return 0;
}

string get_working_path()
{
        char temp[MAXPATHLEN];
        if(getcwd(temp,MAXPATHLEN) != NULL)
                return string(temp);
        else
                return string("");

}
