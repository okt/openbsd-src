/*	$OpenBSD: test_parser_fuzz.c,v 1.5 2021/12/13 16:56:49 deraadt Exp $ */
/*
 * Fuzz tests for payload parsing
 *
 * Placed in the public domain
 */

#include <sys/socket.h>
#include <sys/queue.h>
#include <sys/uio.h>

#include <event.h>
#include <imsg.h>
#include <string.h>

#include "iked.h"
#include "ikev2.h"
#include "test_helper.h"

extern int	ikev2_pld_payloads(struct iked *, struct iked_message *,
		    size_t, size_t, u_int);

void		parser_fuzz_tests(void);

u_int8_t cookies[] = {
	0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0x00, 0x01,	/* initator cookie */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	/* responder cookie */
};

u_int8_t genhdr[] = {
	0x00, 0x20, 0x22, 0x08,	/* next, major/minor, exchange type, flags */
	0x00, 0x00, 0x00, 0x00,	/* message ID */
	0x00, 0x00, 0x00, 0x00	/* total length */
};

u_int8_t sa_pld[] = {
	0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x08, 0x01, 0x01, 0x00, 0x00
};

u_int8_t saxform_pld[] = {
	0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x3c,
	0x01, 0x01, 0x00, 0x06, 0x03, 0x00, 0x00, 0x08,
	0x03, 0x00, 0x00, 0x0c, 0x03, 0x00, 0x00, 0x0c,
	0x01, 0x00, 0x00, 0x0c, 0x80, 0x0e, 0x00, 0xc0,
	0x03, 0x00, 0x00, 0x08, 0x04, 0x00, 0x00, 0x0e,
	0x03, 0x00, 0x00, 0x08, 0x02, 0x00, 0x00, 0x05,
	0x03, 0x00, 0x00, 0x08, 0x02, 0x00, 0x00, 0x02,
	0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x00, 0x01
};

u_int8_t ke_pld[] = {
        0x00, 0x00, 0x01, 0x08, 0x00, 0x0e, 0x00, 0x00, 0x16, 0xcb,
        0x68, 0xaf, 0x63, 0xfe, 0xb0, 0x58, 0x49, 0x0e, 0x7f, 0x85,
        0x60, 0x53, 0x80, 0xae, 0x3f, 0x82, 0xf3, 0x35, 0x21, 0xd5,
        0xae, 0x09, 0x1c, 0xfa, 0x68, 0xc2, 0xfb, 0x4b, 0xb3, 0x84,
        0xda, 0xaf, 0x6e, 0xe2, 0x5e, 0xc5, 0xb6, 0x8c, 0x35, 0x3c,
        0xec, 0x58, 0x7f, 0xa9, 0xf8, 0xa4, 0x24, 0xf3, 0xf8, 0xf4,
        0x65, 0x59, 0x8c, 0x15, 0x4d, 0x2c, 0xf1, 0x5d, 0xeb, 0x57,
        0x68, 0xfe, 0x75, 0x61, 0x5a, 0x80, 0x96, 0xa4, 0x0a, 0xad,
        0x75, 0x71, 0xd8, 0xe0, 0x06, 0xbc, 0xde, 0x16, 0x6d, 0x1e,
        0xd9, 0x5d, 0x2c, 0x00, 0x66, 0x43, 0x82, 0xe4, 0x6f, 0x5f,
        0x95, 0xe7, 0x9b, 0xfd, 0xf2, 0xe2, 0xcb, 0xc5, 0xf1, 0x52,
        0xdd, 0x3b, 0xed, 0x88, 0xd4, 0xa9, 0x13, 0x4e, 0x42, 0xe8,
        0x60, 0x2d, 0x3c, 0xf6, 0xc8, 0xf0, 0x70, 0x42, 0xfa, 0x33,
        0x7f, 0x28, 0xdf, 0x6b, 0x79, 0x2c, 0x79, 0x8f, 0xc0, 0x5d,
        0x81, 0x7a, 0x62, 0xdb, 0xd4, 0x44, 0x3a, 0x3c, 0x21, 0xbf,
        0x85, 0xc8, 0x0b, 0x8c, 0x77, 0x72, 0xe9, 0xfb, 0x50, 0x5c,
        0x03, 0xa6, 0xb2, 0x3f, 0x17, 0x4a, 0xd1, 0xb3, 0x01, 0x30,
        0xad, 0xe4, 0xfa, 0xe2, 0xba, 0x6f, 0x22, 0x83, 0xf4, 0xde,
        0x38, 0x43, 0xe8, 0x27, 0x00, 0xb8, 0x95, 0xbe, 0x03, 0x8f,
        0xcd, 0xd3, 0x72, 0xed, 0xa5, 0xed, 0x8d, 0xf4, 0x68, 0x98,
        0xef, 0x59, 0xcc, 0xfb, 0x54, 0x89, 0xde, 0xa9, 0xd4, 0x88,
        0xcd, 0xb9, 0xca, 0x09, 0xd3, 0xd5, 0x25, 0xb1, 0x8c, 0x58,
        0x12, 0x9c, 0x69, 0x03, 0x72, 0x00, 0xc9, 0xca, 0x95, 0x8a,
        0xce, 0x0d, 0xd2, 0xc8, 0x25, 0xe7, 0x7c, 0xed, 0x5e, 0xee,
        0x35, 0x01, 0xfc, 0x00, 0x56, 0xed, 0xf3, 0x8d, 0x81, 0x6c,
        0x3e, 0x86, 0x6a, 0x40, 0xac, 0xc7, 0x9c, 0x7a, 0xbf, 0x9f,
        0x8e, 0x1f, 0xd8, 0x60
};

u_int8_t nonce_pld[] = {
        0x00, 0x00, 0x00, 0x24, 0x5f, 0x61, 0x42, 0x72, 0x7d, 0xb2,
        0xa8, 0xc1, 0xfe, 0xb1, 0x38, 0x2e, 0xb8, 0x75, 0xa7, 0xc1,
        0x1d, 0x8a, 0xa7, 0xb7, 0x9b, 0x92, 0xe2, 0x0e, 0x3a, 0x18,
        0x20, 0xb6, 0x16, 0xf3, 0x35, 0x67,
};

u_int8_t notify_pld[] = {
        0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x40, 0x04, 0xc7, 0xa0,
        0x68, 0x68, 0x09, 0x0a, 0x7f, 0x12, 0x0b, 0x13, 0xd3, 0x2f,
        0xde, 0x64, 0x8b, 0xf1, 0xc3, 0x3c, 0x79, 0x8f, 0x00, 0x00,
        0x00, 0x1c, 0x00, 0x00, 0x40, 0x05, 0x9f, 0xbc, 0x8c, 0xd0,
        0x91, 0x5e, 0xa0, 0x87, 0x81, 0xab, 0x4f, 0xa1, 0x8a, 0xa7,
        0xa8, 0xf9, 0xeb, 0xdf, 0x9f, 0x2c
};

u_int8_t id_pld[] = {
	0x00, 0x00, 0x00, 0x0c, 0x01, 0x00, 0x00, 0x00,
	0xac, 0x12, 0x7d, 0x01
};

