#include "hextype_rbtree.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define NDEBUG
typedef rbtree_node node;
typedef enum rbtree_node_color color;

static node grandparent(node n);
static node sibling(node n);
static node uncle(node n);
static color node_color(node n);

static node new_node(void* key, void* value, color node_color,
                     node left, node right);
static node lookup_node(rbtree t, void* key);
static void rotate_left(rbtree t, node n);
static void rotate_right(rbtree t, node n);

static void replace_node(rbtree t, node oldn, node newn);
static void insert_case1(rbtree t, node n);
static void insert_case2(rbtree t, node n);
static void insert_case3(rbtree t, node n);
static void insert_case4(rbtree t, node n);
static void insert_case5(rbtree t, node n);
static node maximum_node(node root);
static void delete_case1(rbtree t, node n);
static void delete_case2(rbtree t, node n);
static void delete_case3(rbtree t, node n);
static void delete_case4(rbtree t, node n);
static void delete_case5(rbtree t, node n);
static void delete_case6(rbtree t, node n);

void write_log(char *result, char *filename) {
  if (getenv("HEXTYPE_LOG_PATH") != nullptr) {
    char *home = getenv("HEXTYPE_LOG_PATH");
    char path[MAXPATH];
    strcpy(path, home);
    strcat(path, filename);

    FILE *op = fopen(path, "a");
    if(op != NULL) {
      fprintf(op, "%s\n", result);
      fflush(op);
      fclose(op);
    }
  }
}

node grandparent(node n) {
  assert (n != NULL);
  assert (n->parent != NULL); /* Not the root node */
  assert (n->parent->parent != NULL); /* Not child of root */
  return n->parent->parent;
}

node sibling(node n) {
  assert (n != NULL);
  assert (n->parent != NULL); /* Root node has no sibling */
  if (n == n->parent->left)
    return n->parent->right;
  else
    return n->parent->left;
}

node uncle(node n) {
  assert (n != NULL);
  assert (n->parent != NULL); /* Root node has no uncle */
  assert (n->parent->parent != NULL); /* Children of root have no uncle */
  return sibling(n->parent);
}

color node_color(node n) {
  return n == NULL ? BLACK : n->color;
}

rbtree rbtree_create() {
  rbtree t = (rbtree)malloc(sizeof(struct rbtree_t));
  t->root = NULL;
  return t;
}

node new_node(void* key, void* value, color node_color, node left, node right) {
  node result = (node)malloc(sizeof(struct rbtree_node_t));

  if(result == NULL) {
    return NULL;
  }

  else {
    result->key = key;
    result->value = value;
    result->color = node_color;
    result->left = left;
    result->right = right;
    if (left  != NULL)  left->parent = result;
    if (right != NULL) right->parent = result;
    result->parent = NULL;
    return result;
  }
}

static int compare(void* left, void* right);

int compare(void* leftp, void* rightp) {

  int left = (int)(size_t)leftp;
  int right = (int)(size_t)rightp;
  if (left < right)
    return -1;
  else if (left > right)
    return 1;
  else {
    assert (left == right);
    return 0;
  }
}

node lookup_node(rbtree t, void* key) {

  node n = t->root;
  while (n != NULL) {
    int comp_result = compare(key, n->key);
    if (comp_result == 0) {
      return n;
    } else if (comp_result < 0) {
      n = n->left;
    } else {
      assert(comp_result > 0);
      n = n->right;
    }
  }
  return n;
}

void* rbtree_lookup(rbtree t, void* key) {

  node n = lookup_node(t, key);

  return n == NULL ? NULL : n->value;
}

void rotate_left(rbtree t, node n) {
  node r = n->right;
  replace_node(t, n, r);
  n->right = r->left;
  if (r->left != NULL) {
    r->left->parent = n;
  }
  r->left = n;
  n->parent = r;
}

void rotate_right(rbtree t, node n) {
  node L = n->left;
  replace_node(t, n, L);
  n->left = L->right;
  if (L->right != NULL) {
    L->right->parent = n;
  }
  L->right = n;
  n->parent = L;
}

void replace_node(rbtree t, node oldn, node newn) {
  if (oldn->parent == NULL) {
    t->root = newn;
  } else {
    if (oldn == oldn->parent->left)
      oldn->parent->left = newn;
    else
      oldn->parent->right = newn;
  }
  if (newn != NULL) {
    newn->parent = oldn->parent;
  }
}

void rbtree_insert(rbtree t, void* key, void* value) {

  node inserted_node = new_node(key, value, RED, NULL, NULL);

  if(inserted_node == NULL) {
    return;
  }

  if (t->root == NULL) {
    t->root = inserted_node;
  } else {
    node n = t->root;
    while (1) {
      int comp_result = compare(key, n->key);
      if (comp_result == 0) {
        n->value = value;
        return;
      } else if (comp_result < 0) {
        if (n->left == NULL) {
          n->left = inserted_node;
          break;
        } else {
          n = n->left;
        }
      } else {
        assert (comp_result > 0);
        if (n->right == NULL) {
          n->right = inserted_node;
          break;
        } else {
          n = n->right;
        }
      }
    }
    inserted_node->parent = n;
  }
  insert_case1(t, inserted_node);
}

