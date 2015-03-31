
Libproxy = imports.gi.Libproxy;
px = Libproxy.proxy_factory_new;

print (Libproxy.get_proxies("http://www.google.com"));


