#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <ctime>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <thread>

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

// Classe représentant un validateur (pour PoS)
class Validator {
private:
    std::string address;
    double stake;
    int blocksValidated;
    
public:
    Validator(const std::string& addr, double stk) 
        : address(addr), stake(stk), blocksValidated(0) {}
    
    std::string getAddress() const { return address; }
    double getStake() const { return stake; }
    int getBlocksValidated() const { return blocksValidated; }
    
    void addStake(double amount) { stake += amount; }
    void incrementBlocksValidated() { blocksValidated++; }
    
    void display() const {
        std::cout << "  👤 " << address << " - Stake: " << std::fixed 
                  << std::setprecision(2) << stake << " coins - Blocs validés: " 
                  << blocksValidated << std::endl;
    }
};

// Classe de base pour les blocs
class BaseBlock {
protected:
    int index;
    std::string timestamp;
    std::vector<std::string> transactions;
    std::string previousHash;
    std::string hash;
    
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::ctime(&now_time);
        std::string ts = ss.str();
        if (!ts.empty() && ts.back() == '\n') ts.pop_back();
        return ts;
    }
    
public:
    BaseBlock(int idx, const std::vector<std::string>& txs, const std::string& prevHash)
        : index(idx), transactions(txs), previousHash(prevHash) {
        timestamp = getCurrentTimestamp();
    }
    
    virtual ~BaseBlock() {}
    
    int getIndex() const { return index; }
    std::string getHash() const { return hash; }
    std::string getPreviousHash() const { return previousHash; }
    std::string getTimestamp() const { return timestamp; }
    
    virtual void display() const = 0;
    virtual std::string getConsensusType() const = 0;
};

// Bloc Proof of Work
class PoWBlock : public BaseBlock {
private:
    int nonce;
    int difficulty;
    
    std::string calculateHash() const {
        std::stringstream ss;
        ss << index << timestamp << previousHash << nonce;
        for (const auto& tx : transactions) {
            ss << tx;
        }
        return SHA256::hash(ss.str());
    }
    
public:
    PoWBlock(int idx, const std::vector<std::string>& txs, const std::string& prevHash, int diff)
        : BaseBlock(idx, txs, prevHash), nonce(0), difficulty(diff) {
        hash = calculateHash();
    }
    
    long long mineBlock() {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::string target(difficulty, '0');
        nonce = 0;
        hash = calculateHash();
        
        while (hash.substr(0, difficulty) != target) {
            nonce++;
            hash = calculateHash();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }
    
    void display() const override {
        std::cout << "╔════════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║ BLOC PoW #" << std::setw(47) << std::left << index << "║" << std::endl;
        std::cout << "╠════════════════════════════════════════════════════════════╣" << std::endl;
        std::cout << "║ Timestamp: " << std::setw(47) << std::left << timestamp.substr(0, 47) << "║" << std::endl;
        std::cout << "║ Nonce: " << std::setw(51) << std::left << nonce << "║" << std::endl;
        std::cout << "║ Difficulté: " << std::setw(46) << std::left << difficulty << "║" << std::endl;
        std::cout << "║ Hash: " << std::setw(52) << std::left << hash.substr(0, 52) << "║" << std::endl;
        std::cout << "╚════════════════════════════════════════════════════════════╝" << std::endl;
    }
    
    std::string getConsensusType() const override { return "Proof of Work"; }
    int getNonce() const { return nonce; }
};

// Bloc Proof of Stake
class PoSBlock : public BaseBlock {
private:
    std::string validator;
    double validatorStake;
    
    std::string calculateHash() const {
        std::stringstream ss;
        ss << index << timestamp << previousHash << validator;
        for (const auto& tx : transactions) {
            ss << tx;
        }
        return SHA256::hash(ss.str());
    }
    
public:
    PoSBlock(int idx, const std::vector<std::string>& txs, const std::string& prevHash, 
             const std::string& val, double stake)
        : BaseBlock(idx, txs, prevHash), validator(val), validatorStake(stake) {
        hash = calculateHash();
    }
    
