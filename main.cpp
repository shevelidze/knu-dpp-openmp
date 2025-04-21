#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>

struct HuffmanTreeNode {
  char ch;
  int freq;
  HuffmanTreeNode* left;
  HuffmanTreeNode* right;
  HuffmanTreeNode(char ch, int freq)
      : ch(ch)
      , freq(freq)
      , left(nullptr)
      , right(nullptr)
  {
  }
  HuffmanTreeNode(char ch, int freq, HuffmanTreeNode* left, HuffmanTreeNode* right)
      : ch(ch)
      , freq(freq)
      , left(left)
      , right(right)
  {
  }
};

// Comparison object to be used to order the heap
struct compare {
  bool operator()(HuffmanTreeNode* l, HuffmanTreeNode* r)
  {
      return l->freq > r->freq;
  }
};

// Function to print the Huffman Codes
void generateCodes(HuffmanTreeNode* root, std::string str,
              std::unordered_map<char, std::string>& huffmanCode)
{
  if (root == nullptr)
      return;

  // Found a leaf node
  if (!root->left && !root->right) {
      huffmanCode[root->ch] = str;
  }

  generateCodes(root->left, str + "0", huffmanCode);
  generateCodes(root->right, str + "1", huffmanCode);
}

int main() {
  std::ifstream input_file;

  input_file.open("input.txt", std::ios::ate);

  const auto input_file_size = input_file.tellg();
  input_file.seekg(0, std::ios::beg);

  std::vector<char> buffer(input_file_size);
  input_file.read(buffer.data(), input_file_size);

  std::unordered_map<char, int> frequencies;

  // Calculating the frequency of each character
  #pragma omp parallel for shared(frequencies)
  for (const auto& c : buffer) {
    frequencies[c]++;
  }

  std::priority_queue<HuffmanTreeNode*, std::vector<HuffmanTreeNode*>, compare> queue;

  for (auto pair : frequencies) {
      queue.push(new HuffmanTreeNode(pair.first, pair.second));
  }

  while (queue.size() > 1) {
      HuffmanTreeNode* left = queue.top();
      queue.pop();
      HuffmanTreeNode* right = queue.top();
      queue.pop();

      int sum = left->freq + right->freq;
      queue.push(new HuffmanTreeNode('\0', sum, left, right));
  }

  HuffmanTreeNode* root_node = queue.top();

  // Generating the codes for the characters
  std::unordered_map<char, std::string> codes;
  generateCodes(root_node, "", codes);

  // Encoding the input data
  std::string encoded_data;

  for (const auto& c : buffer) {
    encoded_data += codes[c];
  }

  std::vector<char> encoded_data_buffer(encoded_data.length() + 1);

  encoded_data_buffer.push_back(encoded_data.length());

  #pragma omp parallel for shared(encoded_data_buffer)
  for (int i = 0; i < encoded_data.size(); i += 8) {
    std::string byte_string = encoded_data.substr(i, 8);

    if (byte_string.length() < 8) {
      byte_string.append(8 - byte_string.length(), '0');
    }

    char byte = static_cast<char>(std::stoi(byte_string, nullptr, 2));
    encoded_data_buffer[i + 1] = byte;
  }

  // Writing the encoded data to a file
  std::ofstream output_file("output.bin", std::ios::binary);
  output_file.write(encoded_data_buffer.data(), encoded_data_buffer.size());
  output_file.close();

  // Writing the codes to a file
  std::ofstream codes_file("codes.txt");
  for (const auto& pair : codes) {
    codes_file << pair.first << " " << pair.second << "\n";
  }
  codes_file.close();
}