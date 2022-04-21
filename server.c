// Server side C/C++ program to demonstrate Socket programming

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pwd.h>
#include <sys/wait.h>

#define PORT 8081

int dropPrivilgeToNobody() {
    struct passwd* pwd;
    pid_t childProcessID, pid;
    childProcessID = fork();
    printf("childProcessId %d\n",childProcessID);
    printf("Parent UID : %d \n",getuid());

    if(childProcessID == 0) {
        // Fork successful case
        printf("\n fork succeessful \n");
        // setting priviledge as nobody user for read write
        pwd = getpwnam("nobody");
        pid = setuid(pwd->pw_uid);
        printf("Child UID : %d \n",getuid());
        if(pid==0){
            // setting priviledge success
            return 1;
        }
        // setting priviledge unsuccessful

        return 0;
    }
}
int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";

    // check if running process is same as child 
    if(strcmp(argv[0], "socket") == 0)
    {
        printf("\n The process running is in child process");
        // taking new socket
        // using atoi convert string into integer
        int newSocket = atoi(argv[1]);
        valread = read(newSocket, buffer, 1024);
        printf("%s\n", buffer);
        send(newSocket, hello, strlen(hello), 0);
        printf("===Hello message sent to the client \n");
        exit(0);
    }

    // Show ASLR
    // printf("execve=0x%p\n", execve);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attaching socket to port 80
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR ,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 80
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                       (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    if (dropPrivilgeToNobody()) {
        // message processing
        valread = read(new_socket, buffer, 1024);
        printf("Read %d bytes: %s\n", valread, buffer);
        send(new_socket, hello, strlen(hello), 0);
        printf("Hello message sent\n");

        // create duplicate of socket
        int dupParent = dup(new_socket);
    
        if(dupParent == -1)
            perror("socket dup function failed");
        
        // convert socket to string and store i
        char stringBuffer[10];
        sprintf(stringBuffer, "%d", dupParent);
        char *args[] = {"socket", stringBuffer, NULL};
        execvp(args[0], args);
    }
    wait(NULL);
    return 0;
}
