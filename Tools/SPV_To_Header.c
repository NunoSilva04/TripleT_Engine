#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_STRING 256
#define HEADER_FILE_PATH "Src/Graphics/Internals/"

typedef struct SPV_To_Header_Info_t{
    unsigned int *data;
    char data_var_name[MAX_STRING];
    unsigned int data_size;
    char data_size_var_name[MAX_STRING];
}SPV_To_Header_Info;

void create_header_file_path(char *header_file_path, const char *spv_file_path, SPV_To_Header_Info *spv_to_header_info){
    strcpy(header_file_path, HEADER_FILE_PATH);

    char spv_file_name[MAX_STRING] = {0};
    unsigned int i = 0, j = 0;
    while(spv_file_path[i] != '/'){
	i++;
    }

    i++;
    while(spv_file_path[i] != '.'){
	spv_file_name[j] = spv_file_path[i];
	i++;
	j++;
    }
    strcat(header_file_path, spv_file_name);
    strcat(header_file_path, ".h");

    strcpy(spv_to_header_info->data_var_name, spv_file_name);
    strcat(spv_to_header_info->data_var_name, "_data");
    strcpy(spv_to_header_info->data_size_var_name, spv_file_name);
    strcat(spv_to_header_info->data_size_var_name, "_size");

    return;
}

void read_spv_file(const char *file_path, SPV_To_Header_Info *spv_to_header_info){
    FILE *file = fopen(file_path, "rb");

    fseek(file, 0, SEEK_END);
    spv_to_header_info->data_size = ftell(file);
    unsigned int num_words = spv_to_header_info->data_size / 4;
    spv_to_header_info->data = (unsigned int *)calloc(num_words, sizeof(unsigned int));
    
    fseek(file, 0, SEEK_SET);
    unsigned int i = 0;
    while(fread(&spv_to_header_info->data[i], sizeof(unsigned int), 1, file) == 1)
	i++;

    fclose(file);
    return;
}

void create_header_file(const char *header_file_path, const SPV_To_Header_Info spv_to_header_info){
    FILE *file = fopen(header_file_path, "w");

    fprintf(file, "#pragma once\n\n");
    fprintf(file, "unsigned int %s[] = {\n", spv_to_header_info.data_var_name);
    unsigned int count = 0, num_words = spv_to_header_info.data_size / 4;
    while(count != num_words){
	fprintf(file, "0x%08x, ", spv_to_header_info.data[count]);
	count++;
	if((count % 16) == 0)
	    fprintf(file, "\n");
    }
    fprintf(file, "\n};\nunsigned int %s = %d;\n", spv_to_header_info.data_size_var_name, spv_to_header_info.data_size);

    fclose(file);
    return;
}

// spv_to_header spv_file_path
int main(int argc, char **argv){
    if(argc != 2){
	fprintf(stdout, "Not enough arguments" );
	return 0;
    }

    SPV_To_Header_Info spv_to_header_info = {0};
    char header_file_path[MAX_STRING] = {0};
    create_header_file_path(header_file_path, argv[1], &spv_to_header_info);

    read_spv_file(argv[1], &spv_to_header_info);
    create_header_file(header_file_path, spv_to_header_info);

    free(spv_to_header_info.data);
    return 0;
}
