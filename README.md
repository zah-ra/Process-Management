# Process-Management

## Description
In this project, we are going to find the users who have the most similar personality to a given user.

## Implementation
At first, we create an initial process which reads traits.csv file. Then, this process creates a child process using fork() and exec() system calls for each line in traits.csv and gives the line informations to this child process. Each child process, creates a new process for each user-<num>.csv file and gives the given informations from initial process to this new process. The new process calculates euclidean distance for each line and gives back the informations related to shortest distance to its parrent using a named pipe. Each parrent process chooses the shortest distance among given informations and then, gives the file name and its related informations to the initial process.
Informations from parrent to its children and from children to their parrents are transmited using unnamed pipe and named pipe respectively.
