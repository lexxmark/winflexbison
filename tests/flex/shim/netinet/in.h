/* Test-only shim for <netinet/in.h> on MSVC.
 *
 * flex's serialized-tables scanner (--tables-file / --tables-verify) emits an
 * unconditional `#include <netinet/in.h>`, used solely for ntohs()/ntohl() when
 * byte-swapping the serialized DFA. MSVC has no such header, so the generated
 * output is not self-compilable on Windows without help. This shim supplies the
 * four byte-order helpers via MSVC intrinsics so the table tests can build.
 *
 * NOTE: this documents a real winflexbison porting gap — win_flex's serialized-
 * tables output needs a netinet/in.h substitute to compile under MSVC. See
 * docs/specs/03-test-adoption/spec.md.
 */
#ifndef WINFLEXBISON_TEST_NETINET_IN_H
#define WINFLEXBISON_TEST_NETINET_IN_H

#include <stdlib.h>   /* _byteswap_ushort / _byteswap_ulong (MSVC) */

/* Host (x86/x64) is little-endian, network order is big-endian: always swap. */
static __inline unsigned short ntohs(unsigned short x) { return _byteswap_ushort(x); }
static __inline unsigned short htons(unsigned short x) { return _byteswap_ushort(x); }
static __inline unsigned long  ntohl(unsigned long  x) { return _byteswap_ulong(x);  }
static __inline unsigned long  htonl(unsigned long  x) { return _byteswap_ulong(x);  }

#endif /* WINFLEXBISON_TEST_NETINET_IN_H */
