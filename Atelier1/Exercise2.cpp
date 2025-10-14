#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <ctime>
#include <chrono>
#include <cmath>

// Fonction de hachage SHA-256 simplifiée
class SHA256 {
public:
    static std::string hash(const std::string& input) {
        uint64_t h1 = 0xcbf29ce484222325ULL;
        uint64_t h2 = 0x6a09e667bb67ae85ULL;
        
        for (char c : input) {
            h1 = (h1 ^ static_cast<uint64_t>(c)) * 0x100000001b3ULL;
            h2 = (h2 * 33) ^ static_cast<uint64_t>(c);
        }
        
        std::stringstream ss;
        ss << std::hex << std::setfill('0') 
           << std::setw(16) << h1 
           << std::setw(16) << h2;
        return ss.str();
    }
};

// Classe représentant un bloc de la blockchain
class Block {
private:
    int index;
    std::string timestamp;
    std::vector<std::string> transactions;
    std::string previousHash;
    std::string hash;
    int nonce;
    int difficulty;
    
    // Génère un timestamp
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::ctime(&now_time);
        return ss.str();
    }
    
    // Calcule le hash du bloc
    std::string calculateHash() const {
        std::stringstream ss;
        ss << index << timestamp << previousHash << nonce;
        
        for (const auto& tx : transactions) {
            ss << tx;
        }
        
        return SHA256::hash(ss.str());
    }
    
