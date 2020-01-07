//
// Created by moqi on 2018/6/6.
//

#ifndef FRAMEWORK_OPENSSLTHREADLOCK_H
#define FRAMEWORK_OPENSSLTHREADLOCK_H
#ifdef __cplusplus
extern "C" {
#endif
int openssl_thread_setup(void);
int openssl_thread_cleanup(void);
#ifdef __cplusplus
};
#endif

#endif //FRAMEWORK_OPENSSLTHREADLOCK_H
