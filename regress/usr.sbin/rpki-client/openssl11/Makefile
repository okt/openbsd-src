.if ! exists(/usr/local/bin/eopenssl11)
regress:
	@echo 'Run "pkg_add openssl--%1.1" to run tests against OpenSSL 1.1'
	@echo SKIPPED
.endif

LDADD +=	-Wl,-rpath,/usr/local/lib/eopenssl11 -L/usr/local/lib/eopenssl11
CFLAGS +=	-I${.CURDIR}/ -I/usr/local/include/eopenssl11/

# For mft.c we need ASN1_time_parse() and ASN1_time_tm_cmp() from LibreSSL

# Provide a missing prototype
a_time_tm_gen.c: a_time_tm.c
	echo '#include <openssl/asn1.h>\n' > $@.tmp
	echo '#define ASN1error(err) ASN1err(0, (err));' >> $@.tmp
	cat $> >> $@.tmp
	mv -f $@.tmp $@

CLEANFILES += a_time_tm_gen.c a_time_tm_gen.c.tmp

SRCS_test-ip =		a_time_tm_gen.c o_time.c
SRCS_test-mft =		a_time_tm_gen.c o_time.c
SRCS_test-roa =		a_time_tm_gen.c o_time.c
SRCS_test-cert =	a_time_tm_gen.c o_time.c
SRCS_test-gbr =		a_time_tm_gen.c o_time.c
SRCS_test-tal =		a_time_tm_gen.c o_time.c
SRCS_test-bgpsec =	a_time_tm_gen.c o_time.c
CFLAGS +=	-I${.CURDIR}/../../../../lib/libcrypto/

.PATH:		${.CURDIR}/..
.PATH:		${.CURDIR}/../../../../lib/libcrypto
.PATH:		${.CURDIR}/../../../../lib/libcrypto/asn1

.include <bsd.regress.mk>
