#ifndef TRAVERSAL_H___
#define TRAVERSAL_H___

int directory_traversal_fts(char* directory);

int directory_traversal_dir(char* directory);

int directory_traversal_fts_singleprocess(char* directory);

int directory_traversal_dir_singleprocess(char* directory);


int init_traversal_fts();

int init_traversal_dir();

int init_traversal_fts_singleprocess();

int init_traversal_dir_singleprocess();


int init_traversal_here_fts();

int init_traversal_here_dir();

int init_traversal_here_fts_singleprocess();

int init_traversal_here_dir_singleprocess();

#endif // TRAVERSAL_H___
