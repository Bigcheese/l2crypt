#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>

#ifdef WIN32
# include <io.h>
# include <fcntl.h>
#endif

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
#ifdef WIN32
  // Set cout to binary.
  if (_setmode(_fileno(stdout), _O_BINARY) == -1) {
    std::cerr << "Failed to set cout to binary mode.\n";
  }
#endif

  if (argc != 2) {
    std::cerr << "l2crypt <input-file>\n";
    return 1;
  }

  std::ifstream file(argv[1], std::ios::binary);
  if (!file) {
    std::cerr << "Failed to open " << argv[1] << "\n";
    return 1;
  }

  std::array<char, 22> target_ver =
  { 0x4C, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x65, 0x00,
    0x61, 0x00, 0x67, 0x00, 0x65, 0x00, 0x32, 0x00,
    0x56, 0x00, 0x65, 0x00, 0x72, 0x00
  }; // Lineage2Ver111
  std::array<char, 22> header;
  std::array<char, 6> version;
  file.read(header.data(), 22);
  file.read(version.data(), 6);

  if (header != target_ver) {
    std::cerr << argv[1] << " is not an encrypted Lineage II file.\n";
    return 1;
  }

  std::string ver;
  ver += version[0];
  ver += version[2];
  ver += version[4];

  std::istringstream out(ver);
  int archive_version;
  out >> archive_version;
  if (!out) {
    std::cerr << argv[1] << " does not have a valid version.\n";
    return 1;
  }

  std::uint8_t key;
  switch (archive_version) {
  case 111: key = 0xAC; break;
  case 121: key = 0x89; break;
  default:
    std::cerr << argv[1] << " version " << archive_version
              << " is not currently supported.\n";
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
             , [key](char c) { return c ^ key; }
             );

  return 0;
}