void insert_case1(rbtree t, node n) {
  if (n->parent == NULL)
    n->color = BLACK;
  else
    insert_case2(t, n);
}

void insert_case2(rbtree t, node n) {
  if (node_color(n->parent) == BLACK)
    return; /* Tree is still valid */
  else
    insert_case3(t, n);
}

void insert_case3(rbtree t, node n) {
  if (node_color(uncle(n)) == RED) {
    n->parent->color = BLACK;
    uncle(n)->color = BLACK;
    grandparent(n)->color = RED;
    insert_case1(t, grandparent(n));
  } else {
    insert_case4(t, n);
  }
}

void insert_case4(rbtree t, node n) {
  if (n == n->parent->right && n->parent == grandparent(n)->left) {
    rotate_left(t, n->parent);
    n = n->left;
  } else if (n == n->parent->left && n->parent == grandparent(n)->right) {
    rotate_right(t, n->parent);
    n = n->right;
  }
  insert_case5(t, n);
}

void insert_case5(rbtree t, node n) {
  n->parent->color = BLACK;
  grandparent(n)->color = RED;
  if (n == n->parent->left && n->parent == grandparent(n)->left) {
    rotate_right(t, grandparent(n));
  } else {
    assert (n == n->parent->right && n->parent == grandparent(n)->right);
    rotate_left(t, grandparent(n));
  }
}

int rbtree_delete(rbtree t, void* key) {
  node child;

  node n = lookup_node(t, key);
  if (n == NULL) return 0;  /* Key not found, do nothing */
  if (n->left != NULL && n->right != NULL) {
    /* Copy key/value from predecessor and then delete it instead */
    node pred = maximum_node(n->left);
    n->key   = pred->key;
    n->value = pred->value;
    n = pred;
  }

  assert(n->left == NULL || n->right == NULL);
  child = n->right == NULL ? n->left  : n->right;
  if (node_color(n) == BLACK) {
    n->color = node_color(child);
    delete_case1(t, n);
  }
  replace_node(t, n, child);
  if (n->parent == NULL && child != NULL) // root should be black
    child->color = BLACK;

  free(n);

  return 1;
}

static node maximum_node(node n) {
  assert (n != NULL);
  while (n->right != NULL) {
    n = n->right;
  }
  return n;
}

void delete_case1(rbtree t, node n) {
  if (n->parent == NULL)
    return;
  else
    delete_case2(t, n);
}

void delete_case2(rbtree t, node n) {
  if (node_color(sibling(n)) == RED) {
    n->parent->color = RED;
    sibling(n)->color = BLACK;
    if (n == n->parent->left)
      rotate_left(t, n->parent);
    else
      rotate_right(t, n->parent);
  }
  delete_case3(t, n);
}

void delete_case3(rbtree t, node n) {
  if (node_color(n->parent) == BLACK &&
      node_color(sibling(n)) == BLACK &&
      node_color(sibling(n)->left) == BLACK &&
      node_color(sibling(n)->right) == BLACK)
  {
    sibling(n)->color = RED;
    delete_case1(t, n->parent);
  }
  else
    delete_case4(t, n);
}

void delete_case4(rbtree t, node n) {
  if (node_color(n->parent) == RED &&
      node_color(sibling(n)) == BLACK &&
      node_color(sibling(n)->left) == BLACK &&
      node_color(sibling(n)->right) == BLACK)
  {
    sibling(n)->color = RED;
    n->parent->color = BLACK;
  }
  else
    delete_case5(t, n);
}

void delete_case5(rbtree t, node n) {
  if (n == n->parent->left &&
      node_color(sibling(n)) == BLACK &&
      node_color(sibling(n)->left) == RED &&
      node_color(sibling(n)->right) == BLACK)
  {
    sibling(n)->color = RED;
    sibling(n)->left->color = BLACK;
    rotate_right(t, sibling(n));
  }
  else if (n == n->parent->right &&
           node_color(sibling(n)) == BLACK &&
           node_color(sibling(n)->right) == RED &&
           node_color(sibling(n)->left) == BLACK)
  {
    sibling(n)->color = RED;
    sibling(n)->right->color = BLACK;
    rotate_left(t, sibling(n));
  }
  delete_case6(t, n);
}

void delete_case6(rbtree t, node n) {
  sibling(n)->color = node_color(n->parent);
  n->parent->color = BLACK;
  if (n == n->parent->left) {
    assert (node_color(sibling(n)->right) == RED);
    sibling(n)->right->color = BLACK;
    rotate_left(t, n->parent);
  }
  else
  {
    assert (node_color(sibling(n)->left) == RED);
    sibling(n)->left->color = BLACK;
    rotate_right(t, n->parent);
  }
}
