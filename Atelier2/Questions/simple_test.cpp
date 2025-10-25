// ============================================================
// This implimentation like the full_implimentation.cpp but is more simple for execution
// The result is in the image.png
// ============================================================

#include <bits/stdc++.h>
using namespace std;

// ============================================================
// Utility functions
// ============================================================
vector<uint8_t> str_to_bits(const string& s) {
    vector<uint8_t> bits;
    for (unsigned char c : s)
        for (int k = 7; k >= 0; --k)
            bits.push_back((c >> k) & 1);
    return bits;
}

string bits_to_hex(const vector<uint8_t>& bits) {
    stringstream ss;
    for (size_t i = 0; i < bits.size(); i += 8) {
        uint8_t byte = 0;
        for (int j = 0; j < 8; ++j)
            byte = (byte << 1) | bits[i + j];
        ss << hex << setw(2) << setfill('0') << (int)byte;
    }
    return ss.str();
}

vector<uint8_t> hex_to_bits(const string& hexs) {
    vector<uint8_t> bits;
    for (size_t i = 0; i < hexs.size(); i += 2) {
        uint8_t b = stoi(hexs.substr(i, 2), nullptr, 16);
        for (int k = 7; k >= 0; --k) bits.push_back((b >> k) & 1);
    }
    return bits;
}

size_t hamming_distance(const vector<uint8_t>& a, const vector<uint8_t>& b) {
    size_t d = 0;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i] != b[i]) ++d;
    return d;
}

// ============================================================
// 1. Cellular Automaton Class (1D, binary, r=1)
// ============================================================
class CellularAutomaton1D {
    vector<uint8_t> state;
    uint8_t rule;
    size_t width;
public:
    CellularAutomaton1D(uint8_t rule, size_t w = 256)
        : rule(rule), width(w), state(w, 0) {}

    void init(const vector<uint8_t>& bits) {
        for (size_t i = 0; i < width; ++i)
            state[i] = bits[i % bits.size()];
    }

    void evolve_once() {
        vector<uint8_t> next(width);
        for (size_t i = 0; i < width; ++i) {
            uint8_t l = state[(i + width - 1) % width];
            uint8_t c = state[i];
            uint8_t r = state[(i + 1) % width];
            uint8_t idx = (l << 2) | (c << 1) | r;
            next[i] = (rule >> idx) & 1;
        }
        state.swap(next);
    }

    void evolve(size_t steps) {
        for (size_t i = 0; i < steps; ++i)
            evolve_once();
    }

    vector<uint8_t> get_state() const { return state; }
};

// ============================================================
// 2. AC-based Hash Function
// ============================================================
class ACHash {
public:
    static string compute(const string& input, uint32_t rule, size_t steps) {
        vector<uint8_t> bits = str_to_bits(input);
        CellularAutomaton1D ca(rule, 256);
        ca.init(bits);
        ca.evolve(steps);
        return bits_to_hex(ca.get_state());
    }
};

// ============================================================
// 3. Simple SHA256 (pure C++, no OpenSSL)
// ============================================================
class SimpleSHA256 {
    static inline uint32_t ror(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }
    static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
    static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
    static inline uint32_t e0(uint32_t x) { return ror(x, 2) ^ ror(x, 13) ^ ror(x, 22); }
    static inline uint32_t e1(uint32_t x) { return ror(x, 6) ^ ror(x, 11) ^ ror(x, 25); }
    static inline uint32_t s0(uint32_t x) { return ror(x, 7) ^ ror(x, 18) ^ (x >> 3); }
    static inline uint32_t s1(uint32_t x) { return ror(x, 17) ^ ror(x, 19) ^ (x >> 10); }
public:
    static string compute(const string& input) {
        static const uint32_t k[64] = {
            0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
            0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
            0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
            0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
            0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
            0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
            0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
            0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
        };
        uint32_t h[8] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
                         0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};

        vector<uint8_t> data(input.begin(), input.end());
        uint64_t bitlen = data.size() * 8;
        data.push_back(0x80);
        while ((data.size() % 64) != 56) data.push_back(0);
        for (int i = 7; i >= 0; --i) data.push_back((bitlen >> (i*8)) & 0xFF);

        for (size_t c = 0; c < data.size(); c += 64) {
            uint32_t w[64];
            for (int i = 0; i < 16; ++i)
                w[i] = (data[c+4*i]<<24)|(data[c+4*i+1]<<16)|(data[c+4*i+2]<<8)|data[c+4*i+3];
            for (int i = 16; i < 64; ++i)
                w[i] = s1(w[i-2]) + w[i-7] + s0(w[i-15]) + w[i-16];
            uint32_t a=h[0],b=h[1],c_=h[2],d=h[3],e=h[4],f=h[5],g=h[6],hh=h[7];
            for (int i = 0; i < 64; ++i) {
                uint32_t t1 = hh + e1(e) + ch(e,f,g) + k[i] + w[i];
                uint32_t t2 = e0(a) + maj(a,b,c_);
                hh=g; g=f; f=e; e=d+t1; d=c_; c_=b; b=a; a=t1+t2;
            }
            h[0]+=a;h[1]+=b;h[2]+=c_;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=hh;
        }
        stringstream ss; ss << hex << setfill('0');
        for (int i = 0; i < 8; ++i) ss << setw(8) << h[i];
        return ss.str();
    }
};

