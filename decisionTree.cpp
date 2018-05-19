// copyright Luca Istrate, Andrei Medar

#include "./decisionTree.h"  // NOLINT(build/include)
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using std::string;
using std::pair;
using std::vector;
using std::unordered_map;
using std::make_shared;

// structura unui nod din decision tree
// splitIndex = dimensiunea in functie de care se imparte
// split_value = valoarea in functie de care se imparte
// is_leaf si result sunt pentru cazul in care avem un nod frunza
Node::Node() {
    is_leaf = false;
    left = nullptr;
    right = nullptr;
}

void Node::make_decision_node(const int index, const int val) {
    split_index = index;
    split_value = val;
}

void Node::make_leaf(const vector<vector<int>> &samples,
                     const bool is_single_class) {
    // TODO(you)
    // Seteaza nodul ca fiind de tip frunza (modificati is_leaf si result)
    // is_single_class = true -> toate testele au aceeasi clasa (acela e result)
    // is_single_class = false -> se alege clasa care apare cel mai des
    this->is_leaf = true;
    if (is_single_class == false){
        vector <int> frequency(10, 0);
        int max = 0, index;
        for (int i = 0; i < samples.size(); ++i){
            frequency[samples[i][0]]++;
            if (frequency[samples[i][0]] > max){
                max = frequency[samples[i][0]];
                index = i;
            }
        }
        this->result = samples[index][0];
    } else {
        this->result = samples[0][0];
    }
}


pair<int, int> find_best_split(const vector<vector<int>> &samples,
                               const vector<int> &dimensions) {
    // TODO(you)
    // Intoarce cea mai buna dimensiune si valoare de split dintre testele
    // primite. Prin cel mai bun split (dimensiune si valoare)
    // ne referim la split-ul care maximizeaza IG
    // pair-ul intors este format din (split_index, split_value)
    pair<vector<int>, vector<int>> children;
    vector<int> unique;
    int splitIndex = -1, splitValue = -1;
    float max = -1;
    float ig = 0;
    for (int i = 1; i < samples[0].size(); ++i){
        float H_parent = get_entropy_by_indexes(samples,
                            dimensions);
        unique = compute_unique(samples, i);
        for (int j = 0; j < unique.size(); ++j){
            if (unique[j] < 5 || unique[j] > 250){
                children = get_split_as_indexes(samples, i, unique[j]);
                float H_left = get_entropy_by_indexes(samples, children.first);
                float H_right = get_entropy_by_indexes
                                (samples, children.second);
                ig = H_parent - (children.first.size() * H_left +
                                children.second.size()
                                * H_right)
                                /(children.first.size() +
                                children.second.size());
                if (ig > max){
                    max = ig;
                    splitIndex = i;
                    splitValue = unique[j];
                }
            }
        }
    }
    return pair<int, int>(splitIndex, splitValue);
}

void Node::train(const vector<vector<int>> &samples) {
    // TODO(you)
    // Antreneaza nodul curent si copii sai, daca e nevoie
    // 1) verifica daca toate testele primite au aceeasi clasa (raspuns)
    // Daca da, acest nod devine frunza, altfel continua algoritmul.
    // 2) Daca nu exista niciun split valid, acest nod devine frunza. Altfel,
    // ia cel mai bun split si continua recursiv
    pair <int, int> decision;
    pair <vector <vector<int>>, vector <vector<int>>> subsets;
    if (same_class(samples) == true){
        make_leaf(samples, true);
    } else{
        decision = find_best_split(samples,
                    random_dimensions(samples.size()));
        if (decision.first == -1 && decision.second == -1){
            make_leaf(samples, false);
        } else{
            int new_index = decision.first;
            int new_value = decision.second;
            make_decision_node(new_index, new_value);
            subsets = split(samples, new_index, new_value);
            if (left == nullptr){
                left = make_shared <Node>(Node());
            }
            if (right == nullptr){
                right = make_shared <Node>(Node());
            }
            left->train(subsets.first);
            right->train(subsets.second);
        }
    }
}

