#ifdef CK_SSL
#ifndef CK_ANSIC
#define NOPROTO
#endif /* CK_ANSIC */
#include "bio.h"
#include "buffer.h"
#include "x509.h"
#include "pem.h"
#include "ssl.h"

extern BIO *bio_err;
extern SSL *ssl_con;
extern SSL_CTX *ssl_ctx;
extern int ssl_debug_flag;
extern int ssl_only_flag;
extern int ssl_active_flag;
extern int ssl_verify_flag;
extern int ssl_secure_flag;
extern int ssl_verbose_flag;
extern int ssl_disabled_flag;
extern int ssl_cert_required;
extern int ssl_certsok_flag;
extern int ssl_dummy_flag;

extern char *ssl_log_file;
extern char *ssl_rsa_cert_file;
extern char *ssl_rsa_key_file;
extern char *ssl_dsa_cert_file;
extern char *ssl_dh_key_file;
extern char *ssl_cipher_list;

extern SSL_CTX *tls_ctx;
extern SSL *tls_con;
extern int tls_only_flag;
extern int tls_active_flag;
extern int tls_secure_flag;

_PROTOTYP(int ssl_do_init,(int));
_PROTOTYP(int ssl_display_connect_details,(SSL *,int));
_PROTOTYP(int ssl_server_verify_callback,(int, X509_STORE_CTX *));
_PROTOTYP(int ssl_client_verify_callback,(int, X509_STORE_CTX *));

#endif /* CK_SSL */
