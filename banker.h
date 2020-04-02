#ifndef BANKER_H_
#define BANKER_H_

#include <stdbool.h>

extern bool request_res(int n_customer, int request[]);

extern bool release_res(int n_customer, int release[]);

extern bool is_safe();

#endif
