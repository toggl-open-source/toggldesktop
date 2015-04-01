[CCode (cprefix = "px", lower_case_cprefix = "px_", cheader_filename = "proxy.h")]
namespace Libproxy {
	[Compact]
	[CCode (free_function = "px_proxy_factory_free")]
	public class ProxyFactory {
		public ProxyFactory ();
		[CCode (array_length = false, array_null_terminated = true)]
		public string[] get_proxies (string url);
	}
}
