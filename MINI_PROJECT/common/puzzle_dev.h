#ifndef __PUZZLE_DEV__
#define __PUZZLE_DEV__

#define PUZZLE_MAJOR 242
#define PUZZLE_MINOR 0
#define PUZZLE_NAME "puzzle_dev"



/* ioctl macros */
#define PUZZLE_PUT  _IOW(PUZZLE_MAJOR, 1, struct __puzzle_data*)
#define PUZZLE_DATA_ENABLE  _IOW(PUZZLE_MAJOR, 2, int*)
#define PUZZLE_GET  _IOR(PUZZLE_MAJOR, 3, struct __puzzle_data*)
#define PUZZLE_GET_ENABLE  _IOR(PUZZLE_MAJOR, 4, int*)

#endif /* __PUZZLE_DEV__ */
