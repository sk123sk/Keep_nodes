import json
import time
from web3 import Web3
import sys

web3 = Web3(Web3.HTTPProvider('https://ropsten.infura.io/v3/'+sys.argv[3]))
encrypted_key = sys.argv[1]
key_password = sys.argv[2]
_privateKey = web3.eth.account.decrypt(encrypted_key, password=key_password)
user_account = web3.eth.account.privateKeyToAccount(_privateKey)

def prepareEcdsaContract():
    with open('BondedECDSAKeepFactory.json') as ts:
        ecdsa = json.load(ts)
    ecdsa_abi = ecdsa["abi"]
    ecdsaContract = web3.eth.contract(address=ecdsa["networks"]["3"]["address"], abi=ecdsa_abi)
    return ecdsaContract

def prepareStakingContract():
    with open('TokenStaking.json') as ts:
        tokenStaking = json.load(ts)
    tokenStaking_abi = tokenStaking["abi"]
    stakingContract = web3.eth.contract(address=tokenStaking["networks"]["3"]["address"], abi=tokenStaking_abi)
    return stakingContract

def prepareBondingContract():
    with open('KeepBonding.json') as ts:
        bonding = json.load(ts)
    bonding_abi = bonding["abi"]
    bondingContract = web3.eth.contract(address=bonding["networks"]["3"]["address"], abi=bonding_abi)
    return bondingContract


def authOperatorContract(attempt):
    stakingContract = prepareStakingContract()
    ecdsaContract = prepareEcdsaContract()
    nonce = web3.eth.getTransactionCount(user_account.address)
    gasPrice = web3.eth.gasPrice
    txn = stakingContract.functions.authorizeOperatorContract(user_account.address,
                                                              ecdsaContract.address).buildTransaction({
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
        print('authOperatorContract failed. Confirmation time out')
        return 0
    else:
        if txn_receipt.status == 0:
            print('authOperatorContract failed. Reverted.')
        else:
            print("transaction confirmed operator")

def authSortitionPoolContract(attempt):
    ecdsaContract = prepareEcdsaContract()
    keepBondingContract = prepareBondingContract()
    nonce = web3.eth.getTransactionCount(user_account.address)
    gasPrice = web3.eth.gasPrice
    sortingPoolAddr = ecdsaContract.functions.getSortitionPool('0xc3f96306eDabACEa249D2D22Ec65697f38c6Da69').call()
    txn = keepBondingContract.functions.authorizeSortitionPoolContract(user_account.address,
                                                              sortingPoolAddr).buildTransaction({
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
        print('authSortitionPoolContract failed.Confirmation time out')
    else:
        if txn_receipt.status == 0:
            print('authSortitionPoolContract failed. Reverted.')
        else:
            print("transaction confirmed pool")

authOperatorContract(0)
authSortitionPoolContract(0)