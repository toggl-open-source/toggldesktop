
#https://curl.haxx.se/docs/caextract.html

rm cacert.pem
curl --remote-name --time-cond cacert.pem https://curl.haxx.se/ca/cacert.pem
openssl s_client -showcerts -connect toggl.com:443 </dev/null >> cacert.pem

