# CA
The folder `CA` contains a certificate revocation list (CRL) and if you
want to revoke a certificate you have to execute the following command :

`openssl ca -config CA/ca.conf -revoke <path_to_certificate>/<certificate_name> -keyfile CA/ca.key -cert CA/ca.crt`

And generate a new CRL with thoose commands:

`openssl ca -config CA/ca.conf -gencrl -keyfile CA/ca.key -cert CA/ca.crt -out CA/root.crl.pem`

`openssl crl -inform PEM -in CA/root.crl.pem -outform DER -out CA/root.crl`

`rm CA/root.crl.pem`