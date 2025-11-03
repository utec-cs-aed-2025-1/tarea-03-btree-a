/* btree.h */
#ifndef BTree_H
#define BTree_H
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <stack>
#include "node.h"

using namespace std;

template<typename TK>
class BTree {
private:
    Node<TK> *root;
    int M; // grado u orden del arbol (definido por el usuario como MAX CLAVES)
    int n; // total de elementos en el arbol

public:
    BTree(int orden) : root(nullptr), M(orden), n(0) {
    }

    bool search(TK key) {
        Node<TK> *current = this->root;
        if (current == nullptr) {
            return false;
        }
        while (current != nullptr) {
            int i = 0;
            while (i < current->count && current->keys[i] < key) {
                i++;
            }
            if (i < current->count && current->keys[i] == key) {
                return true;
            }

            if (current->leaf) {
                return false;
            }
            current = current->children[i];
        }
        return false;
    };

    Node<TK> *splitNode(Node<TK> *curr, TK &middleKey, int M) {
        int mid = (curr->count - 1) / 2;
        middleKey = curr->keys[mid];

        Node<TK> *newNode = new Node<TK>(M);
        newNode->leaf = curr->leaf;


        for (int i = mid + 1, j = 0; i < curr->count; ++i, ++j) {
            newNode->keys[j] = curr->keys[i];
            newNode->count++;
        }

        if (!curr->leaf) {
            for (int i = mid + 1, j = 0; i <= curr->count; ++i, ++j)
                newNode->children[j] = curr->children[i];
        }

        for (int i = mid; i < M; ++i) curr->keys[i] = TK();
        curr->count = mid;

        return newNode;
    }


    bool insertIntoParent(stack<Node<TK> *> &s, Node<TK> *curr, Node<TK> *newNode, TK middleKey, int M) {
        if (s.empty()) {
            Node<TK> *newRoot = new Node<TK>(M);
            newRoot->keys[0] = middleKey;
            newRoot->children[0] = curr;
            newRoot->children[1] = newNode;
            newRoot->count = 1;
            newRoot->leaf = false;
            root = newRoot;
            return true;
        }

        Node<TK> *parent = s.top();

        int i;
        for (i = parent->count - 1; i >= 0 && parent->keys[i] > middleKey; --i) {
            parent->keys[i + 1] = parent->keys[i];
            parent->children[i + 2] = parent->children[i + 1];
        }

        parent->keys[i + 1] = middleKey;
        parent->children[i + 2] = newNode;
        parent->count++;

        if (parent->count == M) {
            s.pop();
            TK newMiddle;
            Node<TK> *next = splitNode(parent, newMiddle, M);
            return insertIntoParent(s, parent, next, newMiddle, M);
        }

        return true;
    }

    void insert(TK key) {
        if (search(key)) return;

        if (root == nullptr) {
            root = new Node<TK>(M);
            root->keys[0] = key;
            root->count = 1;
            n++;
            return;
        }

        stack<Node<TK> *> s;
        Node<TK> *curr = root;
        s.push(curr);


        while (!curr->leaf) {
            int i;
            for (i = 0; i < curr->count; ++i)
                if (curr->keys[i] > key) break;

            curr = curr->children[i];
            s.push(curr);
        }


        bool inserted = false;
        while (!inserted) {
            // Se inserta la clave en el nodo (potencialmente desbordándolo)
            curr->insert(key);

            if (curr->count < M) { // OJO: Tu lógica usa < M, no <= M.
                inserted = true;
            } else {
                TK middleKey;
                Node<TK> *newNode = splitNode(curr, middleKey, M);
                s.pop();
                inserted = insertIntoParent(s, curr, newNode, middleKey, M);
            }
        }
        n++;
    }


    TK getPredecessor(Node<TK> *node, int idx) {
        Node<TK> *current = node->children[idx];
        while (!current->leaf) {
            current = current->children[current->count];
        }
        return current->keys[current->count - 1];
    }

    TK getSuccessor(Node<TK> *node, int idx) {
        Node<TK> *current = node->children[idx + 1];
        while (!current->leaf) {
            current = current->children[0];
        }
        return current->keys[0];
    }

    void merge(Node<TK> *parent, int idx) {
        Node<TK> *child = parent->children[idx];
        Node<TK> *sibling = parent->children[idx + 1];

        child->keys[child->count] = parent->keys[idx];

        for (int i = 0; i < sibling->count; i++) {
            child->keys[child->count + 1 + i] = sibling->keys[i];
        }

        if (!child->leaf) {
            for (int i = 0; i <= sibling->count; i++) {
                child->children[child->count + 1 + i] = sibling->children[i];
            }
        }

        child->count += sibling->count + 1;

        for (int i = idx; i < parent->count - 1; i++) {
            parent->keys[i] = parent->keys[i + 1];
        }

        for (int i = idx + 1; i < parent->count; i++) {
            parent->children[i] = parent->children[i + 1];
        }

        parent->count--;
        delete sibling;
    }

