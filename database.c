
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#define QUERY_INP_BUF_SIZE 64
#define TOKENS_BUF_SIZE 256
#define ARGS_SPLIT_CHARS " \t\r\n\a"
#define RESULT_BUF_SIZE 20
#define LINE_BUF_SIZE 32
#define MESSAGE_SIZE 100


char ** split(char * ,char *);
enum QueryType declareQueryType(char**);
enum ConditionType specifyCondition(char**);
char ** initResultArray();


enum QueryType//sorgu tipini belirlemek için kullanılır. BRING ALL tüm satırı getir,BRING NAME sadece ismi getir
{	       //BRING NUMBER sadece no yu getir ve NOT_VALID_QUERY geçerli bir sorgu degil.
	BRING_ALL = 213,
	BRING_NAME = 214,
	BRING_NUMBER = 215,
	NOT_VALID_QUERY = 216
};

enum ConditionType//NO_CONDITION her şeyi getir koşulsuz,NAME_CONDITION isim koşuluna göre getir,NUMBER_CONDITION number koşuluna göre
{			//getir. NOT_VALID_CONDITION geçerli bir koşul girilmedi.
	NO_CONDITION = 0,
	NAME_CONDITION = 1,
	NUMBER_CONDITION =2,
	NOT_VALID_CONDITION = 3
};

int main()
{
	int fd1;
	char * myfifo = "/tmp/myfifo";
	mkfifo(myfifo,0666);
	char query[QUERY_INP_BUF_SIZE];
	char keepQuery[QUERY_INP_BUF_SIZE];

  while(1)
  {
		fflush(stdout);//stdout bufferi temizle
    memset(query,'\0',QUERY_INP_BUF_SIZE); //query bufferini temizle
    fd1 = open(myfifo,O_RDONLY);//named pipe i okumak için aç
  	read(fd1,query,QUERY_INP_BUF_SIZE); //pipedaki veriyi querye oku.
		close(fd1);
		strcpy(keepQuery,query);
		printf("Gelen veri = %s\n", keepQuery);
    char ** splitted = split(query,ARGS_SPLIT_CHARS); // gelen sorguyu tokenlere ayır.
    enum QueryType queryType = declareQueryType(splitted); // sorgunun tipini belirle

		if(queryType == NOT_VALID_QUERY)//eger NOT_VALID ise NULL döndür devam et
		 {
			 free(splitted);
			 printf("Not A Valid Query\n");
			 fd1 = open(myfifo,O_WRONLY);
			 write(fd1,"NULL",strlen("NULL"));
			 close(fd1);
			 continue;
		 }
		 else
		 {
			 enum ConditionType condition = specifyCondition(splitted);//koşulu belirle

			 if(condition == NOT_VALID_CONDITION)//geçersiz koşulsa da NULL döndür
			 {
				 free(splitted);
				 printf("Not A Valid Condition\n");
				 fd1 = open(myfifo,O_WRONLY);
				 write(fd1,"NULL",strlen("NULL"));
				 close(fd1);
				 printf("Gonderdim --> %s\n", "NULL");
				 continue;
			 }
			 else
			 {
				char ** splitted_condition = split(*(splitted+5),"=");//='e göre ayır ad=selin=> ad, selin gibi
			 	
				char ** result = initResultArray(); //result bufferi initialize et.

				char *line = calloc(LINE_BUF_SIZE,sizeof(char));

				FILE * fp;
				fp = fopen(*(splitted+3),"r"); //ilgili dosyayıaç
				int i = 0;
				if(fp == NULL)
				{
					perror("Error opening file");
					free(line);
					free(splitted);
					free(result);
					return 0;
				}
				while(fgets(line,LINE_BUF_SIZE,fp) != NULL) //dosyayı line e oku
				{
					char * tempLine= calloc(LINE_BUF_SIZE,sizeof(char));
					strcpy(tempLine,line);
					char ** splittedLine = split(tempLine,ARGS_SPLIT_CHARS); //line i split et
					
					if(condition == NO_CONDITION)//eğer koşul verilmediyse
					{
						if(queryType == BRING_ALL)
						{
							strcpy(*(result+i),line);
							i++;
						}
						else if(queryType == BRING_NUMBER)
						{
							strcpy(*(result+i),*(splittedLine+1));
							i++;
						}
						else if(queryType == BRING_NAME)
						{
							strcpy(*(result+i),*(splittedLine));
							i++;
						}
					}
					else
					{
					  if(condition == NUMBER_CONDITION)
						{
							if(strcmp(*(splittedLine+1),*(splitted_condition+1)) == 0)
							{
								if(queryType == BRING_ALL)
								{
									strcpy(*(result+i),line);
									i++;
								}
								else if(queryType == BRING_NUMBER)
								{
									strcpy(*(result+i),*(splittedLine+1));
									i++;
								}
								else if(queryType == BRING_NAME)
								{
									strcpy(*(result+i),*(splittedLine));
									i++;
								}
							}
						}
						else if(condition == NAME_CONDITION)
						{
							if(strcmp(*(splittedLine),*(splitted_condition+1))==0)
							{
								if(queryType == BRING_ALL)
								{
									strcpy(*(result+i),line);
									i++;
								}
								else if(queryType == BRING_NUMBER)
								{
									strcpy(*(result+i),*(splittedLine+1));
									i++;
								}
								else if(queryType == BRING_NAME)
								{
									strcpy(*(result+i),*(splittedLine));
									i++;
								}
							}
						}
					}
					memset(line,'\0',LINE_BUF_SIZE);
				}
				*(result+i) = NULL;
				close(fp);
				int j = 0;
				int l = 0;
				char sendingArray[MESSAGE_SIZE];//geri yollanacak arrayi oluştur. 
				fflush(stdout);
				while(*(result+j) != NULL)//resultta ki bütun datayı bu arraye at
				{
					int k;
					for(k=0;k<strlen(*(result+j));k++)
					{
						sendingArray[l] = *(*(result+j)+k);
						l++;
					}
					if(sendingArray[l-1] != '\n')
					{
						sendingArray[l] = '\n';
						l++;
					}

					j++;
		 		}
				printf("\n%s", sendingArray);
				fd1 = open(myfifo,O_WRONLY);
				if(strlen(sendingArray) == 0) //eğer strlen 0 ise NULL döndür
				{
					write(fd1,"NULL",strlen("NULL"));
					close(fd1);

				}
				else //değilse sending array i yolla
				{
					write(fd1,sendingArray,strlen(sendingArray));
			    close(fd1);
				}

				memset(sendingArray,'\0',MESSAGE_SIZE);
				freeResultBuffer(result);
		 }
  }
}
  return 0;
}

