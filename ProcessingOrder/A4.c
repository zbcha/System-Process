/******************************************************************
Author:            Bochao Zhang
Assignment:        CSCI3120 ASN2
Date:              2020/11/9
 *****************************************************************/

//Environment
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//A structure for storing all the tasks
typedef struct process{
  char name[100];
  int arriveTime;
  int brustTime;
}Process;

//An array of all tasks, maximum 50!
Process pros[50];
//The length of the available tasks
int length;

//FCFS algorithm
void FCFS(Process p[],int l){
  //cumu stands for the accumulation of every tasks, same for the rest algorithms
  //wt stands for the waiting time of corresponding task 
  int cumu=0;
  int wt[50];
  printf("FCFS:\n");
  //print all the tasks, and accumaltes the waiting time
  for(int i=0;i<l;i++){
    printf("%s",p[i].name);
    printf("\t%d",cumu);
    wt[i]=cumu-p[i].arriveTime;
    cumu+=p[i].brustTime;
    printf("\t%d\n",cumu);
  }
  //Display all the waiting times, and sum them up for the average waiting time
  float sum=0;
  for(int i=0;i<l;i++){
    printf("Waiting Time %s: %d\n",p[i].name,wt[i]);
    sum+=wt[i];
  }
  printf("Average Waiting Time: %0.2f\n\n",sum/l);
}
//RR algorithm
void RR(Process p[],int l){
  //quantumn is set to 4 by default
  int quant=4;
  int cumu=0;
  //stoe stands for the rest of the bursttime for each task
  int stoe[l];
  //idt is an identification, end the loop if there is no remaining bursttimes
  int idt=0;
  int wt[l];
  printf("RR: \n");
  for(int i=0;i<l;i++){
    stoe[i]=p[i].brustTime;
    wt[i]=p[i].arriveTime;
  }
  //Continue the loop if there is reamaining brust time 
  while(idt!=l){
    //Go to the next task
    for(int i=0;i<l;i++){
      if(stoe[i]!=0){
        printf("%s",p[i].name);
        //calculate waiting time
	//waiting time is acutually reversed in this case
        wt[i]-=cumu;
        printf("\t%d",cumu);
	//absorb the  remaining brust time, if there is no remainings, go to the next task
        for(int j=0;j<quant;j++){
          if(stoe[i]==0) break;
          stoe[i]--;
          cumu++;
        }
	//if this task is done, do not cumulate
        if(stoe[i]!=0) wt[i]+=cumu; 
        printf("\t%d\n",cumu);
      }
    }
    //if there is task done, add up idt
    idt=0;
    for(int i=0;i<l;i++){
      if(stoe[i]==0) idt++;
    }
  }
  //sum of the waiting times
  float sum=0;
  for(int i=0;i<l;i++){
    printf("Waiting Time %s: %d\n",p[i].name,abs(wt[i]));
    sum+=abs(wt[i]);
  }
  printf("Average Waiting Time: %0.2f\n\n",sum/l);
}
//NPSJF algorithm
void NPSJF(Process p[],int l){
  int cumu=0;
  int wt[l];
  //idar is an array of identifiers that informs if the current task is already finished
  int idar[l];
  printf("NPSJF:\n");
  for(int i=0;i<l;i++){
    wt[i]=p[i].arriveTime;
  }
  //first task always comes first
  for(int i=0;i<l;i++){
    if(i==0){
      cumu+=p[i].brustTime;
      idar[i]=1;
      wt[i]=0;
      printf("%s\t%d\t%d\n",p[i].name,wt[i],cumu);
    }else{
      //find the minimum number of brust time
      //Always update the new minval
      int minval=1000000;
      for(int j=0;j<l;j++){
        if(idar[j]!=1 && p[j].brustTime<minval) minval=p[j].brustTime; 
      }
      //pop up the task with minimum brust time by marking corrsponding idar to 1
      for(int j=0;j<l;j++){
        if(p[j].brustTime==minval && idar[j]!=1){
          wt[j]=cumu-p[j].arriveTime;
          printf("%s\t%d\t%d\n",p[j].name,cumu,cumu+minval);
          cumu+=minval;
          idar[j]=1;
	  break;
        }  
      }
    }
  }
  //sum of the waiting time
  float sum=0;
  for(int i=0;i<l;i++){
    printf("Waiting Time %s: %d\n",p[i].name,wt[i]);
    sum+=wt[i];
  }
  printf("Average Waiting Time: %0.2f\n\n",sum/l);
}
//PSJF algorithm
void PSJF(Process p[],int l){
  int cumu=0;
  int wt[l];
  //if a task is skiped currently, tempr stores the current brust time and reduce it by 1 for further calculation
  int tempr[l];
  int idar[l];
  //skip is an identifier that informs if it is needed to skip the current task to the smaller brust time task
  int skip=0;
  //skiptr is an identifer informs if the skip identifer is already used
  int skiptr=0;
  printf("PSJF:\n");
  //fill up all the arrays
  for(int i=0;i<l;i++){
    wt[i]=p[i].arriveTime;
    tempr[i]=p[i].brustTime;
    //Found a smmaller brust time, set skip to 1
    if(p[i].brustTime<p[0].brustTime) skip=1;
  }
  for(int i=0;i<l;i++){
    if(i==0){
      //if there is no task with smaller brust time, treat it like normal SJF
      if(skip==0){
        cumu+=p[0].brustTime;
        idar[0]=1;
        printf("%s\t%d\t%d\n",p[0].name,wt[0],cumu);
      }
      //If there is, then starts PSJF
      if(skip==1){
	//reduce current brust time by 1, add up cumulation by 1
        cumu++;
        tempr[0]--;
        printf("%s\t%d\t%d\n",p[0].name,wt[0],cumu);
      }
    }else{
      //if there is a skip, exdend the loop by 1
      if(i==1 && skip==1 && skiptr==0){ i--; skiptr=1;}
      //Same content as NPSJF
      int minval=10000000;
      for(int j=0;j<l;j++){
        if(tempr[j]<minval && idar[j]!=1) minval=tempr[j];
      }
      for(int j=0;j<l;j++){
        if(idar[j]!=1 && tempr[j]==minval){
          wt[j]=cumu-p[j].arriveTime;
          printf("%s\t%d\t%d\n",p[j].name,cumu,cumu+minval);
          cumu+=minval;
          idar[j]=1;
          break;
        }
      }       
    }    
  }
  //Sum of all the waiting times
  float sum=0;
  if(skip==1) wt[0]--;
  for(int i=0;i<l;i++){
    printf("Waiting Time %s: %d\n",p[i].name,wt[i]);
    sum+=wt[i];
  }
  printf("Average Waiting Time: %0.2f\n\n",sum/l);
}

int main(){
  //File reader
  char c[100];
  char *tk;
  FILE *file;
  file = fopen("TaskSpec.txt","r");
  //Error loading the txt file
  if(file==NULL){
    printf("No such file!");
    exit(1);
  }
  //Do the token process
  while(fgets(c,sizeof(c),file)!=NULL){
    Process cur;
    tk = strtok(c,",");
    strcpy(cur.name,tk);
    tk = strtok(NULL,",");
    cur.arriveTime=atoi(tk);
    tk = strtok(NULL,",");
    cur.brustTime=atoi(tk);
    pros[length++]=cur;
  }
  fclose(file);
  //Store all the standard output to the file
  file = freopen("Output.txt","w",stdout);
  //do all the four algorithms
  FCFS(pros,length);
  RR(pros,length);
  NPSJF(pros,length);
  PSJF(pros,length);
  fclose(file);
 
  return 0;
}
