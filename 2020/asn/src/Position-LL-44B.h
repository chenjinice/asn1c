/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "DefPositionOffset"
 * 	found in "asn/DefPositionOffset.asn"
 * 	`asn1c -fskeletons-copy -fnative-types -gen-PER -pdu=auto -no-gen-example -D ./src`
 */

#ifndef	_Position_LL_44B_H_
#define	_Position_LL_44B_H_


#include <asn_application.h>

/* Including external dependencies */
#include "OffsetLL-B22.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Position-LL-44B */
typedef struct Position_LL_44B {
	OffsetLL_B22_t	 lon;
	OffsetLL_B22_t	 lat;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Position_LL_44B_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Position_LL_44B;
extern asn_SEQUENCE_specifics_t asn_SPC_Position_LL_44B_specs_1;
extern asn_TYPE_member_t asn_MBR_Position_LL_44B_1[2];

#ifdef __cplusplus
}
#endif

#endif	/* _Position_LL_44B_H_ */
#include <asn_internal.h>
