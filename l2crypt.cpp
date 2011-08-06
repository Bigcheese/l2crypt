#include <algorithm>
#include <array>
#include <fstream>
#include <functional>
#include <iostream>

template<class InputIterator, class Size,
         class OutputIterator, class UnaryOperation>
OutputIterator transform_n(InputIterator first, Size n,
                           OutputIterator result, UnaryOperation op) {
  for (Size i = 0; i < n; ++i) {
    *(result++) = op(*(first++));
  }

  return result;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "l2crypt <input-file>\n";
    return 1;
  }

  std::ifstream file(argv[1], std::ios::binary);
  if (!file) {
    std::cerr << "Failed to open " << argv[1] << "\n";
    return 1;
  }

  std::array<char, 28> target_ver =
  { 0x4C, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x65, 0x00,
    0x61, 0x00, 0x67, 0x00, 0x65, 0x00, 0x32, 0x00,
    0x56, 0x00, 0x65, 0x00, 0x72, 0x00, 0x31, 0x00,
    0x31, 0x00, 0x31, 0x00 }; // Lineage2Ver111
  std::array<char, 28> version;
  file.read(version.data(), 28);

  if (version != target_ver) {
    std::cerr << argv[1] << " is not version 111 or is nothingness" << "\n";
    return 1;
  }

  file.seekg(0, std::ios::end);
  std::streamsize file_size = file.tellg();
  file_size -= (20 + 28); // Remove header and footer length.
  file.seekg(28, std::ios::beg);
  std::streamsize bytes_to_read = file_size;

  transform_n( std::istreambuf_iterator<char>(file)
             , file_size
             , std::ostreambuf_iterator<char>(std::cout)
             , [](char c) { return c ^ 0xAC; }
             );

  return 0;
}
