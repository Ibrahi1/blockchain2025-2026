#include <iostream>
#include <vector>
#include <bitset>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdint>

using namespace std;

// --- Tes fonctions existantes ---
vector<int> evolve(const vector<int>& state, int rule_number);
vector<int> init_state(const vector<int>& bits) { return bits; }

// --- Conversion texte → bits ---
vector<int> text_to_bits(const string& input) {
    vector<int> bits;
    for (unsigned char c : input) {
        for (int i = 7; i >= 0; --i)
            bits.push_back((c >> i) & 1);
    }
    return bits;
}

// --- Conversion bits → hex string ---
string bits_to_hex(const vector<int>& bits) {
    stringstream ss;
    for (size_t i = 0; i < bits.size(); i += 8) {
        unsigned char byte = 0;
        for (int b = 0; b < 8; ++b)
            byte = (byte << 1) | bits[i + b];
        ss << hex << setw(2) << setfill('0') << (int)byte;
    }
    return ss.str();
}

// --- Fonction de hachage ---
string ac_hash(const string& input, uint32_t rule, size_t steps) {
    vector<int> state = text_to_bits(input);

    // Ajuster à 256 bits (padding ou troncature)
    if (state.size() < 256) {
        while (state.size() < 256)
            state.push_back(state[state.size() % input.size()]);
    } else if (state.size() > 256) {
        state.resize(256);
    }

    // Évolution de l’automate
    for (size_t s = 0; s < steps; ++s) {
        state = evolve(state, rule);
        // léger mélange : rotation circulaire pour éviter les symétries
        rotate(state.begin(), state.begin() + (s % state.size()), state.end());
    }

    // Retour du hash en hexadécimal (32 octets = 256 bits)
    return bits_to_hex(state);
}

// --- evolve déjà défini précédemment ---
vector<int> evolve(const vector<int>& state, int rule_number) {
    int n = state.size();
    vector<int> next(n, 0);
    bitset<8> rule(rule_number);

    for (int i = 0; i < n; ++i) {
        int left   = (i == 0) ? 0 : state[i - 1];
        int center = state[i];
        int right  = (i == n - 1) ? 0 : state[i + 1];
        int index = (left << 2) | (center << 1) | right;
        next[i] = rule[index];
    }
    return next;
}

// --- Test ---
int main() {
    string h1 = ac_hash("Hello world", 110, 100);
    string h2 = ac_hash("Hello worle", 110, 100);

    cout << "Hash 1: " << h1 << endl;
    cout << "Hash 2: " << h2 << endl;

    if (h1 != h2)
        cout << "Test OK: the two inputs produce different hashes." << endl;
    else
        cout << "Test failed: the hashes are identical." << endl;

    return 0;
}
