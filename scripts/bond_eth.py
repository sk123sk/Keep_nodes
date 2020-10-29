import json
import time
from web3 import Web3
import sys

web3 = Web3(Web3.HTTPProvider('https://ropsten.infura.io/v3/'+sys.argv[3]))
encrypted_key = sys.argv[1]
key_password = sys.argv[2]
amount = float(sys.argv[4])
_privateKey = web3.eth.account.decrypt(encrypted_key, password=key_password)
user_account = web3.eth.account.privateKeyToAccount(_privateKey)

def prepareBondingContract():
    with open('KeepBonding.json') as ts:
        bonding = json.load(ts)
    bonding_abi = bonding["abi"]
    bondingContract = web3.eth.contract(address=bonding["networks"]["3"]["address"], abi=bonding_abi)
    return bondingContract

def bond_eth(attempt):
    keepBondingContract = prepareBondingContract()
    nonce = web3.eth.getTransactionCount(user_account.address)
    gasPrice = web3.eth.gasPrice
    txn = keepBondingContract.functions.deposit(user_account.address).buildTransaction({
        'chainId': 3,
        'gas': 500000,
        'gasPrice': gasPrice*2 if gasPrice > 200000000000 else 200000000000,
        'value': web3.toWei(amount, 'ether'),
        'nonce': nonce,
    })
    signed_txn = web3.eth.account.signTransaction(txn, private_key=_privateKey)
    txn_hash = web3.eth.sendRawTransaction(signed_txn.rawTransaction)
    time.sleep(15)
    try:
        txn_receipt = web3.eth.waitForTransactionReceipt(txn_hash, timeout=200)
    except:
        print('bond_eth failed.Confirmation time out')
    else:
        if txn_receipt.status == 0:
            print('bond_eth failed. Reverted.')
        else:
            print("transaction confirmed")

bond_eth(0)