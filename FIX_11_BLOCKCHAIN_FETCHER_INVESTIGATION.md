# Fix #11: Blockchain Fetcher Duplicate Block Investigation

## Issue Reported
User reported that during training via `train_on_historical_blocks` method, the `self.blockchain_fetcher.fetch_block_by_height` function always returns the same block, making training ineffective.

## Investigation Results

### 1. Initial Analysis
Reviewed the blockchain fetcher implementation in `bitcoin_real_data_validator.py`:
- `BitcoinBlockchainFetcher` class fetches blocks from blockchain.info API
- No caching mechanism found in the original implementation
- The API request structure looked correct

### 2. Testing Phase
Created comprehensive tests to verify the fetcher behavior:

**Test 1: Direct Fetcher Test**
```bash
# Fetched blocks 870000-870002
Result: ✅ Each block had unique nonce - NO DUPLICATES
```

**Test 2: Training Loop Simulation**
```bash
# Simulated training loop fetching blocks 870000-870009
Result: ✅ All 10 blocks unique with different nonces
- Block 870000: 0xa15f5978
- Block 870001: 0x41c85f24
- Block 870002: 0x3f38707d
- Block 870003: 0xb075495f
- Block 870004: 0x482c8f24
- Block 870005: 0x762e0d76
- Block 870006: 0x0282c0b0
- Block 870007: 0xacd5442e
- Block 870008: 0x5bd05856
- Block 870009: 0xc46fdc0f
```

### 3. Root Cause Analysis

**Finding**: The blockchain fetcher is working correctly and returning unique blocks.

**Possible Causes of User's Observation**:

1. **API Rate Limiting**: blockchain.info API might have been rate-limiting requests, causing failed fetches that fell back to cached/stale data
2. **Network Errors**: Transient network issues could cause failed fetches without proper retry logic
3. **Logging Confusion**: Similar log output might have made different blocks look identical
4. **Browser Cache**: Frontend might have been displaying cached training results

## Improvements Made

### 1. Enhanced Blockchain Fetcher (`bitcoin_real_data_validator.py`)

#### A. Duplicate Detection System
```python
def __init__(self, ...):
    self._last_fetched_height = None  # Track last successful fetch
    self._cache_buster = 0  # Simple cache buster counter
```

#### B. Comprehensive Logging
- Added fetch attempt logging with counter
- Added duplicate request detection
- Added block height verification
- Logs requested vs received height

#### C. Retry Logic with Exponential Backoff
```python
max_retries = 3
retry_delay = 2.0

# Retry on:
# - Timeout errors
# - HTTP 429 (rate limiting) with extended wait
# - Other transient errors
```

#### D. Cache Busting
```python
url = f"https://blockchain.info/block-height/{height}?format=json&cors=true&_={self._cache_buster}"
```

#### E. Height Validation
```python
# Verify height matches requested
if data.get('height') != height_or_latest:
    logger.error(f"Height mismatch: requested {height_or_latest}, got {data.get('height')}")
    return None
```

### 2. Enhanced Training Loop (`historical_bitcoin_trainer.py`)

#### A. Duplicate Block Detection
```python
# Track blocks to detect duplicates
seen_blocks = {}

for i in range(count):
    # ... fetch block ...
    
    # DUPLICATE DETECTION
    block_key = f"{block.hash}_{block.nonce}"
    if block_key in seen_blocks:
        logger.error(f"❌ DUPLICATE BLOCK DETECTED!")
        logger.error(f"   Previously seen in iteration {seen_blocks[block_key]}")
    else:
        seen_blocks[block_key] = i+1
        logger.info(f"✅ New unique block: height={block.height}")
```

## Testing Results

### Before Fixes
- No detection of duplicate blocks
- No retry logic for failed requests
- No cache busting
- Limited logging

### After Fixes
- ✅ Comprehensive duplicate detection with alerts
- ✅ Retry logic with 3 attempts and exponential backoff
- ✅ Cache busting parameter in URLs
- ✅ Detailed logging of all fetch attempts
- ✅ Height verification to ensure correct block received

## Verification Commands

### Test Blockchain Fetcher Directly
```bash
cd /home/user/webapp
python3 test_training_blockchain_fetch.py
```

### Test Training with Enhanced Logging
```bash
cd /home/user/webapp
# Start training via web UI - check logs for:
# "🔍 Fetching block at height X (fetch #Y)"
# "✅ New unique block: height=X"
```

## Monitoring

When training is active, watch for these log patterns:

**Good Pattern (No Duplicates)**:
```
🔍 Fetching block at height 870000 (fetch #1)
✅ Successfully fetched block 870000: hash=0000..., nonce=0xa15f5978
✅ New unique block: height=870000
```

**Bad Pattern (Duplicates - Should Not Happen Now)**:
```
🔍 Fetching block at height 870001 (fetch #2)
⚠️ DUPLICATE REQUEST: Fetching same height 870001 twice in a row!
❌ DUPLICATE BLOCK DETECTED!
```

## API Rate Limiting

blockchain.info API has rate limits. The improvements handle this:

1. **Retry on 429**: When rate limited, wait 4 seconds before retry
2. **Timeout Handling**: Increased timeout from 10s to 15s
3. **Rate Limiting Delay**: 1.5s delay between training iterations

## Next Steps

1. **Monitor Training Logs**: Check if duplicates are still reported
2. **If Still Seeing Duplicates**: 
   - Check if different API should be used (Blockchair, Blockcypher)
   - Consider implementing local block cache
   - Investigate if blockchain.info API has issues

## Files Modified

1. `bitcoin_real_data_validator.py`:
   - Added `_last_fetched_height` and `_cache_buster` tracking
   - Enhanced `fetch_block_by_height()` with logging and validation
   - Added retry logic to `_fetch_from_blockchain_info()`
   - Added cache busting parameter to URLs

2. `historical_bitcoin_trainer.py`:
   - Added `seen_blocks` dictionary for duplicate detection
   - Enhanced logging to show unique block confirmations
   - Added duplicate detection alerts

3. `test_training_blockchain_fetch.py`:
   - Created comprehensive test simulating training loop
   - Tests duplicate detection and unique block verification

## Conclusion

**Status**: ✅ **Improvements Deployed**

The blockchain fetcher is working correctly in isolated tests. The improvements add:
- Comprehensive duplicate detection
- Better error handling and retry logic
- Enhanced logging for debugging
- Cache busting to prevent stale data

**If the user still observes duplicates after these changes**, the detailed logging will help identify the exact cause (API issues, network problems, or actual code bugs).

## Git Commit Message
```
fix(training): enhance blockchain fetcher with duplicate detection and retry logic

- Add duplicate block detection in training loop with alerts
- Implement retry logic with exponential backoff (3 attempts)
- Add cache busting parameters to API URLs
- Enhanced logging for fetch attempts and validation
- Add height verification to ensure correct blocks
- Handle rate limiting (HTTP 429) with extended delays
- Track last fetched height to detect consecutive duplicates
- Increase API timeout from 10s to 15s

Addresses user report of duplicate blocks during training.
Testing shows fetcher works correctly with unique blocks.
Enhanced logging will help identify any remaining issues.
```
