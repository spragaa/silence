refer to aes docs: 
https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.197-upd1.pdf

and article about pkcs7 padding:
https://r3zz.io/posts/pkcs7-padding/

## initialization
1. generate random 256 bits key - secret
2. generate random 128 bits initialization vector (IV) - can be transmitted openly

## encryptions steps:
0. use key and IV
1. convert input message into bytes
2. add padding (pkcs7)
3. use aes to encrypt the padded input message -> encrypted message
  3.1 expand the 256 bits key into a total of 15 round keys
  3.2
  
  
cipher() - sequence of fixed transformations of the round. Each round requires the additional input called round key (16 bytes).
cipher(input_message, number_of_rounds, key_expnasion(key))

key_expnasion() - takes the block cipher key and generates the round keys.

aes_128, aes_190 and aes_256 differn in 3 aspects: length of the key, number of rounds and expansion algo.

I want to implement aes_256:
1. key_length = 256 bits (Nk)
2. block_size = 128 bits (Nb)
3. number_of_rounds = 14 (Nr)

aes-256(input, key) = cipher(input, 14, key_expnasion(key))
  
### key_expansion()
takes key and generated the 4 * (number_of_rounds + 1) words. 

for aes_256: 
key_expansion(uint32_t key) -> w[i], 0 <=i <=4 * (number_of_rounds + 1), where w[i] is an array of words (round keys)

### cipher()


## decryption steps:
0. use same key and IV
1. convert input message into bytes
2. use aes to decrypt the message
3. unpadding (pkcs7)
4. convert decrypted message back to string


## Constants
### Round constants:
Table 5. Round constants

j    Rcon[ j]         j     Rcon[ j]
1    [01,00,00,00]    6     [20,00,00,00]
2    [02,00,00,00]    7     [40,00,00,00]
3    [04,00,00,00]    8     [80,00,00,00]
4    [08,00,00,00]    9     [1b,00,00,00]
5    [10,00,00,00]    10    [36,00,00,00]