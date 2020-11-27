/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *    Copyright 2020 (c) Wind River Systems, Inc.
 */

#include <open62541/plugin/securitypolicy_default.h>
#include <open62541/util.h>

#ifdef UA_ENABLE_ENCRYPTION_OPENSSL

#include "securitypolicy_openssl_common.h"

#include <openssl/x509.h>
#include <openssl/rand.h>

#define UA_SHA1_LENGTH                                               20
#define UA_SECURITYPOLICY_BASIC128RSA15_RSAPADDING_LEN               11
#define UA_SECURITYPOLICY_BASIC128RSA15_SYM_ENCRYPTION_KEY_LENGTH    16
#define UA_SECURITYPOLICY_BASIC128RSA15_SYM_ENCRYPTION_BLOCK_SIZE    16
#define UA_SECURITYPOLICY_BASIC128RSA15_SYM_PLAIN_TEXT_BLOCK_SIZE    16
#define UA_SECURITYPOLICY_BASIC128RSA15_SYM_SIGNING_KEY_LENGTH       16
#define UA_SHA1_LENGTH                                               20

typedef struct {
    UA_ByteString             localPrivateKey;
    UA_ByteString             localCertThumbprint;
    const UA_Logger *         logger;
} Policy_Context_Basic128Rsa15;

typedef struct {
    UA_ByteString             localSymSigningKey;  
    UA_ByteString             localSymEncryptingKey; 
    UA_ByteString             localSymIv; 
    UA_ByteString             remoteSymSigningKey;
    UA_ByteString             remoteSymEncryptingKey;
    UA_ByteString             remoteSymIv;

    Policy_Context_Basic128Rsa15 * policyContext;
    UA_ByteString             remoteCertificate;
    X509 *                    remoteCertificateX509;   
} Channel_Context_Basic128Rsa15;

static UA_StatusCode 
UA_Policy_Basic128Rsa15_New_Context (UA_SecurityPolicy * securityPolicy,
                                     const UA_ByteString localPrivateKey,
                                     const UA_Logger *   logger) {
    Policy_Context_Basic128Rsa15 * context = (Policy_Context_Basic128Rsa15 *) 
                                    UA_malloc (sizeof (Policy_Context_Basic128Rsa15));
    if (context == NULL) {
        return UA_STATUSCODE_BADOUTOFMEMORY;
    }

    /* copy the local private key and add a NULL to the end */
    
    UA_StatusCode retval = UA_copyCertificate (&context->localPrivateKey,
                                               &localPrivateKey);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_free (context);
        return retval; 
    }

    retval = UA_Openssl_X509_GetCertificateThumbprint (
                         &securityPolicy->localCertificate,
                         &context->localCertThumbprint, true
                         );
    if (retval != UA_STATUSCODE_GOOD) {
        UA_ByteString_clear (&context->localPrivateKey);
        UA_free (context);
        return retval; 
    }

    context->logger = logger;
    securityPolicy->policyContext = context;

    return UA_STATUSCODE_GOOD;
}                                    

static void
UA_Policy_Basic128Rsa15_Clear_Context (UA_SecurityPolicy *policy) {
    if (policy == NULL) {
        return;
    }
    UA_ByteString_clear(&policy->localCertificate);

    Policy_Context_Basic128Rsa15 * ctx = (Policy_Context_Basic128Rsa15 *) policy->policyContext;    
    if (ctx == NULL) {
        return; 
    }

    /* delete all allocated members in the context */

    UA_ByteString_clear (&ctx->localPrivateKey);
    UA_ByteString_clear (&ctx->localCertThumbprint);
    UA_free (ctx);   

    return;         
}

/* create the channel context */

