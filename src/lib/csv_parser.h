
#ifndef CSVPARSERC_H_
#define CSVPARSERC_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_IMPLEMENTATION
#include "../../resources/arena.h"

#ifndef CUSTOM_ALLOC
#define CUSTOM_ALLOC
#define CSV_ALLOC _arena_context_alloc_noshare
#endif

// #define MIN_ARENA 1048576 // 2^20 bytes

#define MAX_HEADER_NAME 80
#define MAX_NAME_CELL 80

#ifndef SEPARATION_CARACTER
#define SEPARATION_CARACTER ','
#endif

typedef struct _csv_adt {
  char **csv_columns_names;
  char ***csv_raw_data;
  size_t csv_current_count_row;
  size_t _columns_count;
  size_t _rows_count;
  size_t _max_name_cell;
  bool _full_data;
  FILE *_pdp;
  Arena *_context_arena;
} csv_adt;

void * //
_arena_context_alloc_noshare(csv_adt *adt, size_t size);

char ** // ["1", "chat", "csv", "0"]
csvc_read_idx_row(csv_adt *adt, int line);

char * // "1"
csvc_item_idx_row_colum(csv_adt *adt, int line, int colum);

char ** // ["city", "fortal", "maracanau"]
csvc_item_idx_colum(csv_adt *adt, int colum);

csv_adt * // read all data to memory
csvc_dump_csv(char *file_path);

csv_adt * //
csvc_init_read_file_path(char *file_name);

void //
csvc_close_file_path(csv_adt *adt);

void //
csvc_free_context(csv_adt *adt);

char ** //
_csvc_parser_line(csv_adt *adt, char *buff);

void * //
_csvc_parser_colum(char *buff, int column_num);

char ** //
_csvc_alloc_chr_array(csv_adt *adt, size_t arr_size);

size_t //
_csvc_get_idx_chr(char *buff, char chr);

size_t //
_csvc_count_chr_on_str(char *str, char chr);

size_t //
_csvc_count_columns(char **buff);

#endif // CSVPARSERC_H_

#ifdef CSVPARSERC_IMPLEMENTATION

void * // thread safe std alloc
_arena_context_alloc_noshare(csv_adt *adt, size_t size) {
  assert(adt->_context_arena && "arena not pass");
  return arena_alloc(adt->_context_arena, size);
}

char ** // ["1", "chat", "csv", "0"]
csvc_read_idx_row(csv_adt *adt, int line) {
  assert(adt);
  assert(line);
}

char * // "1"
csvc_item_idx_row_colum(csv_adt *adt, int line, int colum) {
  assert(adt);
  assert(line);
  assert(colum);
}

char ** // ["city", "fortal", "maracanau"]
csvc_item_idx_colum(csv_adt *adt, int colum) {
  assert(adt);
  assert(colum);
}

size_t //
_csvc_count_lines(char *file_name_path) {
  FILE *file;
  char ch;
  size_t count = 0;

  // Open the file in read mode
  file = fopen(file_name_path, "ro");

  // Check if the file exists
  if (file == NULL) {
    printf("File not found.\n");
    return 0;
  }

  // Count the lines
  while ((ch = fgetc(file)) != EOF) {
    if (ch == '\n') {
      count++;
    }
  }

  // Close the file
  fclose(file);

  return count;
}

csv_adt * // read all data to memory
csvc_dump_csv(char *file_path) {
  // assert(file_path);

  size_t read;
  char *line_ptr = NULL;
  size_t len = 0;

  size_t file_lines_number = _csvc_count_lines(file_path);

  file_lines_number -= 1; // for header file

  csv_adt *ctx_adt = csvc_init_read_file_path(file_path);

  ctx_adt->_rows_count = file_lines_number;

  char ***raw_data =
      (char ***)CSV_ALLOC(ctx_adt, sizeof(char ***) * file_lines_number);

  getline(&line_ptr, &len, ctx_adt->_pdp);

  ctx_adt->csv_columns_names = _csvc_parser_line(ctx_adt, line_ptr);

  const size_t columns_count = _csvc_count_columns(ctx_adt->csv_columns_names);

  ctx_adt->_columns_count = columns_count;

  // printf("Hello World\n");

  ctx_adt->_full_data = true;

  size_t relative_count = 0;

  while ((read = getline(&line_ptr, &len, ctx_adt->_pdp)) != (size_t)-1) {

    raw_data[relative_count] = _csvc_parser_line(ctx_adt, line_ptr);

    relative_count += 1;
  }

  free(line_ptr);

  ctx_adt->csv_raw_data = raw_data;

  return ctx_adt;
}

