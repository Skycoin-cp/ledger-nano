#include "sky.h"

/** the length of a SHA256 hash */
#define SHA256_HASH_LEN 32

/** the length of a RIPMD160 hash */
#define RIPMD_HASH_LEN 20

/** checksum is first 4 bytes of sha256 */
#define CHECKSUM_LEN 4

/** length of address (RIPMD160 hash + VERSION byte + checksum) */
#define ADDRESS_LEN (RIPMD_HASH_LEN + 1 + CHECKSUM_LEN)

/** length can be between 27 and 34 bytes + \0 */
#define ADDRESS_BASE58_LEN 35

/** the current version in the address field */
#define ADDRESS_VERSION 0

/** Label when a public key has not been set yet */
static const char NO_PUBLIC_KEY_0[] = "No Public Key";
static const char NO_PUBLIC_KEY_1[] = "Requested Yet";


/** array of base58 alphabet letters */
static const char BASE_58_ALPHABET[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

/** blank string, used for clearing a line of text */
static const char TXT_BLANK[] = "                 ";

int encode_base_58(const unsigned char *pbegin, int len, char *result) {
    const unsigned char *pend = pbegin + len;
    // Skip & count leading zeroes.
    int zeroes = 0;
    int length = 0;
    while (pbegin != pend && *pbegin == 0) {
        pbegin++;
        zeroes++;
    }
    unsigned char b58[35];
    for (int i = 0; i < 35; i++) {
        b58[i] = 0;
    }
    int size = len * 138 / 100 + 1;
    // Found all remainders
    while (pbegin != pend) {
        int carry = *pbegin;
        int i = 0;
        // Apply "b58 = b58 * 256 + ch".
        for (int j = size - 1; (carry != 0 || i < length) && j != -1; j--, i++) {
            carry += (b58[j] * 256);
            b58[j] = carry % 58;
            carry /= 58;
        }

        length = i;
        pbegin++;
    }
    // Skip leading zeroes in base58 result.
    int j = size - length;
    while (j != size && b58[j] == 0)
        j++;
    // Translate the result into a string.
    int i = 0;
    while (i != size)
        result[i++] = BASE_58_ALPHABET[b58[j++]];
    result[i] = '\0';
}

static void to_address(const unsigned char *public_key_compressed, char *result) {
    /*
     * Address format 1+20=21 bytes:
     *  Version byte + 20 byte RIPMD160(SHA256(SHA256(compressed public key)))
     *
     * Base 58 address format 20+1+4 bytes:
     *	20 bytes RIPMD(...) + Version byte + 4 checksum bytes ( 4 bytes of SHA256 of the previous 21 bytes)
     *
     */
    static cx_sha256_t address_hash;
    static cx_ripemd160_t address_rip;

    unsigned char address_hash_result_0[SHA256_HASH_LEN];
    unsigned char address_hash_result_1[SHA256_HASH_LEN];

    // do a sha256 hash of the public key twice.
    cx_sha256_init(&address_hash);
    cx_hash(&address_hash.header, CX_LAST, public_key_compressed, 33, address_hash_result_0);
    cx_sha256_init(&address_hash);
    cx_hash(&address_hash.header, CX_LAST, address_hash_result_0, SHA256_HASH_LEN, address_hash_result_1);

    // do a RIPMD160 to sha256(sha256(public key))
    unsigned char address_ripmd_hash[RIPMD_HASH_LEN];
    cx_ripemd160_init(&address_rip);
    cx_hash(&address_rip.header, CX_LAST, address_hash_result_1, SHA256_HASH_LEN, address_ripmd_hash);

    // add version to address
    os_memmove(address, address_ripmd_hash, RIPMD_HASH_LEN);
    address[20] = ADDRESS_VERSION;


    // add checksum to address
    unsigned char checksum[SHA256_HASH_LEN];
    cx_sha256_init(&address_hash);
    cx_hash(&address_hash.header, CX_LAST, address, 21, checksum);
    os_memmove(address + 21, checksum, 4);

    //encode address to base58
    encode_base_58(address, ADDRESS_LEN, result);
}

void display_address(const unsigned char *public_key) {
    // convert public key from uncompressed to compressed
    unsigned char public_key_compressed[33];
    
    // check parity and add appropriate prefix
    public_key_compressed[0] = ((public_key[64] & 1) ? 0x03 : 0x02);
    os_memmove(public_key_compressed + 1, public_key, 32);

    unsigned char address_base58[ADDRESS_BASE58_LEN];
    to_address(public_key_compressed, address_base58);

    os_memmove(address, address_base58, ADDRESS_BASE58_LEN);
}