static UA_StatusCode 
UA_ChannelModule_Basic128Rsa15_New_Context (const UA_SecurityPolicy * securityPolicy,
                                            const UA_ByteString *     remoteCertificate,
                                            void **                   channelContext) {
    if (securityPolicy == NULL || remoteCertificate == NULL || 
        channelContext == NULL) {
        return UA_STATUSCODE_BADINTERNALERROR;                                  
        }
    Channel_Context_Basic128Rsa15 * context = (Channel_Context_Basic128Rsa15 *)
            UA_malloc (sizeof (Channel_Context_Basic128Rsa15));
    if (context == NULL) {
        return UA_STATUSCODE_BADOUTOFMEMORY;
    }

    UA_ByteString_init(&context->localSymSigningKey);
    UA_ByteString_init(&context->localSymEncryptingKey);
    UA_ByteString_init(&context->localSymIv);
    UA_ByteString_init(&context->remoteSymSigningKey);
    UA_ByteString_init(&context->remoteSymEncryptingKey);
    UA_ByteString_init(&context->remoteSymIv);

    UA_StatusCode retval = UA_copyCertificate (&context->remoteCertificate, 
                                               remoteCertificate);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_free (context);
        return retval;
    }

    /* decode to X509 */
    const unsigned char * pData = context->remoteCertificate.data;    
    context->remoteCertificateX509 = d2i_X509 (NULL, &pData, 
                                    (long) context->remoteCertificate.length);
    if (context->remoteCertificateX509 == NULL) {
        UA_ByteString_clear (&context->remoteCertificate); 
        UA_free (context);
        return UA_STATUSCODE_BADCERTIFICATECHAININCOMPLETE;
    }

    context->policyContext = (Policy_Context_Basic128Rsa15 *) 
                             (securityPolicy->policyContext);

    *channelContext = context;

    UA_LOG_INFO (securityPolicy->logger, 
                 UA_LOGCATEGORY_SECURITYPOLICY, 
                 "The Basic128Rsa15 security policy channel with openssl is created.");

    return UA_STATUSCODE_GOOD;
}

/* delete the channel context */

static void 
UA_ChannelModule_Basic128Rsa15_Delete_Context (void * channelContext) {
    if (channelContext != NULL) {
        Channel_Context_Basic128Rsa15 * cc = (Channel_Context_Basic128Rsa15 *)
                                              channelContext;
        X509_free (cc->remoteCertificateX509);                                           
        UA_ByteString_clear (&cc->remoteCertificate); 
        UA_ByteString_clear (&cc->localSymSigningKey);
        UA_ByteString_clear (&cc->localSymEncryptingKey);
        UA_ByteString_clear (&cc->localSymIv);
        UA_ByteString_clear (&cc->remoteSymSigningKey);
        UA_ByteString_clear (&cc->remoteSymEncryptingKey);
        UA_ByteString_clear (&cc->remoteSymIv);
        UA_LOG_INFO (cc->policyContext->logger, 
                 UA_LOGCATEGORY_SECURITYPOLICY, 
                 "The Basic128Rsa15 security policy channel with openssl is deleted.");   

        UA_free (cc);                      
    }
}

static UA_StatusCode
UA_ChannelModule_Basic128Rsa15_setLocalSymSigningKey (void *                channelContext,
                                                      const UA_ByteString * key) {
    if (key == NULL || channelContext == NULL) {
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    }

    Channel_Context_Basic128Rsa15 * cc = (Channel_Context_Basic128Rsa15 *) channelContext;
    UA_ByteString_clear(&cc->localSymSigningKey);
    return UA_ByteString_copy(key, &cc->localSymSigningKey);
}

static UA_StatusCode
UA_ChannelModule_Basic128Rsa15_setLocalSymEncryptingKey (void *                channelContext,
                                                         const UA_ByteString * key) {
    if (key == NULL || channelContext == NULL) {
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    }

    Channel_Context_Basic128Rsa15 * cc = (Channel_Context_Basic128Rsa15 *) channelContext;
    UA_ByteString_clear(&cc->localSymEncryptingKey);
    return UA_ByteString_copy(key, &cc->localSymEncryptingKey);
}

static UA_StatusCode
UA_ChannelModule_Basic128Rsa15_setLocalSymIv (void *                channelContext,
                                              const UA_ByteString * iv) {
    if (iv == NULL || channelContext == NULL) {
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    }

    Channel_Context_Basic128Rsa15 * cc = (Channel_Context_Basic128Rsa15 *) channelContext;
    UA_ByteString_clear(&cc->localSymIv);
    return UA_ByteString_copy(iv, &cc->localSymIv);
}

static UA_StatusCode
UA_ChannelModule_Basic128Rsa15_setRemoteSymSigningKey (void *                channelContext,
                                                       const UA_ByteString * key) {
    if (key == NULL || channelContext == NULL) {
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    }

    Channel_Context_Basic128Rsa15 * cc = (Channel_Context_Basic128Rsa15 *) channelContext;
    UA_ByteString_clear(&cc->remoteSymSigningKey);
    return UA_ByteString_copy(key, &cc->remoteSymSigningKey);
}

