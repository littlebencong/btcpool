/*
 The MIT License (MIT)

 Copyright (c) [2016] [BTC.COM]

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */
#ifndef STRATUM_BITCOIN_H_
#define STRATUM_BITCOIN_H_

#include "Stratum.h"
#include "CommonBitcoin.h"

#include <uint256.h>
// #include <base58.h>
#include "rsk/RskWork.h"
#include "script/standard.h"


//
// max coinbase tx size, bytes
// Tips: currently there is only 1 input and 1, 2 or 3 output (reward, segwit and RSK outputs),
//       so 500 bytes may enough.
#define COINBASE_TX_MAX_SIZE   500

class ShareBitcoin : public ShareBase
{
public:

  const static uint32_t CURRENT_VERSION = 0x00010002u; // first 0001: bitcoin, second 0002: version 2.

  uint64_t jobId_     = 0;
  uint64_t shareDiff_ = 0;
  uint32_t blkBits_   = 0;
  uint32_t height_    = 0;
  uint32_t nonce_     = 0;
  uint32_t sessionId_ = 0;

  ShareBitcoin() = default;
  ShareBitcoin(const ShareBitcoin &r) = default;
  ShareBitcoin &operator=(const ShareBitcoin &r) = default;

  double score() const
  {
    if (shareDiff_ == 0 || blkBits_ == 0)
    {
      return 0.0;
    }

    double networkDifficulty = 0.0;
    BitsToDifficulty(blkBits_, &networkDifficulty);

    // Network diff may less than share diff on testnet or regression test network.
    // On regression test network, the network diff may be zero.
    // But no matter how low the network diff is, you can only dig one block at a time.
    if (networkDifficulty < (double)shareDiff_)
    {
      return 1.0;
    }

    return (double)shareDiff_ / networkDifficulty;
  }

  uint32_t checkSum() const {
    uint64_t c = 0;

    c += (uint64_t) version_;
    c += (uint64_t) workerHashId_;
    c += (uint64_t) userId_;
    c += (uint64_t) status_;
    c += (uint64_t) timestamp_;
    c += (uint64_t) ip_.addrUint64[0];
    c += (uint64_t) ip_.addrUint64[1];
    c += (uint64_t) jobId_;
    c += (uint64_t) shareDiff_;
    c += (uint64_t) blkBits_;
    c += (uint64_t) height_;
    c += (uint64_t) nonce_;
    c += (uint64_t) sessionId_;

    return ((uint32_t) c) + ((uint32_t) (c >> 32));
  }

  bool isValid() const
  {
    if (version_ != CURRENT_VERSION) {
      return false;
    }

    if (checkSum_ != checkSum()) {
      DLOG(INFO) << "checkSum mismatched! checkSum_: " << checkSum_ << ", checkSum(): " << checkSum();
      return false;
    }

    if (jobId_ == 0 || userId_ == 0 || workerHashId_ == 0 ||
        height_ == 0 || blkBits_ == 0 || shareDiff_ == 0)
    {
      return false;
    }
    
    return true;
  }

  string toString() const
  {
    double networkDifficulty = 0.0;
    BitsToDifficulty(blkBits_, &networkDifficulty);

    return Strings::Format("share(jobId: %" PRIu64 ", ip: %s, userId: %d, "
                           "workerId: %" PRId64 ", time: %u/%s, height: %u, "
                           "blkBits: %08x/%lf, nonce: %08x, sessionId: %08x, shareDiff: %" PRIu64 ", "
                           "status: %d/%s)",
                           jobId_, ip_.toString().c_str(), userId_,
                           workerHashId_, timestamp_, date("%F %T", timestamp_).c_str(), height_,
                           blkBits_, networkDifficulty, nonce_, sessionId_, shareDiff_,
                           status_, StratumStatus::toString(status_));
  }
};

class StratumJobBitcoin : public StratumJob
{
public:
  int32_t height_;
  int32_t nVersion_;
  uint32_t nTime_;

  uint32_t nBits_;
  uint32_t minTime_;
  uint256 networkTarget_;

  // rsk merged mining
  string blockHashForMergedMining_;
  uint256 rskNetworkTarget_;
  string rskdRpcAddress_;
  string rskdRpcUserPwd_;
  string feesForMiner_;
  bool isMergedMiningCleanJob_;


  string gbtHash_; // gbt hash id
  uint256 prevHash_;
  string prevHashBeStr_; // little-endian hex, memory's order
  string coinbase1_;
  string coinbase2_;
  vector<uint256> merkleBranch_;

  int64_t coinbaseValue_;
  // if segwit is not active, it will be empty
  string witnessCommitment_;
#ifdef CHAIN_TYPE_UBTC
  // if UB smart contract is not active, it will be empty
  string   rootStateHash_;
#endif


  // namecoin merged mining
  uint32_t nmcAuxBits_;
  uint256 nmcAuxBlockHash_;
  int32_t nmcAuxMerkleSize_;
  int32_t nmcAuxMerkleNonce_;
  uint256 nmcNetworkTarget_;
  int32_t nmcHeight_;
  string nmcRpcAddr_;
  string nmcRpcUserpass_;

public:
  StratumJobBitcoin();
  bool initFromGbt( const char *gbt, const string &poolCoinbaseInfo,
                    const CTxDestination &poolPayoutAddr,
                    const uint32_t blockVersion,
                    const string &nmcAuxBlockJson,
                    const RskWork &latestRskBlockJson,
                    const uint8_t serverId,
                    const bool isMergedMiningUpdate);
  string serializeToJson() const override;
  bool unserializeFromJson(const char *s, size_t len) override;
  bool isEmptyBlock();

};

#endif
