#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <bits/stdc++.h> 
#include <pwd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;

vector<string> history; //Stores the command line history

//Returns true if string a includes string b
bool includes (string a, string b){
	if(a.length() < b.length())
		return false;
	for(int i=0; i<a.length() - b.length() + 1; i++){
		bool includes = true;
		for(int j=0; j<b.length(); j++){
			if(a[i + j] != b[j]){
				includes = false;
			}
		}
		if(includes) return true;
	}
	return false;
}

//Takes the stdout sentence and prints it properly
void printGrep (string sentence, string search){
	search = search.substr(1, search.length() - 2);
	vector<string>ls;
	string str = "";
	for(int i=0; i<sentence.length(); i++){
		if(sentence[i] == '\n'){
			ls.push_back(str);
			str = "";
		} else {
			str.push_back(sentence[i]);
		}
	}
	for(int i=0; i<ls.size(); i++){
		if(includes(ls[i], search))
			cout<<ls[i]<<endl;
	}
}

//Splits the sentene into words
vector<string> split(string sentence) 
{ 
	vector<string>temp;
	string str = "";
	for(int i=0; i<sentence.length(); i++){
		if(sentence[i] == ' '){
			temp.push_back(str);
			str = "";
		} else {
			str.push_back(sentence[i]);
		}
	}
	temp.push_back(str);
	return temp;
}

//Gets the username
string getUserName() {
	uid_t uid = geteuid ();
    struct passwd *pw = getpwuid (uid);
	if (pw) {
		return std::string(pw->pw_name);
	}
	return {};
}

//Forks and executes the command
void forkCommand(string command){
	int outfd;//new file for cat x > y command
	int fd[2] = {0,0};//pipe between parent and child
	vector<string> tokens = split(command);//command words
	string search;//searched element in grep
	if(tokens.size() == 4 && tokens[0].compare("listdir") == 0 && tokens[2].compare("grep") == 0 ||
		tokens.size() == 5 && tokens[0].compare("listdir") == 0 && tokens[3].compare("grep") == 0){
		search = tokens.size() == 5 ? tokens[4] : tokens[3];
		pipe(fd);
		//Generating pipe for listdir | grep command
	}
	pid_t pid = fork();
	if(pid == 0){
		if(command.compare("listdir") == 0){//command: listdir
			execlp("ls", "ls", NULL);
		} else if(command.compare("listdir -a") == 0){//command: listdir -a
			execlp("ls", "ls", "-a", NULL);
		} else if(command.compare("currentpath") == 0){//command: currentpath
			execlp("pwd", "pwd", NULL);
		} else {//commands with multiple parameters
			if(tokens[0].compare("printfile") == 0 && tokens.size() == 2){//command: printfile x.txt
				execlp("cat", "cat", tokens[1].c_str(), NULL);
			} else if(tokens[0].compare("printfile") == 0 && tokens.size() == 4){//command: printfile x.txt > y.txt
				outfd = open(tokens[3].c_str(), O_CREAT|O_WRONLY|O_TRUNC, 0644);
				dup2(outfd, 1);//redirects stdout to file
				execlp("cat", "cat", tokens[1].c_str(), NULL);
			} else if(tokens[0].compare("listdir") == 0 && tokens[2].compare("grep") == 0){//command: listdir x | grep a
				dup2(fd[1], STDOUT_FILENO);//redirects stdout to parent
				close(fd[0]);
				execlp("ls", "ls", NULL);
			} else if(tokens[0].compare("listdir") == 0 && tokens[3].compare("grep") == 0){//command: listdir -a x | grep a
				dup2(fd[1], STDOUT_FILENO);//redirects stdout to parent
				close(fd[0]);
				execlp("ls", "ls", "-a", NULL);
			} else {
				exit(0);
			}
		}
	} else if( pid < 0 ){
		cout << "Error while forking";
	} else {
		if(outfd){
        	close(outfd);//close file if exists
		}
		if(fd[0] != 0){
			char str[4096];
			close(fd[1]);
			read(fd[0], str, 4096);//get stdout input from child
			printGrep(str, search);
		}
		wait(NULL);
		return;
	}
}

//Prints command line history
void printHistory(){
	for(int i=0; i<history.size(); i++){
		cout<<i+1<<" "<<history[i]<<endl;
	}
}

//Adds new history to history list
void pushHistory(string command){
	if(history.size() < 14){
		history.push_back(command);
	} else {
		for(int i = 13; i >= 0; i--){
			history[i + 1] = history[i];
		}
		history[0] = command;
	}
}

int main() 
{ 
	string username = getUserName();
	cout<<username<<" >>> ";
    while (1) { 
		string command;
		getline(cin, command);//gets the input
		pushHistory(command);//adds it to the history
		if(command.compare("footprint") == 0){
			printHistory();//prints the CLI history
		} else if(command.compare("exit") == 0){
			return 0;//execution finishes
		} else {//other comments needs exec calls
			forkCommand(command);
		}
		cout<<username<<" >>> ";//prints the username
    } 
    return 0; 
} 