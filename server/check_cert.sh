#!/usr/bin/env bash
certFile="${1}"
username="${2}"
openssl x509 -noout -pubkey -in "${certFile}" > pub_keys/public-key-$username.pem
time_second=$(($(date -d "`openssl x509 -in ${certFile} -text -noout | grep "Not After" | cut -c 25-`" +%s) - $(date -d "now" +%s)))
if [ $time_second -lt 0 ]
then
  echo "0"
elif [ ! -f $certFile ]
then
    echo "0"
fi
#check if the CRL contains our certificate
mycrt=$(openssl x509 -in "$certFile" -serial -noout)
mycrt=${mycrt#*=}
if grep -rl "$mycrt" CA/root.crl.txt; then
    echo "0"
else
  echo "1"
fi