int Node::predict(const vector<int> &image) const {
    // TODO(you)
    // Intoarce rezultatul prezis de catre decision tree
    if (this->is_leaf == true){
        return this->result;
    } else{
        if (image[split_index - 1] > split_value){
            this->right->predict(image);
        } else{
            this->left->predict(image);
        }
    }
    // return 0;
}

bool same_class(const vector<vector<int>> &samples) {
    // TODO(you)
    // Verifica daca testele primite ca argument au toate aceeasi
    // clasa(rezultat). Este folosit in train pentru a determina daca
    // mai are rost sa caute split-uri

    int aux = samples[0][0];
    for (int i = 1; i < samples.size(); ++i){
        if ( samples[i][0] != aux ){
            return false;
        }
    }
    return true;
}

float get_entropy(const vector<vector<int>> &samples) {
    // Intoarce entropia testelor primite
    assert(!samples.empty());
    vector<int> indexes;

    int size = samples.size();
    for (int i = 0; i < size; i++) indexes.push_back(i);

    return get_entropy_by_indexes(samples, indexes);
}

float get_entropy_by_indexes(const vector<vector<int>> &samples,
                             const vector<int> &index) {
    // TODO(you)
    // Intoarce entropia subsetului din setul de teste total(samples)
    // Cu conditia ca subsetul sa contina testele ale caror indecsi se gasesc in
    // vectorul index (Se considera doar liniile din vectorul index)
    vector<float> freq(10, 0);
    float entropy = 0.0f;
    for (int i = 0; i < index.size(); ++i){
        freq[samples[index[i]][0]]++;
    }
    for (int i = 0; i < freq.size(); ++i){
        float tmp;
        tmp = 1.0 * freq[i]/index.size();
        if (tmp != 0){
            entropy -= tmp * log2(tmp);
        }
    }
    return entropy;
}

vector<int> compute_unique(const vector<vector<int>> &samples, const int col) {
    // TODO(you)
    // Intoarce toate valorile (se elimina duplicatele)
    // care apar in setul de teste, pe coloana col
    vector<int> uniqueValues;
    for (int i = 0; i < samples.size(); ++i){
        int ok = 1;
        for (int j = 0; j < uniqueValues.size(); ++j){
            if (samples[i][col] == uniqueValues[j]){
                ok = 0;
                break;
            }
        }
        if (ok == 1){
            uniqueValues.push_back(samples[i][col]);
        }
    }
    return uniqueValues;
}

pair<vector<vector<int>>, vector<vector<int>>> split(
    const vector<vector<int>> &samples, const int split_index,
    const int split_value) {
    // Intoarce cele 2 subseturi de teste obtinute in urma separarii
    // In functie de split_index si split_value
    vector<vector<int>> left, right;

    auto p = get_split_as_indexes(samples, split_index, split_value);
    for (const auto &i : p.first) left.push_back(samples[i]);
    for (const auto &i : p.second) right.push_back(samples[i]);

    return pair<vector<vector<int>>, vector<vector<int>>>(left, right);
}

pair<vector<int>, vector<int>> get_split_as_indexes(
    const vector<vector<int>> &samples, const int split_index,
    const int split_value) {
    // TODO(you)
    // Intoarce indecsii sample-urilor din cele 2 subseturi obtinute in urma
    // separarii in functie de split_index si split_value
    vector<int> left, right;
    for (int i = 0; i < samples.size(); ++i){
        if (samples[i][split_index] <= split_value){
            left.push_back(i);
        } else{
            right.push_back(i);
        }
    }
    return make_pair(left, right);
}

vector<int> random_dimensions(const int size) {
    // TODO(you)
    // Intoarce sqrt(size) dimensiuni diferite pe care sa caute splitul maxim
    // Precizare: Dimensiunile gasite sunt > 0 si < size
    int nr = floor(sqrt(size));
    int min = 1;
    int max = size - 1;
    vector<int> rez;
    while (nr != rez.size()){
        int ok = 1;
        int num = (min + (std::rand()%(int)(max - min + 1)));
        for (int i = 0; i < rez.size(); ++i){
            if (num == rez[i]){
                ok = 0;
                break;
            }
        }
        if (ok == 1){
            rez.push_back(num);
        }
    }
    return rez;
}
