
from web3 import Web3
import sys

web3 = Web3()

encrypted_key = sys.argv[1]
key_password = sys.argv[2]
_privateKey = web3.eth.account.decrypt(encrypted_key, password=key_password)
user_account = web3.eth.account.privateKeyToAccount(_privateKey)
print(str.lower(user_account.address))

