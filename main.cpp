#include <iostream>
#include "btree.h"
#include "tester.h"

using namespace std;

int main() {
    cout << "\n========== FASE 1: INSERTAR 10 ELEMENTOS ==========" << endl;
    BTree<int>* btree = new BTree<int>(3);
    int numbers[10] = {45, 75, 100, 36, 120, 70, 11, 111, 47, 114};

    for (int i = 0; i < 10; i++) {
        btree->insert(numbers[i]);
    }

    cout << "\n========== DESPUÉS DE INSERTAR ==========" << endl;
    cout << "toString: " << btree->toString(" ") << endl;
    cout << "height: " << btree->height() << endl;
    cout << "size: " << btree->size() << endl;
    cout << "check_properties: " << (btree->check_properties() ? "TRUE" : "FALSE") << endl;

    if(!btree->check_properties()) {
        cout << "ERROR: El árbol no cumple propiedades después de insertar" << endl;
        return 0;
    }

    cout << "\n========== TESTS DESPUÉS DE INSERTAR ==========" << endl;
    ASSERT(btree->toString(" ") == "11 36 45 47 70 75 100 111 114 120",
           "The function insert is not working");
    ASSERT(btree->height() == 2, "The function height is not working");
    for (int i = 0; i < 10; i=i+2) {
        ASSERT(btree->search(numbers[i]) == true,
               "The function search is not working");
    }
    ASSERT(btree->minKey() == 11, "The function minKey is not working");
    ASSERT(btree->maxKey() == 120, "The function maxKey is not working");

    cout << "\n========== FASE 2: REMOVER 100 ==========" << endl;
    cout << "Estado antes: " << btree->toString(" ") << endl;
    btree->remove(100);
    cout << "Estado después: " << btree->toString(" ") << endl;
    cout << "size: " << btree->size() << endl;
    cout << "check_properties: " << (btree->check_properties() ? "TRUE" : "FALSE") << endl;

    cout << "\n========== FASE 3: REMOVER 111 ==========" << endl;
    cout << "Estado antes: " << btree->toString(" ") << endl;
    btree->remove(111);
    cout << "Estado después: " << btree->toString(" ") << endl;
    cout << "size: " << btree->size() << endl;
    cout << "check_properties: " << (btree->check_properties() ? "TRUE" : "FALSE") << endl;

    cout << "\n========== FASE 4: REMOVER 45 ==========" << endl;
    cout << "Estado antes: " << btree->toString(" ") << endl;
    btree->remove(45);
    cout << "Estado después: " << btree->toString(" ") << endl;
    cout << "size: " << btree->size() << endl;
    cout << "check_properties: " << (btree->check_properties() ? "TRUE" : "FALSE") << endl;

    if(!btree->check_properties()) {
        cout << "ERROR: El árbol no cumple propiedades después de remover" << endl;
        return 0;
    }

    cout << "\n========== TEST DESPUÉS DE REMOVER 100, 111, 45 ==========" << endl;
    cout << "Esperado: 11 36 47 70 75 114 120" << endl;
    cout << "Obtenido: " << btree->toString(" ") << endl;
    ASSERT(btree->toString(" ") == "11 36 47 70 75 114 120",
           "The function remove is not working");
    ASSERT(btree->height() == 1, "The function height is not working");

    cout << "\n========== FASE 5: REMOVER 114 ==========" << endl;
    cout << "Estado antes: " << btree->toString(" ") << endl;
    btree->remove(114);
    cout << "Estado después: " << btree->toString(" ") << endl;
    cout << "size: " << btree->size() << endl;

    cout << "\n========== FASE 6: INSERTAR 40 ==========" << endl;
    cout << "Estado antes: " << btree->toString(" ") << endl;
    btree->insert(40);
    cout << "Estado después: " << btree->toString(" ") << endl;
    cout << "size: " << btree->size() << endl;
    cout << "check_properties: " << (btree->check_properties() ? "TRUE" : "FALSE") << endl;

    cout << "\n========== TEST ==========" << endl;
    cout << "Esperado (con -): 11 - 36 - 40 - 47 - 70 - 75 - 120" << endl;
    cout << "Obtenido (con -): " << btree->toString(" - ") << endl;
    ASSERT(btree->toString(" - ") == "11 - 36 - 40 - 47 - 70 - 75 - 120",
           "The function toString is not working");
    ASSERT(btree->height() == 2, "The function height is not working");

    cout << "\n========== FASE 7: INSERTAR 125 ==========" << endl;
    cout << "Estado antes: " << btree->toString(" ") << endl;
    btree->insert(125);
    cout << "Estado después: " << btree->toString(" ") << endl;
    cout << "size: " << btree->size() << endl;

    cout << "\n========== FASE 8: INSERTAR 115 ==========" << endl;
    cout << "Estado antes: " << btree->toString(" ") << endl;
    btree->insert(115);
    cout << "Estado después: " << btree->toString(" ") << endl;
    cout << "size: " << btree->size() << endl;

    cout << "\n========== TESTS ==========" << endl;
    ASSERT(btree->maxKey() == 125, "The function maxKey is not working");
    ASSERT(btree->search(47) == true, "The function search is not working");

    cout << "\n========== FASE 9: REMOVER 11 ==========" << endl;
    cout << "Estado antes: " << btree->toString(" ") << endl;
    btree->remove(11);
    cout << "Estado después: " << btree->toString(" ") << endl;
    cout << "size: " << btree->size() << endl;

    cout << "\n========== TESTS FINALES ==========" << endl;
    ASSERT(btree->search(11) == false, "The function search is not working");
    ASSERT(btree->minKey() == 36, "The function minKey is not working");
    ASSERT(btree->size() == 8, "The function size is not working");

    cout << "\n========== CLEAR ==========" << endl;
    btree->clear();
    ASSERT(btree->size() == 0, "The function size is not working");
    ASSERT(btree->height() == 0, "The function height is not working");

    cout << "\n========== BUILD FROM ORDERED VECTOR ==========" << endl;
    std::vector<int> elements = {1,2,3,4,5,6,7,8,9,10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    BTree<int>* btree2 = BTree<int>::build_from_ordered_vector(elements, 4);
    cout << "toString: " << btree2->toString(",") << endl;
    ASSERT(btree2->toString(",") == "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20", "The function build_from_ordered_vector is not working");
    if(btree2->check_properties()){
        cout << "El árbol 2 cumple con las propiedades de un árbol B." << endl;
    }else{
        cout << "El árbol 2 no cumple con las propiedades de un árbol B." << endl;
    }

    cout << "\n========== RESUMEN ==========" << endl;
    cout << "Total de tests: " << TotalAsserts << endl;
    cout << "Tests pasados: " << TrueAsserts << endl;
    cout << "Tests fallidos: " << (TotalAsserts - TrueAsserts) << endl;

    return 0;
}