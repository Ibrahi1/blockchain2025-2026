// 1. Implémente en C++ un automate cellulaire 1D avec état binaire et voisinage r = 1.
#include <iostream>
#include <vector>
#include <bitset>

using namespace std;

// Fonction d'initialisation
vector<int> init_state(const vector<int>& bits) {
    return bits;
}

// Fonction pour afficher un état
void print_state(const vector<int>& state) {
    for (int cell : state) {
        cout << (cell ? "█" : " "); // joli affichage
    }
    cout << endl;
}

// Fonction evolve : applique une règle de Wolfram (Rule 30, 90, 110, etc.)
vector<int> evolve(const vector<int>& state, int rule_number) {
    int n = state.size();
    vector<int> next(n, 0);

    // Conversion du numéro de règle (0–255) en binaire sur 8 bits
    bitset<8> rule(rule_number);

    for (int i = 0; i < n; ++i) {
        // On considère les voisins gauche, centre, droite
        int left   = (i == 0) ? 0 : state[i - 1];
        int center = state[i];
        int right  = (i == n - 1) ? 0 : state[i + 1];

        // Création du triplet comme un index (ex: 111->7, 110->6, ..., 000->0)
        int index = (left << 2) | (center << 1) | right;

        // La cellule suivante dépend du bit correspondant dans la règle
        next[i] = rule[index];
    }
    return next;
}

// Exemple de test
int main() {
    // Initialisation avec un seul "1" au centre
    int n = 21;
    vector<int> state(n, 0);
    state[n / 2] = 1;
    
    vector<int> state_90(n, 0);
    state_90[n / 2] = 1;
    
    vector<int> state_110(n, 0);
    state_110[n / 2] = 1;

    int rule_number_30 = 30;
    int rule_number_90 = 90;
    int rule_number_110 = 110;
    
    cout << "Automate 1D - Rule " << rule_number_30 << endl;
    print_state(state);

    for (int t = 0; t < 20; ++t) {
        state = evolve(state, rule_number_30);
        print_state(state);
    }
    cout << endl;

    cout << "Automate 1D - Rule " << rule_number_90 << endl;
    print_state(state_90);

    for (int t = 0; t < 20; ++t) {
        state_90 = evolve(state_90, rule_number_90);
        print_state(state_90);
    }
    cout << endl;
    
    cout << "Automate 1D - Rule " << rule_number_110 << endl;
    print_state(state_110);

    for (int t = 0; t < 20; ++t) {
        state_110 = evolve(state_110, rule_number_110);
        print_state(state_110);
    }
    return 0;
}
