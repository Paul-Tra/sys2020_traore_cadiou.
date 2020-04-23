
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <errno.h>
#include "mfile.h"
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/stat.h> /* Pour les constantes « mode » */
#include <fcntl.h> /* Pour les constantes O_* */ 
#define LEN 512


void Init(void)
{
	printf("Projet System 2020 , TRAORE - CADIOU :\n\n");
}

mfifo * mfifo_connect( const char *nom, int options, mode_t permission, size_t capacite ){

	if ( !capacite || capacite == 0 ){
		perror("Capacité NULL ou = 0 , Erreur");
		return NULL;
	}
	mfifo * fifo = malloc(sizeof(mfifo)) ;
	/* debut fifo correspondant au retour de malloc */
	if ( nom != NULL ){
		fifo->nom = nom ;
	}
	else {
		/* mFifo anonyme */
		void * addr = mmap(NULL, capacite, PROT_READ | PROT_WRITE,  MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		if (addr == MAP_FAILED){
            perror("mmap");
           	return NULL;
        }
		fill_mfifo(fifo,(size_t) addr, capacite);
	    return fifo ;
	}
	if ( nom != NULL ){
		int fd ;
		char * name = malloc(sizeof(char)*(sizeof(nom)+2));
		strcat(name, "/");
		strcat(name, nom);
		switch(options){
			case 0 :
				printf("\nCONNEXION ....\n");
				if( permission !=0){
					mfifo * res = connexion_mfifo_nomme(name, capacite, permission); 
					if(  res == NULL){
						printf("echec de connexion \n");
						return NULL;
					}
					printf("Contenu memoire au retour de conecxion nomme  : %s\n", res->memory );
					return res ;
				}
				break;

			case O_CREAT :
				if ( permission != 0  ){
					mfifo * res = creation_mfifo_nomme( name, capacite, permission);
					if( res == NULL){
						res = connexion_mfifo_nomme(name, capacite, permission);
						if( res == NULL ){
							printf("echec de conne\n");
							return NULL;
						}
					}
					return res;
				}					
				break;

			case O_CREAT|O_EXCL :
				if( permission != 0){
					printf("excl\n");
					mfifo * res = creation_mfifo_nomme( name, capacite, permission);
					printf("fin create\n");
					if( res == NULL){
						printf("echec de création \n");
						return NULL;
					}
					return res;
				}
				break;
		}
	}
	return fifo ;
}

/**
* Gère la connexion d'un mfifo nommé
*
*/
mfifo * connexion_mfifo_nomme(char * name, size_t capacite, mode_t permission){
	mfifo * res = malloc(sizeof(mfifo)+capacite);
	int fd = shm_open(name, O_RDWR, permission);
	if( fd == -1 ){
		perror("shm open ");
		exit(1);
	}
	struct stat buf_stat;
	if (fstat(fd, &buf_stat) == -1) {
    	perror(" fstat ");
    	exit(1);
    }
	res = mmap(NULL, capacite, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if( res == MAP_FAILED){
		//mfifo_unlink(name);
		perror("O - mmap");
		exit(1);
	}
    close(fd);
	printf("Dans 0 , adresse du fifo : %p\nReturn Fifo.\n", res );
	printf("Contenu memoire a la connexion : %s\n", res->memory );
    fill_mfifo(res,(size_t) res , capacite);
   	printf("Contenu memoire a la connexion : %s\n", res->memory );

    return res;
}

mfifo * creation_mfifo_nomme(char * name, size_t capacite, mode_t permission){
	printf("dans creation nomme\n");
	mfifo * res = malloc(sizeof(mfifo)+capacite);
	printf("malloc\n");	
	int fd = shm_open(name, O_CREAT | O_RDWR | O_EXCL, permission );
	if(fd == -1){
		perror("shm open ");
		return NULL;
	}
	struct stat buf_stat;
	if (fstat(fd, &buf_stat) == -1) {
		perror(" fstat ");
    	exit(1);
	}
	printf("stat\n");
	if( buf_stat.st_size == 0 && ftruncate( fd, capacite ) == -1){
		perror("Ftruncate");
		exit(1);
	}
	printf("truncate\n");
  	res = mmap(NULL, capacite, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if( res == MAP_FAILED){
	    //mfifo_unlink(name);
	    perror("O_CREAT - mmap");
	    exit(1);
  	}
	close(fd);
	printf("Dans O_create fifo_Vide , adresse du fifo : %p\nReturn Fifo.\n", &res[0] );
	fill_mfifo(res,(size_t) res , capacite);
	init_memory_mfifo(res);

	return res;
}

/**
* Remplie un objet mfifo par diverses 
*
* @param fifo 		objet mfifo à remplir
* @param addr 		adresse de début de fifo
* @param capacite 	capacité totale de fifo
*/
void fill_mfifo(mfifo * fifo, size_t addr, size_t capacite){
	printf("%d\n",capacite );
	//realloc(fifo, sizeof(mfifo)+capacite);
	fifo->debut = addr ;
	fifo->fin = fifo->debut + capacite ;
	fifo->capacity = capacite;
	fifo->pid = -1 ;
	//*fifo->memory = addr ;
	printf("fill_fifo : memory : %s\n", fifo->memory );

	//*(fifo->memory) = &addr ;
	if(sem_init(&fifo->sem,1,1) == -1 ){
		perror("sem init 210 ");
	}
}

/**
* Initialise la memoire d'un mfifo
*
* @param fifo objet fifo dont il faut initialiser la memoire
*/
void init_memory_mfifo(mfifo * fifo){
	memset(fifo->memory, 0, strlen(fifo->memory));
}

	/*
	mfifo_write() bloque le processus appelant jusqu’à ce que len octets soient écrits dans
	fifo. Les octets écrits ne doivent pas être mélangés avec les octets écrits par d’autres
	processus, et le processus appelant reste donc bloqué tant qu’il n’y a pas de place pour
	len octets dans fifo.
	*/
int mfifo_write(mfifo *fifo, const void *val, size_t len){
	size_t cpt = mfifo_free_memory(fifo);
	// On test que LEN est bien < fifo->capacite .
	if ( len > fifo->capacity || cpt < len ){
		perror("Erreur , Len > fifo->capacite\n");
		printf("len : %d, fifo->capacity : %d, cpt : %d \n",len,fifo->capacity, cpt );
		errno = EMSGSIZE;
		return -1;
	}
	//memcpy(&fifo->memory[0] , val , len) ;
	for ( int i = 0 ; i < len ; i++ ){
		memcpy(&fifo->memory[i] , &val[i] , 1 ) ; 
	}
	//snprintf(fifo->memory, strlen(fifo->memory, "%d %s\n", last_idx, val);
	//snprintf(fifo->memory+strlen(fifo->memory), 0, "%s", val);
    printf("Content : %s a l'adresse : %p \n",fifo->memory, fifo->memory );
    msync(fifo,fifo->capacity,MS_SYNC);
	return len;
}

/*
Supposons qu’un mfifo contient n octets et l = min(len, n). La fonction mfifo_read()
copie l octets de mfifo à l’adresse buf. Les octets copiés sont supprimés du mfifo et
mfifo_read retourne le nombre d’octets copiés.
S’il y plusieurs processus lecteurs qui tentent de lire en même temps, tous sauf un seront
bloqués en attendant la fin de la lecture du seul processus autorisé à lire. Chaque lecture
lit donc un segment contigu d’octets stockés dans le mfifo.
*/

ssize_t mfifo_read(mfifo *fifo, void *buf, size_t len){
	if( len > fifo->capacity - mfifo_free_memory(fifo)){
		/* on veut lire plus d'octet qu'il n'en est contenu dans fifo */
		printf("on veut lire plus d'octet qu'il n'en est contenu dans fifo\n" );
	}
	realloc(buf, sizeof(char)*len);
	snprintf(buf, len, "%s", fifo->memory);
	printf("buf : %s\n",buf );
}


/**
* Verrouille le mfifo pour la lecture, fonction bloquante, attend tend que 
* le verrou n'a pas été posé sur l'objet mfifo
*
* @param fifo objet mfifo sur lequel nous voulons poser un verrou
* @return -1 en cas d'echec , 0 sinon
*/
int mfifo_lock(mfifo *fifo){
	/*ajout du pid du processus appelant dans la queue de la semaphore*/
	if(sem_wait(&fifo->sem) == -1){
		perror("sem wait ");
		return -1;
	}
	return 0;
}

/**
*	Essais de vérouiller l'objet mfifo
*
* @param 	fifo objet mfifo sur lequel nous voulons essayer de poser un verrou
* @return 	revoie 0 si l'obtention du verrou réussi, -1 sinon
*/
int mfifo_trylock(mfifo *fifo){
	if(sem_trywait(&fifo->sem) == -1){
		perror("sem trywait");
		return -1;
	}
	return 0;
}

/**
* Déverrouille l’accès au mfifo en lecture
*
* @param fifo 	objet mfifo dont il faut lever un verrou
* @return     	renvoie 0 lors de la libération du verrou , -1 en cas d'échec
*/
int mfifo_unlock(mfifo *fifo){
	if(sem_post(&fifo->sem) == -1 ){
		perror("sem post  ");
		return -1;
	}
	return 0;
}

/*
* Informe de la capacité totale de l'objet mfifo
*
* @param fifo 	objet mfifo dont l'on savoir la capacité de stockage
* @return 		renvoie la capicité totale de stockage de fifo
*/
size_t mfifo_capacity(mfifo *fifo){
	return fifo->capacity;
}

/*
* Libere la mémoire allouer par l'objet fifo et ses diverses champs
*
* @param 	fifo ojbet mfifo dont l'on veut libérer la mémoire
* @return 	renvoie la taille du fifo apres libération de mémoire
*/
int free_mfifo(mfifo *fifo){
	printf("dans free mfifo\n");
	free(&fifo->nom);
	if( strlen(fifo->memory) !=0 ){
		free(fifo->memory);
	}
	if(sem_destroy(&fifo->sem) == -1){
		perror("destroy semaphore ");
		return -1;
	}
	return sizeof(fifo);
}

/**
* Renvoie la quantié de mémoire libre de l'objet mfifo
*
* @param fifo 	objet mfifo dont l'on veut savoir la quantité de mémoire libre
* @return 		renvoie la quantité de mmémoire libre de fifo
*/
size_t mfifo_free_memory(mfifo *fifo){
	return (fifo->capacity - strlen(fifo->memory) );
}

/**
* Cette fonction déconnect, rend inutilisable, un objet mfifo, retourn -1 en
* cas d'erreur sinon 0
*
* @param fifo	objet mfifo à rendre ne plus utiliser
* @return 		renvoie 0 en cas de succès, -1 sinon
*/
int mfifo_disconnect(mfifo *fifo){
	if( munmap(fifo->memory , fifo->capacity) == -1){
		perror("munmap ");
		return -1;
	}
	return 0;
}

/**
* Supprime un objet mfifo par son nom 
*
* @param nom 	nom de l'objet mfifo à supprimer
* @return 		retourne -1 en cas d'erreur sinon 0
*/
int mfifo_unlink(const char * nom){
	printf("dans unlink : nom := %s \n",nom );
	int r = shm_unlink(nom);
	if( r == -1){
		perror("shm unlink");
	}
	return r;
}

/**
* Créer un objet message
*
* @param buf contenu du message à créer
*/
void create_message(char * buf, message * res){
	printf("dans create  message\n");
	int taille = sizeof(message)+strlen(buf)+1;
	realloc(res,taille);
	printf("malloc\n");
	res->l = strlen(buf)+1;
	printf("len : %ld\n", res->l);
	memset(res->mes, 0, strlen(res->mes));
	memmove(res->mes,buf,res->l);
	printf("mem : %s\n", res->mes);
	//printf("l %ld\n",strlen(res->l) );
	printf("l : %ld + contenu : %ld = %ld\n",sizeof(res->l), res->l, sizeof(res) );
	//return res;
}