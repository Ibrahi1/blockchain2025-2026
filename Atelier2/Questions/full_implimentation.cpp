#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <bitset>
#include <cmath>
#include <algorithm>

// ==================== AUTOMATE CELLULAIRE ====================

class CellularAutomaton {
private:
    std::vector<bool> state;
    uint32_t rule;
    
    // Applique la règle sur 3 bits pour obtenir le nouveau bit central
    bool apply_rule(bool left, bool center, bool right) {
        int index = (left << 2) | (center << 1) | right;
        return (rule >> index) & 1;
    }
    
public:
    CellularAutomaton(uint32_t r = 30) : rule(r) {}
    
    // 1.1. Initialiser l'état à partir d'un vecteur de bits
    void init_state(const std::vector<bool>& initial_state) {
        state = initial_state;
    }
    
    // 1.2. Faire évoluer l'automate d'un pas
    void evolve() {
        if (state.empty()) return;
        
        std::vector<bool> new_state(state.size());
        
        for (size_t i = 0; i < state.size(); i++) {
            bool left = (i == 0) ? false : state[i - 1];
            bool center = state[i];
            bool right = (i == state.size() - 1) ? false : state[i + 1];
            
            new_state[i] = apply_rule(left, center, right);
        }
        
        state = new_state;
    }
    
    const std::vector<bool>& get_state() const {
        return state;
    }
    
    void set_rule(uint32_t r) {
        rule = r;
    }
};

// ==================== FONCTION DE HACHAGE AC ====================

// 2.1. Fonction de hachage basée sur l'automate cellulaire
std::string ac_hash(const std::string& input, uint32_t rule, size_t steps) {
    // 2.2. Conversion du texte en bits
    std::vector<bool> bits;
    
    // Convertir chaque caractère en 8 bits
    for (char c : input) {
        for (int i = 7; i >= 0; i--) {
            bits.push_back((c >> i) & 1);
        }
    }
    
    // Padding pour avoir au moins 256 bits
    while (bits.size() < 256) {
        bits.push_back(false);
    }
    
    // 2.3. Processus de production du hash de 256 bits
    CellularAutomaton ca(rule);
    ca.init_state(bits);
    
    // Faire évoluer l'automate
    for (size_t i = 0; i < steps; i++) {
        ca.evolve();
    }
    
    // Extraire les 256 premiers bits et les convertir en hexadécimal
    const auto& final_state = ca.get_state();
    std::stringstream ss;
    
    for (size_t i = 0; i < 256 && i < final_state.size(); i += 8) {
        uint8_t byte = 0;
        for (int j = 0; j < 8 && (i + j) < final_state.size(); j++) {
            if (final_state[i + j]) {
                byte |= (1 << (7 - j));
            }
        }
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    }
    
    return ss.str();
}

// Version simple de SHA256 (simulée pour comparaison)
std::string simple_sha256(const std::string& input) {
    // Simulation simplifiée - dans un vrai projet, utiliser OpenSSL ou crypto++
    std::hash<std::string> hasher;
    size_t hash_value = hasher(input);
    
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hash_value;
    
    // Compléter à 64 caractères (256 bits)
    std::string result = ss.str();
    while (result.length() < 64) {
        result += "0";
    }
    return result.substr(0, 64);
}

// ==================== BLOCKCHAIN ====================

struct Block {
    int index;
    std::time_t timestamp;
    std::string data;
    std::string previous_hash;
    int nonce;
    std::string hash;
    
    Block(int idx, const std::string& d, const std::string& prev_hash)
        : index(idx), data(d), previous_hash(prev_hash), nonce(0) {
        timestamp = std::time(nullptr);
    }
};

class Blockchain {
private:
    std::vector<Block> chain;
    int difficulty;
    std::string hash_mode; // "SHA256" ou "AC_HASH"
    uint32_t ac_rule;
    size_t ac_steps;
    
    std::string calculate_hash(const Block& block) {
        std::stringstream ss;
        ss << block.index << block.timestamp << block.data 
           << block.previous_hash << block.nonce;
        
        if (hash_mode == "AC_HASH") {
            return ac_hash(ss.str(), ac_rule, ac_steps);
        } else {
            return simple_sha256(ss.str());
        }
    }
    