    void borrowFromLeft(Node<TK> *parent, int childIdx) {
        Node<TK> *child = parent->children[childIdx];
        Node<TK> *leftSibling = parent->children[childIdx - 1];

        for (int i = child->count - 1; i >= 0; i--) {
            child->keys[i + 1] = child->keys[i];
        }

        child->keys[0] = parent->keys[childIdx - 1];

        if (!child->leaf) {
            for (int i = child->count; i >= 0; i--) {
                child->children[i + 1] = child->children[i];
            }
            child->children[0] = leftSibling->children[leftSibling->count];
        }

        parent->keys[childIdx - 1] = leftSibling->keys[leftSibling->count - 1];

        child->count++;
        leftSibling->count--;
    }

    void borrowFromRight(Node<TK> *parent, int childIdx) {
        Node<TK> *child = parent->children[childIdx];
        Node<TK> *rightSibling = parent->children[childIdx + 1];

        child->keys[child->count] = parent->keys[childIdx];

        if (!child->leaf) {
            child->children[child->count + 1] = rightSibling->children[0];
        }

        parent->keys[childIdx] = rightSibling->keys[0];

        for (int i = 0; i < rightSibling->count - 1; i++) {
            rightSibling->keys[i] = rightSibling->keys[i + 1];
        }

        if (!rightSibling->leaf) {
            for (int i = 0; i < rightSibling->count; i++) {
                rightSibling->children[i] = rightSibling->children[i + 1];
            }
        }

        child->count++;
        rightSibling->count--;
    }

    void removeFromNode(Node<TK> *node, TK key) {
        int i = 0;

        while (i < node->count && node->keys[i] < key) {
            i++;
        }

        if (i < node->count && node->keys[i] == key) {
            if (node->leaf) {
                for (int j = i; j < node->count - 1; j++) {
                    node->keys[j] = node->keys[j + 1];
                }
                node->count--;
            } else {
                TK keyToDelete = node->keys[i];
                // Tu M es max_keys. min_keys es (M-1)/2
                int min_keys = (M - 1) / 2;

                if (node->children[i]->count > min_keys) {
                    TK predecessor = getPredecessor(node, i);
                    node->keys[i] = predecessor;
                    removeFromNode(node->children[i], predecessor);
                }
                else if (node->children[i + 1]->count > min_keys) {
                    TK successor = getSuccessor(node, i);
                    node->keys[i] = successor;
                    removeFromNode(node->children[i + 1], successor);
                }
                else {
                    merge(node, i);
                    removeFromNode(node->children[i], keyToDelete);
                }
            }
        }
        else if (!node->leaf) {
            int min_keys = (M - 1) / 2;
            bool is_last_child = (i == node->count);

            if (node->children[i]->count == min_keys) {
                if (i != 0 && node->children[i - 1]->count > min_keys) {
                    borrowFromLeft(node, i);
                }
                else if (i != node->count && node->children[i + 1]->count > min_keys) {
                    borrowFromRight(node, i);
                }
                else {
                    if (i != node->count) {
                        merge(node, i);
                    } else {
                        merge(node, i - 1);
                        i--;
                    }
                }
            }

            if (root->count == 0 && !root->leaf) {
                 Node<TK>* oldRoot = root;
                 root = root->children[0];
                 delete oldRoot;
            }


            removeFromNode(node->children[i], key);
        }
    }

    void remove(TK key) {
        if (this->root == nullptr) return;

        bool key_found = search(key);

        if (!key_found) return; // No hacer nada si la clave no existe

        removeFromNode(this->root, key);

        if (this->root->count == 0) {
            Node<TK> *oldRoot = this->root;
            if (!this->root->leaf) {
                this->root = this->root->children[0];
            } else {
                this->root = nullptr;
            }
            delete oldRoot;
        }

        if (key_found) {
            this->n--;
        }
    }


    int height() {
        if (this->root == nullptr) {
            return 0;
        }
        int h = 1;
        Node<TK> *Act = this->root;
        while (!Act->leaf) {
            if (Act->children[0] == nullptr) {
                break;
            }
            Act = Act->children[0];
            h++;
        }
        return h;
    };

    string toString(const string& sep) {
        string result;
        inorder(root, sep, result);
        if (result.size() >= sep.size())
            result.erase(result.size() - sep.size());
        return result;
    }

    void inorder(Node<TK>* node, const string& sep, string& out) {
        if (!node) return;

        for (int i = 0; i < node->count; i++) {
            if (!node->leaf)
                inorder(node->children[i], sep, out);

            out += to_string(node->keys[i]) + sep;
        }

        if (!node->leaf)
            inorder(node->children[node->count], sep, out);
    }

