#include <gmp.h>
#include <omp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "aks.c"

const int number_of_threads = 4;
const int number_of_digits = 1000000;

mpz_t gmp_0;
mpz_t gmp_1;
mpz_t gmp_2;
mpz_t gmp_3;

typedef struct collatz_prime_pointer_struct {
    mpz_t collatz_index;
    mpz_t prime_index;
} collatz_prime_pointer_t;

typedef struct thread_state_struct {
    mpz_t calc_collatz_digits__start;
    char calc_collatz_digits__digits[number_of_digits];

    mpz_t collatz_number;
    char parse_collatz_digits__digits[number_of_digits];

    mpz_t prime_q__root;
    mpz_t prime_q__x;

    mpz_t calc_collatz_difference__x;
    mpz_t calc_collatz_difference__start;

    mpz_t collatz_prime_q__max;

    mpz_t collatz_difference;

    mpz_t find_next_collatz_prime__collatz_number_index;

    collatz_prime_pointer_t head;

    mpz_t periodic_q__repetitions;
    mpz_t periodic_q__i;
    mpz_t periodic_q__steps;
    mpz_t periodic_q__x;
    collatz_prime_pointer_t periodic_q__current_head;
    collatz_prime_pointer_t periodic_q__current_index;
} thread_state_t;

thread_state_t state[number_of_threads];

// void f(int t, );

// In: calc_collatz_digits__start
// Out: calc_collatz_digits__digits
void calc_collatz_digits(int t, int index) {
    if (mpz_cmp(state[t].calc_collatz_digits__start, gmp_1) == 0) {
        state[t].calc_collatz_digits__digits[index] = '\0';
        return;
    }

    if (mpz_divisible_p(state[t].calc_collatz_digits__start, gmp_2)) {
        mpz_tdiv_q(state[t].calc_collatz_digits__start, state[t].calc_collatz_digits__start, gmp_2);
        state[t].calc_collatz_digits__digits[index] = '1';
    } else {
        mpz_mul(state[t].calc_collatz_digits__start, state[t].calc_collatz_digits__start, gmp_3);
        mpz_add(state[t].calc_collatz_digits__start, state[t].calc_collatz_digits__start, gmp_1);
        state[t].calc_collatz_digits__digits[index] = '0';
    }

    calc_collatz_digits(t, index + 1);
}

// In: calc_collatz_digits__digits
// Out: collatz_number
void parse_collatz_digits(int t) {
    int length = strlen(state[t].calc_collatz_digits__digits);

    for (int i = 0; i < length; i++) {
        state[t].parse_collatz_digits__digits[i] = state[t].calc_collatz_digits__digits[length - i - 1];
    }
    state[t].parse_collatz_digits__digits[length] = '\0';

    int r = mpz_set_str(state[t].collatz_number, state[t].parse_collatz_digits__digits, 2);
}

// In: collatz_number
int prime_q(int t) {
    // mpz_out_str(stdout, 10, state[t].collatz_number);
    // printf(" collatzing\n");
    // printf("Entering prime\n");
    int r = aks(state[t].collatz_difference);
    // printf("Exiting prime\n");
    return r;

    int probably_prime = mpz_probab_prime_p(state[t].collatz_number, 50);

    if (probably_prime == 0) {
        return 0;
    } else if (probably_prime == 2) {
        return 1;
    }

    // printf("%d pp\n", probably_prime);

    mpz_sqrt(state[t].prime_q__root, state[t].collatz_number);
    mpz_set(state[t].prime_q__x, gmp_1);
    mpz_add(state[t].prime_q__root, state[t].prime_q__root, gmp_3);
    mpz_out_str(stdout, 10, state[t].collatz_number);
    // printf(" collatzing\n");
    mpz_out_str(stdout, 10, state[t].prime_q__root);
    // printf(" rooting\n");

    // printf("entering prime_q\n");
    while (mpz_cmp(state[t].prime_q__x, state[t].prime_q__root) < 0) {
        mpz_add(state[t].prime_q__x, state[t].prime_q__x, gmp_1);
        // mpz_out_str(stdout, 10, state[t].prime_q__x);
        //printf(" priming\n");
        if (mpz_divisible_p(state[t].collatz_number, state[t].prime_q__x)) {
            // printf("leaving prime_q\n");
            return 0;
        }

    }
    // printf("is prime_q\n");

    return 1;
}

