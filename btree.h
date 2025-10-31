#ifndef BTree_H
#define BTree_H
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include "node.h"

using namespace std;

template <typename TK>
class BTree {
 private:
  Node<TK>* root;
  int M;  // grado u orden del arbol
  int n; // total de elementos en el arbol 

 public:
  BTree(int _M) : root(nullptr), M(_M) {}

  bool search(TK key){
      Node<TK>* current = this->root;
      if (current == nullptr){
        return false;
      }
      while (current != nullptr){
        int i = 0;
        while (i < current->count && current->keys[i] < key){
          i++;
        }
        if (i < current->count && current->keys[i] == key){
          return true;
        }

        if (current->leaf){
          return false;
        }
        current = current->children[i];
      }
      return false;      
  };

  void split(Node<TK>* p, int h){
    Node<TK>* Hlleno = p->children[h];
    Node<TK>* Vnuevo = new Node(this->M);
    Vnuevo->leaf = Hlleno->leaf;

    int mid = (M-1)/2;
    TK Medio = Hlleno->keys[mid];
    int j = 0;
    for (int i = mid + 1; i < Hlleno->count; i++)
    {
      Vnuevo->keys[j] = Hlleno->keys[i];
      j++;
    }
    Vnuevo->count = j;

    if(!Hlleno->leaf){
      int j = 0;
      for (int i = mid + 1; i <= Hlleno->count; i++) {
            Vnuevo->children[j] = Hlleno->children[i];
            j++;
        }
    }
    Hlleno->count = mid;

    for (int i = p->count; i > h; --i) {
        p->children[i + 1] = p->children[i];
    }
    p->children[h+1] = Vnuevo;

    for (int i = p->count - 1; i >= h; --i) {
        p->keys[i + 1] = p->keys[i];
    }
    p->keys[h] = Medio;
    p->count++;

  }

  void insertNonFull(Node<TK>* node, TK key){
    if(node->leaf){
      int i = node->count - 1;
      while (i >= 0 && node->keys[i] > key){
        node->keys[i + 1] = node->keys[i];
        i--;
      }
      node->keys[i+1] = key;
      node->count++;
      this->n++;
    }
    else{
      int i = 0;
      while(i < node->count && key > node->keys[i]){
        i++;
      }
      if (node->children[i]->count == M-1){
        split(node,i);
        if (key > node->keys[i]){
          i++;
        }
      }
      insertNonFull(node->children[i], key);
    }
  }

  void insert(TK key){
    if (this->root == nullptr){
      this->root = new Node<TK>(this->M);
      this->root->keys[0] = key;
      this->root->count = 1;
      this->root->leaf = true;
      this->n = 1;
      return;
    }

    else if (this->root->count == M - 1){
      Node<TK>* nuevo = new Node<TK>(this->M);
      nuevo->leaf = false;
      nuevo->children[0] = this->root;
      this->root = nuevo;
      split(this->root, 0);
      insertNonFull(this->root, key);
    }
    else{
      insertNonFull(this->root,key);
    }
  };
  
  void remove(TK key);  //elimina un elemento
  
  //altura del arbol. Considerar altura 0 para arbol vacio
  int height(){
    if (this->root == nullptr){
      return 0;
    }
    int h = 1;
    Node<TK>* Act = this->root;
    while (!Act->leaf){
      if(Act->children[0] == nullptr){
        break;
      }
      Act = Act->children[0];
      h++;
    }
    return h;
  };
  string toString(const string& sep);  // recorrido inorder
  vector<TK> rangeSearch(TK begin, TK end);

  // minimo valor de la llave en el arbol
  TK minKey(){
    Node<TK>* Act = this->root;
    while (!Act->leaf){
      Act = Act->children[0];
    }
    return Act->keys[0];
  };
  // maximo valor de la llave en el arbol
  TK maxKey(){
    Node<TK>* Act = this->root;
    while (!Act->leaf){
      Act = Act->children[Act->count];
    }
    return Act->keys[Act->count-1];
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
  int size(){
    return n;
  };  
  
  // Construya un árbol B a partir de un vector de elementos ordenados
  static BTree<TK>* build_from_ordered_vector(const vector<TK>& elements, int M) {
    BTree<TK>* tree = new BTree<TK>(M);
    for (const auto& e : elements) {
      tree->insert(e);
    }
    return tree;
  }
  // Verifique las propiedades de un árbol B
  bool check_properties(){
    // Árbol vacío es válido
    if (this->root == nullptr) return true;

    int t = (this->M + 1) / 2;
    int maxKeys = this->M - 1;
    int expectedLeafDepth = -1;

    function<bool(Node<TK>*, int, bool, TK, bool, TK)> check =
      [&](Node<TK>* node, int depth, bool hasMin, TK minV, bool hasMax, TK maxV) -> bool {
        if (node == nullptr) return false;

        //Rango de cantidad de claves
        if (node == this->root) {
          if (node->count < 1 || node->count > maxKeys) return false;
        } else {
          int minKeys = t - 1; // mínimo en nodos no-raíz
          if (node->count < minKeys || node->count > maxKeys) return false;
        }

        //Claves ordenadas y dentro de límites [minV, maxV)
        for (int i = 0; i < node->count; ++i) {
          if (i > 0 && !(node->keys[i-1] < node->keys[i])) return false;
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
          if (!check(node->children[i], depth + 1, true, node->keys[i-1], true, node->keys[i])) return false;
        }
        // hijo final
        if (!check(node->children[node->count], depth + 1, true, node->keys[node->count - 1], hasMax, maxV)) return false;

        return true;
      };

    return check(this->root, /*depth*/0, /*hasMin*/false, TK{}, /*hasMax*/false, TK{});
  };

  ~BTree(){
    clear();
  }     // liberar memoria
};

#endif
