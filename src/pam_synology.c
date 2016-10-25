#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <syslog.h>
#ifdef HAVE_PAM_APPL_H
#include <pam_appl.h>
#elif defined(HAVE_SECURITY_PAM_APPL_H)
#include <security/pam_appl.h>
#endif
#ifdef HAVE_PAM_MODULES_H
#include <pam_modules.h>
#elif defined(HAVE_SECURITY_PAM_MODULES_H)
#include <security/pam_modules.h>
#endif
#include <curl/curl.h>
#include "stdlib_wrapper.h"

#define LOG_PREFIX PROJECT_NAME ": "

/* expected hook */
PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	(void)pamh;
	(void)flags;
	(void)argc;
	(void)argv;
	logmsg(LOG_INFO, "%s", LOG_PREFIX "setcred called but not implemented.");
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	(void)pamh;
	(void)flags;
	(void)argc;
	(void)argv;
	logmsg(LOG_INFO, "%s", LOG_PREFIX "acct_mgmt called but not implemented.");
	return PAM_SUCCESS;
}

/**
 * Makes getting arguments easier. Accepted arguments are of the form: name=value
 *
 * @param username- name of the argument to get
 * @param argc- number of total arguments
 * @param argv- arguments
 * @return Pointer to value or NULL
 */
static const char *pam_syno_get_arg(const char *username, int argc, const char **argv)
{
	size_t len = strlen(username);
	int i;

	for (i = 0; i < argc; i++) {
		// only give the part url part (after the equals sign)
		if (strncmp(username, argv[i], len) == 0 && argv[i][len] == '=')
			return argv[i] + len + 1;
	}
	return 0;
}

static char syno_reply[1024];

/**
 * Function to handle stuff from HTTP response.
 *
 * @param buf- Raw buffer from libcurl.
 * @param len- number of indices
 * @param size- size of each index
 * @param userdata- any extra user data needed
 * @return Number of bytes actually handled. If different from len * size, curl will throw an error
 */
static size_t pam_syno_write_fn(void *buf, size_t len, size_t size, void *userdata)
{
	(void)buf;
	(void)userdata;
	memcpy(syno_reply, buf, size);
	return len * size;
}

static int pam_syno_get_url(const char *urlstr, const char *usernamestr, const char *passwd, const char *cafile)
{
	int res = -1;
	char *userpass;
	CURL *curl_handle = curl_easy_init();

	if (!curl_handle) {
		logmsg(LOG_DEBUG, LOG_PREFIX "curl-handle fail");
		return -1;
	}

	userpass = malloc(256);

	(void)cafile;

	snprintf(userpass, 256,
		 "%s/webapi/auth.cgi?api=SYNO.API.Auth&version=3&method=login&account=%s&passwd=%s&session=FileStation&format=cookie",
		 urlstr, usernamestr, passwd);
	curl_easy_setopt(curl_handle, CURLOPT_URL, userpass);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, pam_syno_write_fn);
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1); // we don't care about progress
	curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1);

	// fixme
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);

	curl_easy_setopt(curl_handle, CURLOPT_RANDOM_FILE, "/dev/urandom");
	//curl_easy_setopt(curl_handle, CURLOPT_USE_SSL, CURLUSESSL_ALL);

	memset(syno_reply, 0, sizeof(syno_reply));
	res = curl_easy_perform(curl_handle);

	memset(userpass, 0, 256);
	free(userpass);
	curl_easy_cleanup(curl_handle);

	if (res == 0) {
		size_t reply_len = strlen(syno_reply);

		if (reply_len == 0)
			res = 1;
		else if (!strstr(syno_reply, "\"success\":true"))
			res = 2;
		else
			res = 0;
	}

	logmsg(LOG_INFO, LOG_PREFIX "res: %d\n", res);

	return res;
}

/* expected hook, this is where custom stuff happens */
PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	int ret = 0;

	const char *usernamestr = NULL;
	const char *urlstr = NULL;
	const char *cafile = NULL;

	(void)flags;

	struct pam_message msg;
	struct pam_conv *pam_item;
	struct pam_response *pam_resp;
	const struct pam_message *pam_msg = &msg;

	msg.msg_style = PAM_PROMPT_ECHO_OFF;
	msg.msg = "Synology password: ";

	if (pam_get_user(pamh, &usernamestr, NULL) != PAM_SUCCESS) {
		logmsg(LOG_DEBUG, LOG_PREFIX "username error", usernamestr);
		return PAM_AUTH_ERR;
	}
	logmsg(LOG_DEBUG, LOG_PREFIX "username: %s", usernamestr);

	urlstr = pam_syno_get_arg("url", argc, argv);
	if (!urlstr) {
		logmsg(LOG_DEBUG, LOG_PREFIX "urlstr error", usernamestr);
		return PAM_AUTH_ERR;
	}

	logmsg(LOG_DEBUG, LOG_PREFIX "urlstr: %s", urlstr);

	cafile = pam_syno_get_arg("cafile", argc, argv);
	if (pam_get_item(pamh, PAM_CONV, (const void **)&pam_item) != PAM_SUCCESS || !pam_item) {
		logmsg(LOG_DEBUG, LOG_PREFIX "pam_conv failure?", usernamestr);
		return PAM_AUTH_ERR;
	}

	pam_item->conv(1, &pam_msg, &pam_resp, pam_item->appdata_ptr);

	ret = PAM_SUCCESS;

	if (pam_syno_get_url(urlstr, usernamestr, pam_resp[0].resp, cafile) != 0)
		ret = PAM_AUTH_ERR;

	memset(pam_resp[0].resp, 0, strlen(pam_resp[0].resp));
	free(pam_resp);

	return ret;
}
