Create a certificate and key file with this command:
```
openssl req -x509 -newkey rsa:4096 -nodes -keyout serkey.pem -out sercert.pem -days 365
openssl req -x509 -newkey rsa:4096 -nodes -keyout clikey.pem -out clicert.pem -days 365
```