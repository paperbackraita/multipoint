#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>

char buff[2000];

int initial_id_array[10], initial_id_count = 0;
int master_id_array[10], master_id_count = 0;
int slave_id_array[10], slave_id_count = 0;
int temp_id_array[10], temp_id_count = 0;

void printContent(void);

void handleINT(int sig)
{
	int i;
	char comm[100];
	signal(sig, SIG_IGN);
	printf("\nJoining all mouse pointers back to one...\n");
	
	sleep(2);
	
	for(i = 0 ; i < 10; i++)
	{
		if(slave_id_array[i] != 0)
		{
			sprintf(comm,"xinput reattach %d 2",slave_id_array[i]);
			system(comm);
		}
		if(master_id_array[i] != 0 && master_id_array[i] != 2)
		{
			sprintf(comm,"xinput remove-master %d",master_id_array[i]);
			system(comm);
		}
	}
	
	sleep(2);
	
	printf("Quitting...\n");
	sleep(1);
	exit(0);
}

void output(char* command_string)
{
	char line[50];
	FILE *input_file;
	buff[0] = '\0';
	if((input_file = popen(command_string, "r")))
	{
		while(fgets(line, 50, input_file) != NULL)
			strcat(buff,line);
		pclose(input_file);
	}
	else
		return;
}

void findID(void)
{
	temp_id_count = 0;
	char *point,*temp = buff-1;
    int ctr[10],ctr_pt = 0, ct = 0;
    while(((point = (char *)strstr(temp+1, "id")) != NULL))
    {
    	temp = point;
    	char num[10]="", temp[2]="";
    	point = point + 3;
    	while(!isspace(*point))
    	{
    		temp[0] = *point;
    		strcat(num,temp);
    		point++;
    	}
    	int n = atoi(num);
    	ctr[ctr_pt++] = n;
    	
    }
    
    for(ct = 0; ct < 10; ct++)
    {
    	temp_id_array[ct] = ctr[ct];
    	if(ctr[ct] == 0 || ctr[ct] > 100)
    		temp_id_array[ct] = 0;
    	else	
    		temp_id_count++;
    }
}

void slaveAddition(void)
{
	int i,flag,j;
	for(i = 0; i < 10; i++)
	{
		flag = 0;
		for(j = 0; j < 10; j++)
		{
			if(temp_id_array[i] == slave_id_array[j])
				flag = 1;
		}
		if(!flag && temp_id_array[i] != 0)
			slave_id_array[slave_id_count++] = temp_id_array[i];
	}
	
	for(i = 0; i < slave_id_count; i++)
	{
		flag = 0;
		for(j = 0; j < 10; j++)
		{
			if(temp_id_array[j] == slave_id_array[i])
			{
				flag = 1;
				break;
			}
		}
		if(!flag)
		{
			slave_id_array[i] = 0;
		}
	}
	
	for(i = 0; i < 10 ; i++)
	{
		if(master_id_array[i] == 0 && slave_id_array[i] == 0)
		{
			int j;
			for(j = i; j < 9; j++)
			{
				int temp = master_id_array[j];
				master_id_array[j] = master_id_array[j+1];
				master_id_array[j+1] = temp;
				temp = slave_id_array[j];
				slave_id_array[j] = slave_id_array[j+1];
				slave_id_array[j+1] = temp;
			}
		}
	}
	
	return;
}



void createMaster(int slave)
{
	printContent();
	//getchar();
	int i;
	char snum[5], comm[100], name[8] = "new";
	
	sprintf(snum,"%d",slave);
	strcat(name,snum);
	sprintf(comm,"xinput create-master %s",name);
	system(comm);
	sprintf(comm,"xinput list | grep -v XTEST | grep pointer | grep master| grep %s",name);
	output(comm);
	findID();
	int masterID = temp_id_array[0];
	printf("Master generated for id=%d is %s with id=%d\n",slave,name,masterID);
	
	sprintf(comm,"xinput reattach %d %d",slave,masterID);
	system(comm);
	printf("Mouse reattached successfully!\n");
	
	for(i = 0; i < 10; i++)
	{
		if(slave_id_array[i] == slave)
		{
			master_id_array[i] = masterID;
			master_id_count++;
			break;
		}
	}
	printContent();
	return;
}

