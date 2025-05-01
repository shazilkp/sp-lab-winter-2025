#include <stdio.h>

int Fibo(int n){

    // Base case: if n is 0 or 1, return n
    if (n <= 1){
        return n;
    }

    // Recursive case: sum of the two preceding Fibonacci numbers
    return Fibo(n - 1) + Fibo(n - 2);
}

int main(){	
	int n;
	scanf("%d",&n);
	
	int result  = Fibo(n);
	printf("%d",result);
	return 0;
}
