#ifndef _WRITER_H_
#define _WRITER_H_


#include <stdio.h>


// set the filename of the output file
void writer_set_filename(char* filename);

// open the file if the filename is set, otherwise, return 0
int writer_ready();

void close_writer();

void writer_set_file(FILE* file);
FILE* writer_get_file();

int get_indent_step();
int get_indent();
void add_indent(int step);
void incr_indent();
void sub_indent(int step);
void decr_indent();


void new_line();
void write_buffer(const char* buffer);
void write_bufferln(const char* buffer);

void smart_write_buffer(const char* buffer);
#endif
