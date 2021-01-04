
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>

char * readline(void);

#define QUERY_INP_BUF_SIZE 100
#define READ_LINE_BUFSIZE 64


int main(){

  int fd;
  char * myfifo = "/tmp/myfifo";
  mkfifo(myfifo,0666);
  char query[QUERY_INP_BUF_SIZE];
  char received[QUERY_INP_BUF_SIZE];
  memset(received,'\0',QUERY_INP_BUF_SIZE);

  while(1)
  {
    fflush(stdout); //stdout bufferi temizle
    memset(query,'\0',QUERY_INP_BUF_SIZE);//query bufferi boşalt
    memset(received,'\0',QUERY_INP_BUF_SIZE); // received bufferi boşalt
    printf("Enter the query:");
    char * input = readline(); //satir oku
    strcpy(query,input);  //pipe a yazmak için bir arreye kopyala.
    fd = open(myfifo,O_WRONLY);//named pipe aç
  	write(fd,query,strlen(query)); //sorguyu named pipe a yaz
    close(fd);
    fd = open(myfifo,O_RDONLY);//named pipe i okumak için aç
 	  read(fd,received,QUERY_INP_BUF_SIZE);//named pipe dan gelen veriyi okuyup receivede yaz
    close(fd);
    printf("\nAldım --> \n%s", received);//alınan verileri göstermek amaçlı output
    printf("\nDosyayı kaydetmek istiyor musunuz? e/h?");
  	char * answer = readline();//kaydetmeye olan cevabı e/h oku
  	if(strcmp(answer,"e") == 0) // eger cevap e ise ...
  	{
	  	int f;
	  	int e;
	  	int pipefd[2];
	  	if(pipe(pipefd)<0)
	  	{
	  		perror("pipe");
	  		exit(1);
	  	}
  		f = fork();
  		if(f ==0 )
  		{
  			write(pipefd[1],received,strlen(received));//child ise pipe a gelen sonucu yaz
  			e = execv("kaydet",NULL); //kaydet programını çalıştır
        close(pipefd[1]);
  		}
  		else
  		{
  			wait(&e);
        close(pipefd[1]);
        close(pipefd[0]);
  		}
  	}
    free(answer);
    free(input);
  }
  return 0;
}

char * readline(void)
{
	int bufsize = READ_LINE_BUFSIZE;
	int index = 0;
	char *buffer = calloc(bufsize,sizeof(char));
	int c;

	if (!buffer)
	{
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		// Bir karakter oku
		c = getchar();
		// Eger satir sonu gelmişse veya dosya sonu gelmişse, o indexi null \0 ata ve döndur
		if (c == EOF || c == '\n')
		{
			buffer[index] = '\0';
			return buffer;

		}
		else
		{
			buffer[index] = c;
		}
		index++;
		//Her bir karakter bir byte oldugundan indeximiz kadar byte yer tutuyor demektir. Ve girilen index
		//ayrılan baytı gectiyse tekrardan realloc ile biraz daha yer almamız gerekir.
		if (index >= bufsize)
		{
			bufsize += READ_LINE_BUFSIZE;
			buffer = realloc(buffer, bufsize);
			if (!buffer)
			{
				exit(EXIT_FAILURE);
			}
		}
	}

}
