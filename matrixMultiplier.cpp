#include <string.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <math.h>
#include <fstream>
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
using namespace std;

int main(){
//Open the file by asking the user for the file name
	ifstream counter;
	char fileIn[100];
	while (!counter.is_open()){
		cout << "What is the name of the file that contains the matrices? ";
		cin>>fileIn;
		counter.open(fileIn);
	}
//Calculate the dimensions of the matrices
	int rowsOne=0;
	int colsOne=1;
	int rowsTwo=0;
	int colsTwo=1;
	char cur;
	//First Matrix
	for (cur = counter.get();cur!='*';cur = counter.get()){
		colsOne=1;
		for (cur = counter.get();(int)cur!=10;cur = counter.get()){
			if (isspace(cur)) colsOne++;
		}
		rowsOne++;
	}
	//Divider
	while ((int)cur!=10) cur = counter.get();
	//Second Matrix
	for (cur = counter.get();counter.good();cur = counter.get()){
		colsTwo=1;
		for (cur = counter.get();(int)cur!=10;cur = counter.get()){
			if (isspace(cur)) colsTwo++;
		}
		rowsTwo++;
	}
	counter.close();
	int rowsOneMax = rowsOne;
	int rowsTwoMax = rowsTwo;
	int colsOneMax = colsOne;
	int colsTwoMax = colsTwo;
//Read the matrices into int arrays
	ifstream in;
	int **one = new int*[rowsOneMax];
	int **two = new int*[rowsTwoMax];
	for (int i=0;i<rowsOneMax;i++) one[i] = new int[colsOneMax];
	for (int i=0;i<rowsTwoMax;i++) two[i] = new int[colsTwoMax];
	rowsOne=0;
	colsOne=0;
	rowsTwo=0;
	colsTwo=0;
	int curNumCount=0;
	float curNum=0;
	in.open(fileIn);
	//First matrix - must account for multiple digit numbers
	for (cur = in.get(); cur!='*'; cur = in.get()){
		one[rowsOne][0] = 0;
		for (colsOne=0; (int)cur!=10; cur = in.get()){
			if (isdigit(cur)){
				curNum += atoi(&cur) * pow(10,curNumCount);
				curNumCount --;				
			}else {
				curNumCount *=-1;
				curNumCount --;
				curNum *=pow(10,curNumCount);
				one[rowsOne][colsOne] = (int)curNum;
				curNumCount=0;
				curNum=0;
				colsOne++;
				one[rowsOne][colsOne] = 0;
			}
		}
		curNumCount *=-1;
		curNumCount --;
		curNum *=pow(10,curNumCount);
		one[rowsOne][colsOne] = (int)curNum;
		curNumCount = 0;
		curNum=0;
		rowsOne++;
	}
	//Divider
	while ((int)cur!=10) cur = in.get();
	//Second matrix - must account for multiple digit numbers
	for (cur = in.get(); in.good(); cur = in.get()){
		two[rowsTwo][0] = 0;
		for (colsTwo=0; (int)cur!=10; cur = in.get()){
			if (isdigit(cur)){
				curNum += atoi(&cur) * pow(10,curNumCount);
				curNumCount --;				
			}else {
				curNumCount *=-1;
				curNumCount --;
				curNum *=pow(10,curNumCount);
				two[rowsTwo][colsTwo] = (int)curNum;
				curNumCount=0;
				curNum=0;
				colsTwo++;
				two[rowsTwo][colsTwo] = 0;
			}
		}
		curNumCount *=-1;
		curNumCount --;
		curNum *=pow(10,curNumCount);
		two[rowsTwo][colsTwo] = (int)curNum;
		curNum=0;
		curNumCount = 0;
		rowsTwo++;
	}
//Matrix Printer
	cout << "One: "<<endl;
	for (int i=0;i<rowsOneMax;i++){ 
		for (int j=0;j<colsOneMax;j++){
			cout << one[i][j]<<" ";
		}
		cout <<endl;
	}
	cout << "Two: "<<endl;
	for (int i=0;i<rowsTwoMax;i++){ 
		for (int j=0;j<colsTwoMax;j++){
			cout << two[i][j]<<" ";
		}
		cout <<endl;
	}
//Create the answer matrix
	int ***answer = new int**[rowsOneMax];
	for (int i=0;i<rowsOneMax;i++){
		answer[i] = new int*[colsTwoMax];
		for  (int j=0;j<colsTwoMax;j++) answer[i][j] = new int[rowsTwoMax];
	}
//Create and run the processes
	int count=0;
	for (int y=0;y<rowsOneMax;y++){
		for (int x=0;x<colsTwoMax;x++){
			for (int index=0;index<rowsTwo;index++){
				//Create a new mini string of shared memory containing the two numbers to be multiplied and a spot for the product
				int shmid;
				key_t key;
				int *shm;
				key = 8838;
				if ((shmid = shmget(key, 1024, IPC_CREAT | 0666)) < 0){
					cout<<"shmget"<<endl;
					exit(1);
			    	}
				if ((shm = (int *)shmat(shmid, 0, 0)) == (int *) -1) {
        				cout<<"shmat"<<endl;
        				exit(1);
    				}
				*shm = one[y][index]; shm++;
				*shm = two[index][x]; shm++;
				*shm = -1;
				int pid = fork();
				if (pid==0){
					//In the child process multiply the numbers together
					int shmid;
					key_t key;
					int *shm;
					key = 8838;
					if ((shmid = shmget(key, 1024, IPC_CREAT | 0666)) < 0){
						cout<<"shmget"<<endl;
						exit(1);
				    	}
					if ((shm = (int *)shmat(shmid, 0, 0)) == (int *) -1) {
        					cout<<"shmat"<<endl;
        					exit(1);
    					}
					*(shm+2) = *(shm+1) * *shm;
					return 0;
				}else {
					//The parent process waits for the child to finish the multiplication
					int shmid;
					key_t key;
					int *shm;
					key = 8838;
					if ((shmid = shmget(key, 1024, IPC_CREAT | 0666)) < 0){
						cout<<"shmget"<<endl;
						exit(1);
				    	}
					if ((shm = (int *)shmat(shmid, 0, 0)) == (int *) -1) {
        					cout<<"shmat"<<endl;
        					exit(1);
    					}
					while (*(shm+2)==-1){}
				}
				//Reinitialize the shared memory in the main process and store the product from the child process in a 3 dimensional array
				if ((shmid = shmget(key, 1024, IPC_CREAT | 0666)) < 0){
					cout<<"shmget"<<endl;
					exit(1);
			    	}
				if ((shm = (int *)shmat(shmid, 0, 0)) == (int *) -1) {
       					cout<<"shmat"<<endl;
       					exit(1);
				}
				answer[y][x][index] =  *(shm+2);
				count ++;
			}
		}
	}
//Create the final product matrix
	int **product = new int*[rowsOneMax];
	for (int i=0;i<rowsOneMax;i++) product[i] = new int[colsTwoMax];
//Add the elements in the answer matrix
	cout << "Product: "<<endl;
	for (int y=0;y<rowsOneMax;y++){
		for (int x=0;x<colsTwoMax;x++){
			for (int index=0;index<rowsTwo;index++){
				if (index==0){ product[y][x] = answer[y][x][0];}
				else {product[y][x] += answer[y][x][index];} 
			}
			cout << product[y][x]<<" ";
		}
		cout <<endl;
	}
//Delete heap variables
	delete (one, two, answer, product);
	return 0;
}
