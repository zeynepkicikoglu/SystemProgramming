/*Verilen kodları değiştirmedim. Koda ek olarak Coordinates structı ve Coordinates move_animal fonksiyonunu yazdım.
Main de mevcut kodda oluşturulmuş olan Animal bear,bird,panda nesnelerini kullanarak threadler oluşturdum. srand(time(NULL))
kullanarak her program çalışmasında farklı random değerler oluşturdum. Simulateanimaldahayvan canlı ise ve simulasyon zamanı 
bitmediyse o threadde çalışmaya devam etmesi için while döngüsü kullandım. Daha sonra döngüde random pozisyon verip hangi 
siteda olduğunun kontrolünü yapıp o koordinattaki animal sayısını arttırıp oluşturduğum *mysite pointerı ile hayvanların 
sırasını bozmadan işaretçiyi son elemanı göstercek şekilde ayarladım. Bu şekilde program döngü bitene kadar devam eder ve 
en son gridin son halini basar. Simulatehunter da benzer şekilde kullanıcının verdiği sayı kadar mainde thread oluşturup 
simulasyon zamanı bitene kadar while döngüsünde o thread çalışır ve random pozisyon hareketi yapar olduğu sitelarda animal 
varsa o hayvanı öldürür ve nanimals sayısından azaltıp kendi pointerını arttırır. En son yine gridin son halini ekrana basar.
Çok nadir chatgptden yardım aldığım oldu (zaten her zaman dogru cevap vermedi). Daha çok ders bilgileriyle kodumu yazdım.
Linuxta çalıştırdım, çalıştı.*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define SIMULATION_TIME 1

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef enum { BEAR, BIRD, PANDA} AnimalType;

typedef enum { ALIVE, DEAD } AnimalStatus;

typedef struct {
    int x;
    int y;
} Location;

typedef enum { FEEDING, NESTING, WINTERING } SiteType;

typedef struct {
    /* animal can be DEAD or ALIVE*/
    AnimalStatus status;
    /* animal type, bear, bird, panda*/
    AnimalType type;
    /* its location in 2D site grid*/
    Location location;
} Animal;

Animal bear, bird, panda;

/* type of Hunter*/
typedef struct {
    /* points indicate the number of animals, a hunter killed*/
    int points;
    /* its location in the site grid*/
    Location location;
} Hunter;

/* type of a site (a cell in the grid)*/
typedef struct {
    /* array of pointers to the hunters located at this site*/
    Hunter **hunters;
    /* the number of hunters at this site*/
    int nhunters;
    /* array of pointers to the animals located at this site*/
    Animal **animals;
    /* the number of animals at this site*/
    int nanimals;
    /* the type of site*/
    SiteType type;
} Site;

/* 2D site grid*/
typedef struct {
    /* number of rows, length at the x-coordinate*/
    int xlength;
    /* number of columns, length at the y-coordinate*/
    int ylength;
    /* the 2d site array*/
    Site **sites;
} Grid;

/* initial grid, empty*/
Grid grid = {0, 0, NULL};


Grid initgrid(int xlength, int ylength) {
    grid.xlength = xlength;
    grid.ylength = ylength;

    grid.sites = (Site **)malloc(sizeof(Site *) * xlength);
    for (int i = 0; i < xlength; i++) {
        grid.sites[i] = (Site *)malloc(sizeof(Site) * ylength);
        for (int j = 0; j < ylength; j++) {
            grid.sites[i][j].animals = NULL;
            grid.sites[i][j].hunters = NULL;
            grid.sites[i][j].nhunters = 0;
            grid.sites[i][j].nanimals = 0;
            double r = rand() / (double)RAND_MAX;
            SiteType st;
            if (r < 0.33)
                st = WINTERING;
            else if (r < 0.66)
                st = FEEDING;
            else
                st = NESTING;
            grid.sites[i][j].type = st;
        }
    }

    return grid;
}


void deletegrid() {
    for (int i = 0; i < grid.xlength; i++) {
        free(grid.sites[i]);
    }

    free(grid.sites);

    grid.sites = NULL;
    grid.xlength = -1;
    grid.ylength = -1;
}


void printgrid() {
    for (int i = 0; i < grid.xlength; i++) {
        for (int j = 0; j < grid.ylength; j++) {
            Site *site = &grid.sites[i][j];
            int count[3] = {0}; /* do not forget to initialize*/
            for (int a = 0; a < site->nanimals; a++) {
                Animal *animal = site->animals[a];
                count[animal->type]++;
                
            }
            printf("|%d-{%d, %d, %d}{%d}|", site->type, count[0], count[1],
                   count[2], site->nhunters);
        }
        printf("\n");
    }
}

