
from web3 import Web3
import sys
web3 = Web3(Web3.HTTPProvider('https://ropsten.infura.io/v3/'+sys.argv[2]))
print(web3.fromWei(web3.eth.getBalance(web3.toChecksumAddress(sys.argv[1])), 'ether'))

