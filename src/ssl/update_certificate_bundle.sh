rm cacert.pem
wget http://curl.haxx.se/ca/cacert.pem
openssl s_client -showcerts -connect toggl.com:443 </dev/null >> cacert.pem