u_int8_t cert_pld[] = {
	0x00, 0x00, 0x01, 0x10, 0x0b, 0x00, 0x00, 0x00,
        0x30, 0x82, 0x01, 0x0c, 0x02, 0x82, 0x01, 0x01, 0x00, 0x8a,
        0x26, 0xf8, 0x9e, 0xe8, 0x09, 0x11, 0x6b, 0x3d, 0x00, 0xd3,
        0x25, 0xf8, 0x9f, 0xe8, 0x09, 0x11, 0x6b, 0x3d, 0x10, 0xd3,
        0x0b, 0x9a, 0xb0, 0xb7, 0xe4, 0x3e, 0x40, 0x59, 0xd7, 0x51,
        0x03, 0xaf, 0x09, 0x79, 0x1b, 0x0d, 0x63, 0x66, 0x28, 0xaa,
        0x97, 0xc8, 0x20, 0x4b, 0x28, 0x9b, 0x5e, 0x8c, 0xa9, 0x8f,
        0x73, 0x81, 0xb4, 0xfa, 0xf4, 0xdd, 0x05, 0x69, 0x0b, 0x71,
        0x72, 0xd8, 0xbb, 0xac, 0x4b, 0x6d, 0x67, 0x5a, 0xa2, 0x63,
        0x5d, 0x6d, 0x27, 0xc5, 0xf4, 0xe6, 0x0a, 0xbd, 0x2b, 0x0a,
        0x64, 0xb2, 0xcf, 0x59, 0x63, 0x9b, 0x5c, 0x4f, 0x26, 0x36,
        0xe3, 0x10, 0x70, 0x3c, 0x39, 0x77, 0x55, 0x07, 0x1c, 0x12,
        0xde, 0x60, 0x53, 0xa1, 0x70, 0xf4, 0xda, 0xfc, 0xcc, 0xec,
        0xad, 0x6d, 0x34, 0xad, 0xe2, 0x36, 0x10, 0x93, 0x59, 0x0c,
        0x81, 0x8d, 0x22, 0x7e, 0x57, 0xeb, 0x89, 0x26, 0xdb, 0x6e,
        0x99, 0x9a, 0xde, 0xbe, 0xad, 0xef, 0xca, 0xaf, 0xfe, 0xfe,
        0x99, 0x9a, 0xde, 0xbe, 0xad, 0xef, 0xca, 0xaf, 0xfe, 0xfe,
        0x6f, 0xd4, 0xe4, 0x63, 0x6c, 0x3e, 0x83, 0x09, 0xf4, 0x32,
        0x78, 0x3b, 0x71, 0xe9, 0x36, 0xb6, 0x92, 0xf6, 0xa8, 0x31,
        0x4d, 0x7c, 0xd0, 0xa1, 0x30, 0x55, 0xb6, 0x6b, 0x9e, 0xb7,
        0x41, 0xa8, 0x77, 0x6c, 0x96, 0xb8, 0xa2, 0x0c, 0x7d, 0x70,
        0xca, 0x51, 0xb9, 0xad, 0xc5, 0x75, 0xa7, 0xf1, 0x1e, 0x0e,
        0xca, 0x51, 0xb9, 0xad, 0xc5, 0x75, 0xa7, 0xf1, 0x1e, 0x0e,
        0xf2, 0xcf, 0x69, 0xbf, 0x20, 0xe9, 0x97, 0x05, 0xdd, 0xf3,
        0xf2, 0xcf, 0x69, 0xbf, 0x20, 0xe9, 0x97, 0x05, 0xdd, 0xf3,
        0x32, 0x58, 0x37, 0x8c, 0x5d, 0x02, 0x05, 0x00, 0xd1, 0x76,
        0x67, 0x01, 0x67, 0x75, 0x3b, 0xba, 0x45, 0xc2, 0xa2, 0x77,
        0x3b, 0x7e, 0xb4, 0x03, 0x88, 0x08, 0x93, 0xfe, 0x07, 0x51,
        0x8e, 0xcf
};

u_int8_t certreq_pld[] = {
	0x00, 0x00, 0x00, 0x05, 0x0b
};

u_int8_t auth_pld[] = {
	0x00, 0x00, 0x01, 0x08, 0x01, 0x00, 0x00, 0x00,
        0x2a, 0x34, 0x80, 0x52, 0x3c, 0x86, 0x1c, 0xfa, 0x9a, 0x2b,
        0x8b, 0xff, 0xbb, 0xb5, 0x0d, 0x6b, 0xa1, 0x62, 0x58, 0xd8,
        0x16, 0xaa, 0x15, 0xe4, 0x34, 0x24, 0xca, 0xc3, 0x09, 0x08,
        0x51, 0x69, 0x69, 0xef, 0xbd, 0xb7, 0xd4, 0xc5, 0x4f, 0x6c,
        0x12, 0xd5, 0xd0, 0x0b, 0xc7, 0x66, 0x0d, 0xcb, 0x6d, 0x01,
        0x7b, 0x8c, 0xec, 0x3d, 0x98, 0xe5, 0x2a, 0xac, 0x11, 0xde,
        0x88, 0x2e, 0xf2, 0x22, 0x98, 0x13, 0x73, 0xa3, 0x38, 0xd0,
        0x43, 0xf4, 0xc6, 0xf0, 0xc1, 0x24, 0x1a, 0x7a, 0x9f, 0xba,
        0x03, 0x25, 0x49, 0xe5, 0x8e, 0xb7, 0x5d, 0x79, 0x76, 0xfd,
        0x22, 0x5c, 0xba, 0x82, 0xb8, 0x75, 0x81, 0xc6, 0x79, 0xb3,
        0x56, 0x44, 0x82, 0x80, 0x5a, 0x3c, 0xe8, 0x21, 0xe4, 0xdb,
        0xfd, 0x1c, 0xd3, 0x18, 0xbd, 0x74, 0x22, 0x25, 0x44, 0xde,
        0x0b, 0x7e, 0x6e, 0xdb, 0xe3, 0x3b, 0x17, 0xc1, 0x4d, 0x5e,
        0x51, 0x87, 0xb0, 0x5a, 0xce, 0x5f, 0x23, 0xce, 0x18, 0x61,
        0x03, 0x02, 0x7e, 0x4b, 0x36, 0xb0, 0x7c, 0x90, 0xcf, 0xac,
        0x81, 0xc4, 0x45, 0xa3, 0x50, 0x01, 0x2e, 0x0a, 0xce, 0x62,
        0x7a, 0xe0, 0xa7, 0xc0, 0x45, 0x5e, 0x90, 0xe2, 0x2e, 0xc6,
        0x90, 0xe9, 0xbe, 0x8f, 0xe9, 0x31, 0xa9, 0xc9, 0x44, 0x62,
        0x31, 0xb6, 0x13, 0xaf, 0xd5, 0x9a, 0x55, 0x9b, 0x14, 0xf9,
        0x80, 0xcc, 0x73, 0xe3, 0x51, 0xdf, 0x2a, 0x04, 0x79, 0x0d,
        0x04, 0xee, 0x4c, 0xa8, 0x9d, 0xaa, 0x67, 0x2f, 0x77, 0x87,
        0x5e, 0x2d, 0x05, 0x95, 0xbe, 0x53, 0x45, 0x96, 0x8b, 0x89,
        0x79, 0x5b, 0x48, 0xe2, 0x6f, 0x3a, 0xc9, 0xef, 0x83, 0x81,
        0xcc, 0x4c, 0xfe, 0xb7, 0x40, 0x2d, 0xa5, 0xa5, 0x51, 0xb7,
        0xad, 0x2f, 0x29, 0xd8, 0xc8, 0x02, 0xbe, 0x18, 0x09, 0xd0,
        0xba, 0x71, 0x77, 0xfe, 0x2c, 0x6d
};

u_int8_t delete_pld[] = {
	0x2a, 0x00, 0x00, 0x10, 0x01, 0x08, 0x00, 0x01,	/* IKE SA */
	0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xaf, 0xfe,
	0x00, 0x00, 0x00, 0x10, 0x03, 0x04, 0x00, 0x02, /* ESP SA */
	0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11
};

