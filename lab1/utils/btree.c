#include "btree.h"
#include <string.h>
#include <stdio.h>

// Создание нового узла
BTreeNode* create_node(int is_leaf) {
    BTreeNode* node = (BTreeNode*)malloc(sizeof(BTreeNode));
    if (!node) {
        perror("Failed to allocate memory for B-tree node");
        exit(EXIT_FAILURE);
    }
    node->is_leaf = is_leaf;
    node->num_keys = 0;
    for (int i = 0; i < MAX_KEYS + 1; i++) {
        node->children[i] = NULL;
    }
    return node;
}

// Инициализация B-дерева
BTree* initialize_btree() {
    BTree* tree = (BTree*)malloc(sizeof(BTree));
    if (!tree) {
        perror("Failed to allocate memory for B-tree");
        exit(EXIT_FAILURE);
    }
    tree->root = create_node(1); // Начинаем с пустого дерева с листом
    return tree;
}

// Поиск ключа в B-дереве
const char* search_key(BTree* tree, int key) {
    BTreeNode* current = tree->root;

    while (current) {
        int i = 0;
        while (i < current->num_keys && key > current->keys[i])
            i++;

        if (i < current->num_keys && key == current->keys[i])
            return (const char*)current->values[i];

        if (current->is_leaf)
            return NULL;

        current = current->children[i];
    }

    return NULL;
}

// Вспомогательная функция для разбиения узла
void split_child(BTreeNode* parent, int idx, BTreeNode* child) {
    BTreeNode* new_child = create_node(child->is_leaf);
    new_child->num_keys = MAX_KEYS / 2;

    for (int i = 0; i < MAX_KEYS / 2; i++)
        new_child->keys[i] = child->keys[i + MAX_KEYS / 2];

    if (!child->is_leaf) {
        for (int i = 0; i < MAX_KEYS / 2 + 1; i++)
            new_child->children[i] = child->children[i + MAX_KEYS / 2];
    }

    child->num_keys = MAX_KEYS / 2;
    for (int i = parent->num_keys; i >= idx + 1; i--)
        parent->children[i + 1] = parent->children[i];

    parent->children[idx + 1] = new_child;
    for (int i = parent->num_keys - 1; i >= idx; i--)
        parent->keys[i + 1] = parent->keys[i];

    parent->keys[idx] = child->keys[MAX_KEYS / 2 - 1];
    parent->num_keys++;
}

