#include "beldex_common.h"
#include <bmq/hex.h>

namespace beldex {

user_pubkey_t& user_pubkey_t::load(std::string_view pk) {
    if (pk.size() == USER_PUBKEY_SIZE_HEX && bmq::is_hex(pk)) {
        uint8_t netid;
        bmq::from_hex(pk.begin(), pk.begin() + 2, &netid);
        network_ = netid;
        pubkey_ = bmq::from_hex(pk.substr(2));
    } else if (pk.size() == USER_PUBKEY_SIZE_BYTES) {
        network_ = static_cast<uint8_t>(pk.front());
        pubkey_ = pk.substr(1);
    } else if (!is_mainnet && pk.size() == USER_PUBKEY_SIZE_HEX - 2 && bmq::is_hex(pk)) {
        network_ = 5;
        pubkey_ = bmq::from_hex(pk);
    } else if (!is_mainnet && pk.size() == USER_PUBKEY_SIZE_BYTES - 1) {
        network_ = 5;
        pubkey_ = pk;
    } else {
        network_ = -1;
        pubkey_.clear();
    }
    return *this;
}

std::string user_pubkey_t::hex() const {
    return bmq::to_hex(pubkey_);
}

std::string user_pubkey_t::prefixed_hex() const {
    std::string hex;
    if (pubkey_.empty())
        return hex;
    hex.reserve(USER_PUBKEY_SIZE_HEX);
    auto bi = std::back_inserter(hex);
    if (uint8_t netid = type(); !(netid == 0 && !is_mainnet))
        bmq::to_hex(&netid, &netid+1, bi);
    bmq::to_hex(pubkey_.begin(), pubkey_.end(), bi);
    return hex;
}

std::string user_pubkey_t::prefixed_raw() const {
    std::string bytes;
    if (pubkey_.empty())
        return bytes;
    bytes.reserve(1 + USER_PUBKEY_SIZE_BYTES);
    bytes += static_cast<uint8_t>(network_);
    bytes += pubkey_;
    return bytes;
}

}