// In: collatz_number
int collatz_prime_q(int t) {
    return prime_q(t)
        && mpz_cmp(state[t].collatz_difference, gmp_3) > 0
        && mpz_cmp(state[t].collatz_difference, state[t].collatz_prime_q__max) < 0;
}

// In: collatz_digits__start
// Out: collatz_difference
void calc_collatz_difference(int t) {
    mpz_set(state[t].calc_collatz_difference__start, state[t].calc_collatz_digits__start);

    calc_collatz_digits(t, 0);
    parse_collatz_digits(t);
    mpz_set(state[t].calc_collatz_difference__x, state[t].collatz_number);

    mpz_set(state[t].calc_collatz_digits__start, state[t].calc_collatz_difference__start);
    mpz_add(state[t].calc_collatz_digits__start, state[t].calc_collatz_digits__start, gmp_1);

    calc_collatz_digits(t, 0);
    parse_collatz_digits(t);

    mpz_sub(state[t].collatz_difference, state[t].collatz_number, state[t].calc_collatz_difference__x);
}

// In: find_next_collatz_prime__collatz_number_index
// Out: find_next_collatz_prime__collatz_number_index
void find_next_collatz_prime(int t) {
    // mpz_add(state[t].find_next_collatz_prime__collatz_number_index, state[t].find_next_collatz_prime__collatz_number_index, gmp_1);
    unsigned long i = 0;
    do {
        // mpz_out_str(stdout, 10, state[t].find_next_collatz_prime__collatz_number_index);
        // printf(" cni\n");
        mpz_add(state[t].find_next_collatz_prime__collatz_number_index, state[t].find_next_collatz_prime__collatz_number_index, gmp_1);
        mpz_set(state[t].calc_collatz_digits__start, state[t].find_next_collatz_prime__collatz_number_index);
        calc_collatz_difference(t);
        // mpz_out_str(stdout, 10, state[t].collatz_difference);
        // printf(" N\n");
        i++;
        // printf("%d ii\n", i);
    } while(!collatz_prime_q(t));
}

void forward_prime_pointer(int t, collatz_prime_pointer_t *prime_pointer) {
    mpz_set(state[t].find_next_collatz_prime__collatz_number_index, prime_pointer->collatz_index);
    mpz_add(prime_pointer->collatz_index, prime_pointer->collatz_index, gmp_1);
    find_next_collatz_prime(t);
    mpz_set(prime_pointer->collatz_index, state[t].find_next_collatz_prime__collatz_number_index);
    mpz_add(prime_pointer->prime_index, prime_pointer->prime_index, gmp_1);
    mpz_set(prime_pointer->collatz_index, state[t].find_next_collatz_prime__collatz_number_index);
}

void init_prime_pointer(int t, collatz_prime_pointer_t *prime_pointer) {
    mpz_init(prime_pointer->collatz_index);
    mpz_init(prime_pointer->prime_index);
}

void set_prime_pointer(int t, collatz_prime_pointer_t *prime_destination, collatz_prime_pointer_t *prime_source) {
    mpz_set(prime_destination->collatz_index, prime_source->collatz_index);
    mpz_set(prime_destination->prime_index, prime_source->prime_index);
}

void clear_prime_pointer(int t, collatz_prime_pointer_t *prime_pointer) {
    mpz_set_ui(prime_pointer->collatz_index, 1);
    mpz_set_ui(prime_pointer->prime_index, 0);
}

// Out: collatz_number
void get_collatz_prime_at_pointer(int t, collatz_prime_pointer_t *prime_pointer) {
    mpz_set(state[t].calc_collatz_digits__start, prime_pointer->collatz_index);
    calc_collatz_digits(t, 0);
    parse_collatz_digits(t);
}