/*This function declares the type of query if is NOT_VALID,BRING_ALL,BRING_NUMBER or BRING_NAME.
	the details of this types writed detailly in defining enum QueryType
*/
enum QueryType declareQueryType(char ** splittedQuery)
{
  enum QueryType queryType;
  if((*(splittedQuery) == NULL)|| (*(splittedQuery+1) == NULL)||
    (*(splittedQuery+2) == NULL)||(*(splittedQuery+3) == NULL))
  {
    queryType = NOT_VALID_QUERY;
    return queryType;
  }
	if((strcmp(*splittedQuery,"select")!=0)&&(strcmp(*(splittedQuery+2),"from")!=0))
  {
  	queryType = NOT_VALID_QUERY;
  }
  else if((strcmp(*(splittedQuery+3),"veri1.txt")!=0) && (strcmp(*(splittedQuery+3),"veri2.txt")!=0) )
  {
  	queryType = NOT_VALID_QUERY;
  }
  else
  {
	 	if(strcmp(*(splittedQuery+1),"*")==0)
	 	{
	 		queryType = BRING_ALL;
	 	}
	 	else if(strcmp(*(splittedQuery+1),"number")==0)
	 	{
	 		queryType = BRING_NUMBER;
	 	}
	 	else if(strcmp(*(splittedQuery+1),"ad")==0)
	 	{
	 		queryType = BRING_NAME;
	 	}
	 	else
	 	{
	 		queryType = NOT_VALID_QUERY;
	 	}
  }
  return queryType;
}
/*This function specifies the condition if is NOT_VALID, NUMBER or NAME.
	this condition types defined detailly in the defining of enum named ConditionType.
*/
enum ConditionType specifyCondition(char** splittedQuery)
{
	enum ConditionType condition;

	if(strcmp(*(splittedQuery+4),"where")!=0)
	{
		condition = NOT_VALID_CONDITION;
	}
	else
	{
		if(*(splittedQuery+5) == NULL || *(splittedQuery+4) ==NULL)
		{
			return NOT_VALID_CONDITION;
		}
		char receivedConditions[25];
		strcpy(receivedConditions,*(splittedQuery+5));
		char ** queryConditions= split(receivedConditions,"=");

		if(strcmp(*(queryConditions),"ad")==0)
		{
			condition = NAME_CONDITION;

		}
		else if(strcmp(*(queryConditions),"number")==0)
		{
			condition = NUMBER_CONDITION;
		}
		else
		{
			condition = NOT_VALID_CONDITION;
		}
		free(queryConditions);
	}
	return condition;
}
/*
	This function initializes the result array.
*/
char ** initResultArray()
{
	char **result_buffer = calloc(RESULT_BUF_SIZE,sizeof(char*));
	int k;
	for ( int k= 0; k< RESULT_BUF_SIZE; k++)
	{
		*(result_buffer+k) = calloc(LINE_BUF_SIZE,sizeof(char));
	}
	return result_buffer;
}
void freeResultBuffer(char ** result)
{
	int k;

	for ( int k= 0; k< RESULT_BUF_SIZE; k++)
	{
		free(*(result+k));
	}
	free(result);
}
char ** split(char * cmd,char * split_char)
//bu fonk. verilen split chara gore hem komutları hem tokenleri ayırmak
//icin kullanılır. CMDS_SPLIT_CHAR  girilirse | ile komutları ayırır.ARGS_SPLIT_CHARS girilirse tokenleri ayırır.
{
	int index = 0;
	int buffer_size = TOKENS_BUF_SIZE;
	char **tokens = malloc(buffer_size* sizeof(char*));
	char * token;
	if(!tokens)
	{
		exit(EXIT_FAILURE);
	}
	token = strtok(cmd,split_char);//bosluklara gore ayır tokenleri
	while(token != NULL)
	{
		tokens[index] = token;
		index++;

		if(index >= buffer_size)//eger ayrılan token alanı az geldiyse
		{
			buffer_size += TOKENS_BUF_SIZE;//alanı genişlet buf size kadar
			tokens = realloc(tokens,buffer_size*sizeof(char*));//tekrar yeni alana gore allocate et
			if(!tokens)//realloc isleminden sonra allocationda sıkıntı oldumu
			{
				exit(EXIT_FAILURE); // failure degeriyle cik
			}
		}
	    token = strtok(NULL,split_char);//bir sonraki argumana (jetona) gec. strtok buffer da kalan inputu tutar
	    								//null ile tekrar cagrilmasi halinde kalan dizide bir daha ayırma yapar.
	}
	tokens[index] = NULL; // dizinin son degerini NULL ile bitir..
	return tokens;
}
