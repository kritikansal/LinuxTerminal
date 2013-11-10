#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <wait.h>
#include <error.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int background=0,loop=1,m=0,j=0,stati[1024]={0},flag=0;
char **hist=NULL,h[100],command[1024];
char dir[1024],**proarr=NULL,**ptr=NULL;
long long pidarr[1024];
void print() //function to print prompt
{
	char * args;
	args=dir;
	char *user,path[1024],orig[1024];
	char hostname[1024];
	user=getlogin();
	strcpy(orig,args);
	gethostname(hostname, sizeof hostname);
	getcwd(path,1024);
	char n[1024]={0};
	strcat(orig,"/%[^\n]");
	sscanf(path,orig,n);
	if(strcmp(path,args)==0)
	{
		printf("<%s@%s:~> ", user,hostname); //if current path is same as invoking directory
	}
	else if(strlen(n)>0)
	{
		printf("<%s@%s:~/%s> ", user,hostname,n); //if current path is out of the invoking directory
	}
	else
	{
		printf("<%s@%s:%s > ", user,hostname,path); //if current path is in the invoking directory
	}
}
void sig_handler()
{
	printf("\n");
	print();
	fflush(stdout);
	signal(SIGINT,  sig_handler);
	signal(SIGTSTP, sig_handler);
	signal(SIGQUIT, sig_handler);
	return;
}
void child_handler( )
{
	int status1,i;
	pid_t pchild = waitpid(-1,&status1,WNOHANG);
	for(i=0;i<m;i++)
	{
		if(pidarr[i]==pchild && stati[i]==1)
		{
			if(WIFEXITED(status1))
			{
				printf("\n%s %lld exited normally\n",proarr[i],pidarr[i]);
				stati[i]=0;
			}
			else if(WIFSIGNALED(status1))
			{
				printf("\n%s %lld killed\n",proarr[i],pidarr[i]);
				stati[i]=0;
			}
			else if(WCOREDUMP(status1))
			{
				printf("\n%s %lld core dumped\n",proarr[i],pidarr[i]);
				stati[i]=0;
			}
			print();
			fflush(stdout);
		}
	}
	return;
}
void history()
{
	int u,l;
	if(strcmp(h,"hist")==0) //checks for hist and histn
	{
		strcpy(h,command+4);
		if(strlen(h)==0)
		{
			for(l=0;l<j-1;l++)
			{
				printf("%d. %s\n",l+1,hist[l]);
			}
		}
		else
		{
			u=atoi(h);
			if(u>j)
			{
				for(l=0;l<j-1;l++)
				{
					printf("%d. %s\n",l+1,hist[l]);
				}
			}
			else
			{
				u=j-u-1;
				for(l=u;l<j-1;l++)
				{
					printf("%d. %s\n",l+1-u,hist[l]);
				}
			}
		}
	}
}
void hhistory()
{
	int u;
	if(strcmp(h,"!hist")==0) //checks for !histn
	{
		strcpy(h,command+5);
		u=atoi(h);
		print();
		if(u<j)
		{
			printf("%s\n",hist[u-1]);
			flag=1;
			strcpy(command,hist[u-1]);
		}
	}
}		
void Pid(char * ee)
{
	int l,k;
	if(ptr[1]==NULL)
	{
		printf("command name: %s process id: %ld\n",ee,(long)getpid());	
	}
	else
	{
		if(strcmp("all",ptr[1])==0)
		{
			printf("List of all processes spawned from this shell:\n");
			for(k=0;k<m;k++)
			{
				printf("command name: %s process id: %lld\n",proarr[k],pidarr[k]);	
			}
		}
		else if(strcmp("current",ptr[1])==0)
		{
			printf("List of currently executing processes spawned from this shell:\n");
			for(l=0;l<m;l++)
			{
				if (kill(pidarr[l], 0) == 0) {
					printf("command name: %s process id: %lld\n",proarr[l],pidarr[l]);
				}
			}
		}
	}
}
int main(int argc,char *argvc[]) 
{
	char buffer[1024],*argv,a[10],pp[50];
	ptr=calloc(1024,sizeof(char));
	hist=calloc(1024,sizeof(char));
	proarr=calloc(1024,sizeof(char));
	int i,l,status,k,o;
	for(i=0;i<1024;i++)
	{
		ptr[i]=calloc(1024,sizeof(char*));
		hist[i]=calloc(1024,sizeof(char*));
		proarr[i]=calloc(1024,sizeof(char*));
	}
	char d[1024];
	pid_t pid;
	getcwd(dir,1024);
	signal(SIGINT,  sig_handler);
	signal(SIGTSTP, sig_handler);
	signal(SIGQUIT, sig_handler);
	signal(SIGCHLD, child_handler);
	print();
	while(loop == 1)
	{
		fflush(0);
		if(flag==0) //flag for !hist ie if we have to scan or execute !hist
		{
			strcpy(d,"\0");
			while(scanf("%[^\n]",d)==EOF);
			getchar();
			if(strcmp(d,"\0")==0)
			{
				print();
				continue;
			}
			strcpy(command,d);
			strcpy(hist[j],command);
			j++;
		}
		flag=0;
		background=0;
		int FLAG=0;
		argv = strtok(command, " \t ");
		i=1;
		if(argv!=NULL) //breaks command into tokens
		{
			ptr[0]=argv;
			while(1)
			{
				argv=strtok(NULL, " \t");
				if(argv==NULL)break;
				else if(argv[0]==32 || argv[0]==9)
				{}
				else
				{
					if(strcmp(argv,">")==0 || strcmp(argv,"<")==0 || strcmp(argv,"|")==0)
					{
						FLAG=1;
					}
					ptr[i]=argv;
					i++;
				}
			}
			ptr[i++]=argv;
			//printf("argv is %s ptr is ptr[i-1] %s\n",argv,ptr[i-1]);
		}
		strncpy(h,command,4);
		h[4]='\0';
		if(strcmp(h,"hist")==0 && FLAG==0)
		{
			history();
			print();
			continue;
		}
		else
		{
			strncpy(h,command,5);
			h[5]='\0';
			if(strcmp(h,"!hist")==0 && FLAG==0)
			{
				hhistory();
				print();
				continue;
			}
			if(strcmp(ptr[0],"pid")==0 && FLAG==0)
			{
				Pid(argvc[0]);
				print();
				continue;
			}
			
			int fff;
		int f1,f2,f3,f4,gg=0;
		char s1[100]="";
		char s2[100]="";
		char s3[100];
		strcpy(s3,"abcd");
		char * new[100];
		if(ptr[1]!=NULL) //checks for "&"
		{
			if(i==2)
			{
				i=1;
			}
			if(strcmp("&",ptr[i-2])==0)
			{
				background=1;
				ptr[i-2]=NULL;
			}
		}
		if(FLAG==1)
		{
			//printf("entered\n");
			strcpy(proarr[m],hist[j-1]);
			pidarr[m]=pid;
			m++;
			//fflush(stdout);
			for(fff=0;fff<i-1;fff++)
			{
				//printf("%s\n",ptr[fff]);
				if(strcmp(ptr[fff],">")==0)
				{
					fff++;
					strcpy(s1,ptr[fff]);
				}
				else if(strcmp(ptr[fff],"<")==0)
				{
					fff++;
					strcpy(s2,ptr[fff]);
				}
				else if(strcmp(ptr[fff],"|")==0)
				{
					//fff++;
					pid=fork();
					if(pid==0)
					{
						f1 = open(s3 , O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
						dup2(f1,1);
						if(strcmp(s2,"")!=0)
						{
							f2 = open(s2, O_RDONLY | O_CREAT);
							dup2(f2,0);
						}
						if(strcmp(ptr[fff-1],"hist")==0)
						{
							history();
						}
						else if(strncmp(ptr[fff-1],"!hist",5)==0)
						{
							hhistory();
						}
						else if(strcmp("pid",ptr[0])==0) //checks for pid,pid all,pid current command
						{
							printf("ss %s\n",argvc[0]);
							Pid(argvc[0]);
						}
						else
						{
							execvp(*new,new);
						}
						exit(0);
					}
					else
					{	
						//strcpy(proarr[m],hist[j-1]);
						//pidarr[m]=pid;
						//m++;
						wait(&status);
						strcpy(s2,s3);
						strcpy(s1,"");
						fflush(stdout);
					}
					for(k=0;k<gg;k++)
					{
						new[k]=NULL;
					}
					gg=0;
					//printf("in | %s %s\n",ptr[fff],ptr[fff+1]);
				}
				else
				{
					//printf("in else %s\n",ptr[fff]);
					if(ptr[fff]==NULL || strcmp(ptr[fff],"hist")==0 || strncmp(ptr[fff],"!hist",5)==0 || strcmp(ptr[fff],"pid")==0)
					{
						
					}
					else
					{
						//printf("%s  hi\n",ptr[fff]);
						new[gg]=calloc(1024,sizeof(char*));
						strcpy(new[gg],ptr[fff]);
						gg++;
					}
				}	
				if(ptr[fff+1]==NULL)
				{
					//printf("helllooo\n");
					//new[gg]=calloc(1024,sizeof(char*));
					//strcpy(new[gg],ptr[fff]);
					//gg++;
					//printf("s1--%s   s2--%s\n",s1,s2);	
					pid=fork();
					if(pid==0)
					{
						//printf( "s1--%s   s2--%s\n",s1,s2);	
						if(strcmp(s2,"")!=0)
						{
							f2 = open(s2, O_RDONLY | O_CREAT);
							dup2(f2,0);
							//printf("oo\n");
						}
						if(strcmp(s1,"")!=0)
						{
							f1 = open(s1 , O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
							dup2(f1 , 1);
							//printf("oo\n");
						}
						/*if(f1<0 || f2<0)
						{
							perror("open");
						}*/
						/*if(strcmp(ptr[fff-1],"hist")==0)
						{
							history();
						}
						else if(strncmp(ptr[fff-1],"!hist",5)==0)
						{
							hhistory();
						}
						else if(strcmp("pid",ptr[0])==0) //checks for pid,pid all,pid current command
						{
							Pid(argvc[0]);
						}
						else
						{*/
							execvp(*new,new);
						//}
						exit(0);
					}
					else
					{
						//strcpy(proarr[m],hist[j-1]);
						//pidarr[m]=pid;
						//m++;
						wait(&status);
						fflush(stdout);
					}
					for(k=0;k<gg;k++)
					{
						new[k]=NULL;
					}
					gg=0;
				}
							//printf("end %s %s i-%d  fff-%d\n",ptr[fff],ptr[fff+1],i,fff);
			}
		}
		else if(strcmp("cd",*ptr)==0) //checks for cd command
		{
			if(ptr[1]==NULL)
			{
				chdir(dir);
			}
			else
			{
				chdir(*(ptr+1));
			}
		}
		else if(strcmp(command, "quit") == 0) //checks for quit
		{
			loop=0;
			exit(3);
		}
		else 
		{
			pid=fork();
			if(pid==0)
			{
				execvp(*ptr,ptr);
				fprintf(stderr,"%s : Command not Found\n",command);
				exit(1);
			}
			else if(pid>0)
			{
				strcpy(proarr[m],hist[j-1]);
				pidarr[m]=pid;
				m++;
				if(background==0)
				{
					waitpid(pidarr[m-1],NULL,0);
				}
				else 
				{
					stati[m-1]=1;
					printf("command %s pid %lld\n",proarr[m-1],pidarr[m-1]);
				}
			}
			else
			{
				fprintf(stderr,"%s : Command not Found\n",command);
			}
		}}
		print();
	}
	return 0;
}
