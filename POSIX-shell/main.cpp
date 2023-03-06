/**
 * @file main.cpp
 * @brief A simple shell program that demonstrates the usage of the C-POSIX library.
 * @date 2023-03-03
 * @version 1.0
 *
 * @author github.com/beckerbauer
 */

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sstream>
#include <vector>
#include <signal.h>
#include <cstring>
#include <algorithm>

using namespace std;

vector<int> pidList; //global list to manage processes
int currentPID; 

/**
 * Handles the SIGSTP signal by printing a message with the current process ID and stopping the current process.
 * @param signum The signal number to handle (should be SIGSTP).
 */
void SIGSTOP_handler(int signum){
    cout << "[SIGSTP: pid " << currentPID << "]"<< endl;
    kill(currentPID, SIGTSTP);
    currentPID = -2;
}

/**
 * Handles the SIGCONT signal by printing a message with the current process ID and continuing
 * @param signum The signal number to handle (should be SIGCONT).
 */
void SIGCONT_handler(int signum){
    cout << "continuation of: " << signum << endl;
    if((pidList.end()) != (find(pidList.begin(), pidList.end(), signum))) {
        kill(signum, SIGCONT);
        waitpid(signum, 0, WNOHANG);
    }
    else {
        kill(signum, SIGCONT);
        currentPID = signum;
        waitpid(signum, 0, WUNTRACED);
    }
}

/**
 * Handles the SIGCHLD signal by checking for any child processes that have terminated and removing them from the PID list.
 * @param signum The signal number to handle (should be SIGCHLD)
 */
void SIGCHILD_handler(int signum){
    pid_t childpid;
    int status;
    for(int l=0; l < pidList.size(); l++){
        childpid = waitpid(pidList[l], &status, WNOHANG);
        if((childpid>0) && (WIFEXITED(status))){
            cout << "Process Nr. " << childpid << " was terminated" << endl;
            pidList.erase(pidList.begin()+l);
        }
    }
}

// assigning signals to the declared functions
signal(SIGTSTP, SIGSTOP_handler); 
signal(SIGCHLD, SIGCHILD_handler);

int main() {
    bool logoutBool = false;
    //main loop
    while (true)
    {
        string userArg;
        cout << "myshell>";
        getline(cin, userArg);

        if (userArg == "logout") {
            if(pidList.empty()){
                while (true) {
                cout << "Do you really want to logout? [y/n]: ";
                cin >> userArg;
                if (userArg == "y") {
                    logoutBool = true;
                    break;
                } else if (userArg == "n") {
                    break;
                    }                
                }
            } else if (!(pidList.empty())) {
                cout << "Logout not possible, there are processes in the background "
                << "with the PIDs: ";
                for(int j=0; j < pidList.size(); j++){
                    cout << pidList[j];
                    if(j < pidList.size()-1){
                        cout << ", ";
                    }
                }
                cout << "\n" << endl;
            }
        }

        if (logoutBool) {
            break;
        }

        // Take in arguments and read instructions

        int options = WUNTRACED;
        // Initializing a c-style string array to save the parts of a prompt
        char** arguments = new char*[10];
 
        // Converting the prompt userArg to a stringstream to read over it
        istringstream userArgStream(userArg);
        string arg;
        int i=0;
        
        //checking for "stop" or "cont" in argument and starting different behaviour for these arguments
        bool stopBool = false; 
        bool contBool = false;
        while(userArgStream >> arg) // splits argument string at space
        {
            if(stopBool == true){ 
                int temp = stoi(arg);
                kill(temp, SIGSTOP);
                stopBool = false;
            }

            if(contBool == true){
                int temp = stoi(arg);
                SIGCONT_handler(temp);
            }

            if(arg == "&") { //if argument "&" is passed, the process will execute in the background
                options = WNOHANG;
                break;
            }
            
            if(arg == "stop"){ stopBool = true; }
            if(arg == "cont"){ contBool = true; }

            arguments[i] = new char[arg.length()+1];
            strcpy(arguments[i], arg.c_str());
            i++;
        }

        // NULL is added to the array of arguments to indicate the end of the argument list
        arguments[i++] = NULL;



        // Program forks itself to create a new process which will run the requested program
        int pid = fork();

        if(options == WUNTRACED){
            currentPID = pid;
        }

        if (pid==0) { // pid == 0 indicates the script is in the child process
            setpgid(0, getpid());
            int err = execvp(arguments[0], arguments);
            if (err == -1) {
                cerr << "Error";
            }
            return 0;
        } else { // script is in parent process
            setpgid(pid, pid);
            if (options == WNOHANG) {
                pidList.push_back(pid);
            } 
            // output of background-processes
            cout << "[Background: ";
            for(auto & elem : pidList) {
                cout << elem <<", ";
            }
            cout << "]\n";
            waitpid(pid, 0, options);
        }
    }
}