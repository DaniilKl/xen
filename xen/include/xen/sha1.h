/* SPDX-License-Identifier: GPL-2.0 */

#ifndef XEN__SHA1_H
#define XEN__SHA1_H

#include <xen/inttypes.h>

#define SHA1_DIGEST_SIZE  20

void sha1_hash(uint8_t digest[SHA1_DIGEST_SIZE], const void *msg, size_t len);

#endif /* XEN__SHA1_H */
