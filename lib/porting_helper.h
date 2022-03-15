#ifndef MM_LIBPORTING_H
#define MM_LIBPORTING_H

#if defined __cplusplus
extern "C" {
#endif

bool is_an_mmsafe_ptr(void *p);
void insert_mmsafe_ptr(void *p);
void erase_mmsafe_ptr(void *p);

#if defined __cplusplus
}
#endif

#endif
