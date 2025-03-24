/*
AUTHORS: Ali Rehman & Reghan Lao
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>


#define TABLE_SIZE 6000  // size for our HashTable TBD
#define MAX_FILES 1000

//represents an entry in the HashTable
typedef struct WordEntry {
  char *word; //key
  int count; //value
  struct WordEntry *next;
} WordEntry;

/*
an array of pointers to WordEntry objects and each index in the array can have a 
a linked list of WordEntry objects to handle collisions 
*/
typedef struct {
  WordEntry *table[TABLE_SIZE];
  int total_words;
} HashTable;

typedef struct {
  char *filename;
  HashTable *freq_table;
} FileData;

FileData files[MAX_FILES];
int file_count = 0;

//implement simple hash function
unsigned int hash(char *word)
{
  unsigned int hash = 0;
  while(*word) {
    hash = (hash * 37) + tolower(*word++);
  }

  return hash % TABLE_SIZE;
}

HashTable *create_table()
{
  HashTable *table = malloc(sizeof(HashTable));
  if(!table)
  {
    perror("malloc");
    exit(1);
  }
  memset(table, 0, sizeof(HashTable));
  return table;
}

//insert word into HashTable
void insert_word(char *word, HashTable *hash_table)
{
  unsigned int index = hash(word);

  WordEntry *entry = hash_table->table[index];

  //checks if word already exists in our hashtable
  while(entry) {
    if(strcasecmp(entry->word, word) == 0)
    {
      entry->count += 1;
      hash_table->total_words += 1;
      return; 
    }

    entry = entry->next;
  }

  entry = malloc(sizeof(WordEntry));
  entry->word = strdup(word);
  entry->count = 1;
  entry->next = hash_table->table[index];
  hash_table->table[index] = entry;
  hash_table->total_words += 1;
}

int ends_with_txt(char *file_path)
{
  //extracts the last . and extension if it exists
  char *extension = strrchr(file_path, '.');
  //ensuring the file ends with .txt
  if(extension != NULL && strcmp(extension, ".txt") == 0) {
    return 1;
  }

  return 0;
}

void normalizeWord(char *word)
{
    int start = 0;
    int end = strlen(word) - 1;

    // No trimming of spec characters here, just whitespace-like cleanup
    while (start <= end && isspace(word[start])) start++;
    while (end >= start && isspace(word[end])) end--;

    if (start > end) {
        word[0] = '\0';
        return;
    }

    // Shift characters to front and lowercase
    int i;
    for (i = start; i <= end; i++) {
        word[i - start] = tolower(word[i]);
    }
    word[i - start] = '\0';

    // Apply required exclusion rules

    //leading punctuation removal 
    while (word[0] == '(' || word[0] == '[' || word[0] == '{' || word[0] == '"' || word[0] == '\'') 
    {
      memmove(word, word + 1, strlen(word));
    }

    //trailing punctuation removal 
    int len = strlen(word);
    while (len > 0 && (word[len - 1] == ')' || word[len - 1] == ']' ||
                    word[len - 1] == '}' || word[len - 1] == '"' ||
                    word[len - 1] == '\'' || word[len - 1] == ',' ||
                    word[len - 1] == '.' || word[len - 1] == '!' ||
                    word[len - 1] == '?')) {
        word[len - 1] = '\0';
        len--;
    }
}

int isValidWord(const char *word)
{
  for(int i = 0; word[i]; ++i)
  {
    if(isalpha(word[i]))
      return 1;
  }
  return 0;
}