static UA_StatusCode
UA_ChannelModule_Basic128Rsa15_setRemoteSymEncryptingKey (void *                channelContext,
                                                          const UA_ByteString * key) {
    if (key == NULL || channelContext == NULL) {
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    }

    Channel_Context_Basic128Rsa15 * cc = (Channel_Context_Basic128Rsa15 *) channelContext;
    UA_ByteString_clear(&cc->remoteSymEncryptingKey);
    return UA_ByteString_copy(key, &cc->remoteSymEncryptingKey);
}

static UA_StatusCode
UA_ChannelModule_Basic128Rsa15_setRemoteSymIv (void *                channelContext,
                                               const UA_ByteString * key) {
    if (key == NULL || channelContext == NULL) {
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    }

    Channel_Context_Basic128Rsa15 * cc = (Channel_Context_Basic128Rsa15 *) channelContext;
    UA_ByteString_clear(&cc->remoteSymIv);
    return UA_ByteString_copy(key, &cc->remoteSymIv);
}

static UA_StatusCode
UA_ChannelModule_Basic128Rsa15_compareCertificate (const void *          channelContext,
                                                   const UA_ByteString * certificate) {
    if(channelContext == NULL || certificate == NULL) {
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    }

    const Channel_Context_Basic128Rsa15 * cc = 
                     (const Channel_Context_Basic128Rsa15 *) channelContext;
    return UA_OpenSSL_X509_compare (certificate, cc->remoteCertificateX509);
}

static UA_StatusCode
UA_Asy_Basic128Rsa15_compareCertificateThumbprint (const UA_SecurityPolicy * securityPolicy,
                                     const UA_ByteString *     certificateThumbprint) {
    if (securityPolicy == NULL || certificateThumbprint == NULL) {
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    }
    Policy_Context_Basic128Rsa15 *pc = (Policy_Context_Basic128Rsa15 *)
                                       securityPolicy->policyContext;
    if(!UA_ByteString_equal(certificateThumbprint, &pc->localCertThumbprint)) {
        return UA_STATUSCODE_BADCERTIFICATEINVALID;
    }
    return UA_STATUSCODE_GOOD;
}

/* Generates a thumbprint for the specified certificate */

static UA_StatusCode 
UA_Asy_Basic128Rsa15_makeCertificateThumbprint (const UA_SecurityPolicy * securityPolicy,
                                  const UA_ByteString *     certificate,
                                  UA_ByteString *           thumbprint) {
    return UA_Openssl_X509_GetCertificateThumbprint (certificate, 
               thumbprint, false); 
}

static size_t
UA_AsySig_Basic128Rsa15_getRemoteSignatureSize (const UA_SecurityPolicy * securityPolicy,
                                                const void *              channelContext) {
    if (securityPolicy == NULL || channelContext == NULL) {
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    }

    const Channel_Context_Basic128Rsa15 * cc = (const Channel_Context_Basic128Rsa15 *) channelContext;
    UA_Int32 keyLen;
    UA_Openssl_RSA_Public_GetKeyLength (cc->remoteCertificateX509, &keyLen);
    return (size_t) keyLen; 
}

static size_t 
UA_AsySig_Basic128Rsa15_getLocalSignatureSize (const UA_SecurityPolicy * securityPolicy,
                                               const void *              channelContext) {
    if (securityPolicy == NULL || channelContext == NULL) {
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    }

    Policy_Context_Basic128Rsa15 * pc = 
               (Policy_Context_Basic128Rsa15 *) securityPolicy->policyContext;
    UA_Int32 keyLen;
    UA_Openssl_RSA_Private_GetKeyLength (&pc->localPrivateKey, &keyLen);

    return (size_t) keyLen; 
}

static UA_StatusCode 
UA_AsySig_Basic128Rsa15_Verify (const UA_SecurityPolicy * securityPolicy,
                                void *                    channelContext, 
                                const UA_ByteString *     message,
                                const UA_ByteString *     signature) {
    if (securityPolicy == NULL || message == NULL || signature == NULL || 
        channelContext == NULL) {
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    }

    Channel_Context_Basic128Rsa15 * cc = (Channel_Context_Basic128Rsa15 *) channelContext;
    UA_StatusCode retval = UA_OpenSSL_RSA_PKCS1_V15_SHA1_Verify (message, 
                            cc->remoteCertificateX509, signature);

    return retval;
}