u_int8_t vendor_pld[] = {
	0x00, 0x00, 0x00, 0x08, 0x11, 0x22, 0x33, 0x44
};

u_int8_t ts_pld[] = {
	0x00, 0x00, 0x00, 0x18, 0x01, 0x00, 0x00, 0x00,
	0x07, 0x00, 0x00, 0x10, 0x00, 0x00, 0xff, 0xff,
	0xac, 0x28, 0x7d, 0x00, 0xac, 0x28, 0x7d, 0xff
};

uint8_t skf_1of1_pld[] = {
	0x21, 0x00, 0x01, 0x98, 0x00, 0x01, 0x00, 0x01, 0x14, 0x77,
	0x25, 0x7b, 0x82, 0xc0, 0xdb, 0x0b, 0x24, 0x36, 0x36, 0x13,
	0x36, 0xe4, 0x99, 0xad, 0xf5, 0xaf, 0x26, 0x6f, 0x47, 0xd2,
	0x0d, 0x65, 0xe1, 0xa8, 0xcb, 0x35, 0x1e, 0x53, 0xce, 0x6d,
	0x8e, 0xf9, 0xe4, 0x51, 0xe3, 0x27, 0x10, 0x43, 0x38, 0x84,
	0x54, 0x1d, 0x7a, 0x1a, 0x89, 0x34, 0x06, 0xb3, 0x62, 0x86,
	0x98, 0x3b, 0x39, 0x91, 0x6e, 0xe8, 0x65, 0x3e, 0x31, 0xa8,
	0x08, 0xfe, 0x83, 0x56, 0x30, 0xd3, 0xe0, 0xfd, 0x73, 0x92,
	0x85, 0x2d, 0xae, 0x1d, 0x7d, 0xdb, 0x47, 0x05, 0x57, 0xe7,
	0x8e, 0xc5, 0xa5, 0x1b, 0x0e, 0x85, 0x1f, 0x12, 0x6d, 0xe6,
	0xdb, 0x3a, 0x3e, 0x99, 0xd1, 0x23, 0x41, 0xa4, 0x1c, 0x46,
	0x38, 0xd1, 0xa8, 0x84, 0x96, 0x13, 0xdb, 0x2a, 0x1d, 0x3b,
	0xb8, 0xd2, 0x04, 0xb3, 0x0d, 0xb4, 0x71, 0x90, 0xdb, 0xf6,
	0x2d, 0x60, 0x01, 0xc2, 0xb2, 0x89, 0xbd, 0xe9, 0x95, 0x7b,
	0x53, 0xa4, 0x94, 0x7e, 0x12, 0xe9, 0x5f, 0xfc, 0x51, 0x17,
	0x94, 0x3e, 0xba, 0xc2, 0xa5, 0x4d, 0x3a, 0x4d, 0x4b, 0x95,
	0x6d, 0x91, 0xc2, 0xb0, 0x2d, 0xb7, 0x24, 0xe8, 0x3b, 0xbd,
	0xe0, 0xcc, 0x09, 0x50, 0x11, 0x83, 0xc0, 0xcd, 0x29, 0x33,
	0xd5, 0x8f, 0x8a, 0xd1, 0xe3, 0xe8, 0x4f, 0x6a, 0x10, 0x4a,
	0x64, 0x97, 0x0f, 0x38, 0x58, 0x8d, 0x7f, 0x5d, 0xb4, 0x6b,
	0xa0, 0x42, 0x5e, 0x95, 0xe6, 0x08, 0x3e, 0x01, 0xf8, 0x82,
	0x90, 0x81, 0xd4, 0x70, 0xb5, 0xb2, 0x8c, 0x64, 0xa9, 0x56,
	0xdd, 0xc2, 0xda, 0xe1, 0xd3, 0xad, 0xf8, 0x5b, 0x99, 0x0b,
	0x19, 0x5e, 0x88, 0x0d, 0x81, 0x04, 0x4d, 0xc1, 0x43, 0x41,
	0xf1, 0xd3, 0x45, 0x65, 0x62, 0x70, 0x2f, 0xfa, 0x62, 0xbe,
	0x7d, 0xf4, 0x94, 0x91, 0xe0, 0xbb, 0xb1, 0xbc, 0xe5, 0x27,
	0xc8, 0x15, 0xd4, 0xcb, 0x82, 0x97, 0x15, 0x46, 0x82, 0xbb,
	0x48, 0xbb, 0x16, 0x25, 0xbe, 0x82, 0xe4, 0x27, 0x80, 0xf3,
	0xc2, 0x92, 0x3b, 0xd6, 0xc3, 0x65, 0x20, 0xec, 0x50, 0xdb,
	0x6a, 0xcb, 0x47, 0x73, 0xf7, 0x98, 0xf1, 0x66, 0x5e, 0xc4,
	0xe9, 0x87, 0xf8, 0xcb, 0x1e, 0x06, 0xa7, 0x67, 0xf5, 0xec,
	0x73, 0xe5, 0xc7, 0x4d, 0xc2, 0x90, 0xe4, 0xdf, 0x9d, 0x1f,
	0x05, 0x67, 0x99, 0xd6, 0xf0, 0xc4, 0x20, 0xbc, 0xf8, 0xf5,
	0x3e, 0x19, 0xe9, 0x3a, 0x12, 0xe1, 0xcc, 0x9f, 0x81, 0x55,
	0x1e, 0xad, 0xc8, 0xa3, 0xe5, 0x98, 0xbe, 0xe0, 0x4d, 0xb7,
	0x6b, 0xd5, 0xbe, 0x6a, 0x3d, 0x76, 0xb6, 0xe2, 0xa5, 0xa7,
	0x96, 0x68, 0xeb, 0x91, 0xee, 0x02, 0xfc, 0xe4, 0x01, 0xc3,
	0x24, 0xda, 0x4c, 0xff, 0x10, 0x27, 0x78, 0xb0, 0x0b, 0x55,
	0x5c, 0xce, 0x62, 0x7d, 0x33, 0x2b, 0x25, 0x99, 0xaa, 0x99,
	0xea, 0xa3, 0x1d, 0xd8, 0x2b, 0x57, 0xb5, 0xe4, 0x04, 0x21,
	0x75, 0xd9, 0xc4, 0xd0, 0x3d, 0xa1, 0xa5, 0x8f
};

