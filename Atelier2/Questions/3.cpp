#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <iostream>
#include <cstdint>
#include <vector>
#include <bitset>
#include <algorithm>

// -- Déclaration de ac_hash() depuis la question 2 --
std::string ac_hash(const std::string& input, uint32_t rule, size_t steps);

// Sélection du mode de hash
enum class HashMode { SHA256, AC_HASH };
HashMode hash_mode = HashMode::SHA256;

// Fonction SHA256 standard
std::string sha256(const std::string& str) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(str.c_str()), str.size(), hash);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return ss.str();
}

// --- Classe Block ---
class Block {
public:
    int index;
    std::string data;
    std::string prevHash;
    std::string hash;
    long nonce;

    Block(int idx, const std::string& d, const std::string& prev)
        : index(idx), data(d), prevHash(prev), nonce(0) {}

    std::string calculateHash() const {
        std::stringstream ss;
        ss << index << prevHash << data << nonce;
        std::string input = ss.str();

        if (hash_mode == HashMode::SHA256)
            return sha256(input);
        else
            return ac_hash(input, 110, 100);  // règle et steps de ton choix
    }

    void mineBlock(int difficulty) {
        std::string target(difficulty, '0');
        do {
            nonce++;
            hash = calculateHash();
        } while (hash.substr(0, difficulty) != target);

        std::cout << "Block mined: " << hash << std::endl;
    }
};
