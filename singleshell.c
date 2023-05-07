/*  Grubumuz: 4 kişiden oluşuyor. Zeynep Kıcıkoğlu, İremnur Memiş, Ardan özpolat, Mehmet mutlu. Ödevin tüm kısımlarını grupça kafa yorarak beraber yazdık. ekstra özellik olarak lab 5 deki user info bilgisini structları kullanarak changeuser komutunu özelleştirdik.time kütüphanesini kullanma,strncmp strcat gibi fonksiyonların nasıl kullanılcağı konusunda chatgbt den yardım aldık onun dışındaki kısımları derste öğrendiğimiz bilgilerle yaptık.
singleshell için istenilen maddeler yapılmıstır. current yada farklı klasorde olsalar bıle kodlar yıne calıstırılabılmıstır. Multishell de xterm uygulamasını kurma gibi bilgileri yine chatgbtden arastırarak yaptık. multishellde default nshell olarak 2 degerı verilmiş bunun değiştirilmesi istenmediği için ve 4 tane singleshell olusması istendiğinden fork ile olusturdugumuz sıngleshellerde default degerın karesını aldık. Parent process de olusan child process ler beklensin ve wait fonksıyonunu kullanmak istediğimizden o kısmı parent process de yazdık. oluşan singlesheller yazılan komutları calıstırmaktadır ss lerini de aldık. daha sonra ıstenılen dosya olusmaktadır. Kodları yazarken değişkenlere kafa karıstırıcı isimler vermedik.  */                                                           #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h> 

#define INBUF_SIZE 256
#define MY_FILE_SIZE 1024
#define MY_SHARED_FILE_NAME "/log.txt"
 

#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define MAX_USERS 100

struct user {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
};

struct user users[MAX_USERS];
int num_users = 0;

void add_user(const char* username, const char* password) {
    
    struct user new_user;
    strncpy(new_user.username, username, MAX_USERNAME_LEN);
    strncpy(new_user.password, password, MAX_PASSWORD_LEN);
    users[num_users++] = new_user;
    
}

bool login(const char* username, const char* password) {
    for (int i = 0; i < num_users; i++) {
        
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0) {
            return true;
        }
    }
    return false;
}

char *addr = NULL;
int fd = -1;

int initmem() {
    fd = shm_open(MY_SHARED_FILE_NAME, O_RDWR, 0666);
    
    if (fd < 0) {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(fd, MY_FILE_SIZE) == -1) {
        perror("ftruncate");
        exit(1);
    }

    addr = mmap(NULL, MY_FILE_SIZE,
                PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    if (addr == MAP_FAILED){
        perror("mmap");
        close(fd);
        exit(1);
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    
    add_user("irem", "1510");
    add_user("zeynep", "2408");
    add_user("mehmet", "2412");
    add_user("ardan", "2312");
    
    char user[20];
    char password[20];
    
    initmem();
    
    char inbuf[INBUF_SIZE];
    int nbyte;
    
    time_t start_time_t = time(NULL);
    time_t finish_time_t;
    
    struct tm *start_time_tm = localtime(&start_time_t);
    struct tm *finish_time_tm;
    
    char start_time[20];
    char finish_time[20];
    
    strftime(start_time, sizeof(start_time), "%Y-%m-%d %H:%M:%S", start_time_tm);
    
    char start_date_description[30] = "Başlangıç tarihi: ";
    strcat(start_date_description, start_time);

    int fp = open("log.txt",
                 O_RDWR | O_APPEND | O_CREAT | O_TRUNC,
                 S_IRWXU, 0666);

   
    if (fp == -1) {
       perror("cannot open file: ");
       return -1;
    }

    int len_start = strlen(start_date_description);


    char buffer[50];
    int n = sprintf(buffer, "Process ID: %d\nParent Process ID: %d\n", getpid(), getppid());
    
    if (n < 0) {
        perror("sprintf");
        exit(1);
    }
    if (write(fp, buffer, n) == -1) {
        perror("write");
        exit(1);
    }
    if (write(fp, start_date_description, len_start) == -1) {
        perror("write");
        exit(1);
    }

    char userinfo[MAX_USERNAME_LEN] = "";

    int counter = 0;

    while(1)
    {
        
        if(user == NULL) {
            write(1,"$",2);
        }
        else {
            if(counter==0) {
                strcat(userinfo,"$");
                counter++;
            }
            write(1,userinfo,8);
        }

          
        if((nbyte=read(0,inbuf,255))<=0) {
            perror("input<=0");
            
        } else {
            inbuf[nbyte-1] = '\0';
        }


        printf("inbuf:%.250s",inbuf);
        printf("\n");

        if(strncmp(inbuf, "changeuser", 11) == 0) {

            strcpy(userinfo, "");
            printf("ad:");
            scanf("%s", user);
            printf("şifre:");
            scanf("%s", password);
    
            if(login(user,password)) {
                printf("Login successful\n");
                strcat(userinfo, user);
                strcat(userinfo,"$");
            } 
            else {
                printf("Login failed\n");
            }
        }
        
        if (strncmp(inbuf, "exit", 4) == 0){
            time(&finish_time_t);
            finish_time_tm = localtime(&finish_time_t);
            strftime(finish_time, sizeof(finish_time), "%Y-%m-%d %H:%M:%S" , finish_time_tm);
            printf("Bitis tarihi: %s\n", finish_time);
            
            char finish_date_description[30] = "\nBitiş tarihi: ";
            
            strcat(finish_date_description, finish_time);
            
            int len_finish = strlen(finish_date_description);
            
            write(fp, "\nInput: ", 7);
            write(fp, "exit", 4);
            write(fp, finish_date_description, len_finish);
            
            break;
        }
        
        // Log input to file
        
        write(fp, "\nInput: ", 7);
        write(fp, inbuf, strlen(inbuf));
        strcat(&addr, inbuf);
        strcat(&addr, "\n");
        printf("%.50s", &addr);
        
        pid_t child_pid = fork();
        
        if (child_pid == 0) {
      
      int r = execl(inbuf, inbuf, NULL);

      if (r == -1) {
        char command[255] = {'/', 'b', 'i', 'n', '/', '\0'};
        strncat(command, inbuf, 250);
        r = execl(command, inbuf, NULL);
        if (r == -1)
            perror("execl");
        exit(1);
      }
    }  else if (child_pid > 0){
            wait(NULL);
        } else {
            perror("fork() hata");
        }
    }
    munmap(addr, 1024);
    close(fp);
    
    close(fd);
    return 0;
}
