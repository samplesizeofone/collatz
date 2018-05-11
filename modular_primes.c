#include <stdio.h>
#include <string.h>
#include <gmp.h>
#include <omp.h>

const unsigned long number_length = 10000000;
const int threads = 12;

typedef char number_t[1000000];

typedef struct thread_state_struct {
    mpz_t collatz_start;
    mpz_t prime_q_counter;
    mpz_t prime_q_x;
    mpz_t prime_q_square_root;
    mpz_t get_collatz_number_collatz_number;
    mpz_t get_collatz_number_start;
    mpz_t next_difference_number_x;
    mpz_t next_difference_number_y;
    mpz_t next_difference_number_difference_number;
    mpz_t next_first_start;
    mpz_t main_mod;
    mpz_t main_seq;
    number_t digits;
    number_t reverse;
    unsigned long nexts;
} thread_state_t;

thread_state_t thread_state[threads];

mpz_t temp1;
mpz_t temp2;

// In: start
// Out: digits
void collatz(thread_state_t *thread_state, unsigned long index) {
    if (mpz_cmp_ui(thread_state->collatz_start, 1) == 0) {
        thread_state->digits[index] = '\0';
        return;
    }

    if(mpz_divisible_ui_p(thread_state->collatz_start, 2)) {
        thread_state->digits[index] = '1';
        mpz_tdiv_q_ui(thread_state->collatz_start, thread_state->collatz_start, 2);
    } else {
        thread_state->digits[index] = '0';
        mpz_mul_ui(thread_state->collatz_start, thread_state->collatz_start, 3);
        mpz_add_ui(thread_state->collatz_start, thread_state->collatz_start, 1);
    }

    collatz(thread_state, index + 1);
}

// In: digits
// Out: reverse
void reverse_digits(thread_state_t *thread_state) {
    unsigned long length = strlen(thread_state->digits);
    for (unsigned long i = 0; i < length; i++) {
        thread_state->reverse[i] = thread_state->digits[length - i - 1];
    }
    thread_state->reverse[length] = '\0';
}

// In: x
int prime_q(thread_state_t *thread_state) {
    return 1;
    mpz_root(thread_state->prime_q_square_root, thread_state->prime_q_x, 2);
    mpz_set_ui(thread_state->prime_q_counter, 1);

    while (mpz_cmp(thread_state->prime_q_counter, thread_state->prime_q_square_root) < 0) {
        mpz_add_ui(thread_state->prime_q_counter, thread_state->prime_q_counter, 1);
        if (mpz_divisible_p(thread_state->prime_q_x, thread_state->prime_q_counter)) {
            return 0;
        }
    }

    return 1;
}

// In:
// Out: get_collatz_number_collatz_number
void get_collatz_number(thread_state_t *thread_state, unsigned long i) {
    mpz_set_ui(thread_state->collatz_start, i);
    collatz(thread_state, 0);

    reverse_digits(thread_state);

    mpz_set_str(thread_state->get_collatz_number_collatz_number, thread_state->reverse, 2);
}

// In:
// Out: thread_state->next_difference_number_difference_number
long next_difference_number(thread_state_t *thread_state, unsigned long start, unsigned long limit, unsigned long max) {
    unsigned long i = start;

    // printf("%lu %lu\n", start, max);
    while (i < start + max) {
        // printf("%lu, %lu %lu %lu\n", i, start + max, start, max);
        get_collatz_number(thread_state, i);
        mpz_set(thread_state->next_difference_number_x, thread_state->get_collatz_number_collatz_number);
        get_collatz_number(thread_state, i + 1);
        mpz_set(thread_state->next_difference_number_y, thread_state->get_collatz_number_collatz_number);

        mpz_sub(thread_state->next_difference_number_difference_number, thread_state->next_difference_number_y, thread_state->next_difference_number_x);
        mpz_set(thread_state->prime_q_x, thread_state->next_difference_number_difference_number);

        if (mpz_cmp_ui(thread_state->next_difference_number_difference_number, 3) > 0
            && mpz_cmp_ui(thread_state->next_difference_number_difference_number, limit) < 0
            && prime_q(thread_state)) {
            return i;
        }

        i++;
    }

    printf("out of room\n");
    return -1;
}

// In: thread_state->next_first_start
// Out: thread_state->next_first_start
long next_first(thread_state_t *thread_state, unsigned long start, unsigned long limit, unsigned long max) {
    unsigned long i = 0;
    unsigned long next = start + 1;
    thread_state->nexts = 0;

    while (i < max) {
        next = next_difference_number(thread_state, next, limit, max);

        if (next == -1) {
            printf("out of room\n");
            return -1;
        } else if (mpz_cmp(thread_state->next_difference_number_difference_number, thread_state->next_first_start) == 0) {
            thread_state->nexts = next;
            return next + 1;
        }

        i++;
        next = next + 1;
    }

    printf("out of room\n");
    return -1;
}

