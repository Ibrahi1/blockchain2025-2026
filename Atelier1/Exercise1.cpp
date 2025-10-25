#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cstdint>
using namespace std;

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
        static string hash(const string& input) {
            // Pour simplifier, utilisons une fonction de hachage basique
            // En production, utilisez une vraie implémentation SHA-256
            uint32_t h = 0x6a09e667;
            
            for (char c : input) {
                h = ((h << 5) + h) + static_cast<uint32_t>(c);
                h ^= (h >> 16);
            }
            
            stringstream ss;
            ss << hex << setfill('0') << setw(8) << h;
            return ss.str();
        }
};

const uint32_t SHA256::K[64] = {0}; // Simplifié pour l'exemple

// Classe représentant un nœud de l'arbre de Merkle
class MerkleNode {
public:
    string hash;
    MerkleNode* left;
    MerkleNode* right;
    
    MerkleNode(const string& h) : hash(h), left(nullptr), right(nullptr) {}
    
    ~MerkleNode() {
        delete left;
        delete right;
    }
};

// Classe principale de l'arbre de Merkle
class MerkleTree {
private:
    MerkleNode* root;
    vector<string> leaves;
    
    // Crée un hash à partir de deux hashes enfants
    string combineHashes(const string& left, const string& right) {
        return SHA256::hash(left + right);
    }
    
    // // Construit l'arbre récursivement
    // MerkleNode* buildTree(vector<string>& hashes) {
    //     if (hashes.empty()) return nullptr;
        
    //     if (hashes.size() == 1) {
    //         return new MerkleNode(hashes[0]);
    //     }
        
    //     vector<string> parentHashes;
        
    //     // Combiner les hashes par paires
    //     for (size_t i = 0; i < hashes.size(); i += 2) {
    //         if (i + 1 < hashes.size()) {
    //             // Deux enfants
    //             string combined = combineHashes(hashes[i], hashes[i + 1]);
    //             parentHashes.push_back(combined);
    //         } else {
    //             // Nombre impair, dupliquer le dernier
    //             string combined = combineHashes(hashes[i], hashes[i]);
    //             parentHashes.push_back(combined);
    //         }
    //     }
        
    //     // Construire le nœud actuel
    //     MerkleNode* node = buildTree(parentHashes);
    //     return node;
    // }
    
    // Construit l'arbre avec les nœuds intermédiaires
    MerkleNode* buildTreeWithNodes(vector<MerkleNode*>& nodes) {
        if (nodes.size() == 1) {
            return nodes[0];
        }
        
        vector<MerkleNode*> parentNodes;
        
        for (size_t i = 0; i < nodes.size(); i += 2) {
            MerkleNode* left = nodes[i];
            MerkleNode* right = (i + 1 < nodes.size()) ? nodes[i + 1] : nodes[i];
            
            string parentHash = combineHashes(left->hash, right->hash);
            MerkleNode* parent = new MerkleNode(parentHash);
            parent->left = left;
            parent->right = (i + 1 < nodes.size()) ? right : nullptr;
            
            parentNodes.push_back(parent);
        }
        
        return buildTreeWithNodes(parentNodes);
    }
    
