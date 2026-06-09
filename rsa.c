#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

const int P = 31;
const int Q = 23;
const int E = 19;
const int PHI = (P - 1) * (Q - 1);


// finding d which is coprime to phi and satisfies (E * D) % PHI == 1
int find_d(){
    int D = 1;
    while ((E * D) % PHI != 1){
        D++;
    }
    return D;
}



// binary function
int binary(int message){
    int num_of_bits = 0;
    int cursor = message;
    while (cursor > 1){
        cursor = cursor>>1;
        num_of_bits++;
    }
    return num_of_bits;
}



//square function
__attribute__((aligned(128))) int square(int d){
    d = (d*d) % (P*Q);
    return d;
}



//multiply function
__attribute__((aligned(128))) int multiply(int message, int horner){
    int result = (message*horner)%(P*Q);
    return result;
}

int horner_loop(int message, int d){

    int num_of_bits = binary(d);
    int horner = 1;

    for (int i = num_of_bits; i >= 0; i--){
        horner = square(horner);
        if ((d >> i) & 1){
            horner = multiply(horner, message);
        }
        for (volatile int i = 0; i < 5000000; i++);

    }
    return horner;
}


// main function
int main(int argc, char *argv[]){
    /*
    if (argc != 2){
        printf("Wrong number of arguments.\n");
        return 1;
    }
    */

    
    int message = 35;
    int d = find_d();

    
    int encrypted_message = horner_loop(message, E);


    while(1) {
 
    int decrypted_message = horner_loop(encrypted_message, d);

    }
    
}

