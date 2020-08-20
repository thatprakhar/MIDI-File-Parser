/* Name, library.c, CS 24000, Spring 2020
 * Last updated March 27, 2020
 */

/* Add any includes here */

#include "library.h"

#include <assert.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <malloc.h>


tree_node_t *g_song_library = NULL;

/*
 * This function inserts a smaller tree
 * into a larger tree
 */

void insert_back(tree_node_t **root, tree_node_t *node) {
  if (!node) {
    return;
  }
  tree_insert(root, node);
  insert_back(root, node->left_child);
  insert_back(root, node->right_child);
} /* insert_back() */

/*
 * Creates a new node
 */

tree_node_t *create_node(char *path) {
  tree_node_t *new_song = NULL;
  new_song = malloc(sizeof(tree_node_t));
  assert(new_song);
  new_song->left_child = NULL;
  new_song->right_child = NULL;
  new_song->song = parse_file(path);
  char *file_name = strchr(new_song->song->path, '/');
  while (file_name) {
    new_song->song_name = file_name + 1;
    file_name = file_name + 1;
    file_name = strchr(file_name, '/');
  }
  return new_song;
} /* create_node() */


/*
 * Returns a pointer to searched node
 */

tree_node_t **find_parent_pointer(tree_node_t **root_ptr, const char *name) {
  assert(root_ptr);
  assert(name);
  if (!strcmp((*root_ptr)->song_name, name)) {
    return root_ptr;
  }
  if ((*root_ptr)->left_child) {
    if (!strcmp((*root_ptr)->left_child->song_name, name)) {
      return &((*root_ptr)->left_child);
    }
  }
  if ((*root_ptr)->right_child) {
    if (!strcmp((*root_ptr)->right_child->song_name, name)) {
      return &((*root_ptr)->right_child);
    }
  }
  if (!((*root_ptr)->left_child) && !((*root_ptr)->right_child)) {
    return NULL;
  }

  if ((*root_ptr)->left_child) {
    tree_node_t **left_search = NULL;
    left_search = find_parent_pointer(&((*root_ptr)->left_child), name);
    if (left_search) {
      return left_search;
    }
  }

  if ((*root_ptr)->right_child) {
    tree_node_t **right_search = NULL;
    right_search = find_parent_pointer(&((*root_ptr)->right_child), name);
    if (right_search) {
      return right_search;
    }
  }

  return NULL;
} /* find_parent_pointer() */

/*
 * Inserts a node into the tree
 */

int tree_insert(tree_node_t **root, tree_node_t *node) {
  assert(node);

  if (*root == NULL) {
    *root = node;
    return INSERT_SUCCESS;
  }
  if (!strcmp((*root)->song_name, node->song_name)) {
    return DUPLICATE_SONG;
  }
  if (strcmp(node->song_name, (*root)->song_name) < 0) {
    if (!((*root)->left_child)) {
      (*root)->left_child = node;
      return INSERT_SUCCESS;
    }
    return tree_insert(&((*root)->left_child), node);
  }

  if (strcmp(node->song_name, (*root)->song_name) > 0) {
    if (!((*root)->right_child)) {
      (*root)->right_child = node;
      return INSERT_SUCCESS;
    }
    return tree_insert(&((*root)->right_child), node);
  }
  return DUPLICATE_SONG;
} /* tree_insert() */

/*
 * Removes a song from the tree
 */

