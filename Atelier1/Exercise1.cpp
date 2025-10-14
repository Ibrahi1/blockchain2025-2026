#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cstdint>

// Implémentation basique de SHA-256 (version simplifiée pour la démonstration)
class SHA256 {
private:
    static const uint32_t K[64];
    
    static uint32_t rotr(uint32_t x, uint32_t n) {
        return (x >> n) | (x << (32 - n));
    }
    
    static uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
        return (x & y) ^ (~x & z);
    }
    
    static uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
        return (x & y) ^ (x & z) ^ (y & z);
    }
    
    static uint32_t sig0(uint32_t x) {
        return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
    }
    
    static uint32_t sig1(uint32_t x) {
        return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
    }
    
public:
    static std::string hash(const std::string& input) {
        // Pour simplifier, utilisons une fonction de hachage basique
        // En production, utilisez une vraie implémentation SHA-256
        uint32_t h = 0x6a09e667;
        
        for (char c : input) {
            h = ((h << 5) + h) + static_cast<uint32_t>(c);
            h ^= (h >> 16);
        }
        
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(8) << h;
        return ss.str();
    }
};

const uint32_t SHA256::K[64] = {0}; // Simplifié pour l'exemple

// Classe représentant un nœud de l'arbre de Merkle
class MerkleNode {
public:
    std::string hash;
    MerkleNode* left;
    MerkleNode* right;
    
    MerkleNode(const std::string& h) : hash(h), left(nullptr), right(nullptr) {}
    
    ~MerkleNode() {
        delete left;
        delete right;
    }
};

// Classe principale de l'arbre de Merkle
class MerkleTree {
private:
    MerkleNode* root;
    std::vector<std::string> leaves;
    
    // Crée un hash à partir de deux hashes enfants
    std::string combineHashes(const std::string& left, const std::string& right) {
        return SHA256::hash(left + right);
    }
    
    // Construit l'arbre récursivement
    MerkleNode* buildTree(std::vector<std::string>& hashes) {
        if (hashes.empty()) return nullptr;
        
        if (hashes.size() == 1) {
            return new MerkleNode(hashes[0]);
        }
        
        std::vector<std::string> parentHashes;
        
        // Combiner les hashes par paires
        for (size_t i = 0; i < hashes.size(); i += 2) {
            if (i + 1 < hashes.size()) {
                // Deux enfants
                std::string combined = combineHashes(hashes[i], hashes[i + 1]);
                parentHashes.push_back(combined);
            } else {
                // Nombre impair, dupliquer le dernier
                std::string combined = combineHashes(hashes[i], hashes[i]);
                parentHashes.push_back(combined);
            }
        }
        
        // Construire le nœud actuel
        MerkleNode* node = buildTree(parentHashes);
        return node;
    }
    
    // Construit l'arbre avec les nœuds intermédiaires
    MerkleNode* buildTreeWithNodes(std::vector<MerkleNode*>& nodes) {
        if (nodes.size() == 1) {
            return nodes[0];
        }
        
        std::vector<MerkleNode*> parentNodes;
        
        for (size_t i = 0; i < nodes.size(); i += 2) {
            MerkleNode* left = nodes[i];
            MerkleNode* right = (i + 1 < nodes.size()) ? nodes[i + 1] : nodes[i];
            
            std::string parentHash = combineHashes(left->hash, right->hash);
            MerkleNode* parent = new MerkleNode(parentHash);
            parent->left = left;
            parent->right = (i + 1 < nodes.size()) ? right : nullptr;
            
            parentNodes.push_back(parent);
        }
        
        return buildTreeWithNodes(parentNodes);
    }
    
    // Affiche l'arbre récursivement
    void printTree(MerkleNode* node, int level, const std::string& prefix) {
        if (node == nullptr) return;
        
        std::cout << prefix;
        std::cout << (level == 0 ? "Root: " : "├── ");
        std::cout << node->hash << std::endl;
        
        if (node->left != nullptr || node->right != nullptr) {
            if (node->left != nullptr) {
                printTree(node->left, level + 1, prefix + "│   ");
            }
            if (node->right != nullptr && node->right != node->left) {
                printTree(node->right, level + 1, prefix + "    ");
            }
        }
    }
    
public:
    MerkleTree() : root(nullptr) {}
    
    ~MerkleTree() {
        delete root;
    }
    
    // Construit l'arbre à partir des données
    void build(const std::vector<std::string>& data) {
        if (data.empty()) {
            std::cout << "Erreur: Aucune donnée fournie" << std::endl;
            return;
        }
        
        leaves.clear();
        std::vector<MerkleNode*> leafNodes;
        
        // Créer les feuilles (hasher chaque donnée)
        for (const auto& item : data) {
            std::string leafHash = SHA256::hash(item);
            leaves.push_back(leafHash);
            leafNodes.push_back(new MerkleNode(leafHash));
        }
        
        // Construire l'arbre
        root = buildTreeWithNodes(leafNodes);
        
        std::cout << "Arbre de Merkle construit avec " << data.size() << " éléments" << std::endl;
    }
    
    // Retourne la racine de Merkle
    std::string getRootHash() {
        if (root == nullptr) return "";
        return root->hash;
    }
    
