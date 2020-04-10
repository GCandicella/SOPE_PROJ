#include <stdio.h>
#include <stdlib.h>
int main(){
    system("make");
    system("(./simpledu ~/ || echo $?) | sort -k2 > testeT_simpledu.txt");
    system("(du ~/ || echo $?) | sort -k2 > testeT_du.txt");
    system("diff -q testeT_simpledu.txt testeT_du.txt > /dev/null 2>&1 && echo OK || echo FAILED");
    //system("rm testeT_du.txt testeT_simpledu.txt a.out");
    return EXIT_SUCCESS;
}