int remove_song_from_tree(tree_node_t **root, const char *song_name) {
  assert(root);
  assert(*root);
  if (!strcmp((*root)->song_name, song_name)) {
    if ((*root)->left_child) {
      tree_node_t *new_head = (*root)->left_child;
      if ((*root)->right_child) {
        insert_back(&new_head, (*root)->right_child);
      }
      free_node(*root);
      *root = new_head;
      return DELETE_SUCCESS;
    }

    if ((*root)->right_child) {
      tree_node_t *new_head = (*root)->right_child;
      if ((*root)->left_child) {
        insert_back(&new_head, (*root)->left_child);
      }
      free_node(*root);
      *root = new_head;
      return DELETE_SUCCESS;
    }
    free_node(*root);
    return DELETE_SUCCESS;
  }

  if ((*root)->left_child) {
    if (!strcmp((*root)->left_child->song_name, song_name)) {
      tree_node_t *node_to_remove = (*root)->left_child;
      (*root)->left_child = NULL;
      tree_node_t *left_subtree = node_to_remove->left_child;
      tree_node_t *right_subtree = node_to_remove->right_child;
      free_node(node_to_remove);
      insert_back(root, left_subtree);
      insert_back(root, right_subtree);
      return DELETE_SUCCESS;
    }
  }

  if ((*root)->right_child) {
    if (!strcmp((*root)->right_child->song_name, song_name)) {
      tree_node_t *node_to_remove = (*root)->right_child;
      (*root)->right_child = NULL;
      tree_node_t *left_subtree = node_to_remove->left_child;
      tree_node_t *right_subtree = node_to_remove->right_child;
      free_node(node_to_remove);
      insert_back(root, left_subtree);
      insert_back(root, right_subtree);
      return DELETE_SUCCESS;
    }
  }

  if ((*root)->left_child) {
    int left_delete = -1;
    left_delete = remove_song_from_tree(&((*root)->left_child), song_name);
    if (left_delete == DELETE_SUCCESS) {
      return DELETE_SUCCESS;
    }
  }

  if ((*root)->right_child) {
    int right_delete = -1;
    right_delete = remove_song_from_tree(&((*root)->right_child), song_name);
    if (right_delete == DELETE_SUCCESS) {
      return DELETE_SUCCESS;
    }
  }

  return SONG_NOT_FOUND;
} /* remove_song_from_tree() */

/*
 * Frees a node
 */

void free_node(tree_node_t *node) {
  free_song(node->song);
  node->song = NULL;
  node->left_child = NULL;
  node->right_child = NULL;
  free(node);
  node = NULL;
} /* free_node() */

/*
 * Prints a node into a file
 */

void print_node(tree_node_t *node, FILE *file_ptr) {
  fprintf(file_ptr, "%s\n", node->song_name);
} /* print_node() */

/*
 * Applies the function to the tree
 * in pre-order fashion
 */

void traverse_pre_order(tree_node_t *root, void *data, traversal_func_t func) {
  if (!root) {
    return;
  }
  func(root, data);
  traverse_pre_order(root->left_child, data, func);
  traverse_pre_order(root->right_child, data, func);
} /* traverse_pre_order() */

/*
 * Applies the function to the tree
 * in in-order fashion
 */

void traverse_in_order(tree_node_t *root, void *data, traversal_func_t func) {
  if (!root) {
    return;
  }
  traverse_in_order(root->left_child, data, func);
  func(root, data);
  traverse_in_order(root->right_child, data, func);
} /* traverse_in_order() */

/*
 * Applies the function to the tree
 * in post-order fashion
 */

void traverse_post_order(tree_node_t *root, void *data, traversal_func_t func) {
  if (!root) {
    return;
  }
  traverse_post_order(root->left_child, data, func);
  traverse_post_order(root->right_child, data, func);
  func(root, data);
} /* traverse_post_order() */

/*
 * Frees the entire library
 */

void free_library(tree_node_t *root) {
  if (root->left_child) {
    free_library(root->left_child);
  }
  if (root->right_child) {
    free_library(root->right_child);
  }
  free_node(root);
} /* free_library() */

/*
 * Writes the song list to the file
 */

void write_song_list(FILE *file_ptr, tree_node_t *root) {
  traversal_func_t print_to_file = (traversal_func_t) print_node;
  traverse_in_order(root, file_ptr, print_to_file);
} /* write_song_list() */

/*
 * Makes a library of specified library
 */

void make_library(const char *directory_path) {
  DIR *directory = NULL;
  struct dirent *dir = NULL;
  directory = opendir(directory_path);
  if (directory) {
    while ((dir = readdir(directory))) {
      if (strstr(dir->d_name, ".mid")) {
        char full_path[1000] = "";
        strcat(full_path, directory_path);
        strcat(full_path, "/");
        strcat(full_path, dir->d_name);
        full_path[strlen(directory_path) + strlen(dir->d_name) + 1] = '\0';
        tree_node_t *new_song = create_node(full_path);
        assert(tree_insert(&g_song_library, new_song) != DUPLICATE_SONG);
      }
    }
    closedir(directory);
    printf("Directory closed\n");
  }
} /* make_library() */
