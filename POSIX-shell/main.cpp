#include <iostream>x
#include <unistd.h>
#include <sys/wait.h>
#include <sstream>
#include <vector>
#include <signal.h>

using namespace std;

vector<int> pidList;
int currentPID;

void SIGSTOP_handler(int signum){
    cout << "[SIGSTP: pid " << currentPID << "]"<< endl;
    kill(currentPID, SIGTSTP);
    currentPID = -2;
}

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

void SIGCHILD_handler(int signum){
    pid_t childpid;
    int status;

    for(int l=0; l < pidList.size(); l++){
        childpid = waitpid(pidList[l], &status, WNOHANG);
        if((childpid>0) && (WIFEXITED(status))){
            cout << "Prozess Nr. " << childpid << " wurde beendet" << endl;
            pidList.erase(pidList.begin()+l);
        }
    }
}

int main() {
    bool weiter = true;
    while (true)
    {
        string eingabe = "";
        cout << "myshell> ";
        getline(cin, eingabe);

        if (eingabe == "logout") {
            if(pidList.empty()){
                while (true) {
                cout << "Wirklich beenden? [y/n]: ";
                cin >> eingabe;
                if (eingabe == "y") {
                    weiter = false;
                    break;
                } else if (eingabe == "n") {
                    break;
                    }                
                }
            } else if (!(pidList.empty())) {
                cout << "Logout nicht möglich, es laufen noch Prozesse im Hintergrund "
                << "mit den PIDs: ";
                for(int j=0; j < pidList.size(); j++){
                    cout << pidList[j];
                    if(j < pidList.size()-1){
                        cout << ", ";
                    }
                } 
                cout << "\n" << endl;
            }
        }

        if (weiter == false) {
            break;
        }

        // Eingabe verarbeiten und Argumente auslesen
        int options = WUNTRACED;
        char** argumente = new char*[10];
 
        istringstream eingabestream(eingabe);
        string arg;
        int i=0;
        
        bool stopBool = false;
        bool contBool = false;
        while(eingabestream >> arg) // trennt nach Leerzeichen
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

            if(arg == "&") {
                options = WNOHANG;
                break;
            }
            
            if(arg == "stop"){ stopBool = true; }
            if(arg == "cont"){ contBool = true; }

            argumente[i] = new char[arg.length()+1];
            strcpy(argumente[i], arg.c_str());
            i++;
        }

        argumente[i++] = NULL;


        signal(SIGTSTP, SIGSTOP_handler);
        signal(SIGCHLD, SIGCHILD_handler);


        // Fork
        int pid = fork();

        if(options == WUNTRACED){
            currentPID = pid;
        }

        if (pid==0) {
            // child process
            setpgid(0, getpid());
            int err = execvp(argumente[0], argumente);
            if (err == -1) {
                //cout << "Fehler\n";
            }
            return 0;
        } else {
            // parent process
            setpgid(pid, pid);
            if (options == WNOHANG) {
                pidList.push_back(pid);
            } 
            // Ausgabe Liste Hintergrundprozesse
            cout << "[Hintergrund: ";
            for(auto & elem : pidList)
            {
                cout << elem <<", ";
            }
            cout << "]\n";

            waitpid(pid, 0, options);
        }

    }
    
}
