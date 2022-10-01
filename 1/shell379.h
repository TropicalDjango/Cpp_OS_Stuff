#ifndef SHELL379_H
#define SHELL379_H
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
#include <string.h>

void exit();
void jobs();
void kill(int);
void resume(int);
void suspend(int);
int my_wait(int);
void execute(std::string);
void redirect();

#endif