import json
import time
from web3 import Web3
import sys

web3 = Web3(Web3.HTTPProvider('https://ropsten.infura.io/v3/'+sys.argv[3]))
encrypted_key = sys.argv[1]
key_password = sys.argv[2]
_privateKey = web3.eth.account.decrypt(encrypted_key, password=key_password)
user_account = web3.eth.account.privateKeyToAccount(_privateKey)

def prepareGrantContract():
    with open('TokenGrant.json') as ts:
        tokenGrant = json.load(ts)
    tokenGrant_abi = tokenGrant["abi"]
    grantContract = web3.eth.contract(address=tokenGrant["networks"]["3"]["address"], abi=tokenGrant_abi)
    return grantContract


def prepareStakingContract():
    with open('TokenStaking.json') as ts:
        tokenStaking = json.load(ts)
    tokenStaking_abi = tokenStaking["abi"]
    stakingContract = web3.eth.contract(address=tokenStaking["networks"]["3"]["address"], abi=tokenStaking_abi)
    return stakingContract


def getGrantsId():
    grantContract = prepareGrantContract()
    grants = grantContract.functions.getGrants(user_account.address).call()
    return grants


def tokensAvailableToStake():
    grantContract = prepareGrantContract()
    grants = getGrantsId()
    amount = grantContract.functions.availableToStake(grants[0]).call()
    print('Keep tokens available:', web3.fromWei(amount, 'gwei'))
    return amount


def delegateKeep(attempt):
    block = web3.eth.getBlock('latest')
    if block.gasLimit < 1350000:
       delay_mins = (1350000 - block.gasLimit)/1300
       print('Delegating failed. Ropsten testnet block gas limit now:', block.gasLimit, 'Keep contract needs 1300000 gas. Wait about', round(delay_mins), 'mins.')
       return 0
    grantContract = prepareGrantContract()
    stakingContract = prepareStakingContract()
    authAddrs = "0x" + user_account.address.lower()[2:] + user_account.address.lower()[
                                                          2:] + user_account.address.lower()[2:]
    nonce = web3.eth.getTransactionCount(user_account.address)
    grants = getGrantsId()
    while not grants:
        time.sleep(10)
        grants = getGrantsId()
    grant = grants[0]
    print('Grant ID:', grant)
    gasPrice = web3.eth.gasPrice
    stakeTx = grantContract.functions.stake(grant, stakingContract.address, tokensAvailableToStake(),
                                            authAddrs).buildTransaction({
        'chainId': 3,
        'gas': 1350000,
        'gasPrice': gasPrice*2 if gasPrice > 200000000000 else 200000000000,
        'value': 0,
        'nonce': nonce,
    })
    signed_txn = web3.eth.account.signTransaction(stakeTx, private_key=_privateKey)
    txn_hash = web3.eth.sendRawTransaction(signed_txn.rawTransaction)
    time.sleep(15)
    try:
       txn_receipt = web3.eth.waitForTransactionReceipt(txn_hash, timeout=200)
    except:
        print('Delegating failed. Confirmation time out')
    else:
        if txn_receipt.status == 0:
            print('Delegating failed. Reverted.')
        else:
            print("transaction confirmed")

delegateKeep(0)
