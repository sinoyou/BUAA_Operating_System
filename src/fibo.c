#include<stdio.h>

int fibo(int num){
    if(num==1) return 1;
    else if (num==2)return 1;
    else return fibo(num-1)+fibo(num-2);
}

int main(){
    int var;
    scanf("%d", &var);
    int i ;
    for(i=1;i<=var;i++) printf("%d ",fibo(i));
    return 0;
}
