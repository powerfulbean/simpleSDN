#include "cRouter.h"
#include "Router.h"
#include "sample_tunnel.h"

int main(int argc, char * argv[])
{
	char* configFilePath;
	cRouter Router;
	struct sockaddr_in rou1Addr;
	struct sockaddr_in rou2Addr;
	for (int i=1;i<argc;i++)
	{
		configFilePath = argv[i];
	}
	Router.readConfigFile(configFilePath);

	while(Router.stageEngine())
	{
	cout<<"-----------------------"<<endl;
	cout << "***stageEngine: run stage " << Router.iStage
		<< "; Router Number: " << Router.iRouteNum;
		if (Router.iStage >= 4)
		{
			cout << "; DropOut Number: " << Router.m_iDropAfter;
		}
		cout << endl;
		switch(Router.iStage)
		{
		  case 1:
	   		stage1(Router, rou1Addr, rou2Addr);
			Router.close();
			if (Router.iFPID == 0)
			{
				exit(0);
			}
			else
			{
				waitpid(Router.iFPID, NULL, 0);
			}
			cout<<"***Stage 1 end, pid: "<<getpid();
			cout<<"\n-----------------------"<<endl<<endl;
			
			break;
		  case 2:
			stage2(Router, rou1Addr, rou2Addr);
		//	tunnel_reader();	
			Router.close();
			cout<<"***Stage 2 end, pid: "<<getpid();
                        cout<<"\n-----------------------"<<endl<<endl;
			break;
		  case 3:
			stage2(Router, rou1Addr, rou2Addr);
			//	tunnel_reader();	
			Router.close();
			cout << "***Stage 3 end, pid: " << getpid();
			cout << "\n-----------------------" << endl << endl;
			break;
		  case 4:
			  stage4(Router, rou1Addr, rou2Addr);
			  //	tunnel_reader();	
			  Router.close();
			  cout << "***Stage 4 end, pid: " << getpid();
			  cout << "\n-----------------------" << endl << endl;
			  break;
		  case 5:
			  stage5(Router, rou1Addr, rou2Addr);
			  //	tunnel_reader();	
			  Router.close();
			  cout << "***Stage 5 end, pid: " << getpid();
			  cout << "\n-----------------------" << endl << endl;
			  break;
		  case 6:
			  stage6(Router, rou1Addr, rou2Addr);
			  Router.close();
			  cout << "***Stage 6 end, pid: " << getpid();
		      cout << "\n-----------------------" << endl << endl;
			  break;
		  case 7:
			  stage7(Router, rou1Addr, rou2Addr);
			  Router.close();
			  cout << "***Stage 7 end, pid: " << getpid();
			  cout << "\n-----------------------" << endl << endl;
			  break;
		  case 8:
			  stage6(Router, rou1Addr, rou2Addr);
			  Router.close();
			  cout << "***Stage 8 end, pid: " << getpid();
			  cout << "\n-----------------------" << endl << endl;
			  break;
		  case 9:
			  stage9(Router, rou1Addr, rou2Addr);
			  Router.close();
			  cout << "***Stage 8 end, pid: " << getpid();
			  cout << "\n-----------------------" << endl << endl;
			  /*if (Router.iFPID != 0)
			  {
				  for (auto i : Router.m_rouFlowTable.m_mTable)
				  {
					  flow_entry temp = i.first;
					  temp.print();
				  }
			  }*/
			  break;
		  default:
			  break;
		}
	}		
}
