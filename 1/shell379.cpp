/*
=============================
Name: Ahmed Refik
Id: 1573813
CMPUT 379, Fall 2022

Assignment 1: Mini Shell
=============================
*/

#include "shell379.h"

#define LINE_LENGTH 	100
#define MAX_ARG 		7
#define MAX_LENGTH		20
#define MAX_PT_ENTRIES  32

using namespace std;

/*
	This is the Process class that the
	Process control block maintains a map
	of.

	Members:
		int pid: the process id of the child
		char state: 'S' for suspended 'R' for running
		string cmd: the inital command for this child 
*/

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

/*
	This is the Process Control Block that keeps track 
	of all the running processes and updates when 'jobs' 
	is called, a process is killed, or a process is added

	Members:
		std::map<int,Process> run_proc: the map of all know processes
		int size: the size of run_proc

	Methods:
		void addProcess(Process): This adds a process to the map and
								  increments the size.
		void removeProcess(int): This finds a Process given a pid and
								 removes it form the map
*/

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
		auto iter = run_proc.find(pid);
		if(iter != run_proc.end()) {
			run_proc.erase(pid);
			size--;
		}
	}
};

/*
	print_total_time() prints the total cputime of all completed processes 
	and the parent process. It uses getrusage() on the parent process
	and the children, adds them up for the total cpu time for both User
	and System. it takes an integer as input to decide which header to print
	since jobs and exit have different headers for the same information
*/

void print_total_time(int status) {
	struct rusage parent_time;
	if(getrusage(RUSAGE_CHILDREN, &parent_time) < 0)
		perror("print_total_time() rusage error");
	
	int user_time_child = parent_time.ru_utime.tv_sec;
	int sys_time_child = parent_time.ru_stime.tv_sec;
	
	if(getrusage(RUSAGE_SELF, &parent_time) < 0)
		perror("print_total_time() rusage error");
	
	int user_time_total = parent_time.ru_utime.tv_sec + user_time_child;
	int sys_time_total = parent_time.ru_stime.tv_sec + sys_time_child;
	
	if(status == 1) {
		cout << "Completed processes:\n";
	} else {
		cout << "Resources used:\n";
	}
	cout << "User time =      " + to_string(user_time_total) + " seconds\n";
	cout << "Sys  time =      " + to_string(sys_time_total) + " seconds\n";
	return;
}

PCB process_cb;

/*
	exit() waits for all children to run to completion,
	prints total time, and finally terminates itself
*/
void exit() 
{
	// wait for all children to exit
	while(wait(NULL) != -1);
	print_total_time(0);
	_exit(0);
}

/*
	jobs() creates a pipe, forks a child whose output is 
	redirected to the pipe and a parent whose input is redirected
	to the pipe. 

	The child runs the command "ps -o pid,cputime" using exec()
	and redirects its output to the parent.

	The parent then parses the output of ps, searches for the pid
	in the process control block, if it's in there then add the
	process to a new map<int,Proccess> and print out all relevant info 
	for that process and continue to the next entry in the table. 

	When there is no more input to read then, the PCB map is replaced 
	with the new map and the total time is printed. 
*/
void jobs()
{
	Process selProcess;
	char buffer[LINE_LENGTH*MAX_PT_ENTRIES];
	string buf_str[sizeof(buffer)];
	char* argv[] = {(char*)"ps",(char*)"-o",(char*)"pid,cputime",NULL};
	
  int id = 0, fd[2], status, run_total = 0, ii = 0;
	
  bool first_run = true;	// need this to print top header for active processes
	
	
	cout << "Running processes:\n";
	if(process_cb.size > 0) {	
		if(pipe(fd) < 0)
			perror("jobs() pipe failed");
		id = fork();
		if(id < 0)
			perror("jobs() fork failed");
		
		//child
		else if(id == 0) { 			
			// redirect pipe to STDIN and STDOUT
			dup2(fd[0], 0);
			dup2(fd[1], 1);
			close(fd[0]);
			execvp(argv[0],argv);

		// Parent
		} else { 					
			my_wait(id);
			close(fd[1]);
			
			// Check if pipe is empty
			if(read(fd[0], buffer, sizeof(buffer)) == '\0')
				perror("jobs() read");

			map<int,Process>* temp = &process_cb.run_proc;
			map<int,Process> replace_runproc;
			string buf_str(buffer);
			stringstream ss(buf_str);
			string time;
			
			// remove headers
			ss >> time >> time;
			while(ss >> time)
			{
				// PID and CPUTIME are the coloums from child
				// So search for PID in PCB
				auto pindex = temp->find(stoi(time));
				if( pindex != temp->end()){
					
					replace_runproc.insert(pair<int,Process>(pindex->first,pindex->second));
					
					if(first_run) {
						cout << "#    PID S SEC COMMAND\n";
						first_run = false;
					}
					selProcess = pindex->second;
					
					cout << to_string(ii) << ": " << time << " ";
					cout << selProcess.state << " ";
					
					ss >> time;
					
					// return seconds from child and remove leading zero
					cout << to_string(stoi(time.substr(time.length()-1,2))); 
					cout << "   " << selProcess.cmd << " \n";
					ii++;
				}
			}
			process_cb.run_proc.clear();
			process_cb.run_proc = replace_runproc;
			process_cb.size = replace_runproc.size();
			
			close(fd[0]);
		}
	}
	cout << "Processes =      " + to_string(process_cb.size) + " active\n";
	print_total_time(1);
	return;
}