void printsite(Site *site) {
    int count[3] = {0}; /* do not forget to initialize*/
    for (int a = 0; a < site->nanimals; a++) {
        Animal *animal = site->animals[a];
        count[animal->type]++;
    }
    printf("|%d-{%d,%d,%d}{%d}|", site->type, count[0], count[1], count[2],
           site->nhunters);
}
typedef struct {
    int x;
    int y;
} Coordinates;

/* animaldan gelen pointer ile yeni konum belirlenir eğer o konum o gride uymuyorsa eski konumu geri yollar*/
Coordinates move_animal(Animal *myanimals) {
    
    int new_x, new_y;
    printf("Bu hayvan suan burada (%d,%d)\n",myanimals->location.x,myanimals->location.y);
    
    int rastgele = rand() % 4;  // 0 ile 3 arasında rastgele bir sayı üret case seçimi için 

    // Rastgele sayıya göre hareket et
    switch (rastgele) {
        case 0:
            new_x = myanimals->location.x + 1; 
            new_y = myanimals->location.y;
            break;
        case 1:
            new_x = myanimals->location.x - 1;
            new_y = myanimals->location.y;
            break;
        case 2:
            new_x = myanimals->location.x;
            new_y = myanimals->location.y + 1;  
            break;
        case 3:
            new_x = myanimals->location.x;
            new_y = myanimals->location.y - 1;  
            break;
        default:
            printf("default\n");
            break;
    }

    if (new_x >= 0 && new_x < grid.xlength && new_y >= 0 && new_y <grid.ylength) {
        myanimals->location.x = new_x;
        myanimals->location.y = new_y;
        printf("harbici new_x : %d,harbici new_y: %d \n",new_x,new_y);
    }
    Coordinates new_coordinates = {myanimals->location.x, myanimals->location.y};
    return new_coordinates;
}

