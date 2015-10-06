/*  Copyright (c) 2006-2008, Philip Busch <philip@0xe3.com>
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *   - Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_BASE64_H_
#define PERFEXPERT_BASE64_H_

#ifndef _STDLIB_H
#include <stdlib.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Encode a string. This is a convenience function. It encodes the first
 *  \c size bytes of the string pointed to by \c in, stores the null-terminated
 *  result in a newly created memory area and returns a pointer to it.
 *
 * @attention After a call to perfexpert_base64_encode(), you have to free() the
 *  result yourself.
 *
 * @param in pointer to string
 * @param size strlen
 * @returns NULL on error (not enough memory) or a pointer to the encoded result
 *
 * @ingroup base64
 */
char *perfexpert_base64_encode(const char *in, size_t size);

/** Decode a string. This is a convenience function. It decodes the
 *  null-terminated string pointed to by \c in, stores the result in a newly
 *  created memory area and returns a pointer to it. The result will be
 *  null-terminated.
 *
 * @attention After a call to perfexpert_base64_decode(), you have to free() the
 *  result yourself.
 *
 * @param in pointer to string
 * @returns NULL on error (not enough memory) or a pointer to the decoded result
 *
 * @ingroup base64
 */
char *perfexpert_base64_decode(const char *in);

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_BASE64_H */