    void display() const override {
        std::cout << "╔════════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║ BLOC PoS #" << std::setw(47) << std::left << index << "║" << std::endl;
        std::cout << "╠════════════════════════════════════════════════════════════╣" << std::endl;
        std::cout << "║ Timestamp: " << std::setw(47) << std::left << timestamp.substr(0, 47) << "║" << std::endl;
        std::cout << "║ Validateur: " << std::setw(46) << std::left << validator.substr(0, 46) << "║" << std::endl;
        std::cout << "║ Stake: " << std::setw(51) << std::left << validatorStake << "║" << std::endl;
        std::cout << "║ Hash: " << std::setw(52) << std::left << hash.substr(0, 52) << "║" << std::endl;
        std::cout << "╚════════════════════════════════════════════════════════════╝" << std::endl;
    }
    
    std::string getConsensusType() const override { return "Proof of Stake"; }
    std::string getValidator() const { return validator; }
};

// Blockchain Proof of Work
class PoWBlockchain {
private:
    std::vector<PoWBlock*> chain;
    int difficulty;
    
public:
    PoWBlockchain(int diff) : difficulty(diff) {
        std::vector<std::string> genesisTxs = {"Genesis Block PoW"};
        PoWBlock* genesis = new PoWBlock(0, genesisTxs, "0", difficulty);
        genesis->mineBlock();
        chain.push_back(genesis);
    }
    
    ~PoWBlockchain() {
        for (auto block : chain) {
            delete block;
        }
    }
    
    long long addBlock(const std::vector<std::string>& transactions) {
        std::string previousHash = chain.back()->getHash();
        int index = chain.size();
        
        PoWBlock* newBlock = new PoWBlock(index, transactions, previousHash, difficulty);
        long long miningTime = newBlock->mineBlock();
        chain.push_back(newBlock);
        
        return miningTime;
    }
    
    void display() const {
        std::cout << "\n🔨 BLOCKCHAIN PROOF OF WORK - " << chain.size() << " blocs\n" << std::endl;
        for (const auto& block : chain) {
            block->display();
            std::cout << std::endl;
        }
    }
    
    int getSize() const { return chain.size(); }
};

// Blockchain Proof of Stake
class PoSBlockchain {
private:
    std::vector<PoSBlock*> chain;
    std::vector<Validator> validators;
    
    // Sélectionne un validateur basé sur le stake (weighted random)
    Validator* selectValidator() {
        if (validators.empty()) return nullptr;
        
        double totalStake = 0;
        for (const auto& v : validators) {
            totalStake += v.getStake();
        }
        
        double random = (static_cast<double>(rand()) / RAND_MAX) * totalStake;
        double cumulative = 0;
        
        for (auto& v : validators) {
            cumulative += v.getStake();
            if (random <= cumulative) {
                return &v;
            }
        }
        
        return &validators[0];
    }
    
public:
    PoSBlockchain() {
        std::vector<std::string> genesisTxs = {"Genesis Block PoS"};
        PoSBlock* genesis = new PoSBlock(0, genesisTxs, "0", "Genesis", 0);
        chain.push_back(genesis);
    }
    
    ~PoSBlockchain() {
        for (auto block : chain) {
            delete block;
        }
    }
    
    void addValidator(const std::string& address, double stake) {
        validators.push_back(Validator(address, stake));
    }
    
    long long addBlock(const std::vector<std::string>& transactions) {
        auto start = std::chrono::high_resolution_clock::now();
        
        Validator* selected = selectValidator();
        if (selected == nullptr) {
            std::cout << "❌ Aucun validateur disponible!" << std::endl;
            return 0;
        }
        
        std::string previousHash = chain.back()->getHash();
        int index = chain.size();
        
        PoSBlock* newBlock = new PoSBlock(index, transactions, previousHash, 
                                          selected->getAddress(), selected->getStake());
        
        // Simuler une courte validation (vérification des transactions)
        // En PoS, pas de mining intensif, juste validation
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        chain.push_back(newBlock);
        selected->incrementBlocksValidated();
        
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }
    
    void display() const {
        std::cout << "\n💎 BLOCKCHAIN PROOF OF STAKE - " << chain.size() << " blocs\n" << std::endl;
        for (const auto& block : chain) {
            block->display();
            std::cout << std::endl;
        }
    }
    
    void displayValidators() const {
        std::cout << "\n👥 VALIDATEURS:\n" << std::endl;
        for (const auto& v : validators) {
            v.display();
        }
    }
    
