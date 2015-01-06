#include <iostream>
#include <sstream>
using namespace std;
int main()
{
    stringstream a;

    for( int i = 0 ; i < 10 ; i++ ){
        a.str("");
        a << i ;
        cout << a.str();
    }
	return 0;
}
