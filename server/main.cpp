/*
Phillip Harmston
CSci 156 Project (server end)

    The program is the server end of bidding system. As soon as a client connects to this program,
    the program will immediately start the bidding process. Each client will select a product "randomly"
    and bid until it either wins one or the quantity of the product runs out. Sends the winner a message about
    winning the product.
    compiled and ran in code block
*/
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <cstdlib>
#include <pthread.h>
#include <cstring>
#include <string>
#include <time.h>
#include <vector>
#include <sstream>
#include "include/Product.h"

#define NTHREADS 5
#define MYPORT 3490    // the port users will be connecting to
#define BACKLOG 10     // how many pending connections queue will hold
#define NUMOFPRODS 5

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
int psize;
std::vector<Product> prods;
int quit = 0;
int count = 1;

struct info {
    //string id;
    int new_fd;
    //int addr;
    std::string id;
    int datasize;
    int exit;
};


void *thread_worker(void *data)
{
    struct info *info =(struct info*) data;
    int temp;
    if( info->datasize != 0)
        temp = rand() % info->datasize;

    while(1){

        if( info->exit){ //if number of sold out products
            printf("soldout\n");
            break;
        }

        while( 1){

            pthread_mutex_lock( &mutex1 );
            info->datasize = psize;
            info->exit = quit;

            if( prods.empty()){
                break;
            }

            if( prods[temp].Getquantity() == 0){    //checks if quantity is zero
                psize--;
                prods.erase(prods.begin()+temp);
                if(prods.empty()){
                    quit = 1;
                }

                pthread_mutex_unlock( &mutex1 );
                break;
            }
            //printf("here\n");
            prods[temp].SetcurrBid( prods[temp].GetcurrBid()+1);
            printf("number of bids: %d\n", count++);

            printf("Thread %ld bidding on %s.\t Current bid is %d\n", (long) pthread_self(), prods[temp].GetprodID().c_str(), prods[temp].GetcurrBid());

            //thread wins the bid
            if( prods[temp].GetcurrBid() == prods[temp].GetmaxBid()){

                //decrementing quantities
                prods[temp].Setquantity( prods[temp].Getquantity()-1);
                prods[temp].SetcurrBid( 0); //reset currbid

                std::stringstream ss;
                ss<<prods[temp].Getquantity();
                //message to winner
                std::string temp2 = "Winning bid console \tnumber of " + prods[temp].GetprodID() + " left: "+ ss.str()+"\n";
                //send to winning console
                if (send(info->new_fd, temp2.c_str(), temp2.size(), 0) == -1){
                    perror("send");
                    exit(1);
                }

                printf("winning thread id: %ld bought %s\n", (long) pthread_self(), prods[temp].GetprodID().c_str());
                pthread_mutex_unlock( &mutex1 );
                break;
            }


            //loses bid
            if( prods[temp].GetmaxBid() < prods[temp].GetcurrBid()){
                //send to losers
                pthread_mutex_unlock( &mutex1 );
                continue;
            }

            pthread_mutex_unlock( &mutex1 );
            sleep(rand() % 3);
        }
        if( info->datasize != 0)
            temp = rand() % info->datasize;
    }
    if (send(info->new_fd, "quit", 4, 0) == -1){
        perror("send");
        exit(1);
    }
    close(info->new_fd);
    printf("thread ending: %ld\n", (long) pthread_self());
    pthread_exit(0);
}

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char **argv)
{
    pthread_t thread_id[NTHREADS];
    struct info *myinfo;
    int sockfd, new_fd;
    int i = 0;  // listen on sock_fd, new connection on new_fd
    struct sockaddr_in my_addr;    // my address information
    struct sockaddr_in their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    pthread_attr_t attr;
    std::stringstream ss;

    srand((unsigned)time(NULL));    // seeding rand

    for( int j = 1; j <= NUMOFPRODS; j++){
        Product *temp;
        ss<<j;
        temp = new Product (rand() % 10+1, "Product " + ss.str(), rand() % 3+1);
        prods.push_back(*temp);
        ss.str("");
    }
    psize = prods.size();

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(MYPORT);     // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    pthread_attr_init(&attr); // Creating thread attributes
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO); // FIFO scheduling for threads
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);


    while(1) {  // main accept() loop
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        myinfo = new struct info;
        myinfo->datasize = prods.size();
        myinfo->new_fd =new_fd;

        printf("server: got connection from %s\n", inet_ntoa(their_addr.sin_addr));

        if (pthread_create(&thread_id[i], NULL, thread_worker, myinfo)){
            printf("Error:unable to create thread");
            exit(0);
        }

        i++;
    }

    printf("done");

    return 0;
}
