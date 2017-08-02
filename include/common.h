/*  =========================================================================
    common - class description

    Copyright (c) the Authors
    =========================================================================
*/

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

//  @interface
//  Create a new common
CZGUIDE_EXPORT common_t *
    common_new (void);

//  Destroy the common
CZGUIDE_EXPORT void
    common_destroy (common_t **self_p);

//  Self test of this class
CZGUIDE_EXPORT void
    common_test (bool verbose);

//  @end

#ifdef __cplusplus
}
#endif

#endif
