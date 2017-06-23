#ifndef __STRUCT_PUZZLE_DATA__

#define MAX_ROW 20
#define MAX_COL 20

struct __puzzle_data {
  int row;
  int col;
  int mat[MAX_ROW][MAX_COL];
};

#endif /* __STRUCT_PUZZLE_DATA__ */
