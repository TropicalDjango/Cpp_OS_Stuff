# CMPUT 379:
## Assignment 2: Producer-Consumer Problem
by Ahmed Refik

### File Structure:

    CMPUT379_Assign2.zip
    |
    |___threader.h
    |___threader.cpp
    |
    |___consumer.h
    |___consumer.cpp
    |
    |___producer.h
    |___producer.cpp
    |
    |___tands.h
    |___tands.cpp
    |
    |___extern.h
    |
    |___READM.md
    |___Makefile

### Design Decisions

    The reason I have some many header files in the directory is to avoid
    all include errors during the linking process. extern.h is just to
    prevent redefinition errors for the Info class

    The only functionally important header file is threader.h which contains
    Info class.

    Info is a class instead of a struct since I was having some werid issues
    with structs and a class seems to work fine.

    There isn't much error checking for most functions since, the description 
    seems to not care about it.

    My implementation also has it so the consumer threads don't ask before the
    producer has made any work available so, if the # of Asks are off by the 
    number of threads then thats why.

    I also changed tands.c into tands.cpp, but I didn't change any functions

### Running Instructions

    1- enter into project directory and run "make" with bash
    2- enter ./prodcon <number of consumer threads> -<id of prodcon.X.log>
    3- direct input either from keyboard or file
    4- results can be viewed in prodcon.X.log X=0 if no id is provided
    5- run "make clean" to clean up directory of all .o, .log, & .exe files

