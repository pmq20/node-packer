/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#ifndef LIBSQUASH_MUTEX_H
#define LIBSQUASH_MUTEX_H

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
#else
    #include <pthread.h>
#endif

#ifdef _WIN32
   #define MUTEX HANDLE
#else
   #define MUTEX pthread_mutex_t
#endif

extern MUTEX squash_global_mutex;

int MUTEX_INIT(MUTEX *mutex);
int MUTEX_LOCK(MUTEX *mutex);
int MUTEX_UNLOCK(MUTEX *mutex);
int MUTEX_DESTORY(MUTEX *mutex);

#endif //LIBSQUASH_MUTEX_H