/*
	my_kill(int) takes a pid and sends a SIGKILL to the process
	with that ID. 

	if successful then PCB is updated
*/

void my_kill(int pid)
{
	if(kill(pid,SIGKILL) != 0) {
		perror("Kill");
	}
	else {
		process_cb.removeProcess(pid);
		return;
	}
}

/*
	resume(int) takes a pid and sends a SIGCONT to the process
	with that ID. 

	if successful then process.state in PCB is updated to 'R'
*/

void resume(int pid)
{
	if(kill(pid, SIGCONT) != 0) {
		perror("resume()");
	} else {
		auto iter = process_cb.run_proc.find(pid);
		iter->second.state = 'R';
		return;
	}
}

/*
	resume(int) takes a pid and sends a SIGSTOP to the process
	with that ID. 

	if successful then process.state in PCB is updated to 'S'
*/

void suspend(int pid)
{
	if(kill(pid, SIGSTOP) != 0) {
		perror("suspend()");
	} else{
		auto iter = process_cb.run_proc.find(pid);
		iter->second.state = 'S';
		return;
	}

}

/*
	my_wait(int) takes a pid and stops the calling process until
	the processs with that id has exited or is currently suspended
	(I assumed that it shouldn't wait on a suspended process)

	if successful then the calling process continues running
*/

int my_wait(int pid)
{
	int status;
	pid = waitpid(pid,&status, WUNTRACED);
	if (pid == -1) {
		perror("wait()");
		return -1;
	} if(pid != -1) {
		// If child exited normally
		if(WIFEXITED(status)) {
			process_cb.removeProcess(pid);
			return 0;
		// If child exits normally
		} else if (WEXITSTATUS(status)){
			process_cb.removeProcess(pid);
			return 0;
		// If child has been suspended
		} else if (WSTOPSIG(status)){
      return 1;
		}
	}
}

/*
	execute(string) takes the command, parses it and adds each word to args
	, and forks a child to run it.

	fin_bool and fout_bool exist so the parent closes those files
*/

void execute(string arg)
{
	int i = 0, fd_in, fd_out;
	stringstream stream(arg);
	string temp;
	char* args[MAX_ARG+1] = {NULL};
	bool fin_bool = false,fout_bool = false, parent_wait = true;
	
	for(; stream >> temp; i++) 
	{
		if(i == MAX_ARG) {perror("execute() too many arguments");}

		if(temp.c_str()[0] == '&') {
			parent_wait = false;
			i--;
		} else if(temp.c_str()[0] == '<') {

			fd_in = open(temp.erase(0,1).c_str(), O_RDONLY);
			if(fd_in == '\0')
				perror("execute() file input");
			fin_bool = true;
			i--;

		} else if(temp.c_str()[0] == '>') {

			fd_out = open(temp.erase(0,1).c_str(), O_WRONLY);
			if(fd_out == '\0')
				perror("execute() file output");
			fout_bool = true;
			i--;

		} else {
			args[i] = strdup(temp.c_str());
		}
	}
	// null terminate the char array
	args[i+1] = NULL;
	int id = fork();
	if(id < 0) 
		perror("execute() fork");
	// Parent
	else if(id > 0) {
		// parent response to special commands 
		if(fin_bool)
			close(fd_in);
		
		if(fout_bool)
			close(fd_out);
		
		if(parent_wait) 
			my_wait(id);

		Process child(id, arg);
		process_cb.addProcess(child);
		return;
	}
	// Child
	else {
		if(fin_bool) {
			dup2(fd_in,0);
			close(fd_in);
		}

		if(fout_bool) {
			dup2(fd_out,1);
			dup2(fd_out,2);
			close(fd_out);
		}
		if(execvp(args[0],args) < 0) 
			perror("execute() execvp");
	}
}
/*
	redirect() takes user input as redirects their request to
	a more specilized function
*/
void redirect() 
{
	string user;
	string word;
	string cmd;
	string arg;
	int pid;

  std::cout << std::endl << "-> ";
	getline(cin,user);

	stringstream in(user);

	in >> word;
	if(word.compare("exit") == 0) 
		exit(); 

	else if(word.compare("jobs") == 0)
		jobs(); 

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
	return;
}

int main() 
{
	while(1) 
		redirect();

}