/* verilen animal threadlere random pozisyon belirledikten sonra o pozisyondaki konumun site tipine göre 
hayvan durumunu ayarlar. hayvan ölene kadar ve sımulasyon zamanı bıtmedıgı surece o hayvan calısır*/
void *simulateanimal(void *args) {
    Animal* animal = (Animal*)args;
    pthread_mutex_lock(&mutex);
    time_t startTime = time(NULL);
    
    while (animal->status == ALIVE && difftime(time(NULL), startTime) < SIMULATION_TIME) {
    
        printf("Animal thread %d started\n", animal->type);
        
        int x = animal->location.x = rand() % grid.xlength;
        int y = animal->location.y = rand() % grid.ylength;
        printf("Animal %d started at (%d,%d).\n", animal->type, x, y);
    
        Site *mysite = &grid.sites[x][y];
        mysite->nanimals += 1;
        mysite->animals = realloc(mysite->animals, mysite->nanimals * sizeof(Animal *));
        mysite->animals[mysite->nanimals - 1] = animal;
    
        printsite(mysite);
        printf("\n");
        if(grid.sites[x][y].type == FEEDING){
            
            printf("FEEDİNG DEYİZ\n");
            sleep(1);
            float probability = 0.8;
            float random_number = (float) rand() / RAND_MAX; // generate random number between 0 and 1
            
            if (random_number < probability) {
                printf("Animal %d stays same location(%d,%d).\n",animal->type, x,y); // 80% probability of stays old location
            }
            else if(random_number > probability) { // moves to another location
                grid.sites[x][y].nanimals -= 1;
                
                Coordinates new_coordinates = move_animal(animal);
                printf("Animal-%d moved to (%d, %d)\n",animal->type, new_coordinates.x, new_coordinates.y);
                
                mysite = &grid.sites[new_coordinates.x][new_coordinates.y];
                mysite->nanimals += 1;
                mysite->animals = realloc(mysite->animals, mysite->nanimals * sizeof(Animal *));
                mysite->animals[mysite->nanimals - 1] = animal;
                
            }
            else{
                printf("Hata\n");
            }
	    
        }
        else if(grid.sites[x][y].type == NESTING){
            
            printf("NESTING DEYİZ\n");
            sleep(1);
            printf("Hemcinsi üredi.\n");
            Animal *new_animal = (Animal *)malloc(sizeof(Animal));
            new_animal->status = ALIVE;
            new_animal->type = animal->type;
            new_animal->location.x = animal->location.x;
            new_animal->location.y = animal->location.y;
            printf("%d tipteki yeni hayvan (%d,%d) yerinde dogdu\n",new_animal->type, new_animal->location.x, new_animal->location.y);
            pthread_mutex_unlock(&mutex);//eski çalışan threadi durdurur.
            
            pthread_t new_animal_thread;//yeni dogan hayvanın threadını baslatır.
            pthread_create(&new_animal_thread, NULL, simulateanimal, (void *)new_animal);
            pthread_join(new_animal_thread, NULL);
            
            printf("Yeni dogan bebek hayvan calisti.Kaldigimiz eski hayvandan devam.\n");
            
            sleep(1);

            Coordinates new_coordinates = move_animal(animal);
            mysite->nanimals -= 1;
            printf("Doguran eski Animal-%d moved to (%d, %d)\n", animal->type, new_coordinates.x, new_coordinates.y);
            mysite = &grid.sites[new_coordinates.x][new_coordinates.y];
            mysite->nanimals += 1;
            mysite->animals = realloc(mysite->animals, mysite->nanimals * sizeof(Animal *));
            mysite->animals[mysite->nanimals - 1] = animal;
        }
        else if(grid.sites[x][y].type == WINTERING){
            
            printf("WINTERING DEYİZ\n");
            sleep(1);
            int rand_num = rand() % 2;
            if(rand_num == 0){
                animal->status = DEAD;
                printf("Animal %d is dead.\n", animal->type);
                grid.sites[x][y].nanimals -= 1;
    ;
            }
            else if(rand_num == 1){
                grid.sites[x][y].nanimals -= 1;
                Coordinates new_coordinates = move_animal(animal);
                printf("Animal-%d moved to (%d, %d)\n",animal->type, new_coordinates.x, new_coordinates.y);
               
                mysite = &grid.sites[new_coordinates.x][new_coordinates.y];
                mysite->nanimals += 1;
                mysite->animals = realloc(mysite->animals, mysite->nanimals * sizeof(Animal *));
                mysite->animals[mysite->nanimals - 1] = animal;
            }
            else{
                printf("Hata\n");
            }
        }
        else{
                printf("Hata\n");
        }

        pthread_mutex_unlock(&mutex);

         
    }
    sleep(1);
    printgrid();
    return NULL;
}
/* verilen hunter threadleri rastgele konumda baslar o konumda hayvan varsa onları vurur sonra baska
konuma gider*/