// Вставка ключа
void insert_non_full(BTreeNode* node, int key, const char* value) {
    int i = node->num_keys - 1;

    if (node->is_leaf) {
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            node->values[i + 1] = node->values[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->values[i + 1] = strdup(value);
        node->num_keys++;
    } else {
        while (i >= 0 && key < node->keys[i])
            i--;

        if (node->children[i + 1]->num_keys == MAX_KEYS) {
            split_child(node, i + 1, node->children[i + 1]);
            if (key > node->keys[i + 1])
                i++;
        }

        insert_non_full(node->children[i + 1], key, value);
    }
}

void insert_key(BTree* tree, int key, const char* value) {
    BTreeNode* root = tree->root;
    if (root->num_keys == MAX_KEYS) {
        BTreeNode* new_root = create_node(0);
        new_root->children[0] = root;
        split_child(new_root, 0, root);
        tree->root = new_root;
        insert_non_full(new_root, key, value);
    } else {
        insert_non_full(root, key, value);
    }
}


void delete_key(BTree* tree, int key) {
    if (!tree->root) return;

    BTreeNode* root = tree->root;

    delete_from_node(root, key);

    // Если корень стал пустым и не является листом
    if (root->num_keys == 0) {
        if (!root->is_leaf) {
            tree->root = root->children[0];
            free(root);
        }
    }
}

void delete_from_node(BTreeNode* node, int key) {
    int idx = 0;
    while (idx < node->num_keys && key > node->keys[idx])
        idx++;

    if (idx < node->num_keys && key == node->keys[idx]) {
        if (node->is_leaf) {
            remove_from_leaf(node, idx);
        } else {
            remove_from_non_leaf(node, idx);
        }
    } else {
        if (node->is_leaf) {
            printf("Key %d not found\n", key);
            return;
        }

        int is_last = (idx == node->num_keys);
        if (node->children[idx]->num_keys < MAX_KEYS / 2 + 1)
            fill_child(node, idx);

        if (is_last && idx > node->num_keys)
            delete_from_node(node->children[idx - 1], key);
        else
            delete_from_node(node->children[idx], key);
    }
}

void remove_from_leaf(BTreeNode* node, int idx) {
    for (int i = idx + 1; i < node->num_keys; ++i) {
        node->keys[i - 1] = node->keys[i];
        node->values[i - 1] = node->values[i];
    }
    node->num_keys--;
}

void remove_from_non_leaf(BTreeNode* node, int idx) {
    int key = node->keys[idx];

    if (node->children[idx]->num_keys >= MAX_KEYS / 2 + 1) {
        int pred = get_predecessor(node, idx);
        node->keys[idx] = pred;
        delete_from_node(node->children[idx], pred);
    } else if (node->children[idx + 1]->num_keys >= MAX_KEYS / 2 + 1) {
        int succ = get_successor(node, idx);
        node->keys[idx] = succ;
        delete_from_node(node->children[idx + 1], succ);
    } else {
        merge_children(node, idx);
        delete_from_node(node->children[idx], key);
    }
}

int get_predecessor(BTreeNode* node, int idx) {
    BTreeNode* current = node->children[idx];
    while (!current->is_leaf)
        current = current->children[current->num_keys];
    return current->keys[current->num_keys - 1];
}

int get_successor(BTreeNode* node, int idx) {
    BTreeNode* current = node->children[idx + 1];
    while (!current->is_leaf)
        current = current->children[0];
    return current->keys[0];
}

void fill_child(BTreeNode* node, int idx) {
    if (idx != 0 && node->children[idx - 1]->num_keys >= MAX_KEYS / 2 + 1) {
        borrow_from_prev(node, idx);
    } else if (idx != node->num_keys && node->children[idx + 1]->num_keys >= MAX_KEYS / 2 + 1) {
        borrow_from_next(node, idx);
    } else {
        if (idx != node->num_keys)
            merge_children(node, idx);
        else
            merge_children(node, idx - 1);
    }
}

void borrow_from_prev(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx - 1];

    for (int i = child->num_keys - 1; i >= 0; --i)
        child->keys[i + 1] = child->keys[i];
    if (!child->is_leaf) {
        for (int i = child->num_keys; i >= 0; --i)
            child->children[i + 1] = child->children[i];
    }

    child->keys[0] = node->keys[idx - 1];
    if (!child->is_leaf)
        child->children[0] = sibling->children[sibling->num_keys];

    node->keys[idx - 1] = sibling->keys[sibling->num_keys - 1];
    child->num_keys++;
    sibling->num_keys--;
}

void borrow_from_next(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx + 1];

    child->keys[child->num_keys] = node->keys[idx];
    if (!child->is_leaf)
        child->children[child->num_keys + 1] = sibling->children[0];

    node->keys[idx] = sibling->keys[0];
    for (int i = 1; i < sibling->num_keys; ++i)
        sibling->keys[i - 1] = sibling->keys[i];
    if (!sibling->is_leaf) {
        for (int i = 1; i <= sibling->num_keys; ++i)
            sibling->children[i - 1] = sibling->children[i];
    }

    child->num_keys++;
    sibling->num_keys--;
}

void merge_children(BTreeNode* node, int idx) {
    BTreeNode* child = node->children[idx];
    BTreeNode* sibling = node->children[idx + 1];

    child->keys[MAX_KEYS / 2] = node->keys[idx];
    for (int i = 0; i < sibling->num_keys; ++i)
        child->keys[i + MAX_KEYS / 2 + 1] = sibling->keys[i];
    if (!child->is_leaf) {
        for (int i = 0; i <= sibling->num_keys; ++i)
            child->children[i + MAX_KEYS / 2 + 1] = sibling->children[i];
    }

    for (int i = idx + 1; i < node->num_keys; ++i) {
        node->keys[i - 1] = node->keys[i];
        node->children[i] = node->children[i + 1];
    }

    child->num_keys += sibling->num_keys + 1;
    node->num_keys--;

    free(sibling);
}