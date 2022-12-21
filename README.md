# Signed bash scripts execution on remote server

## Brief
This repository contains code to simulate a server that can execute client
bash scripts under certain conditions. The client script must be signed with
its private key and must contain its signature at the beginning. The public 
key of the client is contained on the server with an X509 certificate 
attesting that the key is valid.

In case the script sent by the client contains a valid signature with a 
valid key the script is executed on the server and the result is sent to 
the client. Otherwise error messages will be sent to the client [see error message section].

## How it works

Each client is identified on the server with a unique username. 

The client certificates are stored in the `certificate` 
folder under the format `cert-username.pem`. You can add new certificates 
in this folder if you want. Each certificate are generate with openssl with 
this set of command:
- Choose an elliptic curve
`openssl ecparam -list_curves`

- Generate a private key for prime256v1
`openssl ecparam -name prime256v1 -genkey -noout -out private-key.pem`

- Creation of the corresponding public key
`openssl ec -in private-key.pem -pubout -out public-key.pem`

- Certificate generation
`openssl req -new -x509 -key private-key.pem -out cert.pem -days 360`

- Check the date of validity for a certificate
`openssl x509 -noout -in cert.pem -dates`

- Sign file
`openssl dgst -sha1 -sign private-key.pem script.sh > signature.bin`

- Check signature
`openssl dgst -sha1 -verify public-key.pem -signature signature.bin script.sh`


The server can open 10 processes capable of executing client scripts. 
Each time a client connects, the server assigns a process if there is 
one available, otherwise the connection is refused and a message is sent
to the client.

In case the client is accepted, this scheme is executed:

![Alt text](flow-graph.png?raw=true "Client Server communication")

For the checks the server used 2 script written in bash to check if the
certificate has not expired and if his not in the CRL.

### IPC
For the IPC I've choosed to used socket, where a queue is established for connection
and the first connection request is accepted. The request is run in another 
process, up to 10 simultaneous connection.

### Script + signature
Since I had some problem on the signature contained in the script I made
a conversion in hex for the transfer. 
For this I use the following command:
- Convert bin file to text
`od -An -vtx1 file.bin > Check.txt`

- Convert text back to bin
`LC_ALL=C tr -cd 0-9a-fA-F < Check.txt | xxd -r -p > file.bin`

## How to use

### Compilation

#### Server

For the server you have to go in the `server` folder and then execute the 
command `make`. After that it'll create an executable named `server` that 
you can run without any argument with the command `./server`.

Here are all the command :

```commandline
cd server && make && ./server
```

#### Client

For the client a C script is given to send your bash script to the server. 
You have to give the username corresponding to the private key, the signature
of your script and the bash script to the program which will be send to
the server and executed if the signature is good.

## Error messages

- `You have no certificate on the server` : the server doesn't have any 
certificate of the user. Identify with the username given.
- `Your certificate is not valid` : the certificate is in the CRL or 
is expired.

## TESTS

For all tests previously run the command in the server folder:
`make && ./server`

In the client folder:
- To test the case where the client don't have a certificate on the server 
run: 

`make && ./client script.txt file_signature/signature-noCertOnServer.bin noCertOnServer`

It should output: `You have no certificate on the server`
- Test case where the certificate on the server has expired: 


`make && ./client script.txt file_signature/signature-certNotValidOnServer.bin certNotValidOnServer` 

It should output: `Your certificate is not valid`
- Test case where the certificate is in the CRL:


`make && ./client script.txt file_signature/signature-revokeCert.bin revokeCert` 

It should output: `Your certificate is not valid`
- Test case where the client has a certificate valid and a good signature:


`make && ./client script.txt file_signature/signature-anthony.bin anthony` 

It should output the result of `script.txt`: `Hello World`