    vector<TK> rangeSearch(TK begin, TK end) {
        vector<TK> result;

        function<void(Node<TK> *)> search = [&](Node<TK> *node) {
            if (node == nullptr) return;

            int i = 0;
            while (i < node->count && node->keys[i] < begin) {
                i++;
            }

            // Recorrer claves e hijos relevantes en este nodo
            while (i < node->count) {
                if (!node->leaf) {
                    search(node->children[i]);
                }

                if (node->keys[i] >= begin && node->keys[i] <= end) {
                    result.push_back(node->keys[i]);
                } else if (node->keys[i] > end) {
                     // Como las claves están ordenadas, si esta es mayor que 'end',
                     // no necesitamos seguir en este nodo ni en los hijos a la derecha.
                    return;
                }
                i++;
            }

            // Procesar el último hijo (hijo[i] donde i == node->count)
            if (!node->leaf) {
                search(node->children[i]);
            }
        };

        search(this->root);
        return result;
    }

    TK minKey() {
        if (this->root == nullptr) return TK{};
        Node<TK> *Act = this->root;
        while (!Act->leaf) {
            Act = Act->children[0];
        }
        return Act->keys[0];
    };

    TK maxKey() {
        if (this->root == nullptr) return TK{};
        Node<TK> *Act = this->root;
        while (!Act->leaf) {
            Act = Act->children[Act->count];
        }
        return Act->keys[Act->count - 1];
    };

    void clear() {
        if (this->root != nullptr) {
            this->root->killSelf(this->M);
            delete this->root;
            this->root = nullptr;
        }
        this->n = 0;
    }

    int size() {
        return n;
    };

    static BTree<TK> *build_from_ordered_vector(const vector<TK> &elements, int M) {
        BTree<TK> *tree = new BTree<TK>(M);
        for (const auto &e: elements) {
            tree->insert(e);
        }
        return tree;
    }

    bool check_properties() {
        if (this->root == nullptr) return true;

        // M es el número máximo de claves
        int maxKeys = this->M;
        // Mínimo de claves para nodos internos (no raíz)
        int minKeys = (M - 1) / 2;
        int expectedLeafDepth = -1;

        // Usamos una lambda recursiva
        function<bool(Node<TK> *, int, bool, TK, bool, TK)> check =
                [&](Node<TK> *node, int depth, bool hasMin, TK minV, bool hasMax, TK maxV) -> bool {

            if (node == nullptr) return false; // No debería haber hijos nulos

            // 1. Comprobar conteo de claves
            if (node == this->root) {
                // La raíz puede tener de 1 a maxKeys (a menos que el árbol esté vacío y n > 0)
                if (this->n > 0 && node->count < 1) return false;
                if (node->count > maxKeys) {
                    // Tu lógica inserta y luego splitea, por lo que count PUEDE
                    // llegar a M temporalmente. Pero check_properties no
                    // debería llamarse en ese estado. Asumimos que M es el max
                    // de claves *permitido* en un nodo estable.
                    // Si tu M=3, count max = 3.
                    if (node->count > M) return false;
                }
            } else {
                // Nodos internos deben tener de minKeys a maxKeys
                if (node->count < minKeys || node->count > maxKeys) return false;
            }

            // 2. Comprobar orden de claves y límites del padre
            for (int i = 0; i < node->count; ++i) {
                // Claves en orden ascendente
                if (i > 0 && !(node->keys[i - 1] < node->keys[i])) return false;

                // Clave debe ser >= que el límite inferior (clave del padre a la izq)
                // (Usamos >= y <= para ser genéricos, aunque B-Tree es estricto <)
                if (hasMin && !(node->keys[i] > minV)) return false;

                // Clave debe ser <= que el límite superior (clave del padre a la der)
                if (hasMax && !(node->keys[i] < maxV)) return false;
            }

            // 3. Comprobar hojas
            if (node->leaf) {
                if (expectedLeafDepth == -1) {
                    expectedLeafDepth = depth; // Setea la profundidad esperada
                }
                // Todas las hojas deben estar a la misma profundidad
                return (expectedLeafDepth == depth);
            }

            // 4. Comprobar hijos (si no es hoja)
            // Debe tener count + 1 hijos
            for (int i = 0; i <= node->count; ++i) {
                if (node->children[i] == nullptr) return false; // No debe tener hijos nulos
            }

            // 5. Recurso: Comprobar cada sub-árbol

            // Hijo 0: sin mínimo (o el del padre), máximo es keys[0]
            if (!check(node->children[0], depth + 1, hasMin, minV, true, node->keys[0])) return false;

            // Hijos 1 a count-1: mínimo keys[i-1], máximo keys[i]
            for (int i = 1; i < node->count; ++i) {
                if (!check(node->children[i], depth + 1, true, node->keys[i - 1], true, node->keys[i])) return false;
            }

            // Hijo 'count': mínimo keys[count-1], sin máximo (o el del padre)
            if (!check(node->children[node->count], depth + 1, true, node->keys[node->count - 1], hasMax, maxV))
                return false;

            return true;
        };

        return check(this->root, /*depth*/0, /*hasMin*/false, TK{}, /*hasMax*/false, TK{});
    };
    ~BTree() {
        clear();
    }
};

#endif