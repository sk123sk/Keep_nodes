import json
import time
from web3 import Web3
import sys

web3 = Web3(Web3.HTTPProvider('https://ropsten.infura.io/v3/'+sys.argv[3]))
encrypted_key = sys.argv[1]
key_password = sys.argv[2]
_privateKey = web3.eth.account.decrypt(encrypted_key, password=key_password)
user_account = web3.eth.account.privateKeyToAccount(_privateKey)

def prepareStakingContract():
    with open('TokenStaking.json') as ts:
        tokenStaking = json.load(ts)
    tokenStaking_abi = tokenStaking["abi"]
    stakingContract = web3.eth.contract(address=tokenStaking["networks"]["3"]["address"], abi=tokenStaking_abi)
    return stakingContract

def prepareBeaconContract():
    with open('KeepRandomBeaconOperator.json') as ts:
        beacon = json.load(ts)
    beacon_abi = beacon["abi"]
    beaconContract = web3.eth.contract(address=beacon["networks"]["3"]["address"], abi=beacon_abi)
    return beaconContract


def beacon_auth(attempt):
    stakingContract = prepareStakingContract()
    beaconContract = prepareBeaconContract()
    nonce = web3.eth.getTransactionCount(user_account.address)
    gasPrice = web3.eth.gasPrice
    txn = stakingContract.functions.authorizeOperatorContract(user_account.address, beaconContract.address).buildTransaction({
        'chainId': 3,
        'gas': 500000,
        'gasPrice': gasPrice*2 if gasPrice > 200000000000 else 200000000000,
        'value': 0,
        'nonce': nonce,
    })
    signed_txn = web3.eth.account.signTransaction(txn, private_key=_privateKey)
    txn_hash = web3.eth.sendRawTransaction(signed_txn.rawTransaction)
    time.sleep(15)
    try:
        txn_receipt = web3.eth.waitForTransactionReceipt(txn_hash, timeout=200)
    except:
        print('beacon_auth failed. Confirmation time out')
    else:
        if txn_receipt.status == 0:
            print('beacon_auth failed. Reverted.')
        else:
            print("transaction confirmed")

beacon_auth(0)