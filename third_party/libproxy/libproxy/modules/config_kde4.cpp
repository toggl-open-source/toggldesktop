/*******************************************************************************
 * libproxy - A library for proxy configuration
 * Copyright (C) 2006 Nathaniel McCallum <nathaniel@natemccallum.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 ******************************************************************************/

#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <KDE/KGlobal>

#include "../extension_config.hpp"
using namespace libproxy;

static void dummyMessageHandler(QtMsgType, const char *) {}

class kde_config_extension : public config_extension {
public:
	kde_config_extension() {
		/* The constructor of KConfig uses qAppName() which asumes a QApplication object to exist.
		If not, an error message is written. This error message and all others seems to be disabled for
		libraries, but to be sure, we can reemplace temporaly Qt's internal message handler by a
		dummy implementation. */

		// Open the config file
		QtMsgHandler oldHandler = qInstallMsgHandler(dummyMessageHandler);
		this->cfg = new KConfig("kioslaverc", KConfig::NoGlobals);
		this->grp = new KConfigGroup(this->cfg, "Proxy Settings");
		qInstallMsgHandler(oldHandler);
	}

	~kde_config_extension() {
		delete this->grp;
		delete this->cfg;
	}

	vector<url> get_config(const url &dst) throw (runtime_error) {
		string  tmp;
		QString prxy;
		vector<url> response;

		switch (this->grp->readEntry("ProxyType", 0)) {
			case 1: // Use a manual proxy
				prxy = this->grp->readEntry(QString(dst.get_scheme().c_str()) + "Proxy", "");
				if (prxy.isEmpty()) {
					prxy = this->grp->readEntry("httpProxy", "");
					if (prxy.isEmpty()) {
						prxy = this->grp->readEntry("socksProxy", "");
						if (prxy.isEmpty())
							prxy = "direct://";
					};
				};
				// The result of toLatin1() is undefined for non-Latin1 strings.
				// However, KDE saves this entry using IDN and percent-encoding, so no problem...
				response.push_back(string(prxy.toLatin1().data()));
				break;
			case 2: // Use a manual PAC
				// The result of toLatin1() is undefined for non-Latin1 strings.
				// However, KDE saves this entry using IDN and percent-encoding, so no problem...
				tmp = string(this->grp->readEntry("Proxy Config Script", "").toLatin1().data());
				if (url::is_valid("pac+" + tmp))
					response.push_back(url("pac+" + tmp));
				else
				  response.push_back(string("wpad://"));
				break;	
			case 3: // Use WPAD
				response.push_back(string("wpad://"));
				break;
			case 4: // Use envvar
				throw runtime_error("User config_envvar"); // We'll bypass this config plugin and let the envvar plugin work
			default:
				response.push_back(url("direct://"));
				break;
		};

		return response;
	}

	string get_ignore(const url& /*dst*/) {
		// Apply ignore list only for manual proxy configuration
		if (this->grp->readEntry("ProxyType", 0) == 1)  { 
			string prefix = this->grp->readEntry("ReversedException", false) ? "-" : "";
			QStringList list = this->grp->readEntry("NoProxyFor", QStringList());
			for (int i = 0; i < list.size(); ++i)
				list[i] = QUrl(list.at(i)).toEncoded();
			return prefix + string(list.join(",").toLatin1().data());
		}
		return "";
	}

private:
	KConfig*      cfg;
	KConfigGroup* grp;
};

MM_MODULE_INIT_EZ(kde_config_extension, getenv("KDE_FULL_SESSION"), NULL, NULL);