static UA_StatusCode
UA_AsySig_Basic128Rsa15_Sign (const UA_SecurityPolicy * securityPolicy,
                              void *                    channelContext, 
                              const UA_ByteString *     message,
                              UA_ByteString *           signature) {
    if (securityPolicy == NULL || channelContext == NULL ||
        message == NULL || signature == NULL) {
        return UA_STATUSCODE_BADINVALIDARGUMENT; 
    }

    Policy_Context_Basic128Rsa15 * pc = 
               (Policy_Context_Basic128Rsa15 *) securityPolicy->policyContext;
    return UA_Openssl_RSA_PKCS1_V15_SHA1_Sign (message, &pc->localPrivateKey,
                                               signature);
}

static size_t
UA_AsymEn_Basic128Rsa15_getRemotePlainTextBlockSize (const UA_SecurityPolicy * securityPolicy,
                                                     const void *              channelContext) {
    if (securityPolicy == NULL || channelContext == NULL) {
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    }

    const Channel_Context_Basic128Rsa15 * cc = (const Channel_Context_Basic128Rsa15 *) channelContext;
    UA_Int32 keyLen;
    UA_Openssl_RSA_Public_GetKeyLength (cc->remoteCertificateX509, &keyLen);
    return (size_t) keyLen - UA_SECURITYPOLICY_BASIC128RSA15_RSAPADDING_LEN;
}

static size_t 
UA_AsymEn_Basic128Rsa15_getRemoteBlockSize (const UA_SecurityPolicy * securityPolicy,
                                            const void *              channelContext) {
    if (securityPolicy == NULL || channelContext == NULL) {
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    }

    const Channel_Context_Basic128Rsa15 * cc = (const Channel_Context_Basic128Rsa15 *) channelContext;
    UA_Int32 keyLen;
    UA_Openssl_RSA_Public_GetKeyLength (cc->remoteCertificateX509, &keyLen);
    return (size_t) keyLen;
}

static size_t
UA_AsymEn_Basic128Rsa15_getRemoteKeyLength (const UA_SecurityPolicy * securityPolicy,
                                            const void *              channelContext) {
    if (securityPolicy == NULL || channelContext == NULL)
        return UA_STATUSCODE_BADINVALIDARGUMENT;

    const Channel_Context_Basic128Rsa15 * cc = (const Channel_Context_Basic128Rsa15 *) channelContext;
    UA_Int32 keyLen;
    UA_Openssl_RSA_Public_GetKeyLength (cc->remoteCertificateX509, &keyLen);
    return (size_t) keyLen * 8;
}

static size_t 
UA_AsymEn_Basic128Rsa15_getLocalKeyLength (const UA_SecurityPolicy * securityPolicy,
                                           const void *              channelContext) {
    if (securityPolicy == NULL || channelContext == NULL)
        return UA_STATUSCODE_BADINVALIDARGUMENT;

    Policy_Context_Basic128Rsa15 * pc = 
               (Policy_Context_Basic128Rsa15 *) securityPolicy->policyContext;
    UA_Int32 keyLen;
    UA_Openssl_RSA_Private_GetKeyLength (&pc->localPrivateKey, &keyLen);

    return (size_t) keyLen * 8; 
}

static UA_StatusCode 
UA_AsymEn_Basic128Rsa15_Decrypt (const UA_SecurityPolicy * securityPolicy,
                                 void *                    channelContext,
                                 UA_ByteString *           data) {
    if (securityPolicy == NULL || channelContext == NULL || data == NULL) {
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    }

    Channel_Context_Basic128Rsa15 * cc = (Channel_Context_Basic128Rsa15 *) channelContext;
    UA_StatusCode ret = UA_Openssl_RSA_PKCS1_V15_Decrypt (data, 
                        &cc->policyContext->localPrivateKey);
    return ret;                        
}

static UA_StatusCode
UA_AsymEn_Basic128Rsa15_Encrypt (const UA_SecurityPolicy * securityPolicy,
                            void *                    channelContext,
                            UA_ByteString *           data) {
    if (securityPolicy == NULL || channelContext == NULL ||
        data == NULL)
        return UA_STATUSCODE_BADINVALIDARGUMENT; 

    Channel_Context_Basic128Rsa15 * cc = (Channel_Context_Basic128Rsa15 *) channelContext;    
    return UA_Openssl_RSA_PKCS1_V15_Encrypt (data, 
                                             UA_SECURITYPOLICY_BASIC128RSA15_RSAPADDING_LEN,
                                             cc->remoteCertificateX509);
}