u_int8_t sk_pld[] = {
        0x21, 0x00, 0x01, 0x94, 0x14, 0x77, 0x25, 0x7b, 0x82, 0xc0,
        0xdb, 0x0b, 0x24, 0x36, 0x36, 0x13, 0x36, 0xe4, 0x99, 0xad,
        0xf5, 0xaf, 0x26, 0x6f, 0x47, 0xd2, 0x0d, 0x65, 0xe1, 0xa8,
        0xcb, 0x35, 0x1e, 0x53, 0xce, 0x6d, 0x8e, 0xf9, 0xe4, 0x51,
        0xe3, 0x27, 0x10, 0x43, 0x38, 0x84, 0x54, 0x1d, 0x7a, 0x1a,
        0x89, 0x34, 0x06, 0xb3, 0x62, 0x86, 0x98, 0x3b, 0x39, 0x91,
        0x6e, 0xe8, 0x65, 0x3e, 0x31, 0xa8, 0x08, 0xfe, 0x83, 0x56,
        0x30, 0xd3, 0xe0, 0xfd, 0x73, 0x92, 0x85, 0x2d, 0xae, 0x1d,
        0x7d, 0xdb, 0x47, 0x05, 0x57, 0xe7, 0x8e, 0xc5, 0xa5, 0x1b,
        0x0e, 0x85, 0x1f, 0x12, 0x6d, 0xe6, 0xdb, 0x3a, 0x3e, 0x99,
        0xd1, 0x23, 0x41, 0xa4, 0x1c, 0x46, 0x38, 0xd1, 0xa8, 0x84,
        0x96, 0x13, 0xdb, 0x2a, 0x1d, 0x3b, 0xb8, 0xd2, 0x04, 0xb3,
        0x0d, 0xb4, 0x71, 0x90, 0xdb, 0xf6, 0x2d, 0x60, 0x01, 0xc2,
        0xb2, 0x89, 0xbd, 0xe9, 0x95, 0x7b, 0x53, 0xa4, 0x94, 0x7e,
        0x12, 0xe9, 0x5f, 0xfc, 0x51, 0x17, 0x94, 0x3e, 0xba, 0xc2,
        0xa5, 0x4d, 0x3a, 0x4d, 0x4b, 0x95, 0x6d, 0x91, 0xc2, 0xb0,
        0x2d, 0xb7, 0x24, 0xe8, 0x3b, 0xbd, 0xe0, 0xcc, 0x09, 0x50,
        0x11, 0x83, 0xc0, 0xcd, 0x29, 0x33, 0xd5, 0x8f, 0x8a, 0xd1,
        0xe3, 0xe8, 0x4f, 0x6a, 0x10, 0x4a, 0x64, 0x97, 0x0f, 0x38,
        0x58, 0x8d, 0x7f, 0x5d, 0xb4, 0x6b, 0xa0, 0x42, 0x5e, 0x95,
        0xe6, 0x08, 0x3e, 0x01, 0xf8, 0x82, 0x90, 0x81, 0xd4, 0x70,
        0xb5, 0xb2, 0x8c, 0x64, 0xa9, 0x56, 0xdd, 0xc2, 0xda, 0xe1,
        0xd3, 0xad, 0xf8, 0x5b, 0x99, 0x0b, 0x19, 0x5e, 0x88, 0x0d,
        0x81, 0x04, 0x4d, 0xc1, 0x43, 0x41, 0xf1, 0xd3, 0x45, 0x65,
        0x62, 0x70, 0x2f, 0xfa, 0x62, 0xbe, 0x7d, 0xf4, 0x94, 0x91,
        0xe0, 0xbb, 0xb1, 0xbc, 0xe5, 0x27, 0xc8, 0x15, 0xd4, 0xcb,
        0x82, 0x97, 0x15, 0x46, 0x82, 0xbb, 0x48, 0xbb, 0x16, 0x25,
        0xbe, 0x82, 0xe4, 0x27, 0x80, 0xf3, 0xc2, 0x92, 0x3b, 0xd6,
        0xc3, 0x65, 0x20, 0xec, 0x50, 0xdb, 0x6a, 0xcb, 0x47, 0x73,
        0xf7, 0x98, 0xf1, 0x66, 0x5e, 0xc4, 0xe9, 0x87, 0xf8, 0xcb,
        0x1e, 0x06, 0xa7, 0x67, 0xf5, 0xec, 0x73, 0xe5, 0xc7, 0x4d,
        0xc2, 0x90, 0xe4, 0xdf, 0x9d, 0x1f, 0x05, 0x67, 0x99, 0xd6,
        0xf0, 0xc4, 0x20, 0xbc, 0xf8, 0xf5, 0x3e, 0x19, 0xe9, 0x3a,
        0x12, 0xe1, 0xcc, 0x9f, 0x81, 0x55, 0x1e, 0xad, 0xc8, 0xa3,
        0xe5, 0x98, 0xbe, 0xe0, 0x4d, 0xb7, 0x6b, 0xd5, 0xbe, 0x6a,
        0x3d, 0x76, 0xb6, 0xe2, 0xa5, 0xa7, 0x96, 0x68, 0xeb, 0x91,
        0xee, 0x02, 0xfc, 0xe4, 0x01, 0xc3, 0x24, 0xda, 0x4c, 0xff,
        0x10, 0x27, 0x78, 0xb0, 0x0b, 0x55, 0x5c, 0xce, 0x62, 0x7d,
        0x33, 0x2b, 0x25, 0x99, 0xaa, 0x99, 0xea, 0xa3, 0x1d, 0xd8,
        0x2b, 0x57, 0xb5, 0xe4, 0x04, 0x21, 0x75, 0xd9, 0xc4, 0xd0,
        0x3d, 0xa1, 0xa5, 0x8f
};

u_int8_t cp_pld[] = {
	0x2f, 0x00, 0x00, 0x0c,
	0x01, 0x00, 0x00, 0x00,	/* REQUEST */
	0x00, 0x01, 0x00, 0x00,	/* INTERNAL_IP4_ADDRESS */
	0x2f, 0x00, 0x00, 0x10,
	0x02, 0x00, 0x00, 0x00,	/* REPLY */
	0x00, 0x01, 0x00, 0x04,	/* INTERNAL_IP4_ADDRESS */
	0xaa, 0xbb, 0xcc, 0xdd,	/* 170.187.204.221 */
	0x2f, 0x00, 0x00, 0x08,
	0x03, 0x00, 0x00, 0x00,	/* SET (empty) */
	0x2f, 0x00, 0x00, 0x24,
	0x02, 0x00, 0x00, 0x00,	/* REPLY */
	0x00, 0x01, 0x00, 0x04,	/* INTERNAL_IP4_ADDRESS */
	0xaa, 0xaa, 0xaa, 0xaa,	/* 170.170.170.170 */
	0x00, 0x02, 0x00, 0x04,	/* INTERNAL_IP4_NETMASK */
	0xbb, 0xbb, 0xbb, 0xbb,	/* 187.187.187.187 */
	0x00, 0x03, 0x00, 0x04,	/* INTERNAL_IP4_DNS */
	0xcc, 0xcc, 0xcc, 0xcc,	/* 204.204.204.204 */
	0x00, 0x08, 0x00, 0x00,	/* INTERNAL_IP6_ADDRESS */
	0x00, 0x00, 0x00, 0x08,
	0x04, 0x00, 0x00, 0x00,	/* ACK (empty) */
};

u_int8_t eap_pld[] = {
	0x30, 0x00, 0x00, 0x09,
	0x01, 0x00, 0x00, 0x05, 0x01,
	0x30, 0x00, 0x00, 0x0c,
	0x02, 0x00, 0x00, 0x05, 0x01, 0xfa, 0xfb, 0xfc,
	0x30, 0x00, 0x00, 0x08,
	0x03, 0x00, 0x00, 0x04,
	0x00, 0x00, 0x00, 0x08,
	0x04, 0x00, 0x00, 0x04
};

