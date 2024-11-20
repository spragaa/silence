#include "hybrid_crypto_system.hpp"

namespace common {
namespace crypto {

HybridCryptoSystem::HybridCryptoSystem() {
    // p and g should not be hardcoded, but should be same for each client?
    // they should be the same accross all network?
    cpp_int el_gamal_p = hex_to_cpp_int(
		"ffffffffffffffffc90fdaa22168c234c4c6628b80dc1cd129024e088a67cc74"
		"020bbea63b139b22514a08798e3404ddef9519b3cd3a431b302b0a6df25f1437"
		"4fe1356d6d51c245e485b576625e7ec6f44c42e9a637ed6b0bff5cb6f406b7ed"
		"ee386bfb5a899fa5ae9f24117c4b1fe649286651ece45b3dc2007cb8a163bf05"
		"98da48361c55d39a69163fa8fd24cf5f83655d23dca3ad961c62f356208552bb"
		"9ed529077096966d670c354e4abc9804f1746c08ca18217c32905e462e36ce3b"
		"e39e772c180e86039b2783a2ec07a28fb5c55df06f4c52c9de2bcbf695581718"
		"3995497cea956ae515d2261898fa051015728e5a8aacaa68ffffffffffffffff"
	);
	cpp_int el_gamal_g = cpp_int(11);
	
	_el_gamal = std::make_unique<ElGamalEncryption>(el_gamal_p, el_gamal_g);
	
	// same for dsa
	cpp_int dsa_p = hex_to_cpp_int(
        "8df2a494492276aa3d25759bb06869cbeac0d83afb8d0cf7cbb8324f0d7882e5"
        "d0762fc5b7210eafc2e9adac32ab7aac49693dfbf83724c2ec0736ee31c80291"
	);
    cpp_int dsa_q = hex_to_cpp_int(
        "c773218c737ec8ee993b4f2ded30f48edace915f"
    );
    cpp_int dsa_g = hex_to_cpp_int(
        "626d027839ea0a13413163a55b4cb500299d5522956cefcb3bff10f399ce2c2e"
        "71cb9de5fa24babf58e5b79521925c9cc42e9f6f464b088cc572af53e6d78802"
    );

	_dsa = std::make_unique<DSA>(dsa_p, dsa_q, dsa_g);

	INFO_MSG("[HybridCryptoSystem::HybridCryptoSystem] System initialization finished!");
}


cpp_int HybridCryptoSystem::get_el_gamal_public_key() const {
    return _el_gamal->get_public_key();
}

cpp_int HybridCryptoSystem::get_dsa_public_key() const {
    return _dsa->get_public_key();  
}

} // namespace crypto
} // namespace common
