# Coding Challenge #53 - Bloom Filter Spell Checker
This challenge is to build your own micro spell checker using a Bloom filter.
[JOHN CRICKETT](https://substack.com/redirect/fb7ccef3-d8d3-4d9e-8bd0-406e0dbc2922?j=eyJ1Ijoic201MDUifQ.EKNBcC3593op0wK31sVD6SABX4KbsYz3BcjFPwu9Qbc)
MAR 16
# Build on Macos
First dump the dict words in the code directory:
```sh
cat /usr/share/dict/words >> dict.txt
```

Next compile code:
```sh
clang++ -std=c++2a -Wall bloom.cpp
```

Run the code and test:
```sh
./a.out
Number of bits: 2261843 Number of hash fns: 6
Magic: CCBF      n_hashfns: 6 bitset_size: 2261843
Matched Deserialized Bitsets!!
```

```sh
ls -lh
total 5768
-rw-r--r--  1 anubhavrakshit  staff   5.0K Mar 16 20:17 README.md
-rwxr-xr-x  1 anubhavrakshit  staff   111K Mar 17 15:54 a.out
-rw-r--r--  1 anubhavrakshit  staff   4.7K Mar 17 15:54 bloom.cpp
-rw-r--r--@ 1 anubhavrakshit  staff   2.4M Mar 16 20:02 dict.txt
-rw-r--r--  1 anubhavrakshit  staff   276K Mar 17 16:03 words.bf
```

# The Challenge - Building A Simple Spell Checker Using a Bloom Filter
In this challenge you’re going to implement a Bloom filter, and insert thousands of words from a dictionary. You’ll then test other words against the dictionary to see if they’re probably spelt correctly.

## Step Zero
In this step you’re going to set your environment up ready to begin developing and testing your solution. I’ll leave you to setup your IDE / editor of choice and programming language of choice for building command line tools.

If you’re not on a Unix like operating system you’ll need to get a dictionary file for testing.

If you are on a Unix-like OS you should be able to generate one by running the command:
```sh
% cat /usr/share/dict/words >> dict.txt
```
## Step 1
In this step your goal is to use test driven development (TDD) to develop the Bloom filter data structure. OK you don’t have to use TDD, but I’d suggest writing some tests either before or after writing the Bloom filter, both to test that it works correctly and to help you understand how it works.

To implement a Bloom filter:

1. Determine the number of items that are likely to be stored and the probability of false positives you system can tolerate then use that to determine the memory requirements and number of has functions needed. There’s a formula, to use for this.
[From Stackoverflow] (https://stackoverflow.com/questions/658439/how-many-hash-functions-does-my-bloom-filter-need)
2. Create a bit array and set all bits to zero.

3. Insert items by applying the hash functions and setting the corresponding bits to one.

4. Query for the presence of an item by applying the hash functions. If any of the corresponding bits are zero, the item is definitely not in the set. If all of the bits are one, the item is probably in the set.

To determine the how many hash functions you should use and the interaction between the number of bits, the number of items and the number of hash functions there are a set of formulas which you’ll find documented on the [Bloom filter Wikipedia page](https://substack.com/redirect/3b570b5a-a197-4174-aea7-a0a51836b11a?j=eyJ1Ijoic201MDUifQ.EKNBcC3593op0wK31sVD6SABX4KbsYz3BcjFPwu9Qbc).

You can use hash functions from your programming languages standard library, implement your own or implement an existing one yourself. For the latter I suggest you check out the [Fowler–Noll–Vo hash function](https://substack.com/redirect/d5cec92a-42bf-4a5a-a0be-0849f7287f82?j=eyJ1Ijoic201MDUifQ.EKNBcC3593op0wK31sVD6SABX4KbsYz3BcjFPwu9Qbc) and implement a couple of the versions of it. It’s a quick and easy hash function to implement if you’ve never written one before.

## Step 2
In this step your goal is to read the dictionary file and insert the words into the Bloom filter.

That might look like this:
```sh
% ccspellcheck -build dict.txt
```
And result in a file being saved that contains the dictionary. I called mine words.bf.

## Step 3
In this step your goal is to save the Bloom filter to disk. As the Bloom filter is binary we might like to add a simple header:

- The first four bytes will be an identifier, we’ll use CCBF.

- The next two bytes will be a version number to describe the version number of the file.

- The next two bytes will be the number of hash functions used.

- The next four bytes will be the number of bits used for the filter.

All in big endian. After that header, the bit array will be written out.

So when done, running the command from the previous step should result in a file being saved that contains the dictionary. I called mine words.bf.

You can use something like xxd to view a hex dump of the file and check your header, i.e.:

```sh
% xxd -l 32 words.bf                                                     
00000000: 4343 4246 0001 0004 003d 0900 6002 3104  CCBF.....=..`.1.
00000010: 0d40 2902 3095 0008 88a2 6010 0820 4201  .@).0.....`.. B.
```
Here we have the identifier, version 1, using 4 hash functions and 0x003d0900 bits.

Do note how small the saved Bloom filter file is compared to the input dictionary. For my implementation the numbers look like this:

```sh
% du -h dict.txt words.bf 
2.4M    dict.txt
492K    words.bf
```
## Step 4
In this step your goal is to load the Bloom filter from disk. You’ll need to read the header - you should validate the file is of the type and version expected.

## Step 5
In this step your goal is to test words provided on the command line to see if they’re probably spelt right.
```sh
% ccpsellcheck hi hello word concurrency coding challenges imadethis up
These words are spelt wrong:
 coding
 challenges
 imadethis
```
It’s disappointing that ‘coding’ and ‘challenges’ aren’t in the dictionary I picked for Coding Challenges, but there we go, life is like that! 🤷‍♂️.