// In: periodic_q__repetitions, head
int periodic_q(int t) {

    mpz_set_ui(state[t].periodic_q__i, 0);
    mpz_set(state[t].periodic_q__steps, state[t].periodic_q__repetitions);
    mpz_mul(state[t].periodic_q__steps, state[t].periodic_q__repetitions, state[t].head.prime_index);

    clear_prime_pointer(t, &state[t].periodic_q__current_index);
    set_prime_pointer(t, &state[t].periodic_q__current_head, &state[t].head);

    while(mpz_cmp(state[t].periodic_q__i, state[t].periodic_q__steps) < 0) {
        get_collatz_prime_at_pointer(t, &state[t].periodic_q__current_head);
        mpz_set(state[t].periodic_q__x, state[t].collatz_number);
        get_collatz_prime_at_pointer(t, &state[t].periodic_q__current_index);
        // mpz_out_str(stdout, 10, state[t].periodic_q__current_head.collatz_index);
        // printf(" stepping\n");

        if (mpz_cmp(state[t].periodic_q__x, state[t].collatz_number) != 0) {
            return 0;
        }

        mpz_add(state[t].periodic_q__i, state[t].periodic_q__i, gmp_1);
        forward_prime_pointer(t, &state[t].periodic_q__current_head);
    }

    return 1;

}

int main() {
    mpz_init(gmp_0);
    mpz_init(gmp_1);
    mpz_init(gmp_2);
    mpz_init(gmp_3);

    mpz_set_ui(gmp_3, 0);
    mpz_set_ui(gmp_1, 1);
    mpz_set_ui(gmp_2, 2);
    mpz_set_ui(gmp_3, 3);

    for (int i = 0; i < number_of_threads; i++) {
        mpz_init(state[i].calc_collatz_digits__start);

        mpz_init(state[i].collatz_number);

        mpz_init(state[i].prime_q__x);
        mpz_init(state[i].prime_q__root);

        mpz_init(state[i].calc_collatz_difference__x);
        mpz_init(state[i].calc_collatz_difference__start);

        mpz_init(state[i].collatz_difference);

        mpz_init(state[i].find_next_collatz_prime__collatz_number_index);

        mpz_init(state[i].collatz_prime_q__max);

        mpz_init(state[i].periodic_q__steps);
        mpz_init(state[i].periodic_q__x);
        init_prime_pointer(i, &state[i].periodic_q__current_head);
        init_prime_pointer(i, &state[i].periodic_q__current_index);
        mpz_init(state[i].periodic_q__i);
        mpz_init(state[i].periodic_q__repetitions);

        init_prime_pointer(i, &state[i].head);
    }

    mpz_set_ui(state[0].calc_collatz_digits__start, 10);
    calc_collatz_digits(0, 0);
    printf("%s\n", state[0].calc_collatz_digits__digits);

    parse_collatz_digits(0);
    mpz_out_str(stdout, 10, state[0].collatz_number);
    printf(" C(10)\n");

    mpz_set_ui(state[0].calc_collatz_digits__start, 10);
    calc_collatz_difference(0);
    mpz_out_str(stdout, 10, state[0].collatz_difference);
    printf(" C(10) - C(9)\n");

    mpz_set_ui(state[0].collatz_prime_q__max, 1000);

    mpz_set_ui(state[0].find_next_collatz_prime__collatz_number_index, 2);
    for (int i = 0; i < 3; i++) {
        find_next_collatz_prime(0);
        mpz_out_str(stdout, 10, state[0].find_next_collatz_prime__collatz_number_index);
        printf(" next\n");

        mpz_set(state[0].calc_collatz_digits__start, state[0].find_next_collatz_prime__collatz_number_index);
        calc_collatz_difference(0);
        mpz_out_str(stdout, 10, state[0].collatz_difference);
        printf(" CN2\n");
    }

    printf("there\n");
    char str[1000]; 
    int block = 0;
    while (1) {
        unsigned long start = block*16 + 1000000000000000;
        unsigned long end = (block + 1)*16 + 1000000000000000;

        #pragma omp parallel for num_threads(4)
        for (unsigned long i = start; i < end; i++) {
            int t = omp_get_thread_num();

            printf("Starting %ld\n", i);

            clear_prime_pointer(t, &state[t].periodic_q__current_head);
            clear_prime_pointer(t, &state[t].periodic_q__current_index);

            sprintf(str, "%ld", i);
            mpz_set_str(state[t].head.collatz_index, str, 10);
            mpz_set_ui(state[t].periodic_q__repetitions, 50);

            sprintf(str, "%ld", 1000000000000000);
            mpz_set_str(state[t].head.collatz_index, str, 10);
            mpz_set_ui(state[t].head.prime_index, 1);

            printf("Periodic: %d = %d\n", i, periodic_q(t));
        }
        block++;
    }

    return 0;
}