/* Valid initator packet */
u_int8_t valid_packet[] = {
        0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x20, 0x22, 0x08,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x22, 0x00,
        0x00, 0x40, 0x00, 0x00, 0x00, 0x3c, 0x01, 0x01, 0x00, 0x06,
        0x03, 0x00, 0x00, 0x08, 0x03, 0x00, 0x00, 0x0c, 0x03, 0x00,
        0x00, 0x0c, 0x01, 0x00, 0x00, 0x0c, 0x80, 0x0e, 0x00, 0xc0,
        0x03, 0x00, 0x00, 0x08, 0x04, 0x00, 0x00, 0x0e, 0x03, 0x00,
        0x00, 0x08, 0x02, 0x00, 0x00, 0x05, 0x03, 0x00, 0x00, 0x08,
        0x02, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x08, 0x02, 0x00,
        0x00, 0x01, 0x28, 0x00, 0x01, 0x08, 0x00, 0x0e, 0x00, 0x00,
        0x16, 0xcb, 0x68, 0xaf, 0x63, 0xfe, 0xb0, 0x58, 0x49, 0x0e,
        0x7f, 0x85, 0x60, 0x53, 0x80, 0xae, 0x3f, 0x82, 0xf3, 0x35,
        0x21, 0xd5, 0xae, 0x09, 0x1c, 0xfa, 0x68, 0xc2, 0xfb, 0x4b,
        0xb3, 0x84, 0xda, 0xaf, 0x6e, 0xe2, 0x5e, 0xc5, 0xb6, 0x8c,
        0x35, 0x3c, 0xec, 0x58, 0x7f, 0xa9, 0xf8, 0xa4, 0x24, 0xf3,
        0xf8, 0xf4, 0x65, 0x59, 0x8c, 0x15, 0x4d, 0x2c, 0xf1, 0x5d,
        0xeb, 0x57, 0x68, 0xfe, 0x75, 0x61, 0x5a, 0x80, 0x96, 0xa4,
        0x0a, 0xad, 0x75, 0x71, 0xd8, 0xe0, 0x06, 0xbc, 0xde, 0x16,
        0x6d, 0x1e, 0xd9, 0x5d, 0x2c, 0x00, 0x66, 0x43, 0x82, 0xe4,
        0x6f, 0x5f, 0x95, 0xe7, 0x9b, 0xfd, 0xf2, 0xe2, 0xcb, 0xc5,
        0xf1, 0x52, 0xdd, 0x3b, 0xed, 0x88, 0xd4, 0xa9, 0x13, 0x4e,
        0x42, 0xe8, 0x60, 0x2d, 0x3c, 0xf6, 0xc8, 0xf0, 0x70, 0x42,
        0xfa, 0x33, 0x7f, 0x28, 0xdf, 0x6b, 0x79, 0x2c, 0x79, 0x8f,
        0xc0, 0x5d, 0x81, 0x7a, 0x62, 0xdb, 0xd4, 0x44, 0x3a, 0x3c,
        0x21, 0xbf, 0x85, 0xc8, 0x0b, 0x8c, 0x77, 0x72, 0xe9, 0xfb,
        0x50, 0x5c, 0x03, 0xa6, 0xb2, 0x3f, 0x17, 0x4a, 0xd1, 0xb3,
        0x01, 0x30, 0xad, 0xe4, 0xfa, 0xe2, 0xba, 0x6f, 0x22, 0x83,
        0xf4, 0xde, 0x38, 0x43, 0xe8, 0x27, 0x00, 0xb8, 0x95, 0xbe,
        0x03, 0x8f, 0xcd, 0xd3, 0x72, 0xed, 0xa5, 0xed, 0x8d, 0xf4,
        0x68, 0x98, 0xef, 0x59, 0xcc, 0xfb, 0x54, 0x89, 0xde, 0xa9,
        0xd4, 0x88, 0xcd, 0xb9, 0xca, 0x09, 0xd3, 0xd5, 0x25, 0xb1,
        0x8c, 0x58, 0x12, 0x9c, 0x69, 0x03, 0x72, 0x00, 0xc9, 0xca,
        0x95, 0x8a, 0xce, 0x0d, 0xd2, 0xc8, 0x25, 0xe7, 0x7c, 0xed,
        0x5e, 0xee, 0x35, 0x01, 0xfc, 0x00, 0x56, 0xed, 0xf3, 0x8d,
        0x81, 0x6c, 0x3e, 0x86, 0x6a, 0x40, 0xac, 0xc7, 0x9c, 0x7a,
        0xbf, 0x9f, 0x8e, 0x1f, 0xd8, 0x60, 0x29, 0x00, 0x00, 0x24,
        0x5f, 0x61, 0x42, 0x72, 0x7d, 0xb2, 0xa8, 0xc1, 0xfe, 0xb1,
        0x38, 0x2e, 0xb8, 0x75, 0xa7, 0xc1, 0x1d, 0x8a, 0xa7, 0xb7,
        0x9b, 0x92, 0xe2, 0x0e, 0x3a, 0x18, 0x20, 0xb6, 0x16, 0xf3,
        0x35, 0x67, 0x29, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x40, 0x04,
        0xc7, 0xa0, 0x68, 0x68, 0x09, 0x0a, 0x7f, 0x12, 0x0b, 0x13,
        0xd3, 0x2f, 0xde, 0x64, 0x8b, 0xf1, 0xc3, 0x3c, 0x79, 0x8f,
        0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x40, 0x05, 0x9f, 0xbc,
        0x8c, 0xd0, 0x91, 0x5e, 0xa0, 0x87, 0x81, 0xab, 0x4f, 0xa1,
        0x8a, 0xa7, 0xa8, 0xf9, 0xeb, 0xdf, 0x9f, 0x2c
};

#define OFFSET_ICOOKIE		0
#define OFFSET_RCOOKIE		8
#define OFFSET_NEXTPAYLOAD	(0 + sizeof(cookies))
#define OFFSET_VERSION		(1 + sizeof(cookies))
#define OFFSET_EXCHANGE		(2 + sizeof(cookies))
#define OFFSET_LENGTH		(8 + sizeof(cookies))

static u_int8_t *
get_icookie(u_int8_t *data)
{
	return &data[OFFSET_ICOOKIE];
}

static u_int8_t *
get_rcookie(u_int8_t *data)
{
	return &data[OFFSET_RCOOKIE];
}

static u_int8_t
get_nextpayload(u_int8_t *data)
{
	return data[OFFSET_NEXTPAYLOAD];
}

static u_int8_t
get_version(u_int8_t *data)
{
	return data[OFFSET_VERSION];
}

static u_int8_t
get_exchange(u_int8_t *data)
{
	return data[OFFSET_EXCHANGE];
}

static u_int32_t
get_length(u_int8_t *data)
{
	return *(u_int32_t *)&data[OFFSET_LENGTH];
}

static void
set_length(u_int8_t *data, u_int32_t length)
{
	u_int32_t	*p;

	p = (u_int32_t *)&data[OFFSET_LENGTH];
	*p = htobe32(length);
}

static void
set_nextpayload(u_int8_t *data, u_int8_t next)
{
	data[OFFSET_NEXTPAYLOAD] = next;
}

static void
prepare_header(struct ike_header *hdr, struct ibuf *data)
{
	bzero(hdr, sizeof(*hdr));
	bcopy(get_icookie(ibuf_data(data)), &hdr->ike_ispi,
	    sizeof(hdr->ike_ispi));
	bcopy(get_rcookie(ibuf_data(data)), &hdr->ike_rspi,
	    sizeof(hdr->ike_rspi));
	hdr->ike_nextpayload = get_nextpayload(ibuf_data(data));
	hdr->ike_version = get_version(ibuf_data(data));
	hdr->ike_exchange = get_exchange(ibuf_data(data));
	hdr->ike_length = get_length(ibuf_data(data));
}

static void
prepare_message(struct iked_message *msg, struct ibuf *data)
{
	static struct iked_sa	sa;

	bzero(&sa, sizeof(sa));
	bzero(msg, sizeof(*msg));

	msg->msg_sa = &sa;
	msg->msg_data = data;
	msg->msg_e = 1;
	msg->msg_parent = msg;
}

