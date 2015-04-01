use Net::Libproxy;

$p = new Net::Libproxy;
$proxies = $p->getProxies('http://www.google.com');
foreach my $proxy (@$proxies) {
  print $proxy."\n";
}
