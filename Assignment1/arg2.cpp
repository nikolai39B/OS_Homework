#include <iostream>
#include <unistd.h>  /* _exit, fork */
#include <stdlib.h>  /* exit */
#include <errno.h>   /* errno */

using namespace std;

int main(int argc, char* argv[])
{      
     if (argc <= 2)
        cout << "Usage: " << argv[0]  << " / IS\n";
}


