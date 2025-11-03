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
    int M; // grado u orden del arbol
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
        // se copia la mitad derecha


        if (!curr->leaf) {
            for (int i = mid + 1, j = 0; i <= curr->count; ++i, ++j)
                newNode->children[j] = curr->children[i];
        }
        // se copia hijos si no es hoja


        for (int i = mid; i < M; ++i) curr->keys[i] = TK();
        // limpiar la mitad derecha del nodo actual
        curr->count = mid;

        cout << " - Clave media que sube: " << middleKey << endl;
        cout << " - Nueva mitad derecha: ";
        for (int i = 0; i < newNode->count; ++i) cout << newNode->keys[i] << " ";
        cout << endl;

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
            cout << " - [NUEVA RAÍZ] Clave raíz: " << middleKey << endl;
            root = newRoot;
            return true;
        }

        Node<TK> *parent = s.top();
        cout << " - Subiendo clave " << middleKey << " al padre con claves: ";
        for (int i = 0; i < parent->count; ++i) cout << parent->keys[i] << " ";
        cout << endl;

        int i;
        for (i = parent->count - 1; i >= 0 && parent->keys[i] > middleKey; --i) {
            parent->keys[i + 1] = parent->keys[i];
            parent->children[i + 2] = parent->children[i + 1];
        }

        parent->keys[i + 1] = middleKey;
        parent->children[i + 2] = newNode;
        parent->count++;

        cout << " - Padre después de insertar " << middleKey << ": ";
        for (int k = 0; k < parent->count; ++k) cout << parent->keys[k] << " ";
        cout << "(" << parent->count << ")" << endl;

        // Si el padre se llenó, seguimos hacia arriba
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
            cout << "[CREAR RAÍZ] Nueva raíz con clave " << key << endl;
            return;
        }

        stack<Node<TK> *> s;
        Node<TK> *curr = root;
        s.push(curr);


        while (!curr->leaf) {
            int i;
            for (i = 0; i < curr->count; ++i)
                if (curr->keys[i] > key) break;
            // desendemos hasta hoja

            cout << "[BAJAR] Clave " << key << " baja por nodo con claves: ";
            for (int j = 0; j < curr->count; ++j) cout << curr->keys[j] << " ";
            cout << endl;

            curr = curr->children[i];
            s.push(curr);
        }


        bool inserted = false;
        while (!inserted) {
            cout << "[INSERTAR] Insertando " << key << " en nodo con claves: ";
            for (int i = 0; i < curr->count; ++i) cout << curr->keys[i] << " ";
            cout << endl;

            curr->insert(key);

            cout << " -> Ahora el nodo tiene claves: ";
            for (int i = 0; i < curr->count; ++i) cout << curr->keys[i] << " ";
            cout << "(" << curr->count << ")" << endl;

            if (curr->count < M) {
                inserted = true;
            } else {
                cout << "\n>>> [SPLIT] Nodo lleno. Claves antes del split: ";
                for (int i = 0; i < curr->count; ++i) cout << curr->keys[i] << " ";
                cout << endl;

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

    //obtener sucesor (funcion auxiliar)
    TK getSuccessor(Node<TK> *node, int idx) {
        Node<TK> *current = node->children[idx + 1];
        while (!current->leaf) {
            current = current->children[0];
        }
        return current->keys[0];
    }

    //funcion merge (Funcion auxiliar y recursiva)
    void merge(Node<TK> *parent, int idx) {
        Node<TK> *child = parent->children[idx];
        Node<TK> *sibling = parent->children[idx + 1];

        // Pasar la clave del padre al hijo
        child->keys[child->count] = parent->keys[idx];

        // Copiar todas las claves del hermano al hijo
        for (int i = 0; i < sibling->count; i++) {
            child->keys[child->count + 1 + i] = sibling->keys[i];
        }

        // Si no es hoja, copiar también los hijos
        if (!child->leaf) {
            for (int i = 0; i <= sibling->count; i++) {
                child->children[child->count + 1 + i] = sibling->children[i];
            }
        }

        child->count += sibling->count + 1;

        for (int i = idx; i < parent->count - 1; i++) {
            parent->keys[i] = parent->keys[i + 1];
        }

        // Eliminar el puntero al hermano
        for (int i = idx + 1; i < parent->count; i++) {
            parent->children[i] = parent->children[i + 1];
        }

        parent->count--;
        delete sibling;
    }

    //rotacion a la izquierda
    void borrowFromLeft(Node<TK> *parent, int childIdx) {
        Node<TK> *child = parent->children[childIdx];
        Node<TK> *leftSibling = parent->children[childIdx - 1];

        // Desplazar las claves del hijo a la derecha
        for (int i = child->count - 1; i >= 0; i--) {
            child->keys[i + 1] = child->keys[i];
        }

        // Traer clave del padre
        child->keys[0] = parent->keys[childIdx - 1];

        // Si no es hoja, desplazar hijos también
        if (!child->leaf) {
            for (int i = child->count; i >= 0; i--) {
                child->children[i + 1] = child->children[i];
            }
            child->children[0] = leftSibling->children[leftSibling->count];
        }

        // Subir clave del hermano izquierdo al padre
        parent->keys[childIdx - 1] = leftSibling->keys[leftSibling->count - 1];

        child->count++;
        leftSibling->count--;
    }

    //rotacion a la derecha
    void borrowFromRight(Node<TK> *parent, int childIdx) {
        Node<TK> *child = parent->children[childIdx];
        Node<TK> *rightSibling = parent->children[childIdx + 1];

        // Traer clave del padre
        child->keys[child->count] = parent->keys[childIdx];

        // Si no es hoja, traer hijo del hermano derecho
        if (!child->leaf) {
            child->children[child->count + 1] = rightSibling->children[0];
        }

        // Subir clave del hermano derecho al padre
        parent->keys[childIdx] = rightSibling->keys[0];

        // Desplazar claves del hermano derecho
        for (int i = 0; i < rightSibling->count - 1; i++) {
            rightSibling->keys[i] = rightSibling->keys[i + 1];
        }

        // Desplazar hijos del hermano derecho
        if (!rightSibling->leaf) {
            for (int i = 0; i < rightSibling->count; i++) {
                rightSibling->children[i] = rightSibling->children[i + 1];
            }
        }

        child->count++;
        rightSibling->count--;
    }

    //funcion recursiva de eliminacion
    void removeFromNode(Node<TK> *node, TK key) {
        int i = 0;

        // Encontrar la posición de la clave
        while (i < node->count && node->keys[i] < key) {
            i++;
        }

        // Caso 1: La clave está en este nodo
        if (i < node->count && node->keys[i] == key) {
            if (node->leaf) {
                // Caso 1a: Es una hoja, eliminar directamente
                for (int j = i; j < node->count - 1; j++) {
                    node->keys[j] = node->keys[j + 1];
                }
                node->count--;
            } else {
                // Caso 1b: Es un nodo interno
                TK keyToDelete = node->keys[i];
                int min_keys = (M - 1) / 2; // Mínimo de claves

                // Si el hijo izquierdo tiene suficientes claves
                if (node->children[i]->count > min_keys) {
                    TK predecessor = getPredecessor(node, i);
                    node->keys[i] = predecessor;
                    removeFromNode(node->children[i], predecessor);
                }
                // Si el hijo derecho tiene suficientes claves
                else if (node->children[i + 1]->count > min_keys) {
                    TK successor = getSuccessor(node, i);
                    node->keys[i] = successor;
                    removeFromNode(node->children[i + 1], successor);
                }
                // Si ambos tienen el mínimo, fusionarlos
                else {
                    merge(node, i);
                    removeFromNode(node->children[i], keyToDelete);
                }
            }
        }
        // Caso 2: La clave no está en este nodo, buscar en subárbol
        else if (!node->leaf) {
            int min_keys = (M - 1) / 2; // Mínimo de claves
            bool is_last_child = (i == node->count);

            // Si el hijo donde debería estar tiene pocas claves
            if (node->children[i]->count == min_keys) {
                // Intentar tomar del hermano izquierdo
                if (i != 0 && node->children[i - 1]->count > min_keys) {
                    borrowFromLeft(node, i);
                }
                // Intentar tomar del hermano derecho
                else if (i != node->count && node->children[i + 1]->count > min_keys) {
                    borrowFromRight(node, i);
                }
                // Fusionar con un hermano
                else {
                    if (i != node->count) {
                        merge(node, i);
                    } else {
                        merge(node, i - 1);
                        i--; // Ajustar 'i' porque ahora descendemos al nodo [i-1]
                    }
                }
            }

            // Descender al hijo apropiado
            removeFromNode(node->children[i], key);
        }
    }

    //elimina un elemento
    void remove(TK key) {
        if (this->root == nullptr) return;

        bool key_found = search(key); // Verificar si la clave existe

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


    //altura del arbol. Considerar altura 0 para arbol vacio
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

    string toString(const string &sep) {
        string result;
        inorder(root, sep, result);
        if (result.size() >= sep.size())
            result.erase(result.size() - sep.size());
        return result;
    }

    void inorder(Node<TK> *node, const string &sep, string &out) {
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
            // Encontrar la primera clave >= begin
            while (i < node->count && node->keys[i] < begin) {
                i++;
            }

            // Recorrer hijos y claves mientras sean <= end
            while (i < node->count) {
                if (!node->leaf) {
                    search(node->children[i]);
                }

                if (node->keys[i] >= begin && node->keys[i] <= end) {
                    result.push_back(node->keys[i]);
                } else if (node->keys[i] > end) {
                    break; // Salir si la clave ya es mayor que 'end'
                }
                i++;
            }

            // Recorrer el último hijo si no hemos salido
            if (!node->leaf && i == node->count) {
                search(node->children[i]);
            }
        };

        search(this->root);
        return result;
    }

    // minimo valor de la llave en el arbol
    TK minKey() {
        if (this->root == nullptr) return TK{}; // Devolver valor por defecto si está vacío
        Node<TK> *Act = this->root;
        while (!Act->leaf) {
            Act = Act->children[0];
        }
        return Act->keys[0];
    };
    // maximo valor de la llave en el arbol
    TK maxKey() {
        if (this->root == nullptr) return TK{}; // Devolver valor por defecto si está vacío
        Node<TK> *Act = this->root;
        while (!Act->leaf) {
            Act = Act->children[Act->count];
        }
        return Act->keys[Act->count - 1];
    };
    // eliminar todos lo elementos del arbol
    void clear() {
        if (this->root != nullptr) {
            this->root->killSelf(this->M);
            delete this->root;
            this->root = nullptr;
        }
        this->n = 0;
    }

    // retorna el total de elementos insertados
    int size() {
        return n;
    };

    // Construya un árbol B a partir de un vector de elementos ordenados.
    static BTree<TK> *build_from_ordered_vector(const vector<TK> &elements, int M) {
        BTree<TK> *tree = new BTree<TK>(M);
        for (const auto &e: elements) {
            tree->insert(e);
        }
        return tree;
    }

    // Verifique las propiedades de un árbol B
    bool check_properties() {
        // Árbol vacío es válido
        if (this->root == nullptr) return true;

        int t = (M + 1) / 2;
        int maxKeys = this->M - 1;
        int expectedLeafDepth = -1;

        function<bool(Node<TK> *, int, bool, TK, bool, TK)> check =
                [&](Node<TK> *node, int depth, bool hasMin, TK minV, bool hasMax, TK maxV) -> bool {
            if (node == nullptr) return false;

            //Rango de cantidad de claves
            if (node == this->root) {
                if (node->count < 1 || node->count > maxKeys) return false;
            } else {
                int minKeys = (M - 1) / 2; // Mínimo en nodos no-raíz
                if (node->count < minKeys || node->count > maxKeys) return false;
            }

            //Claves ordenadas y dentro de límites [minV, maxV)
            for (int i = 0; i < node->count; ++i) {
                if (i > 0 && !(node->keys[i - 1] < node->keys[i])) return false;
                if (hasMin && !(minV < node->keys[i])) return false;
                if (hasMax && !(node->keys[i] < maxV)) return false;
            }

            if (node->leaf) {
                //Todas las hojas a la misma profundidad
                if (expectedLeafDepth == -1) expectedLeafDepth = depth;
                return expectedLeafDepth == depth;
            }

            //Nodos internos deben tener count+1 hijos no nulos en [0..count]
            for (int i = 0; i <= node->count; ++i) {
                if (node->children[i] == nullptr) return false;
            }

            //Recorrer hijos con límites apropiados
            // hijo 0
            if (!check(node->children[0], depth + 1, hasMin, minV, true, node->keys[0])) return false;
            // hijos intermedios
            for (int i = 1; i < node->count; ++i) {
                if (!check(node->children[i], depth + 1, true, node->keys[i - 1], true, node->keys[i])) return false;
            }
            // hijo final
            if (!check(node->children[node->count], depth + 1, true, node->keys[node->count - 1], hasMax, maxV))
                return
                        false;

            return true;
        };

        return check(this->root, /*depth*/0, /*hasMin*/false, TK{}, /*hasMax*/false, TK{});
    };

    ~BTree() {
        clear();
    } // liberar memoria
};

#endif
