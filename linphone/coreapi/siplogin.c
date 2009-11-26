/*
linphone
Copyright (C) 2009  Simon MORLAT (simon.morlat@linphone.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "linphonecore.h"
#include <ctype.h>

static void sip_login_init_instance(SipSetupContext *ctx){
	LinphoneProxyConfig *cfg=sip_setup_context_get_proxy_config(ctx);
	/*disable registration until the user logs in*/
	linphone_proxy_config_enable_register(cfg,FALSE);
}

static void guess_display_name(LinphoneUri *from){
	char *dn=(char*)ms_malloc(strlen(linphone_uri_get_username(from))+3);
	const char *it;
	char *wptr=dn;
	bool_t begin=TRUE;
	bool_t surname=0;
	for(it=linphone_uri_get_username(from);*it!='\0';++it){
		if (begin){
			*wptr=toupper(*it);
			begin=FALSE;
		}else if (*it=='.'){
			if (surname) break;
			*wptr=' ';
			begin=TRUE;
			surname=TRUE;
		}else *wptr=*it;
		wptr++;
	}
	linphone_uri_set_display_name(from,dn);
	ms_free(dn);
}

static int sip_login_do_login(SipSetupContext * ctx, const char *uri, const char *passwd){
	LinphoneProxyConfig *cfg=sip_setup_context_get_proxy_config(ctx);
	LinphoneCore *lc=linphone_proxy_config_get_core(cfg);
	LinphoneAuthInfo *auth;
	LinphoneUri *parsed_uri;
	char *tmp;

	parsed_uri=linphone_uri_new(uri);
	if (parsed_uri==NULL){
		return -1;
	}
	if (linphone_uri_get_display_name(parsed_uri)!=NULL){
		guess_display_name(parsed_uri);
	}
	tmp=linphone_uri_as_string(parsed_uri);
	linphone_proxy_config_set_identity(cfg,tmp);
	if (passwd ) {
		auth=linphone_auth_info_new(linphone_uri_get_username(parsed_uri),NULL,passwd,NULL,NULL);
		linphone_core_add_auth_info(lc,auth);
	}
	linphone_proxy_config_enable_register(cfg,TRUE);
	linphone_proxy_config_done(cfg);
	ms_free(tmp);
	linphone_uri_destroy(parsed_uri);
	ms_message("SipLogin: done");
	return 0;
}

static int sip_login_do_logout(SipSetupContext * ctx){
	LinphoneProxyConfig *cfg=sip_setup_context_get_proxy_config(ctx);
	linphone_proxy_config_enable_register(cfg,FALSE);
	linphone_proxy_config_done(cfg);
	return 0;
}

/* a simple SipSetup built-in plugin to allow specify the user/password for proxy config at runtime*/
SipSetup linphone_sip_login={
	.name="SipLogin",
	.capabilities=SIP_SETUP_CAP_LOGIN,
	.init_instance=sip_login_init_instance,
	.login_account=sip_login_do_login,
	.logout_account=sip_login_do_logout
};