//process file and update word frequencies in the HashTable
void process_file(const char *file_path)
{
    int fd = open(file_path, O_RDONLY);
    if (fd < 0)
    {
        perror("Unable to open file");
        return;
    }

    HashTable *table = create_table();
    char buf[4096];
    char word[256];
    ssize_t bytes;
    int wlength = 0;

    while ((bytes = read(fd, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < bytes; ++i) {
            char c = buf[i];

            if (!isspace(c)) {
                if (wlength < sizeof(word) - 1) {
                    word[wlength++] = c;
                }
            } else {
                if (wlength > 0) {
                    word[wlength] = '\0';
                    normalizeWord(word);
                    if (isValidWord(word)) {
                        insert_word(word, table);
                    }
                    wlength = 0;
                }
            }
        }
    }
    // Flush last word if file doesn't end in whitespace
    if (wlength > 0) {
        word[wlength] = '\0';
        normalizeWord(word);
        if (isValidWord(word)) {
            insert_word(word, table);
        }
    }

    close(fd);
    files[file_count].filename = strdup(file_path);
    files[file_count].freq_table = table;
    file_count++;
}

void process_directory(const char *directory_name)
{
  DIR *dir = opendir(directory_name);
  if(dir == NULL) {
    perror("Unable to open directory");
    return;
  }

  //refers to an entry in the directory
  struct dirent *dir_entry;

  //while we have more entries to read
  while((dir_entry = readdir(dir))) {
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
    else if(S_ISREG(buffer.st_mode) && ends_with_txt(dir_entry->d_name))
    {
      process_file(file_path);
    }
  }
  closedir(dir);
}

void findOverallFreq(HashTable *overall)
{
    for (int i = 0; i < file_count; ++i) {
        HashTable *fileTable = files[i].freq_table;

        for (int j = 0; j < TABLE_SIZE; ++j) {
            for (WordEntry *entry = fileTable->table[j]; entry; entry = entry->next) {
                unsigned int idx = hash(entry->word);
                WordEntry *globalEntry = overall->table[idx];

                while (globalEntry && strcasecmp(globalEntry->word, entry->word) != 0) {
                    globalEntry = globalEntry->next;
                }

                if (globalEntry) {
                    globalEntry->count += entry->count;
                } else {
                    globalEntry = malloc(sizeof(WordEntry));
                    globalEntry->word = strdup(entry->word);
                    globalEntry->count = entry->count;
                    globalEntry->next = overall->table[idx];
                    overall->table[idx] = globalEntry;
                }
                // Very important: increase global word total only ONCE per count
                overall->total_words += entry->count;
            }
        }
    }
}

void findOutliers(HashTable *overall)
{
    for (int i = 0; i < file_count; ++i)
    {
        HashTable *fileTable = files[i].freq_table;
        char *bestWord = NULL;
        double maxRatio = 0;

        for (int j = 0; j < TABLE_SIZE; ++j)
        {
            for (WordEntry *entry = fileTable->table[j]; entry; entry = entry->next)
            {
                unsigned int idx = hash(entry->word);
                WordEntry *o = overall->table[idx];

                while (o && strcasecmp(o->word, entry->word) != 0)
                    o = o->next;

                if (!o || overall->total_words == 0) continue;

                double f_freq = (double)entry->count / fileTable->total_words;
                double o_freq = (double)o->count / overall->total_words;

                if (o_freq == 0) continue;

                double ratio = f_freq / o_freq;

                if (ratio > maxRatio || (ratio == maxRatio && (bestWord == NULL || strcmp(entry->word, bestWord) < 0))) {
                    maxRatio = ratio;
                    bestWord = entry->word;
                }
            }
        }
        if (bestWord)
        {
            printf("%s: %s\n", files[i].filename, bestWord);
        }
    }
}

int main(int argc, char *argv[]) {
  if(argc < 2) {
    fprintf(stderr, "Usage: %s <files(s) and/or directory/directories>\n", argv[0]);
    return EXIT_FAILURE;
  }

  for(int i = 1; i < argc; i++)
  {
    struct stat st;
    if(stat(argv[i], &st) == -1)
    {continue;}
    if(S_ISDIR(st.st_mode))
    {
      process_directory(argv[i]);
    }
    else if (S_ISREG(st.st_mode))
    {
      process_file(argv[i]);
    }
  }

  HashTable *overall = create_table();
  findOverallFreq(overall);
  findOutliers(overall);

  return EXIT_SUCCESS;
}