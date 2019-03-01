
#https://curl.haxx.se/docs/caextract.html

rm cacert.pem
wget https://curl.haxx.se/ca/cacert.pem --no-check-certificate
openssl s_client -showcerts -connect toggl.com:443 </dev/null >> cacert.pem
openssl s_client -showcerts -connect toggl.space:443 </dev/null >> cacert.pem