    std::string get_difficulty_prefix() {
        return std::string(difficulty, '0');
    }
    
public:
    Blockchain(int diff = 2, const std::string& mode = "SHA256", 
               uint32_t rule = 30, size_t steps = 128)
        : difficulty(diff), hash_mode(mode), ac_rule(rule), ac_steps(steps) {
        // Créer le bloc genesis
        Block genesis(0, "Genesis Block", "0");
        genesis.hash = calculate_hash(genesis);
        chain.push_back(genesis);
    }
    
    // 3.2. Minage avec ac_hash
    void mine_block(Block& block, int& iterations) {
        std::string target = get_difficulty_prefix();
        iterations = 0;
        
        do {
            block.nonce++;
            block.hash = calculate_hash(block);
            iterations++;
        } while (block.hash.substr(0, difficulty) != target);
    }
    
    void add_block(const std::string& data, int& iterations) {
        Block new_block(chain.size(), data, chain.back().hash);
        mine_block(new_block, iterations);
        chain.push_back(new_block);
    }
    
    // 3.3. Validation de bloc
    bool is_chain_valid() {
        for (size_t i = 1; i < chain.size(); i++) {
            const Block& current = chain[i];
            const Block& previous = chain[i - 1];
            
            // Vérifier le hash
            if (current.hash != calculate_hash(current)) {
                return false;
            }
            
            // Vérifier le lien avec le bloc précédent
            if (current.previous_hash != previous.hash) {
                return false;
            }
            
            // Vérifier la difficulté
            if (current.hash.substr(0, difficulty) != get_difficulty_prefix()) {
                return false;
            }
        }
        return true;
    }
    
    void set_hash_mode(const std::string& mode, uint32_t rule = 30, size_t steps = 128) {
        hash_mode = mode;
        ac_rule = rule;
        ac_steps = steps;
    }
    
    void print_chain() {
        for (const auto& block : chain) {
            std::cout << "Block #" << block.index << "\n";
            std::cout << "  Timestamp: " << block.timestamp << "\n";
            std::cout << "  Data: " << block.data << "\n";
            std::cout << "  Hash: " << block.hash << "\n";
            std::cout << "  Previous: " << block.previous_hash << "\n";
            std::cout << "  Nonce: " << block.nonce << "\n\n";
        }
    }
};

// ==================== TESTS ET ANALYSES ====================

// 5. Test effet avalanche
double test_avalanche_effect(uint32_t rule, size_t steps, int num_tests = 100) {
    double total_diff_percentage = 0.0;
    
    for (int t = 0; t < num_tests; t++) {
        std::string msg1 = "test_message_" + std::to_string(t);
        std::string msg2 = msg1;
        
        // Changer un seul bit
        if (!msg2.empty()) {
            msg2[0] ^= 1;
        }
        
        std::string hash1 = ac_hash(msg1, rule, steps);
        std::string hash2 = ac_hash(msg2, rule, steps);
        
        int diff_bits = 0;
        for (size_t i = 0; i < std::min(hash1.length(), hash2.length()); i++) {
            if (hash1[i] != hash2[i]) {
                // Compter les bits différents dans chaque caractère hex
                int val1 = (hash1[i] >= 'a') ? (hash1[i] - 'a' + 10) : (hash1[i] - '0');
                int val2 = (hash2[i] >= 'a') ? (hash2[i] - 'a' + 10) : (hash2[i] - '0');
                int xor_val = val1 ^ val2;
                diff_bits += __builtin_popcount(xor_val);
            }
        }
        
        double percentage = (diff_bits * 100.0) / 256.0;
        total_diff_percentage += percentage;
    }
    
    return total_diff_percentage / num_tests;
}

// 6. Test distribution des bits
double test_bit_distribution(uint32_t rule, size_t steps, int num_samples = 1000) {
    int total_ones = 0;
    int total_bits = 0;
    
    for (int i = 0; i < num_samples; i++) {
        std::string input = "sample_" + std::to_string(i);
        std::string hash = ac_hash(input, rule, steps);
        
        for (char c : hash) {
            int val = (c >= 'a') ? (c - 'a' + 10) : (c >= 'A') ? (c - 'A' + 10) : (c - '0');
            total_ones += __builtin_popcount(val);
            total_bits += 4;
        }
    }
    
    return (total_ones * 100.0) / total_bits;
}

