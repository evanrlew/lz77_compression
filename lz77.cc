#include <iostream>
#include <string>
#include <fstream>


int main(int argc, char* argv[]) {
  bool encodeMode;
  
  if (argc < 4) { 
    std::cout << "Please specificy encode/decode, input file and output file" << std::endl;
    return 1;
  }

  std::string encodeFlag ("-e");
  std::string decodeFlag ("-d");
  if (encodeFlag.compare(argv[1]) == 0) {
    encodeMode = true;
  }
  else if (decodeFlag.compare(argv[1]) == 0) {
    encodeMode = false;
  }
  else {
    std::cout << "First flag must be -d (decode) -e (encode)" << std::endl;
    return 1;
  }

  std::ifstream inputFile;
  inputFile.open(argv[2]);

  std::ofstream outputFile;
  outputFile.open(argv[3]);

  if (!inputFile.is_open()) {
    printf("Could not open '%s'\n", argv[2]);
    return 1;
  }
  if (!outputFile.is_open()) {
    printf("Could not open '%s'\n", argv[3]);
    return 1;
  }

  char c = 0;
  char* input_buf;
  int buf_size = 5;
  int buf_idx = 0;
  bool buf_full = false;

  input_buf = (char *) malloc(buf_size * sizeof(char));
 
  while (inputFile.get(c)) {
    if (buf_full == false && buf_idx <= buf_size-1) {
      buf_full = true;
    }

    input_buf[buf_idx] = c;
    buf_idx = (buf_idx + 1) % buf_size;


    for (int ii = 0; ii < buf_size and (ii <= buf_idx or buf_full); ii++) {
      std::cout << input_buf[ii];
    }
    std::cout << std::endl;

  }

  


  inputFile.close();
  outputFile.close();



  return 0;
};
