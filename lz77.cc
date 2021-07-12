#include <iostream>
#include <string>
#include <fstream>

const int  min_match_len = 4;
const int  match_bits = 4;
const int  max_match_len = (min_match_len+(1<<match_bits));
const int  jump_dist_bits = 8;
const char control_char = 0x24;
const int  control_bytes = 3;

class RingBufferMatchResult {
  public:
    int jump_dist;
    int length;
};

class RingBuffer {
  private:
    char* text_buffer;
    int buf_depth;
    int buf_idx;
    int buf_count;
    int get_index(int);

  public:
    RingBuffer(int);
    void add_chars(char*, int);
    int search(RingBufferMatchResult*, char*, int);
    int partial_search(RingBufferMatchResult*, char*, int, int);
    void partial_search(char*, int, int);
    char get_char(int);
    int  get_count(void);
    int  get_depth(void);
    void dump_state(void);
    void get_substring(char*, int, char);
};

RingBuffer::RingBuffer(int depth) {
  this->buf_idx = 0;
  this->buf_count = 0;
  this->buf_depth = depth;
  this->text_buffer = (char *) malloc(this->buf_depth * sizeof(char));
}
   
void RingBuffer::add_chars(char *input, int length) {
  for (int ii = 0; ii < length; ii++) {
    this->text_buffer[this->buf_idx] = input[ii];
    this->buf_idx = (this->buf_idx + 1) % this->buf_depth;
    if ( this->buf_count < this->buf_depth ) {
      this->buf_count++;
    }
  }
}

void RingBuffer::dump_state(void) {
  //printf("buf_depth = %d\n", this->buf_depth);
  //printf("buf_count = %d\n", this->buf_count);
  //printf("buf_idx   = %d\n", this->buf_idx);
  for (int ii = this->buf_count; ii > 0; ii--) {
    std::cout << this->get_char(ii);
  }
  std::cout << std::endl;
};

int RingBuffer::get_index(int virtual_index) {
  if ( (virtual_index > (this->buf_count-1)) || (virtual_index < -this->buf_count) ) {
    throw std::invalid_argument("Virtual index is out of bounds");
  }

  if (virtual_index >= 0) {
    if (this->buf_count < this->buf_depth) { // positive means index from the head
      return virtual_index;  
    } else {
      return (this->buf_idx + virtual_index) % this->buf_depth;
    }
  } else { // negative means index from the tail
    if (this->buf_count < this->buf_depth) {
      return this->buf_idx + virtual_index;
    } else {
      return (this->buf_depth + this->buf_idx + virtual_index) % this->buf_depth;
    }
  }
}

int RingBuffer::search(RingBufferMatchResult* mr, char *pattern, int length) {
  int chars_matched = 0;
  for (int ii = 0; ii < this->buf_count; ii++) {
    char c = this->get_char(ii);
    if (c == pattern[chars_matched]) {
      chars_matched++;
    } else {
      chars_matched = 0;
    }
    if (chars_matched == length) {
      mr->jump_dist = (this->buf_count - 1 - ii);
      mr->length = length;
      return 0; 
    }
  }
  return -1; // not found
}

int RingBuffer::partial_search(RingBufferMatchResult* mr, char *pattern, int length, int min_match_length) {
  int best_match_pos = -1;
  int best_match_len = -1;
  int chars_matched = 0;
  for (int ii = 0; ii < this->buf_count; ii++) {
    char c = this->get_char(ii);
    if (chars_matched < length && c == pattern[chars_matched]) {
      chars_matched++;
      if (chars_matched >= min_match_length && chars_matched > best_match_len) {
        best_match_pos = (this->buf_count - 1 - ii);
        best_match_len = chars_matched;
      }
    } else {
      chars_matched = 0;
    }
  }
  if (best_match_pos > -1) {
    mr->jump_dist = best_match_pos;
    mr->length = best_match_len;
    return 0; 
  } else {
    return -1; // not found
  }
}
  

void RingBuffer::get_substring(char* output, int virtual_index, char length) {

  if (length > this->buf_depth) {
    throw std::invalid_argument("Length cannot exceed buffer depth");
  }
  if (this->buf_count < virtual_index+length) {
    printf("buf_count       = %d\n", buf_count);
    printf("virtual_index   = %d\n", virtual_index);
    printf("length          = %d\n", length);
    fflush(stdout);
    throw std::invalid_argument("virtual index + length cannot exceed buffer depth");
  }

  for (int ii = 0; ii < length; ii++) {
    output[ii] = this->get_char(virtual_index + ii);
  }
}

