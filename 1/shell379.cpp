/*
=============================
Name: Ahmed Refik
Id: 1573813
CMPUT 379, Fall 2022

Assignment 1: 
=============================
*/
#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <time.h>
#include <bits/stdc++.h>
#include <fcntl.h>
#include <fstream>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <bits/stdc++.h>
#include <vector>
#include "shell379.h"


#define _XOPEN_SOURCE_EXTENDED 1
#define LINE_LENGTH 	100
#define MAX_ARG 		7
#define MAX_LENGTH		20
#define MAX_PT_ENTRIES  32

using namespace std;



class Process
{
	public:
		int pid;
		char state;
		string cmd;

	Process(){}
	
	Process(int pid, string cmd) 
	{
		this->pid = pid;
		state = 'R';
		this->cmd = cmd;
	}
};

class PCB
{
	public:
		std::map<int, Process> run_proc;
		int size;

	void addProcess(Process p) {
		run_proc.insert({p.pid, p});
		size++;
	}

	void removeProcess(int pid) {
		run_proc.erase(pid);
		size--;
	}
};

void print_total_time(int status) {
	struct rusage parent_time;
	if(getrusage(RUSAGE_CHILDREN, &parent_time) < 0) {
		perror("exit() rusage error");
	}
	
	int user_time_child = parent_time.ru_utime.tv_sec;
	int sys_time_child = parent_time.ru_stime.tv_sec;
	
	if(getrusage(RUSAGE_SELF, &parent_time) < 0) {
		perror("exit() rusage error");
	}
	
	int user_time_total = parent_time.ru_utime.tv_sec + user_time_child;
	int sys_time_total = parent_time.ru_stime.tv_sec + sys_time_child;
	
	if(status == 1){cout << "Completed processes:\n";}
	else{cout << "Resources used:\n";}
	cout << "User time =      " + to_string(user_time_total) + " seconds\n";
	cout << "Sys  time =      " + to_string(sys_time_total) + " seconds\n";
}

PCB process_cb;

void exit() 
{
	// wait for all children to exit
	while(wait(NULL) != -1);
	print_total_time(0);
	_exit(0);
}

void jobs()
{
	Process selProcess;
	char buffer[LINE_LENGTH*MAX_PT_ENTRIES];
	string buf_str[sizeof(buffer)];
	char* argv[] = {"ps","-o","pid,cputime",NULL};
	int id = 0;
	int fd[2];
	int status;
	int run_total = 0;
	bool first_run = true;
	int i = 0;
	
	cout << "Running processes:\n";
	if(process_cb.size > 0) {	
		if(pipe(fd) < 0)
			perror("jobs pipe fail");
		id = fork();
		if(id < 0)
			perror("jobs fork fail");
		else if(id == 0) { //child
			dup2(fd[0], 0);
			//fd[1] = open("sleepy.txt",O_WRONLY);
			dup2(fd[1], 1);
			close(fd[0]);
			//close(fd[1]);
			execvp(argv[0],argv);
		} else { //parent
			wait(NULL);
			// ignore the first line
			close(fd[1]);
			
			if(read(fd[0], buffer, sizeof(buffer)) == NULL)
				perror("invaild read for jobs");

			map<int,Process>* temp = &process_cb.run_proc;
			map<int,Process> replace_runproc;
			string buf_str(buffer);
			stringstream ss(buf_str);
			int i = 0;
			string time;
			
			// remove headers
			ss >> time >> time;
			while(ss >> time)
			{
				auto pindex = temp->find(stoi(time));
				if( pindex != temp->end()){
					replace_runproc.insert(pair<int,Process>(pindex->first,pindex->second));
					if(first_run) {
						cout << "#    PID S SEC COMMAND\n";
						first_run = false;
					}
					selProcess = pindex->second;
					cout << to_string(i) << ": " << time << " ";
					cout << selProcess.state << " ";
					ss >> time;
					cout << to_string(stoi(time.substr(time.length()-2,2))) << " " << selProcess.cmd << " \n";
					run_total++;
					i++;
				}
			}
			process_cb.run_proc.clear();
			process_cb.run_proc = replace_runproc;
			close(fd[0]);
		}
	}
	cout << "Processes =      " + to_string(run_total) + " active\n";
	cout << buffer;
	print_total_time(1);
	return;
}

