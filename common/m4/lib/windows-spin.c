/* Spin locks (native Windows implementation).
   Copyright (C) 2019-2021 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2019.  */

#include <config.h>

/* Specification.  */
#include "windows-spin.h"

#include <errno.h>

void
glwthread_spin_init (glwthread_spinlock_t *lock)
{
  lock->word = 0;
  MemoryBarrier ();
}

int
glwthread_spin_lock (glwthread_spinlock_t *lock)
{
  /* Wait until lock->word becomes 0, then replace it with 1.  */
  /* InterlockedCompareExchange
     <https://docs.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-interlockedcompareexchange> */
  while (InterlockedCompareExchange (&lock->word, 1, 0))
    ;
  return 0;
}

int
glwthread_spin_trylock (glwthread_spinlock_t *lock)
{
  /* If lock->word is 0, then replace it with 1.  */
  /* InterlockedCompareExchange
     <https://docs.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-interlockedcompareexchange> */
  if (InterlockedCompareExchange (&lock->word, 1, 0))
    return EBUSY;
  return 0;
}

int
glwthread_spin_unlock (glwthread_spinlock_t *lock)
{
  /* If lock->word is 1, then replace it with 0.  */
  /* InterlockedCompareExchange
     <https://docs.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-interlockedcompareexchange> */
  if (!InterlockedCompareExchange (&lock->word, 0, 1))
    return EINVAL;
  return 0;
}

int
glwthread_spin_destroy (glwthread_spinlock_t *lock)
{
  return 0;
}
