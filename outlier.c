#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>

int ends_with_txt(char *file_path) {
  //extracts the last . and extension if it exists
  char *extension = strrchr(file_path, '.');

  //ensuring the file ends with .txt
  if(extension != NULL && strcmp(extension, ".txt") == 0) {
    return 1;
  }

  //file does not end with .txt
  return 0;
}

//process file and update word frequencies in the HashTable
void process_file(char *file_path, HashTable *word_frequency) {
  //TO BE DONE
  int fd = open(file_path, O_RDONLY);

  if (fd == -1) {
    perror("Error opening file");
    return 1;
  }


}

void process_directory(char *directory_name) {
  DIR *dir = opendir(directory_name);
  if(dir == NULL) {
    perror("Unable to open directory");
    return;
  }

  //refers to an entry in the directory
  struct dirent *dir_entry;

  //while we have more entries to read
  while((dir_entry = readdir(dir)) != NULL) {
    // names beginning with a period (.) are ignored
    if(dir_entry->d_name[0] == '.') {
      continue;
    }

    //constructing pathnames for each file and/or directory encountered 
    char file_path[1024];
    snprintf(file_path, sizeof(file_path), "%s/%s", directory_name, dir_entry->d_name);

    struct stat buffer;
    if(stat(file_path, &buffer) == -1) {
      perror("stat failed");
      continue;
    }

    if(S_ISDIR(buffer.st_mode)) {
      //directories are recursively traversed
      process_directory(file_path);
    }
    else if(S_ISREG(buffer.st_mode)) {
      //process regular files that ONLY end in .txt
      if(ends_with_txt(file_path)) {
        process_file(file_path);
      }
    }
  }
  
  closedir(dir);
}

int main(int argc, char *argv[]) {
  if(argc < 2) {
    fprintf(stderr, "Usage: %s <files(s) and/or directory/directories>\n", argv[0]);
    return EXIT_FAILURE;
  }

  //loop through argument(s) and determine if file or directory 
  for(int i = 1; i < argc; i++) {
    struct stat buffer;

    if(stat(argv[i], &buffer) == -1) {
      perror("stat failed");
      return 1;
    }

    //if file is directory, we recursively traverse it 
    if(S_ISDIR(buffer.st_mode)) {
      process_directory(argv[i]);
    }
    //if file is regular, we scan 
    else if(S_ISREG(buffer.st_mode)) {
      process_file(argv[i]);
    }
    //not a regular file nor a directory 
    else {
      fprintf(stderr, "%s is neither a directory nor a file\n", argv[i]);
    }
  }

  return EXIT_SUCCESS;
}