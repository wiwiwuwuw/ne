#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

#define MJESTA_CEKAONICA 4
#define START_BROJ_KLIJENATA 4
#define NAKNADNI_BROJ_KLIJENATA 6
#define RADNO_VRIJEME 45

sem_t *otvorenSalonsem;
sem_t *zatvorenSalonsem;
sem_t *klijentUzetsem;
sem_t *klijentGotovsem;
sem_t *klijentDosaosem;
sem_t *frizSlobodnasem;

int *otvoreno;
int *zatvoreno;
int *cekaonica;
int *idd;
int *idNo;

void radnovrijeme(int sig)
{
    *zatvoreno = 1;
    sem_post(zatvorenSalonsem);
    *otvoreno = 0;
}


void frizerka(void)
{
    *otvoreno = 1;
    sem_post(otvorenSalonsem);
    printf("Frizerka: Otvaram salon\n");
    printf("Frizerka: Postavljam znak OTVORENO\n");
    printf("Frizerka: spavam dok klijenti ne dodu\n");
    sleep(1);
    while(1){
        if(*zatvoreno){
            printf("Frizerka: Zatvaram salon\n");
            printf("Frizerka: Postavljam znak ZATVORENO\n");
            printf("ZATVORENO\n");
            return;
        }
        if(*cekaonica != 0){
            sem_wait(klijentDosaosem);
            sem_post(klijentUzetsem);
            printf("Frizerka: Idem raditi na klijentu %d\n", ++(*idNo));
            sleep(3);
            sem_post(frizSlobodnasem);
            printf("Frizerka: Klijent %d gotov\n", *idNo);
        }
        if (*zatvoreno != 0 && *cekaonica == 0){
            printf("Frizerka: spavam dok klijenti ne dodu\n");
            sleep(3);
        }
        /*else{
            sem_wait(&zatvorenSalonsem);
        }*/
    }
    return;
}


void klijent(int p)
{
    
    *idd = p;
    printf("\tKlijent(%d): Želim na frizuru\n", *idd);
    
    if(*zatvoreno == 0 && (*cekaonica < MJESTA_CEKAONICA)){
        printf("\tKlijent(%d): Ulazim u čekaonicu\n", *idd);
        *(cekaonica)++;
        sem_post(klijentDosaosem);

        sem_wait(frizSlobodnasem);
        sem_wait(klijentUzetsem);
        printf("\tKlijent(%d): Frizerka mi radi frizuru\n", *idNo);
        
        *(cekaonica)--;
    }
    else{
        printf("\tKlijent(%d): Nema mjesta u čekaoni, vratit ću se sutra\n", *idd);
        return;
    }
    return;

}

int main ()
{
    struct sigaction act;

	act.sa_handler = radnovrijeme;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGALRM, &act, NULL);

    alarm(RADNO_VRIJEME);

	int ID1, ID2, ID3, ID4, ID5,ID6, ID7, ID8, ID9, ID10, ID11, i, id[START_BROJ_KLIJENATA];
	pid_t pid;

	ID1 = shmget (IPC_PRIVATE, sizeof(sem_t), 0600);
	otvorenSalonsem = shmat(ID1, NULL, 0);
	shmctl(ID1, IPC_RMID, NULL);
    ID2 = shmget (IPC_PRIVATE, sizeof(sem_t), 0600);
	zatvorenSalonsem = shmat(ID2, NULL, 0);
	shmctl(ID2, IPC_RMID, NULL);
    ID3 = shmget (IPC_PRIVATE, sizeof(sem_t), 0600);
	klijentUzetsem = shmat(ID3, NULL, 0);
	shmctl(ID3, IPC_RMID, NULL);
    ID4= shmget (IPC_PRIVATE, sizeof(sem_t), 0600);
	klijentGotovsem = shmat(ID4, NULL, 0);
	shmctl(ID4, IPC_RMID, NULL);
    ID5 = shmget (IPC_PRIVATE, sizeof(sem_t), 0600);
	klijentDosaosem = shmat(ID5, NULL, 0);
	shmctl(ID5, IPC_RMID, NULL);
    ID6 = shmget (IPC_PRIVATE, sizeof(sem_t), 0600);
	frizSlobodnasem = shmat(ID6, NULL, 0);
	shmctl(ID6, IPC_RMID, NULL);
    ID7 = shmget (IPC_PRIVATE, sizeof(int), 0600);
	otvoreno = (int *)shmat(ID7, NULL, 0);
	shmctl(ID7, IPC_RMID, NULL);
    ID8 = shmget (IPC_PRIVATE, sizeof(int), 0600);
	zatvoreno = (int *)shmat(ID8, NULL, 0);
	shmctl(ID8, IPC_RMID, NULL);
    ID9 = shmget (IPC_PRIVATE, sizeof(int), 0600);
	cekaonica = (int *)shmat(ID9, NULL, 0);
	shmctl(ID9, IPC_RMID, NULL);
    ID10 = shmget (IPC_PRIVATE, sizeof(int), 0600);
	idd = (int *)shmat(ID10, NULL, 0);
	shmctl(ID10, IPC_RMID, NULL);
    ID11 = shmget (IPC_PRIVATE, sizeof(int), 0600);
	idNo = (int *)shmat(ID11, NULL, 0);
	shmctl(ID11, IPC_RMID, NULL);
    
    sem_init(otvorenSalonsem, 1, 0);
    sem_init(zatvorenSalonsem, 1, 0);
    sem_init(klijentUzetsem, 1, 0);
    sem_init(klijentGotovsem, 1, 0);
    sem_init(klijentDosaosem, 1, 0);
    sem_init(frizSlobodnasem, 1, 1);

    int *otvoreno = 0;
    int *zatvoreno = 0;
    int *cekaonica = 0;
    int *idd = 0;
    int *idNo = 0;

    pid_t pidF;

    if(fork() == 0){
        frizerka();;
        exit(0);
    }
    
	for (i = 0; i < START_BROJ_KLIJENATA; i++ ) {
		pid = fork();
		if ( pid == 0 ) {
			id[i] = i + 1;
			klijent (id[i]);
            sleep(3);
		}
		else if (pid == -1) {
			perror("Greska pri stvaranju procesa");
			exit(1);
		}
	}

    sleep(3);
    for (i = 0; i < NAKNADNI_BROJ_KLIJENATA; i++ ) {
		pid = fork();
		if ( pid == 0 ) {
			id[i] = START_BROJ_KLIJENATA + i + 1;
			klijent (id[i]);
		}
		else if (pid == -1) {
			perror("Greska pri stvaranju procesa");
			exit(1);
		}
	}

	
	for (i = 0; i < START_BROJ_KLIJENATA; i++ )
		wait(NULL);
    for (i = 0; i < NAKNADNI_BROJ_KLIJENATA; i++ )
		wait(NULL);

	sem_destroy(otvorenSalonsem);
    sem_destroy(zatvorenSalonsem);
    sem_destroy(klijentUzetsem);
    sem_destroy(klijentGotovsem);
    sem_destroy(klijentDosaosem);
    sem_destroy(frizSlobodnasem);

	shmdt(otvorenSalonsem);
    shmdt(zatvorenSalonsem);
    shmdt(klijentUzetsem);
    shmdt(klijentGotovsem);
    shmdt(klijentDosaosem);
    shmdt(frizSlobodnasem);
    shmdt(otvoreno);
    shmdt(zatvoreno);
    shmdt(cekaonica);
    shmdt(idd);
    shmdt(idNo);

	return 0;
}
