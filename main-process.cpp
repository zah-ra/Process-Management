#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/wait.h>
#include <vector>
#include <string>

using namespace std;

void read_traits(vector<int*> &traits, string file_path) {
    int cnt = 0;
    int row[5] = {0};
    ifstream traitscsv(file_path);
    string line, num;
    while(getline(traitscsv, line)){
        traits.push_back(new int[5]);
        for(int i = 0; i < 5; i++) {
            num = line[i*2];
            traits[cnt][i] = stoi(num);
        }
        cnt++;
    }
    traitscsv.close();
}

void fork_proc(int fd[2], char *path, char *directory) {
    if (!fork()) {
        char fd0[16], fd1[16];
        sprintf(fd0, "%d", fd[0]);
        sprintf(fd1, "%d", fd[1]);
        char *args[]={fd0, fd1, path, directory, NULL};
        execvp("./trait", args);
    }
}

void write_on_pipe(int fd[2], int trait[]) {
    close(fd[0]);
    if(write(fd[1], trait, 5 * sizeof(int)) == -1) {
        cout << "Error in writing on pipe in main process\n";
        return;
    }
    close(fd[1]);
}

void print_result(vector<int> &fifoes, int num_of_proc) {
    ofstream myfile;
    myfile.open ("result.csv");
    for (int i = 0; i < num_of_proc; i++) {
        char res[128];
        if (read(fifoes[i], res, sizeof(res)) == -1) {
            cout << "Error in reading\n";
            return;
        }
        myfile << res;
        if(i != num_of_proc - 1)
            myfile << "\n";
        // cout << res << endl;
        close(fifoes[i]);
    }
    myfile.close();
}

int main(int argc, char *argv[]) {
    vector<int*> traits;
    vector<int> my_fifoes;
    int num_of_proc, fifo_fd;
    read_traits(traits, argv[1]);
    num_of_proc = traits.size();
    for(int i = 0; i < num_of_proc; i++) {
        char path[128] = "/tmp/my_fifo";
        string temp = to_string(i);
        const char *num_char = temp.c_str();
        strncat(path, num_char, strlen(num_char));
        int fd[2];
        if(pipe(fd) == -1) {
            cout << "Error in main process pipe\n";
            exit(1);
        }
        fork_proc(fd, path, argv[2]);
        mkfifo(path, 0666);
        fifo_fd = open(path, O_RDONLY);
        my_fifoes.push_back(fifo_fd);
        write_on_pipe(fd, traits[i]);
    }
    for(int i = 0; i < num_of_proc; i++)
        wait(NULL);
    print_result(my_fifoes, num_of_proc);
    exit(0);
}