    int getSize() const { return chain.size(); }
};

// Fonction de comparaison
void compareConsensus() {
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║            COMPARAISON: PoW vs PoS                           ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n" << std::endl;
    
    srand(time(nullptr));
    
    const int NUM_BLOCKS = 5;
    const int POW_DIFFICULTY = 4;
    
    // Test Proof of Work
    std::cout << "🔨 === TEST PROOF OF WORK (Difficulté " << POW_DIFFICULTY << ") ===" << std::endl;
    PoWBlockchain powChain(POW_DIFFICULTY);
    
    std::vector<long long> powTimes;
    auto powStart = std::chrono::high_resolution_clock::now();
    
    for (int i = 1; i <= NUM_BLOCKS; i++) {
        std::vector<std::string> txs = {"Transaction PoW #" + std::to_string(i)};
        std::cout << "\n🔨 Mining bloc #" << i << "..." << std::endl;
        long long time = powChain.addBlock(txs);
        powTimes.push_back(time);
        std::cout << "✅ Bloc miné en " << time / 1000.0 << " ms" << std::endl;
    }
    
    auto powEnd = std::chrono::high_resolution_clock::now();
    long long powTotal = std::chrono::duration_cast<std::chrono::milliseconds>(powEnd - powStart).count();
    
    // Test Proof of Stake
    std::cout << "\n\n💎 === TEST PROOF OF STAKE ===" << std::endl;
    PoSBlockchain posChain;
    
    // Ajouter des validateurs
    posChain.addValidator("Alice", 1000);
    posChain.addValidator("Bob", 500);
    posChain.addValidator("Charlie", 2000);
    posChain.addValidator("David", 750);
    
    posChain.displayValidators();
    
    std::vector<long long> posTimes;
    auto posStart = std::chrono::high_resolution_clock::now();
    
    for (int i = 1; i <= NUM_BLOCKS; i++) {
        std::vector<std::string> txs = {"Transaction PoS #" + std::to_string(i)};
        std::cout << "\n💎 Validation bloc #" << i << "..." << std::endl;
        long long time = posChain.addBlock(txs);
        posTimes.push_back(time);
        std::cout << "✅ Bloc validé en " << time / 1000.0 << " ms" << std::endl;
    }
    
    auto posEnd = std::chrono::high_resolution_clock::now();
    long long posTotal = std::chrono::duration_cast<std::chrono::milliseconds>(posEnd - posStart).count();
    
    // Afficher les résultats
    std::cout << "\n\n╔══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║              RÉSULTATS DE LA COMPARAISON                     ║" << std::endl;
    std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
    std::cout << "║                                                              ║" << std::endl;
    std::cout << "║  PROOF OF WORK:                                              ║" << std::endl;
    std::cout << "║    Temps total: " << std::setw(43) << std::left << (std::to_string(powTotal) + " ms") << "║" << std::endl;
    std::cout << "║    Temps moyen/bloc: " << std::setw(38) << std::left << (std::to_string(powTotal/NUM_BLOCKS) + " ms") << "║" << std::endl;
    std::cout << "║    Énergie: ⚡⚡⚡⚡⚡ (TRÈS ÉLEVÉE)                            ║" << std::endl;
    std::cout << "║                                                              ║" << std::endl;
    std::cout << "║  PROOF OF STAKE:                                             ║" << std::endl;
    std::cout << "║    Temps total: " << std::setw(43) << std::left << (std::to_string(posTotal) + " ms") << "║" << std::endl;
    std::cout << "║    Temps moyen/bloc: " << std::setw(38) << std::left << (std::to_string(posTotal/NUM_BLOCKS) + " ms") << "║" << std::endl;
    std::cout << "║    Énergie: ⚡ (TRÈS FAIBLE)                                 ║" << std::endl;
    std::cout << "║                                                              ║" << std::endl;
    std::cout << "╠══════════════════════════════════════════════════════════════╣" << std::endl;
    
    double speedup = static_cast<double>(powTotal) / posTotal;
    std::cout << "║  CONCLUSION:                                                 ║" << std::endl;
    std::cout << "║    PoS est " << std::setw(47) << std::left << (std::to_string(static_cast<int>(speedup)) + "x plus RAPIDE que PoW") << "║" << std::endl;
    std::cout << "║    PoS consomme ~99.9% MOINS d'énergie que PoW               ║" << std::endl;
    std::cout << "║                                                              ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
    
    // Détails par bloc
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║           TEMPS DÉTAILLÉ PAR BLOC (en ms)                    ║" << std::endl;
    std::cout << "╠══════════════╦═══════════════════╦═════════════════════════╣" << std::endl;
    std::cout << "║    Bloc      ║   PoW (mining)    ║   PoS (validation)      ║" << std::endl;
    std::cout << "╠══════════════╬═══════════════════╬═════════════════════════╣" << std::endl;
    
    for (int i = 0; i < NUM_BLOCKS; i++) {
        std::cout << "║      " << std::setw(2) << (i+1) << "      ║  " 
                  << std::setw(16) << std::right << (powTimes[i]/1000.0) << " ║  "
                  << std::setw(22) << std::right << (posTimes[i]/1000.0) << " ║" << std::endl;
    }
    
    std::cout << "╚══════════════╩═══════════════════╩═════════════════════════╝" << std::endl;
}

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║       PROOF OF STAKE vs PROOF OF WORK - BLOCKCHAIN          ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
    
