// copyright Luca Istrate, Andrei Medar
#include "randomForest.h"
#include <iostream>
#include <random>
#include <vector>
#include <string>
#include "decisionTree.h"

using std::vector;
using std::pair;
using std::string;
using std::mt19937;

vector<vector<int>> get_random_samples(const vector<vector<int>> &samples,
                                       int num_to_return) {
    // TODO(you)
    // Intoarce un vector de marime num_to_return cu elemente random,
    // diferite din samples
    int min = 0;
    int max = samples.size() - 1;
    vector<vector<int>> ret;
    while (num_to_return != ret.size()){
        int ok = 1;
        int index = (min + (std::rand()%(int)(max - min + 1)));
        for (int j = 0; j < ret.size(); ++j){
            if (samples[index] == ret[j]){
                ok = 0;
                break;
            }
        }
        if (ok == 1){
            ret.push_back(samples[index]);
        }
    }
    return ret;
}

RandomForest::RandomForest(int num_trees, const vector<vector<int>> &samples)
    : num_trees(num_trees), images(samples) {}

void RandomForest::build() {
    // Aloca pentru fiecare Tree cate n / num_trees
    // Unde n e numarul total de teste de training
    // Apoi antreneaza fiecare tree cu testele alese
    assert(!images.empty());
    vector<vector<int>> random_samples;

    int data_size = images.size() / num_trees;

    for (int i = 0; i < num_trees; i++) {
        // cout << "Creating Tree nr: " << i << endl;
        random_samples = get_random_samples(images, data_size);

        // Construieste un Tree nou si il antreneaza
        trees.push_back(Node());
        trees[trees.size() - 1].train(random_samples);
    }
}

int RandomForest::predict(const vector<int> &image) {
    // TODO(you)
    // Va intoarce cea mai probabila prezicere pentru testul din argument
    // se va interoga fiecare Tree si se va considera raspunsul final ca
    // fiind cel majoritar
    vector <int> freq(10, 0);
    int max = 0, index;
    for (int i = 0; i < num_trees; ++i){
        freq[trees[i].predict(image)]++;

        if (max < freq[trees[i].predict(image)]){
            max = freq[trees[i].predict(image)];
            index = trees[i].predict(image);
        }
    }
    return index;
}