void printContent(void)
{
	int i;
	printf("Printing contents: \n");
//	printf("%s\t%s\t%s\t%s\n","Master","Slave","Temp","Initial");
	printf("M\t");
	for(i = 0; i < 10; i++)
		printf("%d\t",master_id_array[i]);
	printf("\n");
	printf("S\t");
	for(i = 0; i < 10; i++)
		printf("%d\t",slave_id_array[i]);
	printf("\n");
	printf("T\t");
	for(i = 0; i < 10; i++)
		printf("%d\t",temp_id_array[i]);
	printf("\n");
	printf("I\t");
	for(i = 0; i < 10; i++)
		printf("%d\t",initial_id_array[i]);
	printf("\n");
	
	printf("Count: \n%d\t%d\t%d\t%d\n\n",master_id_count,slave_id_count,temp_id_count,initial_id_count);
	return;
}

int main(void)
{
	system("clear");
	int i;
	for(i = 0; i < 10; i++)
		temp_id_array[i] = initial_id_array[i] = master_id_array[i] = slave_id_array[i] = 0;
	
	output("xinput list | grep -v XTEST | grep pointer | grep master");
	findID();
	int main_pointer = temp_id_array[0];
	printf("Multipoint started...\n");
	
	output("xinput list | grep -v XTEST | grep pointer | grep slave");
	findID();
	for(i = 0; i < 10; i++)
	{
		initial_id_array[i] = temp_id_array[i];
		if(initial_id_array[i] > 0)
		{
			initial_id_count++;
			master_id_array[i] = main_pointer;
			master_id_count++;
		}
	}
	
	//printContent();
	
	printf("Starting the program, press enter to continue >>>");
	getchar();
	
	signal(SIGINT, handleINT);
	while(1)
	{
		system("clear");
		printf("Listening for mouse input...\n");
		sleep(1);
		system("clear");
		int tmpt;
		for(tmpt = 0; tmpt < 10000; tmpt++);
		output("xinput list | grep -v XTEST | grep pointer | grep slave");
		findID();
		slaveAddition();
		
		printf("Devices are: ");
		for(i = 0; i < 10; i++)
			if(slave_id_array[i] > 0)
				printf("%d\t",slave_id_array[i]);
		printf("\t\t");
				
		printf("\nExtra devices are: ");
		for(i = initial_id_count; i < 10; i++)
		{
			if(slave_id_array[i] > 0)
			{
				printf(">>>%d\t",slave_id_array[i]);
				if(master_id_array[i] == 0)
				{
					printf("Creating a new master for %d\n",slave_id_array[i]);
					createMaster(slave_id_array[i]);
					//getchar();
				}
			}
		}
		printf("\n");
		
		for(i = 0; i < 10; i++)
		{
			if(slave_id_array[i] == 0 && master_id_array[i] != 0)
			{
				int remMaster = master_id_array[i];
				master_id_array[i] = 0;
				master_id_count--;
				if(remMaster != 2)
				{
					printf("Master being deleted...%d\n",remMaster);
					char comm[100];
					sprintf(comm,"xinput remove-master %d",remMaster);
					system(comm);
					//getchar();
				}
			}
		}
		
		for(i = 0; i < 10 ; i++)
		{
			if(master_id_array[i] == 0 && slave_id_array[i] == 0)
			{
				int j;
				for(j = i; j < 9; j++)
				{
					int temp = master_id_array[j];
					master_id_array[j] = master_id_array[j+1];
					master_id_array[j+1] = temp;
					temp = slave_id_array[j];
					slave_id_array[j] = slave_id_array[j+1];
					slave_id_array[j+1] = temp;
				}
			}
		}
		
		system("clear");
		sleep(1);
		printContent();	
	}

   return 0;
}