public:
    Block(int idx, const std::vector<std::string>& txs, const std::string& prevHash, int diff = 2)
        : index(idx), transactions(txs), previousHash(prevHash), nonce(0), difficulty(diff) {
        timestamp = getCurrentTimestamp();
        hash = calculateHash();
    }
    
    // Proof of Work - Mine le bloc
    void mineBlock() {
        std::string target(difficulty, '0');
        auto start = std::chrono::high_resolution_clock::now();
        
        std::cout << "🔨 Mining block " << index << " avec difficulté " << difficulty 
                  << " (hash doit commencer par " << target << ")..." << std::endl;
        
        nonce = 0;
        hash = calculateHash();
        
        while (hash.substr(0, difficulty) != target) {
            nonce++;
            hash = calculateHash();
            
            // Afficher la progression tous les 100000 essais
            if (nonce % 100000 == 0) {
                std::cout << "  Essai #" << nonce << " - Hash: " << hash.substr(0, 10) << "..." << std::endl;
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "✅ Bloc miné! Nonce: " << nonce << std::endl;
        std::cout << "⏱️  Temps d'exécution: " << duration.count() << " ms" << std::endl;
        std::cout << "🔐 Hash: " << hash << std::endl;
        std::cout << std::endl;
    }
    
    // Getters
    std::string getHash() const { return hash; }
    std::string getPreviousHash() const { return previousHash; }
    int getIndex() const { return index; }
    int getNonce() const { return nonce; }
    int getDifficulty() const { return difficulty; }
    
    // Affiche les informations du bloc
    void display() const {
        std::cout << "╔════════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║ BLOC #" << std::setw(52) << std::left << index << "║" << std::endl;
        std::cout << "╠════════════════════════════════════════════════════════════╣" << std::endl;
        std::cout << "║ Timestamp: " << std::setw(47) << std::left << timestamp.substr(0, 47) << "║" << std::endl;
        std::cout << "║ Transactions: " << std::setw(44) << std::left << std::to_string(transactions.size()) << "║" << std::endl;
        
        for (size_t i = 0; i < transactions.size() && i < 3; i++) {
            std::string tx = transactions[i].substr(0, 50);
            std::cout << "║   - " << std::setw(53) << std::left << tx << "║" << std::endl;
        }
        
        std::cout << "║ Nonce: " << std::setw(51) << std::left << nonce << "║" << std::endl;
        std::cout << "║ Difficulté: " << std::setw(46) << std::left << difficulty << "║" << std::endl;
        std::cout << "║ Hash précédent: " << std::setw(42) << std::left << previousHash.substr(0, 42) << "║" << std::endl;
        std::cout << "║ Hash: " << std::setw(52) << std::left << hash.substr(0, 52) << "║" << std::endl;
        std::cout << "╚════════════════════════════════════════════════════════════╝" << std::endl;
    }
    
    // Vérifie la validité du bloc
    bool isValid() const {
        std::string target(difficulty, '0');
        return hash.substr(0, difficulty) == target && hash == calculateHash();
    }
};

// Classe représentant la blockchain
class Blockchain {
private:
    std::vector<Block*> chain;
    int difficulty;
    
public:
    Blockchain(int diff = 2) : difficulty(diff) {
        // Créer le bloc genesis
        std::vector<std::string> genesisTxs = {"Genesis Block - First Block"};
        Block* genesis = new Block(0, genesisTxs, "0", difficulty);
        genesis->mineBlock();
        chain.push_back(genesis);
    }
    
    ~Blockchain() {
        for (auto block : chain) {
            delete block;
        }
    }
    
    // Ajoute un nouveau bloc à la chaîne
    void addBlock(const std::vector<std::string>& transactions) {
        std::string previousHash = chain.back()->getHash();
        int index = chain.size();
        
        Block* newBlock = new Block(index, transactions, previousHash, difficulty);
        newBlock->mineBlock();
        chain.push_back(newBlock);
    }
    
    // Vérifie l'intégrité de la blockchain
    bool isChainValid() const {
        for (size_t i = 1; i < chain.size(); i++) {
            const Block* currentBlock = chain[i];
            const Block* previousBlock = chain[i - 1];
            
            // Vérifier le hash du bloc actuel
            if (!currentBlock->isValid()) {
                std::cout << "❌ Bloc #" << currentBlock->getIndex() << " invalide!" << std::endl;
                return false;
            }
            
            // Vérifier le lien avec le bloc précédent
            if (currentBlock->getPreviousHash() != previousBlock->getHash()) {
                std::cout << "❌ Chaîne brisée au bloc #" << currentBlock->getIndex() << "!" << std::endl;
                return false;
            }
        }
        return true;
    }
    
    // Affiche toute la blockchain
    void display() const {
        std::cout << "\n╔══════════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║              BLOCKCHAIN - " << chain.size() << " BLOCS                         ║" << std::endl;
        std::cout << "╚══════════════════════════════════════════════════════════════╝\n" << std::endl;
        
        for (const auto& block : chain) {
            block->display();
            std::cout << std::endl;
        }
    }
    
    // Change la difficulté
    void setDifficulty(int diff) {
        difficulty = diff;
        std::cout << "⚙️  Difficulté changée à: " << difficulty << std::endl;
    }
    
    int getDifficulty() const { return difficulty; }
    int getSize() const { return chain.size(); }
};

// Fonction pour tester différents niveaux de difficulté
void testDifficultyLevels() {
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║        TEST DES NIVEAUX DE DIFFICULTÉ                        ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n" << std::endl;
    
    std::vector<int> difficulties = {1, 2, 3, 4, 5};
    std::vector<long long> times;
    
    for (int diff : difficulties) {
        std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        std::cout << "   DIFFICULTÉ: " << diff << std::endl;
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        Blockchain testChain(diff);
        std::vector<std::string> txs = {"Test transaction pour difficulté " + std::to_string(diff)};
        testChain.addBlock(txs);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        times.push_back(duration.count());
    }
    
    // Afficher le tableau récapitulatif
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║           RÉSUMÉ DES TEMPS D'EXÉCUTION                       ║" << std::endl;
    std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
    std::cout << "║ Difficulté │ Temps (ms) │ Complexité approximative          ║" << std::endl;
    std::cout << "╠════════════╪════════════╪═══════════════════════════════════╣" << std::endl;
    
    for (size_t i = 0; i < difficulties.size(); i++) {
        std::cout << "║     " << std::setw(2) << difficulties[i] 
                  << "     │ " << std::setw(10) << std::right << times[i]
                  << " │ ~16^" << difficulties[i] << " = " << std::setw(11) << std::left
                  << static_cast<long long>(pow(16, difficulties[i])) << " possibilités ║" << std::endl;
    }
    
    std::cout << "╚════════════╧════════════╧═══════════════════════════════════╝" << std::endl;
}

// Programme principal
int main() {
    std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║         IMPLÉMENTATION PROOF OF WORK - BLOCKCHAIN            ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
    
    // EXEMPLE 1: Création d'une blockchain simple
    std::cout << "\n\n>>> EXEMPLE 1: Création d'une blockchain avec difficulté 3 <<<\n" << std::endl;
    
    Blockchain blockchain(3);
    
    // Ajouter des blocs avec des transactions
    std::vector<std::string> tx1 = {
        "Alice envoie 50 BTC à Bob",
        "Bob envoie 20 BTC à Charlie"
    };
    blockchain.addBlock(tx1);
    
    std::vector<std::string> tx2 = {
        "Charlie envoie 10 BTC à David",
        "David envoie 5 BTC à Eve"
    };
    blockchain.addBlock(tx2);
    
    std::vector<std::string> tx3 = {
        "Eve envoie 3 BTC à Alice"
    };
    blockchain.addBlock(tx3);
    
    // Afficher la blockchain
    blockchain.display();
    
    // Vérifier l'intégrité
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    std::cout << "   VÉRIFICATION DE L'INTÉGRITÉ" << std::endl;
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << std::endl;
    
    if (blockchain.isChainValid()) {
        std::cout << "✅ La blockchain est VALIDE!" << std::endl;
    } else {
        std::cout << "❌ La blockchain est INVALIDE!" << std::endl;
    }
    
    // EXEMPLE 2: Test des différents niveaux de difficulté
    std::cout << "\n\n>>> EXEMPLE 2: Comparaison des niveaux de difficulté <<<" << std::endl;
    testDifficultyLevels();
    
    // EXEMPLE 3: Démonstration de sécurité
    std::cout << "\n\n>>> EXEMPLE 3: Démonstration de la sécurité (Proof of Work) <<<\n" << std::endl;
    
    std::cout << "📝 Concept du Proof of Work:" << std::endl;
    std::cout << "   - Plus la difficulté est élevée, plus il faut de calculs" << std::endl;
    std::cout << "   - Un attaquant devrait refaire tous les calculs pour modifier un bloc" << std::endl;
    std::cout << "   - La blockchain devient exponentiellement plus sûre avec le temps" << std::endl;
    std::cout << "\n   Difficulté 1: ~16 possibilités" << std::endl;
    std::cout << "   Difficulté 2: ~256 possibilités" << std::endl;
    std::cout << "   Difficulté 3: ~4,096 possibilités" << std::endl;
    std::cout << "   Difficulté 4: ~65,536 possibilités" << std::endl;
    std::cout << "   Difficulté 5: ~1,048,576 possibilités" << std::endl;
    std::cout << "\n   Bitcoin utilise une difficulté de ~19-20 en 2024!" << std::endl;
    
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                    FIN DES EXEMPLES                          ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
    
    return 0;
}