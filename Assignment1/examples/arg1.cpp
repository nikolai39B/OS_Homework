#include <iostream>
#include <unistd.h>  /* _exit, fork */
#include <stdlib.h>  /* exit */
#include <errno.h>   /* errno */

using namespace std;

int main(int argc, char* argv[])
{      
     cout << "This program has " << argc << " arguments." << endl;
     for(int i=0; i<argc; i++)
     {
       cout << i << " : " << argv[i] << endl;
     } 
}


