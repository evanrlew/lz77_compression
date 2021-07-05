#include <iostream>
#include <string>
#include <fstream>

const int  match_len = 4;
const int  max_back_search_dist_bits = 5;
const char control_char = 0x24;
const int  control_bytes = 3;

class TextBuffer {
  private:
    char* text_buffer;
    int buf_depth;
    int buf_idx;
    int buf_count;

  public:
    TextBuffer(int);
    void add_chars(char*, int);
    int search(char*, int);
    void partial_search(char*, int, int);
    char get_char(int);
    int  get_count(void);
    int  get_depth(void);
    int  get_head_idx(void);
    void dump_state(void);
    int  get_substring(char*, int, int);
};

TextBuffer::TextBuffer(int depth) {
  this->buf_idx = 0;
  this->buf_count = 0;
  this->buf_depth = depth;
  this->text_buffer = (char *) malloc(this->buf_depth * sizeof(char));
}
   
void TextBuffer::add_chars(char *input, int length) {
  for (int ii = 0; ii < length; ii++) {
    this->text_buffer[this->buf_idx] = input[ii];
    this->buf_idx = (this->buf_idx + 1) % this->buf_depth;
    if ( this->buf_count < this->buf_depth ) {
      this->buf_count++;
    }
  }
}

void TextBuffer::dump_state(void) {
  //printf("buf_depth = %d\n", this->buf_depth);
  //printf("buf_count = %d\n", this->buf_count);
  //printf("buf_idx   = %d\n", this->buf_idx);
  //printf("get_head_idx()   = %d\n", this->get_head_idx());
  for (int ii = this->buf_count; ii > 0; ii--) {
    std::cout << this->text_buffer[(this->buf_count + this->buf_idx - ii) % this->buf_count];
  }
  std::cout << std::endl;
};

int TextBuffer::get_head_idx(void) {
  if (this->buf_count == 0) {
    return -1;
  }
  else if (this->buf_count < this->buf_depth) {
    return 0;
  } else {
    return this->buf_idx;
  }
}

int TextBuffer::search(char *pattern, int length) {
  int chars_matched = 0;
  for (int ii = 0; ii < this->buf_count; ii++) {
    char c = this->text_buffer[(this->get_head_idx() + ii) % this->buf_depth];
    if (c == pattern[chars_matched]) {
      chars_matched++;
    } else {
      chars_matched = 0;
    }
    if (chars_matched == length) {
      int distance = (this->buf_count - 1 - ii);
      return distance; 
    }
  }
  return -1; // not found
}

int TextBuffer::get_substring(char* output, int virtual_index, int length) {
  // FIXME protect against bad get_head_idx return code
  int start_idx = (this->get_head_idx() + virtual_index) % this->buf_depth;
  int stop_idx = (start_idx + length) % this->buf_depth;

  if (length > this->buf_depth) {
    throw std::invalid_argument("Length cannot exceed buffer depth");
  }
  if (this->buf_count < virtual_index+length) {
    throw std::invalid_argument("Length cannot exceed buffer depth");
  }

    
  //if (start_idx < stop_idx) {
  //  printf("start_idx = %d\n", start_idx);
  //  this->dump_state();
  //  output = this->text_buffer[start_idx];
  //}
  //else {
    for (int ii = 0; ii < length; ii++) {
      output[ii] = this->text_buffer[(start_idx + ii ) % this->buf_depth];
    }
  //}
}

int TextBuffer::get_count(void) {
  return this->buf_count;
}

int TextBuffer::get_depth(void) {
  return this->buf_depth;
}

char TextBuffer::get_char(int virtual_index) {
  return this->text_buffer[(this->get_head_idx() + virtual_index) % this->buf_depth];
}

void create_jump_token(char* output, int jump_dist, int pattern_length) {
  output[0] = control_char;
  output[1] = '.';
  output[2] = '.';
  //output[1] = jump_dist;
  //output[2] = pattern_length;
}

  

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

  char c = '\0';
  char* search_str = (char*) malloc(match_len * sizeof(char));
  int buf_size = (1<<max_back_search_dist_bits) + match_len;
  TextBuffer* tb = new TextBuffer(buf_size);
  
  while (inputFile.get(c)) {
    
    tb->add_chars(&c, 1);
    if (tb->get_count() >= match_len) {
      tb->get_substring(search_str, tb->get_count()-match_len, match_len);
      int loc = tb->search(search_str, match_len);
      if (loc > match_len) {
        char* control_token = (char *) malloc(match_len * sizeof(char));
        printf("searching for %c%c%c%c...", search_str[0],search_str[1],search_str[2],search_str[3]);
        printf("FOUND @ -%d\n", loc);
        create_jump_token(control_token, loc, match_len);
        outputFile.write(control_token, control_bytes);
        free(control_token);
        for (int ii = 0; ii < match_len; ii++) {
          if (inputFile.get(c)) { 
            tb->add_chars(&c, 1);
          }
        }
        
      } else {
        char c = tb->get_char(tb->get_count() - match_len);
        outputFile.write(&c, 1);
      }
    }

  }

  // Pipe down
  for (int ii = 0; ii < match_len; ii++) {
    char c = tb->get_char(tb->get_count() - match_len + ii);
    outputFile.write(&c, 1);
  }

  inputFile.close();
  outputFile.close();

  return 0;
};