static void
perform_test(struct fuzz *fuzz)
{
	struct ibuf		*fuzzed;
	struct ike_header	 hdr;
	struct iked_message	 msg;

	bzero(&hdr, sizeof(hdr));
	bzero(&msg, sizeof(msg));

	for (; !fuzz_done(fuzz); fuzz_next(fuzz)) {
		ASSERT_PTR_NE(fuzzed = ibuf_new(fuzz_ptr(fuzz), fuzz_len(fuzz)),
		    NULL);
		print_hex(ibuf_data(fuzzed), 0, ibuf_size(fuzzed));

		/* We need at least cookies and generic header. */
		if (ibuf_size(fuzzed) < sizeof(cookies) + sizeof(genhdr)) {
			ibuf_free(fuzzed);
			continue;
		}

		prepare_header(&hdr, fuzzed);
		prepare_message(&msg, fuzzed);

		ikev2_pld_parse(NULL, &hdr, &msg, 0);

		ibuf_free(fuzzed);
	}
}

void
parser_fuzz_tests(void)
{
	struct fuzz		*fuzz;
	struct ike_header	 hdr;
	struct iked_message	 msg;
	struct ibuf		*data;

#if 0
	log_init(3);
#endif

	TEST_START("fuzz generic header");
	ASSERT_PTR_NE(data = ibuf_new(cookies, sizeof(cookies)), NULL);
	ASSERT_INT_EQ(ibuf_add(data, genhdr, sizeof(genhdr)), 0);
	set_length(ibuf_data(data), ibuf_size(data));
	print_hex(ibuf_data(data), 0, ibuf_size(data));
	prepare_header(&hdr, data);
	prepare_message(&msg, data);
	ASSERT_INT_EQ(ikev2_pld_parse(NULL, &hdr, &msg, 0), 0);
	fuzz = fuzz_begin(FUZZ_1_BIT_FLIP | FUZZ_2_BIT_FLIP |
	    FUZZ_1_BYTE_FLIP | FUZZ_2_BYTE_FLIP |
	    FUZZ_TRUNCATE_START | FUZZ_TRUNCATE_END |
	    FUZZ_BASE64,
	    ibuf_data(data), ibuf_size(data));
	ibuf_free(data);
	perform_test(fuzz);
	TEST_DONE();

	TEST_START("fuzz skf_1of1 payload");
	ASSERT_PTR_NE(data = ibuf_new(cookies, sizeof(cookies)), NULL);
	ASSERT_INT_EQ(ibuf_add(data, genhdr, sizeof(genhdr)), 0);
	ASSERT_INT_EQ(ibuf_add(data, skf_1of1_pld, sizeof(skf_1of1_pld)), 0);
	set_length(ibuf_data(data), ibuf_size(data));
	set_nextpayload(ibuf_data(data), IKEV2_PAYLOAD_SKF);
	print_hex(ibuf_data(data), 0, ibuf_size(data));
	prepare_header(&hdr, data);
	prepare_message(&msg, data);
	ASSERT_INT_EQ(ikev2_pld_parse(NULL, &hdr, &msg, 0), 0);
	fuzz = fuzz_begin(FUZZ_1_BIT_FLIP | FUZZ_2_BIT_FLIP |
	    FUZZ_1_BYTE_FLIP | FUZZ_2_BYTE_FLIP |
	    FUZZ_TRUNCATE_START | FUZZ_TRUNCATE_END |
	    FUZZ_BASE64,
	    ibuf_data(data), ibuf_size(data));
	ibuf_free(data);
	perform_test(fuzz);
	TEST_DONE();

	TEST_START("fuzz sa payload");
	ASSERT_PTR_NE(data = ibuf_new(cookies, sizeof(cookies)), NULL);
	ASSERT_INT_EQ(ibuf_add(data, genhdr, sizeof(genhdr)), 0);
	ASSERT_INT_EQ(ibuf_add(data, sa_pld, sizeof(sa_pld)), 0);
	set_length(ibuf_data(data), ibuf_size(data));
	set_nextpayload(ibuf_data(data), IKEV2_PAYLOAD_SA);
	print_hex(ibuf_data(data), 0, ibuf_size(data));
	prepare_header(&hdr, data);
	prepare_message(&msg, data);
	ASSERT_INT_EQ(ikev2_pld_parse(NULL, &hdr, &msg, 0), 0);
	fuzz = fuzz_begin(FUZZ_1_BIT_FLIP | FUZZ_2_BIT_FLIP |
	    FUZZ_1_BYTE_FLIP | FUZZ_2_BYTE_FLIP |
	    FUZZ_TRUNCATE_START | FUZZ_TRUNCATE_END |
	    FUZZ_BASE64,
	    ibuf_data(data), ibuf_size(data));
	ibuf_free(data);
	perform_test(fuzz);
	TEST_DONE();

	TEST_START("fuzz sa and xform payload");
	ASSERT_PTR_NE(data = ibuf_new(cookies, sizeof(cookies)), NULL);
	ASSERT_INT_EQ(ibuf_add(data, genhdr, sizeof(genhdr)), 0);
	ASSERT_INT_EQ(ibuf_add(data, saxform_pld, sizeof(saxform_pld)), 0);
	set_length(ibuf_data(data), ibuf_size(data));
	set_nextpayload(ibuf_data(data), IKEV2_PAYLOAD_SA);
	print_hex(ibuf_data(data), 0, ibuf_size(data));
	prepare_header(&hdr, data);
	prepare_message(&msg, data);
	ASSERT_INT_EQ(ikev2_pld_parse(NULL, &hdr, &msg, 0), 0);
	fuzz = fuzz_begin(FUZZ_1_BIT_FLIP | FUZZ_2_BIT_FLIP |
	    FUZZ_1_BYTE_FLIP | FUZZ_2_BYTE_FLIP |
	    FUZZ_TRUNCATE_START | FUZZ_TRUNCATE_END |
	    FUZZ_BASE64,
	    ibuf_data(data), ibuf_size(data));
	ibuf_free(data);
	perform_test(fuzz);
	TEST_DONE();

	TEST_START("fuzz ke payload");
	ASSERT_PTR_NE(data = ibuf_new(cookies, sizeof(cookies)), NULL);
	ASSERT_INT_EQ(ibuf_add(data, genhdr, sizeof(genhdr)), 0);
	ASSERT_INT_EQ(ibuf_add(data, ke_pld, sizeof(ke_pld)), 0);
	set_length(ibuf_data(data), ibuf_size(data));
	set_nextpayload(ibuf_data(data), IKEV2_PAYLOAD_KE);
	print_hex(ibuf_data(data), 0, ibuf_size(data));
	prepare_header(&hdr, data);
	prepare_message(&msg, data);
	ASSERT_INT_EQ(ikev2_pld_parse(NULL, &hdr, &msg, 0), 0);
	fuzz = fuzz_begin(FUZZ_1_BIT_FLIP | FUZZ_2_BIT_FLIP |
	    FUZZ_1_BYTE_FLIP | FUZZ_2_BYTE_FLIP |
	    FUZZ_TRUNCATE_START | FUZZ_TRUNCATE_END |
	    FUZZ_BASE64,
	    ibuf_data(data), ibuf_size(data));
	ibuf_free(data);
	perform_test(fuzz);
	TEST_DONE();

	TEST_START("fuzz nonce payload");
	ASSERT_PTR_NE(data = ibuf_new(cookies, sizeof(cookies)), NULL);
	ASSERT_INT_EQ(ibuf_add(data, genhdr, sizeof(genhdr)), 0);
	ASSERT_INT_EQ(ibuf_add(data, nonce_pld, sizeof(nonce_pld)), 0);
	set_length(ibuf_data(data), ibuf_size(data));
	set_nextpayload(ibuf_data(data), IKEV2_PAYLOAD_NONCE);
	print_hex(ibuf_data(data), 0, ibuf_size(data));
	prepare_header(&hdr, data);
	prepare_message(&msg, data);
	ASSERT_INT_EQ(ikev2_pld_parse(NULL, &hdr, &msg, 0), 0);
	fuzz = fuzz_begin(FUZZ_1_BIT_FLIP | FUZZ_2_BIT_FLIP |
	    FUZZ_1_BYTE_FLIP | FUZZ_2_BYTE_FLIP |
	    FUZZ_TRUNCATE_START | FUZZ_TRUNCATE_END |
	    FUZZ_BASE64,
	    ibuf_data(data), ibuf_size(data));
	ibuf_free(data);
	perform_test(fuzz);
	TEST_DONE();

	TEST_START("fuzz notify payload");
	ASSERT_PTR_NE(data = ibuf_new(cookies, sizeof(cookies)), NULL);
	ASSERT_INT_EQ(ibuf_add(data, genhdr, sizeof(genhdr)), 0);
	ASSERT_INT_EQ(ibuf_add(data, notify_pld, sizeof(notify_pld)), 0);
	set_length(ibuf_data(data), ibuf_size(data));
	set_nextpayload(ibuf_data(data), IKEV2_PAYLOAD_NOTIFY);
	print_hex(ibuf_data(data), 0, ibuf_size(data));
	prepare_header(&hdr, data);
	prepare_message(&msg, data);
	ASSERT_INT_EQ(ikev2_pld_parse(NULL, &hdr, &msg, 0), 0);
	fuzz = fuzz_begin(FUZZ_1_BIT_FLIP | FUZZ_2_BIT_FLIP |
	    FUZZ_1_BYTE_FLIP | FUZZ_2_BYTE_FLIP |
	    FUZZ_TRUNCATE_START | FUZZ_TRUNCATE_END |
	    FUZZ_BASE64,
	    ibuf_data(data), ibuf_size(data));
	ibuf_free(data);
	perform_test(fuzz);
	TEST_DONE();

	TEST_START("fuzz id payload");
	ASSERT_PTR_NE(data = ibuf_new(cookies, sizeof(cookies)), NULL);
	ASSERT_INT_EQ(ibuf_add(data, genhdr, sizeof(genhdr)), 0);
	ASSERT_INT_EQ(ibuf_add(data, id_pld, sizeof(id_pld)), 0);
	set_length(ibuf_data(data), ibuf_size(data));
	set_nextpayload(ibuf_data(data), IKEV2_PAYLOAD_IDi);
	print_hex(ibuf_data(data), 0, ibuf_size(data));
	prepare_header(&hdr, data);
	prepare_message(&msg, data);
	ASSERT_INT_EQ(ikev2_pld_parse(NULL, &hdr, &msg, 0), 0);
	fuzz = fuzz_begin(FUZZ_1_BIT_FLIP | FUZZ_2_BIT_FLIP |
	    FUZZ_1_BYTE_FLIP | FUZZ_2_BYTE_FLIP |
	    FUZZ_TRUNCATE_START | FUZZ_TRUNCATE_END |
	    FUZZ_BASE64,
	    ibuf_data(data), ibuf_size(data));
	ibuf_free(data);
	perform_test(fuzz);
	TEST_DONE();

	TEST_START("fuzz cert payload");
	ASSERT_PTR_NE(data = ibuf_new(cookies, sizeof(cookies)), NULL);
	ASSERT_INT_EQ(ibuf_add(data, genhdr, sizeof(genhdr)), 0);
	ASSERT_INT_EQ(ibuf_add(data, cert_pld, sizeof(cert_pld)), 0);
	set_length(ibuf_data(data), ibuf_size(data));
	set_nextpayload(ibuf_data(data), IKEV2_PAYLOAD_CERT);
	print_hex(ibuf_data(data), 0, ibuf_size(data));
	prepare_header(&hdr, data);
	prepare_message(&msg, data);
	ASSERT_INT_EQ(ikev2_pld_parse(NULL, &hdr, &msg, 0), 0);
	fuzz = fuzz_begin(FUZZ_1_BIT_FLIP | FUZZ_2_BIT_FLIP |
	    FUZZ_1_BYTE_FLIP | FUZZ_2_BYTE_FLIP |
	    FUZZ_TRUNCATE_START | FUZZ_TRUNCATE_END |
	    FUZZ_BASE64,
	    ibuf_data(data), ibuf_size(data));
	ibuf_free(data);
	perform_test(fuzz);
	TEST_DONE();

	TEST_START("fuzz certreq payload");
	ASSERT_PTR_NE(data = ibuf_new(cookies, sizeof(cookies)), NULL);
	ASSERT_INT_EQ(ibuf_add(data, genhdr, sizeof(genhdr)), 0);
	ASSERT_INT_EQ(ibuf_add(data, certreq_pld, sizeof(certreq_pld)), 0);
	set_length(ibuf_data(data), ibuf_size(data));
	set_nextpayload(ibuf_data(data), IKEV2_PAYLOAD_CERTREQ);
	print_hex(ibuf_data(data), 0, ibuf_size(data));
	prepare_header(&hdr, data);
	prepare_message(&msg, data);
	ASSERT_INT_EQ(ikev2_pld_parse(NULL, &hdr, &msg, 0), 0);
	fuzz = fuzz_begin(FUZZ_1_BIT_FLIP | FUZZ_2_BIT_FLIP |
	    FUZZ_1_BYTE_FLIP | FUZZ_2_BYTE_FLIP |
	    FUZZ_TRUNCATE_START | FUZZ_TRUNCATE_END |
	    FUZZ_BASE64,
	    ibuf_data(data), ibuf_size(data));
	ibuf_free(data);
	perform_test(fuzz);
	TEST_DONE();

	TEST_START("fuzz auth payload");
	ASSERT_PTR_NE(data = ibuf_new(cookies, sizeof(cookies)), NULL);
	ASSERT_INT_EQ(ibuf_add(data, genhdr, sizeof(genhdr)), 0);
	ASSERT_INT_EQ(ibuf_add(data, auth_pld, sizeof(auth_pld)), 0);
	set_length(ibuf_data(data), ibuf_size(data));
	set_nextpayload(ibuf_data(data), IKEV2_PAYLOAD_AUTH);
	print_hex(ibuf_data(data), 0, ibuf_size(data));
	prepare_header(&hdr, data);
	prepare_message(&msg, data);
	ASSERT_INT_EQ(ikev2_pld_parse(NULL, &hdr, &msg, 0), 0);
	fuzz = fuzz_begin(FUZZ_1_BIT_FLIP | FUZZ_2_BIT_FLIP |
	    FUZZ_1_BYTE_FLIP | FUZZ_2_BYTE_FLIP |
	    FUZZ_TRUNCATE_START | FUZZ_TRUNCATE_END |
	    FUZZ_BASE64,
	    ibuf_data(data), ibuf_size(data));
	ibuf_free(data);
	perform_test(fuzz);
	TEST_DONE();

	TEST_START("fuzz delete notify payload");
	ASSERT_PTR_NE(data = ibuf_new(cookies, sizeof(cookies)), NULL);
	ASSERT_INT_EQ(ibuf_add(data, genhdr, sizeof(genhdr)), 0);
	ASSERT_INT_EQ(ibuf_add(data, delete_pld, sizeof(delete_pld)), 0);
	set_length(ibuf_data(data), ibuf_size(data));
	set_nextpayload(ibuf_data(data), IKEV2_PAYLOAD_DELETE);
	print_hex(ibuf_data(data), 0, ibuf_size(data));
	prepare_header(&hdr, data);
	prepare_message(&msg, data);
	ASSERT_INT_EQ(ikev2_pld_parse(NULL, &hdr, &msg, 0), 0);
	fuzz = fuzz_begin(FUZZ_1_BIT_FLIP | FUZZ_2_BIT_FLIP |
	    FUZZ_1_BYTE_FLIP | FUZZ_2_BYTE_FLIP |
	    FUZZ_TRUNCATE_START | FUZZ_TRUNCATE_END |
	    FUZZ_BASE64,
	    ibuf_data(data), ibuf_size(data));
	ibuf_free(data);
	perform_test(fuzz);
	TEST_DONE();

	TEST_START("fuzz vendor id payload");
	ASSERT_PTR_NE(data = ibuf_new(cookies, sizeof(cookies)), NULL);
	ASSERT_INT_EQ(ibuf_add(data, genhdr, sizeof(genhdr)), 0);
	ASSERT_INT_EQ(ibuf_add(data, vendor_pld, sizeof(vendor_pld)), 0);
	set_length(ibuf_data(data), ibuf_size(data));
	set_nextpayload(ibuf_data(data), IKEV2_PAYLOAD_VENDOR);
	print_hex(ibuf_data(data), 0, ibuf_size(data));
	prepare_header(&hdr, data);
	prepare_message(&msg, data);
	ASSERT_INT_EQ(ikev2_pld_parse(NULL, &hdr, &msg, 0), 0);
	fuzz = fuzz_begin(FUZZ_1_BIT_FLIP | FUZZ_2_BIT_FLIP |
	    FUZZ_1_BYTE_FLIP | FUZZ_2_BYTE_FLIP |
	    FUZZ_TRUNCATE_START | FUZZ_TRUNCATE_END |
	    FUZZ_BASE64,
	    ibuf_data(data), ibuf_size(data));
	ibuf_free(data);
	perform_test(fuzz);
	TEST_DONE();

	TEST_START("fuzz traffic selector initiator payload");
	ASSERT_PTR_NE(data = ibuf_new(cookies, sizeof(cookies)), NULL);
	ASSERT_INT_EQ(ibuf_add(data, genhdr, sizeof(genhdr)), 0);
	ASSERT_INT_EQ(ibuf_add(data, ts_pld, sizeof(ts_pld)), 0);
	set_length(ibuf_data(data), ibuf_size(data));
	set_nextpayload(ibuf_data(data), IKEV2_PAYLOAD_TSi);
	print_hex(ibuf_data(data), 0, ibuf_size(data));
	prepare_header(&hdr, data);
	prepare_message(&msg, data);
	ASSERT_INT_EQ(ikev2_pld_parse(NULL, &hdr, &msg, 0), 0);
	fuzz = fuzz_begin(FUZZ_1_BIT_FLIP | FUZZ_2_BIT_FLIP |
	    FUZZ_1_BYTE_FLIP | FUZZ_2_BYTE_FLIP |
	    FUZZ_TRUNCATE_START | FUZZ_TRUNCATE_END |
	    FUZZ_BASE64,
	    ibuf_data(data), ibuf_size(data));
	ibuf_free(data);
	perform_test(fuzz);
	TEST_DONE();

	TEST_START("fuzz traffic selector responder payload");
	ASSERT_PTR_NE(data = ibuf_new(cookies, sizeof(cookies)), NULL);
	ASSERT_INT_EQ(ibuf_add(data, genhdr, sizeof(genhdr)), 0);
	ASSERT_INT_EQ(ibuf_add(data, ts_pld, sizeof(ts_pld)), 0);
	set_length(ibuf_data(data), ibuf_size(data));
	set_nextpayload(ibuf_data(data), IKEV2_PAYLOAD_TSr);
	print_hex(ibuf_data(data), 0, ibuf_size(data));
	prepare_header(&hdr, data);
	prepare_message(&msg, data);
	ASSERT_INT_EQ(ikev2_pld_parse(NULL, &hdr, &msg, 0), 0);
	fuzz = fuzz_begin(FUZZ_1_BIT_FLIP | FUZZ_2_BIT_FLIP |
	    FUZZ_1_BYTE_FLIP | FUZZ_2_BYTE_FLIP |
	    FUZZ_TRUNCATE_START | FUZZ_TRUNCATE_END |
	    FUZZ_BASE64,
	    ibuf_data(data), ibuf_size(data));
	ibuf_free(data);
	perform_test(fuzz);
	TEST_DONE();

	TEST_START("fuzz configuration payload");
	ASSERT_PTR_NE(data = ibuf_new(cookies, sizeof(cookies)), NULL);
	ASSERT_INT_EQ(ibuf_add(data, genhdr, sizeof(genhdr)), 0);
	ASSERT_INT_EQ(ibuf_add(data, cp_pld, sizeof(cp_pld)), 0);
	set_length(ibuf_data(data), ibuf_size(data));
	set_nextpayload(ibuf_data(data), IKEV2_PAYLOAD_CP);
	print_hex(ibuf_data(data), 0, ibuf_size(data));
	prepare_header(&hdr, data);
	prepare_message(&msg, data);
	ASSERT_INT_EQ(ikev2_pld_parse(NULL, &hdr, &msg, 0), 0);
	fuzz = fuzz_begin(FUZZ_1_BIT_FLIP | FUZZ_2_BIT_FLIP |
	    FUZZ_1_BYTE_FLIP | FUZZ_2_BYTE_FLIP |
	    FUZZ_TRUNCATE_START | FUZZ_TRUNCATE_END |
	    FUZZ_BASE64,
	    ibuf_data(data), ibuf_size(data));
	ibuf_free(data);
	perform_test(fuzz);
	TEST_DONE();

	TEST_START("fuzz eap payload");
	ASSERT_PTR_NE(data = ibuf_new(cookies, sizeof(cookies)), NULL);
	ASSERT_INT_EQ(ibuf_add(data, genhdr, sizeof(genhdr)), 0);
	ASSERT_INT_EQ(ibuf_add(data, eap_pld, sizeof(eap_pld)), 0);
	set_length(ibuf_data(data), ibuf_size(data));
	set_nextpayload(ibuf_data(data), IKEV2_PAYLOAD_EAP);
	print_hex(ibuf_data(data), 0, ibuf_size(data));
	prepare_header(&hdr, data);
	prepare_message(&msg, data);
	ASSERT_INT_EQ(ikev2_pld_parse(NULL, &hdr, &msg, 0), 0);
	fuzz = fuzz_begin(FUZZ_1_BIT_FLIP | FUZZ_2_BIT_FLIP |
	    FUZZ_1_BYTE_FLIP | FUZZ_2_BYTE_FLIP |
	    FUZZ_TRUNCATE_START | FUZZ_TRUNCATE_END |
	    FUZZ_BASE64,
	    ibuf_data(data), ibuf_size(data));
	ibuf_free(data);
	perform_test(fuzz);
	TEST_DONE();

	TEST_START("fuzz full valid packet");
	ASSERT_PTR_NE(data = ibuf_new(valid_packet, sizeof(valid_packet)),
	    NULL);
	set_length(ibuf_data(data), ibuf_size(data));
	print_hex(ibuf_data(data), 0, ibuf_size(data));
	prepare_header(&hdr, data);
	prepare_message(&msg, data);
	ASSERT_INT_EQ(ikev2_pld_parse(NULL, &hdr, &msg, 0), 0);
	fuzz = fuzz_begin(FUZZ_1_BIT_FLIP | FUZZ_2_BIT_FLIP |
	    FUZZ_1_BYTE_FLIP | FUZZ_2_BYTE_FLIP |
	    FUZZ_TRUNCATE_START | FUZZ_TRUNCATE_END |
	    FUZZ_BASE64,
	    ibuf_data(data), ibuf_size(data));
	ibuf_free(data);
	perform_test(fuzz);
	TEST_DONE();
}
