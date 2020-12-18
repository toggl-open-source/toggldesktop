/*
 * Copyright 2016-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * Licensed under the Apache License 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * https://www.openssl.org/source/license.html
 * or in the file LICENSE in the source distribution.
 */

#ifndef OSSL_CRYPTO_RAND_H
# define OSSL_CRYPTO_RAND_H

# include <openssl/rand.h>

/*
 * Defines related to seed sources
 */
#ifndef DEVRANDOM
/*
 * set this to a comma-separated list of 'random' device files to try out. By
 * default, we will try to read at least one of these files
 */
# define DEVRANDOM "/dev/urandom", "/dev/random", "/dev/hwrng", "/dev/srandom"
# if defined(__linux) && !defined(__ANDROID__)
#  ifndef DEVRANDOM_WAIT
#   define DEVRANDOM_WAIT   "/dev/random"
#  endif
/*
 * Linux kernels 4.8 and later changes how their random device works and there
 * is no reliable way to tell that /dev/urandom has been seeded -- getentropy(2)
 * should be used instead.
 */
#  ifndef DEVRANDOM_SAFE_KERNEL
#   define DEVRANDOM_SAFE_KERNEL        4, 8
#  endif
/*
 * Some operating systems do not permit select(2) on their random devices,
 * defining this to zero will force the use of read(2) to extract one byte
 * from /dev/random.
 */
#  ifndef DEVRANDM_WAIT_USE_SELECT
#   define DEVRANDM_WAIT_USE_SELECT     1
#  endif
/*
 * Define the shared memory identifier used to indicate if the operating
 * system has properly seeded the DEVRANDOM source.
 */
#  ifndef OPENSSL_RAND_SEED_DEVRANDOM_SHM_ID
#   define OPENSSL_RAND_SEED_DEVRANDOM_SHM_ID 114
#  endif

# endif
#endif

#if !defined(OPENSSL_NO_EGD) && !defined(DEVRANDOM_EGD)
/*
 * set this to a comma-separated list of 'egd' sockets to try out. These
 * sockets will be tried in the order listed in case accessing the device
 * files listed in DEVRANDOM did not return enough randomness.
 */
# define DEVRANDOM_EGD "/var/run/egd-pool", "/dev/egd-pool", "/etc/egd-pool", "/etc/entropy"
#endif

void rand_cleanup_int(void);

/*
 * Initialise the random pool reseeding sources.
 *
 * Returns 1 on success and 0 on failure.
 */
int rand_pool_init(void);

/*
 * Finalise the random pool reseeding sources.
 */
void rand_pool_cleanup(void);

/*
 * Control the random pool use of open file descriptors.
 */
void rand_pool_keep_random_devices_open(int keep);

#endif
