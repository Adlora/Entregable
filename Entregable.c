#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

/*
 * Implementación de 
 *  ps aux | grep root | wc -l
 * Para compilar:
 * gcc -Wall -g -o pipe_exec_doble pipe_exec_doble.c
 */

int main(int argc, char **argv){

  int fd[2];
  int fd2[2];
  
  pid_t childpid=0;
  pid_t childpid2 = 0;
  
  if (pipe(fd) < 0){
    perror("Error en llamada a pipe (fd)\n");
    exit(1) ;
  }
  
  childpid = fork();

  if (childpid < 0){
    perror("Error en llamada a fork (hijo)\n");
    exit(1) ;
  }

  if (childpid == 0) {

    /* grep es el hijo */

    if (pipe(fd2) < 0){
      perror("llamada a pipe\n");
      exit(1) ;
    }
    
    childpid2 = fork();

    if (childpid2 < 0){
      perror("2a llamada a fork\n");
      exit(1) ;
    }

    if (childpid2 ==0){

      printf("PID del ps: %i\n",getpid());
      printf("STDOUT_FILENO: %i\n", STDOUT_FILENO);
      close(STDOUT_FILENO); /*Cerramos la salida estándar */

      /* Imprimo el descriptor de fichero */
      fprintf(stderr,"fd2[1]: %i\n",fd2[1]);

      dup2(fd2[1], STDOUT_FILENO);
      
      /* Observad que no cambian los descriptores de fichero */
      fprintf(stderr,"fd2[1]: %i\n",fd2[1]);
      fprintf(stderr,"STDOUT_FILENO: %i\n",STDOUT_FILENO);

      /* 
	 Fijáos que si hiciésemos este printf 
	 ya no se imprime en salida estándar
	 porque ya está redireccionado:

	 printf("Imprimo stdout: STDOUT_FILENO: %i\n",STDOUT_FILENO);
       */

      
      close(fd2[0]);
      close(fd2[1]);
    
      /*Aquí son válidos:

	execlp("ps", "ps", "aux", NULL);
        o
	execl("/bin/ps", "ps", "aux", NULL);
	Si usáis execl tenéis que poner todo el path.
       */
      	execlp("ps", "ps", "aux", NULL);
      /*
	Tenemos que usar perror o fprintf en stderror porque ya tenemos
	redirigido stdout 
      */
      perror(strerror(errno));
    }
    else{
      printf("PID de grep %i\n",getpid());
      close(STDIN_FILENO); /*Cerramos la entrada estándar */
      close(STDOUT_FILENO); /*Cerramos la salida estándar */

      dup2(fd2[0], STDIN_FILENO);
      
      dup2(fd[1], STDOUT_FILENO);
    
      close(fd[0]);
      close(fd[1]);
      close(fd2[0]);
      close(fd2[1]);
    
    
      execl("/bin/grep", "grep", "root", NULL);
    
      perror("Exec fall o en grep");
    }
  } else {
    /* awk es el padre */
    printf("PID de wc %i\n",getpid());
    close(STDIN_FILENO); /*Cerramos la entrada estándar */

    dup2(fd[0], STDIN_FILENO);
    
    close(fd[0]);
    close(fd[1]);
    
    execl("/usr/bin/wc", "wc",  "-l",NULL);
    perror("Exec fall  o en wc");
  }
  exit(0);
}

