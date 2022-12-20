#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

void read_from_pipe(char *fd0, char *fd1, int trait[]) {
    int fd[2];
    fd[0] = atoi(fd0);
    fd[1] = atoi(fd1);
    close(fd[1]);
    read(fd[0], trait, 5*sizeof(int));
    close(fd[0]);
}

void fork_proc(int fd[2], char *fifo_path, char* directory, char* file_name) {
    string _directory(directory);
    string _file_name(file_name);
    string path= _directory + "/" + _file_name;
    char _path[128];
    strcpy(_path, path.c_str());
    if (!fork()) {
        char fd0[16], fd1[16];
        sprintf(fd0, "%d", fd[0]);
        sprintf(fd1, "%d", fd[1]);
        char *args[]={fd0, fd1, fifo_path, _path, NULL};
        execvp("./user", args);
    }
}

void write_on_pipe(int fd[2], int trait[]) {
    close(fd[0]);
    write(fd[1], trait, 5 * sizeof(int));
    close(fd[1]);
}

int calc_dist(int trait[], string user_row) {
    int dist = 0, num;
    int index = user_row.find(',');
    user_row = user_row.substr(index + 1);
    for(int j = 0; j < 5; j++) {
        char num_char[2] = {user_row[j*2]};
        num = atoi(num_char);
        dist += (trait[j] - num) * (trait[j] - num);
    }
    return dist;
}

string calc_min_user(vector<int> &my_fifoes, int trait[], int num_of_proc) {
    int dist = 0, min = 1100000000;
    int number[5] = {0};
    string min_user;
    for (int i = 0; i < num_of_proc; i++) {
        char user_row[128];
        read(my_fifoes[i], user_row, 128);
        string _user_row(user_row);
        
        dist = calc_dist(trait, _user_row);
        if(dist < min) {
            min = dist;
            min_user = _user_row;
        }
        close(my_fifoes[i]);
    }
    return min_user;
}

void write_on_fifo(string result, int my_fifo) {
    char res[128];
    strcpy(res, result.c_str());
    if(write(my_fifo, res, strlen(res) + 1) == -1) {
        cout << "Error in writing in trait\n";
        return;
    }
    close(my_fifo);
}

int main(int argc, char *argv[]) {
    char path[128], user_directory[128];
    int main_trait[5] = {0};
    int fifo_fd, num_of_proc = 0;
    vector<int> my_fifoes;
    strcpy(path, argv[2]);
    strcpy(user_directory, argv[3]);
    int my_fifo = open(path, O_WRONLY);
    read_from_pipe(argv[0], argv[1], main_trait);
    
    struct dirent *de; 
    DIR *dr = opendir(user_directory);
    if (dr == NULL) {
        cout << "Could not open users directory\n";
        return 0;
    }
    while ((de = readdir(dr)) != NULL) {
        if ((de->d_name)[0] != '.') {
            string temp = to_string(num_of_proc);
            const char *num_char = temp.c_str();
            strncat(path, num_char, strlen(num_char));
            num_of_proc++;
            int fd[2];
            if(pipe(fd) == -1) {
                cout << "Error in trait pipe\n";
                exit(1);
            }
            fork_proc(fd, path, user_directory, de->d_name);
            mkfifo(path, 0666);
            fifo_fd = open(path, O_RDONLY);
            my_fifoes.push_back(fifo_fd);
            write_on_pipe(fd, main_trait);
        }
    }
    closedir(dr); 
    string min = calc_min_user(my_fifoes, main_trait, num_of_proc);
    write_on_fifo(min, my_fifo);
    exit(0);
}