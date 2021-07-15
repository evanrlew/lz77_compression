#A LZ compression program. It seems to work well.

##Build
make

##Test:
./test.sh





##Progress Notes
Struggling to find a nice (fast) way to back search the text. Ultimately I would like to explore speed ups using bloom filters and boyer-moore search, but for now I'm going to start with a brute force search. Currently things are complicated because of:
  * ring buffer
  * search greediness
  * how to handle partially full buffer

2021 July 5:
I have a working prototype (with bugs). Very naive implementation. Notable lacking features:
  * variable length pattern match
  * better (faster) search algo
  * cleaner ring buffer. Need a simple way to index from head and tail of the buffer.
  * Better string management. Current method is based on c strings (but worse) and is very fragile.

2021 July 10:
I updated the ring buffer api to support indexing from tail and head which cleans up the usage a lot. I also decided against using the cpp strings and I'm instead going to the thing about the ring buffer content as chunks of data.

2021 July 13:
Working with variable length matching.
