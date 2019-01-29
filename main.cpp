#include "projectA.h"

int main(int argc, char * argv[])
{
	char* configFilePath;
	for (int i=1;i<argc;i++)
	{
		configFilePath = argv[i];
	}
	cRouter Router;
	Router.readConfigFile(configFilePath);
	pid_t fPid;
	fPid = fork();
	if(fPid<0)
	{
		cout<<" fork : error"<<endl;
	}
	else if (fPid==0)
	{
		Router.iRouterID = 1;
		vector<string> vLog;
		string temp = "second router"+to_string(getpid());
		vLog.push_back(temp);
		Router.writeLogFile(vLog);
	}
	else
	{
		vector<string> vLog;
		string temp = "primary router"+to_string(getpid());
		vLog.push_back(temp);
		Router.writeLogFile(vLog);
	}	

}
