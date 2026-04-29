Here are a few observations about this program:

    This is a Standard ISO C++ program using the standard library. Standard library facilities are declared in namespace std in headers without a .h suffix.

    If you want to compile this on a Windows machine, you need to compile it as a "console application". Remember to give your source file the .cpp suffix or the compiler might think that it is C (not C++) source.

    Yes, main() returns an int.

    Reading into a standard vector guarantees that you don't overflow some arbitrary buffer. Reading into an array without making a "silly error" is beyond the ability of complete novices - by the time you get that right, you are no longer a complete novice. If you doubt this claim, I suggest you read my paper "Learning Standard C++ as a New Language", which you can download from my publications list.

    The !cin.eof() is a test of the stream's format. Specifically, it tests whether the loop ended by finding end-of-file (if not, you didn't get input of the expected type/format). For more information, look up "stream state" in your C++ textbook.

    A vector knows its size, so I don't have to count elements.

    Yes, I know that I could declare i to be a vector<double>::size_type rather than plain int to quiet warnings from some hyper-suspicious compilers, but in this case,I consider that too pedantic and distracting.

    This program contains no explicit memory management, and it does not leak memory. A vector keeps track of the memory it uses to store its elements. When a vector needs more memory for elements, it allocates more; when a vector goes out of scope, it frees that memory. Therefore, the user need not be concerned with the allocation and deallocation of memory for vector elements.

    for reading in strings, see How do I read a string from input?.

    The program ends reading input when it sees "end of file". If you run the program from the keybord on a Unix machine "end of file" is Ctrl-D. If you are on a Windows machine that because of a bug doesn't recognize an end-of-file character, you might prefer this slightly more complicated version of the program that terminates input with the word "end":

```c++
#include<iostream>
#include<vector>
#include<algorithm>
#include<string>

using namespace std;

int main()
{
    vector<double> v;

    double d;
    while(cin>>d) v.push_back(d);	// read elements
    if (!cin.eof()) {		// check if input failed
        cin.clear();		// clear error state
        string s;
        cin >> s;		// look for terminator string
        if (s != "end") {
            cerr << "format error\n";
            return 1;	// error return
        }
    }

    cout << "read " << v.size() << " elements\n";

    reverse(v.begin(),v.end());
    cout << "elements in reverse order:\n";
    for (int i = 0; i<v.size(); ++i) cout << v[i] << '\n';

    return 0; // success return
}
```

For more examples of how to use the standard library to do simple things simply, see the "Tour of the Standard Library" Chapter of TC++PL4.
