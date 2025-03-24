# 214_Project_2

# Names of authors
1. Reghan Lao
2. Ali Rehman

# Design Choices 
1. HashTable used to map each word to its frequency count in each file.
2. FileData array keeps track of frequency of table and filenames for each scanned file.
3. Words are normalized by converting to lowercase and rejecting invalid boundary characters such as:
    a. does not start with (,[,{,",or '
    b. does not end with ),],},",',,,.,!, or ?
4. open() and read() are used for file access
5. Frequency Analysis
    a. After scanning all files the program calculates an overall frequency table
    b. for each file, it calculates the relative frequency of each over (file freq/overallfreq)
    c. the word with the maximum relative frequency increase is selected per file
    c. for ties, lexicograaphically first word is selected

# Testing Strategy 
1. 