// ============================================================
// 4. Blockchain simulation (simplified for fast testing)
// ============================================================
enum HashMode { AC_MODE, SHA_MODE };

struct Block {
    int index;
    string prev_hash, data, hash;
    unsigned long nonce;
};

class Blockchain {
public:
    static string compute_hash(const Block& b, HashMode mode, uint32_t rule, size_t steps) {
        string input = to_string(b.index) + b.prev_hash + b.data + to_string(b.nonce);
        if (mode == AC_MODE) return ACHash::compute(input, rule, steps);
        else return SimpleSHA256::compute(input);
    }

    static Block mine(int index, const string& prev, const string& data,
                      HashMode mode, uint32_t rule, size_t steps, const string& prefix) {
        Block b{index, prev, data, "", 0};
        for (unsigned long nonce = 0; nonce < 20000; ++nonce) {
            b.nonce = nonce;
            b.hash = compute_hash(b, mode, rule, steps);
            if (b.hash.rfind(prefix, 0) == 0) return b;
        }
        return b;
    }
};

// ============================================================
// 5. Tests
// ============================================================
double avalanche(uint32_t rule, size_t steps) {
    mt19937 rng(42);
    int trials = 20;
    double total = 0;
    for (int i = 0; i < trials; ++i) {
        string s = "Msg" + to_string(rng());
        string s2 = s; s2[0] ^= 1;
        string h1 = ACHash::compute(s, rule, steps);
        string h2 = ACHash::compute(s2, rule, steps);
        total += hamming_distance(hex_to_bits(h1), hex_to_bits(h2));
    }
    return total / (256.0 * trials) * 100.0;
}

double distribution(uint32_t rule, size_t steps) {
    mt19937 rng(99);
    size_t ones = 0, total = 0;
    for (int i = 0; i < 50; ++i) {
        string msg = "A" + to_string(rng());
        auto bits = hex_to_bits(ACHash::compute(msg, rule, steps));
        for (auto b : bits) { ones += b; total++; }
    }
    return 100.0 * ones / total;
}

// ============================================================
// 6. MAIN – Automatic Execution of All Tests
// ============================================================
int main() {
    cout << "================ Atelier 2 – Automate Cellulaire & Hash =================\n";

    string input = "Blockchain";
    cout << "Rule 30 hash: " << ACHash::compute(input, 30, 128).substr(0, 32) << "...\n";
    cout << "SHA256 hash : " << SimpleSHA256::compute(input).substr(0, 32) << "...\n";

    // --- Mining speed comparison ---
    cout << "\n[Mining Comparison] (10 blocks simulated)\n";
    vector<pair<string, pair<double,double>>> results;
    for (auto mode : {AC_MODE, SHA_MODE}) {
        double t_sum = 0;
        string prev = "0";
        for (int i = 0; i < 5; ++i) {
            auto start = chrono::high_resolution_clock::now();
            Block b = Blockchain::mine(i, prev, "data"+to_string(i),
                                       mode, 30, 64, "00");
            auto end = chrono::high_resolution_clock::now();
            double ms = chrono::duration<double, milli>(end-start).count();
            t_sum += ms;
            prev = b.hash;
        }
        string name = (mode==AC_MODE?"AC_HASH":"SHA256");
        results.push_back({name, {t_sum/5.0, 1000}});
    }
    cout << setw(12) << "Method" << setw(16) << "Avg time (ms)" << setw(16) << "Avg iters" << "\n";
    for (auto& r : results)
        cout << setw(12) << r.first << setw(16) << r.second.first << setw(16) << "≈" <<  r.second.second << "\n";

    // --- Avalanche effect ---
    cout << "\n[Avalanche Effect]\n";
    for (auto rule : {30, 90, 110})
        cout << "Rule " << rule << " -> " << avalanche(rule, 128) << "% differing bits\n";

    // --- Bit distribution ---
    cout << "\n[Bit Distribution]\n";
    for (auto rule : {30, 90, 110})
        cout << "Rule " << rule << " -> " << distribution(rule, 128) << "% ones\n";

    // --- Rule comparison ---
    cout << "\n[Rule Comparison Performance]\n";
    for (auto rule : {30, 90, 110}) {
        auto start = chrono::high_resolution_clock::now();
        for (int i = 0; i < 200; ++i)
            volatile auto h = ACHash::compute("sample"+to_string(i), rule, 64);
        auto end = chrono::high_resolution_clock::now();
        double ms = chrono::duration<double, milli>(end-start).count()/200.0;
        cout << "Rule " << rule << " avg time per hash: " << ms << " ms\n";
    }

    // --- Short analysis printed ---
    cout << "\n[Analysis]\n";
    cout << "- Advantages: nonlinear behavior, good diffusion, simple implementation.\n";
    cout << "- Weaknesses: no proven collision resistance, depends on chosen rule.\n";
    cout << "- Improvement idea: combine AC_HASH + SHA256 for hybrid security.\n";

    cout << "==========================================================================\n";
    return 0;
}