    // Affiche l'arbre récursivement
    void printTree(MerkleNode* node, int level, const string& prefix) {
        if (node == nullptr) return;
        
        cout << prefix;
        cout << (level == 0 ? "Root: " : "├── ");
        cout << node->hash << endl;
        
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
    void build(const vector<string>& data) {
        if (data.empty()) {
            cout << "Erreur: Aucune donnée fournie" << endl;
            return;
        }
        
        leaves.clear();
        vector<MerkleNode*> leafNodes;
        
        // Créer les feuilles (hasher chaque donnée)
        for (const auto& item : data) {
            string leafHash = SHA256::hash(item);
            leaves.push_back(leafHash);
            leafNodes.push_back(new MerkleNode(leafHash));
        }
        
        // Construire l'arbre
        root = buildTreeWithNodes(leafNodes);
        
        cout << "Arbre de Merkle construit avec " << data.size() << " éléments" << endl;
    }
    
    // Retourne la racine de Merkle
    string getRootHash() {
        if (root == nullptr) return "";
        return root->hash;
    }
    
    // Vérifie si une donnée existe dans l'arbre
    bool verify(const string& data) {
        string dataHash = SHA256::hash(data);
        for (const auto& leaf : leaves) {
            if (leaf == dataHash) {
                return true;
            }
        }
        return false;
    }
    
    // Obtient le chemin de preuve pour une donnée
    vector<string> getProof(const string& data) {
        vector<string> proof;
        string dataHash = SHA256::hash(data);
        
        // Trouver l'index de la feuille
        int index = -1;
        for (size_t i = 0; i < leaves.size(); i++) {
            if (leaves[i] == dataHash) {
                index = i;
                break;
            }
        }
        
        if (index == -1) {
            cout << "Donnée non trouvée dans l'arbre" << endl;
            return proof;
        }
        
        // Construire le chemin de preuve
        vector<string> currentLevel = leaves;
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
            vector<string> nextLevel;
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
            cout << "Arbre vide" << endl;
            return;
        }
        cout << "\n=== Structure de l'arbre de Merkle ===" << endl;
        printTree(root, 0, "");
    }
    
    // Affiche les feuilles
    void displayLeaves() {
        cout << "\n=== Feuilles (Hashes des données) ===" << endl;
        for (size_t i = 0; i < leaves.size(); i++) {
            cout << "Feuille " << i << ": " << leaves[i] << endl;
        }
    }
};

// Programme principal avec exemples
int main() {
    cout << "========================================" << endl;
    cout << "   IMPLEMENTATION ARBRE DE MERKLE" << endl;
    cout << "========================================\n" << endl;
    
    // Exemple 1: Arbre avec 4 transactions
    cout << "\n>>> EXEMPLE 1: Arbre avec 4 transactions <<<\n" << endl;
    MerkleTree tree1;
    vector<string> transactions1 = {
        "Alice envoie 10 BTC à Bob",
        "Bob envoie 5 BTC à Charlie",
        "Charlie envoie 2 BTC à David",
        "David envoie 1 BTC à Alice"
    };
    
    tree1.build(transactions1);
    tree1.displayLeaves();
    tree1.display();
    cout << "\nRacine de Merkle: " << tree1.getRootHash() << endl;
    
    // Vérification
    cout << "\n--- Vérification ---" << endl;
    string testData = "Alice envoie 10 BTC à Bob";
    cout << "La transaction '" << testData << "' existe? " 
              << (tree1.verify(testData) ? "OUI" : "NON") << endl;
    
    testData = "Alice envoie 100 BTC à Bob";
    cout << "La transaction '" << testData << "' existe? " 
              << (tree1.verify(testData) ? "OUI" : "NON") << endl;
    
    // Preuve de Merkle
    cout << "\n--- Preuve de Merkle ---" << endl;
    string dataToProve = "Bob envoie 5 BTC à Charlie";
    vector<string> proof = tree1.getProof(dataToProve);
    cout << "Chemin de preuve pour '" << dataToProve << "':" << endl;
    for (size_t i = 0; i < proof.size(); i++) {
        cout << "  Niveau " << i << ": " << proof[i] << endl;
    }
    
    // Exemple 2: Arbre avec nombre impair de transactions
    cout << "\n\n>>> EXEMPLE 2: Arbre avec 5 transactions (nombre impair) <<<\n" << endl;
    MerkleTree tree2;
    vector<string> transactions2 = {
        "TX1", "TX2", "TX3", "TX4", "TX5"
    };
    
    tree2.build(transactions2);
    tree2.display();
    cout << "\nRacine de Merkle: " << tree2.getRootHash() << endl;
    
    // Exemple 3: Arbre avec 1 seule transaction
    cout << "\n\n>>> EXEMPLE 3: Arbre avec 1 transaction <<<\n" << endl;
    MerkleTree tree3;
    vector<string> transactions3 = {"Transaction unique"};
    
    tree3.build(transactions3);
    tree3.display();
    cout << "\nRacine de Merkle: " << tree3.getRootHash() << endl;
    
    cout << "\n========================================" << endl;
    cout << "         FIN DES EXEMPLES" << endl;
    cout << "========================================" << endl;
    
    return 0;
}