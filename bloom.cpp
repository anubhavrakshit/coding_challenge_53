#include <bitset>
#include <cassert>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

// We need to create multiple hashes for the same key
// Use the std::hash to generate a seed for random number
// generator.
// Keep calling the generator to produce the required number of
// hashes
template <typename T>
auto hashGen(const T &key, std::size_t n) -> std::vector<std::size_t> {
  std::default_random_engine e1(std::hash<T>{}(key));
  std::vector<size_t> hashes(n);
  std::generate_n(hashes.begin(), n, e1);
  return hashes;
}


// Supply the number of items and positivity rate to create a Bloom 
// filter
class Bloom {
public:
  Bloom() = delete;
  Bloom(size_t nitems, double positivity) : n_{nitems}, p_{positivity} {
    // Given:

    // n: how many items you expect to have in your filter (e.g. 216,553)
    // p: your acceptable false positive rate {0..1} (e.g. 0.01 → 1%)
    // we want to calculate:

    // m: the number of bits needed in the bloom filter
    // k: the number of hash functions we should apply
    // The formulas:

    // m = -n*ln(p) / (ln(2)^2) the number of bits
    // k = m/n * ln(2) the number of hash functions

    m_ = ((double)n_ * log(p_) * -1) / (log(2) * log(2));
    k_ = (m_ / n_) * log(2);
    std::cout << "Number of bits: " << m_ << " Number of hash fns: " << k_
              << std::endl;
    bitset_.resize(m_);
    std::fill(bitset_.begin(), bitset_.end(), false);
  }

  // Insert a Dictionary to the bloom filter
  size_t insert(std::string dict) {
    std::ifstream data(dict);
    size_t count{0};
    if (!data.is_open()) {
      return 0;
    }
    std::string key;
    while (data >> key) {
      auto hashes = hashGen<std::string>(key, k_);
      count++;
      for (auto h : hashes) {
        bitset_.at(h % m_) = true;
      }
    }
    return count;
  }

  // Query individual words for presence in dictionary
  bool query(std::string key) {
    auto hashes = hashGen<std::string>(key, k_);
    auto ret = true;
    for (auto h : hashes) {
      if (bitset_.at(h % m_) == false) {
        ret = false;
        break;
      }
    }
    return ret;
  }

  // Header to serialize/deserialize the Bloom filter to disk
  struct header {
    uint8_t magic[4] = {'C', 'C', 'B', 'F'}; // CCBF
    uint8_t version[2] = {0x0, 0x1};
    uint16_t n_hashfns;   // NUmber of hash functions
    uint64_t bitset_size; // Number of bits used in filter
    header() = default;
    header(size_t k, size_t m)
        : n_hashfns{static_cast<uint16_t>(k)}, bitset_size{m} {}
    void print() {
      std::cout << "Magic: " << magic[0] << magic[1] << magic[2] << magic[3]
                << "\t"
                << " n_hashfns: " << n_hashfns
                << " bitset_size: " << bitset_size << std::endl;
    }
  };

  // Serialize the Bloom filter to disk
  bool serialize(std::string diskfile) {
    std::ofstream serialized_data(diskfile, std::ios::binary);
    if (!serialized_data.is_open()) {
      return false;
    }
    header hdr(k_, m_);

    serialized_data.write((char *)&hdr, sizeof(hdr));

    for (size_t i = 0; i < bitset_.size(); i += 8) {
      char byte{0};
      size_t pos{0};
      // The minimum granularity to write to disk is 1 byte
      // So we will aggregate bits to a byte and then write
      // to the file stream
      for (auto j = i; j < i + 8 and j < bitset_.size(); j++) {
        if (bitset_[j]) {
          byte |= (0x1 << pos);
        }
        pos++;
      }
      serialized_data.write(&byte, sizeof(byte));
    }
    return true;
  }

  // Deserialize the Bloom filter from disk
  std::vector<bool> deserialize(std::string diskfile) {
    std::vector<bool> des_bitset;
    std::ifstream deserialize_data(diskfile, std::ios::binary);
    if (!deserialize_data.is_open()) {
      return des_bitset;
    }
    header hdr;
    deserialize_data.read((char *)&hdr, sizeof(hdr));
    hdr.print();
    size_t to_read = hdr.bitset_size;
    des_bitset.resize(to_read);
    std::fill(des_bitset.begin(), des_bitset.end(), false);

    size_t bitset_pos{0};
    while (bitset_pos < to_read) {
      char byte{0};
      deserialize_data.read(&byte, sizeof(byte));
      for (size_t i = 0; i < 8 and bitset_pos < to_read; i++) {
        assert(bitset_pos < hdr.bitset_size);
        des_bitset[bitset_pos++] = byte & (0x1 << i);
      }
    }
    // Verify if we got the same bitset as we have in
    // memory
    if (bitset_ == des_bitset) {
      std::cout << "Matched Deserialized Bitsets!!" << std::endl;
    }
    return des_bitset;
  }

private:
  size_t n_{0}; // Number of items in set
  double p_{0}; // Acceptable false positive rate
  size_t m_{0}; // Number of bits in bloom filter
  size_t k_{0}; // Number of hash functions
  std::vector<bool> bitset_{0};
};

int main() {
  // Lets create a bloom filter with 0.01% false positives
  // On mac os dict.txt has 235976 words
  Bloom bf(235976, 0.01);
  // Read the dict.txt and create bloom filter
  bf.insert("dict.txt");
  // Serialize the bloom filter to disk
  bf.serialize("words.bf");
  // Deserialize the filter and verify that bitsets match
  bf.deserialize("words.bf");
  // Run some tests
  assert(bf.query("coding") == false);
  assert(bf.query("word") == true);
  assert(bf.query("Aaronical") == true);
  assert(bf.query("Zyryan") == true);
  assert(bf.query("Zyryen") == false);
  assert(bf.query("concurrency") == true);
  return 0;
}