void *simulatehunter(void *args) {
  
    static int hunter_index = 0;
    hunter_index++;
    Hunter* hunter  = (Hunter*)args;
    
    pthread_mutex_lock(&mutex);
    time_t startTime = time(NULL);
    
    while (difftime(time(NULL), startTime) < SIMULATION_TIME) {
        
        printf("Hunter thread %d started\n", hunter_index);
        
        int hunter_x = hunter->location.x = rand() % grid.xlength; 
        int hunter_y = hunter->location.y = rand() % grid.ylength; 
        printf("Hunter %d started at (%d,%d).\n", hunter_index, hunter_x, hunter_y);
        sleep(1);
        Site *mysite = (Site *)malloc(sizeof(Site));
        mysite = &grid.sites[hunter_x][hunter_y];
        mysite->nhunters += 1;
        mysite->hunters = realloc(mysite->hunters, (mysite->nhunters) * sizeof(Hunter *));
        mysite->hunters[mysite->nhunters - 1] = hunter;
    
        if (grid.sites[hunter_x][hunter_y].nanimals != 0){ //eger o konumda hayvan varsa
            hunter->points += grid.sites[hunter_x][hunter_y].nanimals;
            printf("Hunter killed %d animal.\n", grid.sites[hunter_x][hunter_y].nanimals);
            printf("Hunter %d points: %d\n",hunter_index, hunter->points);
            
            for(int i = 0 ; i< mysite->nanimals;i++){
                mysite->animals[i]->status = DEAD;
                printf("%d-Hayvanın Durum: %s\n",i, mysite->animals[i]->status == DEAD ? "DEAD" : "ALIVE");
            }
            mysite->nanimals = 0;
            
        }
        sleep(1);
        int new_x, new_y;
        int rastgele = rand() % 4;  // 0 ile 3 arasında rastgele bir sayı üret case seçimi için 
    
        // Rastgele sayıya göre hareket et
        switch (rastgele) {
            case 0:
                new_x = hunter_x + 1;
                new_y = hunter_y;
                break;
            case 1:
                new_x = hunter_x - 1;  
                new_y = hunter_y;
                break;
            case 2:
                new_x = hunter_x;
                new_y = hunter_y + 1;  
                break;
            case 3:
                new_x = hunter_x;
                new_y = hunter_y - 1;  
                break;
            default:
                printf("default\n");
                break;
        }
    
        if (new_x >= 0 && new_x < grid.xlength && new_y >= 0 && new_y <grid.ylength) {

            mysite->nhunters -= 1;//eski yerdeki hunter sayısını azalt
            
            hunter->location.x = new_x;
            hunter->location.y = new_y;
            printf("Hunter moved to (%d,%d) \n",new_x, new_y);
            mysite = &grid.sites[new_x][new_y];
            mysite->nhunters += 1;
            mysite->hunters = realloc(mysite->hunters, (mysite->nhunters) * sizeof(Hunter *));
            mysite->hunters[mysite->nhunters - 1] = hunter;
            mysite->nanimals = 0;
        }
        else{
            printf("Hunter stays same old place (%d,%d) because of random position does not fit the grid size. \n",hunter_x, hunter_y);
        }
        
        pthread_mutex_unlock(&mutex);
        
    }
    
    sleep(1);
    printgrid();
    return NULL; 

}
/* animal ve hunter threadleri oluşturulur. hunter thread sayısını kullanıcıdan alır. gridin son hali ekrana basılır*/
int main(int argc, char *argv[]) {
    
    if (argc != 2) { // ./main 2 creates 2 hunters program bu şekilde çalışıcaksa maks 2 değer gireceğinden onun kontrolü yapılır
        printf("Program yanlış başlatıldı.\n");
        return 1; //programı hatalı bitiriş
    } 
        
    int num_hunter_threads = atoi(argv[1]); // program çalıştırılırken verilen değer ,argv[1] stringi, integer'a dönüştürülüyor
    
    srand(time(NULL));
    
    initgrid(5, 5);
    printgrid();
    
    bear.status = ALIVE;
    bear.type = BEAR;
    bird.status = ALIVE;
    bird.type = BIRD;
    panda.status = ALIVE;
    panda.type = PANDA;

    pthread_t animalThreads[3];
    Animal* animals[3] = { &bear, &bird, &panda };
   
    for (int i = 0; i < 3; i++) {
        pthread_create(&animalThreads[i], NULL, simulateanimal, (void*)animals[i]);
        pthread_join(animalThreads[i], NULL);
        sleep(1);
    }
    
    int heartLimit = 3; // Bu sayıyı threadlerin yeniden çalıştırılacak sayısını belirlemek için kullanabilirsiniz
    for (int i = 0; i < heartLimit; i++) {
        for (int j = 0; j < 3; j++) {
            Animal *currentAnimal = animals[j];
            if (currentAnimal->status == ALIVE) {
                pthread_create(&animalThreads[j], NULL, simulateanimal, (void *) currentAnimal);
                pthread_join(animalThreads[j], NULL);
                sleep(1);
            }
        }
    }

    for (int i = 0; i <3; i++) {
        if (animals[i]->status == ALIVE) {
            printf("ANIMAL %d başarıyla çalıştı fakat hala hayatta.\n", i);
        } else {
            printf("ANIMAL %d başarıyla çalıştı ve öldü.\n", i);
        }
    }
    
    printf("Animal threadleri bitti.\n");
    
    pthread_t hunter_threads[num_hunter_threads];
    Hunter hunter_objects[num_hunter_threads];

    for (int i = 0; i <num_hunter_threads; i++) {
        hunter_objects[i].location.x = 0;
        hunter_objects[i].location.y = 0;
        hunter_objects[i].points = 0;
    }

    for (int i = 0; i <num_hunter_threads; i++) {
        pthread_create(&hunter_threads[i], NULL, simulatehunter, (void *) &hunter_objects[i]);
        pthread_join(hunter_threads[i], NULL);
        sleep(1);
    }

    for (int i = 0; i < num_hunter_threads; i++) {
        pthread_join(hunter_threads[i], NULL);
        printf("HUNTER %d başarıyla çalıştı\n", i);
    }

    printf("Hunter threadleri bitti.\n");
    printgrid();
    printf("Simulation finished.\n");
    
    deletegrid();
    
    return 0;
}