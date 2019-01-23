#include "projectA.h"

int main(int argc, char * argv[])
{
	char* configFilePath;
	for (int i=1;i<argc;i++)
	{
		configFilePath = argv[i];
	}
	cRouter primRouter;
	primRouter.readConfigFile(configFilePath);
	pid_t fPid;
	fPid = fork();
	if(fPid<0)
	{
		cout<<" fork : error"<<endl;
	}
	else if (fPid==0)
	{
		cout<<"second router"<<getpid()<<endl;
	}
	else
	{
		cout<<"primary router"<<getpid()<<endl;
	}	

}