    // EXEMPLE 1: Démonstration Proof of Stake
    std::cout << "\n\n>>> EXEMPLE 1: Blockchain Proof of Stake <<<\n" << std::endl;
    
    PoSBlockchain posBlockchain;
    
    // Ajouter des validateurs avec différents stakes
    posBlockchain.addValidator("Alice", 1000);
    posBlockchain.addValidator("Bob", 500);
    posBlockchain.addValidator("Charlie", 2000);
    posBlockchain.addValidator("David", 750);
    posBlockchain.addValidator("Eve", 1500);
    
    posBlockchain.displayValidators();
    
    std::cout << "\n📝 Ajout de blocs avec sélection pondérée des validateurs...\n" << std::endl;
    
    for (int i = 1; i <= 3; i++) {
        std::vector<std::string> txs = {
            "Transaction " + std::to_string(i) + "A",
            "Transaction " + std::to_string(i) + "B"
        };
        long long time = posBlockchain.addBlock(txs);
        std::cout << "✅ Bloc #" << i << " validé en " << time / 1000.0 << " ms" << std::endl;
    }
    
    posBlockchain.display();
    posBlockchain.displayValidators();
    
    // EXEMPLE 2: Comparaison PoW vs PoS
    std::cout << "\n\n>>> EXEMPLE 2: Comparaison Performance PoW vs PoS <<<" << std::endl;
    compareConsensus();
    
    // EXEMPLE 3: Explication des concepts
    std::cout << "\n\n>>> EXEMPLE 3: Différences Clés <<<\n" << std::endl;
    
    std::cout << "📊 PROOF OF WORK (PoW):" << std::endl;
    std::cout << "  ✓ Les mineurs résolvent des puzzles cryptographiques" << std::endl;
    std::cout << "  ✓ Nécessite une puissance de calcul énorme" << std::endl;
    std::cout << "  ✓ Très sécurisé mais LENT et énergivore" << std::endl;
    std::cout << "  ✓ Utilisé par: Bitcoin, Ethereum (avant)" << std::endl;
    std::cout << "  ✗ Consommation électrique massive" << std::endl;
    std::cout << "  ✗ Temps de validation long\n" << std::endl;
    
    std::cout << "💎 PROOF OF STAKE (PoS):" << std::endl;
    std::cout << "  ✓ Les validateurs sont choisis selon leur stake" << std::endl;
    std::cout << "  ✓ Pas de calculs intensifs" << std::endl;
    std::cout << "  ✓ RAPIDE et économe en énergie (~99.9% moins)" << std::endl;
    std::cout << "  ✓ Utilisé par: Ethereum 2.0, Cardano, Polkadot" << std::endl;
    std::cout << "  ✓ Validation en millisecondes vs minutes" << std::endl;
    std::cout << "  ✓ Plus écologique et scalable" << std::endl;
    
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                    FIN DES EXEMPLES                          ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════════╝" << std::endl;
    
    return 0;
}