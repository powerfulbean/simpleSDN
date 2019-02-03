#include "cRouter.h"
#include "sample_tunnel.h"

int main(int argc, char * argv[])
{
	char* configFilePath;
	cRouter Router;
	for (int i=1;i<argc;i++)
	{
		configFilePath = argv[i];
	}
	Router.readConfigFile(configFilePath);
	while(Router.stageEngine())
	{
	cout<<"-----------------------"<<endl;
	cout<<"***stageEngine: run stage "<<Router.iStage<<endl;
		switch(Router.iStage)
		{
		  case 1:
	   		stage1(Router);
			cout<<"***Stage 1 end, pid: "<<getpid();
			cout<<"\n-----------------------"<<endl<<endl;
			
			break;
		  case 2:
			stage2(Router);
			tunnel_reader();
			break;
		  default:
			break;
		}
	}		
}
