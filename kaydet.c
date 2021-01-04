
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define MESSAGE_SIZE 100

const char* filename = "sonuc.txt";

int isFileExist(const char*);

int main(int argc, char*argv[])
{
	char readedData[MESSAGE_SIZE];
	read(3,readedData,MESSAGE_SIZE);//pipedan gelen datayı oku.
  	printf("%s\n",readedData);

	FILE * fp;

	if(isFileExist(filename)) //eger dosya varsa append modunda aç ve yaz
	{
		fp = fopen(filename,"a");
		fprintf(fp,readedData);
		printf("Sonuç kaydedildi\n");
		fclose(fp);
	}
	else//yoksa write modunda aç böylelikle yeni bir dosya oto. oluşur.
	{
		fp = fopen(filename,"w");
		fprintf(fp,readedData);
		printf("Sonuç kaydedildi\n");
		fclose(fp);
	}

	return 0;
}
int isFileExist(const char *filename)
{
	FILE * file;
	if(file = fopen(filename,"r"))
	{
		fclose(file);
		return 1;
	}
	return 0;
}
