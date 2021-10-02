#ifndef PARAMS_H
#define PARAMS_H

#include "config.h"

#define SEEDBYTES 32
#define CRHBYTES 48
#define _N_ 256
#define _Q_ 8380417
#define _D_ 13
#define ROOT_OF_UNITY 1753

#if DILITHIUM_MODE == 2
#define _K_ 4
#define _L_ 4
#define ETA 2
#define TAU 39
#define BETA 78
#define GAMMA1 (1 << 17)
#define GAMMA2 ((_Q_-1)/88)
#define OMEGA 80

#elif DILITHIUM_MODE == 3
#define _K_ 6
#define _L_ 5
#define ETA 4
#define TAU 49
#define BETA 196
#define GAMMA1 (1 << 19)
#define GAMMA2 ((_Q_-1)/32)
#define OMEGA 55

#elif DILITHIUM_MODE == 5
#define _K_ 8
#define _L_ 7
#define ETA 2
#define TAU 60
#define BETA 120
#define GAMMA1 (1 << 19)
#define GAMMA2 ((_Q_-1)/32)
#define OMEGA 75

#endif

#define POLYT1_PACKEDBYTES  320
#define POLYT0_PACKEDBYTES  416
#define POLYVECH_PACKEDBYTES (OMEGA + _K_)

#if GAMMA1 == (1 << 17)
#define POLYZ_PACKEDBYTES   576
#elif GAMMA1 == (1 << 19)
#define POLYZ_PACKEDBYTES   640
#endif

#if GAMMA2 == (_Q_-1)/88
#define POLYW1_PACKEDBYTES  192
#elif GAMMA2 == (_Q_-1)/32
#define POLYW1_PACKEDBYTES  128
#endif

#if ETA == 2
#define POLYETA_PACKEDBYTES  96
#elif ETA == 4
#define POLYETA_PACKEDBYTES 128
#endif

#define CRYPTO_PUBLICKEYBYTES (SEEDBYTES + _K_*POLYT1_PACKEDBYTES)
#define CRYPTO_SECRETKEYBYTES (2*SEEDBYTES + CRHBYTES \
                               + _L_*POLYETA_PACKEDBYTES \
                               + _K_*POLYETA_PACKEDBYTES \
                               + _K_*POLYT0_PACKEDBYTES)
#define CRYPTO_BYTES (SEEDBYTES + _L_*POLYZ_PACKEDBYTES + POLYVECH_PACKEDBYTES)

#endif
