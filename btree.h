#ifndef BTREE_H
#define BTREE_H

#include <stdlib.h>

#define MAX_KEYS 4

// Узел B-дерева
typedef struct BTreeNode {
    int keys[MAX_KEYS];                // Ключи
    void* values[MAX_KEYS];            // Значения
    struct BTreeNode* children[MAX_KEYS + 1]; // Дочерние узлы
    int num_keys;                      // Число ключей в узле
    int is_leaf;                       // Флаг: листовой узел или нет
} BTreeNode;

// Дерево
typedef struct {
    BTreeNode* root; // Корневой узел
} BTree;

// Создание нового узла B-дерева
BTreeNode* create_node(int is_leaf);

// Инициализация B-дерева
BTree* initialize_btree();

// Добавление ключ-значение в B-дерево
void insert_key(BTree* tree, int key, const char* value);

// Поиск значения по ключу
const char* search_key(BTree* tree, int key);

// Удаление узла по ключу
void delete_key(BTree* tree, int key);

// Вспомогательные функции для удаления
void delete_from_node(BTreeNode* node, int key);
void remove_from_leaf(BTreeNode* node, int idx);
void remove_from_non_leaf(BTreeNode* node, int idx);
int get_predecessor(BTreeNode* node, int idx);
int get_successor(BTreeNode* node, int idx);
void fill_child(BTreeNode* node, int idx);
void borrow_from_prev(BTreeNode* node, int idx);
void borrow_from_next(BTreeNode* node, int idx);
void merge_children(BTreeNode* node, int idx);


#endif // BTREE_H
