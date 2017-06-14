/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "squash/mutex.h"


int MUTEX_INIT(MUTEX *mutex)
{

#ifdef _WIN32
    *mutex = CreateMutex(0, FALSE, 0);
    return (*mutex==0);
#else
    return pthread_mutex_init(mutex, NULL);
#endif

}

int MUTEX_LOCK(MUTEX *mutex)
{
#ifdef _WIN32
    return (WaitForSingleObject(*mutex, INFINITE)==WAIT_FAILED?1:0);
#else
    return pthread_mutex_lock(mutex);
#endif
}

int MUTEX_UNLOCK(MUTEX *mutex)
{
#ifdef _WIN32
    return (ReleaseMutex(*mutex)==0);
#else
    return pthread_mutex_unlock(mutex);
#endif
}

int MUTEX_DESTORY(MUTEX *mutex)
{
#ifdef _WIN32
    return CloseHandle(*mutex);
#else
    return pthread_mutex_destroy(mutex);
#endif
}