// ==================== MAIN ====================

int main() {
    std::cout << "=== ATELIER 2: AUTOMATE CELLULAIRE ET BLOCKCHAIN ===\n\n";
    
    // 1.3. Test de l'automate cellulaire
    std::cout << "1. Test de l'automate cellulaire (Rule 30):\n";
    CellularAutomaton ca(30);
    std::vector<bool> init_state = {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0};
    ca.init_state(init_state);
    
    std::cout << "État initial: ";
    for (bool b : ca.get_state()) std::cout << b;
    std::cout << "\n";
    
    for (int i = 0; i < 5; i++) {
        ca.evolve();
        std::cout << "Génération " << (i+1) << ": ";
        for (bool b : ca.get_state()) std::cout << b;
        std::cout << "\n";
    }
    
    // 2.4. Test que deux entrées différentes donnent des sorties différentes
    std::cout << "\n2. Test de hachage AC:\n";
    std::string hash1 = ac_hash("Hello World", 30, 128);
    std::string hash2 = ac_hash("Hello World!", 30, 128);
    std::cout << "Hash('Hello World'): " << hash1 << "\n";
    std::cout << "Hash('Hello World!'): " << hash2 << "\n";
    std::cout << "Hashes différents: " << (hash1 != hash2 ? "OUI" : "NON") << "\n";
    
    // 4. Comparaison SHA256 vs AC_HASH
    std::cout << "\n4. Comparaison des performances:\n";
    std::cout << "Mode\t\tRule\tTemps(ms)\tItérations moy.\n";
    std::cout << "--------------------------------------------------------\n";
    
    std::vector<std::pair<std::string, uint32_t>> modes = {
        {"SHA256", 0}, {"AC_HASH", 30}, {"AC_HASH", 90}, {"AC_HASH", 110}
    };
    
    for (auto& [mode, rule] : modes) {
        Blockchain bc(2, mode, rule, 128);
        
        auto start = std::chrono::high_resolution_clock::now();
        int total_iterations = 0;
        
        for (int i = 0; i < 10; i++) {
            int iterations;
            bc.add_block("Block " + std::to_string(i), iterations);
            total_iterations += iterations;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << mode << "\t";
        if (mode == "AC_HASH") std::cout << rule << "\t";
        else std::cout << "N/A\t";
        std::cout << duration.count() << "\t\t" 
                  << (total_iterations / 10) << "\n";
    }
    
    // 5. Effet avalanche
    std::cout << "\n5. Effet avalanche:\n";
    std::cout << "Rule\tPourcentage de bits différents\n";
    std::cout << "----------------------------------------\n";
    for (uint32_t rule : {30, 90, 110}) {
        double avalanche = test_avalanche_effect(rule, 128, 50);
        std::cout << rule << "\t" << std::fixed << std::setprecision(2) 
                  << avalanche << "%\n";
    }
    
    // 6. Distribution des bits
    std::cout << "\n6. Distribution des bits:\n";
    std::cout << "Rule\tPourcentage de 1\tÉquilibré?\n";
    std::cout << "----------------------------------------\n";
    for (uint32_t rule : {30, 90, 110}) {
        double dist = test_bit_distribution(rule, 128, 200);
        std::cout << rule << "\t" << std::fixed << std::setprecision(2) 
                  << dist << "%\t\t" 
                  << (abs(dist - 50.0) < 5.0 ? "OUI" : "NON") << "\n";
    }
    
    // 3.3. Validation de la blockchain
    std::cout << "\n3. Validation de la blockchain avec AC_HASH:\n";
    Blockchain blockchain(2, "AC_HASH", 30, 128);
    int iter;
    blockchain.add_block("Transaction 1", iter);
    blockchain.add_block("Transaction 2", iter);
    std::cout << "Blockchain valide: " << (blockchain.is_chain_valid() ? "OUI" : "NON") << "\n";
    
    std::cout << "\n=== TESTS TERMINÉS ===\n";
    
    return 0;
}