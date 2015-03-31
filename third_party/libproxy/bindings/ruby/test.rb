require 'Libproxy'

pf = Libproxy.px_proxy_factory_new
proxies = Libproxy.px_proxy_factory_get_proxies(pf, "http://www.google.com")

proxies.each { |proxy| print proxy + " " } 
print "\n"
