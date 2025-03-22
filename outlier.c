#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
  if(argc < 2) {
    fprintf(stderr, "Usage: %s <files(s) and/or directory/directories>\n", argv[0]);
    return EXIT_FAILURE;
  }

  //loop through argument(s) and determine if file or directory 
  for(int i = 1; i < argc; i++) {
    struct stat buffer;

    if(stat(argv[i], &buffer) == -1) {
      perror("stat");
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