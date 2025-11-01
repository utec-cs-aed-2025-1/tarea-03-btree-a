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

    void split_child(Node<TK> *parent, int child_index) {
        Node<TK> *full_child = parent->children[child_index];
        Node<TK> *new_sibling = new Node<TK>(M);
        new_sibling->leaf = full_child->leaf;
        int mid_key_index = (M - 2) / 2;

        // Claves que van al nuevo hermano
        new_sibling->count = (M - 1) - (mid_key_index + 1);
        for (int j = 0; j < new_sibling->count; j++) {
            new_sibling->keys[j] = full_child->keys[j + mid_key_index + 1];
        }

        // Hijos que van al nuevo hermano
        if (!full_child->leaf) {
            for (int j = 0; j <= new_sibling->count; j++) {
                new_sibling->children[j] = full_child->children[j + mid_key_index + 1];
            }
        }

        // Actualizar la cuenta del hijo original
        full_child->count = mid_key_index;

        // Mover hijos en el padre para hacer espacio
        for (int j = parent->count; j > child_index; j--) {
            parent->children[j + 1] = parent->children[j];
        }
        parent->children[child_index + 1] = new_sibling;

        // Mover claves en el padre y promover la clave media
        for (int j = parent->count - 1; j >= child_index; j--) {
            parent->keys[j + 1] = parent->keys[j];
        }
        parent->keys[child_index] = full_child->keys[mid_key_index];
        parent->count++;
    }

    void insert_non_full(Node<TK> *node, TK key) {
        int i = node->count - 1;
        if (node->leaf) {
            // Es hoja, insertar directamente (Node::insert)
            while (i >= 0 && node->keys[i] > key) {
                node->keys[i + 1] = node->keys[i];
                i--;
            }
            node->keys[i + 1] = key;
            node->count++;
            this->n++;
        } else {
            // No es hoja, encontrar hijo
            while (i >= 0 && node->keys[i] > key) {
                i--;
            }
            i++;
            // Si el hijo está lleno, dividirlo
            if (node->children[i]->count == M - 1) {
                split_child(node, i);
                // Decidir a qué hijo bajar
                if (key > node->keys[i]) {
                    i++;
                }
            }
            insert_non_full(node->children[i], key);
        }
    }

    // Auxiliar recursivo para toString
    void toString_internal(Node<TK>* node, string& result, const string &sep) {
        if (node == nullptr) {
            return;
        }
        int i;
        for (i = 0; i < node->count; i++) {
            // Recorrer hijo izquierdo
            if (!node->leaf) {
                toString_internal(node->children[i], result, sep);
            }

            // Procesar clave
            if (!result.empty()) {
                result += sep;
            }
            result += to_string(node->keys[i]);
        }

        // Recorrer último hijo (derecho)
        if (!node->leaf) {
            toString_internal(node->children[i], result, sep);
        }
    }


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

    // --- FUNCIÓN INSERT REESCRITA ---
    void insert(TK key) {
        if (root == nullptr) {
            root = new Node<TK>(M);
            root->keys[0] = key;
            root->count = 1;
            this->n = 1;
            return;
        }

        // Si la raíz está llena, el árbol crece en altura
        if (root->count == M - 1) {
            Node<TK> *new_root = new Node<TK>(M);
            new_root->leaf = false;
            new_root->children[0] = root;
            split_child(new_root, 0);
            root = new_root;
        }

        // Insertar en el árbol (que ahora sabemos que no tiene la raíz llena)
        insert_non_full(root, key);
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

    // recorrido inorder (REESCRITO)
    string toString(const string &sep) {
        string result = "";
        toString_internal(this->root, result, sep);
        return result;
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