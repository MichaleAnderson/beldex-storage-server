#include "beldexd_key.h"

#include "beldex_logger.h"

#include <cstring>
#include <type_traits>

#include <sodium.h>
#include <bmq/base32z.h>
#include <bmq/base64.h>
#include <bmq/hex.h>

namespace beldex {

namespace detail {

void load_from_hex(void* buffer, size_t length, std::string_view hex) {
    if (!bmq::is_hex(hex))
        throw std::runtime_error{"Hex key data is invalid: data is not hex"};
    if (hex.size() != 2*length)
        throw std::runtime_error{
            "Hex key data is invalid: expected " + std::to_string(length) +
                " hex digits, received " + std::to_string(hex.size())};
    bmq::from_hex(hex.begin(), hex.end(), reinterpret_cast<unsigned char*>(buffer));
}

void load_from_bytes(void* buffer, size_t length, std::string_view bytes) {
    if (bytes.size() != length)
        throw std::runtime_error{
            "Key data is invalid: expected " + std::to_string(length) +
                " bytes, received " + std::to_string(bytes.size())};
    std::memmove(buffer, bytes.data(), length);
}

std::string to_hex(const unsigned char* buffer, size_t length) {
    return bmq::to_hex(buffer, buffer + length);
}

}

std::string ed25519_pubkey::mnode_address() const {
    auto addr = bmq::to_base32z(begin(), end());
    addr += ".mnode";
    return addr;
}

legacy_pubkey legacy_seckey::pubkey() const {
    legacy_pubkey pk;
    crypto_scalarmult_ed25519_base_noclamp(pk.data(), data());
    return pk;
};
ed25519_pubkey ed25519_seckey::pubkey() const {
    ed25519_pubkey pk;
    crypto_sign_ed25519_sk_to_pk(pk.data(), data());
    return pk;
};
x25519_pubkey x25519_seckey::pubkey() const {
    x25519_pubkey pk;
    crypto_scalarmult_curve25519_base(pk.data(), data());
    return pk;
};

template <typename T>
static T parse_pubkey(std::string_view pubkey_in) {
    T pk{};
    static_assert(pk.size() == 32);
    if (pubkey_in.size() == 32)
        detail::load_from_bytes(pk.data(), 32, pubkey_in);
    else if (pubkey_in.size() == 64 && bmq::is_hex(pubkey_in))
        bmq::from_hex(pubkey_in.begin(), pubkey_in.end(), pk.begin());
    else if ((pubkey_in.size() == 43 || (pubkey_in.size() == 44 && pubkey_in.back() == '='))
            && bmq::is_base64(pubkey_in))
        bmq::from_base64(pubkey_in.begin(), pubkey_in.end(), pk.begin());
    else if (pubkey_in.size() == 52 && bmq::is_base32z(pubkey_in))
        bmq::from_base32z(pubkey_in.begin(), pubkey_in.end(), pk.begin());
    else {
        BELDEX_LOG(warn, "Invalid public key: not valid bytes, hex, b64, or b32z encoded");
        BELDEX_LOG(debug, "Received public key encoded value of size {}: {}", pubkey_in.size(), pubkey_in);
    }
    return pk;
}

legacy_pubkey parse_legacy_pubkey(std::string_view pubkey_in) {
    return parse_pubkey<legacy_pubkey>(pubkey_in);
}
ed25519_pubkey parse_ed25519_pubkey(std::string_view pubkey_in) {
    return parse_pubkey<ed25519_pubkey>(pubkey_in);
}
x25519_pubkey parse_x25519_pubkey(std::string_view pubkey_in) {
    return parse_pubkey<x25519_pubkey>(pubkey_in);
}

} // namespace beldex
