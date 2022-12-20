#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

void read_users_traits(vector<int*> &users, string file_path) {
    int cnt = 0;
    int row[5] = {0};
    ifstream userscsv(file_path);
    string line, num;
    while(getline(userscsv, line)){
        users.push_back(new int[5]);
        for(int i = 0; i < 5; i++) {
            num = line[i*2];
            users[cnt][i] = stoi(num);
        }
        cnt++;
    }
    userscsv.close();
}

void read_from_pipe(char *fd0, char *fd1, int trait[]) {
    int fd[2];
    fd[0] = atoi(fd0);
    fd[1] = atoi(fd1);
    close(fd[1]);
    if(read(fd[0], trait, 5*sizeof(int)) == -1) {
        cout << "Error in reading pipe in user from trait\n";
        return;
    }
    close(fd[0]);
}

int calc_min(int trait[], vector<int*> users) {
    int dist = 0;
    int min = 1100000000;
    int row = -1;
    for(int i = 0; i < users.size(); i++) {
        for(int j = 0; j < 5; j++) {
            dist += (trait[j] - users[i][j]) * (trait[j] - users[i][j]);
        }
        if(dist < min) {
            min = dist;
            row = i;
        }
        dist = 0;
    }
    return row;
}

string set_result(int row, vector<int*> users, char user_directory[128]){
    string result;
    string ud(user_directory);
    int pos = ud.find('.');
    result = ud.substr(11, pos - 11) + ",";
    for(int i = 0; i < 5; i++) {
        result += to_string(users[row][i]);
        if(i != 4)
            result += ",";
    }
    return result;
}

void write_on_fifo(string result, int my_fifo) {
    char res[128];
    strcpy(res, result.c_str());
    if(write(my_fifo, res, strlen(res) + 1) == -1) {
        cout << "Error in writing in user\n";
        return;
    }
    close(my_fifo);
}

int main(int argc, char *argv[]) {
    vector<int*> users;
    vector<int> my_fifoes;
    int main_trait[5] = {0};
    int row, fifo_fd;
    char path[128], user_directory[128];
    
    strcpy(path, argv[2]);
    strcpy(user_directory, argv[3]);
    read_users_traits(users, user_directory);
    
    fifo_fd = open(path, O_WRONLY);
    read_from_pipe(argv[0], argv[1], main_trait);
    row = calc_min(main_trait, users);
    string result = set_result(row, users, user_directory);
    write_on_fifo(result, fifo_fd);
    exit(0);
}