void my_kill(int pid)
{
	if(kill(pid,SIGKILL) != 0) {
		perror("Kill failed");
	}
	else {
		process_cb.removeProcess(pid);
	}
}

void resume(int pid)
{
	if(kill(pid, SIGCONT) != 0) {
		perror("resume fail");
	} else {
		auto iter = process_cb.run_proc.find(pid);
		iter->second.state = 'R';
	}
}

void suspend(int pid)
{
	if(kill(pid, SIGSTOP) != 0) {
		perror("suspend fail");
	} else{
		auto iter = process_cb.run_proc.find(pid);
		iter->second.state = 'S';
	}

}

int my_wait(int pid)
{
	int status;
	{
		if (waitpid(pid,&status, WUNTRACED) == -1 && errno == EINTR) {
			perror("wait() fail");
			return -1;
		} if(pid != -1) {
			// If child exited normally
			if(WIFEXITED(status)) {
				return 0;
			// If child exits normally
			} else if (WEXITSTATUS(status)){
				return 0;
			// If child has been suspended
			} else if (WSTOPSIG(status)){
				return 1;
			}
		} 
	} while(pid == 0);
}


void execute(string arg)
{
	int i = 0, fd_in, fd_out;
	stringstream stream(arg);
	string temp;
	char* args[MAX_ARG+1] = {NULL};
	bool fin_bool = false,fout_bool = false, parent_wait = true;
	
	for(; stream >> temp; i++) 
	{
		if(i == MAX_ARG) {perror("TOO MANY ARGUMENTS");}

		if(temp.c_str()[0] == '&') {
			parent_wait = false;
			i--;
		} else if(temp.c_str()[0] == '<') {

			fd_in = open(temp.erase(0,1).c_str(), O_RDONLY);
			fin_bool = true;
			i--;

		} else if(temp.c_str()[0] == '>') {

			fd_out = open(temp.erase(0,1).c_str(), O_WRONLY);
			fout_bool = true;
			i--;

		} else {
			args[i] = strdup(temp.c_str());
		}
	}
	args[i+1] = NULL;
	int id = fork();
	if(id < 0) {perror("execute fork fail");}
	else if(id > 0) {	//parent
		// parent response to special commands 
		if(fin_bool){close(fd_in);}
		if(fout_bool){close(fd_out);}
		if(parent_wait) {my_wait(id);}

		Process child(id, arg);
		process_cb.addProcess(child);
	}
	else {//child
		if(fin_bool) {
			dup2(fd_in,0);
			close(fd_in);
		}

		if(fout_bool) {
			dup2(fd_out,1);
			dup2(fd_out,2);
			close(fd_out);
		}
		//signal(SIGUSR1, child_handler);
		//signal(SIGUSR2, child_handler);
		if(execvp(args[0],args) < 0) {perror("execve");}
	}
}

void redirect() 
{
	string user;
	string word;
	string cmd;
	string arg;
	int pid;

	getline(cin,user);

	stringstream in(user);

	in >> word;
	if(word.compare("exit") == 0) {exit();} 

	else if(word.compare("jobs") == 0) {jobs();} 

	else if(word.compare("kill") == 0) {
		in >> pid;
		my_kill(pid);

	} else if(word.compare("resume") == 0) {
		in >> pid;
		resume(pid);

	} else if(word.compare("sleep") == 0) {
		in >> pid;
		sleep((unsigned int)pid);

	} else if(word.compare("suspend") == 0) {
		in >> pid;
		suspend(pid);

	} else if(word.compare("wait") == 0) {
		in >> pid;
		my_wait(pid);
	} else {
		arg = word;
		while(in >> word)
		{
			arg.append(" " + word);
		}

		execute(arg);
	}
}

int main() 
{
	while(1) 
	{
		redirect();
	}

}