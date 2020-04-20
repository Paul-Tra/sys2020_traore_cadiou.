
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mfile.h"

#define LEN 512

int main(void)
{
	Init();
	printf("------ Creation PUIS Connexion : -------\n");
	mfifo * fifo = mfifo_connect("testBis",O_CREAT,0777,LEN);
	fifo = mfifo_connect("testBis",0,0777,LEN);
	int val;
	sem_getvalue(&fifo->sem, &val);

	printf("valeur semaphore main: %d \n",val );
	printf("Debut du pointeur fifo : %ld \n", fifo->debut );
	printf("Cap. fifo : %ld \n" , fifo->capacity);
	printf("capacity : %ld \n",mfifo_capacity(fifo) );
	printf("free memory : %ld \n",mfifo_free_memory(fifo) );
	printf("Fin du pointeur fifo : %ld \n", fifo->fin );
	printf("Pid du fifo : %d \n", fifo->pid );

	printf("\n------ Creation ET connexion : -------\n");
	mfifo * b = mfifo_connect("testBis",O_CREAT|O_EXCL,0777,LEN);

	printf("Debut du pointeur fifo : %ld \n", b->debut );
	printf("Cap. fifo : %ld \n" , b->capacity);
	printf("Fin du pointeur fifo : %ld \n", b->fin );
	printf("Pid du fifo : %d \n", b->pid );

	printf("On tente une ecriture dans mfifo : \n");
	sem_getvalue(&fifo->sem, &val);
	printf("valeur semaphore main: %d \n",val );
	char* buf = "Ceci est un test d'Ecriture" ;
	int res_write = mfifo_write(fifo,buf,strlen(buf));

	char* bu = "Coucou test" ;
	res_write = mfifo_write(fifo,bu,strlen(bu));

	sem_getvalue(&fifo->sem, &val);
	printf("valeur semaphore main: %d \n",val );
	printf("Res write : %d\n", res_write );	

	char * buf_read = malloc(sizeof(char)*strlen(buf)+1) ;

	size_t res_read = mfifo_read(fifo, buf_read,strlen(buf)+1);

	printf("\nRes read : %ld\n " , res_read);

	/*
	printf("\n\n------ Creation Tube anonyme : -------\n");
	mfifo * c = mfifo_connect(NULL,	O_CREAT|O_EXCL,0777,100);
	printf("Debut du pointeur fifo : %ld \n", c->debut );
	printf("Cap. fifo : %ld \n" , c->capacity);
	printf("Fin du pointeur fifo : %ld \n", c->fin );
	printf("Pid du fifo : %d \n", c->pid );
	*/

	printf("---- Test de suppression de mfifo : %s ------ \n\n", fifo->nom);


	printf("Etat avant suppression\n" );
	//printf("Contenu du dossier /dev/shm/ : \n" );
	//execlp("ls","ls","/dev/shm/",NULL);


	printf("deconnexion de : %s\n",fifo->nom );
	mfifo_disconnect(fifo);

	printf("suppression de : %s\n",fifo->nom );
	mfifo_unlink(fifo->nom);

	printf("Etat apres suppression\n" );
	printf("Contenu du dossier /dev/shm/ : \n" );
	execlp("ls","ls","/dev/shm/",NULL);


	return EXIT_SUCCESS;
}
					
