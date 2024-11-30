#pragma once

#include "debug.hpp"
#include "crypto_utils.hpp"
#include "aes256.hpp"
#include "el_gamal_encryption.hpp"
#include "dsa.hpp"

#include <memory>

namespace crypto {

/*
    workflow:
 + 0. generate dsa and el gamal key pairs

    sender:
        1. when first message is initialized between two users, they first should exchange their public keys
        2. generate aes key for particular receiver
        3. encrypt aes key using el gamal public key of receiver
        4. create hash of encrypted message
        5. sign encrypted messsage hash using dsa, where message is c1 || c2  

        -> (first) SecureMessage (el gamal encypted aes key, aes encypted message, dsa signature)

        then in case of new messages:
            6. ecnrypt aes using existing aes key
            7. hash the message and sign it
            -> SecureMessage (aes encypted message, dsa signature)


    receiver:
        1. decrypt aes key using private key
        2. decrypt message itself using aes
        3. verify signature

        then in case of new messages:
            4. decypt message using existing aes
            5. verify signature




    considerations:
    - define key lifetime and key rotation policies
    - ?? cache validated public keys
    - session based keys? -> how to handle receiving messages while offline?



    std::string get_public_key(); // note: return string to use with nlohman json
    verify_signature();           // runs dsa signature verification
    add_user(id, public_key)
    encrypt()
    decrypt()
 */

class HybridCryptoSystem {
public:
	using cpp_int = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<>, boost::multiprecision::et_off>;
	HybridCryptoSystem();

	cpp_int get_el_gamal_public_key() const;
	cpp_int get_dsa_public_key() const;
	EncryptedMessage encrypt_aes_key(const std::string& recipients_public_key) const;
	DSASignature sign(const cpp_int& message_hash);
	
private:

protected:

private:
	std::unique_ptr<AES256> _aes;
	std::unique_ptr<ElGamalEncryption> _el_gamal;
	std::unique_ptr<DSA> _dsa;

	// list of public keys of other users. should it be here or somewhere in the client itself?
	// actually, this might be the tuple (receiver_id, dsa_public_key, el_gamal_public_key, aes_key) -> new struct smth like ...
};


} // namespace crypto