    // Vérifie si une donnée existe dans l'arbre
    bool verify(const std::string& data) {
        std::string dataHash = SHA256::hash(data);
        for (const auto& leaf : leaves) {
            if (leaf == dataHash) {
                return true;
            }
        }
        return false;
    }
    
    // Obtient le chemin de preuve pour une donnée
    std::vector<std::string> getProof(const std::string& data) {
        std::vector<std::string> proof;
        std::string dataHash = SHA256::hash(data);
        
        // Trouver l'index de la feuille
        int index = -1;
        for (size_t i = 0; i < leaves.size(); i++) {
            if (leaves[i] == dataHash) {
                index = i;
                break;
            }
        }
        
        if (index == -1) {
            std::cout << "Donnée non trouvée dans l'arbre" << std::endl;
            return proof;
        }
        
        // Construire le chemin de preuve
        std::vector<std::string> currentLevel = leaves;
        int currentIndex = index;
        
        while (currentLevel.size() > 1) {
            if (currentIndex % 2 == 0) {
                // Nœud gauche, ajouter le frère droit
                if (currentIndex + 1 < currentLevel.size()) {
                    proof.push_back("R:" + currentLevel[currentIndex + 1]);
                } else {
                    proof.push_back("R:" + currentLevel[currentIndex]);
                }
            } else {
                // Nœud droit, ajouter le frère gauche
                proof.push_back("L:" + currentLevel[currentIndex - 1]);
            }
            
            // Passer au niveau suivant
            std::vector<std::string> nextLevel;
            for (size_t i = 0; i < currentLevel.size(); i += 2) {
                if (i + 1 < currentLevel.size()) {
                    nextLevel.push_back(combineHashes(currentLevel[i], currentLevel[i + 1]));
                } else {
                    nextLevel.push_back(combineHashes(currentLevel[i], currentLevel[i]));
                }
            }
            currentLevel = nextLevel;
            currentIndex /= 2;
        }
        
        return proof;
    }
    
    // Affiche l'arbre
    void display() {
        if (root == nullptr) {
            std::cout << "Arbre vide" << std::endl;
            return;
        }
        std::cout << "\n=== Structure de l'arbre de Merkle ===" << std::endl;
        printTree(root, 0, "");
    }
    
    // Affiche les feuilles
    void displayLeaves() {
        std::cout << "\n=== Feuilles (Hashes des données) ===" << std::endl;
        for (size_t i = 0; i < leaves.size(); i++) {
            std::cout << "Feuille " << i << ": " << leaves[i] << std::endl;
        }
    }
};

// Programme principal avec exemples
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "   IMPLEMENTATION ARBRE DE MERKLE" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // Exemple 1: Arbre avec 4 transactions
    std::cout << "\n>>> EXEMPLE 1: Arbre avec 4 transactions <<<\n" << std::endl;
    MerkleTree tree1;
    std::vector<std::string> transactions1 = {
        "Alice envoie 10 BTC à Bob",
        "Bob envoie 5 BTC à Charlie",
        "Charlie envoie 2 BTC à David",
        "David envoie 1 BTC à Alice"
    };
    
    tree1.build(transactions1);
    tree1.displayLeaves();
    tree1.display();
    std::cout << "\nRacine de Merkle: " << tree1.getRootHash() << std::endl;
    
    // Vérification
    std::cout << "\n--- Vérification ---" << std::endl;
    std::string testData = "Alice envoie 10 BTC à Bob";
    std::cout << "La transaction '" << testData << "' existe? " 
              << (tree1.verify(testData) ? "OUI" : "NON") << std::endl;
    
    testData = "Alice envoie 100 BTC à Bob";
    std::cout << "La transaction '" << testData << "' existe? " 
              << (tree1.verify(testData) ? "OUI" : "NON") << std::endl;
    
    // Preuve de Merkle
    std::cout << "\n--- Preuve de Merkle ---" << std::endl;
    std::string dataToProve = "Bob envoie 5 BTC à Charlie";
    std::vector<std::string> proof = tree1.getProof(dataToProve);
    std::cout << "Chemin de preuve pour '" << dataToProve << "':" << std::endl;
    for (size_t i = 0; i < proof.size(); i++) {
        std::cout << "  Niveau " << i << ": " << proof[i] << std::endl;
    }
    
    // Exemple 2: Arbre avec nombre impair de transactions
    std::cout << "\n\n>>> EXEMPLE 2: Arbre avec 5 transactions (nombre impair) <<<\n" << std::endl;
    MerkleTree tree2;
    std::vector<std::string> transactions2 = {
        "TX1", "TX2", "TX3", "TX4", "TX5"
    };
    
    tree2.build(transactions2);
    tree2.display();
    std::cout << "\nRacine de Merkle: " << tree2.getRootHash() << std::endl;
    
    // Exemple 3: Arbre avec 1 seule transaction
    std::cout << "\n\n>>> EXEMPLE 3: Arbre avec 1 transaction <<<\n" << std::endl;
    MerkleTree tree3;
    std::vector<std::string> transactions3 = {"Transaction unique"};
    
    tree3.build(transactions3);
    tree3.display();
    std::cout << "\nRacine de Merkle: " << tree3.getRootHash() << std::endl;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "         FIN DES EXEMPLES" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}