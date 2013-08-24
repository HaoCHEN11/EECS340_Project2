#include <stdio.h>
#include "buffer.h"

int main(){
	Buffer b;
	Buffer add("hello", 5);
	b.AddBack(add);
	printf("%d\n", b.GetSize());
	b.Erase(0, 3);
	Buffer add1("hello1", 6);
	b.AddBack(add1);
        printf("%d\n", b.GetSize());
}