csv_adt * //
csvc_init_read_file_path(char *file_name_path) {

  assert(file_name_path);

  FILE *fp = fopen(file_name_path, "ro");

  if (fp == NULL) {
    printf("file not open correcly :: %s", file_name_path);
    assert(fp);
    return (csv_adt *)NULL;
  }

  csv_adt *_adt_new = (csv_adt *)malloc(sizeof(csv_adt));
  Arena *_arena_new = (Arena *)malloc(sizeof(Arena));

  memset(_arena_new, 0, sizeof(Arena));

  _adt_new->csv_current_count_row = 0;
  _adt_new->_rows_count = 0;
  _adt_new->csv_raw_data = NULL;

  _adt_new->_full_data = false;
  _adt_new->_max_name_cell = MAX_NAME_CELL;
  _adt_new->_columns_count = 0;

  _adt_new->_pdp = fp;

  _adt_new->_context_arena = _arena_new;

  return _adt_new;
}

void //
csvc_close_file_path(csv_adt *adt) {

  fclose(adt->_pdp);
}

void //
csvc_free_context(csv_adt *adt) {

  arena_free(adt->_context_arena);
  free(adt->_context_arena);

  fclose(adt->_pdp);

  free(adt);
}

size_t //
_csvc_get_idx_chr(char *string, char c) {

  char *str = strchr(string, c);

  if (str == NULL) {
    return strlen(string);
  }

  return (size_t)(str - string);
}

size_t //
_csvc_count_chr_on_str(char *str, char chr) {

  const char *p = str;
  size_t count = 0;

  do {
    if (*p == chr) {
      count++;
    }
  } while (*(p++));

  return count;
}

char ** //
_csvc_alloc_chr_array(csv_adt *adt, size_t arr_size) {

  char **ret_char_arr = NULL;

  // ret_char_arr = (char **)malloc(sizeof(char **) * arr_size);
  ret_char_arr = (char **)CSV_ALLOC(adt, sizeof(char **) * arr_size);

  for (size_t i = 0; i < arr_size; i++) {

    (ret_char_arr)[i] = (char *)CSV_ALLOC(adt, sizeof(char) * MAX_NAME_CELL);
    // (ret_char_arr)[i] = (char *)malloc(sizeof(char) * MAX_NAME_CELL);

    memset(&(ret_char_arr)[i][0], 0, sizeof(char) * MAX_NAME_CELL);
  }

  return ret_char_arr;
}

char ** //
_csvc_parser_line(csv_adt *adt, char *buff) {

  char **new_str = NULL;

  const char target = SEPARATION_CARACTER;

  size_t number_of_columns = _csvc_count_chr_on_str(buff, target);

  number_of_columns += 1; // for the default 0 count, and last column

  new_str = _csvc_alloc_chr_array(adt, number_of_columns);

  // int len = strlen(buff);
  size_t idx = 0;
  size_t current_idx = 0;
  size_t relative_count = 0;

  while ((current_idx = _csvc_get_idx_chr(&buff[idx], target)) &&
         (relative_count < number_of_columns)) {

    if (current_idx > MAX_NAME_CELL) {

      strncpy(new_str[relative_count], &buff[idx], MAX_NAME_CELL);
    } else {

      strncpy(new_str[relative_count], &buff[idx], current_idx);
    }

    relative_count += 1;
    idx = idx + current_idx + 1;
  }

  return new_str;
}

size_t //
_csvc_count_columns(char **buff) {

  size_t count = 0;
  while (buff[count] != NULL) {
    count += 1;
  }
  if (count == 0)
    return 0;

  return count - 1;
}

void * //
_csvc_parser_colum(char *buff, int column_num) {
  assert(buff);
  assert(column_num);
}

#endif // CSVPARSERC_IMPLEMENTATION