int main() {
    for (int i = 0; i < threads; i++) {
        mpz_init(thread_state[i].collatz_start);
        mpz_init(thread_state[i].prime_q_counter);
        mpz_init(thread_state[i].prime_q_x);
        mpz_init(thread_state[i].prime_q_square_root);
        mpz_init(thread_state[i].get_collatz_number_start);
        mpz_init(thread_state[i].get_collatz_number_collatz_number);
        mpz_init(thread_state[i].next_difference_number_x);
        mpz_init(thread_state[i].next_difference_number_y);
        mpz_init(thread_state[i].next_difference_number_difference_number);
        mpz_init(thread_state[i].next_first_start);
        mpz_init(thread_state[i].main_seq);
        mpz_init(thread_state[i].main_mod);
        thread_state[i].nexts = 0;
    }

    #pragma omp parallel for schedule(static)
    for (unsigned long ii = 1000; ii < 5000; ii++) {
        unsigned long seq_length = 10000000;
        unsigned long repeat = 100;
        unsigned long i = ii;
        // mpz_set_ui(thread_state[id].collatz_start, 87);
        // collatz(&thread_state[id], 0);

        // reverse_digits(&thread_state[id]);

        // printf("%s\n", thread_state[id].reverse);

        // get_collatz_number(&thread_state[id], 87);
        // mpz_out_str(stdout, 10, thread_state[id].get_collatz_number_collatz_number);
        // printf("cn\n");

	int id = omp_get_thread_num();
        //printf("%d %i\n", id, i);
        mpz_set_ui(thread_state[id].collatz_start, 0);
        mpz_set_ui(thread_state[id].prime_q_counter, 0);
        mpz_set_ui(thread_state[id].prime_q_x, 0);
        mpz_set_ui(thread_state[id].prime_q_square_root, 0);
        mpz_set_ui(thread_state[id].get_collatz_number_start, 0);
        mpz_set_ui(thread_state[id].get_collatz_number_collatz_number, 0);
        mpz_set_ui(thread_state[id].next_difference_number_x, 0);
        mpz_set_ui(thread_state[id].next_difference_number_y, 0);
        mpz_set_ui(thread_state[id].next_difference_number_difference_number,0 );
        mpz_set_ui(thread_state[id].next_first_start, 0);
        mpz_set_ui(thread_state[id].main_seq, 0);
        mpz_set_ui(thread_state[id].main_mod, 0);
        thread_state[id].nexts = 0;
 
        unsigned long limit = i;
        unsigned long max = 1000000000;

        next_difference_number(&thread_state[id], 1, limit, max);
        mpz_set(thread_state[id].next_first_start, thread_state[id].next_difference_number_difference_number);

        unsigned long l = 0;
        long next = 1;
        unsigned long k = 0;
        long base_next = 1;
        //printf("Starting %lu\n", i);
        printf("@");
        while (next != -1) {
            base_next = next_first(&thread_state[id], base_next, limit, max);
            if (base_next == -1) {
                printf("\n\n[Out of room for %ul]\n\n", i);
            }
            next = base_next;
            if (k == 0) {
                // next = next_first(&thread_state[id], next + 1, limit, max);
            }
            next--;
            l += thread_state[id].nexts;
            //mpz_out_str(stdout, 10, thread_state[id].next_difference_number_difference_number);
            //printf("NEXT %ld next\n", base_next);

            long seq = 1;

            unsigned long r = 0;

	    if (l > seq_length/repeat) {
	        printf("Mod %lu too long %lu\n", i, l);
                break;
            }
            while (1) {
                // printf("%ld %ld sn\n", seq, next);
                seq = next_difference_number(&thread_state[id], seq, limit, max);
                mpz_set(thread_state[id].main_seq, thread_state[id].next_difference_number_difference_number);
                // mpz_out_str(stdout, 10, thread_state[id].main_seq);
                // printf("seq\n");
                if (seq == -1) {
                    break;
                }
                seq++;
                next = next_difference_number(&thread_state[id], next, limit, max);
                mpz_set(thread_state[id].main_mod, thread_state[id].next_difference_number_difference_number);
                if (next == -1) {
                    break;
                }
                next++;
                // mpz_out_str(stdout, 10, thread_state[id].main_mod);
                // printf("mod\n");
                if (mpz_cmp(thread_state[id].main_mod, thread_state[id].main_seq) != 0) {
                    //printf("\n\nFail mod %lu for limit %lu\n\n", i, r);
                    printf(" !%lu ", i);
                    break;
                }

                if (r % 200000 == 199999) {
                    printf("*", i, r);
                }

                if (r > seq_length) {
                    //printf("\tSucceed mod %lu for limit %lu\n", i, r);
                    printf(" $%lu ", i);
                    next = -1;
                    break;
                }

                r++;
                k++;
            }
        }
    }

    return 0;
}
