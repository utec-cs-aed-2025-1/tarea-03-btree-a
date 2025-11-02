#ifndef NODE_H
#define NODE_H

using namespace std;

template<typename TK>
struct Node {
    // array de keys
    TK *keys;
    // array de punteros a hijos
    Node **children;
    // cantidad de keys
    int count;
    // indicador de nodo hoja
    bool leaf;

    Node() : keys(nullptr), children(nullptr), count(0) {
    }

    Node(int M) {
        keys = new TK[M - 1];
        children = new Node<TK> *[M];
        count = 0;
        leaf = true;

        for (int i = 0; i < M; i++) {
            children[i] = nullptr;
        }
    }

    void insert(TK key) {
        int i = count - 1;
        while (i >= 0 && keys[i] > key) {
            keys[i + 1] = keys[i];
            i--;
        }
        keys[i + 1] = key;
        count++;
    }

    void killSelf(int M) {
        // Liberar recursivamente hijos
        if (children != nullptr) {
            for (int i = 0; i < M; ++i) {
                if (children[i] != nullptr) {
                    children[i]->killSelf(M);
                    delete children[i];
                    children[i] = nullptr;
                }
            }
            delete[] children;
            children = nullptr;
        }
        // Liberar claves
        if (keys != nullptr) {
            delete[] keys;
            keys = nullptr;
        }
        count = 0;
        leaf = true;
    }
};

#endif