int RingBuffer::get_count(void) {
  return this->buf_count;
}

int RingBuffer::get_depth(void) {
  return this->buf_depth;
}

char RingBuffer::get_char(int virtual_index) {
  return this->text_buffer[this->get_index(virtual_index)];
}

void create_jump_token(char* output, unsigned int jump_dist, unsigned int pattern_length) {
  output[0] = control_char;

  if (jump_dist > 1<<jump_dist_bits) {
    throw std::invalid_argument("jump dist doesn't fit in jump_dist_bits");
  }

  if (pattern_length > 1<<match_bits) {
    throw std::invalid_argument("pattern_length doesn't fit in match_bits");
  }

  output[1] = jump_dist >> (jump_dist_bits-8);
  output[2] = (jump_dist & ((1<<(jump_dist_bits-8))-1)) << match_bits;
  output[2] = output[2] | pattern_length;
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

  
  if (encodeMode) {
    char c = '\0';
    char* search_str = (char*) malloc(max_match_len * sizeof(char));
    char* control_token;
    int buf_size = (1<<jump_dist_bits) + min_match_len;
    int pipe_down = 0;
    RingBuffer* tb = new RingBuffer(buf_size);
    RingBufferMatchResult* mr = new RingBufferMatchResult();

    while (inputFile.get(c)) {
      tb->add_chars(&c, 1);
      if (tb->get_count() >= min_match_len) {
        pipe_down = min_match_len-1;
        tb->get_substring(search_str, -min_match_len, min_match_len);
        int ret = tb->partial_search(mr, search_str, min_match_len, min_match_len);
        //int ret = tb->search(mr, search_str, min_match_len);
        if (ret == 0 && mr->jump_dist > min_match_len ) { // FIXME this second check is due to back design
          control_token = (char *) malloc(control_bytes * sizeof(char));
          //printf("searching for %c%c%c%c...", search_str[0],search_str[1],search_str[2],search_str[3]);
          //printf("FOUND @ -%d\n", loc);
          create_jump_token(control_token, mr->jump_dist-1, mr->length-min_match_len);
          outputFile.write(control_token, control_bytes);
          free(control_token);
          for (int ii = 0; ii < mr->length-1; ii++) {
            if (inputFile.get(c)) { 
              tb->add_chars(&c, 1);
            } else {
              pipe_down = ii;
              break;
            }
          }
          
        } else {
          char c = tb->get_char(tb->get_count() - min_match_len);
          if (c == control_char) {
            control_token = (char *) malloc(control_bytes * sizeof(char));
            create_jump_token(control_token, 1, 0);
            outputFile.write(control_token, control_bytes);
            free(control_token);
          } 
          else {
            outputFile.write(&c, 1);
          }
        }
      }

    }

    // Pipe down
    for (int ii = pipe_down; ii > 0; ii--) {
      char c = tb->get_char(tb->get_count() - ii);
      outputFile.write(&c, 1);
    }
  } 

  else { // decode mode
    char c = '\0';
    int buf_size = (1<<jump_dist_bits);
    char* search_str = (char*) malloc(max_match_len * sizeof(char));
    RingBuffer* tb = new RingBuffer(buf_size);
    while (inputFile.get(c)) {
      if (c == control_char) {
        inputFile.get(c);
        unsigned char ct0 =  c;
        inputFile.get(c);
        unsigned char ct1 =  c;

        
        int jump_dist = ct0 << (jump_dist_bits-8) | (ct1 >> match_bits);
        int match_len = (ct1 & ((1<<match_bits)-1)) + min_match_len;

        if (jump_dist == 0x01) {
          c = control_char;
          tb->add_chars(&c, 1);
          outputFile.write(&c, 1);
        } 
        else { 
         
          tb->get_substring(search_str, -(jump_dist+1), match_len);
          //printf("inserting for %c%c%c%c...\n", search_str[0],search_str[1],search_str[2],search_str[3]);
          
          tb->add_chars(search_str, match_len);
          outputFile.write(search_str, match_len);
        }
      }
      else {
        tb->add_chars(&c, 1);
        outputFile.write(&c, 1);
      }
    } 
  }
    


  inputFile.close();
  outputFile.close();

  return 0;
};
