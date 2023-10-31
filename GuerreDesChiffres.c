#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>

static bool flag_de_fin = false;

static int *tampon;
static int sizeTampon;
static int sommeProduit = 0;
static int sommeConsommee = 0;

static int ip = 0;
static int ic = 0;

static sem_t libre;
static sem_t occupe;
static sem_t mutex;

// fonction exécutée par les producteurs
void *producteur(void *pid)
{
   int nbGenere = 0;
   srand(time(NULL));

   while (1)
   {
      sem_wait(&libre);
      int chiffreRandom = (rand() % 9) + 1;

      sem_wait(&mutex);
      tampon[ip] = chiffreRandom;
      sommeProduit += tampon[ip];
      ip = (ip + 1) % sizeTampon;
      sem_post(&mutex);

      nbGenere++;
      sem_post(&occupe);

      if (flag_de_fin)
         break;
   }

   pthread_exit(&nbGenere);
   return NULL; // A ENLEVER!!!!
}

// fonction exécutée par les consommateurs
void *consommateur(void *cid)
{
   int nbConsomme = 0;
   int objRecupere = 0;

   while (1)
   {
      sem_wait(&occupe);

      sem_wait(&mutex);
      objRecupere = tampon[ic];
      sommeConsommee += objRecupere;
      ic = (ic + 1) % sizeTampon;
      sem_post(&mutex);

      if (objRecupere == 0)
         break;

      nbConsomme++;
      sem_post(&libre);
   }

   pthread_exit(&nbConsomme);
   return NULL; // A ENLEVER!!!!
}

void actionAlarme()
{
   flag_de_fin = true;
}

// fonction main
int main(int argc, char *argv[])
{
   int numProd = atoi(argv[1]);
   int numCons = atoi(argv[2]);
   sizeTampon = atoi(argv[3]);

   pthread_t *prodThreads = malloc(numProd * sizeof(pthread_t));
   pthread_t *consThreads = malloc(numCons * sizeof(pthread_t));
   tampon = malloc(sizeTampon * sizeof(int));

   sem_init(&mutex, 0, 1);
   sem_init(&libre, 0, sizeTampon);
   sem_init(&occupe, 0, 0);

   // Creation des threads producteur:
   for (size_t i = 0; i < numProd; i++)
      pthread_create(&prodThreads[i], NULL, producteur, (void *)i);

   // Creation des threads consommateur:
   for (size_t i = 0; i < numCons; i++)
      pthread_create(&consThreads[i], NULL, consommateur, (void *)i);

   // Lancement de l'alarme systeme:
   signal(SIGALRM, actionAlarme);
   alarm(1);

   // Attente de la fin des treads producteur:
   int nbTotalProduction = 0;
   for (size_t i = 0; i < numProd; i++)
   {
      int *exit_status;
      pthread_join(prodThreads[i], (void **)&exit_status);
      nbTotalProduction += *exit_status;
   }

   // Mettre des zeros pour chaque consommmateur:
   for (size_t i = 0; i < numCons; i++)
   {
      tampon[ip] = 0;
      ip = (ip + 1) % sizeTampon;
   }

   // Attente de la fin des treads consommateurs:
   int nbTotalConsommation = 0;
   for (size_t i = 0; i < numCons; i++)
   {
      int *exit_status;
      pthread_join(consThreads[i], (void **)&exit_status);
      nbTotalConsommation += *exit_status;
   }

   printf("Le nombre production: %d \n", nbTotalProduction);
   printf("Le nombre consommation: %d \n", nbTotalConsommation);

   printf("Le sommeProduit: %d \n", sommeProduit);
   printf("Le sommeConsommer: %d \n", sommeConsommee);

   free(prodThreads);
   free(consThreads);
   free(tampon);

   return 0;
}
