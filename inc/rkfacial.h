#ifndef __RKFACIAL_H__
#define __RKFACIAL_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*display_callback)(void *ptr, int fd, int fmt, int w, int h, int rotation);

enum aiq_control_type {
    AIQ_CONTROL_RGB,
    AIQ_CONTROL_IR,
};

#ifdef __cplusplus
}
#endif

#endif