static UA_StatusCode
UA_Sym_Basic128Rsa15_generateNonce (const UA_SecurityPolicy * sp,
                                    UA_ByteString *           out) {
    UA_Int32 rc = RAND_bytes(out->data, (int) out->length);
    if (rc != 1) {
        return UA_STATUSCODE_BADUNEXPECTEDERROR;
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
UA_Sym_Basic128Rsa15_generateKey (const UA_SecurityPolicy * securityPolicy,
                                  const UA_ByteString *     secret,
                                  const UA_ByteString *     seed, 
                                  UA_ByteString *           out) {
    return UA_Openssl_Random_Key_PSHA1_Derive (secret, seed, out);
}

static size_t 
UA_SymEn_Basic128Rsa15_getLocalKeyLength (const UA_SecurityPolicy * securityPolicy,
                                     const void *              channelContext) {
    /* 16 bytes 128 bits */
    return UA_SECURITYPOLICY_BASIC128RSA15_SYM_ENCRYPTION_KEY_LENGTH; 
}

static size_t 
UA_SymEn_Basic128Rsa15_getBlockSize (const UA_SecurityPolicy * securityPolicy,
                                     const void *              channelContext) {
    return UA_SECURITYPOLICY_BASIC128RSA15_SYM_ENCRYPTION_BLOCK_SIZE;
}

static size_t
UA_SymEn_Basic128Rsa15_getRemoteKeyLength (const UA_SecurityPolicy * securityPolicy,
                                           const void * channelContext) {
    return UA_SECURITYPOLICY_BASIC128RSA15_SYM_ENCRYPTION_KEY_LENGTH; 
}

static size_t 
UA_SymEn_Basic128Rsa15_getPlainTextBlockSize (const UA_SecurityPolicy * securityPolicy,
                                         const void *              channelContext) {
    return UA_SECURITYPOLICY_BASIC128RSA15_SYM_PLAIN_TEXT_BLOCK_SIZE;                                                        
}

static UA_StatusCode
UA_SymEn_Basic128Rsa15_Encrypt (const UA_SecurityPolicy * securityPolicy,
                                void *                    channelContext,
                                UA_ByteString *           data) {
    if(securityPolicy == NULL || channelContext == NULL || data == NULL)
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    
    Channel_Context_Basic128Rsa15 * cc = (Channel_Context_Basic128Rsa15 *) channelContext;
    return UA_OpenSSL_AES_128_CBC_Encrypt (&cc->localSymIv, &cc->localSymEncryptingKey, data);
}

static UA_StatusCode
UA_SymEn_Basic128Rsa15_Decrypt (const UA_SecurityPolicy * securityPolicy,
                                void *                    channelContext,
                                UA_ByteString *           data) {
    if(securityPolicy == NULL || channelContext == NULL || data == NULL)
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    Channel_Context_Basic128Rsa15 * cc = (Channel_Context_Basic128Rsa15 *) channelContext;    
    return UA_OpenSSL_AES_128_CBC_Decrypt (&cc->remoteSymIv, &cc->remoteSymEncryptingKey, data);
}

static size_t 
UA_SymSig_Basic128Rsa15_getKeyLength (const UA_SecurityPolicy * securityPolicy,
                                      const void *              channelContext) {
    return UA_SECURITYPOLICY_BASIC128RSA15_SYM_SIGNING_KEY_LENGTH; 
}

static size_t
UA_SymSig_Basic128Rsa15_getSignatureSize (const UA_SecurityPolicy * securityPolicy,
                                          const void *              channelContext) {
    return UA_SHA1_LENGTH;
}

static UA_StatusCode
UA_SymSig_Basic128Rsa15_Verify (const UA_SecurityPolicy * securityPolicy,
                                void *                    channelContext, 
                                const UA_ByteString *     message,
                                const UA_ByteString *     signature) {
    if (securityPolicy == NULL || channelContext == NULL || 
       message == NULL || signature == NULL)
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    
    Channel_Context_Basic128Rsa15 * cc = (Channel_Context_Basic128Rsa15 *) channelContext;
    return UA_OpenSSL_HMAC_SHA1_Verify (message, 
                                        &cc->remoteSymSigningKey, 
                                        signature);   
}

static UA_StatusCode 
UA_SymSig_Basic128Rsa15_Sign (const UA_SecurityPolicy * securityPolicy,
                              void *                    channelContext, 
                              const UA_ByteString *     message,
                              UA_ByteString *           signature) {
    if (securityPolicy == NULL || channelContext == NULL || 
       message == NULL || signature == NULL)
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    
    Channel_Context_Basic128Rsa15 * cc = (Channel_Context_Basic128Rsa15 *) channelContext;
    return UA_OpenSSL_HMAC_SHA1_Sign (message, &cc->localSymSigningKey, signature);
}

/* the main entry of Basic128Rsa15 */

UA_StatusCode
UA_SecurityPolicy_Basic128Rsa15 (UA_SecurityPolicy * policy,
                                 const UA_ByteString localCertificate,
                                 const UA_ByteString localPrivateKey, 
                                 const UA_Logger *   logger) {

    UA_SecurityPolicyAsymmetricModule * const asymmetricModule = &policy->asymmetricModule;
    UA_SecurityPolicySymmetricModule * const  symmetricModule = &policy->symmetricModule;  
    UA_SecurityPolicyChannelModule * const    channelModule = &policy->channelModule;  
    UA_StatusCode                             retval; 

    UA_LOG_INFO (logger, UA_LOGCATEGORY_SECURITYPOLICY, 
                 "The Basic128Rsa15 security policy with openssl is added.");

    UA_Openssl_Init ();
    memset(policy, 0, sizeof(UA_SecurityPolicy));
    policy->logger = logger;
    policy->policyUri = UA_STRING("http://opcfoundation.org/UA/SecurityPolicy#Basic128Rsa15\0");

    /* set ChannelModule context  */

    channelModule->newContext = UA_ChannelModule_Basic128Rsa15_New_Context;
    channelModule->deleteContext = UA_ChannelModule_Basic128Rsa15_Delete_Context;

    channelModule->setLocalSymSigningKey = UA_ChannelModule_Basic128Rsa15_setLocalSymSigningKey;
    channelModule->setLocalSymEncryptingKey = UA_ChannelModule_Basic128Rsa15_setLocalSymEncryptingKey;
    channelModule->setLocalSymIv = UA_ChannelModule_Basic128Rsa15_setLocalSymIv;
    channelModule->setRemoteSymSigningKey = UA_ChannelModule_Basic128Rsa15_setRemoteSymSigningKey;
    channelModule->setRemoteSymEncryptingKey = UA_ChannelModule_Basic128Rsa15_setRemoteSymEncryptingKey;
    channelModule->setRemoteSymIv = UA_ChannelModule_Basic128Rsa15_setRemoteSymIv;
    channelModule->compareCertificate = UA_ChannelModule_Basic128Rsa15_compareCertificate;    

    /* Copy the certificate and add a NULL to the end */

    retval = UA_copyCertificate (&policy->localCertificate, &localCertificate);
    if (retval != UA_STATUSCODE_GOOD)
        return retval;

    /* asymmetricModule */

    asymmetricModule->compareCertificateThumbprint = UA_Asy_Basic128Rsa15_compareCertificateThumbprint;
    asymmetricModule->makeCertificateThumbprint = UA_Asy_Basic128Rsa15_makeCertificateThumbprint;

    /* AsymmetricModule - signature algorithm */

    UA_SecurityPolicySignatureAlgorithm * asySigAlgorithm = 
                    &asymmetricModule->cryptoModule.signatureAlgorithm;
    asySigAlgorithm->uri = UA_STRING("http://www.w3.org/2000/09/xmldsig#rsa-sha1\0");
    asySigAlgorithm->getRemoteSignatureSize = UA_AsySig_Basic128Rsa15_getRemoteSignatureSize;
    asySigAlgorithm->getLocalSignatureSize = UA_AsySig_Basic128Rsa15_getLocalSignatureSize;
    asySigAlgorithm->getLocalKeyLength = NULL;
    asySigAlgorithm->getRemoteKeyLength = NULL;
    asySigAlgorithm->verify = UA_AsySig_Basic128Rsa15_Verify;    
    asySigAlgorithm->sign = UA_AsySig_Basic128Rsa15_Sign;

    /*  AsymmetricModule encryption algorithm */

    UA_SecurityPolicyEncryptionAlgorithm * asymEncryAlg =
        &asymmetricModule->cryptoModule.encryptionAlgorithm;
    asymEncryAlg->uri = UA_STRING("http://www.w3.org/2001/04/xmlenc#rsa-1_5\0");
    asymEncryAlg->getRemotePlainTextBlockSize = UA_AsymEn_Basic128Rsa15_getRemotePlainTextBlockSize;
    asymEncryAlg->getRemoteBlockSize = UA_AsymEn_Basic128Rsa15_getRemoteBlockSize;
    asymEncryAlg->getRemoteKeyLength = UA_AsymEn_Basic128Rsa15_getRemoteKeyLength;
    asymEncryAlg->getLocalKeyLength = UA_AsymEn_Basic128Rsa15_getLocalKeyLength;
    asymEncryAlg->getLocalPlainTextBlockSize = NULL;
    asymEncryAlg->getLocalBlockSize = NULL;
    asymEncryAlg->decrypt = UA_AsymEn_Basic128Rsa15_Decrypt;
    asymEncryAlg->encrypt = UA_AsymEn_Basic128Rsa15_Encrypt;

    /* SymmetricModule */

    symmetricModule->secureChannelNonceLength = 16;  /* 128 bits*/
    symmetricModule->generateNonce = UA_Sym_Basic128Rsa15_generateNonce;
    symmetricModule->generateKey = UA_Sym_Basic128Rsa15_generateKey; 

    /* Symmetric encryption Algorithm */

    UA_SecurityPolicyEncryptionAlgorithm * symEncryptionAlgorithm =
        &symmetricModule->cryptoModule.encryptionAlgorithm;
    symEncryptionAlgorithm->uri = UA_STRING("http://www.w3.org/2001/04/xmlenc#aes128-cbc\0");   
    symEncryptionAlgorithm->getLocalKeyLength = UA_SymEn_Basic128Rsa15_getLocalKeyLength;
    symEncryptionAlgorithm->getLocalBlockSize = UA_SymEn_Basic128Rsa15_getBlockSize;
    symEncryptionAlgorithm->getRemoteKeyLength = UA_SymEn_Basic128Rsa15_getRemoteKeyLength;
    symEncryptionAlgorithm->getRemoteBlockSize = UA_SymEn_Basic128Rsa15_getBlockSize;
    symEncryptionAlgorithm->getLocalPlainTextBlockSize = UA_SymEn_Basic128Rsa15_getPlainTextBlockSize;
    symEncryptionAlgorithm->getRemotePlainTextBlockSize = UA_SymEn_Basic128Rsa15_getPlainTextBlockSize;
    symEncryptionAlgorithm->decrypt = UA_SymEn_Basic128Rsa15_Decrypt;
    symEncryptionAlgorithm->encrypt = UA_SymEn_Basic128Rsa15_Encrypt;    

    /* Symmetric signature Algorithm */

    UA_SecurityPolicySignatureAlgorithm * symSignatureAlgorithm =
        &symmetricModule->cryptoModule.signatureAlgorithm;
    symSignatureAlgorithm->uri = UA_STRING("http://www.w3.org/2000/09/xmldsig#hmac-sha1\0");
    symSignatureAlgorithm->getLocalKeyLength = UA_SymSig_Basic128Rsa15_getKeyLength;
    symSignatureAlgorithm->getRemoteKeyLength = UA_SymSig_Basic128Rsa15_getKeyLength;
    symSignatureAlgorithm->getRemoteSignatureSize = UA_SymSig_Basic128Rsa15_getSignatureSize;
    symSignatureAlgorithm->getLocalSignatureSize = UA_SymSig_Basic128Rsa15_getSignatureSize;

    symSignatureAlgorithm->verify = UA_SymSig_Basic128Rsa15_Verify;
    symSignatureAlgorithm->sign = UA_SymSig_Basic128Rsa15_Sign;

    /* set the policy context */

    retval = UA_Policy_Basic128Rsa15_New_Context (policy, localPrivateKey, logger);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_ByteString_clear (&policy->localCertificate);
        return retval;
    }
    policy->clear = UA_Policy_Basic128Rsa15_Clear_Context;    

    /* Use the same signature algorithm as the asymmetric component for 
       certificate signing (see standard) */
    policy->certificateSigningAlgorithm = policy->asymmetricModule.cryptoModule.signatureAlgorithm;

    return UA_STATUSCODE_GOOD;
}

#endif
