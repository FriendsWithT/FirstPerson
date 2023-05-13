#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define UINT16 uint16_t
#define UINT32 uint32_t

#define IPC_PIPE_NAME "\\\\.\\pipe\\my_fps_pipe"

#define PI 3.14159
#define BILLION 1000000000L

#define RADIAN_TO_DEGREE(fRadian) (((int) round((fRadian * 180.0) / PI)) % 360)