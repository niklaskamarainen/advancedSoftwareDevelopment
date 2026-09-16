#ifndef WEEK3_BASE_H
#define WEEK3_BASE_H

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static inline void die(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

#endif
