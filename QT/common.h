#ifndef COMMON_H
#define COMMON_H

#ifdef WIN32
#define P_PORT                                                                80
#define P_SSL_PORT                                                           443
#else
#define P_PORT                                                                80
#define P_SSL_PORT                                                           443
#define _STUNNEL_HACK
#endif
#define P_BIND                                                 QHostAddress::Any
#define P_SSL_BIND                        QHostAddress::Any //QHostAddress("ip")
#define P_TRANSFER_RATE                                                 300*1024
#define P_MAX_FROM_IP                                                         10
#define P_TIMEOUT_REQUEST                                                   8000
#define P_TIMEOUT_READ                                                     10000
#define P_KEEPALIVE_TIMEOUT                                                 5000
#define P_KEEPALIVE_MAX                                                      100
#define P_HOSTNAME                                     "bitspeercachedetect.com"
#define P_PEM_CERT                            "bitspeercachedetect.com.cert.pem"
#define P_PEM_KEY                              "bitspeercachedetect.com.key.pem"
#define P_PEM_CHAIN                                              "gd_bundle.crt"
#define P_FOLDER_SESSIONS              (QDir::currentPath() + "/storesessions/")
#define P_FOLDER_BITS_SESSIONS       (QDir::currentPath() + "/htbits/sessions/")
#define P_FOLDER_ROOT                         (QDir::currentPath() + "/htroot/")

enum kSocketState {
    kStateNone,
    kStateHead,
    kStateBody,
    kStateMeta,
    kStateTransitional
};

enum kSocketParser {
    kParseNone,
    kParseGet,
    kParsePost,
    kParseHead,
    kParseBitsPost,
    kParseOptions,
    kParseProfind,
};

#endif // COMMON_H
