#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/ioctl.h>

#include <jni.h>
#include "android/log.h"

#include "../../common/puzzle_data.h"
#include "../../common/puzzle_dev.h"

#define LOG_TAG "PuzzleControllerNative"
#define LOGV(...)   __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

static void shuffle(int *arr, int n);

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
	srand(time(NULL));
	return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL Java_com_example_puzzlecontroller_PuzzleController_generate_1puzzle(JNIEnv *env, jobject this, jint row, jint col)
{
	LOGV("[Puzzle Controller - Native - Generate Puzzle] Row = %d, Col = %d", row, col);

	int num = row * col;
	int *arr = malloc (sizeof(*arr) * num);
	int i, j;

	for (i = 0; i < num; ++i)
		arr[i] = i;

	shuffle(arr, num);

	for (int i = 0; i < num; ++i)
		LOGV("[Puzzle Controller - Native - Generate Puzzle - Puzzle Info] %d: %d", i, arr[i]);

	int puzzle_fd = open("/dev/puzzle_dev", O_RDWR);

	if (puzzle_fd < 0)
	{
		LOGV("[Puzzle Controller - Native - Generate Puzzle] Opening Fail.");
		return;
	}

	struct __puzzle_data puzzle_data;

	puzzle_data.row = row;
	puzzle_data.col = col;

	for (i = 0; i < row; ++i)
		for (j = 0; j < col; ++j)
			puzzle_data.mat[i][j] = arr[i * col + j];

	ioctl(puzzle_fd, PUZZLE_PUT, &puzzle_data);

	free(arr);

	close(puzzle_fd);
}

void shuffle(int *arr, int n)
{
    if (n > 1)
    {
    	int i;
        for (i = 0; i < n - 1; i++)
        {
          int j = i + rand() / (RAND_MAX / (n - i) + 1);
          int t = arr[j];
          arr[j] = arr[i];
          arr[i] = t;
        }
    }
}
